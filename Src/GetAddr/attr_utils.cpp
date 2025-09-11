//
// Created by liaohy on 8/3/25.
//

#include <cstdint>
#include <iostream>
#include "attr_utils.h"

#include "dw_utils.h"
#include "tag_utils.h"
#include "type_utils.h"


std::tuple<int, std::string> get_die_name(Dwarf_Debug dbg, Dwarf_Die die) {
    int res = 0;
    Dwarf_Error error = nullptr;
    char *str = "";
    res = dw_error_check(dwarf_die_text(die, DW_AT_name, &str, &error),dbg, error);
    if (res != DW_DLV_OK) {
        return std::tuple{res, std::string()};
    }
    return std::tuple{res,str};

}

std::tuple<int, uint64_t, Dwarf_Half> get_die_location(Dwarf_Debug dbg, Dwarf_Die die) {
    Dwarf_Attribute attr = nullptr;
    Dwarf_Error error = nullptr;
    int res = dw_error_check(dwarf_attr(die, DW_AT_location, &attr, &error),dbg, error);
    if (res != DW_DLV_OK) {
        return std::tuple{res, 0, 0};
    }

    // 首先获取属性的形式
    Dwarf_Half form;
    res = dw_error_check(dwarf_whatform(attr, &form, &error), dbg, error);
    if (res != DW_DLV_OK) {
        dwarf_dealloc_attribute(attr);
        return std::tuple{res, 0, 0};
    }

    Dwarf_Block* block;
    Dwarf_Ptr block_ptr = nullptr;
    Dwarf_Unsigned len = 0;

    // 根据属性形式选择不同的处理方法
    if (form == DW_FORM_exprloc) {
        // DWARF4+ 使用 exprloc 形式
        res = dw_error_check(dwarf_formexprloc(attr, &len, &block_ptr, &error), dbg, error);
        if (res != DW_DLV_OK) {
            dwarf_dealloc_attribute(attr);
            return std::tuple{res, 0, 0};
        }
    } else if (form == DW_FORM_block1 || form == DW_FORM_block2 ||
               form == DW_FORM_block4 || form == DW_FORM_block) {
        // DWARF3 使用块形式
        res = dw_error_check(dwarf_formblock(attr, &block, &error), dbg, error);
        if (res != DW_DLV_OK) {
            dwarf_dealloc_attribute(attr);
            return std::tuple{res, 0, 0};
        }
        block_ptr = block->bl_data;
        len = block->bl_len;
    } else if (form == DW_FORM_data4 || form == DW_FORM_data8) {
        // 这是一个位置列表引用，需要更复杂的处理
        // 这里简化处理，实际情况可能需要处理位置列表
        Dwarf_Unsigned offset;
        res = dw_error_check(dwarf_formudata(attr, &offset, &error), dbg, error);
        if (res != DW_DLV_OK) {
            dwarf_dealloc_attribute(attr);
            return std::tuple{res, 0, 0};
        }

        // 这里需要使用 dwarf_get_loclist_entry 等函数获取位置列表
        // 由于比较复杂，这部分代码未完全展示
        dwarf_dealloc_attribute(attr);
        return std::tuple{DW_DLV_ERROR, 0, 0}; // 临时返回错误
    } else {
        // 不支持的形式
        dwarf_dealloc_attribute(attr);
        return std::tuple{DW_DLV_ERROR, 0, 0};
    }

    // 解析位置表达式
    uint64_t addr = 0;
    Dwarf_Half opcode = 0;

    if (len > 0) {
        opcode = reinterpret_cast<uint8_t *>(block_ptr)[0];
        if (opcode == DW_OP_addr) { // DW_OP_addr 通常是 0x03
            // 地址大小可能是 4 或 8 字节，取决于目标架构
            if (len >= 5) { // 确保有足够的数据
                // 对于 32 位地址
                addr = *reinterpret_cast<uint32_t *>(reinterpret_cast<uint8_t *>(block_ptr) + 1);
            }
            // 如果是 64 位地址，需要使用 uint64_t 来读取
            // if (len >= 9) {
            //     addr = *reinterpret_cast<uint64_t *>(reinterpret_cast<uint8_t *>(block_ptr) + 1);
            // }
        }
    }

    dwarf_dealloc_attribute(attr);
    return std::tuple{DW_DLV_OK, addr, opcode};
}

