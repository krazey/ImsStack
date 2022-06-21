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
#include "SipMethod.h"
#include "ServiceTrace.h"
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/PreconditionExtension.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
PreconditionExtension::PreconditionExtension() :
        MtcExtension(MtcExtensionSet::OPTION_TAG_PRECONDITION)
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

PUBLIC VIRTUAL void PreconditionExtension::HandleRequest(
        IN IMS_UINT32 nMethod, IN const IMessage& objRequest)
{
    if (nMethod != IMessage::SESSION_START && nMethod != IMessage::SESSION_EARLY_UPDATE &&
            nMethod != IMessage::SESSION_PRACK)
    {
        return;
    }

    if (nMethod != IMessage::SESSION_START && !MessageUtil::HasSdp(&objRequest))
    {
        IMS_TRACE_D("HandleRequest : Don't check precondition feature without SDP.", 0, 0, 0);
        return;
    }

    MtcExtension::HandleRequest(nMethod, objRequest);
}

PUBLIC VIRTUAL void PreconditionExtension::HandleResponse(
        IN IMS_UINT32 nMethod, IN const IMessage& objResponse)
{
    if (nMethod != IMessage::SESSION_START)
    {
        return;
    }

    if (!MessageUtil::HasSdp(&objResponse))
    {
        IMS_TRACE_D("HandleResponse : Don't check precondition feature without SDP.", 0, 0, 0);
        return;
    }

    MtcExtension::HandleResponse(nMethod, objResponse);
}
