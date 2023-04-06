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
#include "SipMethod.h"
#include "call/IMtcCall.h"
#include "call/IMtcCallContext.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/PreconditionExtension.h"
#include "utility/IMessageUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
PreconditionExtension::PreconditionExtension(IN IMtcCallContext& objContext) :
        MtcExtension(objContext, MtcExtensionSet::OPTION_TAG_PRECONDITION)
{
}

PUBLIC
PreconditionExtension::PreconditionExtension(IN const PreconditionExtension& objRhs) :
        MtcExtension(objRhs)
{
}

PUBLIC VIRTUAL PreconditionExtension::~PreconditionExtension() {}

PUBLIC VIRTUAL IMtcExtension* PreconditionExtension::Clone() const
{
    return new PreconditionExtension(*this);
}

PUBLIC VIRTUAL void PreconditionExtension::FormatRequest(
        IN RequestType eType, IN_OUT IMessage& objRequest)
{
    if (eType != RequestType::START && !IsAvailableOnRemote())
    {
        return;
    }

    IMS_SINT32 eHeaderType = ISipHeader::SUPPORTED;
    switch (eType)
    {
        case RequestType::START:
        case RequestType::UPDATE:
            break;

        case RequestType::EARLY_UPDATE:
            if (m_objContext.GetCall().GetState() == IMtcCall::State::UPDATING)
            {
                break;
            }
            eHeaderType = ISipHeader::REQUIRE;
            break;

        case RequestType::PRACK:  // TODO: Check SDP and set Supported header.
        case RequestType::ACK:
        case RequestType::CANCEL_UPDATE:
        case RequestType::TERMINATE:
            return;
    }

    m_objContext.GetMessageUtils().AddValueIfNotExists(&objRequest, GetOptionTag(), eHeaderType);
}

PUBLIC VIRTUAL void PreconditionExtension::FormatResponse(
        IN ResponseType eType, IN_OUT IMessage& objResponse)
{
    if (!IsAvailableOnRemote())
    {
        return;
    }
    if (eType == ResponseType::REJECT)
    {
        return;
    }

    m_objContext.GetMessageUtils().AddValueIfNotExists(
            &objResponse, GetOptionTag(), ISipHeader::REQUIRE);
}

PUBLIC VIRTUAL void PreconditionExtension::HandleRequest(
        IN RequestType eType, IN const IMessage& objRequest)
{
    if (eType != RequestType::START && eType != RequestType::EARLY_UPDATE &&
            eType != RequestType::PRACK)
    {
        return;
    }

    if (eType != RequestType::START && !m_objContext.GetMessageUtils().HasSdp(&objRequest))
    {
        IMS_TRACE_D("HandleRequest : Don't check precondition feature without SDP.", 0, 0, 0);
        return;
    }

    MtcExtension::HandleRequest(eType, objRequest);
}

PUBLIC VIRTUAL void PreconditionExtension::HandleResponse(
        IN ResponseType eType, IN const IMessage& objResponse)
{
    if (eType != ResponseType::PROVISIONAL_RESPONSE)
    {
        return;
    }

    if (!m_objContext.GetMessageUtils().HasSdp(&objResponse))
    {
        IMS_TRACE_D("HandleResponse : Don't check precondition feature without SDP.", 0, 0, 0);
        return;
    }

    MtcExtension::HandleResponse(eType, objResponse);
}
