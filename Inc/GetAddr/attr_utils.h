//
// Created by liaohy on 8/3/25.
//

#pragma once
#include "dw_base_utils.h"

std::tuple<int, std::string> get_die_name(Dwarf_Debug dbg, Dwarf_Die die);
std::tuple<int, uint64_t, Dwarf_Half> get_die_location(Dwarf_Debug dbg, Dwarf_Die die);
std::tuple<int, std::string, uint32_t> get_die_type_with_size(Dwarf_Debug dbg, Dwarf_Die die);
std::tuple<int, std::string, uint32_t> get_type_die_name_with_size(Dwarf_Debug dbg, Dwarf_Die die);

int display_die_name(Dwarf_Debug dbg, Dwarf_Die die);
int display_die_location(Dwarf_Debug dbg, Dwarf_Die die);
int display_die_type_with_size(Dwarf_Debug dbg, Dwarf_Die die);

void display_attr_num(Dwarf_Half attr_num);
std::string trans_dw_attr_num(Dwarf_Half attr_num);
