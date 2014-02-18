////////////////////////////////////////////////////////////////////////////
// te_xmm7160.cpp
//
// Copyright 2009 Intrinsyc Software International, Inc.  All rights reserved.
// Patents pending in the United States of America and other jurisdictions.
//
//
// Description:
//    Overlay for the IMC 7160 modem
//
/////////////////////////////////////////////////////////////////////////////

#include <wchar.h>
#include <math.h>

//  This is for socket-related calls.
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/gsmmux.h>
#include <cutils/properties.h>

#include "types.h"
#include "nd_structs.h"
#include "util.h"
#include "extract.h"
#include "rillog.h"
#include "te.h"
#include "sync_ops.h"
#include "command.h"
#include "te_xmm7160.h"
#include "rildmain.h"
#include "callbacks.h"
#include "oemhookids.h"
#include "repository.h"
#include "reset.h"
#include "data_util.h"
#include "init7160.h"

CTE_XMM7160::CTE_XMM7160(CTE& cte)
: CTE_XMM6360(cte)
{
    m_cte.SetDefaultPDNCid(DEFAULT_PDN_CID);
}

CTE_XMM7160::~CTE_XMM7160()
{
}

CInitializer* CTE_XMM7160::GetInitializer()
{
    RIL_LOG_VERBOSE("CTE_XMM7160::GetInitializer() - Enter\r\n");
    CInitializer* pRet = NULL;

    RIL_LOG_INFO("CTE_XMM7160::GetInitializer() - Creating CInit7160 initializer\r\n");
    m_pInitializer = new CInit7160();
    if (NULL == m_pInitializer)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::GetInitializer() - Failed to create a CInit7160 "
                "initializer!\r\n");
        goto Error;
    }

    pRet = m_pInitializer;

Error:
    RIL_LOG_VERBOSE("CTE_XMM7160::GetInitializer() - Exit\r\n");
    return pRet;
}

char* CTE_XMM7160::GetUnlockInitCommands(UINT32 uiChannelType)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::GetUnlockInitCommands() - Enter\r\n");

    char szInitCmd[MAX_BUFFER_SIZE] = {'\0'};
    char* pUnlockInitCmd = NULL;

    pUnlockInitCmd = CTE_XMM6360::GetUnlockInitCommands(uiChannelType);
    if (pUnlockInitCmd != NULL)
    {
        // copy base class init command
        CopyStringNullTerminate(szInitCmd, pUnlockInitCmd, sizeof(szInitCmd));
        free(pUnlockInitCmd);
    }

    if (RIL_CHANNEL_URC == uiChannelType)
    {
        char szConformanceProperty[PROPERTY_VALUE_MAX] = {'\0'};
        BOOL bConformance = FALSE;
        // read the conformance property
        property_get("persist.conformance", szConformanceProperty, NULL);
        bConformance =
                (0 == strncmp(szConformanceProperty, "true", PROPERTY_VALUE_MAX)) ? TRUE : FALSE;

        // read the property enabling ciphering
        CRepository repository;
        int uiEnableCipheringInd = 1;
        if (!repository.Read(g_szGroupModem, g_szEnableCipheringInd, uiEnableCipheringInd))
        {
            RIL_LOG_VERBOSE("CTE_XMM7160::GetUnlockInitCommands()- Repository read failed!\r\n");
        }

        ConcatenateStringNullTerminate(szInitCmd, sizeof(szInitCmd),
                (bConformance || (uiEnableCipheringInd != 0)) ? "|+XUCCI=1" : "|+XUCCI=0");
    }

    RIL_LOG_VERBOSE("CTE_XMM7160::GetUnlockInitCommands() - Exit\r\n");
    return strndup(szInitCmd, strlen(szInitCmd));
}

const char* CTE_XMM7160::GetRegistrationInitString()
{
    return "+CREG=3|+XREG=3|+CEREG=3";
}

const char* CTE_XMM7160::GetPsRegistrationReadString()
{
    if (m_cte.IsEPSRegistered())
    {
        return "AT+CEREG=3;+XREG=3;+CEREG?;+XREG?;+CEREG=0;+XREG=0;\r";
    }
    else
    {
        return "AT+XREG=3;+XREG?;+XREG=0\r";
    }
}

const char* CTE_XMM7160::GetScreenOnString()
{
    if (m_cte.IsSignalStrengthReportEnabled())
    {
        return "AT+CREG=3;+CGREG=0;+XREG=3;+CEREG=3;+XCESQ=1\r";
    }
    return "AT+CREG=3;+CGREG=0;+XREG=3;+CEREG=3\r";
}

const char* CTE_XMM7160::GetScreenOffString()
{
    if (m_cte.IsLocationUpdatesEnabled())
    {
        return m_cte.IsSignalStrengthReportEnabled() ?
                "AT+CGREG=1;+CEREG=1;+XREG=0;+XCESQ=0\r" : "AT+CGREG=1;+CEREG=1;+XREG=0\r";
    }
    else
    {
        return m_cte.IsSignalStrengthReportEnabled() ?
                "AT+CREG=1;+CGREG=1;+CEREG=1;+XREG=0;+XCESQ=0\r" :
                "AT+CREG=1;+CGREG=1;+CEREG=1;+XREG=0\r";
    }
}

const char* CTE_XMM7160::GetSignalStrengthReportingString()
{
    return "+XCESQ=1";
}

//
// RIL_REQUEST_SETUP_DATA_CALL 27
//
RIL_RESULT_CODE CTE_XMM7160::CoreSetupDataCall(REQUEST_DATA& rReqData,
       void* pData, UINT32 uiDataSize, UINT32& uiCID)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::CoreSetupDataCall() - Enter\r\n");

    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;
    char szIPV4V6[] = "IPV4V6";
    int nPapChap = 0; // no auth
    PdpData stPdpData;
    S_SETUP_DATA_CALL_CONTEXT_DATA* pDataCallContextData = NULL;
    CChannel_Data* pChannelData = NULL;
    int dataProfile = -1;
    int nEmergencyFlag = 0 ; // 1: emergency pdn

    RIL_LOG_INFO("CTE_XMM7160::CoreSetupDataCall() - uiDataSize=[%u]\r\n", uiDataSize);

    memset(&stPdpData, 0, sizeof(PdpData));

    // extract data
    stPdpData.szRadioTechnology = ((char**)pData)[0];  // not used
    stPdpData.szRILDataProfile  = ((char**)pData)[1];
    stPdpData.szApn             = ((char**)pData)[2];
    stPdpData.szUserName        = ((char**)pData)[3];
    stPdpData.szPassword        = ((char**)pData)[4];
    stPdpData.szPAPCHAP         = ((char**)pData)[5];

    pDataCallContextData =
            (S_SETUP_DATA_CALL_CONTEXT_DATA*)malloc(sizeof(S_SETUP_DATA_CALL_CONTEXT_DATA));
    if (NULL == pDataCallContextData)
    {
        goto Error;
    }

    dataProfile = atoi(stPdpData.szRILDataProfile);
    pChannelData = CChannel_Data::GetFreeChnlsRilHsi(uiCID, dataProfile);
    if (NULL == pChannelData)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CoreSetupDataCall() - "
                "****** No free data channels available ******\r\n");
        goto Error;
    }

    pDataCallContextData->uiCID = uiCID;

    RIL_LOG_INFO("CTE_XMM7160::CoreSetupDataCall() - stPdpData.szRadioTechnology=[%s]\r\n",
            stPdpData.szRadioTechnology);
    RIL_LOG_INFO("CTE_XMM7160::CoreSetupDataCall() - stPdpData.szRILDataProfile=[%s]\r\n",
            stPdpData.szRILDataProfile);
    RIL_LOG_INFO("CTE_XMM7160::CoreSetupDataCall() - stPdpData.szApn=[%s]\r\n", stPdpData.szApn);
    RIL_LOG_INFO("CTE_XMM7160::CoreSetupDataCall() - stPdpData.szUserName=[%s]\r\n",
            stPdpData.szUserName);
    RIL_LOG_INFO("CTE_XMM7160::CoreSetupDataCall() - stPdpData.szPassword=[%s]\r\n",
            stPdpData.szPassword);
    RIL_LOG_INFO("CTE_XMM7160::CoreSetupDataCall() - stPdpData.szPAPCHAP=[%s]\r\n",
            stPdpData.szPAPCHAP);

    // if user name is empty, always use no authentication
    if (stPdpData.szUserName == NULL || strlen(stPdpData.szUserName) == 0)
    {
        nPapChap = 0;    // No authentication
    }
    else
    {
        // PAP/CHAP auth type 3 (PAP or CHAP) is not supported. In this case if a
        // a username is defined we will use PAP for authentication.
        // Note: due to an issue in the Android/Fw (missing check of the username
        // length), if the authentication is not defined, it's the value 3 (PAP or
        // CHAP) which is sent to RRIL by default.
        nPapChap = atoi(stPdpData.szPAPCHAP);
        if (nPapChap == 3)
        {
            nPapChap = 1;    // PAP authentication

            RIL_LOG_INFO("CTE_XMM7160::CoreSetupDataCall() - New PAP/CHAP=[%d]\r\n", nPapChap);
        }
    }

    if (RIL_VERSION >= 4 && (uiDataSize >= (7 * sizeof(char*))))
    {
        stPdpData.szPDPType = ((char**)pData)[6]; // new in Android 2.3.4.
        RIL_LOG_INFO("CTE_XMM7160::CoreSetupDataCall() - stPdpData.szPDPType=[%s]\r\n",
                stPdpData.szPDPType);
    }

    if (dataProfile == RIL_DATA_PROFILE_EMERGENCY)
    {
        nEmergencyFlag = 1;
    }

    //
    //  IP type is passed in dynamically.
    if (NULL == stPdpData.szPDPType)
    {
        //  hard-code "IPV4V6" (this is the default)
        stPdpData.szPDPType = szIPV4V6;
    }

    //  dynamic PDP type, need to set XDNS parameter depending on szPDPType.
    //  If not recognized, just use IPV4V6 as default.
    if (0 == strcmp(stPdpData.szPDPType, "IP"))
    {
        if (!PrintStringNullTerminate(rReqData.szCmd1,
                sizeof(rReqData.szCmd1),
                "AT+CGDCONT=%d,\"%s\",\"%s\",,0,0,,%d;+XGAUTH=%d,%u,\"%s\",\"%s\";+XDNS=%d,1\r",
                uiCID, stPdpData.szPDPType,
                stPdpData.szApn, nEmergencyFlag, uiCID, nPapChap, stPdpData.szUserName,
                stPdpData.szPassword, uiCID))
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::CoreSetupDataCall() -"
                    " cannot create CGDCONT command, stPdpData.szPDPType\r\n");
            goto Error;
        }
    }
    else if (0 == strcmp(stPdpData.szPDPType, "IPV6"))
    {
        if (!PrintStringNullTerminate(rReqData.szCmd1,
                sizeof(rReqData.szCmd1),
                "AT+CGDCONT=%d,\"%s\",\"%s\",,0,0,,%d;+XGAUTH=%d,%u,\"%s\",\"%s\";+XDNS=%d,2\r",
                uiCID, stPdpData.szPDPType, stPdpData.szApn, nEmergencyFlag, uiCID, nPapChap,
                stPdpData.szUserName, stPdpData.szPassword, uiCID))
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::CoreSetupDataCall() -"
                    " cannot create CGDCONT command, stPdpData.szPDPType\r\n");
            goto Error;
        }
    }
    else if (0 == strcmp(stPdpData.szPDPType, "IPV4V6"))
    {
        //  XDNS=3 is not supported by the modem so two commands +XDNS=1
        //  and +XDNS=2 should be sent.
        if (!PrintStringNullTerminate(rReqData.szCmd1,
                sizeof(rReqData.szCmd1),
                "AT+CGDCONT=%d,\"IPV4V6\",\"%s\",,0,0,,%d;+XGAUTH=%u,%d,\"%s\","
                "\"%s\";+XDNS=%d,1;+XDNS=%d,2\r", uiCID,
                stPdpData.szApn, nEmergencyFlag, uiCID, nPapChap, stPdpData.szUserName,
                stPdpData.szPassword, uiCID, uiCID))
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::CoreSetupDataCall() -"
                    " cannot create CGDCONT command, stPdpData.szPDPType\r\n");
            goto Error;
        }
    }
    else
    {
        RIL_LOG_CRITICAL("CTE_INF_7160::CoreSetupDataCall() - Wrong PDP type\r\n");
        goto Error;
    }

    res = RRIL_RESULT_OK;

