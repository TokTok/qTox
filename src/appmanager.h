/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2022 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */

#pragma once

#include <QObject>

#include <memory>

class CameraSource;
class IPC;
class MessageBoxManager;
class Nexus;
class QApplication;
class QCommandLineParser;
class Sandbox;
class Settings;
class ToxURIDialog;

class AppManager : public QObject
{
    Q_OBJECT

public:
    AppManager(int& argc, char** argv);
    ~AppManager() override;
    int run();

private:
    int startGui(QCommandLineParser& parser);

private slots:
    void cleanup();

private:
    static void preConstructionInitialization();
    std::unique_ptr<QApplication> qapp;
    std::unique_ptr<MessageBoxManager> messageBoxManager;
    std::unique_ptr<Settings> settings;
    std::unique_ptr<IPC> ipc;
    std::unique_ptr<ToxURIDialog> uriDialog;
    std::unique_ptr<CameraSource> cameraSource;
    std::unique_ptr<Nexus> nexus;
    std::unique_ptr<Sandbox> sandbox;
};
