// #include "RD53BThresholdEqualizationUnbiased.h"

// #include "../Utils/xtensor/xmasked_view.hpp"
// #include "../Utils/xtensor/xindex_view.hpp"
// #include "../Utils/xtensor/xio.hpp"
// #include "../Utils/xtensor/xrandom.hpp"

// #include <TH2F.h>
// #include <TApplication.h>
// #include <TFile.h>

// namespace RD53BTools {

// template <class Flavor>
// ChipDataMap<xt::xtensor<uint8_t, 2>> RD53BThresholdEqualizationUnbiased<Flavor>::run(Task progress) {
//     auto& chipInterface = Base::chipInterface();

//     auto thresholdMeasurementTask   = progress.subTask({0, 0.2});
//     auto tuningTask                 = progress.subTask({0.2, 1});

//     auto& injectionTool = param("injectionTool"_s);
//     auto& offset = injectionTool.param("offset"_s);
//     auto& size = injectionTool.param("size"_s);

//     Base::for_each_chip([&] (Chip* chip) {
//         auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
//         rd53b->pixelConfig.tdac.fill(15);
//         chipInterface.UpdatePixelConfig(rd53b, false, true);
//     });

//     ChipDataMap<size_t> VCAL_HIGH;
//     if (param("targetThreshold"_s) == 0) {
//         auto scanOccMap = param("thresholdScan"_s).run(thresholdMeasurementTask);
//         auto thresholdMap = param("thresholdScan"_s).analyze(scanOccMap)[0];
        
//         Base::for_each_chip([&] (Chip* chip) {
//             double meanThreshold = xt::mean(xt::filter(thresholdMap[chip], xt::view(scanOccMap[chip], -1, xt::all(), xt::all()) > .9))();
//             VCAL_HIGH[chip] = param("thresholdScan"_s).param("vcalMed"_s) + meanThreshold;
//         });
//     }
//     else {
//         Base::for_each_chip([&] (Chip* chip) {
//             VCAL_HIGH[chip] = param("thresholdScan"_s).param("vcalMed"_s) + param("targetThreshold"_s);
//         });
//     }

//     ChipDataMap<xt::xtensor<uint8_t, 2>> bestTDAC;
//     ChipDataMap<xt::xtensor<double, 2>> bestOcc;

//     Base::for_each_chip([&] (Chip* chip) {
//         chipInterface.WriteReg(chip, Flavor::Reg::VCAL_MED, param("thresholdScan"_s).param("vcalMed"_s));
//         bestTDAC.insert({chip, 15 * xt::ones<uint8_t>(size)});
//         bestOcc.insert({chip, xt::zeros<double>(size)});
//     });
    
//     xt::print_options::set_line_width(160);
//     xt::print_options::set_threshold(2000);

//     xt::xtensor<bool, 2> last_step_choice = xt::random::randint<int>(size, 0, 1);

//     size_t nSteps = 6;
//     for (size_t i = 0; i < nSteps; ++i) {
//         size_t tdacStep = i < 4 ? (8 >> i) : 1;

//         Base::for_each_chip([&] (Chip* chip) {
//             chipInterface.WriteReg(chip, Flavor::Reg::VCAL_HIGH, VCAL_HIGH[chip]);
//         });
        
//         auto events = injectionTool.run(tuningTask.subTask({i / double(nSteps), (i + .5) / double(nSteps)}));
//         auto occMap = injectionTool.occupancy(events);

//         Base::for_each_hybrid([&] (Hybrid* hybrid) {
//             chipInterface.WriteReg(hybrid, Flavor::Reg::VCAL_HIGH, 0xFFFF);
//         });
        
//         auto eventsHighCharge = injectionTool.run(tuningTask.subTask({(i + .5) / double(nSteps), (i + 1) / double(nSteps)}));
//         auto occMapHighCharge = injectionTool.occupancy(eventsHighCharge);

//         Base::for_each_chip([&] (Chip* chip) {
//             auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
//             auto tdacView = xt::view(rd53b->pixelConfig.tdac, xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
//             const auto occ = xt::view(occMap[chip], xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
//             const auto occHighCharge = xt::view(occMapHighCharge[chip], xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));

//             const auto stuck = occHighCharge < .9;

//             auto cost = xt::abs(occ - 0.5);
//             auto minCost = xt::abs(bestOcc[chip] - 0.5);

//             xt::xtensor<bool, 2> isBest = !stuck && cost < minCost;

//             if (i < nSteps - 1)
//                 isBest |= xt::isclose(cost, minCost);
//             else
//                 isBest |= xt::isclose(occ, bestOcc[chip]) || (xt::isclose(cost, minCost) && last_step_choice);

//             xt::masked_view(bestOcc[chip], isBest) = occ;
//             xt::masked_view(bestTDAC[chip], isBest) = tdacView;

//             // std::cout << "isBest:\n" <<  xt::filter(isBest, xt::abs(bestOcc[chip] - .5) > .49) << std::endl;
//             // std::cout << "diff:\n" <<  xt::filter((xt::abs(occ - 0.5) - xt::abs(bestOcc[chip] - .5)), xt::abs(bestOcc[chip] - .5) > .49) << std::endl;
//             // std::cout << "occ:\n" <<  xt::filter(occ, xt::abs(bestOcc[chip] - .5) > .49) << std::endl;
//             // std::cout << "bestOcc:\n" <<  xt::filter(bestOcc[chip], xt::abs(bestOcc[chip] - .5) > .49) << std::endl;
//             // std::cout << "bestTDAC:\n" <<  xt::filter(bestTDAC[chip], xt::abs(bestOcc[chip] - .5) > .49) << std::endl;

//             if (i < nSteps - 1) {
//                 tdacView = xt::where(!stuck && (occ < 0.5), tdacView + tdacStep, xt::where(tdacView > tdacStep, tdacView - tdacStep, 0u));
//                 // std::cout << tdacView << std::endl;

//                 chipInterface.UpdatePixelConfig(rd53b, false, true); // update tdacs
//             }
//         });
//     }


//     Base::for_each_chip([&] (Chip* chip) {
        
//         // std::cout << "bestOcc:\n" << bestOcc[chip] << std::endl;
//         // std::cout << "bestTDAC:\n" << bestTDAC[chip] << std::endl;
        
//         auto* rd53b = static_cast<RD53B<Flavor>*>(chip);
//         auto tdacView = xt::view(rd53b->pixelConfig.tdac, xt::range(offset[0], offset[0] + size[0]), xt::range(offset[1], offset[1] + size[1]));
//         tdacView = bestTDAC[chip];
//         // std::cout << rd53b->pixelConfig.tdac << std::endl;
//         chipInterface.UpdatePixelConfig(rd53b, false, true);
//     }); 


//     return bestTDAC;
// }

// template <class Flavor>
// void RD53BThresholdEqualizationUnbiased<Flavor>::draw(const ChipDataMap<xt::xtensor<uint8_t, 2>>& bestTDAC) {
//     Base::createRootFile();
//     auto& offset = param("injectionTool"_s).param("offset"_s);

//     for (const auto& item : bestTDAC) {
//         const auto& tdac = item.second;
        
//         Base::drawMap(tdac, "TDAC Map", "TDAC", offset[0], offset[1]);
//         Base::drawHist(tdac, "TDAC Distribution", 32, 0, 32, "TDAC");
//     }
// }


// template class RD53BThresholdEqualizationUnbiased<RD53BFlavor::ATLAS>;
// template class RD53BThresholdEqualizationUnbiased<RD53BFlavor::CMS>;

// }