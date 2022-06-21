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

#include "IMSTypeDef.h"
#include "ServiceTrace.h"
#include "call/IMtcCallContext.h"
#include "helper/block/MtcBlockChecker.h"
#include "define/MtcStringDef.h"

__IMS_TRACE_TAG_COM_MTC__;

PUBLIC
MtcBlockChecker::MtcBlockChecker(
        IN const IMSList<IMtcBlockRule*>& lstRules, IN IMtcBlockCheckListener* pListener) :
        m_pListener(pListener),
        m_lstRules(lstRules),
        m_nPendingCount(0)
{
}

PUBLIC
MtcBlockChecker::~MtcBlockChecker()
{
    for (IMS_UINT32 nIndex = 0; nIndex < m_lstRules.GetSize(); nIndex++)
    {
        IMtcBlockRule* pRule = m_lstRules.GetAt(nIndex);
        delete pRule;
    }
}

PUBLIC
MtcBlockChecker::Result MtcBlockChecker::Check()
{
    m_nPendingCount = 0;

    for (IMS_UINT32 nIndex = 0; nIndex < m_lstRules.GetSize(); nIndex++)
    {
        IMtcBlockRule* pRule = m_lstRules.GetAt(nIndex);

        Result objResult = pRule->Check(*this);

        if (objResult.eStatus == Result::Status::BLOCKED)
        {
            IMS_TRACE_I("Check : blocked by [%s]", PS_FR(objResult.objReason), 0, 0);
            return objResult;
        }

        if (objResult.eStatus == Result::Status::PENDING)
        {
            m_nPendingCount++;
        }
    }

    if (m_nPendingCount > 0)
    {
        IMS_TRACE_I("Check : [%d] pending remains", m_nPendingCount, 0, 0);
        return Result(Result::Status::PENDING);
    }
    else
    {
        return Result(Result::Status::UNBLOCKED);
    }
}

PUBLIC
void MtcBlockChecker::OnBlockRuleChecked(IN IMtcBlockChecker::Result objResult)
{
    if (IsResultNotified())
    {
        return;
    }

    switch (objResult.eStatus)
    {
        case Result::Status::UNBLOCKED:
            m_nPendingCount--;
            IMS_TRACE_I("OnBlockChecked : [%d] pending remains", m_nPendingCount, 0, 0);
            break;

        case Result::Status::BLOCKED:
            IMS_TRACE_I("OnBlockChecked : blocked by [%s]", PS_FR(objResult.objReason), 0, 0);
            m_nPendingCount = 0;
            break;

        case Result::Status::PENDING:
            IMS_ASSERT(objResult.eStatus != Result::Status::PENDING);
            return;
    }

    if (m_nPendingCount <= 0)
    {
        if (m_pListener != IMS_NULL)
        {
            m_pListener->OnBlockChecked(objResult);
        }
        else
        {
            IMS_TRACE_D("OnBlockRuleChecked : No listener", 0, 0, 0);
        }
    }
}

PRIVATE
IMS_BOOL MtcBlockChecker::IsResultNotified()
{
    return m_nPendingCount <= 0;
}
