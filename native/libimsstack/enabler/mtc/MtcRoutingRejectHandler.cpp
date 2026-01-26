/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "IMtcContext.h"
#include "INetworkWatcher.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "ImsEventDef.h"
#include "MtcImsEventReceiver.h"
#include "MtcRoutingRejectHandler.h"
#include "ServiceTrace.h"
#include "SipMethod.h"
#include "SipStatusCode.h"
#include "configuration/MtcConfigurationProxy.h"

__IMS_TRACE_TAG_COM_MTC__;

LOCAL const IMS_CHAR REASON_PHRASE_ON_EHRPD_TYPE1[] = "On eHRPD";
LOCAL const IMS_CHAR REASON_PHRASE_VOLTE_OFF_TYPE1[] = "VoLTE setting OFF";

PUBLIC MtcRoutingRejectHandler::MtcRoutingRejectHandler(
        IN IMtcContext& objContext, IN INetworkWatcher& objNetworkWatcher) :
        m_objContext(objContext),
        m_objNetworkWatcher(objNetworkWatcher)
{
    IMS_TRACE_I("+MtcRoutingRejectHandler", 0, 0, 0);
}

PUBLIC MtcRoutingRejectHandler::~MtcRoutingRejectHandler()
{
    IMS_TRACE_I("~MtcRoutingRejectHandler", 0, 0, 0);
}

PUBLIC IMS_BOOL MtcRoutingRejectHandler::RoutingReject_NotifyRequest(
        IN ISipMessage* pSipMessage, IN_OUT SipStatusCode& objStatusCode)
{
    if (pSipMessage->GetMethod().Equals(SipMethod::INVITE))
    {
        objStatusCode = GetRoutingRejectCodeForInvite(objStatusCode);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PUBLIC IMS_BOOL MtcRoutingRejectHandler::RoutingReject_NotifyRequest(
        IN ISipServerConnection* pSipServerConnection, IN_OUT SipStatusCode& objStatusCode)
{
    if (pSipServerConnection->GetMethod().Equals(SipMethod::INVITE))
    {
        objStatusCode = GetRoutingRejectCodeForInvite(objStatusCode);
        return IMS_TRUE;
    }

    return IMS_FALSE;
}

PRIVATE
SipStatusCode MtcRoutingRejectHandler::GetRoutingRejectCodeForInvite(
        IN const SipStatusCode& objDefaultStatusCode) const
{
    /*
    TODO: b/381989518 - need IMS platform support
    if subscriber is not provisioned:
        return SipStatusCode(SipStatusCode::SC_488, "Subscriber not provisioned for VoLTE");
    */

    NETRADIO_ENTYPE eRat = m_objNetworkWatcher.GetNetRadioTechType();
    IMS_TRACE_D("GetRoutingRejectCodeForInvite : RAT[%d]", eRat, 0, 0);
    switch (eRat)
    {
        case NW_REPORT_RADIO_EHRPD:
            return SipStatusCode(SipStatusCode::SC_488, REASON_PHRASE_ON_EHRPD_TYPE1);

        case NW_REPORT_RADIO_WLAN:
            if (m_objContext.GetImsEventReceiver().GetWParam(IMS_EVENT_WFC_SETTING_CHANGED) !=
                    IMS_WFC_ON)
            {
                return SipStatusCode(SipStatusCode::SC_486,
                        m_objContext.GetConfigurationProxy()
                                .GetString(ConfigVoice::
                                                KEY_CALL_REJECT_REASON_PHRASE_VOWIFI_OFF_STRING)
                                .GetStr());
            }
            break;

        case NW_REPORT_RADIO_LTE:
            if (m_objContext.GetImsEventReceiver().GetWParam(IMS_EVENT_VOLTE_SETTING) ==
                    IMS_VOLTE_SETTING_OFF)
            {
                return SipStatusCode(SipStatusCode::SC_488, REASON_PHRASE_VOLTE_OFF_TYPE1);
            }
            break;

        default:
            break;
    }
    return objDefaultStatusCode;
}
