/*!
  \file                  SSASCurveAsync.cc
  \brief                 Implementaion of SCurve scan
  \author                kevin Nash
  \version               1.0
  \date                  28/06/18
  Support:               email to knash@gmail.com
*/

#include "SSASCurveAsync.h"
#include "../Utils/Occupancy.h"

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
	Mrms        = this->findValueInSettings("Mrms");
	Vfac        = this->findValueInSettings("Vfac");
	SyncDebug        = this->findValueInSettings("SyncDebug");

	TestPulsePotentiometer        = this->findValueInSettings("TestPulsePotentiometer");
	#ifdef __USE_ROOT__
		fDQMHistogramSSASCurveAsync.book(fResultFile, *fDetectorContainer, fSettingsMap);
	#endif
}

void SSASCurve::run(void)
{
  	ReadoutChip* cFirstReadoutChip = static_cast<ReadoutChip*>(fDetectorContainer->at(0)->at(0)->at(0)->at(0));

  	cWithSSA = (cFirstReadoutChip->getFrontEndType() == FrontEndType::SSA);
  	cWithMPA = (cFirstReadoutChip->getFrontEndType() == FrontEndType::MPA);

	DetectorDataContainer       theHitContainer;
	ContainerFactory::copyAndInitChannel<std::pair<std::array<uint32_t,2>,float>>(*fDetectorContainer, theHitContainer);

	for (auto cBoard : theHitContainer)
	{
	BeBoard* theBeBoard = static_cast<BeBoard*>( fDetectorContainer->at(cBoard->getIndex()) );

	if (cWithSSA)
		{
		if (SyncDebug) LOG (INFO) << BOLDBLUE <<"SYNC DEBUG!"<<RESET;
		else theBeBoard->setEventType(EventType::SSAAS);
		}
	if (cWithMPA)
		{
		if (SyncDebug) LOG (INFO) << BOLDBLUE <<"SYNC DEBUG!"<<RESET;
		else theBeBoard->setEventType(EventType::MPAAS);
		}


        float rms=999.0;
        //float prevrms=999.0;

        //float vfac=1.4;//
        float vfac=Vfac;
	LOG (INFO) << BOLDBLUE <<"VFAC "<<vfac<< RESET;
	LOG (INFO) << BOLDBLUE <<"Mrms "<<Mrms<< RESET;



	this->enableTestPulse( true );
	setFWTestPulse();

        static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureTriggerFSM( NMpulse, 10000, 6, 0, 0);
	static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->ConfigureTestPulseFSM(50,200,1000,0,1,1);


	float wroffset=0.0;
	float Nuntoff=0.0;

	while(rms>Mrms)
	{
		float Nunt=0.0;
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
		bool runpulse=false;

		if (NMpulse>0) runpulse=true;
		if (cWithSSA)
			{
			std::cout << "withSSA" << std::endl;
			if (runpulse) setSameDacBeBoard(theBeBoard, "Bias_CALDAC", TestPulsePotentiometer);
			}
		if (cWithMPA)
			{
			std::cout << "withMPA" << std::endl;
			if (runpulse) 
				{
				setSameDacBeBoard(theBeBoard, "CalDAC0", TestPulsePotentiometer);
				setSameDacBeBoard(theBeBoard, "CalDAC1", TestPulsePotentiometer);
				setSameDacBeBoard(theBeBoard, "CalDAC2", TestPulsePotentiometer);
				setSameDacBeBoard(theBeBoard, "CalDAC3", TestPulsePotentiometer);
				setSameDacBeBoard(theBeBoard, "CalDAC4", TestPulsePotentiometer);
				setSameDacBeBoard(theBeBoard, "CalDAC5", TestPulsePotentiometer);
				setSameDacBeBoard(theBeBoard, "CalDAC6", TestPulsePotentiometer);
				}
			}
			
        	for (size_t thd = StartTHDAC; thd<=StopTHDAC; thd++)
        	{
            		Nstrip=0.0;

			LOG (INFO) << BOLDRED <<"THDAC "<<thd<< RESET;

			//if (thd%RES!=0)continue;

			for(auto cOpticalGroup : *cBoard)
			{
		                    for(auto cHybrid : *cOpticalGroup)
		                    {
		                        for(auto cChip : *cHybrid)
		                        {
						ReadoutChip *theChip = static_cast<ReadoutChip*>(fDetectorContainer->at(cBoard ->getIndex())->at(cOpticalGroup ->getIndex())->at(cHybrid->getIndex())->at(cChip->getIndex()));
						if (cWithSSA)
						{
		                            		this->fReadoutChipInterface->WriteChipReg(theChip, "Bias_THDAC", thd);
						}
						if (cWithMPA)
						{
		                            		this->fReadoutChipInterface->WriteChipReg(theChip, "ThDAC0", thd);
		                            		this->fReadoutChipInterface->WriteChipReg(theChip, "ThDAC1", thd);
		                            		this->fReadoutChipInterface->WriteChipReg(theChip, "ThDAC2", thd);
		                            		this->fReadoutChipInterface->WriteChipReg(theChip, "ThDAC3", thd);
		                            		this->fReadoutChipInterface->WriteChipReg(theChip, "ThDAC4", thd);
		                            		this->fReadoutChipInterface->WriteChipReg(theChip, "ThDAC5", thd);
		                            		this->fReadoutChipInterface->WriteChipReg(theChip, "ThDAC6", thd);
						}
		                            	//this->fReadoutChipInterface->WriteChipReg(theChip,"AsyncRead_StartDel_LSB", (11+1), false );

						unsigned int channelNumber = 0;

						//for(uint32_t ich=0;ich<120;ich++)
						//{
						//	if(ich!=100 ) this->fReadoutChipInterface->WriteChipReg(theChip, "ENFLAGS_S" + std::to_string(ich+1), 5);
						//}
						channelNumber++;

		                        }
		                    }
			}


			static_cast<D19cFWInterface*>(fBeBoardInterface->getFirmwareInterface())->PS_Clear_counters();




			if (SyncDebug) ReadNEvents( theBeBoard, NMpulse );
			else ReadASEvent( theBeBoard, NMsec,NMpulse,false,false );

			const std::vector<Event*> &eventVector = GetEvents ( theBeBoard );
			//LOG (INFO) << BOLDRED << "Nev "<<eventVector.size()<<" "<<runpulse<< RESET;

		        for(auto opticalGroup: *cBoard)
		        {
				for(auto hybrid: *opticalGroup) // for on hybrid - begin
				{
					for(auto cSSA: *hybrid) // for on chip - begin
					{
						std::vector<uint32_t> hits;

						if (!SyncDebug) hits= eventVector.at(0)->GetHits(hybrid->getId(), cSSA->getId());


						unsigned int channelNumber = 0;
						unsigned int curh = 0;
						for(auto &channel : *cSSA->getChannelContainer<std::pair<std::array<uint32_t,2>,float>>())
						{
							Nstrip+=1.0;
							if (SyncDebug)
							{
								curh=0;
								for(auto cev: eventVector)
								{
									curh+=cev->DataBit ( hybrid->getId(), cSSA->getId(), channelNumber);
									//LOG (INFO) << BOLDRED << curh<< RESET;
								}
							}
							else curh=hits[channelNumber];
							//LOG (INFO) << BOLDRED << "thd "<<thd<<" , "<<" channel "<<channelNumber<<" , "<<curh<< RESET;


		                            		if ((channel.first[1]>Nlvl) && (curh<channel.first[1]/2) && (channel.first[0]>channel.first[1]/2))
		                                	{

		                                		channel.second=(float(thd)*float(curh) + float(thd-1)*float(channel.first[0]))/(float(curh) +float(channel.first[0]));
		                                		globalmax=std::max(globalmax,channel.second);

		                                		//if ( channelNumber==1)
		                                  		//LOG (INFO) << BOLDRED << "halfmax "<<curh<<","<<channel.first[0]<<","<<channel.first[1]<< RESET;
		                                  		//  LOG (INFO) << BOLDRED << normvals[channelNumber]<<" "<<thd<< RESET;
                                			}


							if (runpulse) channel.first[1]=NMpulse;
							else channel.first[1] =std::max(channel.first[1],curh);
		                                  	//LOG (INFO) << BOLDRED << channel.first[1]<< RESET;
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

		std::string prestr="";
		if (cWithSSA) prestr="THTRIMMING_S" ;
		if (cWithMPA) prestr="TrimDAC_P";

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

						int32_t cr=fReadoutChipInterface->ReadChipReg(theChip, prestr + std::to_string(istrip));

						//LOG (INFO) << BOLDRED <<channel.second<< RESET;
						//LOG (INFO) << BOLDRED <<float(cr)<< RESET;
						//LOG (INFO) << BOLDRED <<float(vfac)<< RESET;
						//LOG (INFO) << BOLDRED <<float(channel.second)<< RESET;
						//LOG (INFO) << BOLDRED <<mean<< RESET;
						//LOG (INFO) << BOLDRED <<wroffset<< RESET;



						float floatTh=cr;
						if(channel.second>1.0)
	                            			floatTh=float(cr)-vfac*(float(channel.second)-mean)+wroffset;


	                            		uint32_t THtowrite=uint32_t(std::roundf(floatTh));
						//LOG (INFO) << BOLDRED <<"write "<<std::to_string(istrip)<<","<<THtowrite<< RESET;

						THtowrite=std::min(THtowrite,uint32_t(31));
	                            		THtowrite=std::max(THtowrite,uint32_t(0));
	                            		writeave+=THtowrite;
						if (THtowrite==0 || THtowrite==31) LOG (INFO) << BOLDRED <<"Warning, thdac at limit. Chip "<<theChip->getId()<<",Strip "<<istrip<<",TH "<<THtowrite<< RESET;
	                            		if (THtowrite==0) Nunt+=1.0;
	                            		if (THtowrite==31) Nunt-=1.0;

	                            		fReadoutChipInterface->WriteChipReg(theChip, prestr + std::to_string(istrip), THtowrite);
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
            	if(std::abs(Nunt)>0.0)Nuntoff+=Nunt/std::abs(Nunt);
            	else Nuntoff+=0;
		wroffset=15.0-writeave/float(Nstrip)+(Nuntoff);
        	LOG (INFO) << BOLDRED <<"RMS: "<<rms<< RESET;
        	LOG (INFO) << BOLDRED <<"MEAN: "<<mean<< RESET;
        	LOG (INFO) << BOLDRED <<"WRITEMEAN: "<<writeave/float(Nstrip)<< RESET;
        	LOG (INFO) << BOLDRED <<"MAX: "<<globalmax<< RESET;
        	LOG (INFO) << BOLDRED <<"NSTRIP: "<<Nstrip<< RESET;
        	LOG (INFO) << BOLDRED <<"OFF: "<<Nuntoff<< RESET;
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
