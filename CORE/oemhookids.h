////////////////////////////////////////////////////////////////////////////
// oemhookids.h
//
// Copyright 2009 Intrinsyc Software International, Inc.  All rights reserved.
// Patents pending in the United States of America and other jurisdictions.
//
//
// Description:
//    List of enums for specific OEM requests via API
//    RIL_REQUEST_OEM_HOOK_STRINGS API.
//
/////////////////////////////////////////////////////////////////////////////


#pragma once

#include "types.h"

#pragma pack(1)

/**
 * This enum details the additional requests (OEM) to pass to the RIL
 * via the RIL_REQUEST_OEM_HOOK_RAW, RIL_REQUEST_OEM_HOOK_STRINGS APIs.
 */

//  The first int (4 bytes) of the byte[] is the command ID.  The data follows.
//
// The command id allocation is the following:
//      0x00000001 -> 0x0000009F : Product Specific
//      0x000000A0 -> 0x000000CF : Platform Requests
//      0x000000D0 -> 0x000000FF : Platform Unsolicited
//      0x00050000 -> 0x000500FF : ENZO specific
//      0x00080000 -> 0x000800FF : ENZO specific
//      0x000A0000 -> 0x000A00FF : ENZO specific
//

///////////////////////////////////////////////////////////////////////////////
typedef struct TAG_OEM_HOOK_RAW_GAN_RIL
{
    int nCommand; //  Command ID
} sOEM_HOOK_RAW_GAN_RIL;

//
//  RIL_OEM_HOOK_RAW_GAN_RIL
//  Command ID = 0x00000001
//
//  This is reserved for future implementation.
//
const int RIL_OEM_HOOK_RAW_GAN_RIL = 0x00000001;

///////////////////////////////////////////////////////////////////////////////

//
//  RIL_OEM_HOOK_STRING_THERMAL_GET_SENSOR
//  Command ID = 0x000000A2
//
// "data" - Sensor Id
//              “0” = RF temperature sensor.
//              “1” = Baseband chip temperature sensor.
//              “3” = PCB temperature sensor.
// "response" - String containg the temperatures separated by space
//              "Filtered temperature Raw Temperature". Both temperature
//              are formated as a 4 digits number: "2300" = 23.00 celcius
//              degrees  (two digits after dot).
//
const int RIL_OEM_HOOK_STRING_THERMAL_GET_SENSOR = 0x000000A2;

///////////////////////////////////////////////////////////////////////////////

//
//  RIL_OEM_HOOK_STRING_THERMAL_SET_THRESHOLD
//  Command ID = 0x000000A3
//
// "data" - String containing the following values separated by a space
//          boolean activate
//                    true - Activates the threshold notification
//                    false - Deactivates the threshold notification
//          int sensorId
//                    “0” = RF temperature sensor.
//                    “1” = Baseband chip temperature sensor.
//                    “3” = PCB temperature sensor.
//          int minTemperature
//                    Temperature formated as a 4 digit number.
//                    "2300" = 23.00 celcius degrees  (two digits after dot)
//          int maxTemperature: Same as minTemperature
//
//  "response" = NULL
//
const int RIL_OEM_HOOK_STRING_THERMAL_SET_THRESHOLD = 0x000000A3;

///////////////////////////////////////////////////////////////////////////////

//
//  RIL_OEM_HOOK_STRING_SET_MODEM_AUTO_FAST_DORMANCY
//  Command ID = 0x000000A4
//
// "data" - String containing the following values separated by a space
//          boolean FDEnable
//                    true - Activates the autonomous fast dormancy
//                    false - Deactivates the autonomous fast dormancy
//          int delayTimer
//                    integer value in range of 1 to 60 (seconds)
//          int SCRI Timer
//                    integer value in range of 1 to 120 (seconds)
//  "response" = NULL
//
const int RIL_OEM_HOOK_STRING_SET_MODEM_AUTO_FAST_DORMANCY = 0x000000A4;

///////////////////////////////////////////////////////////////////////////////

