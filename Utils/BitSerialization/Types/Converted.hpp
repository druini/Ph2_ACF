#ifndef BITSERIALIZATION__TYPES__CONVERTED_HPP
#define BITSERIALIZATION__TYPES__CONVERTED_HPP

#include "Compose3.hpp"
#include "Transformed.hpp"

namespace BitSerialization {

    template <class InputType, class OutputType, class Transform>
    auto Converted(const InputType& input_type, const OutputType& output_type, const Transform& transform) {
        return Compose3(input_type, Transformed(input_type, transform), output_type);
    }

} // namespace BitSerialization

#endif
