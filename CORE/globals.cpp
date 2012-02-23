////////////////////////////////////////////////////////////////////////////
// globals.cpp
//
// Copyright 2005-2009 Intrinsyc Software International, Inc.  All rights reserved.
// Patents pending in the United States of America and other jurisdictions.
//
//
// Description:
//    Defines some global variables.
//
/////////////////////////////////////////////////////////////////////////////

#include "types.h"
#include "globals.h"

///////////////////////////////////////////////////////////////////////////////
// Timeouts (in milliseconds)

const UINT32 TIMEOUT_INITIALIZATION_COMMAND = 5000;
const UINT32 TIMEOUT_API_DEFAULT            = 10000;
const UINT32 TIMEOUT_WAITFORINIT            = 10000;
const UINT32 TIMEOUT_THRESHOLDFORRETRY      = 10000;


///////////////////////////////////////////////////////////////////////////////
UINT32 g_TimeoutCmdInit = TIMEOUT_INITIALIZATION_COMMAND;
UINT32 g_TimeoutAPIDefault = TIMEOUT_API_DEFAULT;
UINT32 g_TimeoutWaitForInit = TIMEOUT_WAITFORINIT;
UINT32 g_TimeoutThresholdForRetry = TIMEOUT_THRESHOLDFORRETRY;

///////////////////////////////////////////////////////////////////////////////
char g_cTerminator = '\r';
char g_szNewLine[3] = "\r\n";

///////////////////////////////////////////////////////////////////////////////
// This global class instance tracks the radio state and handles notifications
CRadioState g_RadioState;

///////////////////////////////////////////////////////////////////////////////
// This global flag is used to cancel the pending chld requests in ril when
// the call is disconnected.
bool g_clearPendingChlds = false;

///////////////////////////////////////////////////////////////////////////////
// This global variable stores the initial value of the
// Modem Autonomous Fast Dormancy (MAFD) mode in the repository.
const int FAST_DORMANCY_MODE_DEFAULT = 2;
int g_nFastDormancyMode = FAST_DORMANCY_MODE_DEFAULT;

// Globals used for DSDS
char g_szDualSim[PROPERTY_VALUE_MAX];
