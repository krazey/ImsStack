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
#include "ServiceMemory.h"
#include "ServiceTrace.h"

#include "SipControllerManager.h"
#include "SipControllerFactory.h"

__IMS_TRACE_TAG_USER_DECL__("IMS_SNC");

PRIVATE GLOBAL SipControllerFactory* SipControllerFactory::m_gpFactory = IMS_NULL;
PRIVATE GLOBAL ImsMap<IMS_SINT32, SipControllerManager*> SipControllerFactory::m_objManagers =
        ImsMap<IMS_SINT32, SipControllerManager*>();

PUBLIC SipControllerFactory::SipControllerFactory()
{
    IMS_TRACE_D("SipControllerFactory", 0, 0, 0);
}

PUBLIC VIRTUAL SipControllerFactory::~SipControllerFactory()
{
    IMS_TRACE_I("~SipControllerFactory", 0, 0, 0);
    m_objManagers.Clear();
}

PUBLIC GLOBAL void SipControllerFactory::Start(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    IMS_TRACE_D("Start :: slot(%d)", nSlotId, 0, 0);
    if (m_gpFactory == IMS_NULL)
    {
        m_gpFactory = new SipControllerFactory();
    }
    if (GetSipControllerManager(nSlotId) != IMS_NULL)
    {
        return;
    }

    AString strName = AString::ConstEmpty();
    strName.Sprintf("SipControllerManager%02d", nSlotId);

    SipControllerManager* pSipControllerManager = new SipControllerManager(nSlotId, strName);
    m_objManagers.Add(nSlotId, pSipControllerManager);
}

PUBLIC GLOBAL void SipControllerFactory::Stop(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    IMS_TRACE_D("Stop :: slot(%d)", nSlotId, 0, 0);
    if (m_gpFactory == IMS_NULL)
    {
        m_gpFactory = new SipControllerFactory();
    }
    SipControllerManager* pSipControllerManager = GetSipControllerManager(nSlotId);
    if (pSipControllerManager != IMS_NULL)
    {
        delete pSipControllerManager;
        m_objManagers.Remove(nSlotId);
    }
}

PUBLIC GLOBAL SipControllerManager* SipControllerFactory::GetSipControllerManager(
        IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    IMS_SLONG nIndex = m_objManagers.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objManagers.GetValueAt(nIndex);
}
