#ifndef _DQMInterface_h_
#define _DQMInterface_h_

#include <vector>
#include <future>

class TCPNetworkClient;
class DQMHistogramBase;
class TFile;

class DQMInterface
{
public:

	DQMInterface (std::string configurationFileName);
	~DQMInterface(void);

	void configure           (void) ;
	void startProcessingData (std::string runNumber) ;
	void stopProcessingData  (void) ;
	void pauseProcessingData (void) ;
	void resumeProcessingData(void) ;
	//void load(std::string fileName){;}

	bool running(void);

private:
	void destroy(void);
	void destroyHistogram(void);
	TCPNetworkClient* fListener;
	DQMHistogramBase* fDQMHistogram;
	std::vector<char> fDataBuffer;
	bool              fRunning;
	std::future<bool> fRunningFuture;
	std::string       fConfigurationFile;
	TFile *fOutputFile;

};

#endif
