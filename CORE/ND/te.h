////////////////////////////////////////////////////////////////////////////
// te_nd.h
//
// Copyright 2009 Intrinsyc Software International, Inc.  All rights reserved.
// Patents pending in the United States of America and other jurisdictions.
//
//
// Description:
//    Defines the CTE class which handles all overrides to requests and
//    basic behavior for responses for a specific modem
//
// Author:  Mike Worth
// Created: 2009-09-30
//
/////////////////////////////////////////////////////////////////////////////
//  Modification Log:
//
//  Date       Who      Ver   Description
//  ---------  -------  ----  -----------------------------------------------
//  Sept 30/09  FV      1.00  Established v1.00 based on current code base.
//
/////////////////////////////////////////////////////////////////////////////

#ifndef RRIL_TE_H
#define RRIL_TE_H

#include "te_base.h"
#include "rril.h"
#include "sync_ops.h"

class CTE
{
private:
    CTE();
    ~CTE();

    //  Prevent assignment: Declared but not implemented.
    CTE(const CTE& rhs);  // Copy Constructor
    CTE& operator=(const CTE& rhs);  //  Assignment operator


    static CTE*         m_pTEInstance;

    static const UINT32 m_uiMaxModemNameLen = 64;
    CTEBase*            m_pTEBaseInstance;

private:
    static CTEBase*     CreateModemTE();


public:
    static CTE & GetTE();
    static void  DeleteTEObject();

