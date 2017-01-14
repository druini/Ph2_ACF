Objs                    = FrontEndDescription.o BeBoard.o Cbc.o MPA.o Module.o 
CC              = g++
CXX             = g++
CCFlags         = -g -O1 -Wall -fPIC
DevFlags               =

.PHONY: clean print


IncludeDirs     =  /opt/cactus/include ../

IncludePaths            = $(IncludeDirs:%=-I%)

%.o: %.cc %.h
	$(CXX) -std=c++11 $(DevFlags) $(CCFlags) $(UserCCFlags) $(CCDefines) $(IncludePaths) -c -o $@ $<


all: print $(Objs)
	$(CC) -shared -o libPh2_Description.so $(Objs)
	mv libPh2_Description.so ../lib

print:
	@echo '****************************'
	@echo 'Building HWDescription objects'
	@echo '****************************'
clean:
	rm -f *.o
