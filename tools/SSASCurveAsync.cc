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
	NMpulse        = this->findValueInSettings("NMpulse");
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
        float vfac=0.8;


	this->enableTestPulse( true );
	static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureTriggerFSM( NMpulse, 10000, 6, 0, 0);


	float wroffset=0.0;
	float Nunt=0.0;

	while(rms>0.7)
	{

        	LOG (INFO) << BOLDBLUE <<"Running Scurve..."<< RESET;

        	//std::vector<float> cur;
        	//std::vector<float> goal;
        	float mean=0.0;
        	float Nstrip=0.0;
		float Nmeans=0.0;




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

			LOG (INFO) << BOLDRED <<"THDAC "<<thd<< RESET;

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

		        //setFWTestPulse();
		        for ( auto cBoard : *fDetectorContainer )
		        {
		        	setSameDacBeBoard(static_cast<BeBoard*>(cBoard), "Bias_CALDAC", 50);
		        }

			ReadASEvent( theBeBoard, NMsec,NMpulse );
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
		                            		if ((channel.first[1]>Nlvl) && (curh<channel.first[1]/2) && (channel.first[0]>channel.first[1]/2))
		                                {

                                		channel.second=(float(thd)*float(curh) + float(thd-1)*float(channel.first[0]))/(float(curh) +float(channel.first[0]));
                                		globalmax=std::max(globalmax,channel.second);

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


	for(auto opticalGroup: *cBoard)
	{
		for(auto hybrid: *opticalGroup) // for on hybrid - begin
		{
			for(auto cSSA: *hybrid) // for on chip - begin
			{
				for(auto &channel : *cSSA->getChannelContainer<std::pair<std::array<uint32_t,2>,float>>())
				{
				mean+=channel.second;
				Nmeans+=1.0;
				}
			}
		}
	}

        mean/=Nmeans;



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
					float floatTh=cr;
					if(channel.second>1.0)
                            			floatTh=float(cr)-vfac*(float(channel.second)-mean)+wroffset;


                            		uint32_t THtowrite=uint32_t(std::roundf(floatTh));

					THtowrite=std::min(THtowrite,uint32_t(31));
                            		THtowrite=std::max(THtowrite,uint32_t(0));
                            		writeave+=THtowrite;
					if (THtowrite==0 || THtowrite==31) LOG (INFO) << BOLDRED <<"Warning, thdac at limit "<<istrip<<","<<THtowrite<< RESET;
                            		if (THtowrite==0) Nunt+=1.0;
                            		if (THtowrite==31) Nunt-=1.0;

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
		wroffset=15.0-writeave/float(Nstrip)+Nunt;
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
