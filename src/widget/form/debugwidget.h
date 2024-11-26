/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QWidget>

#include <array>

class ContentLayout;
class GenericForm;
class Paths;
class QTabWidget;
class QVBoxLayout;
class Style;
class Widget;

class DebugWidget : public QWidget
{
    Q_OBJECT
public:
    DebugWidget(Paths& paths, Style& style, Widget* parent = nullptr);
    ~DebugWidget();

    bool isShown() const;
    void show(ContentLayout* contentLayout);
    void setBodyHeadStyle(QString style);

private slots:
    void onTabChanged(int index);

private:
    void retranslateUi();

private:
    QVBoxLayout* bodyLayout;
    QTabWidget* debugWidgets;
    std::array<GenericForm*, 2> dbgForms;
    int currentIndex;
};
