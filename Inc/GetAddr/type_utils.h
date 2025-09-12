//
// Created by liaohy on 8/6/25.
//

#pragma once
#include <functional>
#include "dw_base_utils.h"

uint32_t get_array_count(Dwarf_Debug dbg, Dwarf_Die die);
std::tuple<int, uint32_t> get_type_size(Dwarf_Debug dbg, Dwarf_Die die);

int get_type_die(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Die *type_die);
int recursion_type_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)>& func);
std::tuple<std::string, uint32_t> recursion_type_judge(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)>& func);

