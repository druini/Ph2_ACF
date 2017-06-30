YEL='\033[1m\033[33m'
RED='\033[1m\033[31m'
BLU='\033[1m\033[35m'
RESET='\033[0m'

DEPENDENCIES := Utils HWDescription HWInterface RootWeb
ANTENNAINSTALLED = no
AMC13INSTALLED = no
USBINSTINSTALLED = no

ROOTVERSION := $(shell root-config --version)

ifneq ("$(wildcard $(ANTENNADIR))","")
	DEPENDENCIES := CMSPh2_AntennaDriver $(DEPENDENCIES)
	ANTENNAINSTALLED = yes
else
	ANTENNAINSTRUCTIONS = To use the USB Antenna, please download the Driver from 'https://gitlab.cern.ch/cms_tk_ph2/CMSPh2_AntennaDriver.git' and make sure that libusb-devel is installed!
endif

ifneq ("$(wildcard $(AMC13DIR))","")
	DEPENDENCIES := AMC13 $(DEPENDENCIES)
	AMC13INSTALLED = yes
else
	AMC13INSTRUCTIONS = This feature can only be built if the AMC13 libraries are installed with cactus!
endif

ifneq ("$(wildcard $(USBINSTDIR))","")
	USBINSTINSTALLED = yes
	USBINSTINSTRUCTIONS = 
else
	USBINSTINSTRUCTIONS = To use the Ph2_USBInstDriver please download it from 'https://gitlab.cern.ch/cms_tk_ph2/Ph2_USBInstDriver.git'.
endif

.PHONY: print dependencies $(DEPENDENCIES) clean src miniDAQ tools

all: src miniDAQ

dependencies: print $(DEPENDENCIES)

$(DEPENDENCIES):
	$(MAKE) -C $@

System: dependencies
	$(MAKE) -C System

tools: System
	$(MAKE) -C tools

src miniDAQ: tools
	$(MAKE) -C $@



print:
	@echo -e '   '   
	@echo -e ${YEL}'*****************************'
	@echo -e 'BUILDING PH2 ACF FRAMEWORK'
	@echo -e 'with the following Dependencies:'${RESET}
	@echo -e ${BLU}$(DEPENDENCIES)${RESET}
	@echo -e ${YEL}'ROOT Version: '${RED} $(ROOTVERSION)${RESET}
	@echo -e ${YEL}'*****************************'${RESET}
	@echo -e ${YEL}'Amc13 SW installed:'${RED} $(AMC13INSTALLED)${RESET}
	@echo -e $(AMC13INSTRUCTIONS)
	@echo -e ${YEL}'*****************************'${RESET}
	@echo -e ${YEL}'Antenna Driver installed:' ${RED}$(ANTENNAINSTALLED)${RESET}
	@echo -e $(ANTENNAINSTRUCTIONS)
	@echo -e ${YEL}'*****************************'${RESET}
	@echo -e ${YEL}'Ph2 USB Inst Driver installed:' ${RED}$(USBINSTINSTALLED)${RESET}
	@echo -e $(USBINSTINSTRUCTIONS)
	@echo -e ${YEL}'*****************************'${RESET}


clean:
	(cd System; make clean)
	(cd Utils; make clean)
	(cd HWInterface; make clean)
	(cd HWDescription; make clean)
	(cd tools; make clean)
	(cd RootWeb; make clean)
	(cd miniDAQ; make clean)
	(cd AMC13; make clean)
	(rm -f lib/* bin/*)
