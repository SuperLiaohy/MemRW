//
// Created by liaohy on 12/10/25.
//

#include "Dwarf.h"
#include "Utils.h"
#include "Die.h"


namespace dwarf {
Dwarf::Dwarf(const std::string &path) {
    Dwarf_Error error = nullptr;
    int res = dwarf_init_path(path.c_str(), nullptr, 0,
                              DW_GROUPNUMBER_ANY,
                              nullptr, nullptr,
                              &dbg,
                              &error);
    Error::initDwarf(this);
    if (res != DW_DLV_OK) {
        dbg = nullptr;
        std::clog << "can not load dwarf file" << std::endl;
    }
}

std::optional<Die> Dwarf::getNextCU() {
    int res;
    Error error;
    Dwarf_Die cuDie = nullptr;
    Dwarf_Unsigned cu_header_len = 0;
    // Dwarf_Error error = nullptr;

    // dwarf文件的版本
    Dwarf_Half dw_version = 0;
    Dwarf_Off abbrev_off = 0;
    Dwarf_Half address_size = 0;
    Dwarf_Half len_size = 0;
    Dwarf_Half extension_size = 0;
    Dwarf_Sig8 type_signature{};
    Dwarf_Unsigned type_off = 0;
    Dwarf_Unsigned next_cu_header_off = 0;
    Dwarf_Half cu_type = 0;
    res = dwarf_next_cu_header_e(dbg, 1,
                                 &cuDie,
                                 &cu_header_len,
                                 &dw_version, &abbrev_off,
                                 &address_size, &len_size,
                                 &extension_size, &type_signature,
                                 &type_off, &next_cu_header_off,
                                 &cu_type, error.ptr());
    error.autoHandle();
    if (res == DW_DLV_NO_ENTRY) return Die(nullptr);
    if (res != DW_DLV_OK) return std::nullopt;
    return Die(cuDie);
}
}
