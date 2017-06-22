#ifndef COMMONVISITORS_H__
#define COMMONVISITORS_H__


#include "../HWInterface/BeBoardFWInterface.h"
#include "../HWInterface/CbcInterface.h"
#include "../HWInterface/BeBoardInterface.h"
#include "../HWDescription/Definition.h"
#include "../Utils/ConsoleColor.h"
#include "../Utils/Visitor.h"

#include <iostream>
#include <vector>
#include <stdlib.h>
# include <string>



// wriite single reg

using namespace Ph2_HwInterface;
using namespace Ph2_HwDescription;

struct CbcRegWriter : public HwDescriptionVisitor
{
    CbcInterface* fInterface;
    std::string fRegName;
    uint8_t fRegValue;

    CbcRegWriter ( CbcInterface* pInterface, std::string pRegName, uint8_t pRegValue ) : fInterface ( pInterface ), fRegName ( pRegName ), fRegValue ( pRegValue ) {}
    CbcRegWriter ( const CbcRegWriter& writer ) : fInterface ( writer.fInterface ), fRegName ( writer.fRegName ), fRegValue ( writer.fRegValue ) {}

    void setRegister ( std::string pRegName, uint8_t pRegValue )
    {
        fRegName = pRegName;
        fRegValue = pRegValue;
    }

    void visit ( Ph2_HwDescription::Cbc& pCbc )
    {
        fInterface->WriteCbcReg ( &pCbc, fRegName, fRegValue );
    }
};

struct BeBoardRegWriter : public HwDescriptionVisitor
{
    BeBoardInterface* fInterface;
    std::string fRegName;
    uint32_t fRegValue;

    BeBoardRegWriter ( BeBoardInterface* pInterface, std::string pRegName, uint32_t pRegValue ) : fInterface ( pInterface ), fRegName ( pRegName ), fRegValue ( pRegValue ) {}

    BeBoardRegWriter ( const BeBoardRegWriter& writer ) : fInterface ( writer.fInterface ), fRegName ( writer.fRegName ), fRegValue ( writer.fRegValue ) {}

    void setRegister ( std::string pRegName, uint8_t pRegValue )
    {
        fRegName = pRegName;
        fRegValue = pRegValue;
    }

    void visit ( Ph2_HwDescription::BeBoard& pBoard )
    {
        fInterface->WriteBoardReg ( &pBoard, fRegName, fRegValue );
    }
};

//write multi reg
struct CbcMultiRegWriter : public HwDescriptionVisitor
{
    CbcInterface* fInterface;
    std::vector<std::pair<std::string, uint8_t>> fRegVec;

    CbcMultiRegWriter ( CbcInterface* pInterface, std::vector<std::pair<std::string, uint8_t>> pRegVec ) : fInterface ( pInterface ), fRegVec ( pRegVec ) {}

    void visit ( Ph2_HwDescription::Cbc& pCbc )
    {
        fInterface->WriteCbcMultReg ( &pCbc, fRegVec );
    }
};

// HwDescription Objects Counter
class Counter : public HwDescriptionVisitor
{
  private:
    uint32_t fNCbc;
    uint32_t fNFe;
    uint32_t fNBe;
    uint32_t fCbcMask;

  public:
    Counter() : fNCbc ( 0 ), fNFe ( 0 ), fNBe ( 0 ), fCbcMask ( 0 ) {}
    void visit ( Ph2_HwDescription::Cbc& pCbc )
    {
        fNCbc++;
        fCbcMask |= (1 << pCbc.getCbcId() );
    }
    void visit ( Ph2_HwDescription::Module& pModule )
    {
        fNFe++;
    }
    void visit ( Ph2_HwDescription::BeBoard& pBoard )
    {
        fNBe++;
    }
    uint32_t getNCbc() const
    {
        return fNCbc;
    }
    uint32_t getNFe() const
    {
        return fNFe;
    }
    uint32_t getNBe() const
    {
        return fNBe;
    }
    uint32_t getCbcMask() const
    {
        return fCbcMask;
    }
};

// Configurator
class Configurator: public HwDescriptionVisitor
{
  private:
    BeBoardInterface* fBeBoardInterface;
    CbcInterface* fCbcInterface;
  public:
    Configurator ( BeBoardInterface* pBeBoardInterface, CbcInterface* pCbcInterface ) : fBeBoardInterface ( pBeBoardInterface ), fCbcInterface ( pCbcInterface ) {}
    void visit ( BeBoard& pBoard )
    {
        fBeBoardInterface->ConfigureBoard ( &pBoard );
        LOG (INFO) << "Successfully configured Board " << +pBoard.getBeId();
    }
    void visit ( Cbc& pCbc )
    {
        fCbcInterface->ConfigureCbc ( &pCbc );
        LOG (INFO) << "Successfully configured Cbc " <<  +pCbc.getCbcId();

    }
};

