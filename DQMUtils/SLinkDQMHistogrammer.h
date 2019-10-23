/*!
        \file                SLinkDQMHistogrammer.h
        \brief               base class to create and fill monitoring histograms
        \author              Suchandra Dutta and Subir Sarkar
        \version             1.0
        \date                6/11/17
        Support :            mail to : Suchandra.Dutta@cern.ch, Subir.Sarkar@cern.ch

*/

#ifndef __SLINKDQMHISTOGRAMMER_H__
#define __SLINKDQMHISTOGRAMMER_H__

#include "../tools/Tool.h"
#include "../Utils/easylogging++.h"

#include <vector>
#include <string>
#include <map>

class TH1I;
class TH1F;
class TH1D;
class TH2I;
class TProfile;
class TTree;
class DQMEvent;
class TString;
class StubInfo;
/*!
 * \class SLinkDQMHistogrammer
 * \brief Class to create and fill monitoring histograms
 */
#ifdef __OTSDAQ__
namespace ots
{
#endif

class SLinkDQMHistogrammer
{
  public:
    /*!
     * constructor
     */
    SLinkDQMHistogrammer (int evtType = 0);

    /*!
     * destructor
     */
    virtual ~SLinkDQMHistogrammer();
    /*!
     * Book histograms
     */
    void bookHistograms (const std::vector<std::pair<uint8_t, std::vector<uint8_t> > >& fe_mapping);
    void bookFEUHistograms (uint8_t fe_id, uint8_t n_rou);
    void bookROUHistograms (uint8_t fe_id, uint8_t i_rou);

    /*!
     * Fill histograms
     */
    void fillHistograms (const std::vector<DQMEvent*>& event_list);
    void fillHistograms (const DQMEvent* event);
    void saveHistograms (const std::string& out_file);
    void resetHistograms();
    void fillEventTrendHisto (TH1I* th, unsigned long ival, unsigned int val);

  private:
    bool skipDebugHist_;

    uint32_t totalHits_;
    uint32_t totalStubs_;
    int eventType_;

    std::vector<unsigned int>* rouErrorVal_;
    std::vector<unsigned int>* rouPLAddressVal_;
    std::vector<unsigned int>* dut0C0chData_;
    std::vector<unsigned int>* dut0C1chData_;
    std::vector<unsigned int>* dut1C0chData_;
    std::vector<unsigned int>* dut1C1chData_;
    struct ROUHistos
    {
        TH1I* l1ACounterH_;
        TH1I* errBitH_;
        TH1I* plAddH_;
        TH1I* evenChanOccuH_;
        TH1I* oddChanOccuH_;
        TProfile* hitVsVCTHH_;
        bool bookedHistos;
        uint32_t vcthSetting_;
    };

    struct FEUHistos
    {
        TH2I* hitCorrC0H_;
        TH2I* hitCorrC1H_;
        TH1D* deltaHitC0H;
        TH1D* deltaHitC1H;

        TH2I* sen0HitProfH_;
        TH2I* sen1HitProfH_;

        TH1I* sen0HitProfUnfoldedH_;
        TH1I* sen1HitProfUnfoldedH_;

        TH1I* sen0C0HitProfH_;
        TH1I* sen0C1HitProfH_;
        TH1I* sen1C0HitProfH_;
        TH1I* sen1C1HitProfH_;

        TH1I* L1CounterDiffH_;
        TH1I* PLAddPhaseDiffH_;
        TH2I* PLAddPhaseCorrH_;
        TH1I* rouErrorH_;

        TH1I* senCorrH_;
        TH1I* stubCountH_;
        TH1F* stubPositionH_;
        TH1I* stubBendH_;

        TProfile* stubVsHVH_;

        uint8_t nROUs_;
        uint8_t nColumns_;
        bool bookedHistos;
        uint32_t hvSetting_;
    };

    std::map <uint16_t, ROUHistos > rouHMap_;
    std::map <uint8_t, FEUHistos > feuHMap_;

    TH1I* l1AcceptH_;
    TH1I* totalNumberHitsH_;
    TH1I* totalNumberStubsH_;

    void fillFEUHistograms (FEUHistos& feu_h, std::vector<uint8_t>& even_list, std::vector<uint8_t>& odd_list,
                            std::vector<uint8_t>& errs, std::vector<uint16_t>& adds, std::vector<uint16_t>& counts);
    void fillROUHistograms (ROUHistos& rou_h,  uint8_t ch);
    void fillStubInformation (FEUHistos& feu_h, const std::vector<StubInfo>& stubs);
    void readConditionData (const DQMEvent* evt);
};
#ifdef __OTSDAQ__
}
#endif

#endif
