//
// Created by liaohy on 12/11/25.
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

namespace dwarf {

class Dwarf;

class Error {
public:
    Error() : error(nullptr) {}
    ~Error();

    static void initDwarf(Dwarf* file) {dwarfFile=file;}

    Dwarf_Error* ptr() {return &error;}
    bool clear();
    void showMsg();
    void autoHandle();
private:
    static volatile Dwarf* dwarfFile;
    Dwarf_Error error;
};
}