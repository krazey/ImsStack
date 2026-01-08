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

#ifndef INTERFACE_MTC_DIALING_PLAN_H_
#define INTERFACE_MTC_DIALING_PLAN_H_

#include "AString.h"
#include "ImsTypeDef.h"
#include "dialingplan/NormalDialingPlan.h"

class IMtcCallContext;
using Scheme = NormalDialingPlan::Scheme;

/**
 * @brief An interface for a dialing plan that translates a dialed number into a SIP or TEL URI.
 */
class IMtcDialingPlan
{
public:
    virtual ~IMtcDialingPlan() {}

    /**
     * @brief Returns a translated SIP/TEL To URI.
     *
     * @param strNumber The number a user dialed.
     * @param objContext Call context which provides the information needed to combine the To URI.
     * @param eScheme The URI scheme for the translation.
     * @return The translated SIP/TEL URI.
     */
    virtual AString GetToUri(IN const AString& strNumber, IN IMtcCallContext& objContext,
            IN Scheme eScheme = Scheme::UNKNOWN) = 0;

    /**
     * @brief Returns a translated URI for an emergency test number.
     *
     * @param strNumber The emergency test number dialed by the user.
     * @param objContext Call context which provides the information needed to combine the To URI.
     * @return The translated URI for the emergency test number.
     */
    virtual AString GetToUriForEmergencyTestNumber(
            IN const AString& strNumber, IN IMtcCallContext& objContext) = 0;
};

#endif
