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

    void add_children(uint32_t number, uint32_t size, std::string name, std::string type ,const uint64_t addr) {
        auto p = std::make_shared<VariNode>(std::move(name),std::move(type), addr);
        node.push_back(p);
        for (int i = 0; i < number; ++i) {
            p->add_child_with_offset(std::move(name),std::move(type), size*number);
        }
    }

    void add_children(uint32_t number, const std::shared_ptr<VariNode>& child) {
        node.push_back(child);
    }

    void add_child_with_offset(std::string name, std::string type , uint64_t addr) {
        addr += this->addr;
        node.push_back(std::make_shared<VariNode>(std::move(name),std::move(type), addr));
    }

    void add_child_with_offset(const std::shared_ptr<VariNode>& child) {
        child->addr += this->addr;
        node.push_back(child);
    }

    void add_children_with_offset(uint32_t number, std::string name, std::string type , uint64_t addr) {
        addr += this->addr;
        node.push_back(std::make_shared<VariNode>(std::move(name),std::move(type), addr));
    }

    void add_children_with_offset(uint32_t number, const std::shared_ptr<VariNode>& child) {
        child->addr += this->addr;
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

    void add_child_with_offset(const std::shared_ptr<VariNode>& child) {
        child->addr += this->root->addr;
        root->add_child(child);
    }

};
