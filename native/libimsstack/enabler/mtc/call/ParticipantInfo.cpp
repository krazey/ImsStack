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

#include "CarrierConfig.h"
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
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

const AString ParticipantInfo::URI_SET_BY_IMS_ENGINE = AString::ConstNull();

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
    const SuppService* pSuppService =
            m_objContext.GetSupplementaryService().Get(SuppType::TARGET_URI);
    if (pSuppService != IMS_NULL)
    {
        IMS_TRACE_D("GetRemoteUri : From supplementary service[%s]",
                pSuppService->strValue.GetStr(), 0, 0);
        return pSuppService->strValue;
    }

    if (m_objContext.GetSupplementaryService().Get(SuppType::CALL_PULL))
    {
        const ICoreService* piCoreService = m_objContext.GetService().GetICoreService();
        if (piCoreService == IMS_NULL)
        {
            IMS_TRACE_E(0, "GetRemoteUri : CoreService is null", 0, 0, 0);
            return AString::ConstNull();
        }

        IMS_TRACE_D("GetRemoteUri : Use local uri for call pull[%s]",
                piCoreService->GetAuthorizedUserId().ToString().GetStr(), 0, 0);
        // CoreService also use this value when URI_SET_BY_IMS_ENGINE is used.
        return piCoreService->GetAuthorizedUserId().ToString();
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
    IMS_TRACE_D("UpdateFromRemoteNumber : URI[%s]", m_strRemoteUri.GetStr(), 0, 0);
}

PUBLIC void ParticipantInfo::HandleRequest(IN RequestType eType, IN const IMessage& objRequest)
{
    if (eType != RequestType::START)
    {
        return;
    }

    m_strRemoteUri = m_objContext.GetMessageUtils().GetRemoteUri(
            &m_objContext.GetSession()->GetISession(), PeerType::MT);
    m_strRemoteNumber = GetRemoteNumberFromMessage(objRequest);
    MtcSupplementaryService::ConvertGlobalNumberToLocalNumber(
            m_objContext.GetConfigurationProxy(), m_strRemoteNumber);

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

    if (!m_objContext.GetConfigurationProxy().GetBoolean(
                ConfigVoice::KEY_OIP_SOURCE_FROM_HEADER_BOOL))
    {
        // Examine PAID first
        strNumber = m_objContext.GetMessageUtils().GetUserPart(
                &objMessage, ISipHeader::P_ASSERTED_IDENTITY);
    }

    if (strNumber.GetLength() <= 0)
    {
        strNumber = m_objContext.GetMessageUtils().GetUserPart(&objMessage, ISipHeader::FROM);
    }

    return strNumber;
}
