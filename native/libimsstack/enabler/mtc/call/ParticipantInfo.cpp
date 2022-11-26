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

#include "ICoreService.h"
#include "IImsAosInfo.h"
#include "IMessage.h"
#include "ISipHeader.h"
#include "MtcDef.h"
#include "ServiceTrace.h"
#include "SipAddress.h"
#include "call/IMtcCallContext.h"
#include "call/IMtcSession.h"
#include "call/ParticipantInfo.h"
#include "configuration/ConfigDef.h"
#include "configuration/MtcConfigurationProxy.h"
#include "dialingplan/IMtcDialingPlan.h"
#include "helper/IMtcAosConnector.h"
#include "helper/MtcSupplementaryService.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

const AString ParticipantInfo::URI_SET_BY_IMS_ENGINE = AString::ConstNull();
const AString ParticipantInfo::ANONYMOUS_ADDRESS = "sip:anonymous@anonymous.invalid";
const AString ParticipantInfo::ANONYMOUS_DISPLAY_NAME = "Anonymous";

PUBLIC
ParticipantInfo::ParticipantInfo(IN IMtcCallContext& objContext) :
        m_objContext(objContext),
        m_strRemoteNumber(AString::ConstNull()),
        m_strRemoteUri(AString::ConstNull())
{
}

PUBLIC
ParticipantInfo::~ParticipantInfo() {}

PUBLIC
AString ParticipantInfo::GetLocalNumber() const
{
    const ICoreService* piCoreService = m_objContext.GetService().GetICoreService();
    if (piCoreService == IMS_NULL)
    {
        IMS_TRACE_E(0, "GetLocalNumber : CoreService is null", 0, 0, 0);
        return AString::ConstNull();
    }

    return piCoreService->GetAuthorizedUserId().GetUser();
}

PUBLIC
AString ParticipantInfo::GetLocalUri() const
{
    if (m_objContext.GetService().IsEmergency())
    {
        return GetLocalUriForEmergencyCall();
    }
    return URI_SET_BY_IMS_ENGINE;
}

PUBLIC
AString ParticipantInfo::GetRemoteNumber() const
{
    return m_strRemoteNumber;
}

PUBLIC
AString ParticipantInfo::GetRemoteUri() const
{
    /*
    TODO: E911 redial I/F will be changed
    AStringBuffer objServiceUrn;
    UCRedial::GetInstance()->GetServiceURNForRedial(
            m_objContext.GetSlotId(), strNumber, objServiceUrn);
    UCRedial::GetInstance()->ClearServiceURNForRedial(m_objContext.GetSlotId());
    if (objServiceUrn.GetLength() > 0 && m_objContext.GetCallInfo().bEmergency)
    {
        return objServiceUrn.GetString();
    }
    */

    if (m_objContext.GetCallInfo().bConference)
    {
        return m_objContext.GetDialingPlan().GetToUri("", m_objContext.GetCallInfo());
    }

    const SuppService* pSuppService =
            m_objContext.GetSupplementaryService().Get(SuppType::TARGET_URI);
    if (pSuppService != IMS_NULL)
    {
        IMS_TRACE_D("GetRemoteUri : From supplementary service[%s]",
                pSuppService->strValue.GetStr(), 0, 0);
        return pSuppService->strValue;
    }

    IMS_TRACE_D("GetRemoteUri : URI[%s]", m_strRemoteUri.GetStr(), 0, 0);
    return m_strRemoteUri;
}

PUBLIC
AString ParticipantInfo::GetRemoteDisplayName() const
{
    const SuppService* pSuppService = m_objContext.GetSupplementaryService().Get(SuppType::CNAP);
    if (pSuppService != IMS_NULL)
    {
        return pSuppService->strValue;
    }

    return m_strRemoteNumber;
}

PUBLIC
OipType ParticipantInfo::GetOipType() const
{
    const SuppService* pSuppService =
            m_objContext.GetSupplementaryService().Get(SuppType::CALLER_ID);
    if (pSuppService != IMS_NULL)
    {
        return static_cast<OipType>(pSuppService->nValue);
    }
    return OipType::NONE;
}

PUBLIC
void ParticipantInfo::UpdateFromRemoteNumber(IN const AString& strRemoteNumber)
{
    m_strRemoteNumber = strRemoteNumber;
    m_strRemoteUri =
            m_objContext.GetDialingPlan().GetToUri(strRemoteNumber, m_objContext.GetCallInfo());
}

PUBLIC void ParticipantInfo::HandleRequest(IN RequestType eType, IN const IMessage& objRequest)
{
    if (eType != RequestType::START)
    {
        return;
    }

    MessageUtil::GetRemoteUri(
            &m_objContext.GetSession()->GetISession(), PeerType::MT, m_strRemoteUri);
    m_strRemoteNumber = GetRemoteNumberFromMessage(objRequest);

    IMS_TRACE_D("HandleRequest : Remote URI[%s] Number[%s]", m_strRemoteUri.GetStr(),
            m_strRemoteNumber.GetStr(), 0);
}

PUBLIC void ParticipantInfo::HandleResponse(
        IN ResponseType /* eType */, IN const IMessage& /* objRequest */)
{
}

PRIVATE AString ParticipantInfo::GetRemoteNumberFromMessage(IN const IMessage& objMessage) const
{
    AString strNumber;

    if (!m_objContext.GetConfigurationProxy().Is(Feature::OIP_SOURCE_FROM_HEADER))
    {
        // Examine PAID first
        MessageUtil::GetUserPart(&objMessage, ISipHeader::P_ASSERTED_IDENTITY, strNumber);
    }

    if (strNumber.GetLength() <= 0)
    {
        MessageUtil::GetUserPart(&objMessage, ISipHeader::FROM, strNumber);
    }

    return strNumber;
}

PRIVATE
AString ParticipantInfo::GetLocalUriForEmergencyCall() const
{
    IMtcAosConnector* pAosConnector = m_objContext.GetService().GetAosConnector();
    IMS_UINT32 nAosRegistrationMode =
            pAosConnector ? pAosConnector->GetRegistrationMode() : IImsAosInfo::REG_MODE_UNKNOWN;

    if (nAosRegistrationMode == IImsAosInfo::REG_MODE_NOUICC ||
            nAosRegistrationMode == IImsAosInfo::REG_MODE_ADMIN)
    {
        SipAddress objSipAddress;
        objSipAddress.Create(ANONYMOUS_ADDRESS);
        objSipAddress.SetDisplayName(ANONYMOUS_DISPLAY_NAME);
        return objSipAddress.ToString();
    }

    return URI_SET_BY_IMS_ENGINE;
}
