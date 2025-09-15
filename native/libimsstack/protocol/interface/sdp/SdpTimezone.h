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
#ifndef SDP_TIMEZONE_H_
#define SDP_TIMEZONE_H_

#include "SdpLine.h"

class SdpTimezone : public SdpLine
{
public:
    class ZoneAdjustment
    {
    public:
        inline ZoneAdjustment() :
                m_nAdjustmentTime(0),
                m_nOffset(0)
        {
        }
        inline ZoneAdjustment(IN IMS_UINT32 nAdjustmentTime, IN IMS_SINT32 nOffset) :
                m_nAdjustmentTime(nAdjustmentTime),
                m_nOffset(nOffset)
        {
        }
        inline ZoneAdjustment(IN const ZoneAdjustment& other) :
                m_nAdjustmentTime(other.m_nAdjustmentTime),
                m_nOffset(other.m_nOffset)
        {
        }

    public:
        inline ZoneAdjustment& operator=(IN const ZoneAdjustment& other)
        {
            if (this != &other)
            {
                m_nAdjustmentTime = other.m_nAdjustmentTime;
                m_nOffset = other.m_nOffset;
            }

            return (*this);
        }

        inline IMS_BOOL Equals(IN const ZoneAdjustment& other) const
        {
            return m_nAdjustmentTime == other.m_nAdjustmentTime && m_nOffset == other.m_nOffset;
        }

    public:
        inline IMS_UINT32 GetAdjustmentTime() const { return m_nAdjustmentTime; }
        inline IMS_SINT32 GetOffset() const { return m_nOffset; }

    private:
        IMS_UINT32 m_nAdjustmentTime;
        IMS_SINT32 m_nOffset;
    };

public:
    SdpTimezone();
    SdpTimezone(IN const SdpTimezone& other);
    ~SdpTimezone() override;

public:
    SdpTimezone& operator=(IN const SdpTimezone& other);

public:
    // SdpLine class
    /**
     * @brief Decodes the timezone line ("z=") in the session description.
     *        The strValue contains a full timezone line without "z=".
     */
    IMS_BOOL Decode(IN const AString& strValue) override;

    /**
     * @brief Encodes the timezone line ("z=") in the session description.
     *        The returned value contains a full timezone line with "z=".
     */
    AString Encode() const override;

    /**
     * @brief Returns the full timezone line without "z=".
     */
    AString GetValue() const override;

    /**
     * @brief Adds the adjustment time & offset to the timezone line.
     */
    IMS_BOOL AddAdjustment(IN IMS_UINT32 nAdjustmentTime, IN IMS_SINT32 nOffset);

    /**
     * @brief Returns the ZoneAdjustment objects from the timezone line.
     */
    inline const ImsList<SdpTimezone::ZoneAdjustment>& GetAdjustments() const
    {
        return m_objZoneAdjustments;
    }

private:
    // z=<adjustment time> <offset> <adjustment time> <offset> ...
    ImsList<ZoneAdjustment> m_objZoneAdjustments;
};

inline IMS_BOOL operator==(
        IN const SdpTimezone::ZoneAdjustment& objZa1, IN const SdpTimezone::ZoneAdjustment& objZa2)
{
    return objZa1.Equals(objZa2);
}

#endif