std::tuple<int, std::string, uint32_t> get_die_type_with_size(Dwarf_Debug dbg, Dwarf_Die die) {

    int res = 0;
    Dwarf_Die type_die = nullptr;
    res = get_type_die(dbg, die, &type_die);
    if (res == DW_DLV_ERROR) {
        return std::tuple{res, std::string(), 0};
    } else if (res == DW_DLV_NO_ENTRY) {
        if (std::tuple{DW_DLV_OK,DW_TAG_pointer_type}==get_die_tag(dbg,die)) {
            return std::tuple{DW_DLV_OK, std::string("void"), 4};
        }
    }

    std::string name;
    uint32_t size = 0;
    uint32_t size_flag = 0;
    std::tie(res, name) = get_die_name(dbg, type_die);
    std::tie(size_flag, size) = get_type_size(dbg,type_die);

    Dwarf_Half tag = 0;
    int is_tag = 0;
    std::tie(is_tag, tag) = get_die_tag(dbg, type_die);
    if (is_tag != DW_DLV_OK) {
        return std::tuple{res, std::string(), 0};
    }

    if (res!=DW_DLV_OK&&tag==DW_TAG_subroutine_type) {
        if (std::tuple{DW_DLV_OK,DW_TAG_pointer_type}==get_die_tag(dbg,die))
            return std::tuple{DW_DLV_OK, std::string("void"), 4};
    }

    if (res != DW_DLV_OK) {
        uint32_t array_count=0;
        switch (tag) {
            case DW_TAG_union_type:
                dwarf_dealloc_die(type_die);
                return std::tuple{DW_DLV_OK, "<anonymous union>",size};
            case DW_TAG_structure_type:
                dwarf_dealloc_die(type_die);
                return std::tuple{DW_DLV_OK, "<anonymous struct>",size};
            case DW_TAG_class_type:
                dwarf_dealloc_die(type_die);
                return std::tuple{DW_DLV_OK, "<anonymous class>",size};
            case DW_TAG_const_type:
            case DW_TAG_volatile_type:
                std::tie(res, name,size) = get_die_type_with_size(dbg, type_die);
                break;
            case DW_TAG_pointer_type:
                std::tie(res, name,size) = get_die_type_with_size(dbg, type_die);
                name = name + " *";
                break;
            case DW_TAG_array_type:
                std::tie(res, name,size) = get_die_type_with_size(dbg, type_die);
                if (res!=DW_DLV_OK) {return {res, {},0};}
                array_count = get_array_count(dbg,type_die);
                name = name + '[' + std::to_string(array_count) + ']';
                break;
            default: {
                dwarf_dealloc_die(type_die);
                std::cout << "error: error in get type die name" << std::endl;
                return std::tuple{res, std::string(),0};
            }
        }
        dwarf_dealloc_die(type_die);
        return std::tuple{res, name, size};
    }

    std::string recursion_name;
    switch (tag) {
        case DW_TAG_typedef:
            std::tie(res, recursion_name,size) = get_die_type_with_size(dbg, type_die);
            if (recursion_name.find("<anonymous")!=std::string::npos) {break;}
            dwarf_dealloc_die(type_die);
            return std::tuple{res, recursion_name,size};
        default:
            break;
    }

    dwarf_dealloc_die(type_die);
    return std::tuple{res, name,size};
}