//
//  RIL_OEM_HOOK_STRING_GET_ATR
//  Command ID = 0x000000A5
//
//  This command sends AT+XGATR to the modem.
//
//  "data" = NULL
//  "response" = Answer to Reset
//
const int RIL_OEM_HOOK_STRING_GET_ATR = 0x000000A5;

///////////////////////////////////////////////////////////////////////////////

//
//  RIL_OEM_HOOK_STRING_GET_GPRS_CELL_ENV
//  Command ID = 0x000000A6
//
//  This command sends AT+CGED to the modem.
//
//  "data" = NULL
//  "response" = string containing dump of GPRS cell environment
//
const int RIL_OEM_HOOK_STRING_GET_GPRS_CELL_ENV = 0x000000A6;

///////////////////////////////////////////////////////////////////////////////

//
//  RIL_OEM_HOOK_STRING_DEBUG_SCREEN_COMMAND
//  Command ID = 0x000000A7
//
//  This command sends AT+XCGEDPAGE to the modem.
//
//  "data" = An array of null-terminated UTF-8 strings:
//          ((const char **)data)[0] - mode
//               0 - one shot dump
//               3 - reset statistics of all pages (page_nr ignored)
//               4 - STOP the EM (page_nr ignored)
//
//          ((const char **)data)[1] - Number of response page (page_nr)
//
//  "response" = string containing dump of the cell environment debug screen
//
const int RIL_OEM_HOOK_STRING_DEBUG_SCREEN_COMMAND = 0x000000A7;

///////////////////////////////////////////////////////////////////////////////

//
//  RIL_OEM_HOOK_STRING_RELEASE_ALL_CALLS
//  Command ID = 0x000000A8
//
//  This command sends AT+CHLD=8 to the modem.
//
//  "data" = NULL
//  "response" = NULL
//
const int RIL_OEM_HOOK_STRING_RELEASE_ALL_CALLS = 0x000000A8;

///////////////////////////////////////////////////////////////////////////////

//
//  RIL_OEM_HOOK_STRING_GET_SMS_TRANSPORT_MODE
//  Command ID = 0x000000A9
//
//  This command retrieves the current SMS transport mode.
//
//  "data" = NULL
//  "response" - String containing the value of the service according to the
//               27.007 10.1.21
//               “0” = Packet Domain.
//               “1” = Circuit Switched.
//               “2” = Packet Domain Preferred.
//               “3” = Circuit Switched Preferred.
//
const int RIL_OEM_HOOK_STRING_GET_SMS_TRANSPORT_MODE = 0x000000A9;

///////////////////////////////////////////////////////////////////////////////

//
//  RIL_OEM_HOOK_STRING_SET_SMS_TRANSPORT_MODE
//  Command ID = 0x000000AA
//
//  This command sets the SMS transport mode.
//
// "data" - int service
//                    “0” = Packet Domain.
//                    “1” = Circuit Switched.
//                    “2” = Packet Domain Preferred.
//                    “3” = Circuit Switched Preferred.
//  "response" = NULL
//
const int RIL_OEM_HOOK_STRING_SET_SMS_TRANSPORT_MODE = 0x000000AA;

///////////////////////////////////////////////////////////////////////////////

//
//  RIL_OEM_HOOK_STRING_GET_RF_POWER_CUTBACK_TABLE
//  Command ID = 0x000000AB
//
//  This command retrieves the Output Power cutback/boost table set.
//
//  "data" = NULL
//  "response" - String containing the state of the Conducted/Radiated GPIO, the
//               TX Power offset table index and the state of the TX Power
//               Controller.
//
const int RIL_OEM_HOOK_STRING_GET_RF_POWER_CUTBACK_TABLE = 0x000000AB;

///////////////////////////////////////////////////////////////////////////////

//
//  RIL_OEM_HOOK_STRING_SET_RF_POWER_CUTBACK_TABLE
//  Command ID = 0x000000AC
//
//  This command sets the TX power offset table to be used for reduction/boost.
//
// "data" - int power offset table
//                    “0” = Table set #0.
//                    “1” = Table set #1.
//                    “2” = Table set #2.
//  "response" = NULL
//
const int RIL_OEM_HOOK_STRING_SET_RF_POWER_CUTBACK_TABLE = 0x000000AC;

