#include <iostream>
#include <stdio.h>
#include <unistd.h>

#include "../tools/CBCPulseShape.h"
#include "../tools/CalibrationExample.h"
#include "../tools/CombinedCalibration.h"
#include "../tools/PedeNoise.h"
#include "../tools/PedestalEqualization.h"
#include "../tools/RD53ClockDelay.h"
#include "../tools/RD53Gain.h"
#include "../tools/RD53GainOptimization.h"
#include "../tools/RD53InjectionDelay.h"
#include "../tools/RD53Latency.h"
#include "../tools/RD53Physics.h"
#include "../tools/RD53PixelAlive.h"
#include "../tools/RD53SCurve.h"
#include "../tools/RD53ThrAdjustment.h"
#include "../tools/RD53ThrEqualization.h"
#include "../tools/RD53ThrMinimization.h"
#include "../tools/Tool.h"
#include "MiddlewareController.h"
//#include "../tools/SSAPhysics.h"
#include "../tools/BackEndAlignment.h"

//========================================================================================================================
MiddlewareController::MiddlewareController(int serverPort) : TCPServer(serverPort, 1)
{
    // TCPServer::setReceiveTimeout(1,0);//Doesn't work
}

//========================================================================================================================
MiddlewareController::~MiddlewareController(void) { LOG(INFO) << __PRETTY_FUNCTION__ << " DESTRUCTOR" << RESET; }

//========================================================================================================================
std::string MiddlewareController::interpretMessage(const std::string& buffer)
{
    LOG(INFO) << __PRETTY_FUNCTION__ << " Message received from OTSDAQ: " << buffer << RESET;

    if(buffer == "Initialize") // Changing the status changes the mode in threadMain (BBC) function
    { return "InitializeDone"; }
    else if(buffer.substr(0, 5) == "Start") // Changing the status changes the mode in threadMain (BBC) function
    {
        currentRun_ = getVariableValue("RunNumber", buffer);
        theSystemController_->Start(stoi(currentRun_));
        return "StartDone";
    }
    else if(buffer.substr(0, 4) == "Stop")
    {
        theSystemController_->Stop();
        LOG(INFO) << "Run " << currentRun_ << " stopped" << RESET;
        return "StopDone";
    }
    else if(buffer.substr(0, 4) == "Halt")
    {
        theSystemController_->Stop();
        theSystemController_->Destroy();
        theSystemController_ = nullptr;
        LOG(INFO) << "Run " << currentRun_ << " halted" << RESET;
        return "HaltDone";
    }
    else if(buffer == "Pause")
    {
        LOG(INFO) << BOLDBLUE << "Paused" << RESET;
        return "PauseDone";
    }
    else if(buffer == "Resume")
    {
        LOG(INFO) << BOLDBLUE << "Resumed" << RESET;
        return "ResumeDone";
    }
    else if(buffer.substr(0, 9) == "Configure")
    {
        LOG(INFO) << BOLDBLUE << "Configuring" << RESET;

        if(getVariableValue("Calibration", buffer) == "calibration")
            theSystemController_ = new CombinedCalibration<BackEndAlignment, PedestalEqualization>;
        else if(getVariableValue("Calibration", buffer) == "pedenoise")
            theSystemController_ = new CombinedCalibration<BackEndAlignment, PedeNoise>;
        else if(getVariableValue("Calibration", buffer) == "calibrationandpedenoise")
            theSystemController_ = new CombinedCalibration<BackEndAlignment, PedestalEqualization, PedeNoise>;
        else if(getVariableValue("Calibration", buffer) == "calibrationexample")
            theSystemController_ = new CombinedCalibration<BackEndAlignment, CalibrationExample>;
        else if(getVariableValue("Calibration", buffer) == "cbcPulseShape")
            theSystemController_ = new CombinedCalibration<BackEndAlignment, CBCPulseShape>;

        else if(getVariableValue("Calibration", buffer) == "pixelalive")
            theSystemController_ = new PixelAlive;
        else if(getVariableValue("Calibration", buffer) == "noise")
            theSystemController_ = new PixelAlive;
        else if(getVariableValue("Calibration", buffer) == "scurve")
            theSystemController_ = new SCurve;
        else if(getVariableValue("Calibration", buffer) == "gain")
            theSystemController_ = new Gain;
        else if(getVariableValue("Calibration", buffer) == "gainopt")
            theSystemController_ = new GainOptimization;
        else if(getVariableValue("Calibration", buffer) == "threqu")
            theSystemController_ = new ThrEqualization;
        else if(getVariableValue("Calibration", buffer) == "thrmin")
            theSystemController_ = new ThrMinimization;
        else if(getVariableValue("Calibration", buffer) == "thradj")
            theSystemController_ = new ThrAdjustment;
        else if(getVariableValue("Calibration", buffer) == "latency")
            theSystemController_ = new Latency;
        else if(getVariableValue("Calibration", buffer) == "injdelay")
            theSystemController_ = new InjectionDelay;
        else if(getVariableValue("Calibration", buffer) == "clockdelay")
            theSystemController_ = new ClockDelay;
        else if(getVariableValue("Calibration", buffer) == "physics")
            theSystemController_ = new Physics;

        else
        {
            LOG(ERROR) << BOLDRED << __PRETTY_FUNCTION__ << " Calibration type " << getVariableValue("Calibration", buffer) << " not found, Aborting" << RESET;
            abort();
        }

        LOG(INFO) << BOLDBLUE << "SystemController created" << RESET;
        theSystemController_->Configure(getVariableValue("ConfigurationFile", buffer), true);
        return "ConfigureDone";
    }
    else if(buffer.substr(0, 6) == "Error:")
    {
        if(buffer == "Error: Connection closed") LOG(ERROR) << BOLDRED << __PRETTY_FUNCTION__ << buffer << ". Closing client server connection!" << RESET;
        return "";
    }
    else
    {
        LOG(ERROR) << BOLDRED << __PRETTY_FUNCTION__ << " Can't recognige message: " << buffer << ". Aborting..." << RESET;
        abort();
    }

    if(running_ || paused_) // We go through here after start and resume or pause: sending back current status
    { LOG(INFO) << BOLDBLUE << "Getting time and status here" << RESET; }

    return "Didn't understand the message!";
}
