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
#ifndef SDP_PARSER_H_
#define SDP_PARSER_H_

#include "SdpMediaDescription.h"
#include "SdpSessionDescription.h"

class SdpParser
{
public:
    SdpParser();
    ~SdpParser();

public:
    /**
     * @brief Decodes the SDP message.
     *        It creates a session & media level description.
     */
    IMS_BOOL Decode(IN const AString& strSdp);

    /**
     * @brief Decodes the SDP message without session-level description.
     *        It creates only a media-level description.
     */
    IMS_BOOL DecodeWithoutSession(IN const AString& strSdp);

    /**
     * @brief Returns the session-level description.
     */
    inline const SdpSessionDescription& GetSessionDescription() const
    {
        return m_objSessionDescription;
    }

    /**
     * @brief Returns the count of media-level description.
     */
    inline IMS_SINT32 GetMediaCount() const
    {
        return static_cast<IMS_SINT32>(m_objMediaDescriptions.GetSize());
    }

    /**
     * @brief Returns the media-level description at the specified index.
     */
    const SdpMediaDescription* GetMediaDescription(IN IMS_UINT32 nIndex) const;

    /**
     * @brief Returns all the media-level descriptions.
     */
    inline const IMSList<SdpMediaDescription>& GetMediaDescriptions() const
    {
        return m_objMediaDescriptions;
    }

    /**
     * @brief Validates the SDP after parsing the message.
     */
    IMS_BOOL ValidateSdp() const;

private:
    /**
     * @brief Returns all the media indexes from the specified SDP lines.
     */
    static IMSList<IMS_SINT32> GetMediaIndexes(IN const AStringArray& objSdpLines);

    /**
     * @brief Splits the message for each lines.
     */
    static void SplitLines(IN const AString& strSdp, OUT AStringArray& objSdpLines);

private:
    SdpSessionDescription m_objSessionDescription;
    IMSList<SdpMediaDescription> m_objMediaDescriptions;
};

#endif