std::tuple<int, std::string, uint32_t> get_type_die_name_with_size(Dwarf_Debug dbg, Dwarf_Die die) {

    int res = 0;
    std::string name;
    uint32_t size = 0;
    uint32_t size_flag = 0;
    std::tie(res, name) = get_die_name(dbg, die);
    std::tie(size_flag, size) = get_type_size(dbg,die);

    Dwarf_Half tag = 0;
    int is_tag = 0;
    std::tie(is_tag, tag) = get_die_tag(dbg, die);
    if (is_tag != DW_DLV_OK) {
        return std::tuple{res, std::string(), 0};
    }

    if (res != DW_DLV_OK) {
        switch (tag) {
            case DW_TAG_union_type:
                return std::tuple{DW_DLV_OK, "<anonymous union>", size};
            case DW_TAG_structure_type:
                return std::tuple{DW_DLV_OK, "<anonymous struct>", size};
            case DW_TAG_class_type:
                return std::tuple{DW_DLV_OK, "<anonymous class>", size};
            default: {
                std::cout << "error: error in get type die name" << std::endl;
                return std::tuple{res, std::string(),0};
            }
        }
    }
    std::string recursion_name;
    switch (tag) {
        case DW_TAG_typedef:
             std::tie(res, recursion_name,size) = get_die_type_with_size(dbg, die);
            if (recursion_name.find("<anonymous")!=std::string::npos||recursion_name.empty()) {break;}
            return std::tuple{res, recursion_name,size};
        default:
            break;
    }
    return std::tuple{res, name,size};
}

int display_die_name(Dwarf_Debug dbg, Dwarf_Die die) {
    int res = 0;
    std::string name;
    std::tie(res, name) = get_die_name(dbg, die);
    if (res == DW_DLV_OK)
        std::cout << "name: " << name << std::endl;
    else
        std::cout << "name: no name!" << std::endl;
    return res;
}

int display_die_location(Dwarf_Debug dbg, Dwarf_Die die) {
    int res = 0;
    Dwarf_Half opcode = 0;
    uint64_t addr = 0;

    std::tie(res, addr, opcode) = get_die_location(dbg, die);
    if (res == DW_DLV_OK) {
        std::cout << "opcode: " << trans_dw_op(opcode) << std::endl;
        std::cout << "addr: " << std::hex << addr << std::dec << std::endl;
    } else {
        std::cout << "opcode: no opcode!" << std::endl;
        std::cout << "addr: no addr!" << std::endl;
    }
    return res;
}

int display_die_type_with_size(Dwarf_Debug dbg, Dwarf_Die die) {
    int res = 0;
    uint32_t size = 0;
    std::string type;
    std::tie(res, type, size) = get_die_type_with_size(dbg, die);
    if (res == DW_DLV_OK)
        std::cout << "type: " << type << "\tsize: " << size << std::endl;
    else
        std::cout << "type: no type!" << std::endl;
    return res;
}

void display_attr_num(Dwarf_Half attr_num) {
    std::cout << "attr_num: " << trans_dw_attr_num(attr_num) << std::endl;
}