Error:
    if (RRIL_RESULT_OK != res)
    {
        free(pDataCallContextData);
    }
    else
    {
        pChannelData->SetDataState(E_DATA_STATE_INITING);

        rReqData.pContextData = (void*)pDataCallContextData;
        rReqData.cbContextData = sizeof(S_SETUP_DATA_CALL_CONTEXT_DATA);
    }

    RIL_LOG_VERBOSE("CTE_XMM7160::CoreSetupDataCall() - Exit\r\n");
    return res;
}

//
// RIL_REQUEST_SIGNAL_STRENGTH 19
//
RIL_RESULT_CODE CTE_XMM7160::CoreSignalStrength(REQUEST_DATA& rReqData,
        void* pData, UINT32 uiDataSize)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::CoreSignalStrength() - Enter\r\n");
    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;

    if (CopyStringNullTerminate(rReqData.szCmd1, "AT+XCESQ?\r", sizeof(rReqData.szCmd1)))
    {
        res = RRIL_RESULT_OK;
    }

    RIL_LOG_VERBOSE("CTE_XMM7160::CoreSignalStrength() - Exit\r\n");
    return res;
}

RIL_RESULT_CODE CTE_XMM7160::ParseSignalStrength(RESPONSE_DATA& rRspData)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::ParseSignalStrength() - Enter\r\n");

    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;
    RIL_SignalStrength_v6* pSigStrData = NULL;
    const char* pszRsp = rRspData.szResponse;

    pSigStrData = ParseXCESQ(pszRsp, FALSE);
    if (NULL == pSigStrData)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseSignalStrength() -"
                " Could not allocate memory for RIL_SignalStrength_v6.\r\n");
        goto Error;
    }

    rRspData.pData   = (void*)pSigStrData;
    rRspData.uiDataSize  = sizeof(RIL_SignalStrength_v6);

    res = RRIL_RESULT_OK;

Error:
    RIL_LOG_VERBOSE("CTE_XMM7160::ParseSignalStrength() - Exit\r\n");
    return res;
}

//
// RIL_REQUEST_DATA_REGISTRATION_STATE 21
//
RIL_RESULT_CODE CTE_XMM7160::CoreGPRSRegistrationState(REQUEST_DATA& rReqData,
        void* pData, UINT32 uiDataSize)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::CoreGPRSRegistrationState() - Enter\r\n");
    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;

    if (CopyStringNullTerminate(rReqData.szCmd1, GetPsRegistrationReadString(),
            sizeof(rReqData.szCmd1)))
    {
        res = RRIL_RESULT_OK;
    }

    RIL_LOG_VERBOSE("CTE_XMM7160::CoreGPRSRegistrationState() - Exit\r\n");
    return res;
}

RIL_RESULT_CODE CTE_XMM7160::ParseGPRSRegistrationState(RESPONSE_DATA& rRspData)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::ParseGPRSRegistrationState() - Enter\r\n");

    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;
    const char* pszRsp = rRspData.szResponse;
    const char* pszDummy;

    S_ND_GPRS_REG_STATUS psRegStatus;
    P_ND_GPRS_REG_STATUS pGPRSRegStatus = NULL;

    pGPRSRegStatus = (P_ND_GPRS_REG_STATUS)malloc(sizeof(S_ND_GPRS_REG_STATUS));
    if (NULL == pGPRSRegStatus)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseGPRSRegistrationState() -"
                " Could not allocate memory for a S_ND_GPRS_REG_STATUS struct.\r\n");
        goto Error;
    }
    memset(pGPRSRegStatus, 0, sizeof(S_ND_GPRS_REG_STATUS));

    if (FindAndSkipString(pszRsp, "+CEREG: ", pszDummy))
    {
        if (!m_cte.ParseCEREG(pszRsp, FALSE, psRegStatus))
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::ParseGPRSRegistrationState() - "
                    "ERROR in parsing CEREG response.\r\n");
            goto Error;
        }

        m_cte.StoreRegistrationInfo(&psRegStatus, E_REGISTRATION_TYPE_CEREG);
    }

    if (FindAndSkipString(pszRsp, "+XREG: ", pszDummy))
    {
        if (!m_cte.ParseXREG(pszRsp, FALSE, psRegStatus))
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::ParseGPRSRegistrationState() - "
                    "ERROR in parsing XREG response.\r\n");
            goto Error;
        }

        m_cte.StoreRegistrationInfo(&psRegStatus, E_REGISTRATION_TYPE_XREG);
    }

    m_cte.CopyCachedRegistrationInfo(pGPRSRegStatus, TRUE);

    rRspData.pData  = (void*)pGPRSRegStatus;
    rRspData.uiDataSize = sizeof(S_ND_GPRS_REG_STATUS_POINTERS);

    res = RRIL_RESULT_OK;

Error:
    if (RRIL_RESULT_OK != res)
    {
        free(pGPRSRegStatus);
        pGPRSRegStatus = NULL;
    }

    RIL_LOG_VERBOSE("CTE_XMM7160::ParseGPRSRegistrationState() - Exit\r\n");
    return res;
}

//
// RIL_REQUEST_DEACTIVATE_DATA_CALL 41
//
RIL_RESULT_CODE CTE_XMM7160::CoreDeactivateDataCall(REQUEST_DATA& rReqData,
                                                                void* pData,
                                                                UINT32 uiDataSize)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::CoreDeactivateDataCall() - Enter\r\n");
    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;

    char* pszCid = NULL;
    UINT32 uiCID = 0;
    const LONG REASON_RADIO_OFF = 1;
    const LONG REASON_PDP_RESET = 2;
    LONG reason = 0;

    if (uiDataSize < (1 * sizeof(char *)))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CoreDeactivateDataCall() -"
                " Passed data size mismatch. Found %d bytes\r\n", uiDataSize);
        goto Error;
    }

    if (NULL == pData)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CoreDeactivateDataCall() -"
                " Passed data pointer was NULL\r\n");
        goto Error;
    }

    RIL_LOG_INFO("CTE_XMM7160::CoreDeactivateDataCall() - uiDataSize=[%d]\r\n", uiDataSize);

    pszCid = ((char**)pData)[0];
    if (pszCid == NULL || '\0' == pszCid[0])
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CoreDeactivateDataCall() - pszCid was NULL\r\n");
        goto Error;
    }

    RIL_LOG_INFO("CTE_XMM7160::CoreDeactivateDataCall() - pszCid=[%s]\r\n", pszCid);

    //  Get CID as UINT32.
    if (sscanf(pszCid, "%u", &uiCID) == EOF)
    {
        // Error
        RIL_LOG_CRITICAL("CTE_XMM7160::CoreDeactivateDataCall() -  cannot convert %s to int\r\n",
                pszCid);
        goto Error;
    }

    if ((RIL_VERSION >= 4) && (uiDataSize >= (2 * sizeof(char *))))
    {
        reason = GetDataDeactivateReason(((char**)pData)[1]);
        RIL_LOG_INFO("CTE_XMM7160::CoreDeactivateDataCall() - reason=[%ld]\r\n", reason);
    }

    if (reason == REASON_RADIO_OFF || RIL_APPSTATE_READY != GetSimAppState())
    {
        // complete the request without sending the AT command to modem.
        res = RRIL_RESULT_OK_IMMEDIATE;
        DataConfigDown(uiCID, TRUE);
    }
    else if (reason != REASON_PDP_RESET && m_cte.IsEPSRegistered() && uiCID == DEFAULT_PDN_CID)
    {
        // complete the request without sending the AT command to modem.
        res = RRIL_RESULT_OK_IMMEDIATE;
        DataConfigDown(uiCID, FALSE);
    }
    else
    {
        res = CTE_XMM6260::CoreDeactivateDataCall(rReqData, pData, uiDataSize);
    }

