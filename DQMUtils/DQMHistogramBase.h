/*!
        \file                DQMHistogramBase.h
        \brief               base class to create and fill monitoring histograms
        \author              Fabio Ravera, Lorenzo Uplegger
        \version             1.0
        \date                6/5/19
        Support :            mail to : fabio.ravera@cern.ch

*/

#ifndef __DQMHISTOGRAMBASE_H__
#define __DQMHISTOGRAMBASE_H__

#include <string>
#include <vector>
#include <../Utils/Container.h>

class DetectorContainer;

/*!
 * \class DQMHistogramBase
 * \brief Base class for monitoring histograms
 */
class DQMHistogramBase
{

  public:
    /*!
     * constructor
     */
    DQMHistogramBase (){;}

    /*!
     * destructor
     */
    virtual ~DQMHistogramBase(){;}

    /*!
     * \brief Book histograms
     * \param theDetectorStructure : Container of the Detector structure
     */
    virtual void book(DetectorContainer &theDetectorStructure) = 0;

    /*!
     * \brief Book histograms
     * \param configurationFileName : xml configuration file
     */
    virtual void fill (std::vector<char>& dataBuffer) = 0;
    
    /*!
     * \brief SAve histograms
     * \param outFile : ouput file name
     */
    virtual void save (const std::string& outFile) = 0;
    
    /*!
     * \brief Book histograms
     * \param configurationFileName : xml configuration file
     */
    virtual void reset(void) = 0;
    //virtual void summarizeHistos();

protected:
    DetectorContainer fDetectorStructure;
    
};
#endif
