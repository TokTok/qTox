/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QBoxLayout>
#include <QFrame>

class Settings;
class Style;

class ContentLayout : public QVBoxLayout
{
public:
    ContentLayout(Settings& settings, Style& style);
    explicit ContentLayout(Settings& settings, Style& style, QWidget* parent);
    ~ContentLayout();

    void clear();

    QFrame mainHLine;
    QHBoxLayout mainHLineLayout;
    QWidget* mainContent;
    QWidget* mainHead;
    Settings& settings;
    Style& style;

public slots:
    void reloadTheme();

private:
    void init();
};
