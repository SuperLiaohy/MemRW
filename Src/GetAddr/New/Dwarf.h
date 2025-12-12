//
// Created by liaohy on 12/10/25.
//

#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <libdwarf.h>
#include <dwarf.h>
#ifdef __cplusplus
}
#endif

#include <iostream>
#include <string>
#include <optional>


namespace dwarf {
class Die;
class Dwarf {
    friend class Error;
    friend class Die;
public:
    Dwarf() = delete;
    Dwarf(const std::string& path);
    Dwarf(const Dwarf&) = delete;
    Dwarf operator=(const Dwarf&) = delete;
    Dwarf(Dwarf&&other) noexcept {dbg = other.dbg;other.dbg=nullptr;}
    Dwarf& operator=(Dwarf&&other) noexcept {if (&other!=this) {this->~Dwarf();dbg = other.dbg;other.dbg=nullptr;} return *this;};
    ~Dwarf() {if (dbg != nullptr) dwarf_finish(dbg);}

    bool operator==(nullptr_t) const {return dbg==nullptr;}

    [[nodiscard]] std::optional<Die> getNextCU();

private:
    Dwarf_Debug dbg;
};
}
