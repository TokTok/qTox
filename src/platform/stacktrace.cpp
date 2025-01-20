/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2025 The TokTok team.
 */

#include "stacktrace.h"

#include <QDebug>
#include <cstring>
#include <cxxabi.h> // __cxa_demangle (GNU-like dependency)

#ifdef QTOX_USE_LIBUNWIND
#define UNW_LOCAL_ONLY
#include <dlfcn.h>
#include <libunwind.h>

void Stacktrace::detail::process(Stacktrace::detail::Callback* callback, void* userdata)
{
    unw_cursor_t cursor;
    unw_context_t context;

    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    int i = 0;
    while (unw_step(&cursor) > 0) {
        unw_word_t offset, pc;
        unw_get_reg(&cursor, UNW_REG_IP, &pc);
        char name[256];
        unw_get_proc_name(&cursor, name, sizeof(name), &offset);

        Dl_info info;
        if (dladdr(reinterpret_cast<void*>(pc), &info) == 0) {
            callback(userdata, Frame{i, pc, "", name, offset});
            ++i;
            continue;
        }

        const auto base = pc - reinterpret_cast<uintptr_t>(info.dli_fbase);
        callback(userdata, Frame{i, base, info.dli_fname, name, offset});
        ++i;
    }
}
#elif defined(Q_OS_MACOS) || defined(__GLIBC__)
#include <dlfcn.h>
#include <execinfo.h>

void Stacktrace::detail::process(Stacktrace::detail::Callback* callback, void* userdata)
{
    void* buffer[256];
    const auto size = backtrace(buffer, sizeof(buffer) / sizeof(buffer[0]));

    for (int i = 0; i < size; ++i) {
        Dl_info info;
        if (dladdr(buffer[i], &info) == 0) {
            callback(userdata,
                     Frame{i, reinterpret_cast<uintptr_t>(buffer[i]), "<binary>", "<function>", 0});
            continue;
        }

        const auto sym = reinterpret_cast<uintptr_t>(info.dli_saddr)
                         - reinterpret_cast<uintptr_t>(info.dli_fbase);
        const auto offset =
            reinterpret_cast<uintptr_t>(buffer[i]) - reinterpret_cast<uintptr_t>(info.dli_saddr);
        callback(userdata, Frame{i, sym + offset, info.dli_fname, info.dli_sname, offset});
    }
}
#else
void Stacktrace::detail::process(Stacktrace::detail::Callback* callback, void* userdata)
{
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
