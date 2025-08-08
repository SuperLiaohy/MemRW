//
// Created by liaohy on 8/4/25.
//

#pragma once
#include <dw_base_utils.h>
#include <functional>

int display_single_vari_die(Dwarf_Debug dbg, Dwarf_Die die);
int display_single_die(Dwarf_Debug dbg, Dwarf_Die die);
int display_recursion_die(Dwarf_Debug dbg, Dwarf_Die die);

int recursion_die_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)>& func);
int root_recursion_die_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)> &func);
