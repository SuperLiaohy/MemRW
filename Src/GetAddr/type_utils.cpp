//
// Created by liaohy on 8/6/25.
//

#include "type_utils.h"
#include "attr_utils.h"
#include "tag_utils.h"

#include <iostream>
#include <memory>

#include "dw_utils.h"
#include "VariTree.h"



uint32_t get_array_count(Dwarf_Debug dbg, Dwarf_Die die) {
    Dwarf_Unsigned count = 0;
    one_step_recursion_die_do(dbg, die, [&count](Dwarf_Debug dbg, Dwarf_Die die) {
        int res = 0;
        Dwarf_Error error = nullptr;
        Dwarf_Half tag = 0;
        std::tie(res, tag) = get_die_tag(dbg, die);
        if (tag == DW_TAG_subrange_type) {
            Dwarf_Attribute attr = nullptr;
            res = dw_error_check(dwarf_attr(die,DW_AT_count,&attr,&error),dbg,error);
            if (res != DW_DLV_OK) {
                error = nullptr;
                res = dw_error_check(dwarf_attr(die, DW_AT_upper_bound, &attr, &error), dbg, error);
                if (res != DW_DLV_OK) {
                    res = dw_error_check(dwarf_attr(die, DW_AT_lower_bound, &attr, &error), dbg, error);
                }
                if (attr== nullptr) {return;}
                res = dw_error_check(dwarf_formudata(attr, &count, &error), dbg, error);
                if (res == DW_DLV_OK) {
                    ++count;
                    std::cout << "count:" << count << std::endl;
                }
                dwarf_dealloc_attribute(attr);
                return;
            }
            res = dw_error_check(dwarf_formudata(attr,&count,&error),dbg,error);
            dwarf_dealloc_attribute(attr);
            if (res != DW_DLV_OK) {return;}
        }
    });
    return count;
}

std::tuple<int, uint32_t> get_type_size(Dwarf_Debug dbg, Dwarf_Die die) {
    int res = 0;
    Dwarf_Unsigned byte_size = 0;
    Dwarf_Attribute attr = nullptr;
    Dwarf_Error error = nullptr;
    res = dw_error_check(dwarf_attr(die, DW_AT_byte_size,&attr,&error),dbg,error);
    if (res == DW_DLV_OK) {
        res = dw_error_check(dwarf_formudata(attr,&byte_size,&error),dbg,error);
    }
    if (attr != nullptr)
        dwarf_dealloc_attribute(attr);
    return std::tuple{res,byte_size};
}

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


int recursion_type_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)>& func) {
    Dwarf_Die type_die = nullptr;
    int res = 0;

    res = get_type_die(dbg, die, &type_die);
    if (res != DW_DLV_OK) {return res;}

    Dwarf_Half tag = 0;
    std::tie(res, tag) = get_die_tag(dbg, type_die);
    if (res != DW_DLV_OK) {dwarf_dealloc_die(type_die);return res;}
    std::cout << "type tag: " << trans_dw_tag(tag) << std::endl;
    switch (tag) {
        case DW_TAG_typedef:
            recursion_type_do(dbg, type_die, func);
            break;
        case DW_TAG_inheritance:    // maybe unused
        case DW_TAG_union_type:
        case DW_TAG_class_type:
        case DW_TAG_structure_type:
            root_recursion_die_do(dbg, type_die, func, [](Dwarf_Debug dbg, Dwarf_Die die) ->bool {
                int res = 0; Dwarf_Half tag = 0;
                std::tie(res, tag)=get_die_tag(dbg,die);
                if (res!=DW_DLV_OK) {return false;}
                display_single_die(dbg,die);
                if (tag==DW_TAG_member||tag==DW_TAG_inheritance) {return true;}
                // if (tag==DW_TAG_subprogram) {display_die_name(dbg,die);}
                return false;
            });
            break;
        case DW_TAG_array_type:
            func(dbg,type_die);
            break;
        case DW_TAG_pointer_type:
            break;
        case DW_TAG_base_type: {
            std::string type;
            std::tie(res, type) = get_die_name(dbg, type_die);
            if (res == DW_DLV_OK) {
                std::cout << "type: " << type << std::endl;
            } else
                std::cout << "type: error base type" << std::endl;
        }break;
        default:
            std::cout << "unknown type die tag: " << trans_dw_tag(tag) << std::endl;
            break;
    }
    dwarf_dealloc_die(type_die);
    return res;


}

std::tuple<std::string, uint32_t> recursion_type_judge(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)>& func) {
    int res = 0;

    std::string self_type_name;
    uint32_t size = 0;
    std::tie(res,self_type_name, size) = get_type_die_name_with_size(dbg, die);
    if (res!=DW_DLV_OK) {return {};}

    Dwarf_Half tag = 0;
    std::tie(res, tag) = get_die_tag(dbg, die);
    std::cout << "type tag: " << trans_dw_tag(tag) << std::endl;

    switch (tag) {
        case DW_TAG_typedef:
            recursion_type_do(dbg, die, func);
            break;
        case DW_TAG_inheritance:    // maybe unused
        case DW_TAG_class_type:
        case DW_TAG_structure_type:
        case DW_TAG_union_type:
            root_recursion_die_do(dbg, die, func, [](Dwarf_Debug dbg, Dwarf_Die die) ->bool {
                int res = 0; Dwarf_Half tag = 0;
                std::tie(res, tag)=get_die_tag(dbg,die);
                if (res!=DW_DLV_OK) {return false;}
                display_die_tag(dbg,die);
                if (tag==DW_TAG_member||tag==DW_TAG_inheritance) {return true;}
                // if (tag==DW_TAG_subprogram) {display_die_name(dbg,die);}
                return false;
            });
            break;
        case DW_TAG_array_type:
            func(dbg,die);
            break;
        case DW_TAG_base_type: {
            std::string type;
            std::tie(res, type) = get_die_name(dbg, die);
            if (res == DW_DLV_OK) {
                std::cout << "type: " << type << std::endl;
                std::tie(res, size) = get_type_size(dbg, die);
                std::cout << "type size: " << size << std::endl;
            } else
                std::cout << "type: error base type" << std::endl;
        }
            break;
        default:
            std::cout << "unknown type die tag: " << trans_dw_tag(tag) << std::endl;
            break;
    }
    return {self_type_name,size};
}