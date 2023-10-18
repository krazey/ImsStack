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
#include "ServiceMemory.h"

#include "ISipHeader.h"
#include "ISipMessage.h"
#include "Sip.h"
#include "SipParsingHelper.h"
#include "util/OperatorFeatureResolver.h"

PUBLIC GLOBAL IMS_BOOL OperatorFeatureResolver::IsMessageForEarlySessionModel(
        IN const ISipMessage* piSipMsg)
{
    if (piSipMsg == IMS_NULL)
    {
        return IMS_FALSE;
    }

    const AString strEarlySession(Sip::STR_EARLY_SESSION);

    if (!piSipMsg->IsOptionRequired(strEarlySession))
    {
        return IMS_FALSE;
    }

    AString strHeaderBody = piSipMsg->GetHeader(ISipHeader::CONTENT_DISPOSITION);

    if (strHeaderBody.GetLength() == 0)
    {
        return IMS_FALSE;
    }

    ISipHeader* piHeader =
            SipParsingHelper::CreateHeader(ISipHeader::CONTENT_DISPOSITION, strHeaderBody);

    if (piHeader == IMS_NULL)
    {
        return IMS_FALSE;
    }

    AString strHeaderValue = piHeader->GetValue();
    piHeader->Destroy();

    if (!strHeaderValue.EqualsIgnoreCase(strEarlySession))
    {
        return IMS_FALSE;
    }

    return (piSipMsg->GetSdpBodyPart() != IMS_NULL);
}
