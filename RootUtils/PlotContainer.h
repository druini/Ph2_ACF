/*

        \file                          PlotContainer.h
        \brief                         Generic PlotContainer for DQM
        \author                        Fabio Ravera, Lorenzo Uplegger
        \version                       1.0
        \date                          18/06/19
        Support :                      mail to : fabio.ravera@cern.ch

 */

#ifndef __PLOT_CONTAINER_H__
#define __PLOT_CONTAINER_H__

#include <iostream>

class PlotContainer //: public streammable
{
public:
    PlotContainer() {;}
    PlotContainer(const PlotContainer& container) = default;
    PlotContainer& operator= (const PlotContainer& container) = default;
    
    virtual ~PlotContainer() {;}

    virtual void setNameTitle(std::string histogramName, std::string histogramTitle) = 0;
    virtual std::string getName() const = 0;

};

#endif
