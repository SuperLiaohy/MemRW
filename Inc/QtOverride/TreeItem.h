//
// Created by liaohy on 8/29/25.
//

#pragma once

#include <QVariant>
#include "VariTree.h"



class TreeItem {
public:
    using OriginType = VariNode;
    explicit TreeItem(const std::shared_ptr<OriginType>& origin_data, TreeItem *parent = nullptr);

    void appendChild(const std::shared_ptr<TreeItem>& child);
    void appendChild(const std::shared_ptr<OriginType>& child);
    int childCount();
    static int columnCount() {return 3;}
    TreeItem* parent();
    TreeItem* child(int row);
    QVariant data(int column) const;

private:
    std::shared_ptr<OriginType> node;
    QList<std::shared_ptr<TreeItem>> child_list;
    TreeItem* m_parent;
};



