//
// Created by liaohy on 8/29/25.
//

#include "TreeItem.h"

TreeItem::TreeItem(const std::shared_ptr<VariNode> &origin_data, TreeItem *parent) : m_parent(parent) {
    node = origin_data;
    for (auto & origin_child : node->node) {
        appendChild(std::make_shared<TreeItem>(origin_child, this));
    }
}

void TreeItem::appendChild(const std::shared_ptr<TreeItem> &child) {
    child->m_parent = this;
    child_list.push_back(child);
}

void TreeItem::appendChild(const std::shared_ptr<OriginType> &child) {
    child_list.push_back(std::make_shared<TreeItem>(child,this));
}

int TreeItem::childCount() {
    return static_cast<int>(child_list.size());
}

TreeItem *TreeItem::parent() {
    return m_parent;
}

TreeItem *TreeItem::child(int row) {
    if (row < 0 || row >= child_list.size()) { return nullptr; }
    return child_list[row].get();
}

QVariant TreeItem::data(int column) const {
    switch (column) {
        case 0:
            return QString::fromStdString(node->name);
        case 1:
            return QString::fromStdString(node->type);
        case 2:
            if (node->type=="cu") return QVariant();
            return QString("0x%1").arg(node->addr,0,16);
        default:
            return QVariant();

    }
}
