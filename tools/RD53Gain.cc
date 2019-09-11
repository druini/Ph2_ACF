/*!OA
  \file                  RD53Gain.cc
  \brief                 Implementaion of Gain scan
  \author                Mauro DINARDO
  \version               1.0
  \date                  28/06/18
  Support:               email to mauro.dinardo@cern.ch
*/

#include "RD53Gain.h"

Gain::Gain (const char* fileRes, size_t rowStart, size_t rowStop, size_t colStart, size_t colStop, size_t nEvents, size_t startValue, size_t stopValue, size_t nSteps, size_t offset)
  : Tool       ()
  , fileRes    (fileRes)
  , rowStart   (rowStart)
  , rowStop    (rowStop)
  , colStart   (colStart)
  , colStop    (colStop)
  , nEvents    (nEvents)
  , startValue (startValue)
  , stopValue  (stopValue)
  , nSteps     (nSteps)
  , offset     (offset)
  , histos     (nEvents, startValue-offset, stopValue-offset, nSteps)
{
  // ########################
  // # Custom channel group #
  // ########################
  ChannelGroup<RD53::nRows,RD53::nCols> customChannelGroup;
  customChannelGroup.disableAllChannels();

  for (auto row = rowStart; row <= rowStop; row++)
    for (auto col = colStart; col <= colStop; col++)
      customChannelGroup.enableChannel(row,col);
  
  theChnGroupHandler = std::make_shared<RD53ChannelGroupHandler>();
  theChnGroupHandler->setCustomChannelGroup(customChannelGroup);


  // ##############################
  // # Initialize dac scan values #
  // ##############################
  float step = (stopValue - startValue) / nSteps;
  for (auto i = 0u; i < nSteps; i++) dacList.push_back(startValue + step * i);
}

void Gain::run ()
{
  
  for (auto i = 0u; i < detectorContainerVector.size(); i++) delete detectorContainerVector[i];
  detectorContainerVector.clear();
  detectorContainerVector.reserve(dacList.size());
  for (auto i = 0u; i < dacList.size(); i++)
    {
      detectorContainerVector.emplace_back(new DetectorDataContainer());
      ContainerFactory::copyAndInitStructure<OccupancyAndPh>(*fDetectorContainer, *detectorContainerVector.back());
    }
  
  this->fChannelGroupHandler = theChnGroupHandler.get();
  this->SetTestPulse(true);
  this->fMaskChannelsFromOtherGroups = true;
  this->scanDac("VCAL_HIGH", dacList, nEvents, detectorContainerVector);


  // ################
  // # Error report #
  // ################
  this->chipErrorReport();
}

void Gain::draw (bool display, bool save)
{
  TApplication* myApp;

  if (display == true) myApp = new TApplication("myApp",nullptr,nullptr);
  if (save    == true)
    {
      this->CreateResultDirectory("Results",false,false);
      this->InitResultFile(fileRes);
    }

  this->initHisto();
  this->fillHisto();
  this->display();

  if (save    == true) this->WriteRootFile();
  if (display == true) myApp->Run(true);
}

std::shared_ptr<DetectorDataContainer> Gain::analyze ()
{
  double gain, gainErr, intercept, interceptErr;
  std::vector<float> x(dacList.size(),0);
  std::vector<float> y(dacList.size(),0);
  std::vector<float> e(dacList.size(),0);

  theGainAndInterceptContainer = std::shared_ptr<DetectorDataContainer>(new DetectorDataContainer());
  ContainerFactory::copyAndInitStructure<GainAndIntercept>(*fDetectorContainer, *theGainAndInterceptContainer);

  size_t index = 0;
  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  int VCalMED = static_cast<RD53*>(cChip)->getReg("VCAL_MED");

	  for (auto row = 0u; row < RD53::nRows; row++)
	    for (auto col = 0u; col < RD53::nCols; col++)
	      if (static_cast<RD53*>(cChip)->getChipOriginalMask()->isChannelEnabled(row,col) && this->fChannelGroupHandler->allChannelGroup()->isChannelEnabled(row,col))
		{
		  for (auto i = 0u; i < dacList.size()-1; i++)
		    {
		      x[i] = dacList[i]-VCalMED;
		      y[i] = detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPh;
		      e[i] = detectorContainerVector[i]->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<OccupancyAndPh>(row,col).fPhError;
		    }
		  
		  this->computeStats(x,y,e,gain,gainErr,intercept,interceptErr);
		  
		  if (gain != 0)
		    {
		      theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fGain           = gain;
		      theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fGainError      = gainErr;
		      theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fIntercept      = intercept;
		      theGainAndInterceptContainer->at(cBoard->getIndex())->at(cModule->getIndex())->at(cChip->getIndex())->getChannel<GainAndIntercept>(row,col).fInterceptError = interceptErr;
		    }
		}
	  
	  index++;
	}
  
  return theGainAndInterceptContainer;
}

