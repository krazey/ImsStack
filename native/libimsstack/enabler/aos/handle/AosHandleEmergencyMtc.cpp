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
#include "ServiceTrace.h"

#include "interface/IAosAppContext.h"
#include "interface/IAosNConfiguration.h"
#include "provider/AosProvider.h"

#include "handle/AosHandleEmergencyMtc.h"

__IMS_TRACE_TAG_AOS__;

#define APPPROFILE m_strTag.GetStr()

PUBLIC
AosHandleEmergencyMtc::AosHandleEmergencyMtc(IN IAosAppContext* piAosAppContext,
        IN const AString& strAppId, IN const AString& strServiceId,
        IN const IMS_SINT32 nServiceType) :
        AosHandle(piAosAppContext, strAppId, strServiceId, nServiceType)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosHandleEmergencyMtc = %" PFLS_u "/%" PFLS_x,
            strAppId.GetStr(), sizeof(AosHandleEmergencyMtc), this);
}

PUBLIC VIRTUAL AosHandleEmergencyMtc::~AosHandleEmergencyMtc()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosHandleEmergencyMtc = %" PFLS_u "/%" PFLS_x,
            m_strAppId.GetStr(), sizeof(AosHandleEmergencyMtc), this);
}

PROTECTED VIRTUAL void AosHandleEmergencyMtc::InitializeServiceFeature()
{
    m_objFeatureTagList.Clear();

    m_objFeatureTagList.AddFeature(ImsAosFeature::MMTEL);

    if (GET_N_CONFIG(m_nSlotId)->IsRttSupported())
    {
        m_objFeatureTagList.AddFeature(ImsAosFeature::TEXT);
    }

    A_IMS_TRACE_I(APPPROFILE, "InitializeServiceFeature :: Features(%x)",
            m_objFeatureTagList.GetFeatures(), 0, 0);
}