    // RIL_REQUEST_GET_SIM_STATUS 1
    RIL_RESULT_CODE RequestGetSimStatus(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGetSimStatus(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_ENTER_SIM_PIN 2
    RIL_RESULT_CODE RequestEnterSimPin(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseEnterSimPin(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_ENTER_SIM_PUK 3
    RIL_RESULT_CODE RequestEnterSimPuk(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseEnterSimPuk(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_ENTER_SIM_PIN2 4
    RIL_RESULT_CODE RequestEnterSimPin2(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseEnterSimPin2(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_ENTER_SIM_PUK2 5
    RIL_RESULT_CODE RequestEnterSimPuk2(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseEnterSimPuk2(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CHANGE_SIM_PIN 6
    RIL_RESULT_CODE RequestChangeSimPin(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseChangeSimPin(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CHANGE_SIM_PIN2 7
    RIL_RESULT_CODE RequestChangeSimPin2(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseChangeSimPin2(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION 8
    RIL_RESULT_CODE RequestEnterNetworkDepersonalization(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseEnterNetworkDepersonalization(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_GET_CURRENT_CALLS 9
    RIL_RESULT_CODE RequestGetCurrentCalls(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGetCurrentCalls(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_DIAL 10
    RIL_RESULT_CODE RequestDial(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseDial(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_GET_IMSI 11
    RIL_RESULT_CODE RequestGetImsi(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGetImsi(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_HANGUP 12
    RIL_RESULT_CODE RequestHangup(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseHangup(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_HANGUP_WAITING_OR_BACKGROUND 13
    RIL_RESULT_CODE RequestHangupWaitingOrBackground(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseHangupWaitingOrBackground(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND 14
    RIL_RESULT_CODE RequestHangupForegroundResumeBackground(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseHangupForegroundResumeBackground(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE 15
    // RIL_REQUEST_SWITCH_HOLDING_AND_ACTIVE 15
    RIL_RESULT_CODE RequestSwitchHoldingAndActive(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSwitchHoldingAndActive(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CONFERENCE 16
    RIL_RESULT_CODE RequestConference(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseConference(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_UDUB 17
    RIL_RESULT_CODE RequestUdub(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseUdub(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_LAST_CALL_FAIL_CAUSE 18
    RIL_RESULT_CODE RequestLastCallFailCause(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseLastCallFailCause(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SIGNAL_STRENGTH 19
    RIL_RESULT_CODE RequestSignalStrength(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSignalStrength(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_REGISTRATION_STATE 20
    RIL_RESULT_CODE RequestRegistrationState(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseRegistrationState(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_GPRS_REGISTRATION_STATE 21
    RIL_RESULT_CODE RequestGPRSRegistrationState(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGPRSRegistrationState(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_OPERATOR 22
    RIL_RESULT_CODE RequestOperator(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseOperator(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_RADIO_POWER 23
    RIL_RESULT_CODE RequestRadioPower(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseRadioPower(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_DTMF 24
    RIL_RESULT_CODE RequestDtmf(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseDtmf(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SEND_SMS 25
    RIL_RESULT_CODE RequestSendSms(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSendSms(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SEND_SMS_EXPECT_MORE 26
    RIL_RESULT_CODE RequestSendSmsExpectMore(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSendSmsExpectMore(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SETUP_DATA_CALL 27
    RIL_RESULT_CODE RequestSetupDataCall(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetupDataCall(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SIM_IO 28
    RIL_RESULT_CODE RequestSimIo(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSimIo(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SEND_USSD 29
    RIL_RESULT_CODE RequestSendUssd(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSendUssd(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CANCEL_USSD 30
    RIL_RESULT_CODE RequestCancelUssd(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCancelUssd(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_GET_CLIR 31
    RIL_RESULT_CODE RequestGetClir(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGetClir(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SET_CLIR 32
    RIL_RESULT_CODE RequestSetClir(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetClir(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_QUERY_CALL_FORWARD_STATUS 33
    RIL_RESULT_CODE RequestQueryCallForwardStatus(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseQueryCallForwardStatus(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SET_CALL_FORWARD 34
    RIL_RESULT_CODE RequestSetCallForward(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetCallForward(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_QUERY_CALL_WAITING 35
    RIL_RESULT_CODE RequestQueryCallWaiting(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseQueryCallWaiting(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SET_CALL_WAITING 36
    RIL_RESULT_CODE RequestSetCallWaiting(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetCallWaiting(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SMS_ACKNOWLEDGE 37
    RIL_RESULT_CODE RequestSmsAcknowledge(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSmsAcknowledge(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_GET_IMEI 38
    RIL_RESULT_CODE RequestGetImei(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGetImei(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_GET_IMEISV 39
    RIL_RESULT_CODE RequestGetImeisv(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGetImeisv(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_ANSWER 40
    RIL_RESULT_CODE RequestAnswer(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseAnswer(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_DEACTIVATE_DATA_CALL 41
    RIL_RESULT_CODE RequestDeactivateDataCall(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseDeactivateDataCall(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_QUERY_FACILITY_LOCK 42
    RIL_RESULT_CODE RequestQueryFacilityLock(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseQueryFacilityLock(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SET_FACILITY_LOCK 43
    RIL_RESULT_CODE RequestSetFacilityLock(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetFacilityLock(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CHANGE_BARRING_PASSWORD 44
    RIL_RESULT_CODE RequestChangeBarringPassword(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseChangeBarringPassword(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_QUERY_NETWORK_SELECTION_MODE 45
    RIL_RESULT_CODE RequestQueryNetworkSelectionMode(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseQueryNetworkSelectionMode(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SET_NETWORK_SELECTION_AUTOMATIC 46
    RIL_RESULT_CODE RequestSetNetworkSelectionAutomatic(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetNetworkSelectionAutomatic(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SET_NETWORK_SELECTION_MANUAL 47
    RIL_RESULT_CODE RequestSetNetworkSelectionManual(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetNetworkSelectionManual(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_QUERY_AVAILABLE_NETWORKS 48
    RIL_RESULT_CODE RequestQueryAvailableNetworks(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseQueryAvailableNetworks(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_DTMF_START 49
    RIL_RESULT_CODE RequestDtmfStart(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseDtmfStart(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_DTMF_STOP 50
    RIL_RESULT_CODE RequestDtmfStop(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseDtmfStop(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_BASEBAND_VERSION 51
    RIL_RESULT_CODE RequestBasebandVersion(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseBasebandVersion(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SEPARATE_CONNECTION 52
    RIL_RESULT_CODE RequestSeparateConnection(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSeparateConnection(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SET_MUTE 53
    RIL_RESULT_CODE RequestSetMute(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetMute(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_GET_MUTE 54
    RIL_RESULT_CODE RequestGetMute(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGetMute(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_QUERY_CLIP 55
    RIL_RESULT_CODE RequestQueryClip(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseQueryClip(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE 56
    RIL_RESULT_CODE RequestLastDataCallFailCause(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseLastDataCallFailCause(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_DATA_CALL_LIST 57
    RIL_RESULT_CODE RequestDataCallList(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseDataCallList(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_RESET_RADIO 58
    RIL_RESULT_CODE RequestResetRadio(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseResetRadio(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_OEM_HOOK_RAW 59
    RIL_RESULT_CODE RequestHookRaw(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseHookRaw(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_OEM_HOOK_STRINGS 60
    RIL_RESULT_CODE RequestHookStrings(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseHookStrings(RESPONSE_DATA & rRspData);
    RIL_RESULT_CODE ParseGetVersion(RESPONSE_DATA & rRspData);
    RIL_RESULT_CODE ParseGetRxGain(RESPONSE_DATA & rRspData);
    RIL_RESULT_CODE ParseSetRxGain(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SCREEN_STATE 61
    RIL_RESULT_CODE RequestScreenState(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseScreenState(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SET_SUPP_SVC_NOTIFICATION 62
    RIL_RESULT_CODE RequestSetSuppSvcNotification(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetSuppSvcNotification(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_WRITE_SMS_TO_SIM 63
    RIL_RESULT_CODE RequestWriteSmsToSim(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseWriteSmsToSim(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_DELETE_SMS_ON_SIM 64
    RIL_RESULT_CODE RequestDeleteSmsOnSim(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseDeleteSmsOnSim(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SET_BAND_MODE 65
    RIL_RESULT_CODE RequestSetBandMode(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetBandMode(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_QUERY_AVAILABLE_BAND_MODE 66
    RIL_RESULT_CODE RequestQueryAvailableBandMode(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseQueryAvailableBandMode(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_STK_GET_PROFILE 67
    RIL_RESULT_CODE RequestStkGetProfile(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseStkGetProfile(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_STK_SET_PROFILE 68
    RIL_RESULT_CODE RequestStkSetProfile(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseStkSetProfile(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_STK_SEND_ENVELOPE_COMMAND 69
    RIL_RESULT_CODE RequestStkSendEnvelopeCommand(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseStkSendEnvelopeCommand(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_STK_SEND_TERMINAL_RESPONSE 70
    RIL_RESULT_CODE RequestStkSendTerminalResponse(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseStkSendTerminalResponse(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_STK_HANDLE_CALL_SETUP_REQUESTED_FROM_SIM 71
    RIL_RESULT_CODE RequestStkHandleCallSetupRequestedFromSim(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseStkHandleCallSetupRequestedFromSim(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_EXPLICIT_CALL_TRANSFER 72
    RIL_RESULT_CODE RequestExplicitCallTransfer(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseExplicitCallTransfer(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE 73
    RIL_RESULT_CODE RequestSetPreferredNetworkType(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetPreferredNetworkType(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE 74
    RIL_RESULT_CODE RequestGetPreferredNetworkType(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGetPreferredNetworkType(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_GET_NEIGHBORING_CELL_IDS 75
    RIL_RESULT_CODE RequestGetNeighboringCellIDs(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGetNeighboringCellIDs(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SET_LOCATION_UPDATES 76
    RIL_RESULT_CODE RequestSetLocationUpdates(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetLocationUpdates(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_SET_SUBSCRIPTION 77
    RIL_RESULT_CODE RequestCdmaSetSubscription(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaSetSubscription(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_SET_ROAMING_PREFERENCE 78
    RIL_RESULT_CODE RequestCdmaSetRoamingPreference(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaSetRoamingPreference(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE 79
    RIL_RESULT_CODE RequestCdmaQueryRoamingPreference(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaQueryRoamingPreference(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SET_TTY_MODE 80
    RIL_RESULT_CODE RequestSetTtyMode(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetTtyMode(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_QUERY_TTY_MODE 81
    RIL_RESULT_CODE RequestQueryTtyMode(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseQueryTtyMode(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_SET_PREFERRED_VOICE_PRIVACY_MODE 82
    RIL_RESULT_CODE RequestCdmaSetPreferredVoicePrivacyMode(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaSetPreferredVoicePrivacyMode(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE 83
    RIL_RESULT_CODE RequestCdmaQueryPreferredVoicePrivacyMode(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaQueryPreferredVoicePrivacyMode(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_FLASH 84
    RIL_RESULT_CODE RequestCdmaFlash(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaFlash(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_BURST_DTMF 85
    RIL_RESULT_CODE RequestCdmaBurstDtmf(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaBurstDtmf(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_VALIDATE_AKEY 86
    RIL_RESULT_CODE RequestCdmaValidateAndWriteAkey(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaValidateAndWriteAkey(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_SEND_SMS 87
    RIL_RESULT_CODE RequestCdmaSendSms(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaSendSms(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_SMS_ACKNOWLEDGE 88
    RIL_RESULT_CODE RequestCdmaSmsAcknowledge(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaSmsAcknowledge(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_GSM_GET_BROADCAST_SMS_CONFIG 89
    RIL_RESULT_CODE RequestGsmGetBroadcastSmsConfig(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGsmGetBroadcastSmsConfig(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_GSM_SET_BROADCAST_SMS_CONFIG 90
    RIL_RESULT_CODE RequestGsmSetBroadcastSmsConfig(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGsmSetBroadcastSmsConfig(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_GSM_SMS_BROADCAST_ACTIVATION 91
    RIL_RESULT_CODE RequestGsmSmsBroadcastActivation(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGsmSmsBroadcastActivation(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_GET_BROADCAST_SMS_CONFIG 92
    RIL_RESULT_CODE RequestCdmaGetBroadcastSmsConfig(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaGetBroadcastSmsConfig(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_SET_BROADCAST_SMS_CONFIG 93
    RIL_RESULT_CODE RequestCdmaSetBroadcastSmsConfig(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaSetBroadcastSmsConfig(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_SMS_BROADCAST_ACTIVATION 94
    RIL_RESULT_CODE RequestCdmaSmsBroadcastActivation(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaSmsBroadcastActivation(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_SUBSCRIPTION 95
    RIL_RESULT_CODE RequestCdmaSubscription(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaSubscription(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_WRITE_SMS_TO_RUIM 96
    RIL_RESULT_CODE RequestCdmaWriteSmsToRuim(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaWriteSmsToRuim(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_CDMA_DELETE_SMS_ON_RUIM 97
    RIL_RESULT_CODE RequestCdmaDeleteSmsOnRuim(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseCdmaDeleteSmsOnRuim(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_DEVICE_IDENTITY 98
    RIL_RESULT_CODE RequestDeviceIdentity(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseDeviceIdentity(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_EXIT_EMERGENCY_CALLBACK_MODE 99
    RIL_RESULT_CODE RequestExitEmergencyCallbackMode(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseExitEmergencyCallbackMode(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_GET_SMSC_ADDRESS 100
    RIL_RESULT_CODE RequestGetSmscAddress(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseGetSmscAddress(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SET_SMSC_ADDRESS 101
    RIL_RESULT_CODE RequestSetSmscAddress(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSetSmscAddress(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_REPORT_SMS_MEMORY_STATUS 102
    RIL_RESULT_CODE RequestReportSmsMemoryStatus(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseReportSmsMemoryStatus(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_REPORT_STK_SERVICE_IS_RUNNING 103
    RIL_RESULT_CODE RequestReportStkServiceRunning(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseReportStkServiceRunning(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SIM_TRANSMIT_BASIC 104
    RIL_RESULT_CODE RequestSimTransmitBasic(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSimTransmitBasic(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SIM_OPEN_CHANNEL 105
    RIL_RESULT_CODE RequestSimOpenChannel(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSimOpenChannel(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SIM_CLOSE_CHANNEL 106
    RIL_RESULT_CODE RequestSimCloseChannel(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSimCloseChannel(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_SIM_TRANSMIT_CHANNEL 107
    RIL_RESULT_CODE RequestSimTransmitChannel(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseSimTransmitChannel(RESPONSE_DATA & rRspData);

#if defined(M2_FEATURE_ENABLED)
    // RIL_REQUEST_HANGUP_VT 108
    RIL_RESULT_CODE RequestHangupVT(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseHangupVT(RESPONSE_DATA & rRspData);

    // RIL_REQUEST_DIAL_VT 109
    RIL_RESULT_CODE RequestDialVT(RIL_Token rilToken, void * pData, size_t datalen);
    RIL_RESULT_CODE ParseDialVT(RESPONSE_DATA & rRspData);
#endif // M2_FEATURE_ENABLED

    // RIL_UNSOL_SIGNAL_STRENGTH  1009
    RIL_RESULT_CODE ParseUnsolicitedSignalStrength(RESPONSE_DATA & rRspData);

    // RIL_UNSOL_DATA_CALL_LIST_CHANGED  1010
    RIL_RESULT_CODE ParseDataCallListChanged(RESPONSE_DATA & rRspData);

    // REQ_ID_GETIPADDRESS
    RIL_RESULT_CODE ParseIpAddress(RESPONSE_DATA & rRspData);

    // REQ_ID_GETDNS
    RIL_RESULT_CODE ParseDns(RESPONSE_DATA & rRspData);

    // REQ_ID_QUERYPIN2
    RIL_RESULT_CODE ParseQueryPIN2(RESPONSE_DATA & rRspData);
};

#endif
