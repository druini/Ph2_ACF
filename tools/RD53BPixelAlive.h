
#include <boost/function_types/function_arity.hpp>


struct ObjectCollection {
    ObjectCollection() {}

    template <class... Ts>
    ObjectCollection(const Ts&... values) {
        int unused[] = {0, (set(values), 0)...};
        (void)unused;
    }

    template <class T>
    void set(const T& object) {
        objects[std::type_index(typeid(T))] = new T{object};
    }

    template <class T>
    T& get() {
        auto it = objects.find(std::type_index(typeid(T)));
        if (it == objects.end()){
            throw std::runtime_error("Type not found in ObjectCollection.");
        }
        return *static_cast<T*>(it->second);
    }

    std::unordered_map<std::type_index, void*> objects;
};

template <class Flavor>
struct RD53BInjectionMaskGenerator {
    RD53BInjectionMaskGenerator(size_t hitsPerCol = 1, std::array<size_t, 2> offset = {0, 0}, std::array<size_t, 2> size = {Flavor::nRows, Flavor::nCols})
      : _offset(offset)
      , _size(size)
      , _hitsPerCol(hitsPerCol)
    {}

    auto operator()(size_t i) {
        RD53B<Flavor>::PixelMatrix<bool> mask(false);
        
        for (size_t col = 0; col < _size[1]; ++col) {
            for (size_t i = 0; i < _hitsPerCol; ++i) {
                size_t row = 8 * col + j * _size[0] / _hitsPerCol;
                row += (row / _size[0]) % 8 + i;
                row %= _size[0];
                mask(offset[0] + row, offset[1] + col) = true;
            }
        }

        return mask;
    }

    size_t size() const { return _hitsPerCol * _size[1]}

private:
    std::array<size_t, 2> _offset;
    std::array<size_t, 2> _size;
    size_t _hitsPerCol;
}


struct RD53BInjectionMaskGenerator {
    RD53BInjectionMaskGenerator(size_t width, size_t height, size_t hitsPerCol = 1)
      : _width(width)
      , _height(height)
      , _hitsPerCol(hitsPerCol)
    {}

    auto operator()(size_t i) {
        RD53B<Flavor>::PixelMatrix<bool> mask(false);
        for (size_t col = 0; col < _height; ++col) {
            for (size_t i = 0; i < _hitsPerCol; ++i) {
                size_t row = 8 * col + j * _width / _hitsPerCol;
                row += (row / _width) % 8 + i;
                row %= _width;
                mask(row, col) = true;
            }
        }
        return mask;
    }

    size_t size() const { return _hitsPerCol * _height}

private:
    size_t _width;
    size_t _height;
    size_t _hitsPerCol;
}

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


template<class F, size_t Arity>
using enable_if_functor_arity = std::enable_if_t<boost::function_types::function_arity<decltype(&F::operator())>::value - 1 == Arity, int>;


template <class TargetDevice, class F, typename enable_if_functor_arity<F, 2> = 0>
void for_each_device(TargetDevice* device, F&& f, DeviceChain devices = {}) {
    devices.set(device);
    std::forward<F>(f)(devices);
}

template <class TargetDevice, class F, class Device, typename enable_if_functor_arity<F, 2> = 0>
void for_each_device(Device* device, F&& f, DeviceChain devices = {}) {
    devices.set(device);
    for (auto* next_level_device : *device) {
        for_each_device<TargetDevice>(next_level_device, std::forward<F>(f), devices);
    }
}

template <class TargetDevice, class F, typename enable_if_functor_arity<F, 1> = 0>
void for_each_device(TargetDevice* device, F&& f) {
    std::forward<F>(f)(device);
}

template <class TargetDevice, class F, class Device, typename enable_if_functor_arity<F, 1> = 0>
void for_each_device(Device* device, F&& f) {
    for (auto* next_level_device : *device) {
        for_each_device<TargetDevice>(next_level_device, std::forward<F>(f));
    }
}


template <class TargetDevice, class F>
void for_each_device(SystemController& system, F&& f) {
    for (auto* board : *sys.fDetectorContainer) {
        for_each_device<TargetDevice>(board, std::forward<F>(f));
    }
}