///////////////////////////////////////////////////////////////////////////////

//
//  RIL_OEM_HOOK_STRING_IMS_REGISTRATION
//  Command ID = 0x000000AD
//
//  This command ask for IMS registration/unregistration.
//
// "data" - string containing an int value registration state request
//                    “0” = unregister.
//                    “1” = register.
//  "response" = NULL
//  Effective modification of the state is sent back through an unsolicited
//  response.
//
const int RIL_OEM_HOOK_STRING_IMS_REGISTRATION = 0x000000AD;

///////////////////////////////////////////////////////////////////////////////

//
//  RIL_OEM_HOOK_STRING_IMS_CONFIG
//  Command ID = 0x000000AE
//
//  This command ask for IMS apn configuration.
//
// "data" = An array of strings representing +XICFG parameters:
//          ((const char **)data)[0] - string parameter <IMS_APN>
//          ((const char **)data)[1] - string parameter <pcscf_address>
//          ((const char **)data)[2] - string parameter <pcscf_port>
//          ((const char **)data)[3] - string parameter <auth_mode>
//          ((const char **)data)[4] - string parameter <phone_context>
//          ((const char **)data)[5] - string parameter <localbreakout>
//          ((const char **)data)[6] - string parameter <xcap_apn>
//          ((const char **)data)[7] - string parameter <xcap_root_uri>
//          ((const char **)data)[8] - string parameter <xcap_username>
//          ((const char **)data)[9] - string parameter <xcap_password>
//  "response" = NULL
//
const int RIL_OEM_HOOK_STRING_IMS_CONFIG = 0x000000AE;

///////////////////////////////////////////////////////////////////////////////

#if defined(M2_DUALSIM_FEATURE_ENABLED)

//
//  RIL_OEM_HOOK_STRING_SWAP_PS
//  Command ID = 0x000000B2
//
//  This command sends AT+XRAT=8 to the modem.
//
//  "data" = NULL
//  "response" = Error code returned by AT command
//
const int RIL_OEM_HOOK_STRING_SWAP_PS = 0x000000B2;

#endif // M2_DUALSIM_FEATURE_ENABLED

///////////////////////////////////////////////////////////////////////////////

typedef struct TAG_OEM_HOOK_RAW_UNSOL_THERMAL_ALARM_IND
{
    int nCommand; //  Command ID
    int nSensorId; // sensor id
    int nTemp; // temperature
} sOEM_HOOK_RAW_UNSOL_THERMAL_ALARM_IND;

//
//  RIL_OEM_HOOK_RAW_UNSOL_THERMAL_ALARM_IND
//  Command ID = 0x000000D0
//
//  "data" = sOEM_HOOK_RAW_UNSOL_THERMAL_ALARM_IND
//  "response" = NULL
//
const int RIL_OEM_HOOK_RAW_UNSOL_THERMAL_ALARM_IND = 0x000000D0;

///////////////////////////////////////////////////////////////////////////////

#if defined(M2_DUALSIM_FEATURE_ENABLED)

//
//  OEM_HOOK_RAW_UNSOL_FAST_OOS_IND
//  Command ID = 0x000000D1
//
//  "data" is a byte[] containing the command id
//  data[0] = command id
//
const int RIL_OEM_HOOK_RAW_UNSOL_FAST_OOS_IND = 0x000000D1;

//
//  OEM_HOOK_RAW_UNSOL_IN_SERVICE_IND
//  Command ID = 0x000000D2
//
//  "data" is a byte[] containing the command id
//  data[0] = command id
//
const int RIL_OEM_HOOK_RAW_UNSOL_IN_SERVICE_IND = 0x000000D2;

///////////////////////////////////////////////////////////////////////////////

#endif // M2_DUALSIM_FEATURE_ENABLED

typedef struct TAG_OEM_HOOK_RAW_UNSOL_DATA_STATUS_IND
{
    int command; //  Command ID
    int status; // result code
} sOEM_HOOK_RAW_UNSOL_DATA_STATUS_IND;

