//
// Created by liaohy on 8/7/25.
//

#pragma once

#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include <memory>


class VariNode {
public:
    VariNode() : addr(0) {};
    VariNode(std::string name, std::string type ,const uint64_t addr) : name(std::move(name)), type(std::move(type)), addr(addr), node() {};

    std::string name;
    std::string type;
    uint64_t addr;
    std::vector<std::shared_ptr<VariNode>> node;

    void add_child(std::string name, std::string type ,const uint64_t addr) {
        node.push_back(std::make_shared<VariNode>(std::move(name),std::move(type), addr));
    }

    void add_child(const std::shared_ptr<VariNode>& child) {
        node.push_back(child);
    }
};

class VariTree {
public:

    VariTree(const std::shared_ptr<VariNode>& root) : root(root) {}

    VariTree(std::string name, std::string type ,const uint64_t addr) {
        root = std::make_shared<VariNode>(std::move(name), std::move(type), addr);
    }

    std::shared_ptr<VariNode> root;

    void add_child(const std::shared_ptr<VariNode>& child) {
        root->add_child(child);
    }

};
