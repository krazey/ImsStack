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
#ifndef SDP_TIME_DESCRIPTION_H_
#define SDP_TIME_DESCRIPTION_H_

#include "SdpRepeatTime.h"

class AStringArray;

class SdpTime;

class SdpTimeDescription
{
public:
    SdpTimeDescription();
    explicit SdpTimeDescription(IN SdpTime* pTime);
    SdpTimeDescription(IN const SdpTimeDescription& other);
    ~SdpTimeDescription();

public:
    SdpTimeDescription& operator=(IN const SdpTimeDescription& other);

public:
    /**
     * @brief Decodes the SDP lines in the time description.
     */
    IMS_BOOL Decode(IN const AStringArray& objLines, IN IMS_SINT32 nStartLine = 0,
            IN IMS_SINT32 nEndLine = -1);

    /**
     * @brief Encodes the SDP lines in the time description.
     */
    AString Encode() const;

    /**
     * @brief Returns the SdpTime object from the time description.
     */
    inline const SdpTime* GetTime() const { return m_pTime; }

    /**
     * @brief Returns the SdpTime object from the time description.
     */
    inline SdpTime* GetTime() { return m_pTime; }

    /**
     * @brief Adds the repeat-time line into the time description.
     */
    IMS_BOOL AddRepeatTime(IN const SdpRepeatTime& objRepeatTime);

    /**
     * @brief Returns the SdpRepeatTime objects from the time description.
     */
    inline const ImsList<SdpRepeatTime>& GetRepeatTimes() const { return m_objRepeatTimes; }

private:
    // SDP order: t, *(r)
    SdpTime* m_pTime;
    ImsList<SdpRepeatTime> m_objRepeatTimes;
};

#endif
