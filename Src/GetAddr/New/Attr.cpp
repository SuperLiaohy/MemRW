//
// Created by liaohy on 12/11/25.
//

#include "Attr.h"

namespace dwarf {
std::optional<Attr::Form> Attr::getForm() {
    return 0;
}

std::optional<Attr::Symbol> Attr::getSymbol() {
    Error error;
    Symbol symbol;
    auto res = dwarf_whatattr(attr, &symbol, error.ptr());
    error.autoHandle();
    if (res != DW_DLV_OK) return std::nullopt;
    return symbol;
}

std::optional<std::string> Attr::getData() {
    Error error;
    char* str;
    // if (getSymbol()==DW_AT_name) {
        auto res = dwarf_formstring(attr, &str, error.ptr());
        if (res!=DW_DLV_OK) return std::nullopt;
        return std::string(str);
    // }
    // return std::nullopt;
}

}
