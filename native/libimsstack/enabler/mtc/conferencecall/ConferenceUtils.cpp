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
#include "ServiceTrace.h"
#include "SipAddress.h"
#include "call/IMtcCallManager.h"
#include "conferencecall/ConferenceUtils.h"

__IMS_TRACE_TAG_COM_MTC__;

// this can move into MessageUtil
PUBLIC GLOBAL const AString& ConferenceUtils::GetUserPart(
        IN const AString& strUri, OUT AString& strUserPart)
{
    if (strUri.Contains("sip") || strUri.Contains("tel"))
    {
        SipAddress objSIPAddress(strUri);

        if (objSIPAddress.GetUserInfoPart() != IMS_NULL)
        {
            strUserPart = objSIPAddress.GetUserInfoPart()->GetUser();
        }
    }
    else
    {
        strUserPart = strUri;
    }

    return strUserPart;
}
