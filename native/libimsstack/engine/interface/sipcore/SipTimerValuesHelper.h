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
#ifndef SIP_TIMER_VALUES_HELPER_H_
#define SIP_TIMER_VALUES_HELPER_H_

#include "AString.h"
#include "SipTimerValues.h"

class SipProfile;

/**
 * @brief This class defines the helper class to set the SIP transaction timer values.
 */
class SipTimerValuesHelper
{
public:
    /// Types of SIP transaction
    enum
    {
        /// non-INVITE client transaction
        NON_INVITE_CLIENT = 0,
        /// non-INVITE server transaction
        NON_INVITE_SERVER = 1,
        /// INVITE client transaction
        INVITE_CLIENT = 2,
        /// INVITE server transaction
        INVITE_SERVER = 3
    };

public:
    SipTimerValuesHelper() = delete;

public:
    /**
     * @brief Creates SipTimerValues from the given information.
     *
     * @param nSlotId Current slot id
     * @param pSipProfile SIP profile for this SIP transaction timer
     * @param nTxnType SIP transaction type\n
     *                 #NON_INVITE_CLIENT\n
     *                 #NON_INVITE_SERVER\n
     *                 #INVITE_CLIENT\n
     *                 #INVITE_SERVER
     * @return The newly created SipTimerValues.
     */
    static SipTimerValues GetValues(IN IMS_SINT32 nSlotId,
            IN const SipProfile* pSipProfile = IMS_NULL,
            IN IMS_SINT32 nTxnType = NON_INVITE_CLIENT);
};

#endif