Error:
    RIL_LOG_VERBOSE("CTE_XMM7160::CoreDeactivateDataCall() - Exit\r\n");
    return res;
}

//
// RIL_REQUEST_SET_PREFERRED_NETWORK_TYPE 73
//
RIL_RESULT_CODE CTE_XMM7160::CoreSetPreferredNetworkType(REQUEST_DATA& rReqData,
        void* pData, UINT32 uiDataSize)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::CoreSetPreferredNetworkType() - Enter\r\n");

    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;
    RIL_PreferredNetworkType networkType = PREF_NET_TYPE_LTE_GSM_WCDMA; //9
    UINT32 uiModeOfOperation = m_cte.GetModeOfOperation();

    RIL_LOG_INFO("CTE_XMM7160::CoreSetPreferredNetworkType() - "
            "Mode of Operation:%u.\r\n", uiModeOfOperation);

    if (NULL == pData)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CoreSetPreferredNetworkType() - Data "
                "pointer is NULL.\r\n");
        goto Error;
    }

    if (uiDataSize != sizeof(RIL_PreferredNetworkType*))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CoreSetPreferredNetworkType() - "
                "Invalid data size.\r\n");
        goto Error;
    }

    RIL_LOG_INFO("CTE_XMM7160::CoreSetPreferredNetworkType() - "
                 "Network type {%d} from framework.\r\n",
                 ((RIL_PreferredNetworkType*)pData)[0]);

    networkType = ((RIL_PreferredNetworkType*)pData)[0];

    // if network type already set, NO-OP this command
    if (m_currentNetworkType == networkType)
    {
        rReqData.szCmd1[0] = '\0';
        res = RRIL_RESULT_OK;
        RIL_LOG_INFO("CTE_XMM7160::CoreSetPreferredNetworkType() - "
                "Network type {%d} already set.\r\n", networkType);
        goto Error;
    }

    switch (networkType)
    {
        case PREF_NET_TYPE_GSM_WCDMA: // WCDMA Preferred
            RIL_LOG_VERBOSE("CTE_XMM7160::CoreSetPreferredNetworkType() - "
                            "WCDMA pref:XACT=3,1) - Enter\r\n");
            if (!CopyStringNullTerminate(rReqData.szCmd1, "AT+XACT=3,1\r",
                    sizeof(rReqData.szCmd1)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::HandleNetworkType() - Can't "
                        "construct szCmd1 networkType=%d\r\n", networkType);
                break;
            }
            res = RRIL_RESULT_OK;
            break;

        case PREF_NET_TYPE_GSM_ONLY: // GSM Only
            RIL_LOG_VERBOSE("CTE_XMM7160::CoreSetPreferredNetworkType() -"
                    "GSM only:XACT=0) - Enter\r\n");
            if (!CopyStringNullTerminate(rReqData.szCmd1, "AT+XACT=0\r",
                    sizeof(rReqData.szCmd1)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::HandleNetworkType() - Can't "
                        "construct szCmd1 networkType=%d\r\n", networkType);
                break;
            }
            res = RRIL_RESULT_OK;
            break;

        case PREF_NET_TYPE_WCDMA: // WCDMA Only
            RIL_LOG_VERBOSE("CTE_XMM7160::CoreSetPreferredNetworkType() - "
                    "WCDMA only:XACT=1) - Enter\r\n");
            if (!CopyStringNullTerminate(rReqData.szCmd1, "AT+XACT=1\r",
                    sizeof(rReqData.szCmd1)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::HandleNetworkType() - Can't "
                        "construct szCmd1 networkType=%d\r\n", networkType);
                break;
            }
            res = RRIL_RESULT_OK;
            break;

        case PREF_NET_TYPE_LTE_ONLY: // LTE Only
            RIL_LOG_VERBOSE("CTE_XMM7160::CoreSetPreferredNetworkType() - "
                    "LTE Only:XACT=2) - Enter\r\n");
            if (!CopyStringNullTerminate(rReqData.szCmd1, "AT+XACT=2;+CEMODE=2\r",
                    sizeof(rReqData.szCmd1)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::CoreSetPreferredNetworkType() - "
                        "Can't construct szCmd1 networkType=%d\r\n", networkType);
                goto Error;
            }
            break;

        /*
         * PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO value is received as a result
         * of the recovery mechanism in the framework.
         */
        case PREF_NET_TYPE_LTE_GSM_WCDMA: // LTE Preferred
        case PREF_NET_TYPE_GSM_WCDMA_CDMA_EVDO_AUTO:
            RIL_LOG_VERBOSE("CTE_XMM7160::CoreSetPreferredNetworkType() - "
                    "LTE,GSM,WCDMA:XACT=6,2,1) - Enter\r\n");
            if (!PrintStringNullTerminate(rReqData.szCmd1, sizeof(rReqData.szCmd1),
                    "AT+XACT=6,2,1;+CEMODE=%u\r", uiModeOfOperation))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::CoreSetPreferredNetworkType() - "
                    "Can't construct szCmd1 networkType=%d\r\n", networkType);
                goto Error;
            }
            break;

        case PREF_NET_TYPE_LTE_WCDMA: // LTE Preferred
            RIL_LOG_VERBOSE("CTE_XMM7160::CoreSetPreferredNetworkType() - "
                    "LTE,WCDMA:XACT=4,2) - Enter\r\n");
            if (!PrintStringNullTerminate(rReqData.szCmd1, sizeof(rReqData.szCmd1),
                    "AT+XACT=4,2;+CEMODE=%u\r", uiModeOfOperation))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::HandleNetworkType() - Can't "
                        "construct szCmd1 networkType=%d\r\n", networkType);
                goto Error;
            }
            break;

        default:
            RIL_LOG_CRITICAL("CTE_XMM7160::CoreSetPreferredNetworkType() - "
                    "Undefined rat code: %d\r\n", networkType);
            res = RIL_E_MODE_NOT_SUPPORTED;
            goto Error;
    }

    //  Set the context of this command to the network type we're attempting to set
    rReqData.pContextData = (void*)networkType;  // Store this as an int.

    res = RRIL_RESULT_OK;

Error:
    RIL_LOG_VERBOSE("CTE_XMM7160::CoreSetPreferredNetworkType() - "
                    "Exit:%d\r\n", res);
    return res;
}

// RIL_REQUEST_GET_PREFERRED_NETWORK_TYPE 74
//
RIL_RESULT_CODE CTE_XMM7160::CoreGetPreferredNetworkType(REQUEST_DATA& rReqData,
        void* pData, UINT32 uiDataSize)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::CoreGetPreferredNetworkType() - Enter\r\n");

    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;

    if (CopyStringNullTerminate(rReqData.szCmd1, "AT+XACT?\r",
            sizeof(rReqData.szCmd1)))
    {
        res = RRIL_RESULT_OK;
    }

    RIL_LOG_VERBOSE("CTE_XMM7160::CoreGetPreferredNetworkType() - Exit\r\n");
    return res;
}

RIL_RESULT_CODE CTE_XMM7160::ParseGetPreferredNetworkType(RESPONSE_DATA& rRspData)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::ParseGetPreferredNetworkType() - Enter\r\n");

    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;
    const char* pszRsp = rRspData.szResponse;

    UINT32 rat = 0;
    UINT32 pref = 0;

    int* pRat = (int*)malloc(sizeof(int));
    if (NULL == pRat)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseGetPreferredNetworkType() - Could "
                "not allocate memory for response.\r\n");
        goto Error;
    }

    // Skip "<prefix>"
    if (!SkipRspStart(pszRsp, m_szNewLine, pszRsp))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseGetPreferredNetworkType() - Could "
                "not skip response prefix.\r\n");
        goto Error;
    }

    // Skip "+XACT: "
    if (!SkipString(pszRsp, "+XACT: ", pszRsp))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseGetPreferredNetworkType() - Could "
                "not skip \"+XACT: \".\r\n");
        goto Error;
    }

    if (!ExtractUInt32(pszRsp, rat, pszRsp))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseGetPreferredNetworkType() - Could "
                "not extract rat value.\r\n");
        goto Error;
    }

    if (FindAndSkipString(pszRsp, ",", pszRsp))
    {
        if (!ExtractUInt32(pszRsp, pref, pszRsp))
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::ParseGetPreferredNetworkType() - "
                    "Could not find and skip pref value even though it was expected.\r\n");
            goto Error;
        }
    }

    switch (rat)
    {
        case 0:     // GSM Only
        {
            pRat[0] = PREF_NET_TYPE_GSM_ONLY;
            m_currentNetworkType = PREF_NET_TYPE_GSM_ONLY;
            break;
        }

        case 1:     // WCDMA Only
        {
            pRat[0] = PREF_NET_TYPE_WCDMA;
            m_currentNetworkType = PREF_NET_TYPE_WCDMA;
            break;
        }

        case 2:     // LTE only
        {
            pRat[0] = PREF_NET_TYPE_LTE_ONLY;
            m_currentNetworkType = PREF_NET_TYPE_LTE_ONLY;
            break;
        }

        case 3:     // WCDMA preferred
        {
            pRat[0] = PREF_NET_TYPE_GSM_WCDMA;
            m_currentNetworkType = PREF_NET_TYPE_GSM_WCDMA;
            break;
        }

        case 4:     // LTE/WCDMA, LTE preferred
        {
            pRat[0] = PREF_NET_TYPE_LTE_WCDMA;
            m_currentNetworkType = PREF_NET_TYPE_LTE_WCDMA;
            break;
        }

        case 6:     // LTE/WCDMA/GSM, LTE preferred
        {
            pRat[0] = PREF_NET_TYPE_LTE_GSM_WCDMA;
            m_currentNetworkType = PREF_NET_TYPE_LTE_GSM_WCDMA;
            break;
        }

        default:
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::ParseGetPreferredNetworkType() - "
                    "Unexpected rat found: %d. Failing out.\r\n", rat);
            goto Error;
        }
    }

    rRspData.pData  = (void*)pRat;
    rRspData.uiDataSize = sizeof(int*);

    res = RRIL_RESULT_OK;