// read a single CBC register from fRegMap, from the physical CBC
struct CbcRegReader : public HwDescriptionVisitor
{
    std::string fRegName;
    uint8_t fRegValue;
    uint8_t fReadRegValue;
    CbcInterface* fInterface;

    CbcRegReader ( CbcInterface* pInterface, std::string pRegName ) : fInterface ( pInterface ), fRegName ( pRegName ) {}
    CbcRegReader ( const CbcRegReader& reader ) : fInterface ( reader.fInterface ), fRegName ( reader.fRegName ) {}

    void setRegister ( std::string pRegName )
    {
        fRegName = pRegName;
    }
    void visit ( Cbc& pCbc )
    {
        fRegValue = pCbc.getReg ( fRegName );
        fInterface->ReadCbcReg ( &pCbc, fRegName );
        fReadRegValue = pCbc.getReg ( fRegName );

        LOG (INFO) << "Reading Reg " << RED << fRegName << RESET << " on CBC " << +pCbc.getCbcId() << " memory value: " << std::hex << +fRegValue << " read value: " << +fReadRegValue << std::dec ;
    }
};

struct CbcRegIncrementer : public HwDescriptionVisitor
{
    CbcInterface* fInterface;
    std::string fRegName;
    int fRegIncrement;

    CbcRegIncrementer ( CbcInterface* pInterface, std::string pRegName, int pRegIncrement ) : fInterface ( pInterface ), fRegName ( pRegName ), fRegIncrement ( pRegIncrement ) {}
    CbcRegIncrementer ( const CbcRegIncrementer& incrementer ) : fInterface ( incrementer.fInterface ), fRegName ( incrementer.fRegName ), fRegIncrement ( incrementer.fRegIncrement ) {}

    void setRegister ( std::string pRegName, int pRegIncrement )
    {
        fRegName = pRegName;
        fRegIncrement = pRegIncrement;
    }

    void visit ( Ph2_HwDescription::Cbc& pCbc )
    {
        uint8_t currentValue = pCbc.getReg (fRegName);
        int targetValue = int (currentValue) + fRegIncrement;

        if (targetValue > 255) LOG (ERROR) << "Error: cannot increment register above 255" << std::endl, targetValue = 255;
        else if (targetValue < 0) LOG (ERROR) << "Error: cannot increment register below 0 " << std::endl, targetValue = 0;

        fInterface->WriteCbcReg ( &pCbc, fRegName, uint8_t (targetValue) );
    }
};


struct ThresholdVisitor : public HwDescriptionVisitor
{
    uint16_t fThreshold;
    CbcInterface* fInterface;
    char fOption;

    // Write constructor
    ThresholdVisitor (CbcInterface* pInterface, uint16_t pThreshold) : fInterface (pInterface), fThreshold (pThreshold), fOption ('w')
    {
        if (fThreshold > 1023)
        {
            LOG (ERROR) << "Error, Threshold value can be 10 bit max (1023)! - quitting";
            exit (1);
        }
    }
    // Read constructor
    ThresholdVisitor (CbcInterface* pInterface) : fInterface (pInterface), fOption ('r')
    {
    }
    // Copy constructor
    ThresholdVisitor (const ThresholdVisitor& pSetter) : fInterface (pSetter.fInterface), fThreshold (pSetter.fThreshold), fOption (pSetter.fOption)
    {
    }

    void setOption (char pOption)
    {
        if (pOption == 'w' || pOption == 'r')
            fOption = pOption;
        else
            LOG (ERROR) << "Error, not a valid option!";
    }
    uint16_t getThreshold()
    {
        return fThreshold;
    }
    void setThreshold (uint16_t pThreshold)
    {
        fThreshold = pThreshold;
    }

