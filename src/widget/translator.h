/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */


#pragma once

#include <QMutex>
#include <QPair>
#include <QVector>
#include <functional>

class QTranslator;

class Translator
{
public:
    static void translate(const QString& localeName);
    static void registerHandler(const std::function<void()>& f, void* owner);
    static void unregister(void* owner);

private:
    using Callback = QPair<void*, std::function<void()>>;
    static QVector<Callback> callbacks;
    static QMutex lock;
    static QTranslator* core_translator;
    static QTranslator* app_translator;
};
