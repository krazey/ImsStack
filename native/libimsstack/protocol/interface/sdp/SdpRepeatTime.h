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
#ifndef SDP_REPEAT_TIME_H_
#define SDP_REPEAT_TIME_H_

#include "SdpLine.h"

class SdpRepeatTime : public SdpLine
{
public:
    SdpRepeatTime();
    SdpRepeatTime(IN const SdpRepeatTime& other);
    ~SdpRepeatTime() override;

public:
    SdpRepeatTime& operator=(IN const SdpRepeatTime& other);

public:
    // SdpLine class
    /**
     * @brief Decodes the repeat-time line ("r=") in the session description.
     *        The strValue contains a full repeat-time line without "r=".
     */
    IMS_BOOL Decode(IN const AString& strValue) override;

    /**
     * @brief Encodes the repeat-time line ("r=") in the session description.
     *        The returned value contains a full repeat-time line with "r=".
     */
    AString Encode() const override;

    /**
     * @brief Returns the full repeat-time line without "r=".
     */
    AString GetValue() const override;

    /**
     * @brief Returns the repeat interval from the repeat-time line.
     */
    inline IMS_UINT32 GetRepeatInterval() const { return m_nRepeatInterval; }

    /**
     * @brief Returns the active duration from the repeat-time line.
     */
    inline IMS_UINT32 GetActiveDuration() const { return m_nActiveDuration; }

    /**
     * @brief Returns the first offset value from the repeat-time line.
     */
    inline IMS_UINT32 GetFirstOffset() const { return m_nFirstOffset; }

    /**
     * @brief Returns the additional offsets from the repeat-time line.
     */
    inline const ImsList<IMS_UINT32>& GetAdditionalOffsets() const
    {
        return m_objAdditionalOffsets;
    }

    /**
     * @brief Sets all the parameters for the repeat-time line.
     */
    IMS_BOOL SetValue(IN IMS_UINT32 nInterval, IN IMS_UINT32 nActiveDuration,
            IN IMS_UINT32 nFirstOffset, IN const ImsList<IMS_UINT32>& objAdditionalOffsets);

private:
    IMS_BOOL IsValid() const;

private:
    // r=<repeat interval> <active duration> <offsets from start-time>
    IMS_UINT32 m_nRepeatInterval;
    IMS_UINT32 m_nActiveDuration;
    IMS_UINT32 m_nFirstOffset;
    ImsList<IMS_UINT32> m_objAdditionalOffsets;
};

#endif
