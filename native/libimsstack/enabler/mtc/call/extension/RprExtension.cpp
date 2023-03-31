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
#include "call/extension/MtcExtensionSet.h"
#include "call/extension/RprExtension.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
RprExtension::RprExtension() :
        MtcExtension(MtcExtensionSet::OPTION_TAG_RPR)
{
}

PUBLIC
RprExtension::RprExtension(IN const RprExtension& objRhs) :
        MtcExtension(objRhs)
{
}

PUBLIC VIRTUAL RprExtension::~RprExtension() {}

PUBLIC VIRTUAL IMtcExtension* RprExtension::Clone() const
{
    return new RprExtension(*this);
}

PUBLIC VIRTUAL void RprExtension::FormatRequest(IN RequestType eType, IN_OUT IMessage& objRequest)
{
    if (eType != RequestType::START && eType != RequestType::UPDATE)
    {
        return;
    }

    MessageUtil::AddValueIfNotExists(&objRequest, GetOptionTag(), ISipHeader::SUPPORTED);
}

PUBLIC VIRTUAL void RprExtension::HandleRequest(IN RequestType eType, IN const IMessage& objRequest)
{
    // re-INVITE should update 100rel extension supportability
    if (eType != RequestType::START && eType != RequestType::UPDATE)
    {
        return;
    }

    MtcExtension::HandleRequest(eType, objRequest);
}

PUBLIC VIRTUAL void RprExtension::HandleResponse(
        IN ResponseType eType, IN const IMessage& objResponse)
{
    if (eType != ResponseType::PROVISIONAL_RESPONSE)
    {
        return;
    }

    MtcExtension::HandleResponse(eType, objResponse);
}
