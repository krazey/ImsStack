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
#include "handle/AosHandleSipController.h"
#include "provider/AosProvider.h"

__IMS_TRACE_TAG_AOS__;

#define APPPROFILE m_strTag.GetStr()

PUBLIC
AosHandleSipController::AosHandleSipController(IN IAosAppContext* piAosAppContext,
        IN const AString& strAppId, IN const AString& strServiceId,
        IN const IMS_SINT32 nServiceType) :
        AosHandle(piAosAppContext, strAppId, strServiceId, nServiceType)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosHandleSipController = %" PFLS_u "/%" PFLS_x,
            strAppId.GetStr(), sizeof(AosHandleSipController), this);
}

PUBLIC VIRTUAL AosHandleSipController::~AosHandleSipController()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosHandleSipController = %" PFLS_u "/%" PFLS_x,
            m_strAppId.GetStr(), sizeof(AosHandleSipController), this);
}

PROTECTED VIRTUAL void AosHandleSipController::InitializeServiceBlock()
{
    if (!GET_N_CONFIG(m_nSlotId)->IsImsSingleRegistrationRequired())
    {
        m_bBlocked = IMS_TRUE;
    }

    A_IMS_TRACE_I(APPPROFILE, "InitializeServiceBlock :: block(%s)", _TRACE_B_(m_bBlocked), 0, 0);
}