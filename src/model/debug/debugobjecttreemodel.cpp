/* SPDX-License-Identifier: GPL-3.0-or-later
 * Copyright © 2024-2025 The TokTok team.
 */

#include "debugobjecttreemodel.h"

class DebugObjectTreeModel::TreeItem
{
public:
    const TreeItem* child(int row)
    {
        return row >= 0 && row < childCount() ? children.at(row).get() : nullptr;
    }

    int childCount() const
    {
        return int(children.size());
    }

    int columnCount() const
    {
        return 3;
    }

    QVariant data(int column) const
    {
        switch (column) {
        case 0:
            return name;
        case 1:
            return type;
        case 2:
            return QString::number(address, 16);
        default:
            return {};
        }
    }

    int row() const
    {
        if (parent == nullptr) {
            return 0;
        }
        const auto it = std::find_if(parent->children.cbegin(), parent->children.cend(),
                                     [this](const std::unique_ptr<TreeItem>& treeItem) {
                                         return treeItem.get() == this;
                                     });

        if (it != parent->children.cend()) {
            return std::distance(parent->children.cbegin(), it);
        }
        qFatal("Parent tree item does not contain this item");
    }

    TreeItem* parentItem() const
    {
        return parent;
    }

    uintptr_t address = 0;
    QString name;
    QString type;
    TreeItem* parent = nullptr;
    std::vector<std::unique_ptr<TreeItem>> children;
};

namespace {
std::unique_ptr<DebugObjectTreeModel::TreeItem> buildObjectTree(const QObject* object,
                                                                DebugObjectTreeModel::TreeItem* parent)
{
    auto tree = std::make_unique<DebugObjectTreeModel::TreeItem>();
    tree->address = reinterpret_cast<uintptr_t>(object);
    tree->name = object->objectName();
    tree->type = QString::fromLatin1(object->metaObject()->className());
    tree->parent = parent;

    for (auto* child : object->children()) {
        tree->children.push_back(buildObjectTree(child, tree.get()));
    }

    return tree;
}
} // namespace

DebugObjectTreeModel::DebugObjectTreeModel(QObject* parent)
    : QAbstractItemModel(parent)
    , root_(std::make_unique<TreeItem>())
{
    reload();
}

DebugObjectTreeModel::~DebugObjectTreeModel() = default;

void DebugObjectTreeModel::reload()
{
    // Find the root object
    QObject* root = this;
    while (root->parent() != nullptr) {
        root = root->parent();
    }

    auto tree = buildObjectTree(root, root_.get());

    beginResetModel();
    root_->children.clear();
    root_->children.push_back(std::move(tree));
    endResetModel();
}

QModelIndex DebugObjectTreeModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
        return {};

    TreeItem* parentItem =
        parent.isValid() ? static_cast<TreeItem*>(parent.internalPointer()) : root_.get();

    if (auto* childItem = parentItem->child(row))
        return createIndex(row, column, childItem);
    return {};
}

QModelIndex DebugObjectTreeModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return {};

    auto* childItem = static_cast<TreeItem*>(index.internalPointer());
    TreeItem* parentItem = childItem->parentItem();

    return parentItem != root_.get() ? createIndex(parentItem->row(), 0, parentItem) : QModelIndex{};
}

int DebugObjectTreeModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;

    const TreeItem* parentItem =
        parent.isValid() ? static_cast<const TreeItem*>(parent.internalPointer()) : root_.get();

    return parentItem->childCount();
}

int DebugObjectTreeModel::columnCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return static_cast<TreeItem*>(parent.internalPointer())->columnCount();
    return root_->columnCount();
}

QVariant DebugObjectTreeModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return {};

    const auto* item = static_cast<const TreeItem*>(index.internalPointer());
    return item->data(index.column());
}

Qt::ItemFlags DebugObjectTreeModel::flags(const QModelIndex& index) const
{
    return index.isValid() ? QAbstractItemModel::flags(index) : Qt::ItemFlags(Qt::NoItemFlags);
}

QVariant DebugObjectTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
    case 0:
        return QStringLiteral("Name");
    case 1:
        return QStringLiteral("Type");
    case 2:
        return QStringLiteral("Address");
    default:
        return {};
    }
}
