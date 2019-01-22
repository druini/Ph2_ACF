### Naming Convention

All files in the current folder have a certain naming convention:

> **d19c_fmc1_fmc2.bin**

where:
* fmc1 - describes what is plugged in the fmc1 (top, l12)
* fmc2 - describes what is plugged in the fmc2 (bottom, l8)

For example:
* d19c_2cbc3_none.bin - means that 2xCBC3 hybrid is connected to the FMC1 and nothing is connected to the FMC2
* d19c_8cbc3-1_8cbc3-2.bin - means that 8xCBC3 hybrid occupies both FMCs - 8xcbc3-1 is the main FMC
* d19c_8cbc2_dio5.bin - means that 8xCBC3 hybrid is connected to the FMC1 and DIO5 is connected to the FMC2 (optional) 

### Loading the Firmware

In order to load the firmware, please, run from the base folder:

> ```fpgaconfig -c settings/D19CDescription.xml -f ./firmware/d19c/name_in_folder.bin -i name_to_store.bin```

where:
* name_in_folder.bin - is the name of the file in the d19c folder
* name_to_store.bin - is name used to store binary on the microSD card

Use ```fpgaconfig -h``` to display help message.

