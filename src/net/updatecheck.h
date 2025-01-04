/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QUrl>

class Settings;
class QString;
class QUrl;
class QNetworkReply;

class UpdateCheck : public QObject
{
    Q_OBJECT

public:
    static bool isCurrentVersionStable();

    explicit UpdateCheck(const Settings& settings_, QObject* parent = nullptr);

    constexpr bool canCheck() const
    {
#ifdef UPDATE_CHECK_ENABLED
        return true;
#else
        return false;
#endif
    }

    void checkForUpdate();

signals:
    void complete(QString currentVersion, QString latestVersion, QUrl link);
    void updateAvailable(QString latestVersion, QUrl link);
    void upToDate();
    void updateCheckFailed();
    void versionIsUnstable();

private slots:
    void handleResponse(QNetworkReply* reply);

private:
    QNetworkAccessManager manager;
    QTimer updateTimer;
    const Settings& settings;
};
