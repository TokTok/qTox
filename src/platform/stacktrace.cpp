/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025-2026 The TokTok team.
 */

#include "stacktrace.h"

#include <QDebug>

#include <cstring>
#include <cxxabi.h> // __cxa_demangle (GNU-like dependency)

#if defined(QTOX_USE_LIBUNWIND) || defined(Q_OS_MACOS) || defined(__GLIBC__)
#include <dlfcn.h>
#include <sys/ucontext.h>

// Platforms where we can read the address that a signal interrupted.
#if (defined(Q_OS_MACOS) || defined(Q_OS_LINUX)) && (defined(__x86_64__) || defined(__aarch64__))
#define QTOX_HAVE_INTERRUPTED_PC

namespace {
/** Where the signal interrupted the thread. */
uintptr_t interruptedPc(const ucontext_t& context)
{
#if defined(Q_OS_MACOS) && defined(__x86_64__)
    return context.uc_mcontext->__ss.__rip;
#elif defined(Q_OS_MACOS) && defined(__aarch64__)
    return __darwin_arm_thread_state64_get_pc(context.uc_mcontext->__ss);
#elif defined(Q_OS_LINUX) && defined(__x86_64__)
    return context.uc_mcontext.gregs[REG_RIP];
#elif defined(Q_OS_LINUX) && defined(__aarch64__)
    return context.uc_mcontext.pc;
#endif
}
} // namespace
#endif
#endif

#ifdef QTOX_USE_LIBUNWIND
#define UNW_LOCAL_ONLY
#include <libunwind.h>

namespace {
/** Calls `visit` with each frame above the caller's, while it returns true. */
template <typename Visit>
void walk(Visit&& visit)
{
    unw_cursor_t cursor;
    unw_context_t context;

    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    while (unw_step(&cursor) > 0 && visit(cursor)) {
    }
}

uintptr_t pcOf(unw_cursor_t& cursor)
{
    unw_word_t pc;
    unw_get_reg(&cursor, UNW_REG_IP, &pc);
    return pc;
}

void describe(Stacktrace::detail::Callback* callback, void* userdata, int number, unw_cursor_t& cursor)
{
    const uintptr_t pc = pcOf(cursor);
    unw_word_t offset;
    char name[256];
    unw_get_proc_name(&cursor, name, sizeof(name), &offset);

    Dl_info info;
    if (dladdr(reinterpret_cast<void*>(pc), &info) == 0) {
        callback(userdata, Stacktrace::Frame{number, pc, "", name, offset});
        return;
    }

    const auto base = pc - reinterpret_cast<uintptr_t>(info.dli_fbase);
    callback(userdata, Stacktrace::Frame{number, base, info.dli_fname, name, offset});
}
} // namespace

void Stacktrace::detail::process(Stacktrace::detail::Callback* callback, void* userdata)
{
    int i = 0;
    walk([&](unw_cursor_t& cursor) {
        describe(callback, userdata, i++, cursor);
        return true;
    });
}

void Stacktrace::detail::processInterrupted(const void* context,
                                            Stacktrace::detail::Callback* callback, void* userdata)
{
#ifdef QTOX_HAVE_INTERRUPTED_PC
    // The unwinder steps over the signal handler to the interrupted frame, so
    // we start there.
    const uintptr_t pc = interruptedPc(*static_cast<const ucontext_t*>(context));
    bool found = false;
    walk([&](unw_cursor_t& cursor) {
        found = pcOf(cursor) == pc;
        return !found;
    });
    if (found) {
        int i = -1;
        walk([&](unw_cursor_t& cursor) {
            if (i < 0 && pcOf(cursor) != pc) {
                return true;
            }
            describe(callback, userdata, ++i, cursor);
            return true;
        });
        return;
    }
#else
    std::ignore = context;
#endif
    process(callback, userdata);
}
#elif defined(Q_OS_MACOS) || defined(__GLIBC__)
#include <execinfo.h>