struct RD53BEvent {
    struct Hit {
        uint16_t row;
        uint16_t col;
        uint8_t tot;
    }

    struct PToTHit {
        uint16_t col_pair;
        uint16_t timeOfArrival;
        uint16_t ToT;
    }

    std::vector<Hit> hits;
    std::vector<PToTHit> pToTHits;
    uint16_t bc_id;
    uint8_t trigger_tag;
    uint8_t trigger_id;
};

// template <class Flavor>
// struct RD53BTool {

//     RD53BTool(SystemController& sys)
//       : system(sys)
//       , chipInterface(*static_cast<RD53BInterface<Flavor>*>(_system.fReadoutChipInterface))
//     {}

//     RD53FWInterface& fwInterface(BeBoard* pBoard) {
//         return *static_cast<RD53FWInterface*>(system.fBeBoardFWMap[pBoard->getId()]);
//     }

    

//     void inject(BeBoard* pBoard) {
//         auto& fwInterface = fwInterface(pBoard);
//         fwInterface.Start()
//         while(fwInterface.ReadReg("user.stat_regs.trigger_cntr") < pNEvents * (1 + fwInterface.getLocalCfgFastCmd->trigger_duration))
//             std::this_thread::sleep_for(std::chrono::microseconds(RD53Shared::READOUTSLEEP));
//         fwInterface.Stop();
//     }

//     std::vector<uint32_t> readData(BeBoard* pBoard) {
//         std::vector<uint32_t> result;
//         fwInterface(pBoard).ReadData(pBoard, false, result, true);
//         return result;
//     }

// protected:
//     SystemController& system;
//     RD53BInterface<Flavor>& chipInterface;
// };

template <class Flavor>
struct RD53BInjectionTool {
    enum class InjectionType : size_t {
        Analog = 0,
        Digital
    };

    RD53BInjectionTool(
        size_t nInjections, 
        size_t triggerDuration, 
        InjectionType injectionType = InjectionType::Analog,
        size_t hitsPerCol = 1, 
        std::array<size_t, 2> offset = {0, 0}, 
        std::array<size_t, 2> size = {Flavor::nRows, Flavor::nCols}
    )
      : RD53BTool<Flavor>(system)
      , _offset(offset)
      , _size(size)
      , _nInjections(nInjections)
      , _triggerDuration(triggerDuration)
      , _injectionType(injectionType)
      , _hitsPerCol(hitsPerCol)
    {}

    struct ChipAddress {
        ChipInfo(Chip* pChip) : board_id(pChip->getBoardId()), hybrid_id(pChip->getHybridId()), chip_id(pChip->getId()) {}

        Chip* getChip(SystemController& system) {
            return system.fDetectorContainer->at(board_id)->at(0)->at(hybrid_id)->at(chip_id);
        }

        uint8_t board_id;
        uint8_t hybrid_id;
        uint8_t chip_id;

        size_t hash() const { return (board_id << 16) | (hybrid_id << 8) | chip_id; }

        friend bool operator<(const A& l, const A& r ) {
            return l.hash() < r.hash();
        }
    };

