/*!
        \file                DQMHistogramSSAScurveAsync.h
        \brief               DQM class for Calibration example -> use it as a templare
        \author              Fabio Ravera
        \date                25/7/19
        Support :            mail to : fabio.ravera@cern.ch
*/

#ifndef __DQMHISTOGRAMSSAScurveAsync_H__
#define __DQMHISTOGRAMSSAScurveAsync_H__
#include "../DQMUtils/DQMHistogramBase.h"
#include "../Utils/Container.h"
#include "../Utils/DataContainer.h"

class TFile;

/*!
 * \class DQMHistogramSSAScurveAsync
 * \brief Class for SSAScurveAsync monitoring histograms
 */
class DQMHistogramSSAScurveAsync : public DQMHistogramBase
{

  public:
    /*!
     * constructor
     */
    DQMHistogramSSAScurveAsync ();

    /*!
     * destructor
     */
    ~DQMHistogramSSAScurveAsync();

    /*!
     * \brief Book histograms
     * \param theOutputFile : where histograms will be saved
     * \param theDetectorStructure : Detector container as obtained after file parsing, used to create histograms for all board/chip/module/channel
     * \param pSettingsMap : setting as for Tool setting map in case coe informations are needed (i.e. FitSCurve)
     */
    void book(TFile *theOutputFile, const DetectorContainer &theDetectorStructure, const Ph2_System::SettingsMap& pSettingsMap) override;

    /*!
     * \brief fill : fill histograms from TCP stream, need to be overwritten to avoid compilation errors, but it is not needed if you do not fo into the SoC
     * \param dataBuffer : vector of char with the TCP datastream
     */
    bool fill (std::vector<char>& dataBuffer) override;

    /*!
     * \brief process : do something with the histogram like colors, fit, drawing canvases, etc
     */
    void process () override;

    /*!
     * \brief Reset histogram
     */
    void reset(void) override;
   
    /*!
     * \brief fillSSAScurveAsyncPlots
     * \param theHitContainer : Container with the hits you want to plot 
     */
    void fillSSAScurveAsyncPlots(DetectorDataContainer &theHitContainer,uint32_t thresh);

  private:

    DetectorDataContainer fDetectorHitHistograms;
    DetectorDataContainer fDetectorData;
};
#endif