Error:
    if (RRIL_RESULT_OK != res)
    {
        free(pRat);
        pRat = NULL;
    }

    RIL_LOG_VERBOSE("CTE_XMM7160::ParseGetPreferredNetworkType() - Exit\r\n");
    return res;
}

BOOL CTE_XMM7160::IMSRegister(REQUEST_DATA& rReqData, void* pData,
        UINT32 uiDataSize)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::IMSRegister() - Enter\r\n");

    BOOL bRet = FALSE;

    if ((NULL == pData) || (sizeof(int) != uiDataSize))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::IMSRegister() - Invalid input data\r\n");
        return bRet;
    }

    int* pService = (int*)pData;

    if (!PrintStringNullTerminate(rReqData.szCmd1, sizeof(rReqData.szCmd1),
                "AT+XIREG=%d\r", *pService))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::IMSRegister() - Can't construct szCmd1.\r\n");
        return bRet;
    }

    int temp = 0;
    const int DEFAULT_XIREG_TIMEOUT = 180000;
    CRepository repository;

    if (repository.Read(g_szGroupOtherTimeouts, g_szTimeoutWaitForXIREG, temp))
    {
        rReqData.uiTimeout = temp;
    }
    else
    {
        rReqData.uiTimeout = DEFAULT_XIREG_TIMEOUT;
    }

    bRet = TRUE;

    return bRet;
}

RIL_RESULT_CODE CTE_XMM7160::ParseIMSRegister(RESPONSE_DATA& rRspData)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::ParseIMSRegister() - Enter\r\n");
    RIL_RESULT_CODE res = RRIL_RESULT_OK;
    RIL_LOG_VERBOSE("CTE_XMM7160::ParseIMSRegister() - Exit\r\n");
    return res;
}

RIL_RESULT_CODE CTE_XMM7160::CreateIMSRegistrationReq(REQUEST_DATA& rReqData,
        const char** pszRequest,
        const UINT32 uiDataSize)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::CreateIMSRegistrationReq() - Enter\r\n");
    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;

    if (pszRequest == NULL || '\0' == pszRequest[1])
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSRegistrationReq() - pszRequest was empty\r\n");
        goto Error;
    }

    if (uiDataSize < (2 * sizeof(char*)))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSRegistrationReq() :"
                " received_size < required_size\r\n");
        goto Error;
    }

    int service;
    if (sscanf(pszRequest[1], "%d", &service) == EOF)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSRegistrationReq() -"
                " cannot convert %s to int\r\n", pszRequest);
        goto Error;
    }

    if ((service < 0) || (service > 1))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSRegistrationReq() -"
                " service %s out of boundaries\r\n", service);
        goto Error;
    }

    RIL_LOG_INFO("CTE_XMM7160::CreateIMSRegistrationReq() - service=[%d]\r\n", service);

    if (!IMSRegister(rReqData, &service, sizeof(int)))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSRegistrationReq() - Can't construct szCmd1.\r\n");
        goto Error;
    }

    res = RRIL_RESULT_OK;
Error:
    RIL_LOG_VERBOSE("CTE_XMM7160::CreateIMSRegistrationReq() - Exit\r\n");
    return res;
}

RIL_RESULT_CODE CTE_XMM7160::CreateIMSConfigReq(REQUEST_DATA& rReqData,
        const char** pszRequest,
        const int nNumStrings)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::CreateIMSConfigReq() - Enter\r\n");
    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;
    char szXicfgCmd[MAX_BUFFER_SIZE] = {'\0'};
    int xicfgParams[XICFG_N_PARAMS] = {0};

    if (pszRequest == NULL)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSConfigReq() - pszRequest was empty\r\n");
        return res;
    }

    // There should be XICFG_N_PARAMS parameters and the request ID
    if (nNumStrings < (XICFG_N_PARAMS + 1))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSConfigReq() :"
                " received_size < required_size\r\n");
        return res;
    }

    xicfgParams[0] = E_XICFG_IMS_APN;
    xicfgParams[1] = E_XICFG_PCSCF_ADDRESS;
    xicfgParams[2] = E_XICFG_PCSCF_PORT;
    xicfgParams[3] = E_XICFG_IMS_AUTH_MODE;
    xicfgParams[4] = E_XICFG_PHONE_CONTEXT;
    xicfgParams[5] = E_XICFG_LOCAL_BREAKOUT;
    xicfgParams[6] = E_XICFG_XCAP_APN;
    xicfgParams[7] = E_XICFG_XCAP_ROOT_URI;
    xicfgParams[8] = E_XICFG_XCAP_USER_NAME;
    xicfgParams[9] = E_XICFG_XCAP_USER_PASSWORD;

    char szTemp1Xicfg[MAX_BUFFER_SIZE] = {'\0'};
    char szTemp2Xicfg[MAX_BUFFER_SIZE] = {'\0'};
    int nParams = 0;

    for (int i = 1; i <= XICFG_N_PARAMS; i++)
    {
        if ((pszRequest[i] != NULL) && (0 != strncmp(pszRequest[i], "default", 7)))
        {   // The XICFG parameter is a numeric hence "" not required.
            if (xicfgParams[i - 1] == E_XICFG_PCSCF_PORT ||
                xicfgParams[i - 1] == E_XICFG_IMS_AUTH_MODE ||
                xicfgParams[i - 1] == E_XICFG_LOCAL_BREAKOUT)
            {
                if (!PrintStringNullTerminate(szTemp1Xicfg,
                                             MAX_BUFFER_SIZE,",%d,%s",
                                             xicfgParams[i - 1], pszRequest[i]))
                {
                    RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSConfigReq() - Can't add %s.\r\n",
                                     pszRequest[i]);
                    goto Error;
                }
            }
            else
            {   // The XICFG parameter is a string hence "" required.
                if (!PrintStringNullTerminate(szTemp1Xicfg,
                                              MAX_BUFFER_SIZE,",%d,\"%s\"",
                                              xicfgParams[i - 1], pszRequest[i]))
                {
                    RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSConfigReq() - Can't add %s.\r\n",
                                     pszRequest[i]);
                    goto Error;
                }
            }
            if (!ConcatenateStringNullTerminate(szTemp2Xicfg, sizeof(szTemp2Xicfg), szTemp1Xicfg))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSConfigReq() - Can't add %s.\r\n",
                                 szTemp1Xicfg);
                goto Error;
            }
            nParams++;
        }
    }

    if (nParams == 0)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSConfigReq() - nParams=0\r\n");
        goto Error;
    }

    if (!ConcatenateStringNullTerminate(szTemp2Xicfg, sizeof(szTemp2Xicfg), "\r"))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSConfigReq() - Can't add %s.\r\n",
                         "\r");
        goto Error;
    }

    if (!PrintStringNullTerminate(szXicfgCmd,
            MAX_BUFFER_SIZE,"AT+XICFG=%d,%d", XICFG_SET, nParams))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSConfigReq() - Can't construct szCmd1.\r\n");
        goto Error;
    }

    if (!ConcatenateStringNullTerminate(szXicfgCmd, sizeof(szXicfgCmd), szTemp2Xicfg))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSConfigReq() - Can't construct szCmd1.\r\n");
        goto Error;
    }

    RIL_LOG_INFO("CTE_XMM7160::CreateIMSConfigReq() - IMS_APN=[%s]\r\n",
                 szXicfgCmd);

    if (!PrintStringNullTerminate(rReqData.szCmd1, sizeof(rReqData.szCmd1), szXicfgCmd))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateIMSConfigReq() - Can't construct szCmd1.\r\n");
        goto Error;
    }

    res = RRIL_RESULT_OK;
Error:
    RIL_LOG_VERBOSE("CTE_XMM7160::CreateIMSConfigReq() - Exit\r\n");
    return res;
}

BOOL CTE_XMM7160::QueryIpAndDns(REQUEST_DATA& rReqData, UINT32 uiCID)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::QueryIpAndDns() - Enter\r\n");
    BOOL bRet = FALSE;

    if (uiCID != 0)
    {
        if (PrintStringNullTerminate(rReqData.szCmd1, sizeof(rReqData.szCmd1),
                "AT+CGCONTRDP=%u\r", uiCID))
        {
            bRet = TRUE;
        }
    }

    RIL_LOG_VERBOSE("CTE_XMM7160::QueryIpAndDns() - Exit\r\n");
    return bRet;
}

RIL_RESULT_CODE CTE_XMM7160::ParseQueryIpAndDns(RESPONSE_DATA& rRspData)
{
    return ParseReadContextParams(rRspData);
}

