/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QObject>

#include <memory>

class MessageBoxManager;
class Settings;
class IPC;
class QApplication;
class QCommandLineParser;
class ToxURIDialog;
class Nexus;
class CameraSource;

class AppManager : public QObject
{
    Q_OBJECT

public:
    AppManager(int& argc, char** argv);
    ~AppManager();
    int run();

private:
    int startGui(QCommandLineParser& parser);

private slots:
    void cleanup();

private:
    void preConstructionInitialization();
    std::unique_ptr<QApplication> qapp;
    std::unique_ptr<MessageBoxManager> messageBoxManager;
    std::unique_ptr<Settings> settings;
    std::unique_ptr<IPC> ipc;
    std::unique_ptr<ToxURIDialog> uriDialog;
    std::unique_ptr<CameraSource> cameraSource;
    std::unique_ptr<Nexus> nexus;
};
