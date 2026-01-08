/*
 * Copyright (C) 2023 The Android Open Source Project
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
#include "ImsList.h"
#include "MtcDef.h"
#include "SipHeaderName.h"
#include "helper/MtcLocationObject.h"
#include "utility/CallComposerUtil.h"
#include <memory>

LOCAL const IMS_CHAR PRIORITY_NONE[] = "none";
LOCAL const IMS_CHAR PRIORITY_URGENT[] = "urgent";
LOCAL const IMS_CHAR PICTURE_URL_FORMAT[] = "<%s>;purpose=icon";
LOCAL const AString PICTURE_URL_PARAMETER = ";purpose=icon";

GLOBAL IMS_SINT32 CallComposerUtil::GetPriority(IN const IMessage& objMessage)
{
    ImsList<AString> lstHeaders = objMessage.GetHeaders(SipHeaderName::PRIORITY);
    AString strPriority = lstHeaders.GetSize() > 0 ? lstHeaders.GetAt(0) : AString::ConstNull();

    if (strPriority.EqualsIgnoreCase(PRIORITY_NONE))
    {
        return CALL_COMPOSER_PRIORITY_NONE;
    }
    else if (strPriority.EqualsIgnoreCase(PRIORITY_URGENT))
    {
        return CALL_COMPOSER_PRIORITY_URGENT;
    }
    return -1;
}

GLOBAL AString CallComposerUtil::GetSubject(IN const IMessage& objMessage)
{
    ImsList<AString> lstHeaders = objMessage.GetHeaders(SipHeaderName::SUBJECT);
    return lstHeaders.GetSize() > 0 ? lstHeaders.GetAt(0) : AString::ConstNull();
}

GLOBAL AString CallComposerUtil::GetPicture(IN const IMessage& objMessage)
{
    ImsList<AString> lstHeaders = objMessage.GetHeaders(SipHeaderName::CALL_INFO);
    for (IMS_UINT32 i = 0; i < lstHeaders.GetSize(); i++)
    {
        AString strHeader = lstHeaders.GetAt(i);
        if (strHeader.MakeLower().EndsWith(PICTURE_URL_PARAMETER))
        {
            strHeader = strHeader.Left(strHeader.GetLength() - PICTURE_URL_PARAMETER.GetLength());
            strHeader = strHeader.Replace("<", "");
            strHeader = strHeader.Replace(">", "");
            return strHeader;
        }
    }
    return AString::ConstNull();
}

GLOBAL std::pair<AString, AString> CallComposerUtil::GetLocation(IN const IMessage& objMessage)
{
    std::unique_ptr<MtcLocationProperties> pLocation(
            MtcLocationObject::GetLocationFromMessage(objMessage));
    if (pLocation == nullptr)
    {
        return std::make_pair(AString::ConstNull(), AString::ConstNull());
    }

    return std::make_pair(pLocation->GetLatitude(), pLocation->GetLongitude());
}

GLOBAL IMS_BOOL CallComposerUtil::IsBusiness(IN const IMessage& objMessage)
{
    ImsList<AString> lstHeaders = objMessage.GetHeaders(SipHeaderName::ORGANIZATION);
    return lstHeaders.GetSize() > 0 && lstHeaders.GetAt(0).GetLength() > 0;
}

GLOBAL void CallComposerUtil::SetPriority(IN IMS_SINT32 nPriority, OUT IMessage& objMessage)
{
    switch (nPriority)
    {
        case CALL_COMPOSER_PRIORITY_NONE:
            objMessage.AddHeader(SipHeaderName::PRIORITY, PRIORITY_NONE);
            break;

        case CALL_COMPOSER_PRIORITY_URGENT:
            objMessage.AddHeader(SipHeaderName::PRIORITY, PRIORITY_URGENT);
            break;

        default:
            break;
    }
}

GLOBAL void CallComposerUtil::SetSubject(IN const AString& strSubject, OUT IMessage& objMessage)
{
    if (strSubject.GetLength() <= 0)
    {
        return;
    }

    objMessage.AddHeader(SipHeaderName::SUBJECT, strSubject);
}

GLOBAL void CallComposerUtil::SetPicture(IN const AString& strUrl, OUT IMessage& objMessage)
{
    if (strUrl.GetLength() <= 0)
    {
        return;
    }

    AString strHeaderValue;
    strHeaderValue.Sprintf(PICTURE_URL_FORMAT, strUrl.GetStr());
    objMessage.AddHeader(SipHeaderName::CALL_INFO, strHeaderValue);
}

GLOBAL void CallComposerUtil::SetLocation(IN const AString& strLatitude,
        IN const AString& strLongitude, IN IMtcCallContext& objContext, OUT IMessage& objMessage)
{
    if (strLatitude.GetLength() <= 0 || strLongitude.GetLength() <= 0)
    {
        return;
    }

    MtcLocationObject objLocation(objContext);
    objLocation.SetLocationToMessage(objMessage, IMS_FALSE,
            objLocation.CreateCallComposerLocationBody(strLatitude, strLongitude));
}
