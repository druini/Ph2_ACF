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
    ContainerFactory::copyAndInitChannel<std::pair<std::array<uint32_t,2>,float>>(*fDetectorContainer, theHitContainer);



    for (auto cBoard : theHitContainer)
    {
        BeBoard* theBeBoard = static_cast<BeBoard*>( fDetectorContainer->at(cBoard->getIndex()) );
        theBeBoard->setEventType(EventType::SSAAS);
        float rms=999.0;
        //float prevrms=999.0;

        //float vfac=1.4;//
        float vfac=1.0;


        //std::vector<float> prev;
        //std::vector<float> prevgoal;
        while(rms>0.1)
        {
        //float rmsfrac=rms/prevrms;
        //prevrms=rms;
        LOG (INFO) << BOLDBLUE <<"Running Scurve..."<< RESET;  

        //std::vector<float> cur;
        //std::vector<float> goal;
        float mean=0.0;
        float Nstrip=0.0;

       
        /*for(auto cBoard : *fDetectorContainer)
            {
                for(auto cOpticalGroup : *cBoard)
                {
                    for(auto cHybrid : *cOpticalGroup)
                    {
                        for(auto cChip : *cHybrid)
                        {
                            this->fReadoutChipInterface->enableInjection(static_cast<ReadoutChip*>(cChip), 1);
                            this->fReadoutChipInterface->setInjectionAmplitude(static_cast<ReadoutChip*>(cChip), 40);
                        }
                    }
                }
        }   */


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
                        for(auto &channel : *cSSA->getChannelContainer<std::pair<std::array<uint32_t,2>,float>>()) 
                        {
                            Nstrip+=1.0;
                            curh=hits[channelNumber];


                            //if ( channelNumber==1)
                            //LOG (INFO) << BOLDRED <<thd<<": "<<curh<< RESET;
                            if ((channel.first[1]>Nlvl) && (curh<channel.first[1]/2) && (channel.first[0]>channel.first[1]/2))
                                {

                                channel.second=(float(thd)*float(curh) + float(thd-1)*float(channel.first[0]))/(float(curh) +float(channel.first[0]));
                                globalmax=std::max(globalmax,channel.second);
                                mean+=channel.second;
                                //if ( channelNumber==1)
                                  //  LOG (INFO) << BOLDRED << "halfmax "<<curh<<","<<channel.first[0]<<","<<channel.first[1]<< RESET;    
                                  //  LOG (INFO) << BOLDRED << normvals[channelNumber]<<" "<<thd<< RESET;   
                                }
                            channel.first[1] =std::max(channel.first[1],curh);
                            channel.first[0] = curh;
                            channelNumber++;
                        } 
                    }
                }
            } 
        #ifdef __USE_ROOT__
                fDQMHistogramSSASCurveAsync.fillSSASCurveAsyncPlots(theHitContainer,thd);
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
                        for(auto &channel : *cSSA->getChannelContainer<std::pair<std::array<uint32_t,2>,float>>()) // for on channel - begin 
                        {
                            ReadoutChip *theChip = static_cast<ReadoutChip*>(fDetectorContainer->at(cBoard ->getIndex())->at(opticalGroup ->getIndex())->at(hybrid->getIndex())->at(cSSA->getIndex()));

                            rms+=(channel.second-mean)*(channel.second-mean);
                            int32_t cr=fReadoutChipInterface->ReadChipReg(theChip, "THTRIMMING_S" + std::to_string(istrip));
                            float floatTh=float(cr)-vfac*(float(channel.second)-mean);
                            uint32_t THtowrite=uint32_t(std::roundf(floatTh));
                            //cur.push_back(channel.second);
                            //goal.push_back(mean);
                            //uint32_t ind= cur.size()-1;
                            //if(prev.size()>0 and prevgoal.size()>0) 
                                //if (cur[ind]!=std::roundf(prevgoal[ind]))
                                    //std::cout<<prev[ind]<<" "<<cur[ind]<<" g "<<std::roundf(prevgoal[ind])<<std::endl;


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
        //prev = cur;
        //prevgoal=goal;
        LOG (INFO) << BOLDBLUE <<"Done"<< RESET;  
        rms/=float(Nstrip);
        rms = std::sqrt(rms);
        LOG (INFO) << BOLDRED <<"RMS: "<<rms<< RESET;  
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


