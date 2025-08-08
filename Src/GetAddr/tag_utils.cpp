//
// Created by liaohy on 8/4/25.
//

#include "tag_utils.h"
#include <iostream>


std::tuple<int, Dwarf_Half> get_die_tag(Dwarf_Debug dbg, Dwarf_Die die) {
    int res = 0;
    Dwarf_Error error = nullptr;
    Dwarf_Half tag = 0;
    res = dw_error_check(dwarf_tag(die, &tag, &error),dbg,error);
    return std::tuple{res, tag};
}

int display_die_tag(Dwarf_Debug dbg, Dwarf_Die die) {
    int res = 0;
    Dwarf_Half tag = 0;
    std::tie(res, tag) = get_die_tag(dbg, die);
    if (res == DW_DLV_OK) {
        display_tag(tag);
    } else {
        std:: cout << "tag: no tag!"  << std::endl;
    }
    return res;
}

void display_tag(Dwarf_Half tag) {
        std:: cout << "tag: " << trans_dw_tag(tag) << std::endl;
}


constexpr const char * trans_DW_TAG[] = {
    "no tag 0",
    "DW_TAG_array_type",
    "DW_TAG_class_type",
    "DW_TAG_entry_point",
    "DW_TAG_enumeration_type",
    "DW_TAG_formal_parameter",
    "no tag 6",
    "no tag 7",
    "DW_TAG_imported_declaration",
    "no tag 9",
    "DW_TAG_label",
    "DW_TAG_lexical_block",
    "no tag c",
    "DW_TAG_member",
    "no tag e",
    "DW_TAG_pointer_type",
    "DW_TAG_reference_type",
    "DW_TAG_compile_unit",
    "DW_TAG_string_type",
    "DW_TAG_structure_type",
    "no tag 14",
    "DW_TAG_subroutine_type",
    "DW_TAG_typedef",
    "DW_TAG_union_type",
    "DW_TAG_unspecified_parameters",
    "DW_TAG_variant",
    "DW_TAG_common_block",
    "DW_TAG_common_inclusion",
    "DW_TAG_inheritance",
    "DW_TAG_inlined_subroutine",
    "DW_TAG_module",
    "DW_TAG_ptr_to_member_type",
    "DW_TAG_set_type",
    "DW_TAG_subrange_type",
    "DW_TAG_with_stmt",
    "DW_TAG_access_declaration",
    "DW_TAG_base_type",
    "DW_TAG_catch_block",
    "DW_TAG_const_type",
    "DW_TAG_constant",
    "DW_TAG_enumerator",
    "DW_TAG_file_type",
    "DW_TAG_friend",
    "DW_TAG_namelist",
    "DW_TAG_namelist_items",
    "DW_TAG_packed_type",
    "DW_TAG_subprogram",
    "DW_TAG_template_type_parameter",
    "DW_TAG_template_value_parameter",
    "DW_TAG_thrown_type",
    "DW_TAG_try_block",
    "DW_TAG_variant_part",
    "DW_TAG_variable",
    "DW_TAG_volatile_type",
    "DW_TAG_dwarf_procedure",
    "DW_TAG_restrict_type",
    "DW_TAG_interface_type",
    "DW_TAG_namespace",
    "DW_TAG_imported_module",
    "DW_TAG_unspecified_type",
    "DW_TAG_partial_unit",
    "DW_TAG_imported_unit",
    "DW_TAG_mutable_type",
    "DW_TAG_condition",
    "DW_TAG_shared_type",
    "DW_TAG_type_unit",
    "DW_TAG_rvalue_reference_type",
    "DW_TAG_template_alias",
    "DW_TAG_coarray_type",
    "DW_TAG_generic_subrange",
    "DW_TAG_dynamic_type",
    "DW_TAG_atomic_type",
    "DW_TAG_call_site",
    "DW_TAG_call_site_parameter",
    "DW_TAG_skeleton_unit",
    "DW_TAG_immutable_type",

};

std::string trans_dw_tag(Dwarf_Half tag) {
    if (tag <= 0x4b) {
        return trans_DW_TAG[tag];
    }
    return "unknown tag: " + std::to_string(tag);
}