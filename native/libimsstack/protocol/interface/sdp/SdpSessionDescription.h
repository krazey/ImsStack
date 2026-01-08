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
#ifndef SDP_SESSION_DESCRIPTION_H_
#define SDP_SESSION_DESCRIPTION_H_

#include "SdpDescription.h"
#include "SdpOrigin.h"
#include "SdpSessionName.h"
#include "SdpTimeDescription.h"
#include "SdpVersion.h"

class IpAddress;

class SdpConnection;
class SdpTimezone;
class SdpUri;

class SdpSessionDescription : public SdpDescription
{
public:
    SdpSessionDescription();
    SdpSessionDescription(IN const SdpSessionDescription& other);
    ~SdpSessionDescription() override;

public:
    SdpSessionDescription& operator=(IN const SdpSessionDescription& other);

public:
    // SdpDescription class
    /**
     * @brief Decodes the SDP lines in the session-level session description.
     */
    IMS_BOOL Decode(IN const AStringArray& objLines, IN IMS_SINT32 nStartLine = 0,
            IN IMS_SINT32 nEndLine = -1) override;

    /**
     * @brief Encodes the SDP lines in the sessions-level session description.
     */
    AString Encode() const override;

    /**
     * @brief Creates the mandatory SDP lines for the session-level description.
     */
    IMS_BOOL CreateMandatoryLines(IN const AString& strUserId, IN const IpAddress& objLocalAddress);

    /**
     * @brief Returns the SdpVersion object from the session-level description.
     */
    inline const SdpVersion& GetVersion() const { return m_objVersion; }

    /**
     * @brief Returns the SdpOrigin object from the session-level description.
     */
    inline const SdpOrigin& GetOrigin() const { return m_objOrigin; }

    /**
     * @brief Returns the SdpSessionName object from the session-level description.
     */
    inline const SdpSessionName& GetSessionName() const { return m_objSessionName; }

    /**
     * @brief Returns the SdpConnection object from the session-level description.
     */
    inline const SdpConnection* GetConnection() const { return m_pConnection; }

    /**
     * @brief Returns the SdpUri object from the session-level description.
     */
    inline const SdpUri* GetUri() const { return m_pUri; }

    /**
     * @brief Returns the SdpTimeDescription objects from the session-level description.
     */
    inline const ImsList<SdpTimeDescription>& GetTimeDescriptions() const
    {
        return m_objTimeDescriptions;
    }

    /**
     * @brief Returns the SdpTimezone object from the session-level description.
     */
    inline const SdpTimezone* GetTimezone() const { return m_pTimezone; }

    /**
     * @brief Sets the session name in the session-level description.
     */
    void SetSessionName(IN const SdpSessionName& objSessionName);

    /**
     * @brief Sets the connection line in the session-level description.
     */
    void SetConnection(IN const SdpConnection& objConnection);

protected:
    // SDP order: v, o, s, i, u, e, p, c, b, 1*(t, *r), z, k, *(a), *(m)
    SdpVersion m_objVersion;
    SdpOrigin m_objOrigin;
    SdpSessionName m_objSessionName;
    SdpUri* m_pUri;
    SdpConnection* m_pConnection;
    ImsList<SdpTimeDescription> m_objTimeDescriptions;
    SdpTimezone* m_pTimezone;

    // SDPEmail / SDPPhone does not include in this version
};

#endif
