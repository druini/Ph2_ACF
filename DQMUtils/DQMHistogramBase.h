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
     * Book histograms
     */
    virtual void bookHistos(void) = 0;

    /*!
     * Fill histogram
     */
    virtual void fillHistos (void) = 0;
    virtual void saveHistos (const std::string& outFile) = 0;
    virtual void resetHistos(void) = 0;
    //virtual void summarizeHistos();
};
#endif
