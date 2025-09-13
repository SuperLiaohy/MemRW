//
// Created by liaohy on 8/7/25.
//

#pragma once

#include <algorithm>
#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <variant>
#include <format>
enum class DWARF_MODE {
    SIMPLE,
    COMPLEX
};


class VariNode {
public:
    VariNode() : addr(0), size(0), father(nullptr) {};
    VariNode(std::string name, std::string type ,const uint64_t addr, uint32_t size, VariNode* parent = nullptr) : name(std::move(name)), type(std::move(type)), size(size), addr(addr), children(), father(parent) {};

    std::string name;
    std::string type;
    uint32_t size;
    uint64_t addr;
    VariNode* father;
    std::vector<std::shared_ptr<VariNode>> children;

    std::string data(int column) const {
        switch (column) {
            case 0:{
                auto node = this;
                if (node->father == nullptr) return name;;
                if (node->father->father== nullptr) return name;
                int index = 0;
                std::string end_name = node->name;

                while (node->father->father->father!=nullptr)  {
                    node = node->father;
                    std::string result = node->name;
                    if (end_name[0]!='[') result += '.';
                    result += end_name;
                    end_name = result;
                }
                return end_name;
            }
            case 1:
                return type;
            case 2: {
                auto node = this;
                if (node->father == nullptr) return {};;
                if (node->father->father== nullptr) return {};;
                uint64_t address = addr;
                while (node->father->father->father != nullptr)  {
                    node = node->father;
                    address+=node->addr;
                }
                return std::format("0x{:x}", address);
            }
            case 3:
                if (this->father == nullptr) return {};;
                if (this->father->father== nullptr) return {};;
                return std::format("{}", size);
            default:
                return {};

        }
    }
    VariNode* child(int row) {
        if (row < 0 || row >= children.size()) { return nullptr; }
        return children[row].get();
    }
    int childCount() {
        return children.size();
    }
    VariNode *parent() {
        return father;
    }
    
    void add_child(std::string child_name, std::string child_type ,const uint64_t child_addr, uint32_t child_size) {
        children.push_back(std::make_shared<VariNode>(std::move(child_name), std::move(child_type), child_addr, child_size, this));
    }

    void add_child(const std::shared_ptr<VariNode>& child) {
        children.push_back(child);
        child->father = this;
    }

    void add_type_tree(const std::shared_ptr<VariNode>& type_tree) {
        this->size = type_tree->size;
        add_tree(this, type_tree);
    }

    template<typename Fun>
    void traversal_end_node(Fun fun){
        recursion_end_node(this,fun);
    }

private:
    static void add_tree(VariNode* parent, const std::shared_ptr<VariNode>& tree) {
        for (auto & index : tree->children) {
            auto node = std::make_shared<VariNode>(*index);
            parent->add_child(node);
            node->children.clear();
            VariNode::add_tree(node.get(),index);
        }
    }

    template<typename Fun>
    static void recursion_end_node(VariNode* parent, Fun fun) {
        for (auto & node : parent->children) {
            if (node->children.empty())
                fun(node);
            else
                recursion_end_node(node.get(),fun);
        }
    }


};

class VariTree {
public:

    explicit VariTree(const std::shared_ptr<VariNode>& root) : root(root) {}

    VariTree() {
        root = std::make_shared<VariNode>("root", "N/A", 0, 0, nullptr);
    }

    std::shared_ptr<VariNode> root;

    void add_child(const std::shared_ptr<VariNode>& child) {
        root->add_child(child);
    }

};

using TreeItem = VariNode;
