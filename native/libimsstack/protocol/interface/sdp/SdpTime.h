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
#ifndef SDP_TIME_H_
#define SDP_TIME_H_

#include "SdpLine.h"

class SdpTime : public SdpLine
{
public:
    SdpTime();
    SdpTime(IN const SdpTime& other);
    ~SdpTime() override;

public:
    SdpTime& operator=(IN const SdpTime& other);

public:
    // SdpLine class
    /**
     * @brief Decodes the time line ("t=") in the session description.
     *        The strValue contains a full time line without "t=".
     */
    IMS_BOOL Decode(IN const AString& strValue) override;

    /**
     * @brief Encodes the time line ("t=") in the session description.
     *        The returned value contains a full time line with "t=".
     */
    AString Encode() const override;

    /**
     * @brief Returns the full time line without "t=".
     */
    AString GetValue() const override;

    /**
     * @brief Returns the start-time from the time line.
     */
    inline IMS_UINT32 GetStartTime() const { return m_nStartTime; }

    /**
     * @brief Returns the stop-time from the time line.
     */
    inline IMS_UINT32 GetStopTime() const { return m_nStopTime; }

    /**
     * @brief Sets all the parameters for the time line.
     */
    IMS_BOOL SetValue(IN IMS_UINT32 nStartTime = 0, IN IMS_UINT32 nStopTime = 0);

private:
    IMS_UINT32 m_nStartTime;
    IMS_UINT32 m_nStopTime;
};

#endif
