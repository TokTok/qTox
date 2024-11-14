/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#include "toxsave.h"
#include "src/persistence/settings.h"
#include "src/widget/widget.h"
#include "src/nexus.h"
#include "src/ipc.h"
#include "src/widget/tool/profileimporter.h"
#include <QCoreApplication>
#include <QString>

const QString ToxSave::eventHandlerKey = QStringLiteral("save");

ToxSave::ToxSave(Settings& settings_, IPC& ipc_, QWidget* parent_)
    : settings{settings_}
    , ipc{ipc_}
    , parent{parent_}
{}

ToxSave::~ToxSave()
{
    ipc.unregisterEventHandler(eventHandlerKey);
}

bool ToxSave::toxSaveEventHandler(const QByteArray& eventData, void* userData)
{
    auto toxSave = static_cast<ToxSave*>(userData);

    if (!eventData.endsWith(".tox")) {
        return false;
    }

    toxSave->handleToxSave(QString::fromUtf8(eventData));
    return true;
}

/**
 * @brief Import new profile.
 * @note Will wait until the core is ready first.
 * @param path Path to .tox file.
 * @return True if import success, false, otherwise.
 */
bool ToxSave::handleToxSave(const QString& path)
{
    ProfileImporter importer(settings, parent);
    return importer.importProfile(path);
}
