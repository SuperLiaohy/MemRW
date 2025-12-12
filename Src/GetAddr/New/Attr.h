//
// Created by liaohy on 12/11/25.
//

#pragma once
#include <optional>
#include <string>
#include "Utils.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libdwarf.h>
#include <dwarf.h>
#ifdef __cplusplus
}
#endif


namespace dwarf {
class Attr {
public:
    using Form = Dwarf_Half;
    using Symbol = Dwarf_Half;
    Attr() : attr(nullptr) {}
    Attr(Dwarf_Attribute attr) : attr(attr) {}
    ~Attr() {if (attr!=nullptr) dwarf_dealloc_attribute(attr);}

    [[nodiscard]] std::optional<Form> getForm();
    [[nodiscard]] std::optional<Symbol> getSymbol();

    [[nodiscard]] std::optional<std::string> getData();

private:
    Dwarf_Attribute attr;
};
}
