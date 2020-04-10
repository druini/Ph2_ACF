/*!
  \file                  SSASCurveAsync.cc
  \brief                 Implementaion of SCurve scan
  \author                kevin Nash
  \version               1.0
  \date                  28/06/18
  Support:               email to knash@gmail.com
*/

#include "SSASCurveAsync.h"

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;

SSASCurve::SSASCurve() :
    Tool()
{
}


SSASCurve::~SSASCurve() 
{
}


void SSASCurve::Initialise(void)
{
  StartTHDAC        = this->findValueInSettings("StartTHDAC");
  StopTHDAC        = this->findValueInSettings("StopTHDAC");
  NMsec        = this->findValueInSettings("NMsec");
  Res        = this->findValueInSettings("Res");
  Nlvl        = this->findValueInSettings("Nlvl");
    #ifdef __USE_ROOT__  
        fDQMHistogramSSASCurveAsync.book(fResultFile, *fDetectorContainer, fSettingsMap);
    #endif    
}

void SSASCurve::run(void)
{
    DetectorDataContainer       theHitContainer;
    ContainerFactory::copyAndInitChannel<std::array<uint32_t,3>>(*fDetectorContainer, theHitContainer);
    for (auto cBoard : theHitContainer)
    {
        BeBoard* theBeBoard = static_cast<BeBoard*>( fDetectorContainer->at(cBoard->getIndex()) );
        theBeBoard->setEventType(EventType::SSAAS);
        float rms=999.0;
        while(rms>0.001)
        {
        LOG (INFO) << BOLDBLUE <<"Running Scurve..."<< RESET;  
        float mean=0.0;
        float Nstrip=0.0;
        for (size_t thd = StartTHDAC; thd<=StopTHDAC; thd++)
        {
            Nstrip=0.0;

            //if (thd%RES!=0)continue;
            for(auto cBoard : *fDetectorContainer)
            {
                for(auto cOpticalGroup : *cBoard)
                {
                    for(auto cHybrid : *cOpticalGroup)
                    {
                        for(auto cChip : *cHybrid)
                        {
                            this->fReadoutChipInterface->WriteChipReg(static_cast<ReadoutChip*>(cChip), "Bias_THDAC", thd);
                        }
                    }
                }
            }   

            static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PS_Clear_counters();
            ReadASEvent( theBeBoard, NMsec ); 
            const std::vector<Event*> &eventVector = GetEvents ( theBeBoard );
            for(auto opticalGroup: *cBoard)
            {
                for(auto hybrid: *opticalGroup) // for on hybrid - begin 
                {
                    for(auto cSSA: *hybrid) // for on chip - begin 
                    {

                        std::vector<uint32_t> hits= eventVector.at(0)->GetHits(hybrid->getId(), cSSA->getId());

                        unsigned int channelNumber = 0;
                        unsigned int curh = 0;
                        for(auto &channel : *cSSA->getChannelContainer<std::array<uint32_t,3>>()) 
                        {
                            Nstrip+=1.0;
                            curh=hits[channelNumber];


                            //if ( channelNumber==1)
                            //LOG (INFO) << BOLDRED <<curh<< RESET;
                            if ((channel[1]>Nlvl) && (curh<channel[1]/2) && (channel[0]>channel[1]/2))
                                {
                                channel[2]=std::roundf(float(thd)*float(curh) + float(thd-1)*float(channel[0]))/(float(curh) +float(channel[0]));
                                globalmax=std::max(globalmax,channel[2]);
                                mean+=channel[2];
                                //if ( channelNumber==1)
                                  //  LOG (INFO) << BOLDRED << "halfmax "<<curh<<","<<channel[0]<<","<<channel[1]<< RESET;    
                                  //  LOG (INFO) << BOLDRED << normvals[channelNumber]<<" "<<thd<< RESET;   
                                }
                            channel[1] =std::max(channel[1],curh);
                            channel[0] = curh;
                            channelNumber++;
                        } 
                    }
                }
            } 
        #ifdef __USE_ROOT__
                fDQMHistogramSSASCurveAsync.fillSSAScurveAsyncPlots(theHitContainer,thd);
        #endif  
        //
 
        }
        rms=0.0;
        mean/=Nstrip;

        float writeave=0.0;
        for(auto opticalGroup: *cBoard)
            {
                for(auto hybrid: *opticalGroup) // for on hybrid - begin 
                {
                    for(auto cSSA: *hybrid) // for on chip - begin 
                    {

                        uint32_t  istrip=1;
                        for(auto &channel : *cSSA->getChannelContainer<std::array<uint32_t,3>>()) // for on channel - begin 
                        {
                            ReadoutChip *theChip = static_cast<ReadoutChip*>(fDetectorContainer->at(cBoard ->getIndex())->at(opticalGroup ->getIndex())->at(hybrid->getIndex())->at(cSSA->getIndex()));

                            rms+=(channel[2]-mean)*(channel[2]-mean);
                            int32_t cr=fReadoutChipInterface->ReadChipReg(theChip, "THTRIMMING_S" + std::to_string(istrip));
                            uint32_t THtowrite=uint32_t(std::roundf(float(cr)-1.0*(float(channel[2])-mean)));
                            THtowrite=std::min(THtowrite,uint32_t(31));
                            THtowrite=std::max(THtowrite,uint32_t(0));
                            writeave+=THtowrite;
                            if (THtowrite==0 || THtowrite==31)LOG (INFO) << BOLDRED <<"Warning, thdac at limit "<<istrip<<","<<THtowrite<< RESET;  

                            fReadoutChipInterface->WriteChipReg(theChip, "THTRIMMING_S" + std::to_string(istrip), THtowrite);
                            istrip+=1;
                        }
                    }
                }
            }
        LOG (INFO) << BOLDBLUE <<"Done"<< RESET;  
        rms/=float(Nstrip);
        LOG (INFO) << BOLDRED <<"RMS: "<<std::sqrt(rms)<< RESET;  
        LOG (INFO) << BOLDRED <<"MEAN: "<<mean<< RESET; 
        LOG (INFO) << BOLDRED <<"WRITEMEAN: "<<writeave/float(Nstrip)<< RESET; 
        LOG (INFO) << BOLDRED <<"MAX: "<<globalmax<< RESET; 
        LOG (INFO) << BOLDRED <<"NSTRIP: "<<Nstrip<< RESET; 
        //break;
        }
    }
}
void SSASCurve::writeObjects(void)
{
    #ifdef __USE_ROOT__
        fDQMHistogramSSASCurveAsync.process();
    #endif
    SaveResults();
    closeFileHandler();
}


