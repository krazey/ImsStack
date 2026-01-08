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
#ifndef SDP_MEDIA_DESCRIPTION_H_
#define SDP_MEDIA_DESCRIPTION_H_

#include "SdpConnection.h"
#include "SdpDescription.h"
#include "SdpMedia.h"

class SdpMediaDescription : public SdpDescription
{
public:
    SdpMediaDescription();
    SdpMediaDescription(IN const SdpMediaDescription& other);
    ~SdpMediaDescription() override;

public:
    SdpMediaDescription& operator=(IN const SdpMediaDescription& other);

public:
    // SdpDescription class
    /**
     * @brief Decodes the SDP lines in the media-level session description.
     */
    IMS_BOOL Decode(IN const AStringArray& objLines, IN IMS_SINT32 nStartLine = 0,
            IN IMS_SINT32 nEndLine = -1) override;

    /**
     * @brief Encodes the SDP lines in the media-level session description.
     */
    AString Encode() const override;

    /**
     * @brief Returns the SdpMedia object in the media-level description.
     */
    inline const SdpMedia& GetMedia() const { return m_objMedia; }

    /**
     * @brief Returns the SdpConnection object in the media-level description.
     */
    const SdpConnection* GetConnection() const;

    /**
     * @brief Returns the list of SdpConnection object in the media-level description.
     */
    inline const ImsList<SdpConnection>& GetConnections() const { return m_objConnections; }

    /**
     * @brief Removes all the SdpConnection objects from the media-level description.
     */
    void RemoveConnections();

    /**
     * @brief Sets the SdpConnection objects to the media-level description.
     */
    void SetConnections(IN const ImsList<SdpConnection>& objConnections);

    /**
     * @brief Sets the SdpMedia object to the media-level description.
     */
    inline void SetMedia(IN const SdpMedia& objMedia) { m_objMedia = objMedia; }

private:
    SdpMedia m_objMedia;
    ImsList<SdpConnection> m_objConnections;
};

#endif
