//
// Created by liaohy on 12/10/25.
//

#pragma once
#include <optional>
#include <string>
#include <vector>

#include "Attr.h"
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
class Die {
public:
    using Tag = Dwarf_Half;
    Die() = delete;
    Die(Dwarf_Die die) : die(die) {}
    Die(const Die&) = delete;
    Die operator=(const Die&) = delete;
    Die(Die&&other) noexcept {die = other.die;other.die=nullptr;}
    Die& operator=(Die&&other) noexcept {if (&other!=this) {this->~Die();die = other.die;other.die=nullptr;} return *this;};
    ~Die() {if (die!=nullptr) dwarf_dealloc_die(die);}

    bool operator==(nullptr_t) const {return die==nullptr;}

    std::optional<Die> getChild() noexcept;
    std::optional<Die> getSib() noexcept;
    template<typename Callback>
    void recursion(Callback callback, void* userData) noexcept;

    [[nodiscard]] bool isVariable() noexcept;
    [[nodiscard]] bool isArray() noexcept;
    [[nodiscard]] bool isType() noexcept;

    [[nodiscard]] std::optional<Tag> getTag() noexcept;
    static std::string tagStr(Tag tag) noexcept;

    std::vector<Attr> getAttrList() noexcept;
private:
    Dwarf_Die die;
};


template<typename Callback>
void Die::recursion(Callback callback, void *userData) noexcept {
    auto child = this->getChild();
    while (child!=std::nullopt) {
        if(callback(*child, userData))
            child->recursion(callback, userData);
        child = child->getSib();
    }
}

}
