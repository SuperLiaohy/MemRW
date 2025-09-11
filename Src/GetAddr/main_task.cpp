#include <iostream>
#include <cstring>
#include <chrono>


#include "attr_utils.h"
#include "tag_utils.h"
#include "type_utils.h"
#include "dw_utils.h"
#include "VariTree.h"



void recursion_replenish(std::unordered_map<std::string, std::shared_ptr<VariNode>>& map, std::shared_ptr<VariNode>& parent_node) {
    parent_node->traversal_end_node([&map](std::shared_ptr<VariNode>&node) {
        if (map.count(node->type)) {
            recursion_replenish(map,map[node->type]);
            node->add_type_tree(map[node->type]);
        }
    });
};

std::shared_ptr<VariTree> get_addr_task(const std::string& file, DWARF_MODE mode) {
    VariTree dwarf{};
    Dwarf_Debug dbg = nullptr;
    const char *path = file.data();
    char *true_path = nullptr;
    uint32_t true_pathlen = 0;
    Dwarf_Handler errhand = nullptr;
    Dwarf_Ptr errarg = nullptr;
    Dwarf_Error error = nullptr;

    // 加载dwarf文件
    int res = dwarf_init_path(path, true_path, true_pathlen,
                              DW_GROUPNUMBER_ANY,
                              errhand, errarg,
                              &dbg,
                              &error);
    if (res != DW_DLV_OK) {
        std::cout << "can not load dwarf file" << std::endl;
        return 0;
    }
    // 读取dwarf文件中的编译单元
    Dwarf_Bool is_info = 1;
    Dwarf_Die cu_die = nullptr;
    Dwarf_Unsigned cu_header_len = 0;
    // dwarf文件的版本
    Dwarf_Half dw_version = 0;
    Dwarf_Off abbrev_off = 0;
    Dwarf_Half address_size = 0;
    Dwarf_Half len_size = 0;
    Dwarf_Half extension_size = 0;
    Dwarf_Sig8 type_signature;
    memset(&type_signature, 0, sizeof(type_signature));
    Dwarf_Unsigned type_off = 0;
    Dwarf_Unsigned next_cu_header_off = 0;
    Dwarf_Half cu_type = 0;

    std::string cu_name;
    std::string last_cu_name;
    std::shared_ptr<VariNode> cu_node;


    auto start_time = std::chrono::high_resolution_clock::now();

    if (mode == DWARF_MODE::COMPLEX) {
        std::unordered_map<std::string, std::shared_ptr<VariNode>> type_map;

        type_map.reserve(500);
        while (true) {
            res = dwarf_next_cu_header_e(dbg, is_info,
                                         &cu_die,
                                         &cu_header_len,
                                         &dw_version, &abbrev_off,
                                         &address_size, &len_size,
                                         &extension_size, &type_signature,
                                         &type_off, &next_cu_header_off,
                                         &cu_type, &error);

            if (res == DW_DLV_OK) {
                std::cout << "--------------------------------------" << std::endl;
                std::cout << "dwarf version:" << dw_version << std::endl;
                std::tie(res, cu_name) = get_die_name(dbg, cu_die);

                if (res == DW_DLV_OK) {
                    recursion_die_do(dbg, cu_die, [&type_map](Dwarf_Debug dbg, Dwarf_Die die) {
                        int res = 0;
                        Dwarf_Half tag = 0;
                        std::tie(res, tag) = get_die_tag(dbg, die);
                        if (res != DW_DLV_OK) { return; }
                        std::shared_ptr<VariNode> type_node = std::make_shared<VariNode>();
                        auto generate_func = [](std::shared_ptr<VariNode> node, auto &&self) -> std::function<void(
                                Dwarf_Debug dbg, Dwarf_Die die)> {
                            return [node, self](Dwarf_Debug dbg, Dwarf_Die die) {
                                std::string die_name;
                                std::string type;
                                uint32_t size = 0;
                                int res = 0;
                                Dwarf_Half tag = 0;
                                std::tie(res, tag) = get_die_tag(dbg, die);
                                if (res != DW_DLV_OK) { return; }
                                switch (tag) {
                                    case DW_TAG_member: {
                                        std::tie(res, die_name) = get_die_name(dbg, die);
                                        if (res == DW_DLV_OK) { std::cout << "member name: " << die_name << std::endl; }

                                        Dwarf_Attribute attr = nullptr;
                                        Dwarf_Error error = nullptr;
                                        Dwarf_Unsigned location = 0;
                                        res = dw_error_check(dwarf_attr(die, DW_AT_data_member_location, &attr, &error),
                                                             dbg, error);

                                        if (res == DW_DLV_OK) {
                                            res = dw_error_check(dwarf_formudata(attr, &location, &error), dbg, error);
                                            dwarf_dealloc_attribute(attr);
                                            if (res == DW_DLV_OK)
                                                std::cout << "member location: " << location << std::endl;
                                        }
                                        std::tie(res, type, size) = get_die_type_with_size(dbg, die);
                                        if (type == " *") {
                                            std::cout << " g " << std::endl;
                                        }
                                        std::tie(res, type, size) = get_die_type_with_size(dbg, die);
                                        if (res != DW_DLV_OK)
                                            std::cout << "error: no direct type!\t";
                                        else
                                            std::cout << "direct type: " << type << std::endl;

                                        auto child = std::make_shared<VariNode>(std::string(die_name),
                                                                                std::string(type), location, size);
                                        node->add_child(child);
                                        recursion_type_do(dbg, die, self(child, self));
                                    }
                                        break;
                                    case DW_TAG_inheritance: {
                                        recursion_type_do(dbg, die, self(node, self));
                                    }
                                        break;
                                    case DW_TAG_array_type: {
                                        uint32_t count = get_array_count(dbg, die);
                                        uint32_t type_size = node->size;
                                        node->size = node->size*count;
                                        Dwarf_Die type_die = nullptr;
                                        if (get_type_die(dbg, die, &type_die) != DW_DLV_OK) { return; }
                                        node->add_child(std::make_shared<VariNode>());
                                        std::tie(node->children[0]->type,
                                                 node->children[0]->size) = recursion_type_judge(
                                                dbg, type_die, self(node, self));
                                        dwarf_dealloc_die(type_die);
                                        if (node->children[0]->type.empty()) { return; }
                                        node->children[0]->name = "[0]";
                                        for (int index = 1; index < count; ++index) {
                                            node->children.push_back(
                                                    std::make_shared<VariNode>(*node->children[index - 1]));
                                            node->children[index]->name = "[" + std::to_string(index) + "]";
                                            node->children[index]->addr = type_size*index;
                                        }
                                    }
                                        break;
                                    default:
                                        display_die_tag(dbg, die);
                                }
                            };
                        };
                        std::tie(type_node->type, type_node->size) = recursion_type_judge(dbg, die,
                                                                                          generate_func(type_node,
                                                                                                        generate_func));
                        if (!type_node->type.empty() && !type_node->children.empty() &&
                            type_node->type.find("<anonymous ") == std::string::npos) {
                            type_map.emplace(type_node->type, type_node);
                        }
                    });
                } else {
                    std::cout << "--------------------------------------" << std::endl;
                    recursion_die_do(dbg, cu_die, display_single_die);
                }
            } else if (res == DW_DLV_ERROR) {
                std::cout << "read cu die failed" << std::endl;
                break;
            } else if (res == DW_DLV_NO_ENTRY) {
                std::cout << "read cu die over" << std::endl;
                break;
            }
            dwarf_dealloc_die(cu_die);
        }



//    auto recursion_replenish = [&type_map](std::shared_ptr<VariNode>& parent_node, auto&&self) {
//
//    };

        auto index = 0;
        for (auto &item: type_map) {
            recursion_replenish(type_map, item.second);
            std::cout << "type name " << ++index << ": " << item.first << std::endl;
        }

        dwarf_finish(dbg);
        res = dwarf_init_path(path, true_path, true_pathlen,
                              DW_GROUPNUMBER_ANY,
                              errhand, errarg,
                              &dbg,
                              &error);

        if (res != DW_DLV_OK) { return {}; }

        while (true) {
            res = dwarf_next_cu_header_e(dbg, is_info,
                                         &cu_die,
                                         &cu_header_len,
                                         &dw_version, &abbrev_off,
                                         &address_size, &len_size,
                                         &extension_size, &type_signature,
                                         &type_off, &next_cu_header_off,
                                         &cu_type, &error);

            if (res == DW_DLV_OK) {
                std::cout << "--------------------------------------" << std::endl;
                std::cout << "dwarf version:" << dw_version << std::endl;
                std::tie(res, cu_name) = get_die_name(dbg, cu_die);

                if (res == DW_DLV_OK) {
                    if (last_cu_name != cu_name || last_cu_name.empty()) {
                        cu_node = std::make_shared<VariNode>(std::string(cu_name), std::string("compile unit"), 0, 0);
                        dwarf.add_child(cu_node);
                        std::cout << "cu name: " << cu_node->name << std::endl;
                        std::cout << "--------------------------------------" << std::endl;
                    }
                    recursion_die_do(dbg, cu_die, [&cu_node, &type_map](Dwarf_Debug dbg, Dwarf_Die die) {
                        int res = 0;
                        Dwarf_Half tag = 0;
                        std::string name;
                        std::string type;
                        uint32_t size = 0;
                        uint64_t addr = 0;
                        Dwarf_Half opcode = 0;
                        std::tie(res, tag) = get_die_tag(dbg, die);
                        if (tag != DW_TAG_variable || res != DW_DLV_OK) { return; }

                        std::tie(res, name) = get_die_name(dbg, die);
                        if (res != DW_DLV_OK) { return; }

                        std::tie(res, addr, opcode) = get_die_location(dbg, die);
                        if (res != DW_DLV_OK || addr == 0) { return; }
                        std::cout << "--------------------" << std::endl;
                        std::cout << "var name: " << name << std::endl;
                        std::cout << "opcode: " << opcode << "\taddr: " << std::hex << addr << std::dec << std::endl;

                        std::tie(res, type, size) = get_die_type_with_size(dbg, die);
                        if (res != DW_DLV_OK) { return; }
                        std::cout << "direct type: " << type << std::endl;

                        auto child_node = std::make_shared<VariNode>(std::string(name), std::string(type), addr, size);
                        cu_node->add_child(child_node);

                        auto generate_func = [&type_map](std::shared_ptr<VariNode> node,
                                                         auto &&self) -> std::function<void(Dwarf_Debug dbg,
                                                                                            Dwarf_Die die)> {
                            return [node, self, &type_map](Dwarf_Debug dbg, Dwarf_Die die) {
                                std::string die_name;
                                int res = 0;
                                std::string type;
                                Dwarf_Half tag = 0;
                                uint32_t size = 0;
                                std::tie(res, tag) = get_die_tag(dbg, die);
                                if (res != DW_DLV_OK) { return; }
                                switch (tag) {
                                    case DW_TAG_member: {
                                        std::tie(res, die_name) = get_die_name(dbg, die);
                                        if (res == DW_DLV_OK) { std::cout << "member name: " << die_name << std::endl; }

                                        Dwarf_Attribute attr = nullptr;
                                        Dwarf_Error error = nullptr;
                                        Dwarf_Unsigned location = 0;
                                        res = dw_error_check(dwarf_attr(die, DW_AT_data_member_location, &attr, &error),
                                                             dbg, error);
                                        if (res == DW_DLV_OK) {
                                            res = dw_error_check(dwarf_formudata(attr, &location, &error), dbg, error);
                                            dwarf_dealloc_attribute(attr);
                                            if (res == DW_DLV_OK)
                                                std::cout << "member location: " << location << std::endl;
                                        }

                                        std::tie(res, type, size) = get_die_type_with_size(dbg, die);
                                        if (res != DW_DLV_OK) {
                                            std::cout << "error: no direct type!\t";
                                        } else
                                            std::cout << "direct type: " << type << std::endl;

                                        auto child = std::make_shared<VariNode>(std::string(die_name),
                                                                                std::string(type), location, size);
                                        node->add_child(child);

                                        if (child->children.empty() && type_map.count(child->type)) {
                                            child->add_type_tree(type_map[child->type]);
                                        }

                                        recursion_type_do(dbg, die, self(child, self));
                                    }
                                        break;
                                    case DW_TAG_inheritance:
                                        recursion_type_do(dbg, die, self(node, self));
                                        break;
                                    case DW_TAG_array_type: {
                                        uint32_t count = get_array_count(dbg, die);
                                        uint32_t type_size = node->size;
                                        node->size = node->size*count;
                                        Dwarf_Die type_die = nullptr;
                                        if (get_type_die(dbg, die, &type_die) != DW_DLV_OK) { return; }
                                        node->add_child(std::make_shared<VariNode>());
                                        std::tie(node->children[0]->type,
                                                 node->children[0]->size) = recursion_type_judge(
                                                dbg, type_die, self(node, self));
                                        dwarf_dealloc_die(type_die);
                                        if (node->children[0]->type.empty()) { return; }
                                        node->children[0]->name = "[0]";
                                        for (int index = 1; index < count; ++index) {
                                            node->children.push_back(
                                                    std::make_shared<VariNode>(*node->children[index - 1]));
                                            node->children[index]->name = "[" + std::to_string(index) + "]";
                                            node->children[index]->addr = type_size*index;
                                        }
                                    }
                                        break;
                                    default:
                                        display_die_tag(dbg, die);
                                }
                            };
                        };

                        recursion_type_do(dbg, die, generate_func(child_node, generate_func));

                        if (child_node->children.empty() && type_map.count(child_node->type)) {
                            child_node->add_type_tree(type_map[child_node->type]);
                        }

                    });

                    if (cu_node->children.empty()) {
                        dwarf.root->children.pop_back();
                    } else {
                        last_cu_name = cu_name;
                    }
                } else {
                    std::cout << "--------------------------------------" << std::endl;
                    recursion_die_do(dbg, cu_die, display_single_die);
                }

            } else if (res == DW_DLV_ERROR) {
                std::cout << "read cu die failed" << std::endl;
                break;
            } else if (res == DW_DLV_NO_ENTRY) {
                std::cout << "read cu die over" << std::endl;
                break;
            }
            dwarf_dealloc_die(cu_die);
        }
    } else if (mode == DWARF_MODE::SIMPLE) {
        while (true) {
            res = dwarf_next_cu_header_e(dbg, is_info,
                                         &cu_die,
                                         &cu_header_len,
                                         &dw_version, &abbrev_off,
                                         &address_size, &len_size,
                                         &extension_size, &type_signature,
                                         &type_off, &next_cu_header_off,
                                         &cu_type, &error);

            if (res == DW_DLV_OK) {
                std::cout << "--------------------------------------" << std::endl;
                std::cout << "dwarf version:" << dw_version << std::endl;
                std::tie(res, cu_name) = get_die_name(dbg, cu_die);

                if (res == DW_DLV_OK) {
                    if (last_cu_name != cu_name || last_cu_name.empty()) {
                        cu_node = std::make_shared<VariNode>(std::string(cu_name), std::string("compile unit"), 0, 0);
                        dwarf.add_child(cu_node);
                        std::cout << "cu name: " << cu_node->name << std::endl;
                        std::cout << "--------------------------------------" << std::endl;
                    }
                    recursion_die_do(dbg, cu_die, [&cu_node](Dwarf_Debug dbg, Dwarf_Die die) {
                        int res = 0;
                        Dwarf_Half tag = 0;
                        std::string name;
                        std::string type;
                        uint32_t size = 0;
                        uint64_t addr = 0;
                        Dwarf_Half opcode = 0;
                        std::tie(res, tag) = get_die_tag(dbg, die);
                        if (tag != DW_TAG_variable || res != DW_DLV_OK) { return; }

                        std::tie(res, name) = get_die_name(dbg, die);
                        if (res != DW_DLV_OK) { return; }

                        std::tie(res, addr, opcode) = get_die_location(dbg, die);
                        if (res != DW_DLV_OK || addr == 0) { return; }
                        std::cout << "--------------------" << std::endl;
                        std::cout << "var name: " << name << std::endl;
                        std::cout << "opcode: " << opcode << "\taddr: " << std::hex << addr << std::dec << std::endl;

                        std::tie(res, type, size) = get_die_type_with_size(dbg, die);
                        if (res != DW_DLV_OK) { return; }
                        std::cout << "direct type: " << type << std::endl;

                        auto child_node = std::make_shared<VariNode>(std::string(name), std::string(type), addr, size);
                        cu_node->add_child(child_node);

                        auto generate_func = [](std::shared_ptr<VariNode> node,auto &&self) -> std::function<void(Dwarf_Debug dbg,Dwarf_Die die)> {
                            return [node, self](Dwarf_Debug dbg, Dwarf_Die die) {
                                std::string die_name;
                                int res = 0;
                                std::string type;
                                Dwarf_Half tag = 0;
                                uint32_t size = 0;
                                std::tie(res, tag) = get_die_tag(dbg, die);
                                if (res != DW_DLV_OK) { return; }
                                switch (tag) {
                                    case DW_TAG_member: {
                                        std::tie(res, die_name) = get_die_name(dbg, die);
                                        if (res == DW_DLV_OK) { std::cout << "member name: " << die_name << std::endl; }

                                        Dwarf_Attribute attr = nullptr;
                                        Dwarf_Error error = nullptr;
                                        Dwarf_Unsigned location = 0;
                                        res = dw_error_check(dwarf_attr(die, DW_AT_data_member_location, &attr, &error),
                                                             dbg, error);
                                        if (res == DW_DLV_OK) {
                                            res = dw_error_check(dwarf_formudata(attr, &location, &error), dbg, error);
                                            dwarf_dealloc_attribute(attr);
                                            if (res == DW_DLV_OK)
                                                std::cout << "member location: " << location << std::endl;
                                        }

                                        std::tie(res, type, size) = get_die_type_with_size(dbg, die);
                                        if (res != DW_DLV_OK) {
                                            std::cout << "error: no direct type!\t";
                                        } else
                                            std::cout << "direct type: " << type << std::endl;

                                        auto child = std::make_shared<VariNode>(std::string(die_name),
                                                                                std::string(type), location, size);
                                        node->add_child(child);

                                        recursion_type_do(dbg, die, self(child, self));
                                    }
                                        break;
                                    case DW_TAG_inheritance:
                                        recursion_type_do(dbg, die, self(node, self));
                                        break;
                                    case DW_TAG_array_type: {
                                        uint32_t count = get_array_count(dbg, die);
                                        uint32_t type_size = node->size;
                                        node->size = node->size*count;
                                        Dwarf_Die type_die = nullptr;
                                        if (get_type_die(dbg, die, &type_die) != DW_DLV_OK) { return; }
                                        node->add_child(std::make_shared<VariNode>());
                                        std::tie(node->children[0]->type,
                                                 node->children[0]->size) = recursion_type_judge(
                                                dbg, type_die, self(node, self));
                                        dwarf_dealloc_die(type_die);
                                        if (node->children[0]->type.empty()) { return; }
                                        node->children[0]->name = "[0]";
                                        for (int index = 1; index < count; ++index) {
                                            node->children.push_back(
                                                    std::make_shared<VariNode>(*node->children[index - 1]));
                                            node->children[index]->name = "[" + std::to_string(index) + "]";
                                            node->children[index]->addr = type_size*index;
                                        }
                                    }
                                        break;
                                    default:
                                        display_die_tag(dbg, die);
                                }
                            };
                        };

                        recursion_type_do(dbg, die, generate_func(child_node, generate_func));

                    });

                    if (cu_node->children.empty()) {
                        dwarf.root->children.pop_back();
                    } else {
                        last_cu_name = cu_name;
                    }
                } else {
                    std::cout << "--------------------------------------" << std::endl;
                    recursion_die_do(dbg, cu_die, display_single_die);
                }

            } else if (res == DW_DLV_ERROR) {
                std::cout << "read cu die failed" << std::endl;
                break;
            } else if (res == DW_DLV_NO_ENTRY) {
                std::cout << "read cu die over" << std::endl;
                break;
            }
            dwarf_dealloc_die(cu_die);
        }
    }
    dwarf_finish(dbg);

    auto end_time = std::chrono::high_resolution_clock::now();
    std::cout << "time spend: " << duration_cast<std::chrono::microseconds>(end_time - start_time) << std::endl;

    std::cout << "Hello, World!" << std::endl;
    return std::make_shared<VariTree>(dwarf);
}
