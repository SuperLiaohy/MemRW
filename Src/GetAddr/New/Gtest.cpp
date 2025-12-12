//
// Created by liaohy on 12/11/25.
//

#include <gtest/gtest.h>
#include "Utils.h"
#include "Dwarf.h"
#include "Attr.h"
#include "Die.h"

TEST(DWARFCPP, DwarfInit) {
    using namespace dwarf;
    Dwarf dwarfFileErr("");
    if (dwarfFileErr==nullptr) std::clog << "False" <<std::endl;
    else std::clog << "True" <<std::endl;
}
TEST(DWARFCPP, DwarfCU) {
    using namespace dwarf;
    Dwarf dwarfFile("/home/liaohy/User/Code/CLion/EmbeddedPack/engineer/build/engineer.elf");
    if (dwarfFile==nullptr) std::clog << "False" <<std::endl;
    else std::clog << "True" <<std::endl;
    std::size_t count=0;
    while (true) {
        auto cu = dwarfFile.getNextCU();
        if (cu==std::nullopt) {std::clog << "cu error." << std::endl; continue;}
        if (*cu == nullptr) {std::clog << "cu over." << std::endl; break;}
        std::clog << "got a right cu, count: " << ++count << "." << std::endl;
    }
}
TEST(DWARFCPP, Die) {
    using namespace dwarf;
    Dwarf dwarfFile("/home/liaohy/User/Code/CLion/EmbeddedPack/engineer/build/engineer.elf");
    if (dwarfFile==nullptr) std::clog << "False" <<std::endl;
    else std::clog << "True" <<std::endl;
    std::size_t count=0;
    while (true) {
        auto cu = dwarfFile.getNextCU();
        if (cu==std::nullopt) {std::clog << "cu error." << std::endl; continue;}
        if (*cu == nullptr) {std::clog << "cu over." << std::endl; break;}
        std::clog << "got a right cu, count: " << ++count << "." << std::endl;
        cu->recursion([](Die& die, void* userData) -> bool {
            if (die.getTag()!=DW_TAG_variable) return false;
            std::clog << '*';
            return true;
        }, nullptr);
        std::clog << std::endl;
    }
}
TEST(DWARFCPP, DieAttr) {
    using namespace dwarf;
    Dwarf dwarfFile("/home/liaohy/User/Code/CLion/EmbeddedPack/engineer/build/engineer.elf");
    if (dwarfFile==nullptr) std::clog << "False" <<std::endl;
    else std::clog << "True" <<std::endl;
    std::size_t count=0;
    while (true) {
        auto cu = dwarfFile.getNextCU();
        if (cu==std::nullopt) {std::clog << "cu error." << std::endl; continue;}
        if (*cu == nullptr) {std::clog << "cu over." << std::endl; break;}
        std::clog << "got a right cu, count: " << ++count << "." << std::endl;
        cu->recursion([](Die& die, void* userData) -> bool {
            auto tag = *die.getTag();
            if (tag!=DW_TAG_variable&&tag!=DW_TAG_namespace) {
                return false;
            }
            if (tag==DW_TAG_variable)
                std::clog << '*';
            else if (tag==DW_TAG_namespace) {
                std::clog << '@';
            }
            auto list = die.getAttrList();
            if (list.empty()) return false;
            for (auto & attr: list) {
                if (attr.getSymbol()==DW_AT_name)
                    std::clog << '|' << attr.getData().value_or("None") << '|';
            }
            return true;
        }, nullptr);
        std::clog << std::endl;
    }
}

