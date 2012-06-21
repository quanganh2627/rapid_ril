////////////////////////////////////////////////////////////////////////////
// channel_nd.h
//
// Copyright 2009 Intrinsyc Software International, Inc.  All rights reserved.
// Patents pending in the United States of America and other jurisdictions.
//
//
// Description:
//    Defines channel-related classes, constants, and structures.
//
/////////////////////////////////////////////////////////////////////////////


#ifndef RRIL_CHANNEL_ND_H
#define RRIL_CHANNEL_ND_H

#include "channelbase.h"

class CCommand;
class CResponse;

class CChannel : public CChannelBase
{
public:
    CChannel(UINT32 uiChannel);
    virtual ~CChannel();

private:
    //  Prevent assignment: Declared but not implemented.
    CChannel(const CChannel& rhs);  // Copy Constructor
    CChannel& operator=(const CChannel& rhs);  //  Assignment operator

public:
    //  Init functions
    virtual UINT32  CommandThread()  { return CChannelBase::CommandThread(); }
    virtual UINT32  ResponseThread() { return CChannelBase::ResponseThread(); }

    virtual void FlushResponse();

    /*
     * Goes through Tx queue, finds identical request IDs and completes
     * ril request with the provided result code and response.
     */
    virtual BOOL FindIdenticalRequestsAndSendResponses(UINT32 uiReqID,
                                                        UINT32 uiResultCode,
                                                        void* pResponse,
                                                        size_t responseLen);

protected:
    //  Init functions
    virtual BOOL    FinishInit() = 0;

    //  Silo-related functions
    virtual BOOL    AddSilos() = 0;

    //  Framework functions
    virtual BOOL            SendCommand(CCommand*& rpCmd);
    virtual RIL_RESULT_CODE GetResponse(CCommand*& rpCmd, CResponse*& rpRsp);
    virtual BOOL            ParseResponse(CCommand*& rpCmd, CResponse*& rpRsp);

    // Called at end of ResponseThread()
    // Give GPRS response thread a chance to handle Rx data in Data mode
    virtual BOOL    ProcessModemData(char *szData, UINT32 uiRead);

    //  Handle the timeout scenario (ABORT command, PING)
    virtual BOOL    HandleTimeout(CCommand*& rpCmd, CResponse*& rpRsp);

    //  Helper function to determine whether to send phase 2 of a command
    bool SendCommandPhase2(const UINT32 uiResCode, const UINT32 uiReqID) const;

private:
    // Helper functions
    RIL_RESULT_CODE ReadQueue(CResponse*& rpRsp, UINT32 uiTimeout);
    BOOL            ProcessResponse(CResponse*& rpRsp);
    BOOL            ProcessNoop(CResponse*& rpRsp);
    BOOL            RejectRadioOff(CResponse*& rpRsp);

    //  helper function to close and open the port.
    void            CloseOpenPort();

protected:
    CResponse*      m_pResponse;
};


#endif // RRIL_CHANNEL_ND_H

