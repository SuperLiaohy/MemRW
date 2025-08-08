//
// Created by liaohy on 8/5/25.
//

#pragma once
#include <cstdint>
#include <tuple>
#include <string>
#include <libdwarf.h>
#include <dwarf.h>


void display_dw_error(Dwarf_Error error);
int dw_error_check(int res, Dwarf_Debug dbg, Dwarf_Error err);

void display_form(Dwarf_Half form);
void display_op(Dwarf_Half op);

std::string trans_dw_op(Dwarf_Half op);
std::string trans_dw_form(Dwarf_Half form);
