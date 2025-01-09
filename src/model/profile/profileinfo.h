/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2017-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include "iprofileinfo.h"

#include "src/core/toxpk.h"
#include "util/interface.h"

#include <QObject>

class Core;
class QFile;
class QPoint;
class Profile;
class Settings;
class Nexus;

class ProfileInfo : public QObject, public IProfileInfo
{
    Q_OBJECT
public:
    ProfileInfo(Core* core_, Profile* profile_, Settings& settings, Nexus& nexus);

    bool setPassword(const QString& password) override;
    bool deletePassword() override;
    bool isEncrypted() const override;

    void copyId() const override;

    void setUsername(const QString& name) override;
    void setStatusMessage(const QString& status) override;

    QString getProfileName() const override;
    RenameResult renameProfile(const QString& name) override;
    SaveResult exportProfile(const QString& path) const override;
    QStringList removeProfile() override;
    void logout() override;

    void copyQr(const QImage& image) const override;
    SaveResult saveQr(const QImage& image, const QString& path) const override;

    SetAvatarResult setAvatar(const QString& path) override;
    void removeAvatar() override;

    SIGNAL_IMPL(ProfileInfo, idChanged, const ToxId& id)
    SIGNAL_IMPL(ProfileInfo, usernameChanged, const QString& name)
    SIGNAL_IMPL(ProfileInfo, statusMessageChanged, const QString& message)

private:
    IProfileInfo::SetAvatarResult createAvatarFromFile(QFile& file, QByteArray& avatar);
    IProfileInfo::SetAvatarResult byteArrayToPng(QByteArray inData, QByteArray& outPng);
    IProfileInfo::SetAvatarResult scalePngToAvatar(QByteArray& avatar);
    Profile* const profile;
    Core* const core;
    Settings& settings;
    Nexus& nexus;
};
