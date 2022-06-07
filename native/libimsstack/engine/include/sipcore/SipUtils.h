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
#ifndef SIP_UTILS_H_
#define SIP_UTILS_H_

#include "AString.h"

class SipUtils
{
public:
    SipUtils() = delete;

public:
    static AString GenerateBoundary();
    static AString GenerateCallId(IN const AString& strHost);
    // HEADER_REQ_SESSION-ID
    // draft-kaplan-insipid-session-id-04, for Session-ID header field
    static AString GenerateSessionId(IN IMS_SINT32 nSlotId, IN const AString& strCallId);
    static AString GenerateTag(IN const AString& strMagicCookie);
    static AString GenerateViaBranch(IN const IMS_CHAR* pszToTag, IN const IMS_CHAR* pszFromTag,
            IN const IMS_CHAR* pszCallId, IN const IMS_CHAR* pszRequestUri,
            IN const IMS_CHAR* pszTopmostVia, IN IMS_SINT32 nCSeqNum,
            IN const AString& strExtensionToken = AString::ConstNull());
    static AString GenerateViaBranch(IN const AString& strExtensionToken = AString::ConstNull());

    static void Init(IN IMS_SINT32 nSlotId);

private:
    // HEADER_REQ_SESSION-ID
    static AString* s_pFixedKeyForSessionId;
};

#endif
