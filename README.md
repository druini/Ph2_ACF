Cbc_default_electron# CMS Ph2 ACF (Acquisition & Control Framework) 

##### Contains:

- A middleware API layer, implemented in C++, which wraps the firmware calls and handshakes into abstracted functions

- A C++ object-based library describing the system components (CBCs,
        Hybrids, Boards) and their properties(values, status)

- several utilities (like visitors to execute certain tasks for each item in the hierarchical Item description)

- a tools/ directory with several utilities (currently: calibration, hybrid testing, common-mode analysis)

    - some applications: datatest, interfacetest, hybridtest, system, calibrate, commission, fpgaconfig

##### Different versions

On this Repo, you can find different version of the software :
    - a hopefully working and stable version on the master branch
    - An in-progress version in the Dev branch

### Setup

Firmware for the GLIB can be found in /firmware. Since the "old" FMC flavour is deprecated, only new FMCs (both connectors on the same side) are supported.
You'll need Xilinx Impact and a [Xilinx Platform Cable USB II] (http://uk.farnell.com/xilinx/hw-usb-ii-g/platform-cable-configuration-prog/dp/1649384)

#### Setup on SLC6

1. Install the latest gcc compiler:
        $> sudo wget -O /etc/yum.repos.d/slc6-devtoolset.repo http://linuxsoft.cern.ch/cern/devtoolset/slc6-devtoolset.repo
        $> sudo yum install devtoolset-2
        $> . /opt/rh/devtoolset-2/enable   # add this to your .bashrc
        $> sudo ln -s /opt/rh/devtoolset-2/root/usr/bin/* /usr/local/bin/
        $> hash -r

    This should give you a more recent gcc (e.g. gcc 4.8.2)

        $> gcc --version

1. Install uHAL  version 2.3:

        $> wget http://svnweb.cern.ch/trac/cactus/export/28265/tags/ipbus_sw/uhal_2_3_0/scripts/release/cactus.slc6.x86_64.repo 

    (You may need the --no-check-certificate)

        $> sudo cp cactus.slc6.x86_64.repo /etc/yum.repos.d/cactus.repo

    then

        $> sudo yum clean all
        $> sudo yum groupinstall uhal

1. Install CERN ROOT version 5.34.32: [Instructions](http://root.cern.ch/drupal/content/installing-root-source) - make sure to use "fixed location installation" when building yourself. If root is installed on a CERN computer of virtual machine you can use:
       
        $> sudo yum install root
        $> sudo yum install root-net-http root-graf3d-gl root-physics libusb-devel root-montecarlo-eg root-graf3d-eve root-geom

1. If you are working on a remote machine, you need these packages for the Canvases to show

        $> sudo yum install xorg-x11-xauth.x86_64

    Note: You may also need to set the environment variables (or source setup.sh):

        $> export LD_LIBRARY_PATH=/opt/cactus/lib:$LD_LIBRARY_PATH
        $> export PATH=/opt/cactus/bin:$PATH

### The Ph2_ACF Software : 

Follow these instructions to install and compile the libraries:
(provided you installed the latest version of gcc, µHal, boost as mentioned above).

1. Clone the GitHub repo and run setup.sh
  
        $> git clone https://:@gitlab.cern.ch:8443/cms_tk_ph2/Ph2_ACF.git
        $> source setup.sh

1. Do a `make` in the root of the repo (make sure you have all µHal, root libraries on your computer).

1. Launch 

        $> systemtest --help

    command if you want to test the parsing of the HWDescription.xml file.

1. Launch

        $> datatest --help

    command if you want to test if you can correctly read data

1. Launch

        $> calibrate --help

    to calibrate a hybrid,

        $> hybridtest --help

    to test a hybird's I2C registers and input channel connectivity

          $> cmtest --help

    to run the CM noise study

          $> pulseshape --help

    to measure the analog pulseshape of the cbc

          $> configure --help

    to apply a configuration to the CBCs

1. Launch

          $> commission --help

    to do latency & threshold scans

1. Launch 

          $> fpgaconfig --help

    to upload a new FW image to the GLIB

1. Launch

          $> miniDAQ --help

    to save binary data from the GLIB to file

1. Launch

          $> miniDQM --help

    to run the DQM code from the June '15 beamtest

##### What can you do with the software ?

At the moment the package provides the following features:

  - Configure the Glib & Cbcs
  - Manipulate the registers in the Glib
  - Manipulate the registers in the Cbcs
  - Read Data
  - Calibrate Hybrids
  - Validate Hybrids
  - Perform CM noise tests
  - user external trigger and clock signals for your tests
  - upload .mcs files to the GLIB
  - perform simple commissioning procedures
  - save binary data to file
  - create simple DQM histograms from binary data
  - measure the pulseshape of the CBC amp
  - any other routine you want to implement yourself ... 


##### Nota Bene:
When you write a register in the Glib or the Cbc, the corresponding map of the HWDescription object in memory is also updated, so that you always have an exact replica of the HW Status in the memory.

Register values are:
  - 8-bit unsigend integers for the CBCs that should be edited in hex notation, i.e. '0xFF'
  - 32-bit unsigned integers for the GLIB: decimal values

For debugging purpose, you can activate DEV_FLAG in the sources or in the Makefile and also activate the uHal log in RegManager.cc.


#### External Clock and Trigger:

In order to use external Clock and Trigger functionality, a DIO5 mezzanine is required. It is available from the [CERN OHR](http://www.ohwr.org/projects/fmc-dio-5chttla) and sold by several commercial vendors.
For instructions on how to use it, see this [file](https://github.com/gauzinge/Ph2_ACF/blob/Dev/doc/TK_DAQ_MONO_GLIB3_FMCDIO5_v3.0_09-12-2014.pdf). The [firmware](https://github.com/gauzinge/Ph2_ACF/tree/Dev/firmware) is included in this repository.



#### Example HWDescription.xml File with DIO5 support:

```xml

          <?xml version='1.0' encoding = 'UTF-8' ?>
          <HwDescription>
          <Connections name="file://settings/connections_2CBC.xml"/>

          <Shelve Id="0" >
          <BeBoard Id="0" connectionId="board0" boardType="GLIB">
          <FW_Version NCbcDataSize="4"/>

          <Module FeId="0" FMCId="0" ModuleId="0" Status="1">
          <!--Global_CBC_Register name="VCth"> 0x88 </Global_CBC_Register>
          <Global_CBC_Register name="TriggerLatency"> 0x0C </Global_CBC_Register-->

          <CBC_Files path="./settings/"/>
          <CBC Id="0" configfile="Cbc_default_hole.txt"/>
          <CBC Id="1" configfile="Cbc_default_hole.txt"/>
          </Module>

          <!-- Commissioning Mode -->
          <!-- set to 1 to enable commissioning mode -->
          <Register name="COMMISSIONNING_MODE_RQ">0</Register>
          <!-- set to 1 to enable test pulse in commissioning mode -->
          <Register name="COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID">0</Register>
          <Register name="COMMISSIONNING_MODE_DELAY_AFTER_FAST_RESET">50</Register>
          <Register name="COMMISSIONNING_MODE_DELAY_AFTER_L1A">400</Register>
          <Register name="COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE">201</Register>

          <!-- Acquisition -->
          <Register name="cbc_stubdata_latency_adjust_fe1">1</Register>
          <Register name="cbc_stubdata_latency_adjust_fe2">1</Register>
          <Register name="user_wb_ttc_fmc_regs.pc_commands.CBC_DATA_PACKET_NUMBER">9</Register>

          <!-- Trigger -->
          <!-- set to 1 to use external triggers -->
          <Register name="user_wb_ttc_fmc_regs.pc_commands.TRIGGER_SEL">0</Register>
          <Register name="user_wb_ttc_fmc_regs.pc_commands.INT_TRIGGER_FREQ">10</Register>

          <!-- Clock -->
          <!-- set to 1 for external clocking -->
          <Register name="user_wb_ttc_fmc_regs.dio5.clk_mux_sel">0</Register>

          <!-- DIO5 Config -->
          <!-- set to 0 for rising edge, 1 for falling -->
          <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_edge">0</Register>
          <!-- set to 1 to output L1A signal, 0 for input pulse -->
          <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_lemo2_sig_sel">1</Register>
          <!-- set to 1 for active low or 1 for active high || NEEDS TO BE 0 for the TLU-->
          <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_backpressure_out_polar">0</Register>

          <!-- DIO5 threshold: [v]/3.3*256 -->
          <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_trig_in">40</Register>
          <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_clk_in">40</Register>

          <!-- DIO5 Termination -->
          <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_clk_in_50ohms">1</Register>
          <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_clk_out_50ohms">0</Register>
          <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_50ohms">1</Register>
          <Register name="user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_out_50ohms">0</Register>
          </BeBoard>
          </Shelve>
          </HwDescription>

          <Settings>
          <Setting name="RunNumber"> 1 </Setting>
          <Setting name="HoleMode"> 1 </Setting>
          </Settings>

```

#### Example HWDescription.json File with DIO5 support:

```json
{
    "HwDescription":{
        "Connections":"file://settings/connections_2CBC.xml",
            "Shelves":[
            {
                "Id":0,
                "BeBoards":[
                {
                    "Id":0,
                    "boardType":"Glib",
                    "connectionId":"board0",
                    "Modules":[
                    {
                        "FeId":0,
                        "FMCId":0,
                        "ModuleId":0,
                        "Status":1,
                        "Global_CBC_Registers":{
                            "VCth":"0x78",
                            "TriggerLatency":"0x0C"
                        },
                        "CbcFilePath":"./settings/",
                        "CBCs":[
                        {
                            "Id":0,
                            "configfile":"Cbc_default_hole.txt",
                            "Register":{
                                "VCth":"0x78"
                            }
                        },
                        {
                            "Id":1,
                            "configfile":"Cbc_default_hole.txt"
                        }
                        ]
                    }
                    ],
                    "RegisterName":{
                        "COMMISSIONNING_MODE_RQ":"//COMMISSIONING MODE REGISTERS",
                        "COMMISSIONNING_MODE_RQ":"//set to 1 to enable commissioning mode",
                        "COMMISSIONNING_MODE_RQ":1,
                        "COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID":"//set to 1 to enable test pulses in comissioningn mode",
                        "COMMISSIONNING_MODE_CBC_TEST_PULSE_VALID":1,
                        "COMMISSIONNING_MODE_DELAY_AFTER_FAST_RESET":50,
                        "COMMISSIONNING_MODE_DELAY_AFTER_L1A":400,
                        "COMMISSIONNING_MODE_DELAY_AFTER_TEST_PULSE":201,

                        "user_wb_ttc_fmc_regs.pc_commands.TRIGGER_SEL":"//TRIGGER",
                        "user_wb_ttc_fmc_regs.pc_commands.TRIGGER_SEL":"//set to 1 to enable external triggers",
                        "user_wb_ttc_fmc_regs.pc_commands.TRIGGER_SEL":0,
                        "user_wb_ttc_fmc_regs.pc_commands.INT_TRIGGER_FREQ":10,
                        "user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_trig_in":"//DIO5 threshold: [v]/3.3*256",
                        "user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_trig_in":40,
                        "user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_edge":"//set to 0 for rising edge, 1 for falling",
                        "user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_edge":0,
                        "user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_in_50ohms":1,
                        "user_wb_ttc_fmc_regs.dio5.fmcdio5_trig_out_50ohms":0,
                        "user_wb_ttc_fmc_regs.dio5.fmcdio5_lemo2_sig_sel":"//set to 1 to output L1A signal, 0 for input pulse ",
                        "user_wb_ttc_fmc_regs.dio5.fmcdio5_lemo2_sig_sel":1,

                        "user_wb_ttc_fmc_regs.dio5.clk_mux_sel":"//CLOCK",
                        "user_wb_ttc_fmc_regs.dio5.clk_mux_sel":"//set to 1 to enable external clocking",
                        "user_wb_ttc_fmc_regs.dio5.clk_mux_sel":0,
                        "user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_clk_in":"//DIO5 threshold: [v]/3.3*256",
                        "user_wb_ttc_fmc_regs.dio5.fmcdio5_threshold_clk_in":40,
                        "user_wb_ttc_fmc_regs.dio5.fmcdio5_clk_in_50ohms":1,
                        "user_wb_ttc_fmc_regs.dio5.fmcdio5_clk_out_50ohms":0,

                        "user_wb_ttc_fmc_regs.pc_commands.ACQ_MODE":"//ACQUISITION",
                        "user_wb_ttc_fmc_regs.pc_commands.ACQ_MODE":1,
                        "cbc_stubdata_latency_adjust_fe1":1,
                        "cbc_stubdata_latency_adjust_fe2":1,
                        "user_wb_ttc_fmc_regs.pc_commands.CBC_DATA_GENE":1,
                        "user_wb_ttc_fmc_regs.pc_commands.CBC_DATA_PACKET_NUMBER":10,
                        "user_wb_ttc_fmc_regs.pc_commands2.clock_shift":0,

                        "user_wb_ttc_fmc_regs.pc_commands2.negative_logic_CBC":"//POLARITY",
                        "user_wb_ttc_fmc_regs.pc_commands2.negative_logic_CBC":1,
                        "user_wb_ttc_fmc_regs.pc_commands2.negative_logic_sTTS":0,
                        "user_wb_ttc_fmc_regs.pc_commands2.polarity_tlu":0
                    }
                }
                ]
            }
        ]
    },
        "Settings":{
            "RunNumber":1,
            "HoleMode":1
        }
}
```

### Known Issues:

Several bugs / problematic behavior has been reported by various users that is not direclty linked to the Ph2_ACF libraries, however, some workarounds are provided:

- When configuring a CBC object (writing all registers at once), the MSB of the Register "FrontEncControl" is read back incorrectly. This only manifests in electron mode (0xC3 instead of 0x43). The cause of this problem is identified as a FW artefact and the error itself can be safely ignored until the problem is solved. The chips will still properly configure and data quality should not be affected.

- uHAL exceptions and UDP timeouts when reading larger packet sizes from the GLIB board: 
this can happen for some users (cause not yet identified) but can be circumvented by changing the line

"ipbusudp-2.0://192.168.000.175:50001"

in the connections.xml file to

"chtcp-2.0://localhost:10203?target=192.168.000.175:50001"

& then launching the CACTUS control hub by the command:

/opt/cactus/bin/controlhub_start

This uses TCP protocol instead of UDP which accounts for packet loss but decreases the performance.

- SegmentationViolations on lines that contain

gStyle->Set ... ;

statements. This has been observed by several users on the VM and can be fixed by re-compiling ROOT using GCC 4.8


### Support, Suggestions ?

For any support/suggestions, mail georg.auzingerSPAMNOT@cern.ch
