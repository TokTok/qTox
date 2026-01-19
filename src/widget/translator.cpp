/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2014-2019 by The qTox Project Contributors
 * Copyright © 2024-2025 The TokTok team.
 */


#include "translator.h"

#include <QApplication>
#include <QDebug>
#include <QLibraryInfo>
#include <QLocale>
#include <QMutexLocker>
#include <QString>
#include <QTranslator>

#include <algorithm>

QTranslator* Translator::core_translator{nullptr};
QTranslator* Translator::app_translator{nullptr};
QVector<Translator::Callback> Translator::callbacks;
QMutex Translator::lock;

/**
 * @brief Loads the translations according to the settings or locale.
 */
void Translator::translate(const QString& localeName)
{
    const QMutexLocker<QMutex> locker{&lock};

    if (core_translator == nullptr)
        core_translator = new QTranslator();

    if (app_translator == nullptr)
        app_translator = new QTranslator();

    // Remove old translations
    QCoreApplication::removeTranslator(core_translator);
    QApplication::removeTranslator(app_translator);

    // Load translations
    const QString& locale = localeName.isEmpty() ? "en" : localeName;

    if (core_translator->load(locale, ":translations/")) {
        qDebug() << "Loaded translation" << locale;

        // System menu translation
        const QString s_locale = "qt_" + locale;
        const QString location = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
        if (app_translator->load(s_locale, location)) {
            QApplication::installTranslator(app_translator);
            qDebug() << "System translation loaded" << locale;
        } else {
            qDebug() << "System translation not loaded" << locale;
        }

        // Application translation
        QCoreApplication::installTranslator(core_translator);
    } else {
        qDebug() << "Error loading translation" << locale;
    }

    // After the language is changed from RTL to LTR, the layout direction isn't
    // always restored
    const QString direction =
        QApplication::tr("LTR", "DO NOT TRANSLATE. Instead, set this string "
                                "to 'RTL' in right-to-left languages (for example "
                                "Hebrew and Arabic) to get proper widget layout. "
                                "This string should only ever be 'LTR' or 'RTL'.");

    QGuiApplication::setLayoutDirection(direction == "RTL" ? Qt::RightToLeft : Qt::LeftToRight);

    for (auto pair : callbacks)
        pair.second();
}

/**
 * @brief Register a function to be called when the UI needs to be retranslated.
 * @param f Function, which will called.
 * @param owner Widget to retranslate.
 */
void Translator::registerHandler(const std::function<void()>& f, QObject* owner)
{
    const QMutexLocker<QMutex> locker{&lock};
    callbacks.push_back({owner, f});
}

/**
 * @brief Unregisters all handlers of an owner.
 * @param owner Owner to unregister.
 */
void Translator::unregister(QObject* owner)
{
    const QMutexLocker<QMutex> locker{&lock};
    callbacks.erase(std::remove_if(begin(callbacks), end(callbacks),
                                   [=](const Callback& c) { return c.first == owner; }),
                    end(callbacks));
}
