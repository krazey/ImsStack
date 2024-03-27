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

#ifndef AUDIO_NEGO_AMR_H_
#define AUDIO_NEGO_AMR_H_

#include "audio/AudioProfile.h"

class AString;

class AudioNegoAmr
{
public:
    static AString SetSdpFmtpFromAmrFmtp(IN AudioProfile::AmrFmtp* amrFmtp);
    static void AppendSeparatorIfNotEmpty(OUT AString& str, IN AString separator);
    static void AddModeSetListToFmtp(IN AudioProfile::AmrFmtp* profile, OUT AString& strFmtp);
    static void AddOctetAlignToFmtp(IN AudioProfile::AmrFmtp* profile, OUT AString& strFmtp);
    static void AddModeChangeCapabilityToFmtp(
            IN AudioProfile::AmrFmtp* profile, OUT AString& strFmtp);
    static void AddModeChangePeriodToFmtp(IN AudioProfile::AmrFmtp* profile, OUT AString& strFmtp);
    static void AddModeChangeNeighborToFmtp(
            IN AudioProfile::AmrFmtp* profile, OUT AString& strFmtp);
    static void AddMaxRedToFmtp(IN AudioProfile::AmrFmtp* profile, OUT AString& strFmtp);
    static void AddPtimeToFmtp(IN AudioProfile::AmrFmtp* profile, OUT AString& strFmtp);
    static void AddMaxPtimeToFmtp(IN AudioProfile::AmrFmtp* profile, OUT AString& strFmtp);
};
#endif