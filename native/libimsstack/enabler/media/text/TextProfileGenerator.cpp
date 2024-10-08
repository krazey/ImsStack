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

#include "ServiceTrace.h"
#include "MediaProfileUtil.h"
#include "config/TextConfiguration.h"
#include "text/TextProfileGenerator.h"

__IMS_TRACE_TAG_MEDIA__;

PUBLIC TextProfileGenerator::TextProfileGenerator() :
        MediaProfileGenerator(MEDIA_TYPE_TEXT)
{
    IMS_TRACE_I("+TextProfileGenerator()", 0, 0, 0);
}

PUBLIC VIRTUAL TextProfileGenerator::~TextProfileGenerator()
{
    IMS_TRACE_I("~TextProfileGenerator()", 0, 0, 0);
}

PUBLIC
TextProfile* TextProfileGenerator::SetProfile(IN MediaBaseProfile* pProfile,
        IN MediaConfiguration* pConfig, IN MediaEnvironment* pEnvironment, IN IMS_SINT32 nSlotId)
{
    if (pProfile == IMS_NULL || pConfig == IMS_NULL || pEnvironment == IMS_NULL)
    {
        return IMS_NULL;
    }

    SetCommonProfile(pProfile, pConfig, pEnvironment, nSlotId);

    TextProfile* pTextProfile = static_cast<TextProfile*>(pProfile);
    TextConfiguration* pTextConfig = static_cast<TextConfiguration*>(pConfig);

    pTextProfile->SetTransportType("RTP/AVP");
    pTextProfile->SetKeepRedundantLevel(pTextConfig->IsTextCodecEmptyRedundantEnabled());

    IMS_TRACE_I("SetProfile() - transport type[%s], keep red level[%d]",
            pTextProfile->GetTransportType().GetStr(), pTextProfile->GetKeepRedundantLevel(), 0);

    return pTextProfile;
}
