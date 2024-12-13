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

#include "handle/AosHandleEmergencyMts.h"

__IMS_TRACE_TAG_AOS__;

#define APPPROFILE m_strTag.GetStr()

PUBLIC
AosHandleEmergencyMts::AosHandleEmergencyMts(IN IAosAppContext* piAppContext,
        IN const AString& strAppId, IN const AString& strServiceId,
        IN const IMS_SINT32 nServiceType) :
        AosHandle(piAppContext, strAppId, strServiceId, nServiceType)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosHandleEmergencyMts = %" PFLS_u "/%" PFLS_x,
            strAppId.GetStr(), sizeof(AosHandleEmergencyMts), this);
}

PUBLIC VIRTUAL AosHandleEmergencyMts::~AosHandleEmergencyMts()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosHandleEmergencyMts = %" PFLS_u "/%" PFLS_x,
            m_strAppId.GetStr(), sizeof(AosHandleEmergencyMts), this);
}

PROTECTED VIRTUAL void AosHandleEmergencyMts::InitializeServiceBlock()
{
    if (!GET_N_CONFIG(m_nSlotId)->IsEmergencySmsOverImsSupported())
    {
        m_bBlocked = IMS_TRUE;
    }

    A_IMS_TRACE_I(APPPROFILE, "InitializeServiceBlock :: block(%s)", _TRACE_B_(m_bBlocked), 0, 0);
}

PROTECTED VIRTUAL void AosHandleEmergencyMts::InitializeServiceFeature()
{
    m_objFeatureTagList.Clear();

    if (GET_N_CONFIG(m_nSlotId)->IsEmergencySmsOverImsSupported())
    {
        m_objFeatureTagList.AddFeature(ImsAosFeature::SMSIP);
    }

    A_IMS_TRACE_I(APPPROFILE, "InitializeServiceFeature :: Features(%x)",
            m_objFeatureTagList.GetFeatures(), 0, 0);
}