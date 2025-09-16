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
#ifndef SDP_SESSION_PARAMETER_H_
#define SDP_SESSION_PARAMETER_H_

#include "SdpSessionDescription.h"
#include "offeranswer/SdpParameter.h"

class SdpSessionParameter : public SdpParameter
{
public:
    SdpSessionParameter();
    SdpSessionParameter(IN const SdpSessionParameter& other);
    ~SdpSessionParameter() override;

public:
    SdpSessionParameter& operator=(IN const SdpSessionParameter& other);

public:
    // SdpParameter class
    /**
     * @brief Returns the connection address from the session-level description.
     */
    const AString& GetConnectionAddress() const override;

    /**
     * @brief Returns the SDP message from the session-level parameter.
     */
    AString ToSdp() const override;

    /**
     * @brief Negotiates the session-level parameters through the SDP offer/answer exchange.
     */
    IMS_SINT32 Compare(
            IN const SdpSessionParameter& objPeerParam, OUT SdpSessionParameter& objProposalParam);

    /**
     * @brief Creates the session-level parameter from the SDP session description.
     */
    IMS_BOOL Create(IN const SdpSessionDescription& objSessionDescription);

    /**
     * @brief Returns the SdpOrigin object from the session-level parameter.
     */
    inline const SdpOrigin& GetOrigin() const { return m_objOrigin; }

    /**
     * @brief Returns the SdpSessionName object from the session-level parameter.
     */
    inline const SdpSessionName& GetSessionName() const { return m_objSessionName; }

    /**
     * @brief Returns the SdpSessionName object from the session-level parameter.
     */
    inline SdpSessionName& GetSessionName() { return m_objSessionName; }

    /**
     * @brief Returns the SdpVersion object from the session-level parameter.
     */
    inline const SdpVersion& GetVersion() const { return m_objVersion; }

    /**
     * @brief Increases the session version in the session-level parameter.
     */
    inline void IncreaseSessionVersion() { m_objOrigin.IncreaseSessionVersion(); }

    /**
     * @brief Checks if the version of the specified session parameter equals to this version.
     */
    IMS_BOOL IsSameVersion(IN const SdpSessionParameter* pSessionParam) const;

    /**
     * @brief Sets the connection line to the media-level parameter.
     */
    IMS_BOOL SetConnectionAddress(IN const AString& strAddress);

    /**
     * @brief Updates the properties from the specified session-level parameter.
     */
    void UpdateProperties(IN const SdpSessionParameter& objSessionParam);

private:
    /**
     * @brief Clears the properties for the session-level parameter.
     */
    void Clear() override;

    /**
     * @brief Clears the properties for the session-level parameter.
     */
    void ClearAllSessionParameters();

private:
    // Version
    SdpVersion m_objVersion;
    SdpOrigin m_objOrigin;

    // s-line information
    SdpSessionName m_objSessionName;

    // u-line
    SdpUri* m_pUri;

    // e-line, list type

    // p-line, list type

    // c-line information
    SdpConnection* m_pConnection;

    // Used to resume the held media if the c=0.0.0.0 is used to mute.
    SdpConnection* m_pPreviousConnection;

    // t-line & r-line & z-line
    ImsList<SdpTimeDescription> m_objTimeDescriptions;
    SdpTimezone* m_pTimezone;
};

#endif
