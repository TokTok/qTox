/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024 The TokTok team.
 */

#pragma once

#include <QAbstractItemModel>

class DebugObjectTreeModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit DebugObjectTreeModel(QObject* parent = nullptr);
    ~DebugObjectTreeModel() override;

    void reload();

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    class TreeItem;

private:
    std::unique_ptr<TreeItem> root_;
};
