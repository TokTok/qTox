/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include <QApplication>
#include "src/persistence/settings.h"
#include "src/platform/autorun.h"
#include <string>
#include <windows.h>

namespace {
#ifdef UNICODE
/**
 * tstring is either std::wstring or std::string, depending on whether the user
 * is building a Unicode or Multi-Byte version of qTox. This makes the code
 * easier to reuse and compatible with both setups.
 */
using tstring = std::wstring;
inline tstring toTString(QString s)
{
    return s.toStdWString();
}
#else
using tstring = std::string;
inline tstring toTString(QString s)
{
    return s.toStdString();
}
#endif
} // namespace

namespace Platform {
inline tstring currentCommandLine(const Settings& settings)
{
    return toTString("\"" + QApplication::applicationFilePath().replace('/', '\\') + "\" -p \""
                     + settings.getCurrentProfile() + "\"");
}

inline tstring currentRegistryKeyName(const Settings& settings)
{
    return toTString("qTox - " + settings.getCurrentProfile());
}
}

bool Platform::setAutorun(const Settings& settings, bool on)
{
    HKEY key = nullptr;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                     0, KEY_ALL_ACCESS, &key)
        != ERROR_SUCCESS)
        return false;

    bool result = false;
    tstring keyName = currentRegistryKeyName(settings);

    if (on) {
        tstring path = currentCommandLine(settings);
        result = RegSetValueEx(key, keyName.c_str(), 0, REG_SZ, const_cast<PBYTE>(reinterpret_cast<const unsigned char*>(path.c_str())),
                               path.length() * sizeof(TCHAR))
                 == ERROR_SUCCESS;
    } else
        result = RegDeleteValue(key, keyName.c_str()) == ERROR_SUCCESS;

    RegCloseKey(key);
    return result;
}

bool Platform::getAutorun(const Settings& settings)
{
    HKEY key = nullptr;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"),
                     0, KEY_ALL_ACCESS, &key)
        != ERROR_SUCCESS)
        return false;

    tstring keyName = currentRegistryKeyName(settings);

    TCHAR path[MAX_PATH] = {0};
    DWORD length = sizeof(path);
    DWORD type = REG_SZ;
    bool result = false;

    if (RegQueryValueEx(key, keyName.c_str(), nullptr, &type, const_cast<PBYTE>(reinterpret_cast<const unsigned char*>(path)), &length) == ERROR_SUCCESS
        && type == REG_SZ)
        result = true;

    RegCloseKey(key);
    return result;
}
