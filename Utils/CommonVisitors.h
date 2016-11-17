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

        LOG (INFO) << "Reading Reg " << RED << fRegName << RESET << " on CBC " << +pCbc.getCbcId() << " memory value: " << +fRegValue << " read value: " << +fReadRegValue ;
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

        if (targetValue > 255) LOG (ERROR) << "Error: cannot increment register above 255" << std::endl , targetValue = 255;
        else if (targetValue < 0) LOG (ERROR) << "Error: cannot increment register below 0 " << std::endl , targetValue = 0;

        fInterface->WriteCbcReg ( &pCbc, fRegName, uint8_t (targetValue) );
    }
};



#endif
