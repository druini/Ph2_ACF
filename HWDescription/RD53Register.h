#ifndef RD53REGISTER_H
#define RD53REGISTER_H

namespace Ph2_HwDescription
{

struct RD53Register {
    RD53Register(
        std::string name,
        size_t address,
        std::initializer_list<uint8_t> fieldSizes, 
        std::initializer_list<uint16_t> defaultValues
    )
        : RD53Register(std::move(name), address, fieldSizes, defaultValues, false)
    {}

    RD53Register(
        std::string name,
        size_t address,
        std::initializer_list<uint8_t> fieldSizes, 
        std::initializer_list<uint16_t> defaultValues,
        bool readOnly
    )
        : fieldSizes(std::rbegin(fieldSizes), std::rend(fieldSizes))
        , defaultValues(std::rbegin(defaultValues), std::rend(defaultValues))
        , name(std::move(name))
        , address(address)
        , defaultValue(0)
        , size(std::accumulate(fieldSizes.begin(), fieldSizes.end(), 0))
        , readOnly(readOnly)
    {
        size_t offset = 0;
        for (size_t i = 0; i < fieldSizes.size(); ++i) {
            defaultValue |= (this->defaultValues[i] & ((1 << this->fieldSizes[i]) - 1)) << offset;
            offset += this->fieldSizes[i];
        }
    }

    friend bool operator==(const RD53Register& a, const RD53Register& b) {
        return a.address == b.address;
    }
    
    friend bool operator!=(const RD53Register& a, const RD53Register& b) {
        return !(a == b);
    }

    std::vector<uint8_t> fieldSizes;
    std::vector<uint16_t> defaultValues;
    std::string name;
    uint16_t address;
    uint16_t defaultValue;
    uint8_t size;
    bool readOnly;
};

}

#endif