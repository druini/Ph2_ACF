/*!
*
* \file ShortFinder.h
* \brief Short Finder  class - converted into a tool based on T.Gadesk's algorithm (FindShorts in HybridTester.cc)
* \author Sarah SEIF EL NASR_STOREY
* \date 20 / 10 / 16
*
* \Support : sarah.storey@cern.ch
*
*/

#ifndef ShortFinder_h__
#define ShortFinder_h__

#include "Tool.h"
#include "Channel.h"
#include "../Utils/Visitor.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/Utilities.h"


#include <map>
#include "TH2.h"
#include "TCanvas.h"
#include "TProfile.h"
#include "TString.h"
#include "TGraphErrors.h"
#include "TString.h"
#include "TText.h"
#include "TStyle.h"
#include "TRandom.h"
#include "TMath.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;


// Typedefs for Containers
typedef std::array<int, 2> Short;
typedef std::vector<Short> ShortsList;
typedef std::array<int, 5> ShortedChannel;
typedef std::vector<ShortedChannel> ShortedGroup ;
typedef std::array<ShortedGroup, 8> ShortedGroupsList ;



class ShortFinder : public Tool
{
  public:
    ShortFinder()
    {
        fNShorts = 0;
        fNShortsTop = 0 ;
        fNShortsBottom = 0;
        fTotalEvents = 0;
        fNCbc = 2;
    };

    // D'tor
    ~ShortFinder() {};

    ///Reload CBC registers from file found in results (fDirectoryName) directory .
    //If no directory is found use the default files for the different operational modes found in Ph2_ACF/settings
    void ReconfigureRegisters();
    // configure only the Vcth value
    void ConfigureVcth ( uint16_t pVcth = 0x78 );


    void Initialize();
    void FindShorts (std::ostream& os = std::cout );
    void SaveResults();
    uint32_t GetNShorts()
    {
        return fNShorts;
    };
    void GetPercentageShorts()
    {
        100 * double (fNShorts) / ( fNCbc / 2 * 254 );
    };
    void GetPercentateShorts_TopPad()
    {
        100 * double (fNShortsBottom) / ( fNCbc / 2 * 254 );
    };
    void GetPercentateShorts_BottomPad()
    {
        100 * double (fNShortsBottom) / ( fNCbc / 2 * 254 );
    };

  private :
    // Canvases
    TCanvas* fDataCanvas;   /*!<Canvas to output single-strip efficiency */
    TCanvas* fShortsCanvas;  /*!<Canvas to output shorts on module*/

    // Histograms
    TH1F* fHistTop;   /*!< Histogram for top pads */
    TH1F* fHistBottom;   /*!< Histogram for bottom pads */
    TH1F* fHistTopMerged;   /*!< Histogram for top pads used for segmented antenna testing routine*/
    TH1F* fHistBottomMerged;   /*!< Histogram for bottom pads used for segmented antenna testing routine*/

    TH2F* fHistShortBackground;
    TH1F* fHistShortsTop;
    TH1F* fHistShortsBottom;

    // Containers
    Short fShort;
    ShortsList fShortsList;

    // Counters
    uint32_t fNShorts;
    uint32_t fNShortsTop;
    uint32_t fNShortsBottom;
    uint32_t fTotalEvents;
    uint32_t fNCbc;

    // booleans
    bool fHoleMode;


    // functions/methods
    void SetBeBoard (BeBoard* pBoard);
    bool CheckChannel (Short pShort , ShortsList pShortsList);
    void MergeShorts (ShortsList pShortA);
    void ReconstructShorts (ShortedGroupsList pShortedGroupsArray , std::ostream& os = std::cout );

    ShortsList MergeShorts (ShortsList pShortA, ShortsList pShortB);
    bool CheckChannelInShortPresence ( Short pShortedChannel, ShortsList pShort);
    bool CheckShortsConnection (ShortsList pShortA, ShortsList pShortB);

    /*!
    * \brief private method to periodically update the output graphs
    */
    void UpdateHists();
    /*!
    * \brief private method to update histograms after short scan is completed
    */
    void UpdateHistsMerged();
    /*!
    * \brief private method that calls the constructors for the histograms
    */
    void InitializeHists();
    /*!
    * \brief private method that calls reads the settings from the settings map in private member variables
    */
    void InitialiseSettings();

    /*!
    * \brief private method that writes all histograms/graphs/canvases to the results file opened by the "parent" tool
    */
    void writeGraphs();



};
#endif
