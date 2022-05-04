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
#ifndef INTERFACE_SIP_TOKEN_GENERATOR_H_
#define INTERFACE_SIP_TOKEN_GENERATOR_H_

#include "AString.h"

/**
 * @brief This class provides an interface to generate SIP related tokens.
 */
class ISipTokenGenerator
{
public:
    /**
     * @brief Gets the boundary token for the multipart MIME body parts.
     *
     * @param strBoundary Boundary token
     */
    virtual void GenerateBoundary(OUT AString& strBoundary) = 0;

    /**
     * @brief Gets the call-id token for the Call-ID header.
     *
     * @param strHost Host information (i.e. IP address of the device)
     * @param strCallId Call-id token
     */
    virtual void GenerateCallId(IN const AString& strHost, OUT AString& strCallId) = 0;

    /**
     * @brief Gets the tag token for the From/To header.
     *
     * @param strMagicCookie Magic cookie (prefix of the tag)
     * @param strTag Tag token
     */
    virtual void GenerateTag(IN const AString& strMagicCookie, OUT AString& strTag) = 0;

    /**
     * @brief Gets the via branch token for the Via header.
     *
     * @param strViaBranch Via branch token
     * @param strExtensionToken Extension token
     */
    virtual void GenerateViaBranch(OUT AString& strViaBranch,
            IN const AString& strExtensionToken = AString::ConstNull()) = 0;
};

#endif
