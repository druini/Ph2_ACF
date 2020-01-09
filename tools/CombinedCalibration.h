#include "../System/SystemController.h"
#include "../tools/Tool.h"
// #include "../tools/Calibration.h"
// #include "../tools/PedeNoise.h"

template<class... Cs> 
class CombinedCalibration
{
public:
    Tool *toolPointer;
};

template<typename T, typename... Rest >
void privateStart(int currentRun, CombinedCalibration<T, Rest...>& calList);


template<class Head, class... Tail> 
class CombinedCalibration<Head, Tail...> : public Tool//: public CombinedCalibration<Tail...>
{
public:
    CombinedCalibration() 
    {
        std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Constructor !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
    }

    void Configure(std::string cHWFile, bool enableStream = false) override
    {
        std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Configuring !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
        toolPointer = this;
        Tool::Configure(cHWFile,enableStream);
        Tool::CreateResultDirectory("Results",false,false);
    }

    void Start(int currentRun) override
    {
        std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Starting !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
        privateStart(currentRun, *this);
        std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Starting Done !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
        // std::cout<<"Starting\n";
        // calibrationHead.Inherit(this);
        // calibrationHead.Start(currentRun);
        // std::cout<<"TheRest\n";
        // calibrationTail.Start(currentRun);
    }

    void Stop() override
    {
        std::cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Stopping !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";

        toolPointer->dumpConfigFiles();
        toolPointer->SaveResults();
        toolPointer->Destroy();
    }

    Head calibrationHead;
    CombinedCalibration<Tail...> calibrationTail;
    Tool *toolPointer;

};

void privateStart(int currentRun, CombinedCalibration<>& calList) 
{
    std::cout << __PRETTY_FUNCTION__ <<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Private Starting Empty !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
}

template<typename T, typename... Rest >
void privateStart(int currentRun, CombinedCalibration<T, Rest...>& calList)
{
    std::cout << __PRETTY_FUNCTION__ << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Private Starting !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
    std::cout << __PRETTY_FUNCTION__ << calList.toolPointer<<std::endl;
    calList.calibrationHead.Inherit(calList.toolPointer);
    calList.calibrationHead.ConfigureCalibration();
    calList.calibrationHead.Start(currentRun);
    /* calList.calibrationHead.writeObjects( ); */
    calList.calibrationHead.resetPointers();
    std::cout << __PRETTY_FUNCTION__ <<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! The Rest !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
    calList.calibrationTail.toolPointer = calList.toolPointer;
    privateStart(currentRun,calList.calibrationTail);
    std::cout << __PRETTY_FUNCTION__ <<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! Done with the rest !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
}


// typedef CombinedCalibration<Calibration,PedeNoise> CalibrationAndPedeNoise;

// class CalibrationAndPedeNoise : public Tool
// {
// public:
//     CalibrationAndPedeNoise()
//     {

//     }
//     ~CalibrationAndPedeNoise()
//     {
        
//     }

//     void Start(int currentRun) override
//     {

//         CreateResultDirectory ( "Results/Run_" + std::to_string(currentRun) +"_CalibrationAndPedeNoise" );
//         InitResultFile ( "CalibrationAndPedeNoiseResults" );

//         theCalibration.Inherit(this);
//         theCalibration.Initialise ( true, true );

//         theCalibration.FindVplus();
//         theCalibration.FindOffsets();
//         theCalibration.writeObjects();


//         thePedenoise.Inherit(this);
//         thePedenoise.Initialise ( true, true );
//         thePedenoise.measureNoise();
//         thePedenoise.Validate();
//         thePedenoise.writeObjects();

//         dumpConfigFiles();
//         SaveResults();
//         CloseResultFile();
//         Destroy();
//     }

//     void Stop() override {;}
//     void Configure(std::string cHWFile) override
//     {
//         SystemController::Configure(cHWFile);
//     }

//     void Pause() override  {;}
//     void Resume() override  {;}


// private:
//     Calibration theCalibration;
//     PedeNoise   thePedenoise;

// };
