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

#include "IMtcContext.h"
#include "ImsTypeDef.h"
#include "MtcContextRepository.h"
#include "ServiceThread.h"

MtcContextRepository* MtcContextRepository::s_pThis = IMS_NULL;

PUBLIC
MtcContextRepository::MtcContextRepository() :
        m_objContexts(IMSMap<IMS_SINT32, IMtcContext*>())
{
}

PUBLIC
MtcContextRepository::~MtcContextRepository() {}

PUBLIC GLOBAL MtcContextRepository* MtcContextRepository::GetInstance()
{
    if (s_pThis == IMS_NULL)
    {
        s_pThis = new MtcContextRepository();
    }
    return s_pThis;
}

PUBLIC GLOBAL IMtcContext* MtcContextRepository::GetContext(
        IN IMS_SINT32 nSlotId /* = INVALID_SLOT_ID*/)
{
    if (nSlotId == INVALID_SLOT_ID)
    {
        nSlotId = ThreadService::GetCurrentSlotId(IMS_SLOT_0);
    }

    return MtcContextRepository::GetInstance()->GetContextBySlot(nSlotId);
}

PUBLIC
IMtcContext* MtcContextRepository::GetContextBySlot(IN IMS_SINT32 nSlotId)
{
    IMS_SLONG nIndex = m_objContexts.GetIndexOfKey(nSlotId);
    if (nIndex < 0)
    {
        return IMS_NULL;
    }
    return m_objContexts.GetValueAt(nIndex);
}

PUBLIC
void MtcContextRepository::AddContext(IN IMS_SINT32 nSlotId, IN IMtcContext* piContext)
{
    m_objContexts.Add(nSlotId, piContext);
}

PUBLIC
void MtcContextRepository::RemoveContext(IN IMS_SINT32 nSlotId)
{
    m_objContexts.Remove(nSlotId);
}
