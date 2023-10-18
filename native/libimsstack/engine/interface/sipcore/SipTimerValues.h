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
#ifndef SIP_TIMER_VALUES_H_
#define SIP_TIMER_VALUES_H_

#include "ImsTypeDef.h"

/**
 * @brief This class defines the SIP transaction timer values which are set by the application.
 */
class SipTimerValues
{
public:
    SipTimerValues();
    SipTimerValues(IN const SipTimerValues& other);
    ~SipTimerValues();

public:
    SipTimerValues& operator=(IN const SipTimerValues& other);

public:
    /**
     * @brief Gets the timer value for the give type.
     *
     * @param nType Type of SIP timer\n
     *              #TIMER_T1\n
     *              #TIMER_T2\n
     *              #TV_TIMER_B\n
     *              #TV_TIMER_D\n
     *              #TV_TIMER_F\n
     *              #TV_TIMER_H\n
     *              #TV_TIMER_I\n
     *              #TV_TIMER_J\n
     *              #TV_TIMER_K
     * @return The timer value.
     */
    IMS_SINT32 GetValue(IN IMS_SINT32 nType) const;

    /**
     * @brief Checks if the given timer is provisioned or not.
     *
     * @param nType Type of SIP timer\n
     *              #TIMER_T1\n
     *              #TIMER_T2\n
     *              #TV_TIMER_B\n
     *              #TV_TIMER_D\n
     *              #TV_TIMER_F\n
     *              #TV_TIMER_H\n
     *              #TV_TIMER_I\n
     *              #TV_TIMER_J\n
     *              #TV_TIMER_K
     * @return If the timer is provisioned, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    inline IMS_BOOL IsSet(IN IMS_SINT32 nType) const { return ((m_nFlags & nType) != 0); }

    /**
     * @brief Sets the timer value.
     *
     * @param nType Type of SIP timer\n
     *              #TIMER_T1\n
     *              #TIMER_T2\n
     *              #TV_TIMER_B\n
     *              #TV_TIMER_D\n
     *              #TV_TIMER_F\n
     *              #TV_TIMER_H\n
     *              #TV_TIMER_I\n
     *              #TV_TIMER_J\n
     *              #TV_TIMER_K
     * @param nValue Timer value to be set
     */
    void SetValue(IN IMS_SINT32 nType, IN IMS_UINT32 nValue);

    /**
     * @brief Creates SipTimerValues based on T1 and T2.
     *
     * @param nT1 T1 timer value
     * @param nT2 T2 timer value
     * @return The newly created SipTimerValues.
     */
    static SipTimerValues CreateTimerValues(IN IMS_SINT32 nT1, IN IMS_SINT32 nT2);

public:
    /// Types of SIP transaction timer
    enum
    {
        /// T1: RTT estimate
        TIMER_T1 = 0x0001,
        /// T2: The maximum retransmit interval for non-INVITE requests and INVITE responses
        TIMER_T2 = 0x0002,
        /// Timer B: INVITE transaction timeout timer
        TIMER_B = 0x0004,
        /// Timer D: Wait time for response retransmits
        TIMER_D = 0x0008,
        /// Timer F: non-INVITE transaction timeout timer
        TIMER_F = 0x0010,
        /// Tiimer H: Wait time for ACK receipt
        TIMER_H = 0x0020,
        /// Timer I: Wait time for ACK retransmits
        TIMER_I = 0x0040,
        /// Timer J: Wait time for non-INVITE request retransmits
        TIMER_J = 0x0080,
        /// Timer K: Wait time for response retransmits
        TIMER_K = 0x0100,
        TIMER_ALL = 0x01FF
    };

private:
    IMS_UINT32 m_nFlags;
    IMS_UINT32 m_nT1;
    IMS_UINT32 m_nT2;
    IMS_UINT32 m_nTimerB;  // UAC, INVITE
    IMS_UINT32 m_nTimerD;  // UAC, INVITE response retransmit received
    IMS_UINT32 m_nTimerF;  // UAC, non-INVITE
    IMS_UINT32 m_nTimerH;  // ACK receipt
    IMS_UINT32 m_nTimerI;  // ACK retransmit received
    IMS_UINT32 m_nTimerJ;  // UAS, non-INVITE
    IMS_UINT32 m_nTimerK;  // UAC, non-INVITE
};

#endif