    void visit (Ph2_HwDescription::Cbc& pCbc)
    {

        if (pCbc.getChipType() == ChipType::CBC2)
        {

            if (fOption == 'w')
            {
                if (fThreshold > 255) LOG (ERROR) << "Error, Threshold for CBC2 can only be 8 bit max (255)!";
                else
                {
                    uint8_t cVCth = fThreshold & 0x00FF;
                    fInterface->WriteCbcReg ( &pCbc, "VCth", cVCth );
                }
            }
            else if (fOption == 'r')
            {
                fInterface->ReadCbcReg ( &pCbc, "VCth" );
                fThreshold = (pCbc.getReg ("VCth") ) & 0x00FF;
            }
            else
                LOG (ERROR) << "Unknown option " << fOption;
        }
        else if (pCbc.getChipType() == ChipType::CBC3)
        {

            if (fOption == 'w')
            {
                if (fThreshold > 1023) LOG (ERROR) << "Error, Threshold for CBC3 can only be 10 bit max (1023)!";
                else
                {
                    std::vector<std::pair<std::string, uint8_t>> cRegVec;
                    // VCth1 holds bits 0-7 and VCth2 holds 8-9
                    uint8_t cVCth1 = fThreshold & 0x00FF;
                    uint8_t cVCth2 = (fThreshold & 0x0300) >> 8;
                    cRegVec.emplace_back ("VCth1", cVCth1);
                    cRegVec.emplace_back ("VCth2", cVCth2);
                    fInterface->WriteCbcMultReg (&pCbc, cRegVec);
                }
            }
            else if (fOption == 'r')
            {
                fInterface->ReadCbcReg (&pCbc, "VCth1");
                fInterface->ReadCbcReg (&pCbc, "VCth2");
                uint8_t cVCth2 = pCbc.getReg ("VCth2");
                uint8_t cVCth1 = pCbc.getReg ("VCth1");
                fThreshold = ( ( (cVCth2 & 0x03) << 8) | (cVCth1 & 0xFF) );
            }
            else
                LOG (ERROR) << "Unknown option " << fOption;
        }
        else
            LOG (ERROR) << "Not a valid chip type!";
    }
};

struct LatencyVisitor : public HwDescriptionVisitor
{
    uint16_t fLatency;
    CbcInterface* fInterface;
    char fOption;

    // write constructor
    LatencyVisitor (CbcInterface* pInterface, uint16_t pLatency) : fInterface (pInterface), fLatency (pLatency), fOption ('w') {}
    // read constructor
    LatencyVisitor (CbcInterface* pInterface) : fInterface (pInterface), fOption ('r') {}
    // copy constructor
    LatencyVisitor (const LatencyVisitor& pVisitor) : fInterface (pVisitor.fInterface), fLatency (pVisitor.fLatency), fOption (pVisitor.fOption) {}

    void setOption (char pOption)
    {
        if (pOption == 'w' || pOption == 'r')
            fOption = pOption;
        else
            LOG (ERROR) << "Error, not a valid option!";
    }
    uint16_t getLatency()
    {
        return fLatency;
    }
    void setLatency (uint16_t pLatency)
    {
        fLatency = pLatency;
    }
    void visit (Ph2_HwDescription::Cbc& pCbc)
    {

        if (pCbc.getChipType() == ChipType::CBC2)
        {

            if (fOption == 'w')
            {

                if (fLatency > 255) LOG (ERROR) << "Error, Latency for CBC2 can only be 8 bit max (255)!";
                else
                {
                    uint8_t cLat = fLatency & 0x00FF;
                    fInterface->WriteCbcReg ( &pCbc, "TriggerLatency", cLat );
                }
            }
            else
            {
                fInterface->ReadCbcReg ( &pCbc, "TriggerLatency" );
                fLatency = (pCbc.getReg ("TriggerLatency") ) & 0x00FF;
            }
        }
        else if (pCbc.getChipType() == ChipType::CBC3)
        {
            if (fOption == 'w')
            {
                std::vector<std::pair<std::string, uint8_t>> cRegVec;
                // TriggerLatency1 holds bits 0-7 and FeCtrl&TrgLate2 holds 8
                uint8_t cLat1 = fLatency & 0x00FF;
                //in order to not mess with the other settings in FrontEndControl&TriggerLatency2, I have to read the reg
                uint8_t presentValue = pCbc.getReg ("FeCtrl&TrgLat2") & 0xFE;
                uint8_t cLat2 = presentValue | ( (fLatency & 0x0100) >> 8);
                cRegVec.emplace_back ("TriggerLatency1", cLat1);
                cRegVec.emplace_back ("FeCtrl&TrgLat2", cLat2);
                fInterface->WriteCbcMultReg (&pCbc, cRegVec);
            }
            else
            {
                fInterface->ReadCbcReg (&pCbc, "TriggerLatency1");
                fInterface->ReadCbcReg (&pCbc, "FeCtrl&TrgLat2");
                fLatency = ( (pCbc.getReg ("FeCtrl&TrgLat2") & 0x01) << 8) | (pCbc.getReg ("TriggerLatency1") & 0xFF);
            }
        }
        else
            LOG (ERROR) << "Not a valid chip type!";
    }
};


#endif
