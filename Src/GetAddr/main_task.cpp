#include <iostream>
#include <cstring>

#include "attr_utils.h"
#include "tag_utils.h"
#include "type_utils.h"
#include "dw_utils.h"
#include "VariTree.h"

void dw_print_err(Dwarf_Error error, Dwarf_Ptr ptr) {
    printf("ERROR Exit on %lx due to error 0x%lx %s\n",
           reinterpret_cast<uint64_t>(ptr),
           static_cast<uint64_t>(dwarf_errno(error)),
           dwarf_errmsg(error));
    exit(1);
}

void display_dw_error(Dwarf_Error error);



int get_addr_task() {
    VariTree dwarf{std::make_shared<VariNode>("root","root",0)};
    Dwarf_Debug dbg = 0;
    //    const char *path = "./Air.axf";
    const char *path = "/home/liaohy/User/Code/QtCreator/MemRW/Res/armclang_engineer.axf";
    char *true_path = 0;
    uint32_t true_pathlen = 0;
    Dwarf_Handler errhand = 0;
    Dwarf_Ptr errarg = 0;
    Dwarf_Error error = 0;

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
    Dwarf_Die cu_die = 0;
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
                auto cu_node = std::make_shared<VariNode>(std::string(cu_name),std::string("cu"), 0);
                dwarf.add_child(cu_node);
                std::cout << "cu name: " << cu_node->name << std::endl;
                std::cout << "--------------------------------------" << std::endl;
                if (res == DW_DLV_OK) {
                    recursion_die_do(dbg, cu_die, [&cu_node](Dwarf_Debug dbg, Dwarf_Die die) {
                        int res=0;
                        Dwarf_Half tag = 0;
                        std::string name;
                        std::string type;
                        uint64_t addr = 0;
                        Dwarf_Half opcode = 0;
                        std::tie(res, tag) = get_die_tag(dbg, die);
                        if (tag!=DW_TAG_variable || res != DW_DLV_OK ) {return;}
                        std::tie(res, name) = get_die_name(dbg, die);
                        if (res != DW_DLV_OK) {return;}
                        std::tie(res, addr, opcode) = get_die_location(dbg, die);
                        if (res != DW_DLV_OK || addr == 0) {return;}
                        std::cout << "--------------------" << std::endl;
                        std::cout << "var name: " << name << std::endl;
                        std::cout << "opcode: " << opcode << "\taddr: " << std::hex <<  addr << std::dec << std::endl;
                        std::tie(res, type) = get_die_type(dbg, die);
                        if (res != DW_DLV_OK) {return;}
                        std::cout << "direct type: " << type << std::endl;
                        auto child_node = std::make_shared<VariNode>(std::string(name), std::string(type), addr);
                        cu_node->add_child(child_node);

                        // if (type.find('[')==std::string::npos)
                        //     cu_node->add_child(child_node);
                        // else {
                        //     cu_node->add_child(child_node);
                        // }

                        auto generate_func = [](std::shared_ptr<VariNode> node, auto&& self) -> std::function<void(Dwarf_Debug dbg, Dwarf_Die die)> {
                            return [node,self](Dwarf_Debug dbg, Dwarf_Die die) {
                                std::string die_name;
                                int res = 0; std::string type;
                                Dwarf_Half tag = 0;
                                std::tie(res, tag) = get_die_tag(dbg, die);
                                if (res!=DW_DLV_OK) {return;}
                                switch (tag) {
                                    case DW_TAG_member: {
                                        std::tie(res, die_name) = get_die_name(dbg, die);
                                        if (res == DW_DLV_OK) {std::cout << "member name: " << die_name << std::endl;}

                                        Dwarf_Attribute attr = nullptr; Dwarf_Error error = nullptr;
                                        Dwarf_Unsigned location = 0;
                                        res = dw_error_check(dwarf_attr(die, DW_AT_data_member_location, &attr, &error), dbg, error);
                                        if (res == DW_DLV_OK) {
                                            res = dw_error_check(dwarf_formudata(attr, &location, &error), dbg, error);
                                            dwarf_dealloc_attribute(attr);
                                            if (res == DW_DLV_OK)
                                                std::cout << "member location: " << location << std::endl;
                                        }

                                        std::tie(res, type) = get_die_type(dbg, die);
                                        if (res != DW_DLV_OK) {
                                            std::cout << "error: no direct type!\t";
                                        } else
                                            std::cout << "direct type: " << type << std::endl;

                                        auto child = std::make_shared<VariNode>(std::string(die_name),std::string(type),location);
                                        node->add_child_with_offset(child);
                                        recursion_type_do(dbg,die,self(child,self));
                                    }   break;
                                    case DW_TAG_inheritance:{
                                        recursion_type_do(dbg,die,self(node,self));
                                    }   break;
                                    default:
                                        display_die_tag(dbg,die);
                                }
                            };
                        };

                        recursion_type_do(dbg, die, generate_func(child_node, generate_func));

                    });
                }
            } else {
                std::cout << "--------------------------------------" << std::endl;
                recursion_die_do(dbg, cu_die,display_single_vari_die);
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
    dwarf_finish(dbg);

    std::cout << "Hello, World!" << std::endl;
    return 0;
}
