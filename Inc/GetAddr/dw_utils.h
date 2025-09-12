//
// Created by liaohy on 8/4/25.
//

#pragma once
#include <dw_base_utils.h>
#include <functional>

int display_single_die(Dwarf_Debug dbg, Dwarf_Die die);

int recursion_die_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)>& func);
int one_step_recursion_die_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)>& func);
int root_recursion_die_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)> &func);

int recursion_die_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)>& func, const std::function<bool(Dwarf_Debug, Dwarf_Die)> &valid);
int root_recursion_die_do(Dwarf_Debug dbg, Dwarf_Die die, const std::function<void(Dwarf_Debug, Dwarf_Die)> &func, const std::function<bool(Dwarf_Debug, Dwarf_Die)> &valid);
