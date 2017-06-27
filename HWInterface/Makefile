Objs            = RegManager.o BeBoardFWInterface.o MPAInterface.o MPAGlibFWInterface.o GlibFWInterface.o ICGlibFWInterface.o CtaFWInterface.o ICFc7FWInterface.o Cbc3Fc7FWInterface.o D19cFWInterface.o BeBoardInterface.o FpgaConfig.o GlibFpgaConfig.o CtaFpgaConfig.o CbcInterface.o MmcPipeInterface.o Firmware.o
CC              = g++
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pthread -pedantic -fPIC  

.PHONY: clean print

IncludeDirs  = $(CACTUSINCLUDE) ../ .
IncludePaths = $(IncludeDirs:%=-I%)

%.o: %.cc %.h
	$(CXX) -std=c++0x $(CCFlags) $(IncludePaths) -c -o $@ $<

all: print $(Objs) ../HWDescription/Definition.h
	$(CC) -std=c++0x -shared -pthread -o libPh2_Interface.so $(Objs)
	mv libPh2_Interface.so ../lib

print:
	@echo '****************************'
	@echo 'Building HWInterfaces '
	@echo '****************************'
clean:
	rm -f *.o