RIL_RESULT_CODE CTE_XMM7160::HandleSetupDefaultPDN(RIL_Token rilToken,
        CChannel_Data* pChannelData)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::HandleSetupDefaultPDN() - Enter\r\n");

    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;
    char* szModemResourceName = {'\0'};
    int muxControlChannel = -1;
    int hsiChannel = pChannelData->GetHSIChannel();
    int ipcDataChannelMin = 0;
    UINT32 uiRilChannel = pChannelData->GetRilChannel();
    REQUEST_DATA reqData;
    S_SETUP_DATA_CALL_CONTEXT_DATA* pDataCallContextData = NULL;
    CCommand* pCmd = NULL;
    int dataState = pChannelData->GetDataState();

    pDataCallContextData =
            (S_SETUP_DATA_CALL_CONTEXT_DATA*)malloc(sizeof(S_SETUP_DATA_CALL_CONTEXT_DATA));
    if (NULL == pDataCallContextData)
    {
        goto Error;
    }

    pDataCallContextData->uiCID = pChannelData->GetContextID();

    // Get the mux channel id corresponding to the control of the data channel
    switch (uiRilChannel)
    {
        case RIL_CHANNEL_DATA1:
            sscanf(g_szDataPort1, "/dev/gsmtty%d", &muxControlChannel);
            break;
        case RIL_CHANNEL_DATA2:
            sscanf(g_szDataPort2, "/dev/gsmtty%d", &muxControlChannel);
            break;
        case RIL_CHANNEL_DATA3:
            sscanf(g_szDataPort3, "/dev/gsmtty%d", &muxControlChannel);
            break;
        case RIL_CHANNEL_DATA4:
            sscanf(g_szDataPort4, "/dev/gsmtty%d", &muxControlChannel);
            break;
        case RIL_CHANNEL_DATA5:
            sscanf(g_szDataPort5, "/dev/gsmtty%d", &muxControlChannel);
            break;
        default:
            RIL_LOG_CRITICAL("CTE_XMM7160::HandleSetupDefaultPDN() - Unknown mux channel"
                    "for RIL Channel [%u] \r\n", uiRilChannel);
            goto Error;
    }

    szModemResourceName = pChannelData->GetModemResourceName();
    ipcDataChannelMin = pChannelData->GetIpcDataChannelMin();

    if (ipcDataChannelMin > hsiChannel || RIL_MAX_NUM_IPC_CHANNEL <= hsiChannel )
    {
       RIL_LOG_CRITICAL("CTE_XMM7160::HandleSetupDefaultPDN() - Unknown HSI Channel [%d] \r\n",
                hsiChannel);
       goto Error;
    }

    memset(&reqData, 0, sizeof(REQUEST_DATA));

    if (E_DATA_STATE_ACTIVATING == dataState)
    {
        /*
         * ACTIVATING means that context is up but the routing is not enabled and also channel
         * is not configured for Data.
         */
        if (!PrintStringNullTerminate(reqData.szCmd1, sizeof(reqData.szCmd1),
                "AT+XDATACHANNEL=1,1,\"/mux/%d\",\"/%s/%d\",0,%d;+CGDATA=\"M-RAW_IP\",%d\r",
                muxControlChannel, szModemResourceName, hsiChannel,
                pDataCallContextData->uiCID, pDataCallContextData->uiCID))
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::HandleSetupDefaultPDN() - cannot create XDATACHANNEL"
                    "command\r\n");
            goto Error;
        }
    }
    else if (E_DATA_STATE_ACTIVE == dataState)
    {
        // Dont send any command to modem. Default PDN is already active
    }

    pCmd = new CCommand(uiRilChannel, rilToken, REQ_ID_NONE, reqData,
            &CTE::ParseSetupDefaultPDN, &CTE::PostSetupDefaultPDN);

    if (pCmd)
    {
        pCmd->SetContextData(pDataCallContextData);
        pCmd->SetContextDataSize(sizeof(S_SETUP_DATA_CALL_CONTEXT_DATA));

        if (!CCommand::AddCmdToQueue(pCmd))
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::HandleSetupDefaultPDN() - "
                    "Unable to add command to queue\r\n");
            delete pCmd;
            pCmd = NULL;
            goto Error;
        }
    }
    else
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::HandleSetupDefaultPDN() -"
                " Unable to allocate memory for command\r\n");
        goto Error;
    }

    res = RRIL_RESULT_OK;

Error:
    if (RRIL_RESULT_OK != res)
    {
        free(pDataCallContextData);
        pDataCallContextData = NULL;
    }

    RIL_LOG_VERBOSE("CTE_XMM7160::HandleSetupDefaultPDN() - Exit\r\n");
    return res;
}

RIL_RESULT_CODE CTE_XMM7160::ParseSetupDefaultPDN(RESPONSE_DATA& rRspData)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::ParseSetupDefaultPDN() - Enter\r\n");

    RIL_LOG_VERBOSE("CTE_XMM7160::ParseSetupDefaultPDN() - Exit\r\n");
    return RRIL_RESULT_OK;
}

void CTE_XMM7160::PostSetupDefaultPDN(POST_CMD_HANDLER_DATA& rData)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::PostSetupDefaultPDN() Enter\r\n");

    BOOL bSuccess = FALSE;
    S_SETUP_DATA_CALL_CONTEXT_DATA* pDataCallContextData = NULL;
    UINT32 uiCID = 0;
    CChannel_Data* pChannelData = NULL;

    if (NULL == rData.pContextData ||
            sizeof(S_SETUP_DATA_CALL_CONTEXT_DATA) != rData.uiContextDataSize)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::PostSetupDefaultPDN() - Invalid context data\r\n");
        goto Error;
    }

    pDataCallContextData = (S_SETUP_DATA_CALL_CONTEXT_DATA*)rData.pContextData;
    uiCID = pDataCallContextData->uiCID;

    if (RIL_E_SUCCESS != rData.uiResultCode)
    {
        RIL_LOG_INFO("CTE_XMM7160::PostSetupDefaultPDN() - Failure\r\n");
        goto Error;
    }

    RIL_LOG_INFO("CTE_XMM7160::PostSetupDefaultPDN() - CID=%d\r\n", uiCID);

    pChannelData = CChannel_Data::GetChnlFromContextID(uiCID);
    if (NULL == pChannelData)
    {
        RIL_LOG_INFO("CTE_XMM7160::PostSetupDefaultPDN() -"
                " No Data Channel for CID %u.\r\n", uiCID);
        goto Error;
    }

    RIL_LOG_VERBOSE("CTE_XMM7160::PostSetupDefaultPDN() set channel data\r\n");

    pChannelData->SetDataState(E_DATA_STATE_ACTIVE);

    if (!SetupInterface(uiCID))
    {
        RIL_LOG_INFO("CTE_XMM7160::PostSetupDefaultPDN() - SetupInterface failed\r\n");
        goto Error;
    }

    bSuccess = TRUE;

Error:
    free(rData.pContextData);
    rData.pContextData = NULL;

    if (!bSuccess)
    {
        HandleSetupDataCallFailure(uiCID, rData.pRilToken, rData.uiResultCode);
    }
    else
    {
        HandleSetupDataCallSuccess(uiCID, rData.pRilToken);
    }
}

//
//  Call this whenever data is disconnected
//
BOOL CTE_XMM7160::DataConfigDown(UINT32 uiCID, BOOL bForceCleanup)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::DataConfigDown() - Enter\r\n");

    //  First check to see if uiCID is valid
    if (uiCID > MAX_PDP_CONTEXTS || uiCID == 0)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::DataConfigDown() - Invalid CID = [%u]\r\n", uiCID);
        return FALSE;
    }

    CChannel_Data* pChannelData = CChannel_Data::GetChnlFromContextID(uiCID);
    if (NULL == pChannelData)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::DataConfigDown() -"
                " Invalid CID=[%u], no data channel found!\r\n", uiCID);
        return FALSE;
    }

    pChannelData->RemoveInterface();

    if (!m_cte.IsEPSRegistered() || uiCID != DEFAULT_PDN_CID || bForceCleanup)
    {
        pChannelData->ResetDataCallInfo();
    }

    RIL_LOG_VERBOSE("CTE_XMM7160::DataConfigDown() EXIT\r\n");
    return TRUE;
}

//
// RIL_REQUEST_SET_BAND_MODE 65
//
RIL_RESULT_CODE CTE_XMM7160::CoreSetBandMode(REQUEST_DATA& rReqData,
                                                         void* pData,
                                                         UINT32 uiDataSize)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::CoreSetBandMode() - Enter\r\n");
    // TODO: Change to +XACT usage when the modem is ready
    return CTE_XMM6260::CoreSetBandMode(rReqData, pData, uiDataSize);
}

RIL_RESULT_CODE CTE_XMM7160::ParseSetBandMode(RESPONSE_DATA & rRspData)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::ParseSetBandMode() - Enter\r\n");
    RIL_RESULT_CODE res = RRIL_RESULT_OK;
    RIL_LOG_VERBOSE("CTE_XMM7160::ParseSetBandMode() - Exit\r\n");
    return res;
}

RIL_RESULT_CODE CTE_XMM7160::CreateSetDefaultApnReq(REQUEST_DATA& rReqData,
        const char** pszRequest, const int numStrings)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::CreateSetDefaultApnReq() - Enter\r\n");

    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;

    if (numStrings != 3)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateSetDefaultApnReq() :"
                " received_size != required_size\r\n");
        return res;
    }

    if (!PrintStringNullTerminate(rReqData.szCmd1, sizeof(rReqData.szCmd1),
            "AT+CGDCONT=1,\"%s\",\"%s\"\r", pszRequest[2], pszRequest[1]))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CreateSetDefaultApnReq() - "
                "Can't construct szCmd1.\r\n");
        goto Error;
    }

    res = RRIL_RESULT_OK;
Error:
    RIL_LOG_VERBOSE("CTE_XMM7160::CreateSetDefaultApnReq() - Exit\r\n");
    return res;
}