    std::map<ChipAddress, std::vector<RD53BEvent>> run(SystemController& system) const {
        std::map<ChipAddress, std::vector<RD53BEvent>> chipEventsMap;

        configureInjections(system);

        for_each_device<BeBoard>(system, [&] (BeBoard* board) {

            auto& fwInterface = fwInterface(pBoard);
            std::vector<RD53BEvent> events;

            auto decoder = RD53BFWEventDecoder<Flavor>(board);

            for (int i = 0; i < _nInjections * _triggerDuration ; ++i) {
                auto mask = generateInjectionMask(i);

                for_each_device<Hybrid>(board, [&] (Hybrid* hybrid) {
                    auto cfg = static_cast<RD53B<Flavor>*>(devices.hybrid->at(0))->PixelConfig;

                    cfg.enable[{Slice(offset[0], size[0]), Slice(offset[1], size[1])}] = mask;
                    cfg.enableInjections[{Slice(offset[0], size[0]), Slice(offset[1], size[1])}] = mask;

                    chipInterface.UpdatePixelConfig(devices.hybrid, cfg, true, false);

                    for (auto* chip : *hybrid)
                        static_cast<RD53B<Flavor>*>(chip)->PixelConfig = cfg;
                });

                while (true) {
                    fwInterface.Start();

                    while(fwInterface.ReadReg("user.stat_regs.trigger_cntr") < nInjections * triggerDuration)
                        std::this_thread::sleep_for(std::chrono::microseconds(RD53Shared::READOUTSLEEP));

                    fwInterface.Stop();

                    {
                        std::vector<uint32_t> data;
                        fwInterface.ReadData(pBoard, false, data, true);


                        auto result = decoder.parse(BitView<uint32_t>(data));
                        if (!result) {
                            std::cout << "Decoding error: " << result.error() << std::endl;;
                            continue;
                        }

                        events = std::move(result.value());
                    }

                    if (events.size() < nInjections * triggerDuration) {
                        std::cout << "Not enough events (" << events.size() << " / " << (nInjections * triggerDuration) << ")." << std::endl;
                        continue
                    }
                }
            }

            boardEventsMap[board->getId()] = std::move(events)
        });

        return boardEventsMap;
    }

private:
    auto generateInjectionMask(size_t i) const {
        RD53B<Flavor>::PixelMatrix<bool> mask(false);
        for (size_t col = 0; col < size[1]; ++col) {
            for (size_t i = 0; i < hitsPerCol; ++i) {
                size_t row = 8 * col + j * size[0] / hitsPerCol;
                row += (row / size[0]) % 8 + i;
                row %= size[0];
                mask(row, col) = true;
            }
        }
        return mask;
    }

    void configureInjections(SystemController& system) const {
        for (auto* board : *system.fDetectorContainer) {
            auto& fwInterface = fwInterface(pBoard);
            auto& fastCmdConfig = *fwInterface.getLocalCfgFastCmd();

            fastCmdConfig.n_triggers = nInjections;
            fastCmdConfig.trigger_duration = triggerDuration - 1;
            fastCmdConfig.fast_cmd_fsm.trigger_en = true;
            fastCmdConfig.fast_cmd_fsm.secondCalEnable = true;
            if (injectionType == InjectionType::Analog) {
                fastCmdConfig.fast_cmd_fsm.firstCalEnable = true;
                fastCmdConfig.fast_cmd_fsm.first_cal_data = bits::pack<1, 5, 8, 1, 5>(1, 0, 2, 0, 0);
                fastCmdConfig.fast_cmd_fsm.second_cal_data = bits::pack<1, 5, 8, 1, 5>(0, 0, 16, 0, 0);
            }
            else {
                fastCmdConfig.fast_cmd_fsm.firstCalEnable = false;
                fastCmdConfig.fast_cmd_fsm.second_cal_data = bits::pack<1, 5, 8, 1, 5>(1, 0, 4, 0, 0);
            }
            fwInterface.ConfigureFastCommands(fastCmdConfig);
        }
    }

    std::array<size_t, 2> _offset;
    std::array<size_t, 2> _size;
    size_t _nInjections;
    size_t _triggerDuration;
    InjectionType _injectionType;
    size_t _hitsPerCol;
};


template <class Flavor>
struct RD53BPixelAlive {

    RD53BPixelAlive(const RD53BInjectionTool& injectionTool) : _injectionTool(injectionTool){}

    struct Result {
        RD53B<Flavor>::PixelMatrix<size_t> nHits;
        std::array<size_t, (1 << RD53B<Flavor>::ToTLength)> totHistogram;
        size_t nInjections;
    }

    Result run(SystemController& system) {
        Result result;

        auto boardEventsMap = _injectionTool.run(system);

        for (const auto& item : boardEventsMap) {
            const auto& events = item.second;

            for (const auto& event : events) {
                for (const auto& hit : hits) {

                }
            }
        }
        
    }

    void draw(Result) {}

private:
    RD53BInjectionTool _injectionTool;
}