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
#include "IAosService.h"
#include "interface/IAosAppContext.h"
#include "interface/IAosConditionListener.h"
#include "interface/IAosNetTracker.h"
#include "interface/IAosSubscriber.h"
#include "provider/AosProvider.h"
#include "condition/AosBlock.h"
#include "condition/AosECondition.h"

__IMS_TRACE_TAG_AOS__;

#define APPPROFILE m_strTag.GetStr()

PUBLIC
AosECondition::AosECondition(IN IAosAppContext* piAppContext) :
        AosCondition(piAppContext)
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_M : [%s] AosECondition = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosECondition), this);
    RemoveListener(LISTENER_CALLTRACKER);
}

PUBLIC VIRTUAL AosECondition::~AosECondition()
{
    IMS_TRACE_MEM("AOS_MEM", "AOS_F : [%s] AosECondition = %" PFLS_u "/%" PFLS_x, APPPROFILE,
            sizeof(AosECondition), this);
}

PUBLIC VIRTUAL IMS_BOOL AosECondition::IsReady()
{
    IMS_BOOL bReturn = m_piBlock->IsCleared();
    A_IMS_TRACE_I(APPPROFILE, "IsReady(%s)", _TRACE_B_(bReturn), 0, 0);

    return bReturn;
}

PROTECTED VIRTUAL void AosECondition::AddAosServiceListener()
{
    IAosService* pService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (pService != IMS_NULL)
    {
        pService->AddListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
    }
}

PROTECTED VIRTUAL void AosECondition::RemoveAosServiceListener()
{
    IAosService* pService = AosProvider::GetInstance()->GetService(m_nSlotId);
    if (pService != IMS_NULL)
    {
        pService->RemoveListener(DYNAMIC_CAST(IAosServicePhoneListener*, this));
    }
}

// IAosBlockListener
PROTECTED VIRTUAL void AosECondition::Block_Changed(IN IMS_UINT32 nType, IN IMS_UINT32 nParam)
{
    A_IMS_TRACE_I(APPPROFILE, "Block_Changed :: Reason(%s)(%d) - %s",
            AosBlock::BlockReasonToString(nType), nType, (nParam > 0) ? "BLOCK" : "NOT_BLOCK");

    PrintBlockReasons();

    m_piListener->Condition_Changed();
}

// AosServicePhoneListener
PROTECTED VIRTUAL void AosECondition::ServicePhone_AosStart()
{
    A_IMS_TRACE_D(APPPROFILE, "ServicePhone_AosStart()", 0, 0, 0);
    m_piBlock->ResetBlockReason(BLOCK_AOS_INCOMPLETED);
}