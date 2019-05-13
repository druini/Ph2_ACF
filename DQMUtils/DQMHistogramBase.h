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
    virtual void book(std::string configurationFileName) = 0;

    /*!
     * Fill histogram
     */
    virtual void fill (std::vector<char>& dataBuffer) = 0;
    virtual void save (const std::string& outFile) = 0;
    virtual void reset(void) = 0;
    //virtual void summarizeHistos();
};
#endif
