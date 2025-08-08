//
// Created by liaohy on 8/6/25.
//

#pragma once
#include <functional>
#include "dw_base_utils.h"

int get_type_die(Dwarf_Debug dbg, Dwarf_Die die, Dwarf_Die *type_die);
int display_full_type(Dwarf_Debug dbg, Dwarf_Die die);
int recursion_type_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)>& func);
void display_recursion_die_type(Dwarf_Debug dbg, Dwarf_Die die);

