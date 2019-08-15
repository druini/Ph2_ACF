/*!
        \file                DQMHistogramCalibration.h
        \brief               base class to create and fill monitoring histograms
        \author              Fabio Ravera, Lorenzo Uplegger
        \version             1.0
        \date                6/5/19
        Support :            mail to : fabio.ravera@cern.ch
*/

#ifndef __DQMHISTOGRAMCALIBRATION_H__
#define __DQMHISTOGRAMCALIBRATION_H__
#include "../DQMUtils/DQMHistogramBase.h"
#include "../Utils/Container.h"
#include "../Utils/DataContainer.h"

class TFile;

/*!
 * \class DQMHistogramCalibration
 * \brief Class for PedeNoise monitoring histograms
 */
class DQMHistogramCalibration : public DQMHistogramBase
{

  public:
    /*!
     * constructor
     */
    DQMHistogramCalibration ();

    /*!
     * destructor
     */
    ~DQMHistogramCalibration();

    /*!
     * Book histograms
     */
    void book(TFile *theOutputFile, const DetectorContainer &theDetectorStructure, std::map<std::string, double> pSettingsMap) override;

    /*!
     * Fill histogram
     */
    bool fill (std::vector<char>& dataBuffer) override;

    /*!
     * Save histogram
     */
    void process () override;

    /*!
     * Reset histogram
     */
    void reset(void) override;
    //virtual void summarizeHistos();

  private:
    DetectorDataContainer fDetectorData;

};
#endif
