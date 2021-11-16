#ifndef RD53BREGISTER_H
#define RD53BREGISTER_H

#include <string>
#include <vector>

namespace Ph2_HwDescription
{

namespace RD53BConstants {

enum class RegType {
    ReadWrite,
    ReadOnly,
    Special
};

struct Register {
    std::string name;
    uint16_t address;
    uint8_t size;
    uint16_t defaultValue;
    bool isVolatile;
    RegType type;

    friend bool operator==(const Register& a, const Register& b) {
        return a.address == b.address;
    }
    
    friend bool operator!=(const Register& a, const Register& b) {
        return !(a == b);
    }

    friend bool operator<(const Register& a, const Register& b) {
        return a.address < b.address;
    }
};

struct RegisterField {
    const Register& reg;
    uint8_t offset;
    uint8_t size;
};

struct VirtualRegister {
    std::string name;
    std::vector<RegisterField> fields;
};

}

}

#endif