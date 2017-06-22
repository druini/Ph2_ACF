Objs            = Amc13Description.o Amc13Interface.o Amc13Controller.o
CC              = g++
CXX             = g++
CCFlags         = -g -O1 -w -Wall -pedantic -fPIC -Wcpp

IncludeDirs     = ../

LibraryDirs =  $(CACTUSLIB) ../ ../lib

ExternalObjects =

ifneq ("$(wildcard $(AMC13DIR))","")
	AMC13INSTALLED = 'Building AMC13 related Objects'
	ExternalObjects += -lcactus_amc13_amc13 
	IncludeDirs += $(AMC13DIR)/..
else
	AMC13INSTALLED = 'This feature can only be built if the AMC13 libraries are installed with cactus!' 
endif

IncludePaths    = $(IncludeDirs:%=-I%)
LibraryPaths = $(LibraryDirs:%=-L%)

.PHONY: clean print

%.o: %.cc %.h
	$(CXX) -std=c++0x $(CCFlags) $(IncludePaths) $(LibraryPaths) $(ExternalObjects) -c -o $@ $<

all:print $(Objs) 
	$(CC) -std=c++0x -shared -o libPh2_Amc13.so $(Objs)
	mv libPh2_Amc13.so ../lib

print:
	@echo '****************************************'
	@echo $(AMC13INSTALLED)
	@echo '****************************************'

clean:
	rm -f *.o

