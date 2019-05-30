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

/*!
 * \class DQMHistogramPedeNoise
 * \brief Base class for monitoring histograms
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
    void book(std::string configurationFileName);

    /*!
     * Fill histogram
     */
    void fill (std::vector<char>& dataBuffer);
    void save (const std::string& outFile);
    void reset(void);
    //virtual void summarizeHistos();
  private:
    DetectorContainer fDetectorData;
    DetectorContainer fDetectorStructure;
    DetectorContainer fDetectorValidationHistograms;
    DetectorContainer fDetectorPedestalHistograms;
    DetectorContainer fDetectorNoiseHistograms;
};
#endif
