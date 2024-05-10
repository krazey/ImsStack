/**
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

#ifndef TEXT_PROFILE_UTIL_H_
#define TEXT_PROFILE_UTIL_H_

class TextConfiguration;

#include "text/TextProfile.h"

class TextProfileUtil
{
public:
    /**
     * @brief Make negotiated bandwidth AS/RS/RR value in SDP attribute with the parameters
     *
     * @param pConfig The carrier configuration for text
     * @param pLocalProfile The local TextProfile
     * @param pPeerProfile The peer TextProfile created from the SDP
     * @param isOfferReceived The flag to identify the offer received
     * @param nASValueOfNegoticatedCodec The bandwidth AS attribute negotiated
     * @param pNegotiatedProfile The negotiated TextProfile to update
     */
    static void MakeNegotiatedBandwidth(IN TextConfiguration* pConfig,
            IN TextProfile* pLocalProfile, IN TextProfile* pPeerProfile,
            IN IMS_BOOL isOfferReceived, IN IMS_SINT32 nASValueOfNegoticatedCodec,
            OUT TextProfile* pNegotiatedProfile);
};

#endif
