/*!
        \file                DQMHistogramPedeNoise.h
        \brief               base class to create and fill monitoring histograms
        \author              Fabio Ravera, Lorenzo Uplegger
        \version             1.0
        \date                6/5/19
        Support :            mail to : fabio.ravera@cern.ch
*/

#ifndef __DQMHISTOGRAMPEDENOISE_H__
#define __DQMHISTOGRAMPEDENOISE_H__
#include "../DQMUtils/DQMHistogramBase.h"
#include "../Utils/Container.h"
#include "../Utils/DataContainer.h"

/*!
 * \class DQMHistogramPedeNoise
 * \brief Class for PedeNoise monitoring histograms
 */
class DQMHistogramPedeNoise : public DQMHistogramBase
{

  public:
    /*!
     * constructor
     */
    DQMHistogramPedeNoise ();

    /*!
     * destructor
     */
    ~DQMHistogramPedeNoise();

    /*!
     * Book histograms
     */
    void book(DetectorContainer &theDetectorStructure) override;

    /*!
     * Fill histogram
     */
    void fill (std::vector<char>& dataBuffer) override;

    /*!
     * Save histogram
     */
    void save (const std::string& outFile) override;

    /*!
     * Reset histogram
     */
    void reset(void) override;
    //virtual void summarizeHistos();

    /*!
     * \brief Fill validation histograms
     * \param theOccupancy : DataContainer for the occupancy
     */
    void fillValidationPlots(DetectorDataContainer &theOccupancy);

    /*!
     * \brief Fill validation histograms
     * \param theOccupancy : DataContainer for pedestal and occupancy
     */
    void fillPedestalAndNoisePlots(DetectorDataContainer &thePedestalAndNoise);


  private:
    DetectorDataContainer fDetectorData;
    DetectorDataContainer fDetectorValidationHistograms;
    DetectorDataContainer fDetectorPedestalHistograms;
    DetectorDataContainer fDetectorNoiseHistograms;
};
#endif
