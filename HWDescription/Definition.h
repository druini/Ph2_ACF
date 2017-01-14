/*

    \file                          Definition.h
    \brief                         Definition File, listing the registers
    \author                        Nicolas PIERRE
    \version                       1.0
    \date                          07/06/14
    Support :                      mail to : nico.pierre@icloud.com

 */



//-----------------------------------------------------------------------------
//Glib Config Files

#define XML_DESCRIPTION_FILE_2CBC "settings/ICDescription.xml"
#define XML_DESCRIPTION_FILE_4CBC "settings/HWDescription_2CBC.xml"
#define XML_DESCRIPTION_FILE_8CBC "settings/HWDescription_8CBC.xml"
#define XML_DESCRIPTION_FILE_16CBC "settings/Beamtest_Nov15.xml"


//Time out for stack writing
#define TIME_OUT         5


//------------------------------------------------------------------------------
//Events
#define NCHANNELS                   254

//in uint32_t words
#define CBC_EVENT_SIZE_32   9 // 9 32bit words per CBC
#define EVENT_HEADER_TDC_SIZE_32    6 // total of 6 32 bit words for HEADER + TDC
#define EVENT_HEADER_SIZE_32    5  // 5 words for the header

#define MPA_HEADER_SIZE_32   4099 
#define MPA_EVENT_SIZE_32   240 


//Event
#define OFFSET_BUNCH               8
#define WIDTH_BUNCH                24
#define OFFSET_ORBIT               1*32+8
#define WIDTH_ORBIT                24
#define OFFSET_LUMI                2*32+8
#define WIDTH_LUMI                 24
#define OFFSET_EVENT_COUNT         3*32+8
#define WIDTH_EVENT_COUNT          24
#define OFFSET_EVENT_COUNT_CBC     4*32+8
#define WIDTH_EVENT_COUNT_CBC      3*8

//Cbc Event
#define NCHANNELS                   254
#define OFFSET_ERROR              0
#define WIDTH_ERROR               2
#define OFFSET_PIPELINE_ADDRESS   2       //OFFSET_ERROR + WIDTH_ERROR
#define WIDTH_PIPELINE_ADDRESS    8
#define OFFSET_CBCDATA            2+8     //OFFSET_PIPELINE_ADDRESS + WIDTH_PIPELINE_ADDRESS
#define WIDTH_CBCDATA             254     //NCHANNELS
#define OFFSET_GLIBFLAG           10+254  //OFFSET_CBCDATA + WIDTH_CBCDATA
#define WIDTH_GLIBFLAG            12
#define OFFSET_CBCSTUBDATA        264+23  //LAST BIT
#define IC_OFFSET_CBCSTUBDATA        276  //BIT 12
#define WIDTH_CBCSTUBDATA         12
//------------------------------------------------------------------------------
