#include "../System/SystemController.h"
#include "../Utils/CommonVisitors.h"
#include "../Utils/Timer.h"
#include "../Utils/argvparser.h"

#ifdef __POWERSUPPLY__
#include "DeviceHandler.h"
#include "PowerSupply.h"
#include "PowerSupplyChannel.h"
#endif

using namespace Ph2_HwDescription;
using namespace Ph2_HwInterface;
using namespace Ph2_System;
using namespace CommandLineProcessing;
INITIALIZE_EASYLOGGINGPP

int main(int argc, char** argv)
{
#ifdef __POWERSUPPLY__
    // configure the logger
    el::Configurations conf(std::string(std::getenv("PH2ACF_BASE_DIR")) + "/settings/logger.conf");
    el::Loggers::reconfigureAllLoggers(conf);

    ArgvParser cmd;

    // init
    cmd.setIntroductoryDescription("CMS Ph2_ACF power supply example application");
    // error codes
    cmd.addErrorCode(0, "Success");
    cmd.addErrorCode(1, "Error");
    // options
    cmd.setHelpOption("h", "help", "Example: powersupply -f CMSIT.xml --name TestKeithley --channel Front -v 1.8");

    cmd.defineOption("name", "Name of the power supply as described in the HW file", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/);
    cmd.defineOptionAlternative("name", "n");

    cmd.defineOption("channel", "Channel of the power supply as described in the HW file (e.g.: \"Front\")", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/);
    cmd.defineOptionAlternative("channel", "ch");

    cmd.defineOption("voltage", "Voltage to be set", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/);
    cmd.defineOptionAlternative("voltage", "v");

    cmd.defineOption("i_max", "Maximum current setting", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/);
    cmd.defineOptionAlternative("i_max", "i_max");

    cmd.defineOption("v_max", "Maximum voltage setting", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/);
    cmd.defineOptionAlternative("v_max", "v_max");

    cmd.defineOption("off", "Turn off power supply");
    cmd.defineOptionAlternative("off", "o");

    // cmd.defineOption ( "GUI", "Named pipe for GUI communication", ArgvParser::OptionRequiresValue /*|
    // ArgvParser::OptionRequired*/ ); cmd.defineOptionAlternative ( "GUI", "g" );

    cmd.defineOption("file", "Hw Description File . Default value: settings/D19CDescription_Cic2.xml", ArgvParser::OptionRequiresValue /*| ArgvParser::OptionRequired*/);
    cmd.defineOptionAlternative("file", "f");

    int result = cmd.parse(argc, argv);

    if(result != ArgvParser::NoParserError)
    {
        LOG(INFO) << cmd.parseErrorDescription(result);
        exit(1);
    }

    // now query the parsing results
    std::string cHWFile             = (cmd.foundOption("file")) ? cmd.optionValue("file") : "settings/D19CDescription_Cic2.xml";
    std::string cPowerSupply        = (cmd.foundOption("name")) ? cmd.optionValue("name") : "";
    std::string cPowerSupplyChannel = (cmd.foundOption("channel")) ? cmd.optionValue("channel") : "";
    double      cVoltsLimit         = (cmd.foundOption("v_max")) ? std::stod(cmd.optionValue("v_max").c_str()) : 10.5;
    double      cAmpsLimit          = (cmd.foundOption("i_max")) ? std::stod(cmd.optionValue("i_max").c_str()) : 1.3;
    double      cVolts              = (cmd.foundOption("v")) ? std::stod(cmd.optionValue("v").c_str()) : 0;
    bool        cTurnOff            = cmd.foundOption("o");

    std::string docPath = cHWFile;
    LOG(INFO) << "Init PS with " << docPath;
    pugi::xml_document docSettings;

    DeviceHandler ps_deviceHandler;
    ps_deviceHandler.readSettings(docPath, docSettings); // Includes connection configuration/initialization

    PowerSupply* dPowerSupply = ps_deviceHandler.getPowerSupply(cPowerSupply);  // Will throw std::out_of_range error if not found
    PowerSupplyChannel* dPowerSupplyChannel = dPowerSupply->getChannel(cPowerSupplyChannel);

    // if(dPowerSupply->isOpen()) // power supply doesn't have isOpen method
    // {
        if(cTurnOff)
        {
            LOG(INFO) << "Turn off " << cPowerSupply;
            dPowerSupplyChannel->turnOff();
        }
        else
        {
            if(cmd.foundOption("v_max")) { dPowerSupplyChannel->setVoltageCompliance(cVoltsLimit); }
            if(cmd.foundOption("i_max")) { dPowerSupplyChannel->setCurrentCompliance(cAmpsLimit); }
            if(cmd.foundOption("v"))
            {
                dPowerSupplyChannel->setVoltage(cVolts);
                dPowerSupplyChannel->turnOn();
            }
        }
        sleep(1);
        LOG(INFO) << BOLDWHITE << cPowerSupply << " channel " << cPowerSupplyChannel << " status:" RESET;
        LOG(INFO) << "\tV(set):\t\t" << BOLDWHITE << dPowerSupplyChannel->getSetVoltage() << RESET;
        LOG(INFO) << "\tV(meas):\t" << BOLDWHITE << dPowerSupplyChannel->getOutputVoltage() << RESET;
        LOG(INFO) << "\tI_max(set):\t" << BOLDWHITE << dPowerSupplyChannel->getCurrentCompliance() << RESET;
        LOG(INFO) << "\tI(meas):\t" << BOLDWHITE << dPowerSupplyChannel->getCurrent() << RESET;

    // }
    // else
    // {
    //     LOG(INFO) << cPowerSupply << " not found!";
    // }

#endif

    return 0;
}