constexpr const char* trans_DW_ATTR_NUM[] = {
    "no attr 0",
    "DW_AT_sibling",
    "DW_AT_location",
    "DW_AT_name",
    "no attr 4",
    "no attr 5",
    "no attr 6",
    "no attr 7",
    "no attr 8",
    "DW_AT_ordering",
    "DW_AT_subscr_data",
    "DW_AT_byte_size",
    "DW_AT_bit_offset",
    "DW_AT_bit_size",
    "no attr e",
    "DW_AT_element_list",
    "DW_AT_stmt_list",
    "DW_AT_low_pc",
    "DW_AT_high_pc",
    "DW_AT_language",
    "DW_AT_member",
    "DW_AT_discr",
    "DW_AT_discr_value",
    "DW_AT_visibility",
    "DW_AT_import",
    "DW_AT_string_length",
    "DW_AT_common_reference",
    "DW_AT_comp_dir",
    "DW_AT_const_value",
    "DW_AT_containing_type",
    "DW_AT_default_value",
    "no attr 1f",
    "DW_AT_inline",
    "DW_AT_is_optional",
    "DW_AT_lower_bound",
    "no attr 23",
    "no attr 24",
    "DW_AT_producer",
    "no attr 26",
    "DW_AT_prototyped",
    "no attr 28",
    "no attr 29",
    "DW_AT_return_addr",
    "no attr 2b",
    "DW_AT_start_scope",
    "no attr 2d",
    "DW_AT_bit_stride",
    "DW_AT_upper_bound",
    "no attr 30",
    "DW_AT_abstract_origin",
    "DW_AT_accessibility",
    "DW_AT_address_class",
    "DW_AT_artificial",
    "DW_AT_base_types",
    "DW_AT_calling_convention",
    "DW_AT_count",
    "DW_AT_data_member_location",
    "DW_AT_decl_column",
    "DW_AT_decl_file",
    "DW_AT_decl_line",
    "DW_AT_declaration",
    "DW_AT_discr_list",
    "DW_AT_encoding",
    "DW_AT_external",
    "DW_AT_frame_base",
    "DW_AT_friend",
    "DW_AT_identifier_case",
    "DW_AT_macro_info",
    "DW_AT_namelist_item",
    "DW_AT_priority",
    "DW_AT_segment",
    "DW_AT_specification",
    "DW_AT_static_link",
    "DW_AT_type",
    "DW_AT_use_location",
    "DW_AT_variable_parameter",
    "DW_AT_virtuality",
    "DW_AT_vtable_elem_location",
    "DW_AT_allocated",
    "DW_AT_associated",
    "DW_AT_data_location",
    "DW_AT_byte_stride",
    "DW_AT_entry_pc",
    "DW_AT_use_UTF8",
    "DW_AT_extension",
    "DW_AT_ranges",
    "DW_AT_trampoline",
    "DW_AT_call_column",
    "DW_AT_call_file",
    "DW_AT_call_line",
    "DW_AT_description",
    "DW_AT_binary_scale",
    "DW_AT_decimal_scale",
    "DW_AT_small",
    "DW_AT_decimal_sign",
    "DW_AT_digit_count",
    "DW_AT_picture_string",
    "DW_AT_mutable",
    "DW_AT_threads_scaled",
    "DW_AT_explicit",
    "DW_AT_object_pointer",
    "DW_AT_endianity",
    "DW_AT_elemental",
    "DW_AT_pure",
    "DW_AT_recursive",
    "DW_AT_signature",
    "DW_AT_main_subprogram",
    "DW_AT_data_bit_offset",
    "DW_AT_const_expr",
    "DW_AT_enum_class",
    "DW_AT_linkage_name",
    "DW_AT_string_length_bit_size",
    "DW_AT_string_length_byte_size",
    "DW_AT_rank",
    "DW_AT_str_offsets_base",
    "DW_AT_addr_base",

    "DW_AT_rnglists_base",
    "DW_AT_dwo_id",
    "DW_AT_dwo_name",
    "DW_AT_reference",
    "DW_AT_rvalue_reference",
    "DW_AT_macros",
    "DW_AT_call_all_calls",
    "DW_AT_call_all_source_calls",
    "DW_AT_call_all_tail_calls",
    "DW_AT_call_return_pc",
    "DW_AT_call_value",
    "DW_AT_call_origin",
    "DW_AT_call_parameter",
    "DW_AT_call_pc",
    "DW_AT_call_tail_call",
    "DW_AT_call_target",
    "DW_AT_call_target_clobbered",
    "DW_AT_call_data_location",
    "DW_AT_call_data_value",
    "DW_AT_noreturn",
    "DW_AT_alignment",
    "DW_AT_export_symbols",
    "DW_AT_deleted",
    "DW_AT_defaulted",
    "DW_AT_loclists_base",
    "no attr 8d",
    "no attr 8e",
    "no attr 8f",
    "DW_AT_language_name",
    "DW_AT_language_version",
};


std::string trans_dw_attr_num(Dwarf_Half attr_num) {
    if (attr_num <= 0x91) {
        return trans_DW_ATTR_NUM[attr_num];
    }
    return "unknown attr num: " + std::to_string(attr_num);
}
