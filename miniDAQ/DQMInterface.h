#ifndef _DQMInterface_h_
#define _DQMInterface_h_

#include <vector>

class TCPNetworkClient;
class DQMHistogramBase;

class DQMInterface
{
public:

	DQMInterface (void);
	~DQMInterface(void);

	void configure           (void) ;
	void startProcessingData (std::string runNumber) ;
	void stopProcessingData  (void) ;
	void pauseProcessingData (void) ;
	void resumeProcessingData(void) ;
	//void load(std::string fileName){;}

	bool running();

private:
	void destroy(void);
	void destroyHistogram(void);
	TCPNetworkClient* fListener;
	DQMHistogramBase* fDQMHistogram;
	std::vector<char> fDataBuffer;

};

#endif
