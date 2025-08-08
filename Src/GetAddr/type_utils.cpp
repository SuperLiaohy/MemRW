//
// Created by liaohy on 8/6/25.
//

#include "type_utils.h"
#include "attr_utils.h"
#include "tag_utils.h"

#include <iostream>

#include "dw_utils.h"


int get_type_die(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Die *type_die) {
    Dwarf_Attribute attr = nullptr;
    Dwarf_Error error = nullptr;
    int res = dw_error_check(dwarf_attr(die, DW_AT_type, &attr, &error),dbg, error);
    if (res != DW_DLV_OK) {
        return res;
    }

    Dwarf_Off off = 0;
    Dwarf_Bool is_in = 0;
    res = dw_error_check(dwarf_global_formref_b(attr, &off, &is_in, &error),dbg, error);
    dwarf_dealloc_attribute(attr);
    if (res != DW_DLV_OK) {
        return res;
    }

    res = dw_error_check(dwarf_offdie_b(dbg, off, is_in, type_die, &error), dbg, error);
    if (res != DW_DLV_OK) {
        return res;
    }
    return res;
}

int display_full_type(Dwarf_Debug dbg, Dwarf_Die die) {
    Dwarf_Die type_die = nullptr;
    Dwarf_Error error = nullptr;
    int res = 0;
    res = get_type_die(dbg, die, &type_die);
    if (res != DW_DLV_OK) {
        return res;
    }

    Dwarf_Half tag = 0;
    std::tie(res, tag) = get_die_tag(dbg, type_die);
    if (res != DW_DLV_OK) {
        dwarf_dealloc_die(type_die);
        return res;
    }
    std::cout << "type tag: " << trans_dw_tag(tag) << std::endl;

    switch (tag) {
        case DW_TAG_typedef:
            display_full_type(dbg, type_die);
            break;
        case DW_TAG_inheritance:    // maybe unused
        case DW_TAG_class_type:
        case DW_TAG_structure_type: {
            root_recursion_die_do(dbg, type_die, [](Dwarf_Debug dbg, Dwarf_Die die) {
                std::string die_name;
                int res = 0; std::string type;

                std::tie(res, die_name) = get_die_name(dbg, die);
                if (res == DW_DLV_OK) {
                    std::cout << "member name: " << die_name << std::endl;
                }

                Dwarf_Attribute attr = nullptr;Dwarf_Error error = nullptr;
                res = dw_error_check(dwarf_attr(die, DW_AT_data_member_location, &attr, &error), dbg, error);
                if (res != DW_DLV_OK) {return;}
                Dwarf_Unsigned location = 0;
                res = dw_error_check(dwarf_formudata(attr,&location,&error), dbg,error);
                dwarf_dealloc_attribute(attr);
                if (res != DW_DLV_OK) {return;}
                std::cout << "member location: " << location << std::endl;

                std::tie(res, type) = get_die_type(dbg, die);
                if (res != DW_DLV_OK) {
                    std::cout << "error: no direct type!\t";

                    display_die_tag(dbg,die);
                    return;
                }
                std::cout << "direct type: " << type << std::endl;
                display_full_type(dbg,die);
            });
        }
            break;
        case DW_TAG_union_type:
            break;
        case DW_TAG_array_type:
            break;
        case DW_TAG_pointer_type:
            break;
        case DW_TAG_base_type: {
            std::string type;
            std::tie(res, type) = get_die_name(dbg, type_die);
            if (res == DW_DLV_OK)
                std::cout << "type: " << type << std::endl;
            else
                std::cout << "type: error base type" << std::endl;
        }
            break;


        default:
            std::cout << "unknown type die tag: " << trans_dw_tag(tag) << std::endl;
            break;
    }
    dwarf_dealloc_die(type_die);
    return res;
}

int display_tag_name(Dwarf_Debug dbg, Dwarf_Die cur_die) {
    int res = 0;
    Dwarf_Half tag = 0;

    std::tie(res, tag) = get_die_tag(dbg, cur_die);
    if (res != DW_DLV_OK) {
        return res;
    }

    if (tag == DW_TAG_member) {
    std::cout << "--" << std::endl;
        std::cout << "member: ";
        res = display_die_name(dbg, cur_die);
        std::cout << "member: ";
        display_full_type(dbg, cur_die);
    std::cout << "--" << std::endl;

    } else if (tag == DW_TAG_inheritance) {
        std::cout << "--" << std::endl;
        std::cout << "inheritance: ";
        display_full_type(dbg, cur_die);
        std::cout << "--" << std::endl;

    } else if (tag!=DW_TAG_subprogram&&tag!=DW_TAG_formal_parameter){
        display_tag(tag);

    }


    return res;
}

void display_recursion_die_type(Dwarf_Debug dbg, Dwarf_Die die) {
    int res=0;
    Dwarf_Error error = nullptr;
    Dwarf_Half tag = 0;
    Dwarf_Die cur_die = die, sib_die, child_die;

    display_tag_name(dbg,cur_die);

    while (true) {
        res = dw_error_check(dwarf_child(cur_die, &child_die, &error), dbg, error);
        if (res == DW_DLV_OK) {
            display_recursion_die_type(dbg, child_die);
            dwarf_dealloc_die(child_die);
        }
        res = dw_error_check(dwarf_siblingof_c(cur_die, &sib_die, &error), dbg, error);
        if (cur_die != die) {
            dwarf_dealloc_die(cur_die);
        }
        if (res == DW_DLV_ERROR) {
            printf("occur error!\n");
            break;
        }
        if (res == DW_DLV_NO_ENTRY) {
            break;
        }
        cur_die = sib_die;

        display_tag_name(dbg,cur_die);

    }
}
