/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QByteArray>

#include <memory>

class ICoreSettings;
struct Tox_Options;

class ToxOptions
{
public:
    ~ToxOptions();
    ToxOptions(ToxOptions&& from);
    operator Tox_Options*();
    const char* getProxyAddrData() const;
    static std::unique_ptr<ToxOptions> makeToxOptions(const QByteArray& savedata,
                                                      const ICoreSettings& s);
    bool getIPv6Enabled() const;
    void setIPv6Enabled(bool enabled);

private:
    ToxOptions(Tox_Options* options_, QByteArray proxyAddrData_);

private:
    Tox_Options* options = nullptr;
    QByteArray proxyAddrData;
};
