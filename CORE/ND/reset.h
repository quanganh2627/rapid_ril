////////////////////////////////////////////////////////////////////////////
// reset.h
//
// Copyright 2005-2011 Intrinsyc Software International, Inc.  All rights reserved.
// Patents pending in the United States of America and other jurisdictions.
//
//
// Description:
//    Implementation of modem reset.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef RRIL_RESET_H
#define RRIL_RESET_H

#include "rilqueue.h"
#include "mmgr_cli.h"

class CResetQueueNode
{
public:
    virtual ~CResetQueueNode() { /* none */ }
    virtual void Execute() = 0;
};

class CDeferThread
{
public:
    static BOOL Init();
    static BOOL QueueWork(CResetQueueNode* pNode, BOOL bNeedDeferring);

    static BOOL DequeueWork(CResetQueueNode*& pNode) { return m_pResetQueue->Dequeue(pNode); }
    static void Lock()   { CMutex::Lock(m_pThreadStartLock); }
    static void Unlock() { CMutex::Unlock(m_pThreadStartLock); }
    static void SetThreadFinished() { m_bIsThreadRunning = FALSE; }

private:
    static CRilQueue<CResetQueueNode*>* m_pResetQueue;
    static CMutex* m_pThreadStartLock;
    static BOOL m_bIsThreadRunning;
};

enum eRadioError
{
    eRadioError_ForceShutdown,      //  Critical error occured
    eRadioError_RequestCleanup,     //  General request clean up
    eRadioError_LowMemory,          //  Couldn't allocate memory
    eRadioError_ChannelDead,        //  Modem non-responsive
    eRadioError_InitFailure,        //  AT command init sequence failed
    eRadioError_OpenPortFailure     //  Couldn't open the tty ports successfully
};

const char* Print_eRadioError(eRadioError e);

void ModemResetUpdate();

void do_request_clean_up(eRadioError eError, UINT32 uiLineNum, const char* lpszFileName);
int ModemManagerEventHandler(mmgr_cli_event_t* param);

enum ePCache_Code
{
    PIN_NO_PIN_AVAILABLE = -4,
    PIN_WRONG_INTEGRITY = -3,
    PIN_INVALID_UICC = -2,
    PIN_NOK = -1,
    PIN_OK = 0
};

ePCache_Code PCache_Store_PIN(const char* szUICC, const char* szPIN);
ePCache_Code PCache_Get_PIN(const char* szUICC, char* szPIN);
ePCache_Code PCache_Clear();

ePCache_Code PCache_SetUseCachedPIN(bool bFlag);
bool PCache_GetUseCachedPIN();

const char szRIL_usecachedpin[] = "ril.usecachedpin";
const char szRIL_cachedpin[] = "ril.cachedpin";
const char szRIL_cacheduicc[] = "ril.cacheduicc";


#endif // RRIL_RESET_H