namespace {
constexpr int maxFrames = 256;

void describe(Stacktrace::detail::Callback* callback, void* userdata, void* const* frames, int size)
{
    for (int i = 0; i < size; ++i) {
        Dl_info info;
        if (dladdr(frames[i], &info) == 0) {
            callback(userdata, Stacktrace::Frame{i, reinterpret_cast<uintptr_t>(frames[i]),
                                                 "<binary>", "<function>", 0});
            continue;
        }

        const auto sym = reinterpret_cast<uintptr_t>(info.dli_saddr)
                         - reinterpret_cast<uintptr_t>(info.dli_fbase);
        const auto offset =
            reinterpret_cast<uintptr_t>(frames[i]) - reinterpret_cast<uintptr_t>(info.dli_saddr);
        callback(userdata, Stacktrace::Frame{i, sym + offset, info.dli_fname, info.dli_sname, offset});
    }
}
} // namespace

void Stacktrace::detail::process(Stacktrace::detail::Callback* callback, void* userdata)
{
    void* frames[maxFrames];
    describe(callback, userdata, frames, backtrace(frames, maxFrames));
}

void Stacktrace::detail::processInterrupted(const void* context,
                                            Stacktrace::detail::Callback* callback, void* userdata)
{
    const auto& ucontext = *static_cast<const ucontext_t*>(context);
    void* frames[maxFrames];
#ifdef Q_OS_MACOS
    // backtrace() follows the frame pointers from the handler, and loses the
    // interrupted frame at the signal trampoline. We follow them from the
    // interrupted frame instead: macOS code always keeps frame pointers.
#if defined(__x86_64__)
    void* const fp = reinterpret_cast<void*>(ucontext.uc_mcontext->__ss.__rbp);
#else
    void* const fp =
        reinterpret_cast<void*>(__darwin_arm_thread_state64_get_fp(ucontext.uc_mcontext->__ss));
#endif
    frames[0] = reinterpret_cast<void*>(interruptedPc(ucontext));
    describe(callback, userdata, frames, 1 + backtrace_from_fp(fp, frames + 1, maxFrames - 1));
#else
    const int size = backtrace(frames, maxFrames);
#ifdef QTOX_HAVE_INTERRUPTED_PC
    // backtrace() steps over the signal handler to the interrupted frame, so
    // we start there.
    const auto pc = reinterpret_cast<void*>(interruptedPc(ucontext));
    for (int i = 0; i < size; ++i) {
        if (frames[i] == pc) {
            describe(callback, userdata, frames + i, size - i);
            return;
        }
    }
#else
    std::ignore = ucontext;
#endif
    describe(callback, userdata, frames, size);
#endif
}
#else
void Stacktrace::detail::process(Stacktrace::detail::Callback* callback, void* userdata)
{
    std::ignore = callback;
    std::ignore = userdata;
}

void Stacktrace::detail::processInterrupted(const void* context,
                                            Stacktrace::detail::Callback* callback, void* userdata)
{
    std::ignore = context;
    std::ignore = callback;
    std::ignore = userdata;
}
#endif

namespace {
struct FreeDeleter
{
    Q_DECL_UNUSED
    void operator()(char* ptr) const
    {
        std::free(ptr);
    }
};
using char_ptr = std::unique_ptr<char, FreeDeleter>;

const char* pathBasename(const char* path)
{
    const char* base = std::strrchr(path, '/');
    return (base != nullptr) ? base + 1 : path;
}

char_ptr demangle(const char* name)
{
    int status;
    char_ptr demangled{abi::__cxa_demangle(name, nullptr, nullptr, &status)};
    if (status == 0) {
        return demangled;
    }

    return char_ptr{strdup(name != nullptr ? name : "<unknown>")};
}
} // namespace

QDebug Stacktrace::operator<<(QDebug dbg, const Stacktrace::Frame& frame)
{
    return dbg.nospace().noquote()
           << "    #" << frame.number << " 0x" << QString::number(frame.base, 16) << " in "
           << demangle(frame.function).get() << "+0x" << QString::number(frame.offset, 16) << " ("
           << pathBasename(frame.object) << ")";
}
