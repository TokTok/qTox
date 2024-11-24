/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QNetworkAccessManager>
#include <QObject>
#include <QTimer>
#include <QUrl>

#include <memory>

#ifdef UPDATE_CHECK_ENABLED
class Settings;
class QString;
class QUrl;
class QNetworkReply;
class UpdateCheck : public QObject
{
    Q_OBJECT

public:
    UpdateCheck(const Settings& settings_);
    void checkForUpdate();

signals:
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
#endif
