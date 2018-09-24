# CMS Ph2 ACF (Acquisition & Control Framework) 

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

1. Install a gcc compiler version > 4.8 - on scientific linux you can obtain this by installing devtoolset-2 or devtoolset-2.1:

        $> sudo wget -O /etc/yum.repos.d/slc6-devtoolset.repo http://linuxsoft.cern.ch/cern/devtoolset/slc6-devtoolset.repo
        $> sudo yum install devtoolset-2
        $> . /opt/rh/devtoolset-2/enable   # add this to your .bashrc
        $> sudo ln -s /opt/rh/devtoolset-2/root/usr/bin/* /usr/local/bin/
        $> hash -r

    This should give you a more recent gcc (e.g. gcc 4.8.2)

        $> gcc --version

2. Alternatively you can use a gcc version > 4.8 from AFS
 
If you are working on CC7, gcc4.8 is the default compiler.

3. Install uHAL (latest version [Instructions](http://ipbus.web.cern.ch/ipbus/doc/user/html/software/install/yum.html), instructions below for v2.3):

        $> wget http://svnweb.cern.ch/trac/cactus/export/28265/tags/ipbus_sw/uhal_2_3_0/scripts/release/cactus.slc6.x86_64.repo 

    (You may need the --no-check-certificate)

        $> sudo cp cactus.slc6.x86_64.repo /etc/yum.repos.d/cactus.repo

    then

        $> sudo yum clean all
        $> sudo yum groupinstall uhal

4. Install CERN ROOT version 5.34.32(SLC6) or 6.10(CC7): [Instructions](http://root.cern.ch/drupal/content/installing-root-source) - make sure to use "fixed location installation" when building yourself. If root is installed on a CERN computer of virtual machine you can use:
       
        $> sudo yum install root
        $> sudo yum install root-net-http root-graf3d-gl root-physics root-montecarlo-eg root-graf3d-eve root-geom libusb-devel xorg-x11-xauth.x86_64

5. Install CMAKE > 2.8. On SLC6 the default is cmake 2.8

        $> sudo yum install cmake

6. On CC7 you also need to install boost v1.53 headers (default on this system) as they don't ship with uHAL any more:

        $> sudo yum install boost-devel

### The Ph2_ACF Software : 

Follow these instructions to install and compile the libraries:
(provided you installed the latest version of gcc, µHal,  mentioned above).

1. Clone the GitHub repo and run cmake
  
        $> git clone https://:@gitlab.cern.ch:8443/cms_tk_ph2/Ph2_ACF.git
        $> cd Ph2_ACF/build 
        $> cmake ..

2. Do a `make -jN` in the build/ directory or alternatively do `make -C build/ -jN` in the Ph2_ACF root directory.

3. Don't forget to `source setup.sh` to set all the environment variables correctly.


4. Launch 

        $> systemtest --help

    command if you want to test the parsing of the HWDescription.xml file.

5. Launch

        $> datatest --help

    command if you want to test if you can correctly read data

6. Launch

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

7. Launch

          $> commission --help

    to do latency & threshold scans

8. Launch 

          $> fpgaconfig --help

    to upload a new FW image to the GLIB

9. Launch

          $> miniDAQ --help

    to save binary data from the GLIB to file

10. Launch

          $> miniDQM --help

    to run the DQM code from the June '15 beamtest


##### Nota Bene:
When you write a register in the Glib or the Cbc, the corresponding map of the HWDescription object in memory is also updated, so that you always have an exact replica of the HW Status in the memory.

Register values are:
  - 8-bit unsigend integers for the CBCs that should be edited in hex notation, i.e. '0xFF'
  - 32-bit unsigned integers for the GLIB: decimal values

For debugging purpose, you can activate DEV_FLAG in the sources or in the Makefile and also activate the uHal log in RegManager.cc.


#### External Clock and Trigger:
Please see the D19C FW  [documentation](https://gitlab.cern.ch/cms_tk_ph2/d19c-firmware/blob/master/doc/Middleware_Short_Guide.md) for instructions on how to use external clock and trigger with the various FMCs (DIO5 and CBC3 FMC)


#### Example HWDescription.xml File with DIO5 support:

```xml

<?xml version="1.0" encoding="utf-8"?>
<HwDescription>
  <BeBoard Id="0" boardType="D19C" eventType="VR">
      <connection id="board" uri="chtcp-2.0://localhost:10203?target=192.168.1.81:50001" address_table="file://settings/address_tables/d19c_address_table.xml" />

    <Module FeId="0" FMCId="0" ModuleId="0" Status="1">
        <Global>
            <Settings threshold="550" latency="26"/>
            <TestPulse enable="0" polarity="0" amplitude="0xFF" channelgroup="0" delay="0" groundothers="1"/>
            <ClusterStub clusterwidth="4" ptwidth="3" layerswap="0" off1="0" off2="0" off3="0" off4="0"/>
            <Misc analogmux="0b00000" pipelogic="0" stublogic="0" or254="1" tpgclock="1" testclock="1" dll="4"/>
            <ChannelMask disable=""/>
        </Global>
        <CBC_Files path="./settings/CbcFiles/" />
        <CBC Id="0" configfile="CBC3_default.txt" />
        <CBC Id="1" configfile="CBC3_default.txt" />
    </Module>

    <SLink>
        <DebugMode type="FULL"/>
        <ConditionData type="I2C" Register="VCth1" FeId="0" CbcId="0"/>
        <ConditionData type="User" UID="0x80" FeId="0" CbcId="0"> 0x22 </ConditionData>
        <ConditionData type="HV" FeId="0" Sensor="2"> 250 </ConditionData>
        <ConditionData type="TDC" FeId="0xFF"/>
    </SLink>

    <!--CONFIG-->
    <Register name="clock_source">3</Register> <!-- 3 - default (internal oscillator), 2 - backplane, 0 - AMC13 -->
    <Register name="fc7_daq_cnfg">
	<!-- Clock control -->
	<Register name="clock">
	    <Register name="ext_clk_en"> 0 </Register>
	</Register>
        <!-- TTC -->
        <Register name="ttc">
            <Register name="ttc_enable"> 0 </Register>
        </Register>
        <!-- Fast Command Block -->
        <Register name="fast_command_block">
		<Register name="triggers_to_accept"> 0 </Register>
		<Register name="trigger_source"> 3 </Register>
		<Register name="user_trigger_frequency"> 1 </Register>
		<Register name="stubs_mask"> 1 </Register>
                <!--this is the delay for the stub trigger-->
		<Register name="stub_trigger_delay_value"> 0 </Register>
                <Register name="stub_trigger_veto_length"> 0 </Register>
		<Register name="test_pulse">
			<Register name="delay_after_fast_reset"> 50 </Register>
			<Register name="delay_after_test_pulse"> 200 </Register>
			<Register name="delay_before_next_pulse"> 400 </Register>
			<Register name="en_fast_reset"> 1 </Register>
			<Register name="en_test_pulse"> 1 </Register>
			<Register name="en_l1a"> 1 </Register>
		</Register>
                <Register name="ext_trigger_delay_value"> 50 </Register>
                <Register name="antenna_trigger_delay_value"> 200 </Register>
                <Register name="delay_between_two_consecutive"> 10 </Register>
                <Register name="misc">
                        <Register name="backpressure_enable"> 1 </Register>
                        <Register name="stubOR"> 1 </Register>
                        <Register name="initial_fast_reset_enable"> 0 </Register>
                </Register>
        </Register>
	<!-- I2C manager -->
        <Register name="command_processor_block">
	</Register>
	<!-- Phy Block -->
	<Register name="physical_interface_block">
		<Register name="i2c">
                	<Register name="frequency"> 4 </Register>
		</Register>
	</Register>
	<!-- Readout Block -->
    	<Register name="readout_block">
            <Register name="packet_nbr"> 99 </Register>
            <Register name="global">
		    <Register name="data_handshake_enable"> 1 </Register>
                    <Register name="int_trig_enable"> 0 </Register>
                    <Register name="int_trig_rate"> 0 </Register>
                    <Register name="trigger_type"> 0 </Register>
                    <Register name="data_type"> 0 </Register>
                    <!--this is what is commonly known as stub latency-->
                    <Register name="common_stubdata_delay"> 194 </Register>
            </Register>
    	</Register>
	<!-- DIO5 Block -->
	<Register name="dio5_block">
	    <Register name="dio5_en"> 0 </Register>
            <Register name="ch1">
                <Register name="out_enable"> 1 </Register>
                <Register name="term_enable"> 0 </Register>
                <Register name="threshold"> 0 </Register>
            </Register>
	    <Register name="ch2">
                <Register name="out_enable"> 0 </Register>
                <Register name="term_enable"> 1 </Register>
                <Register name="threshold"> 50 </Register>
            </Register>
	    <Register name="ch3">
                <Register name="out_enable"> 1 </Register>
                <Register name="term_enable"> 0 </Register>
                <Register name="threshold"> 0 </Register>
            </Register>
	    <Register name="ch4">
                <Register name="out_enable"> 0 </Register>
                <Register name="term_enable"> 1 </Register>
                <Register name="threshold"> 50 </Register>
            </Register>
	    <Register name="ch5">
                <Register name="out_enable"> 0 </Register>
                <Register name="term_enable"> 1 </Register>
                <Register name="threshold"> 50 </Register>
            </Register>
	</Register>
	<!-- TLU Block -->
	<Register name="tlu_block">
		<Register name="handshake_mode"> 2 </Register>
		<Register name="tlu_enabled"> 0 </Register>
	</Register>
    </Register>
  </BeBoard>

<Settings>

    <!--[>Calibration<]-->
    <Setting name="TargetVcth">0x78</Setting>
    <Setting name="TargetOffset">0x50</Setting>
    <Setting name="Nevents">50</Setting>
    <Setting name="TestPulsePotentiometer">0x00</Setting>
    <Setting name="HoleMode">0</Setting>
    <Setting name="VerificationLoop">1</Setting>

    <!--Signal Scan Fit-->
	  <Setting name="InitialVcth">0x78</Setting>
	  <Setting name="SignalScanStep">2</Setting>
    <Setting name="FitSignal">0</Setting>

</Settings>
</HwDescription>


```


### Known Issues:


- uHAL exceptions and UDP timeouts when reading larger packet sizes from the GLIB board: 
this can happen for some users (cause not yet identified) but can be circumvented by changing the line

"ipbusudp-2.0://192.168.000.175:50001"

in the connections.xml file to

"chtcp-2.0://localhost:10203?target=192.168.000.175:50001"

& then launching the CACTUS control hub by the command:

/opt/cactus/bin/controlhub_start

This uses TCP protocol instead of UDP which accounts for packet loss but decreases the performance.


### Support, Suggestions ?

For any support/suggestions, mail georg.auzingerSPAMNOT@cern.ch