void Gain::initHisto () { histos.book(fResultFile, *fDetectorContainer, fSettingsMap); }
void Gain::fillHisto ()
{
  for (auto i = 0u; i < dacList.size(); i++)
    histos.fillOccupancy(*detectorContainerVector[i], dacList[i]-offset);
  histos.fillGainIntercept(*theGainAndInterceptContainer);
}
void Gain::display   () { histos.process(); }

void Gain::computeStats (std::vector<float>& x, std::vector<float>& y, std::vector<float>& e, double& gain, double& gainErr, double& intercept, double& interceptErr)
// ##############################################
// # Linear regression with least-square method #
// # Model: y = f(x) = q + mx                   #
// # Measurements with uncertainty: Y = AX + E  #
// ##############################################
// # A = (XtX)^(-1)XtY                          #
// # X = | 1 x1 |                               #
// #     | 1 x2 |                               #
// #     ...                                    #
// # A = | q |                                  #
// #     | m |                                  #
// ##############################################
{
  float a  = 0, b  = 0, c  = 0, d  = 0;
  float ai = 0, bi = 0, ci = 0, di = 0;
  float it = 0;
  float det;

  intercept    = 0;
  gain         = 0;
  interceptErr = 0;
  gainErr      = 0;


  // #######
  // # XtX #
  // #######
  for (auto i = 0u; i < x.size(); i++)
    if (e[i] != 0)
      {
	b += x[i];
	d += x[i] * x[i];
	it++;
      }
  a = it;
  c = b;


  // ##############
  // # (XtX)^(-1) #
  // ##############
  det = a*d - b*c;
  if (det != 0)
    {
      ai =  d/det;
      bi = -b/det;
      ci = -c/det;
      di =  a/det;
    }


  // #################
  // # (XtX)^(-1)XtY #
  // #################
  for (auto i = 0u; i < x.size(); i++)
    if (e[i] != 0)
      {
	intercept    += (ai + bi*x[i]) * y[i];
	gain         += (ci + di*x[i]) * y[i];
	
	interceptErr += (ai + bi*x[i])*(ai + bi*x[i]) * e[i]*e[i];
	gainErr      += (ci + di*x[i])*(ci + di*x[i]) * e[i]*e[i];
      }
  
  interceptErr = sqrt(interceptErr);
  gainErr      = sqrt(gainErr);
}

void Gain::chipErrorReport ()
{
  auto RD53ChipInterface = static_cast<RD53Interface*>(this->fReadoutChipInterface);

  for (const auto cBoard : *fDetectorContainer)
    for (const auto cModule : *cBoard)
      for (const auto cChip : *cModule)
	{
	  LOG (INFO) << BOLDGREEN << "\t--> Readout chip error report for [board/module/chip = " << BOLDYELLOW << cBoard->getId() << "/" << cModule->getId() << "/" << cChip->getId() << BOLDGREEN << "]" << RESET;
	  LOG (INFO) << BOLDBLUE << "LOCKLOSS_CNT    = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "LOCKLOSS_CNT")    << RESET;
	  LOG (INFO) << BOLDBLUE << "BITFLIP_WNG_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_WNG_CNT") << RESET;
	  LOG (INFO) << BOLDBLUE << "BITFLIP_ERR_CNT = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "BITFLIP_ERR_CNT") << RESET;
	  LOG (INFO) << BOLDBLUE << "CMDERR_CNT      = " << BOLDYELLOW << RD53ChipInterface->ReadChipReg (static_cast<RD53*>(cChip), "CMDERR_CNT")      << RESET;
	}
}