RIL_RESULT_CODE CTE_XMM7160::ParseNeighboringCellInfo(P_ND_N_CELL_DATA pCellData,
                                                    const char* pszRsp,
                                                    UINT32 uiIndex,
                                                    UINT32 uiMode)
{
    RIL_RESULT_CODE res = RIL_E_GENERIC_FAILURE;
    UINT32 uiTAC = 0, uiCI = 0, uiMcc =0, uiMnc = 0, uiEARFCN = 0;
    UINT32 uiPhyCI = 0, uiRSRQ = 0, uiRSRP = 0, uiRSSNR =0, uiTA = 0;
    const char* pszStart = pszRsp;

    switch (uiMode)
    {
        case 0:
        case 1:
        case 2:
        case 3:
        {
            return CTE_XMM6260::ParseNeighboringCellInfo(pCellData, pszRsp,
                     uiIndex, uiMode);
         }
         break;
        //  LTE cells:
        //  +XCELLINFO: 5,<MCC>,<MNC>,<CI>,<PCI>,<TAC>,<rsrp>,<rsrq>,<rssnr>,<ta>
        //  +XCELLINFO: 6,<EARFCN>,<PhyCI>,<rsrp>,<rsrq>,
        case 5:
        {

            //  Read <MCC>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiMcc, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                        " mode 5, could not extract MCC value\r\n");
                goto Error;
            }

            //  Read <MNC>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiMnc, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                        " mode 5, could not extract MNC value\r\n");
                goto Error;
            }

            //  Read <CI>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiCI, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                        " mode 5, could not extract CI value\r\n");
                goto Error;
            }

            //  Read <phyCI>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiPhyCI, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                        " mode 5, could not extract PhyCI value\r\n");
                goto Error;
            }

            //  Read <TAC>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiTAC, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                        " mode 5, could not extract TAC\r\n");
                goto Error;
            }

            //  Read <RSRP>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiRSRP, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                        " mode 5, could not extract RSRP value\r\n");
                goto Error;
            }

            //  Read <RSRQ>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiRSRQ, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                        " mode 5, could not extract RSRQ value\r\n");
                goto Error;
            }

            //  Read <RSSNR>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiRSSNR, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                        " mode 5, could not extract RSSNR value\r\n");
                goto Error;
            }

            //  Read <TA>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiTA, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                        " mode 5, could not extract TA value\r\n");
                goto Error;
            }

            //  We now have what we want, copy to main structure.
            pCellData->pnCellData[uiIndex].cid = pCellData->pnCellCIDBuffers[uiIndex];

            //  cid = upper 16 bits (LAC), lower 16 bits (CID)
            snprintf(pCellData->pnCellCIDBuffers[uiIndex], CELL_ID_ARRAY_LENGTH,
                    "%04X%04X", uiTAC, uiCI);
            RIL_LOG_INFO("CTE_XMM7160::ParseNeighboringCellInfo() -"
                    " mode 5 LTE TAC,CID index=[%d]  cid=[%s]\r\n",
                    uiIndex, pCellData->pnCellCIDBuffers[uiIndex]);

            // rssi ~ rsrp
            pCellData->pnCellData[uiIndex].rssi = (int)(uiRSRP);
            RIL_LOG_INFO("CTE_XMM7160::ParseNeighboringCellInfo() -"
                    " mode 5 LTE rsrp index=[%d]  rsrp=[%d]\r\n",
                    uiIndex, pCellData->pnCellData[uiIndex].rssi);
            res = RRIL_RESULT_OK;
        }
        break;

        case 6:
        {
            //  +XCELLINFO: 6,<EARFCN>,<PCI>,<rsrp>,<rsrq>,
            // Extract EARFCN
            // This parameter is not used yet
            if (!SkipString(pszRsp, ",", pszRsp) ||
                    !ExtractUInt32(pszRsp, uiEARFCN, pszRsp))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                        " mode 6, could not extract EARFCN\r\n");
                goto Error;
            }

            // Extract PhyCI
            if (!SkipString(pszRsp, ",", pszRsp) ||
                    !ExtractUInt32(pszRsp, uiPhyCI, pszRsp))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                        " mode 6, could not extract PhyCI\r\n");
                goto Error;
            }
            //  Read <RSRP>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiRSRP, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                        " mode 6, could not extract RSRP value\r\n");
                goto Error;
            }
            //  Read <RSRQ>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiRSRQ, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                        " mode 6, could not extract RSRQ value\r\n");
                goto Error;
            }
            // Reset the values to invalid
            uiTAC = 0;
            uiCI = 0;
            //  We now have what we want, copy to main structure.
            pCellData->pnCellData[uiIndex].cid = pCellData->pnCellCIDBuffers[uiIndex];

            //  cid = upper 16 bits (LAC), lower 16 bits (CID)
            snprintf(pCellData->pnCellCIDBuffers[uiIndex], CELL_ID_ARRAY_LENGTH,
                    "%04X%04X", uiTAC, uiCI);
            RIL_LOG_INFO("CTE_XMM7160::ParseNeighboringCellInfo() -"
                    " mode 6 LTE TAC,CID index=[%d]  cid=[%s]\r\n",
                    uiIndex, pCellData->pnCellCIDBuffers[uiIndex]);

            // rssi ~ rsrp
            pCellData->pnCellData[uiIndex].rssi = (int)(uiRSRP);
            RIL_LOG_INFO("CTE_XMM7160::ParseNeighboringCellInfo() -"
                    " mode 6 LTE rsrp index=[%d]  rsrp=[%d]\r\n",
                    uiIndex, pCellData->pnCellData[uiIndex].rssi);
            res = RRIL_RESULT_OK;
        }
        break;

        default:
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::ParseNeighboringCellInfo() -"
                    " Invalid nMode=[%d]\r\n", uiMode);
            goto Error;
        }
        break;
    }

Error:
    return res;
}

RIL_RESULT_CODE CTE_XMM7160::ParseCellInfo(P_ND_N_CELL_INFO_DATA pCellData,
                                                    const char* pszRsp,
                                                    UINT32 uiIndex,
                                                    UINT32 uiMode)
{
    RIL_RESULT_CODE res = RIL_E_GENERIC_FAILURE;
    UINT32 uiTAC = 0, uiCI = 0, uiRSSI = 0, uiScramblingCode = 0, uiMcc = 0, uiMnc = 0;
    UINT32 uiPhyCI = 0, uiRSRQ = 0, uiRSRP = 0, uiRSSNR =0, uiTA = 0, uiEARFCN = 0;
    const char* pszStart = pszRsp;

    switch (uiMode)
    {
        //GSM/UMTS Cells
        case 0:
        case 1:
        case 2:
        case 3:

            return CTE_XMM6260::ParseCellInfo(pCellData,
                        pszRsp, uiIndex, uiMode);
        break;

        //  LTE cells:
        //  +XCELLINFO: 5,<MCC>,<MNC>,<CI>,<PCI>,<TAC>,<rsrp>,<rsrq>,<rssnr>,<ta>
        //  +XCELLINFO: 6,<EARFCN>,<PhyCI>,<rsrp>,<rsrq>,

        case 5:
        {

            //  Read <MCC>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiMcc, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseCellInfo() -"
                        " mode 5, could not extract MCC value\r\n");
                goto Error;
            }

            //  Read <MNC>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiMnc, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseCellInfo() -"
                        " mode 5, could not extract MNC value\r\n");
                goto Error;
            }

            //  Read <CI>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiCI, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseCellInfo() -"
                        " mode 5, could not extract CI value\r\n");
                goto Error;
            }

            //  Read <phyCI>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiPhyCI, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseCellInfo() -"
                        " mode 5, could not extract PhyCI value\r\n");
                goto Error;
            }

            //  Read <TAC>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiTAC, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseCellInfo() -"
                        " mode 5, could not extract TAC\r\n");
                goto Error;
            }

            //  Read <RSRP>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiRSRP, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseCellInfo() -"
                        " mode 5, could not extract RSRP value\r\n");
                goto Error;
            }

            //  Read <RSRQ>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiRSRQ, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseCellInfo() -"
                        " mode 5, could not extract RSRQ value\r\n");
                goto Error;
            }

            //  Read <RSSNR>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiRSSNR, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseCellInfo() -"
                        " mode 5, could not extract RSSNR value\r\n");
                goto Error;
            }

            //  Read <TA>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiTA, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseCellInfo() -"
                        " mode 5, could not extract TA value\r\n");
                goto Error;
            }

            RIL_CellInfo& info = pCellData->pnCellData[uiIndex];
            info.registered = 1;
            info.cellInfoType = RIL_CELL_INFO_TYPE_LTE;
            info.timeStampType = RIL_TIMESTAMP_TYPE_JAVA_RIL;
            info.timeStamp = ril_nano_time();
            info.CellInfo.lte.signalStrengthLte.signalStrength = 99;
            info.CellInfo.lte.signalStrengthLte.rsrp = uiRSRP;
            info.CellInfo.lte.signalStrengthLte.rsrq = uiRSRQ;
            info.CellInfo.lte.signalStrengthLte.rssnr = uiRSSNR;
            info.CellInfo.lte.signalStrengthLte.cqi = INT_MAX;
            info.CellInfo.lte.signalStrengthLte.timingAdvance = uiTA;
            info.CellInfo.lte.cellIdentityLte.tac = uiTAC;
            info.CellInfo.lte.cellIdentityLte.ci = uiCI;
            info.CellInfo.lte.cellIdentityLte.pci = uiPhyCI;
            info.CellInfo.lte.cellIdentityLte.mnc = uiMnc;
            info.CellInfo.lte.cellIdentityLte.mcc = uiMcc;
            RIL_LOG_INFO("CTE_XMM7160::ParseCellInfo() -"
                    " mode 5 LTE TAC/CID/MNC/MCC/RSRP/RSRQ/TA/RSSNR "
                    "index=[%d] cid=[%d] tac[%d] mnc[%d] mcc[%d] [rsrp[%d] rsrq[%d]"
                    " ta[%d] rssnr[%d] Phyci[%d] \r\n",
                    uiIndex, info.CellInfo.lte.cellIdentityLte.ci,
                    info.CellInfo.lte.cellIdentityLte.tac,
                    info.CellInfo.lte.cellIdentityLte.mnc,
                    info.CellInfo.lte.cellIdentityLte.mcc,
                    info.CellInfo.lte.signalStrengthLte.rsrp,
                    info.CellInfo.lte.signalStrengthLte.rsrq,
                    info.CellInfo.lte.signalStrengthLte.timingAdvance,
                    info.CellInfo.lte.signalStrengthLte.rssnr,
                    info.CellInfo.lte.cellIdentityLte.pci);
            res = RRIL_RESULT_OK;
        }
        break;

        case 6:
        {
            //  +XCELLINFO: 6,<EARFCN>,<PCI>,<rsrp>,<rsrq>,
            // Extract EARFCN
            // This parameter is not used yet
            if (!SkipString(pszRsp, ",", pszRsp) ||
                    !ExtractUInt32(pszRsp, uiEARFCN, pszRsp))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseCellInfo() -"
                        " mode 6, could not extract EARFCN\r\n");
                goto Error;
            }

            // Extract PhyCI
            if (!SkipString(pszRsp, ",", pszRsp) ||
                    !ExtractUInt32(pszRsp, uiPhyCI, pszRsp))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseCellInfo() -"
                        " mode 6, could not extract PhyCI\r\n");
                goto Error;
            }
            //  Read <RSRP>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiRSRP, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseCellInfo() -"
                        " mode 6, could not extract RSRP value\r\n");
                goto Error;
            }
            //  Read <RSRQ>
            if ((!SkipString(pszRsp, ",", pszRsp)) ||
                    (!ExtractUInt32(pszRsp, uiRSRQ, pszRsp)))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseCellInfo() -"
                        " mode 6, could not extract RSRQ value\r\n");
                goto Error;
            }

            RIL_CellInfo& info = pCellData->pnCellData[uiIndex];
            info.registered = 0;
            info.cellInfoType = RIL_CELL_INFO_TYPE_LTE;
            info.timeStampType = RIL_TIMESTAMP_TYPE_JAVA_RIL;
            info.timeStamp = ril_nano_time();
            info.CellInfo.lte.signalStrengthLte.signalStrength = 99;
            info.CellInfo.lte.signalStrengthLte.rsrp = uiRSRP;
            info.CellInfo.lte.signalStrengthLte.rsrq = uiRSRQ;
            info.CellInfo.lte.signalStrengthLte.rssnr = INT_MAX;
            info.CellInfo.lte.signalStrengthLte.cqi = INT_MAX;
            info.CellInfo.lte.signalStrengthLte.timingAdvance = INT_MAX;
            info.CellInfo.lte.cellIdentityLte.tac = INT_MAX;
            info.CellInfo.lte.cellIdentityLte.ci = INT_MAX;
            info.CellInfo.lte.cellIdentityLte.pci = uiPhyCI;
            info.CellInfo.lte.cellIdentityLte.mnc = INT_MAX;
            info.CellInfo.lte.cellIdentityLte.mcc = INT_MAX;
            RIL_LOG_INFO("CTE_XMM7160::ParseCellInfo() -"
                    " mode 6 LTE TAC/CID/MNC/MCC/RSRP/RSRQ/TA/RSSNR "
                    "index=[%d] cid=[%d] tac[%d] mnc[%d] mcc[%d] [rsrp[%d] rsrq[%d]"
                    " ta[%d] rssnr[%d] Phyci[%d] \r\n",
                    uiIndex, info.CellInfo.lte.cellIdentityLte.ci,
                    info.CellInfo.lte.cellIdentityLte.tac,
                    info.CellInfo.lte.cellIdentityLte.mnc,
                    info.CellInfo.lte.cellIdentityLte.mcc,
                    info.CellInfo.lte.signalStrengthLte.rsrp,
                    info.CellInfo.lte.signalStrengthLte.rsrq,
                    info.CellInfo.lte.signalStrengthLte.timingAdvance,
                    info.CellInfo.lte.signalStrengthLte.rssnr,
                    info.CellInfo.lte.cellIdentityLte.pci);
            res = RRIL_RESULT_OK;
        }
        break;

        default:
        {
            RIL_LOG_INFO("CTE_XMM7160::ParseCellInfo() -"
                    " Invalid nMode=[%d]\r\n", uiMode);
            goto Error;
        }
        break;
    }
