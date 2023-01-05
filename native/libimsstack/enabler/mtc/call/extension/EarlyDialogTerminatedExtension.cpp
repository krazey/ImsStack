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
#include "call/extension/EarlyDialogTerminatedExtension.h"
#include "call/extension/MtcExtensionSet.h"
#include "utility/MessageUtil.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
EarlyDialogTerminatedExtension::EarlyDialogTerminatedExtension() :
        MtcExtension(MtcExtensionSet::OPTION_TAG_EARLY_DIALOG_TERMINATED)
{
}

PUBLIC
EarlyDialogTerminatedExtension::EarlyDialogTerminatedExtension(
        IN const EarlyDialogTerminatedExtension& objRhs) :
        MtcExtension(objRhs)
{
}

PUBLIC VIRTUAL EarlyDialogTerminatedExtension::~EarlyDialogTerminatedExtension() {}

PUBLIC VIRTUAL IMtcExtension* EarlyDialogTerminatedExtension::Clone() const
{
    return new EarlyDialogTerminatedExtension(*this);
}

PUBLIC VIRTUAL void EarlyDialogTerminatedExtension::FormatRequest(
        IN RequestType eType, IN_OUT IMessage& objRequest)
{
    if (eType != RequestType::START)
    {
        return;
    }

    MessageUtil::AddValueIfNotExists(&objRequest, GetOptionTag(), ISipHeader::SUPPORTED);
}
