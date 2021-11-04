#ifndef RD53BUTILS_H
#define RD53BUTILS_H

#include "../System/SystemController.h"

namespace RD53BUtils {

    namespace detail {
        template< class... >
        using void_t = void;

        template <class F, class = void, typename... Args>
        struct detect_callable_with : std::false_type {};

        template <class F, typename... Args>
        struct detect_callable_with<F, void_t<decltype(std::declval<F>()(std::declval<Args>()...))>, Args...>
            : std::true_type {};

        template <class F, class... Args>
        struct callable_with : std::integral_constant<bool, 
            (detect_callable_with<F, void, Args...>::value || detect_callable_with<F, void, Args&...>::value)
        > {};

    }

    using namespace Ph2_HwDescription;

    struct DeviceChain {
        BeBoard* board = nullptr;
        OpticalGroup* opticalGroup = nullptr;
        Hybrid* hybrid = nullptr;
        Chip* chip = nullptr;

        void set(BeBoard* board) { this->board = board; }
        void set(OpticalGroup* opticalGroup) { this->opticalGroup = opticalGroup; }
        void set(Hybrid* hybrid) { this->hybrid = hybrid; }
        void set(Chip* chip) { this->chip = chip; }
    };

    struct ChipLocation {
        ChipLocation(Chip* pChip) : board_id(pChip->getBeBoardId()), hybrid_id(pChip->getHybridId()), chip_id(pChip->getId()) {}

        // Chip* getChip(SystemController& system) {
        //     return system.fDetectorContainer->at(board_id)->at(0)->at(hybrid_id)->at(chip_id);
        // }

        uint16_t board_id;
        uint16_t hybrid_id;
        uint16_t chip_id;

        size_t hash() const { return (board_id << 16) | (hybrid_id << 8) | chip_id; }

        friend bool operator<(const ChipLocation& l, const ChipLocation& r ) {
            return l.hash() < r.hash();
        }

        friend std::ostream& operator<<(std::ostream& os, const ChipLocation& loc) {
            return (os << '[' << loc.board_id << '/' << loc.hybrid_id << '/' << loc.chip_id << ']');
        }
    };


    // for_each_device: call given function for each device of given type
    // usage: for_each_device<Target>(root, f);
    // where: 
    //  - Target is one of: BeBoard, Hybrid, Chip
    //  - root is an object of one of the following types: SystemController&, BeBoard*, Hybrid*, Chip*
    //  - f is a function with one parameter of one of the following types: BeBoard*, Hybrid*, Chip*, DeviceChain

    template <
        class TargetDevice, 
        class F, 
        typename std::enable_if_t<!detail::callable_with<F, TargetDevice*>::value, int> = 0
    >
    void for_each_device(TargetDevice* device, F&& f, DeviceChain devices = {}) {
        devices.set(device);
        std::forward<F>(f)(devices);
    }

    template <
        class TargetDevice, 
        class F, 
        class Device, 
        typename std::enable_if_t<(!detail::callable_with<F, TargetDevice*>::value && !std::is_base_of<TargetDevice, Device>::value), int> = 0
    >
    void for_each_device(Device* device, F&& f, DeviceChain devices = {}) {
        devices.set(device);
        for (auto* child_device : *device) {
            for_each_device<TargetDevice>(child_device, std::forward<F>(f), devices);
        }
    }

    template <
        class TargetDevice, 
        class F, 
        typename std::enable_if_t<detail::callable_with<F, TargetDevice*>::value, int> = 0
    >
    void for_each_device(TargetDevice* device, F&& f) {
        std::forward<F>(f)(device);
    }

    template <
        class TargetDevice, 
        class F, 
        class Device, 
        typename std::enable_if_t<(detail::callable_with<F, TargetDevice*>::value && !std::is_base_of<TargetDevice, Device>::value), int> = 0
    >
    void for_each_device(Device* device, F&& f) {
        for (auto* child_device : *device) {
            for_each_device<TargetDevice>(child_device, std::forward<F>(f));
        }
    }

    // System wide loop
    template <class TargetDevice, class F>
    void for_each_device(Ph2_System::SystemController& system, F&& f) {
        for (auto* board : *system.fDetectorContainer) {
            for_each_device<TargetDevice>(board, std::forward<F>(f));
        }
    }

}

#endif