Error:
    return res;

}

RIL_SignalStrength_v6* CTE_XMM7160::ParseXCESQ(const char*& rszPointer, const BOOL bUnsolicited)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::ParseXCESQ() - Enter\r\n");
    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;

    int mode = 0;
    int rxlev = 0; // received signal strength level
    int ber = 0; // channel bit error rate
    int rscp = 0; // Received signal code power
    // ratio of the received energy per PN chip to the total received power spectral density
    int ec = 0;
    int rsrq = 0; // Reference signal received quality
    int rsrp = 0; // Reference signal received power
    int rssnr = -1; // Radio signal strength Noise Ratio value
    RIL_SignalStrength_v6* pSigStrData = NULL;

    if (!bUnsolicited)
    {
        // Parse "<prefix>+XCESQ: <n>,<rxlev>,<ber>,<rscp>,<ecno>,<rsrq>,<rsrp>,<rssnr><postfix>"
        if (!SkipRspStart(rszPointer, m_szNewLine, rszPointer)
                || !SkipString(rszPointer, "+XCESQ: ", rszPointer))
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::ParseUnsolicitedSignalStrength() - "
                    "Could not find AT response.\r\n");
            goto Error;
        }

        if (!ExtractInt(rszPointer, mode, rszPointer))
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::ParseUnsolicitedSignalStrength() - "
                    "Could not extract <mode>\r\n");
            goto Error;
        }

        if (!SkipString(rszPointer, ",", rszPointer))
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::ParseXCESQ() - Could not extract , before <rxlev>\r\n");
            goto Error;
        }
    }

    if (!ExtractInt(rszPointer, rxlev, rszPointer))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseXCESQ() - Could not extract <rxlev>\r\n");
        goto Error;
    }

    if (!SkipString(rszPointer, ",", rszPointer)
            || !ExtractInt(rszPointer, ber, rszPointer))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseXCESQ() - Could not extract <ber>\r\n");
        goto Error;
    }

    if (!SkipString(rszPointer, ",", rszPointer)
            || !ExtractInt(rszPointer, rscp, rszPointer))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseXCESQ() - Could not extract <rscp>\r\n");
        goto Error;
    }

    // Not used
    if (!SkipString(rszPointer, ",", rszPointer)
            || !ExtractInt(rszPointer, ec, rszPointer))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseXCESQ() - Could not extract <ecno>\r\n");
        goto Error;
    }

    if (!SkipString(rszPointer, ",", rszPointer)
            || !ExtractInt(rszPointer, rsrq, rszPointer))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseXCESQ() - Could not extract <rsrq>\r\n");
        goto Error;
    }

    if (!SkipString(rszPointer, ",", rszPointer)
            || !ExtractInt(rszPointer, rsrp, rszPointer))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseXCESQ() - Could not extract <rsrp>.\r\n");
        goto Error;
    }

    if (!SkipString(rszPointer, ",", rszPointer)
            || !ExtractInt(rszPointer, rssnr, rszPointer))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseXCESQ() - "
                "Could not extract <rssnr>.\r\n");
        goto Error;
    }

    if (!bUnsolicited)
    {
        if (!FindAndSkipRspEnd(rszPointer, m_szNewLine, rszPointer))
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::ParseXCESQ() -"
                    " Could not extract the response end.\r\n");
            goto Error;
        }
    }

    pSigStrData = (RIL_SignalStrength_v6*)malloc(sizeof(RIL_SignalStrength_v6));
    if (NULL == pSigStrData)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseXCESQ() -"
                " Could not allocate memory for RIL_SignalStrength_v6.\r\n");
        goto Error;
    }

    // reset to default values
    pSigStrData->GW_SignalStrength.signalStrength = -1;
    pSigStrData->GW_SignalStrength.bitErrorRate   = -1;
    pSigStrData->CDMA_SignalStrength.dbm = -1;
    pSigStrData->CDMA_SignalStrength.ecio = -1;
    pSigStrData->EVDO_SignalStrength.dbm = -1;
    pSigStrData->EVDO_SignalStrength.ecio = -1;
    pSigStrData->EVDO_SignalStrength.signalNoiseRatio = -1;
    pSigStrData->LTE_SignalStrength.signalStrength = -1;
    pSigStrData->LTE_SignalStrength.rsrp = INT_MAX;
    pSigStrData->LTE_SignalStrength.rsrq = INT_MAX;
    pSigStrData->LTE_SignalStrength.rssnr = INT_MAX;
    pSigStrData->LTE_SignalStrength.cqi = INT_MAX;

    /*
     * If the current serving cell is GERAN cell, then <rxlev> and <ber> are set to
     * valid values.
     * For <rxlev>, valid values are 0 to 63.
     * For <ber>, valid values are 0 to 7.
     * If the current service cell is not GERAN cell, then <rxlev> and <ber> are set
     * to value 99.
     *
     * If the current serving cell is UTRA cell, then <rscp> is set to valid value.
     * For <rscp>, valid values are 0 to 96.
     * If the current service cell is not UTRA cell, then <rscp> is set to value 255.
     *
     * If the current serving cell is E-UTRA cell, then <rsrq> and <rsrp> are set to
     * valid values.
     * For <rsrq>, valid values are 0 to 34.
     * For <rsrp>, valid values are 0 to 97.
     * If the current service cell is not E-UTRA cell, then <rsrq> and <rsrp> are set
     * to value 255.
     */
    if (99 != rxlev)
    {
        /*
         * As <rxlev> reported as part of XCESQ is not in line with the <rssi> reported
         * as part of AT+CSQ and also what android expects, following conversion is done.
         */
        if (rxlev <= 57)
        {
            rxlev = floor(rxlev / 2) + 2;
        }
        else
        {
            rxlev = 31;
        }

        pSigStrData->GW_SignalStrength.signalStrength = rxlev;
        pSigStrData->GW_SignalStrength.bitErrorRate   = ber;
    }
    else if (255 != rscp)
    {
        /*
         * As <rscp> reported as part of XCESQ is not in line with the <rssi> reported
         * as part of AT+CSQ and also what android expects, following conversion is done.
         */
        if (rscp <= 7)
        {
            rscp = 0;
        }
        else if (rscp <= 67)
        {
            rscp = floor((rscp - 6) / 2);
        }
        else
        {
            rscp = 31;
        }

        pSigStrData->GW_SignalStrength.signalStrength = rscp;
    }
    else if (255 != rsrq && 255 != rsrp)
    {
        /*
         * for rsrp if modem returns 0 then rsrp = -140 dBm.
         * for rsrp if modem returns 1 then rsrp = -139 dBm.
         * As Android does the inversion, rapid ril needs to send (140 - rsrp) to framework.
         *
         * for rsrq if modem return 0 then rsrq = -19.5 dBm.
         * for rsrq if modem return 1 then rsrq = -19 dBm.
         * As Android does the inversion, rapid ril needs to send (20 - rsrq/2) to framework.
         *
         * for rssnr if modem returns 0 then rssnr = 0 dBm
         * for rssnr if modem returns 1 then rssnr = 0.5 dBm
         * As Android has granularity of 0.1 dB units, rapid ril needs to send
         * (rssnr/2)*10 => rssnr * 5 to framework.
         *
         * You can refer to the latest CAT specification on XCESQI AT command
         * to understand where these numbers come from
         */
        pSigStrData->LTE_SignalStrength.rsrp = 140 - rsrp;
        pSigStrData->LTE_SignalStrength.rsrq = 20 - rsrq / 2;
        pSigStrData->LTE_SignalStrength.rssnr = rssnr * 5;
    }
    else
    {
        RIL_LOG_INFO("CTE_XMM7160::ParseXCESQ - "
                "pSigStrData set to default values\r\n");
    }

    res = RRIL_RESULT_OK;
