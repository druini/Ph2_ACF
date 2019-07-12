#ifndef _DQMInterface_h_
#define _DQMInterface_h_

#include <vector>
#include <future>

class TCPSubscribeClient;
class DQMHistogramBase;
class TFile;

class DQMInterface
{
public:

	DQMInterface ();
	~DQMInterface(void);

	void configure           (std::string calibrationName, std::string configurationFilePath) ;
	void startProcessingData (std::string runNumber) ;
	void stopProcessingData  (void) ;
	void pauseProcessingData (void) ;
	void resumeProcessingData(void) ;
	//void load(std::string fileName){;}

	bool running(void);

private:
	void destroy(void);
	void destroyHistogram(void);
	TCPSubscribeClient* fListener;
	DQMHistogramBase*   fDQMHistogram;
	std::vector<char>   fDataBuffer;
	bool                fRunning;
	std::future<bool>   fRunningFuture;
	TFile*              fOutputFile;

};

#endif
