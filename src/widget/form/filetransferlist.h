/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2026 The TokTok team.
 */

#pragma once

#include "src/core/toxfile.h"

#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QTableView>

class FriendList;
class Settings;
class Style;

namespace FileTransferList {

enum class Column : int
{
    // NOTE: Order defines order in UI
    fileName,
    contact,
    progress,
    size,
    speed,
    status,
    control,
    invalid,
};

Column toFileTransferListColumn(int in);
QString toQString(Column column);

enum class EditorAction : int
{
    pause,
    cancel,
    invalid,
};

EditorAction toEditorAction(int in);

class Model : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit Model(FriendList& friendList, QObject* parent = nullptr);
    ~Model() override = default;

    void onFileUpdated(const ToxFile& file);

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

signals:
    void togglePause(ToxFile file);
    void cancel(ToxFile file);

private:
    QHash<QByteArray /*file id*/, int /*row index*/> idToRow;
    std::vector<ToxFile> files;
    FriendList& friendList;
};

class Delegate : public QStyledItemDelegate
{
public:
    Delegate(Settings& settings, Style& style, QWidget* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;

    bool editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem& option,
                     const QModelIndex& index) override;

private:
    Settings& settings;
    Style& style;
};

class View : public QTableView
{
public:
    View(QAbstractItemModel* model, Settings& settings, Style& style, QWidget* parent = nullptr);
    ~View() override;
};
} // namespace FileTransferList
