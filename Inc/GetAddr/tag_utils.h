//
// Created by liaohy on 8/4/25.
//

#pragma once

#include "dw_base_utils.h"

std::tuple<int, Dwarf_Half> get_die_tag(Dwarf_Debug dbg, Dwarf_Die die);

int display_die_tag(Dwarf_Debug dbg, Dwarf_Die die);

void display_tag(Dwarf_Half tag);
std::string trans_dw_tag(Dwarf_Half tag);


