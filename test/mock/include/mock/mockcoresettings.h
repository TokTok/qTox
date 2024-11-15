/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "src/core/dhtserver.h"
#include "src/core/icoresettings.h"
#include "src/core/toxpk.h"
#include "src/model/status.h"

#include <QList>
#include <QObject>

class MockSettings : public QObject, public ICoreSettings
{
    Q_OBJECT
public:
    MockSettings()
    {
        Q_INIT_RESOURCE(res);
    }

    ~MockSettings();

    bool getEnableIPv6() const override
    {
        return false;
    }
    void setEnableIPv6(bool enable) override { std::ignore = enable; }

    bool getForceTCP() const override
    {
        return false;
    }
    void setForceTCP(bool force) override { std::ignore = force; }

    bool getEnableLanDiscovery() const override
    {
        return false;
    }
    void setEnableLanDiscovery(bool enable) override { std::ignore = enable; }

    QString getProxyAddr() const override
    {
        return addr;
    }
    void setProxyAddr(const QString& addr_) override
    {
        addr = addr_;
    }

    ProxyType getProxyType() const override
    {
        return type;
    }
    void setProxyType(ProxyType type_) override
    {
        type = type_;
    }

    quint16 getProxyPort() const override
    {
        return port;
    }
    void setProxyPort(quint16 port_) override
    {
        port = port_;
    }

    QNetworkProxy getProxy() const override
    {
        return QNetworkProxy(QNetworkProxy::ProxyType::NoProxy);
    }

    SIGNAL_IMPL(MockSettings, enableIPv6Changed, bool enabled)
    SIGNAL_IMPL(MockSettings, forceTCPChanged, bool enabled)
    SIGNAL_IMPL(MockSettings, enableLanDiscoveryChanged, bool enabled)
    SIGNAL_IMPL(MockSettings, proxyTypeChanged, ICoreSettings::ProxyType type)
    SIGNAL_IMPL(MockSettings, proxyAddressChanged, const QString& address)
    SIGNAL_IMPL(MockSettings, proxyPortChanged, quint16 port)

private:
    QString addr;
    ProxyType type;
    quint16 port;
};
