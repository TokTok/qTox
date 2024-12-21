/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2015-2019 by The qTox Project Contributors
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include "searchtypes.h"
#include <QLineEdit>
#include <QWidget>

class QPushButton;
class QLabel;
class LineEdit;
class SearchSettingsForm;
class Settings;
class Style;

class SearchForm final : public QWidget
{
    Q_OBJECT
public:
    enum class ToolButtonState
    {
        Disabled = 0, // Grey
        Common = 1,   // Green
        Active = 2,   // Red
    };

    SearchForm(Settings& settings, Style& style, QWidget* parent = nullptr);
    void removeSearchPhrase();
    QString getSearchPhrase() const;
    ParameterSearch getParameterSearch();
    void setFocusEditor();
    void insertEditor(const QString& text);

protected:
    void showEvent(QShowEvent* event) final;

private:
    // TODO: Merge with 'createButton' from chatformheader.cpp
    QPushButton* createButton(const QString& name, const QString& state);
    ParameterSearch getAndCheckParameterSearch();
    void setStateName(QPushButton* btn, ToolButtonState state);
    void useBeginState();

    QPushButton* settingsButton;
    QPushButton* upButton;
    QPushButton* downButton;
    QPushButton* hideButton;
    QPushButton* startButton;
    LineEdit* searchLine;
    SearchSettingsForm* searchSettingsForm;
    QLabel* messageLabel;

    QString searchPhrase;
    ParameterSearch parameter;

    bool isActiveSettings{false};
    bool isChangedPhrase{false};
    bool isSearchInBegin{true};
    bool isPrevSearch{false};
    Settings& settings;
    Style& style;

private slots:
    void changedSearchPhrase(const QString& text);
    void clickedUp();
    void clickedDown();
    void clickedHide();
    void clickedStart();
    void clickedSearch();
    void changedState(bool isUpdate);

public slots:
    void showMessageNotFound(SearchDirection direction);
    void reloadTheme();

signals:
    void searchInBegin(const QString& phrase, const ParameterSearch& parameter);
    void searchUp(const QString& phrase, const ParameterSearch& parameter);
    void searchDown(const QString& phrase, const ParameterSearch& parameter);
    void visibleChanged();
};

class LineEdit : public QLineEdit
{
    Q_OBJECT

public:
    LineEdit(QWidget* parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent* event) final;

signals:
    void clickEnter();
    void clickShiftEnter();
    void clickEsc();
};
