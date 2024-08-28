/**
 * Copyright (C) 2024 The Android Open Source Project
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

#ifndef AUDIO_NEGO_EVS_H_
#define AUDIO_NEGO_EVS_H_

#include "audio/AudioProfile.h"

class AudioNegoEvs
{
public:
    static AString SetSdpFmtpFromEvsFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp);
    static void AppendSeparatorIfNotEmpty(OUT AString& str, IN AString separator);
    static void AddDtxToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddHfOnlyToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddEvsModeSwitchToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddMaxRedToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddBwToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddBrToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddCmrToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddChannelAwModeToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddModeSetListToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddModeChangeCapabilityToFmtp(
            IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddModeChangePeriodToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddModeChangeNeighborToFmtp(
            IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddBwSendToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddBwRecvToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddBrSendToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
    static void AddBrRecvToFmtp(IN AudioProfile::EvsFmtp* pEvsFmtp, OUT AString& strFmtp);
};
#endif
