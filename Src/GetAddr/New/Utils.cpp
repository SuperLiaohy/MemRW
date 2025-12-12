//
// Created by liaohy on 12/11/25.
//

#include "Utils.h"

#include "Dwarf.h"

namespace dwarf {
volatile Dwarf* Error::dwarfFile = nullptr;
Error::~Error() {
    if (error!=nullptr)
        dwarf_dealloc_error(dwarfFile->dbg,error);
}

bool Error::clear() {
    if (error==nullptr) return false;
    dwarf_dealloc_error(dwarfFile->dbg,error);
    error = nullptr;
    return true;
}

void Error::showMsg() {
    if (error==nullptr) return;
    std::clog << "ERROR due to error: " << dwarf_errmsg(error) << std::endl;
}

void Error::autoHandle() {
    showMsg();
    clear();
}
}
