/*!
  \file                  CanvasContainer.h
  \brief                 Header file of histogram container
  \author                Alkiviadis PAPADOPOULOS
  \version               1.0
  \date                  28/06/18
  Support:               email to alkiviadis.papadopoulos@cern.ch
  Support:               email to mauro.dinardo@cern.ch
*/

#ifndef CanvasContainer_H
#define CanvasContainer_H

#include <iostream>
#include "../Utils/Container.h"
#include "../RootUtils/PlotContainer.h"
#include <TCanvas.h>


class CanvasContainerBase : public PlotContainer {
protected:
    static int& canvasId() {
      static int value = 0;
      return value;
    };
};

template <class Hist>
class CanvasContainer : public CanvasContainerBase
{
 public:
 CanvasContainer() : fTheHistogram(nullptr) {}

  CanvasContainer (const CanvasContainer<Hist>& container) = delete;
  CanvasContainer<Hist>& operator= (const CanvasContainer<Hist>& container) = delete;

  template <class... Args>
    CanvasContainer (Args... args)
    {
      fTheHistogram = new Hist(args...);
      fTheHistogram->SetDirectory(0);
    }

  ~CanvasContainer() 
    {
      if (fHasToBeDeletedManually) {
          delete fTheHistogram;
          if (fCanvas)
            delete fCanvas;
      }
      fTheHistogram = nullptr;
      
    }

  CanvasContainer (CanvasContainer<Hist>&& container)
    {
      fHasToBeDeletedManually = container.fHasToBeDeletedManually;
      fTheHistogram = container.fTheHistogram;
      container.fTheHistogram = nullptr;
      fCanvas = container.fCanvas;
      container.fCanvas = nullptr;
    }

  CanvasContainer<Hist>& operator= (CanvasContainer<Hist>&& container)
    {
      fHasToBeDeletedManually = container.fHasToBeDeletedManually;
      fTheHistogram = container.fTheHistogram;
      container.fTheHistogram = nullptr;
      fCanvas = container.fCanvas;
      container.fCanvas = nullptr;
      return *this;
    }

  void initialize (std::string name, std::string title, const PlotContainer* reference) override
  {
    fHasToBeDeletedManually = false;
    
    fCanvas = new TCanvas(name.data(), title.data());

    fTheHistogram = new Hist(*(static_cast<const CanvasContainer<Hist>*>(reference)->fTheHistogram));
    fTheHistogram->SetName(name.data());
    fTheHistogram->SetTitle(title.data());
    fTheHistogram->SetDirectory(0);

    gDirectory->Append(fCanvas);
  }

  void print (void)
  { 
    std::cout << "CanvasContainer " << fTheHistogram->GetName() << std::endl;
  }

  template<typename T>
    void makeChannelAverage (const ChipContainer* theChipContainer, const ChannelGroupBase* chipOriginalMask, const ChannelGroupBase* cTestChannelGroup, const uint32_t numberOfEvents) {}

    void makeSummaryAverage (const std::vector<CanvasContainer<Hist>>* theTH1FContainerVector, const std::vector<uint32_t>& theNumberOfEnabledChannelsList, const uint32_t numberOfEvents) {}

  void normalize (const uint32_t numberOfEvents) {}

  void setNameTitle (std::string histogramName, std::string histogramTitle) override 
  {
    fTheHistogram->SetNameTitle(histogramName.data(), histogramTitle.data());
  }
  
  std::string getName() const override
    {
      return fTheHistogram->GetName();
    }

  std::string getTitle() const override
    {
      return fTheHistogram->GetTitle();
    }
  
  Hist* fTheHistogram;
  TCanvas* fCanvas = nullptr;
};

#endif
