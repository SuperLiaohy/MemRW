//
// Created by liaohy on 8/29/25.
//

#include "TreeModel.h"

TreeModel::TreeModel(const std::shared_ptr<VariTree> &data, QObject *parent)
    : QAbstractItemModel(parent),
      tree(data) {
}

TreeModel::~TreeModel() {
}

QVariant TreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    if (role == Qt::DisplayRole) {
        auto item = static_cast<VariNode *>(index.internalPointer());
        return QString::fromStdString(item->data(index.column()));
    }
    return QVariant();
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return QString("Name");
            case 1:
                return QString("Type");
            case 2:
                return QString("Address");
            case 3:
                return QString("Size");
        }
    }
    return QVariant();
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) return QModelIndex();
    VariNode *parentItem = parent.isValid()
                               ? static_cast<VariNode *>(parent.internalPointer())
                               : tree->root.get();
    VariNode *childItem = parentItem->child(row);
    if (childItem) return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex &index) const {
    if (!index.isValid()) return QModelIndex();
    auto *childItem = static_cast<VariNode *>(index.internalPointer());
    VariNode *parentItem = childItem->parent();

    return parentItem != tree->root.get()
               ? createIndex(parentItem->childCount(), 0, parentItem)
               : QModelIndex{};
}

int TreeModel::rowCount(const QModelIndex &parent) const {
    if (!parent.isValid()) { return tree->root->childCount(); }
    auto parent_node = static_cast<VariNode *>(parent.internalPointer());
    return parent_node->childCount();
}

int TreeModel::columnCount(const QModelIndex &parent) const {
    return 4;
}