Error:
    if (RRIL_RESULT_OK != res)
    {
        free(pSigStrData);
        pSigStrData = NULL;
    }

    RIL_LOG_VERBOSE("CTE_XMM7160::ParseXCESQ - Exit()\r\n");
    return pSigStrData;
}

void CTE_XMM7160::QuerySignalStrength()
{
    CCommand* pCmd = new CCommand(g_pReqInfo[RIL_REQUEST_SIGNAL_STRENGTH].uiChannel, NULL,
            RIL_REQUEST_SIGNAL_STRENGTH, "AT+XCESQ?\r", &CTE::ParseUnsolicitedSignalStrength);

    if (pCmd)
    {
        if (!CCommand::AddCmdToQueue(pCmd))
        {
            RIL_LOG_CRITICAL("CTE_XMM7160::QuerySignalStrength() - Unable to queue command!\r\n");
            delete pCmd;
            pCmd = NULL;
        }
    }
    else
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::QuerySignalStrength() - "
                "Unable to allocate memory for new command!\r\n");
    }
}

//
// RIL_UNSOL_SIGNAL_STRENGTH  1009
//
RIL_RESULT_CODE CTE_XMM7160::ParseUnsolicitedSignalStrength(RESPONSE_DATA& rRspData)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::ParseUnsolicitedSignalStrength() - Enter\r\n");

    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;
    RIL_SignalStrength_v6* pSigStrData = NULL;
    const char* pszRsp = rRspData.szResponse;

    pSigStrData = ParseXCESQ(pszRsp, FALSE);
    if (NULL == pSigStrData)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseUnsolicitedSignalStrength() -"
                " parsing failed\r\n");
        goto Error;
    }

    res = RRIL_RESULT_OK;

    RIL_onUnsolicitedResponse(RIL_UNSOL_SIGNAL_STRENGTH, (void*)pSigStrData,
            sizeof(RIL_SignalStrength_v6));

Error:
    free(pSigStrData);
    pSigStrData = NULL;

    RIL_LOG_VERBOSE("CTE_XMM7160::ParseUnsolicitedSignalStrength() - Exit\r\n");
    return res;
}

//
// RIL_REQUEST_ISIM_AUTHENTICATE 106
//
RIL_RESULT_CODE CTE_XMM7160::CoreISimAuthenticate(REQUEST_DATA& rReqData,
                                                                void* pData,
                                                                UINT32 uiDataSize)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::CoreISimAuthenticate() - Enter\r\n");
    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;

    char szAutn[AUTN_LENGTH+1]; //32 bytes + null terminated
    char szRand[RAND_LENGTH+1]; //32 bytes + null terminated
    char* pszInput = (char*) pData;
    int nChannelId = 0; // Use default channel ( USIM ) for now. may need to use the one
                        // for ISIM.
    int nContext = 1;   // 1 is USIM security context. ( see CAT Spec )

    if (NULL == pData)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CoreISimAuthenticate() - Passed data pointer was NULL\r\n");
        goto Error;
    }

    CopyStringNullTerminate(szAutn, pszInput, sizeof(szAutn));
    CopyStringNullTerminate(szRand, pszInput+AUTN_LENGTH, sizeof(szRand));

    if (!PrintStringNullTerminate(rReqData.szCmd1, sizeof(rReqData.szCmd1),
            "AT+XAUTH=%u,%u,\"%s\",\"%s\"\r", nChannelId, nContext, szRand, szAutn))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::CoreISimAuthenticate() - Cannot create XAUTH command -"
                " szRand=%s, szAutn=%s\r\n",szRand,szAutn);
        goto Error;
    }

    res = RRIL_RESULT_OK;
Error:
    RIL_LOG_VERBOSE("CTE_XMM7160::CoreISimAuthenticate() - Exit\r\n");

    return res;
}

RIL_RESULT_CODE CTE_XMM7160::ParseISimAuthenticate(RESPONSE_DATA& rRspData)
{
    RIL_LOG_VERBOSE("CTE_XMM7160::ParseISimAuthenticate() - Enter\r\n");
    RIL_RESULT_CODE res = RRIL_RESULT_ERROR;
    const char* pszRsp = rRspData.szResponse;
    int reslen = 0;
    char * pszResult = NULL;
    UINT32 uiStatus;
    char szRes[MAX_BUFFER_SIZE];
    char szCk[MAX_BUFFER_SIZE];
    char szIk[MAX_BUFFER_SIZE];
    char szKc[MAX_BUFFER_SIZE];

    memset(szRes, '\0', sizeof(szRes));
    memset(szCk, '\0', sizeof(szCk));
    memset(szIk, '\0', sizeof(szIk));
    memset(szKc, '\0', sizeof(szKc));

    if (NULL == rRspData.szResponse)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                " Response String pointer is NULL.\r\n");
        goto Error;
    }

    if (FindAndSkipString(pszRsp, "+XAUTH: ", pszRsp))
    {
        if (!ExtractUInt32(pszRsp, uiStatus, pszRsp)) {
            RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                    " Error parsing status.\r\n");
            goto Error;
        }

        if ((uiStatus == 0)||(uiStatus == 1))
        {
            // Success, need to parse the extra parameters...
            if (!SkipString(pszRsp, ",", pszRsp))
            {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                                 " Error parsing status.\r\n");
                goto Error;
            }

            if (!ExtractQuotedString(pszRsp, szRes, sizeof(szRes), pszRsp)) {
                RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                                 " Error parsing Res.\r\n");
                goto Error;
            }

            if (uiStatus == 0)
            {
                // This is success, so we need to get CK, IK, KC
                if (!SkipString(pszRsp, ",", pszRsp))
                {
                    RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                                     " Error parsing Res.\r\n");
                    goto Error;
                }

                if (!ExtractQuotedString(pszRsp, szCk, sizeof(szCk), pszRsp)) {
                    RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                                     " Error parsing CK.\r\n");
                    goto Error;
                }
                if (!SkipString(pszRsp, ",", pszRsp))
                {
                    RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                                     " Error parsing CK.\r\n");
                    goto Error;
                }

                if (!ExtractQuotedString(pszRsp, szIk, sizeof(szIk), pszRsp)) {
                    RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                                     " Error parsing IK.\r\n");
                    goto Error;
                }
                if (!SkipString(pszRsp, ",", pszRsp))
                {
                    RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                                     " Error parsing IK.\r\n");
                    goto Error;
                }

                if (!ExtractQuotedString(pszRsp, szKc, sizeof(szKc), pszRsp)) {
                    RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                                     " Error parsing Kc.\r\n");
                    goto Error;
                }
            }

            // Log the result for debug
            RIL_LOG_VERBOSE("CTE_XMM7160::ParseISimAuthenticate - Res/Auts -"
                            " =%s\r\n", szRes);
        }
    }
    else
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                " Error searching +XAUTH:.\r\n");
        goto Error;
    }

    if (!FindAndSkipRspEnd(pszRsp, m_szNewLine, pszRsp))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                " Could not extract the response end.\r\n");
        goto Error;
    }

    // reslen = 3 bytes for the status (int), 4 bytes for the :, and the rest + 1 for
    // the carriage return...
    reslen = 3 + 4 + strlen(szRes) + strlen(szCk) + strlen(szIk) + strlen(szKc) + 1;
    pszResult = (char *) malloc(reslen);
    if (!pszResult)
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                " Could not allocate memory for result string.\r\n");
        goto Error;
    }
    if (!PrintStringNullTerminate(pszResult, reslen,
                                  "%d:%s:%s:%s:%s", uiStatus, szRes, szCk, szIk, szKc))
    {
        RIL_LOG_CRITICAL("CTE_XMM7160::ParseISimAuthenticate() -"
                " Error creating response string.\r\n");
        goto Error;
    }
    rRspData.pData = (void *) pszResult;
    rRspData.uiDataSize = reslen;

    res = RRIL_RESULT_OK;

Error:
    RIL_LOG_VERBOSE("CTE_XMM7160::ParseISimAuthenticate() - Exit\r\n");
    return res;
}
