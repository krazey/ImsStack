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
#ifndef SDP_MEDIA_GROUP_H_
#define SDP_MEDIA_GROUP_H_

#include "AStringArray.h"

class SdpMediaParameter;

class SdpMediaGroup
{
public:
    SdpMediaGroup();
    SdpMediaGroup(IN const SdpMediaGroup& other);
    ~SdpMediaGroup();

public:
    SdpMediaGroup& operator=(IN const SdpMediaGroup& other);

public:
    /**
     * @brief Creates a media group from the group attribute & media parameters.
     */
    IMS_BOOL Create(IN const AString& strGroupAttribute,
            IN const ImsList<SdpMediaParameter*>& objMediaParams);

    /**
     * @brief Returns the type of group.
     *
     * @return The type of group.\n
     *         #LS\n
     *         #FID\n
     *         #SRF\n
     *         #GROUP_OTHER
     */
    inline IMS_SINT32 GetType() const { return m_nType; }

    /**
     * @brief Returns the type of group as a string value.
     *
     * @return The type of group as a string value.\n
     *         "LS"\n
     *         "FID"\n
     *         "SRF"
     */
    inline const AString& GetTypeEx() const { return m_strType; }

    /**
     * @brief Returns all the mids which are included in the current media group.
     *
     * @return The list of all mids.\n
     *         #LS\n
     *         #FID\n
     *         #SRF\n
     *         #GROUP_OTHER
     */
    inline const AStringArray& GetMids() const { return m_objMids; }

    /**
     * @brief Returns all the list of media stream index in the current media group.
     */
    inline const ImsList<IMS_SINT32>& GetMediaStreams() const { return m_objMediaStreamIndexes; }

    /**
     * @brief Removes the media stream which matches with the specified mid value.
     */
    IMS_BOOL RemoveMediaStream(IN IMS_SINT32 nMid);

    /**
     * @brief Returns the SDP lines for this media group.
     */
    AString ToSdp() const;

public:
    // "group" type
    enum
    {
        LS,   // Lip Synchronization
        FID,  // Flow Identification
        SRF,  // Single Reservation Flow
        GROUP_OTHER
    };

private:
    // Type of group
    IMS_SINT32 m_nType;
    AString m_strType;
    AStringArray m_objMids;
    ImsList<IMS_SINT32> m_objMediaStreamIndexes;
};

#endif