//
//  OEM_HOOK_RAW_UNSOL_DATA_STATUS_IND
//  Command ID = 0x000000D3
//
//  "data" is sOEM_HOOK_RAW_UNSOL_DATA_STATUS_IND
//
const int RIL_OEM_HOOK_RAW_UNSOL_DATA_STATUS_IND = 0x000000D3;

///////////////////////////////////////////////////////////////////////////////

typedef struct TAG_OEM_HOOK_RAW_UNSOL_MT_CLASS_IND
{
    int command; //  Command ID
    int mt_class; // result code
} sOEM_HOOK_RAW_UNSOL_MT_CLASS_IND;

//
//  OEM_HOOK_RAW_UNSOL_MT_CLASS_IND
//  Command ID = 0x000000D4
//
//  "data" is sOEM_HOOK_RAW_UNSOL_MT_CLASS_IND
//
const int RIL_OEM_HOOK_RAW_UNSOL_MT_CLASS_IND = 0x000000D4;

///////////////////////////////////////////////////////////////////////////////

const int CRASHTOOL_NAME_SIZE         = 64;
const int CRASHTOOL_NB_DATA           = 6;
const int CRASHTOOL_BUFFER_SIZE       = 255;
const int CRASHTOOL_LARGE_BUFFER_SIZE = 512;

typedef struct TAG_OEM_HOOK_RAW_UNSOL_CRASHTOOL_EVENT_IND
{
    int command;        // Command ID
    int type;           // Event type (INFO, ERROR, STAT)
    int nameSize;
    char name[CRASHTOOL_NAME_SIZE];     // Event name
    int dataSize[CRASHTOOL_NB_DATA];    // Real size of data0 to data5
    char data0[CRASHTOOL_BUFFER_SIZE];  // Data size are based on the crashtool database
    char data1[CRASHTOOL_BUFFER_SIZE];
    char data2[CRASHTOOL_LARGE_BUFFER_SIZE];
    char data3[CRASHTOOL_BUFFER_SIZE];
    char data4[CRASHTOOL_LARGE_BUFFER_SIZE];
    char data5[CRASHTOOL_LARGE_BUFFER_SIZE];
} sOEM_HOOK_RAW_UNSOL_CRASHTOOL_EVENT_IND;

// Constant for the event type
const int CRASHTOOL_INFO  = 0;
const int CRASHTOOL_ERROR = 1;
const int CRASHTOOL_STATS = 3;

//
//  OEM_HOOK_RAW_UNSOL_CRASHTOOL_EVENT_IND
//  Command ID = 0x000000D5
//
//  "data" is sOEM_HOOK_RAW_UNSOL_CHRASTOOL_EVENT_IND
//
const int RIL_OEM_HOOK_RAW_UNSOL_CRASHTOOL_EVENT_IND = 0x000000D5;

///////////////////////////////////////////////////////////////////////////////

typedef struct TAG_OEM_HOOK_RAW_UNSOL_UNSOL_IMS_REG_STATUS
{
    int command; //  Command ID
    int status; // IMS registration status
} sOEM_HOOK_RAW_UNSOL_IMS_REG_STATUS;

//
//  OEM_HOOK_RAW_UNSOL_IMS_REG_STATUS
//  Command ID = 0x000000D6
//
//  "data" is sOEM_HOOK_RAW_UNSOL_IMS_REG_STATUS
//
const int RIL_OEM_HOOK_RAW_UNSOL_IMS_REG_STATUS = 0x000000D6;

///////////////////////////////////////////////////////////////////////////////

typedef struct TAG_OEM_HOOK_RAW_UNSOL_UNSOL_IMS_SUPPORT_STATUS
{
    int command; //  Command ID
    int status; // result code
} sOEM_HOOK_RAW_UNSOL_IMS_SUPPORT_STATUS;

//
//  OEM_HOOK_RAW_UNSOL_IMS_SUPPORT_STATUS
//  Command ID = 0x000000D7
//
//  "data" is sOEM_HOOK_RAW_UNSOL_IMS_SUPPORT_STATUS
//
const int RIL_OEM_HOOK_RAW_UNSOL_IMS_SUPPORT_STATUS = 0x000000D7;

#pragma pack()

/***********************************************************************/


