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

#include "UceApp.h"
#include "UceFactory.h"

__IMS_TRACE_TAG_USER_DECL__("Uce");

PRIVATE GLOBAL UceFactory* UceFactory::m_gpUceFactory = IMS_NULL;

PRIVATE GLOBAL ImsMap<IMS_SINT32, UceApp*> UceFactory::m_objUceManagers =
        ImsMap<IMS_SINT32, UceApp*>();

PUBLIC
UceFactory::UceFactory()
{
    IMS_TRACE_D("UceFactory", 0, 0, 0);
}

PUBLIC VIRTUAL UceFactory::~UceFactory() {}

PUBLIC GLOBAL void UceFactory::Start(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    if (m_gpUceFactory == IMS_NULL)
    {
        m_gpUceFactory = new UceFactory();
    }

    IMS_TRACE_D("Start :: slot(%d)", nSlotId, 0, 0);

    if (GetUceApp(nSlotId) != IMS_NULL)
    {
        return;
    }

    AString strName;
    strName.Sprintf("UceApp%02d", nSlotId);

    UceApp* pUceApp = new UceApp(nSlotId, strName);
    m_objUceManagers.Add(nSlotId, pUceApp);
}

PUBLIC GLOBAL void UceFactory::Stop(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    if (m_gpUceFactory == IMS_NULL)
    {
        m_gpUceFactory = new UceFactory();
    }

    IMS_TRACE_D("Stop :: slot(%d)", nSlotId, 0, 0);

    UceApp* pUceApp = GetUceApp(nSlotId);
    if (pUceApp != IMS_NULL)
    {
        delete pUceApp;
        m_objUceManagers.Remove(nSlotId);
    }
}

PUBLIC GLOBAL UceApp* UceFactory::GetUceApp(IN IMS_SINT32 nSlotId /* = IMS_SLOT_0 */)
{
    IMS_SLONG nIndex = m_objUceManagers.GetIndexOfKey(nSlotId);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objUceManagers.GetValueAt(nIndex);
}
