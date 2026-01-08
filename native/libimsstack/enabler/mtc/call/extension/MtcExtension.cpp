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

#include "IMessage.h"
#include "ISipHeader.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "call/extension/MtcExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "utility/IMessageUtils.h"
#include <vector>

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcExtension::MtcExtension(IN IMtcCallContext& objContext, IN const AString& strOptionTag,
        IN const std::vector<RequestType>& lstSupportedRequestType,
        IN const std::vector<ResponseType>& lstSupportedResponseType) :
        m_objContext(objContext),
        m_strOptionTag(strOptionTag),
        m_lstSupportedRequestType(lstSupportedRequestType),
        m_lstSupportedResponseType(lstSupportedResponseType),
        m_bRequiredOnRemote(IMS_FALSE),
        m_bSupportedOnRemote(IMS_FALSE)
{
}

PUBLIC
MtcExtension::MtcExtension(IN const MtcExtension& objRhs) :
        m_objContext(objRhs.m_objContext),
        m_strOptionTag(objRhs.m_strOptionTag),
        m_lstSupportedRequestType(objRhs.m_lstSupportedRequestType),
        m_lstSupportedResponseType(objRhs.m_lstSupportedResponseType),
        m_bRequiredOnRemote(objRhs.m_bRequiredOnRemote),
        m_bSupportedOnRemote(objRhs.m_bSupportedOnRemote)
{
}

PUBLIC VIRTUAL MtcExtension::~MtcExtension() {}

PUBLIC VIRTUAL IMtcExtension* MtcExtension::Clone() const
{
    return new MtcExtension(*this);
}

PUBLIC VIRTUAL IMS_BOOL MtcExtension::IsAvailableOnRemote() const
{
    return m_bRequiredOnRemote || m_bSupportedOnRemote;
}

PUBLIC VIRTUAL IMS_BOOL MtcExtension::IsRequiredOnRemote() const
{
    return m_bRequiredOnRemote;
}

PUBLIC VIRTUAL const AString& MtcExtension::GetOptionTag() const
{
    return m_strOptionTag;
}

PUBLIC VIRTUAL void MtcExtension::FormatRequest(IN RequestType eType, IN_OUT IMessage& objRequest)
{
    if (!IsSupportedType(eType))
    {
        return;
    }

    m_objContext.GetMessageUtils().AddValueIfNotExists(
            &objRequest, GetOptionTag(), ISipHeader::SUPPORTED);
}

PUBLIC VIRTUAL void MtcExtension::FormatResponse(
        IN ResponseType eType, IN_OUT IMessage& objResponse)
{
    if (!IsSupportedType(eType))
    {
        return;
    }

    if (!IsAvailableOnRemote())
    {
        return;
    }

    m_objContext.GetMessageUtils().AddValueIfNotExists(
            &objResponse, GetOptionTag(), ISipHeader::SUPPORTED);
}

PUBLIC VIRTUAL void MtcExtension::HandleRequest(IN RequestType eType, IN const IMessage& objRequest)
{
    if (!IsSupportedType(eType))
    {
        return;
    }

    UpdateFromRequireAndSupportedHeader(objRequest);
}

PUBLIC VIRTUAL void MtcExtension::HandleResponse(
        IN ResponseType eType, IN const IMessage& objResponse)
{
    if (!IsSupportedType(eType))
    {
        return;
    }

    UpdateFromRequireAndSupportedHeader(objResponse);
}

PRIVATE
void MtcExtension::UpdateFromRequireAndSupportedHeader(IN const IMessage& objMessage)
{
    m_bRequiredOnRemote = m_objContext.GetMessageUtils().ContainsValueIgnoreCase(
            &objMessage, GetOptionTag(), ISipHeader::REQUIRE);
    m_bSupportedOnRemote = m_objContext.GetMessageUtils().ContainsValueIgnoreCase(
            &objMessage, GetOptionTag(), ISipHeader::SUPPORTED);

    IMS_TRACE_D("UpdateFromRequireAndSupportedHeader : Tag[%s] Require[%s] Supported[%s]",
            m_strOptionTag.GetStr(), _TRACE_B_(m_bRequiredOnRemote),
            _TRACE_B_(m_bSupportedOnRemote));
}
