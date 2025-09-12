//
// Created by liaohy on 8/4/25.
//

#include "dw_utils.h"
#include "type_utils.h"
#include "attr_utils.h"
#include "tag_utils.h"
#include <iostream>

int display_single_die(Dwarf_Debug dbg, Dwarf_Die die) {
    int res=0;
    Dwarf_Error error = nullptr;
    Dwarf_Attribute* attr_list = nullptr;
    Dwarf_Signed attr_count = 0;
    Dwarf_Half form = 0;
    Dwarf_Half symbol = 0;
    Dwarf_Half tag = 0;
    char *dw_str = nullptr;

    std::cout << "------" << std::endl;

    std::tie(res, tag) = get_die_tag(dbg, die);
    if (res != DW_DLV_OK) {
        std::cout << "error: error in tag!" << std::endl;
        return res;
    }
    display_tag(tag);

    res = dw_error_check(dwarf_attrlist(die, &attr_list,&attr_count, &error), dbg, error);
    if (res == DW_DLV_OK) {
        std::cout << "attr_count: " << attr_count << std::endl;
        for (int i = 0; i < attr_count; ++i) {
            res = dw_error_check(dwarf_whatform(attr_list[i], &form, &error),dbg, error);
            if(res==DW_DLV_OK)
                std::cout << "form: " << trans_dw_form(form);

            res = dw_error_check(dwarf_whatattr(attr_list[i], &symbol, &error),dbg, error);
            if (res==DW_DLV_OK) {
                std::cout << " \tsymbol:" << trans_dw_attr_num(symbol);
                Dwarf_Unsigned num=0;
                switch (form) {
                    case DW_FORM_data1:
                        res = dw_error_check(dwarf_formudata(attr_list[i],&num,&error),dbg,error);
                        if (res==DW_DLV_OK)
                            std::cout << "\tnum: " << num << std::endl;
                        else
                            std::cout << "\tnum: no num!" << std::endl;
                        break;
                    default:
                        res = dw_error_check(dwarf_formstring(attr_list[i], &dw_str, &error),dbg,error);
                        if (res==DW_DLV_OK)
                            std::cout << "\ttext: " << dw_str << std::endl;
                        else
                            std::cout << "\ttext: no text!" << std::endl;
                        break;
                }


            }
            dwarf_dealloc_attribute(attr_list[i]);
        }
    }
    std::cout << "------" << std::endl;

    return res;
}

int recursion_die_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)>& func) {
    int res=0;
    Dwarf_Error error=nullptr;
    Dwarf_Die cur_die = die, sib_die = nullptr, child_die = nullptr;
    func(dbg, cur_die);
    while (true) {
        res = dw_error_check(dwarf_child(cur_die, &child_die, &error), dbg, error);
        if (res == DW_DLV_OK) {
            recursion_die_do(dbg, child_die, func);
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
        func(dbg, sib_die);
    }
    return res;
}

int one_step_recursion_die_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)>& func) {
    int res=0;
    Dwarf_Error error=nullptr;
    Dwarf_Die cur_die = die, sib_die = nullptr, child_die = nullptr;
    res = dw_error_check(dwarf_child(cur_die, &child_die, &error), dbg, error);
    if (res != DW_DLV_OK) {return res;}
    cur_die = child_die;
    func(dbg,cur_die);
    while (true) {

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
        func(dbg, sib_die);
    }
    return res;
}

int root_recursion_die_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)> &func) {
    int res = 0;
    Dwarf_Die child = nullptr;
    Dwarf_Error error = nullptr;
    res = dw_error_check(dwarf_child(die,&child, &error),dbg ,error);
    if (res != DW_DLV_OK) {
        std::cout << "no child" << std::endl;
        return res;
    }
    res = recursion_die_do(dbg, child, func);
    dwarf_dealloc_die(child);
    return res;
}

int recursion_die_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)> &func,
    const std::function<bool(Dwarf_Debug, Dwarf_Die)> &valid) {
    int res=0;
    Dwarf_Error error=nullptr;
    Dwarf_Die cur_die = die, sib_die = nullptr, child_die = nullptr;
    bool is_valid = valid(dbg, cur_die);
    if (is_valid) {
        func(dbg, cur_die);
    }
    while (true) {
        if (is_valid) {
            res = dw_error_check(dwarf_child(cur_die, &child_die, &error), dbg, error);
            if (res == DW_DLV_OK) {
                recursion_die_do(dbg, child_die, func);
                dwarf_dealloc_die(child_die);
            }
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

        is_valid = valid(dbg, cur_die);
        if (is_valid) {
            func(dbg, cur_die);
        }
    }
    return res;
}

int root_recursion_die_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)> &func,
                          const std::function<bool(Dwarf_Debug, Dwarf_Die)> &valid) {
    int res = 0;
    Dwarf_Die child = nullptr;
    Dwarf_Error error = nullptr;
    res = dw_error_check(dwarf_child(die,&child, &error),dbg ,error);
    if (res != DW_DLV_OK) {
        std::cout << "no child" << std::endl;
        return res;
    }
    res = recursion_die_do(dbg, child, func, valid);
    dwarf_dealloc_die(child);
    return res;
}


