/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2019 by The qTox Project Contributors
 * Copyright © 2024-2026 The TokTok team.
 */

#pragma once

#include "util/interface.h"

#include <QList>
#include <QNetworkProxy>
#include <QString>

class ICoreSettings
{
public:
    enum class ProxyType
    {
        // If changed, don't forget to update Settings::fixInvalidProxyType
        ptNone = 0,
        ptSOCKS5 = 1,
        ptHTTP = 2
    };
    ICoreSettings() = default;
    virtual ~ICoreSettings();
    ICoreSettings(const ICoreSettings&) = default;
    ICoreSettings& operator=(const ICoreSettings&) = default;
    ICoreSettings(ICoreSettings&&) = default;
    ICoreSettings& operator=(ICoreSettings&&) = default;

    virtual bool getEnableIPv6() const = 0;
    virtual void setEnableIPv6(bool enable) = 0;

    virtual bool getForceTCP() const = 0;
    virtual void setForceTCP(bool enable) = 0;

    virtual bool getEnableLanDiscovery() const = 0;
    virtual void setEnableLanDiscovery(bool enable) = 0;

    virtual QString getProxyAddr() const = 0;
    virtual void setProxyAddr(const QString& address) = 0;

    virtual ProxyType getProxyType() const = 0;
    virtual void setProxyType(ProxyType type) = 0;

    virtual quint16 getProxyPort() const = 0;
    virtual void setProxyPort(quint16 port) = 0;

    virtual QNetworkProxy getProxy() const = 0;

    virtual QStringList getSavedGroups() const = 0;

    DECLARE_SIGNAL(enableIPv6Changed, bool enabled);
    DECLARE_SIGNAL(forceTCPChanged, bool enabled);
    DECLARE_SIGNAL(enableLanDiscoveryChanged, bool enabled);
    DECLARE_SIGNAL(proxyTypeChanged, ICoreSettings::ProxyType type);
    DECLARE_SIGNAL(proxyAddressChanged, const QString& address);
    DECLARE_SIGNAL(proxyPortChanged, quint16 port);
};
