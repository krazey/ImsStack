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

#include "MtsApp.h"
#include "IMtsApp.h"
#include "MtsFactory.h"
#include "utility/MtsUtils.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsFactory::MtsFactory()
{
    IMS_TRACE_D("MtsFactory", 0, 0, 0);
    m_objMtsApp = IMSMap<IMS_SINT32, IMtsApp*>();
}

PUBLIC
MtsFactory::~MtsFactory()
{
    IMS_TRACE_D("~MtsFactory", 0, 0, 0);

    IMS_UINT32 nMtsAppSize = m_objMtsApp.GetSize();
    for (IMS_UINT32 index = 0; index < nMtsAppSize; index++)
    {
        IMtsApp* pApp = m_objMtsApp.GetValueAt(index);
        if (pApp == IMS_NULL)
        {
            continue;
        }

        delete DYNAMIC_CAST(MtsApp*, pApp);
        pApp = IMS_NULL;
    }
    m_objMtsApp.Clear();
}

PUBLIC GLOBAL MtsFactory* MtsFactory::GetInstance()
{
    static MtsFactory* s_pFactory = IMS_NULL;

    if (s_pFactory == IMS_NULL)
    {
        s_pFactory = new MtsFactory();
    }

    return s_pFactory;
}

PUBLIC
void MtsFactory::Destroy(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("Destroy[%d]", nSlotId, 0, 0);

    DestroyMtsApp(nSlotId);
}

PUBLIC
void MtsFactory::StartMts(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("StartMts[%d]", nSlotId, 0, 0);

    IMtsApp* MtsApp = CreateMtsApp(nSlotId);
    if (MtsApp == IMS_NULL)
    {
        IMS_TRACE_E(0, "StartMts : Fail to create MtsApp", 0, 0, 0);
        return;
    }
    MtsApp->Start();
}

PUBLIC
void MtsFactory::StopMts(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("StopMts[%d]", nSlotId, 0, 0);

    DestroyMtsApp(nSlotId);
}

PUBLIC
IMS_BOOL MtsFactory::DestroyMtsApp(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("DestroyMtsApp[%d]", nSlotId, 0, 0);

    IMS_SLONG nIndex = m_objMtsApp.GetIndexOfKey(nSlotId);
    if (nIndex < 0)
    {
        IMS_TRACE_D("DestroyMtsApp : Not Found", 0, 0, 0);
        return IMS_FALSE;
    }

    IMtsApp* piMtsApp = m_objMtsApp.GetValueAt(nIndex);
    piMtsApp->Stop();
    MtsApp* pMtsApp = DYNAMIC_CAST(MtsApp*, piMtsApp);

    delete pMtsApp;
    pMtsApp = IMS_NULL;

    m_objMtsApp.RemoveAt(nIndex);
    return IMS_TRUE;
}

PUBLIC
MtsApp* MtsFactory::GetMtsApp(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("GetMtsApp[%d]", nSlotId, 0, 0);

    IMS_SLONG nIndex = m_objMtsApp.GetIndexOfKey(nSlotId);
    IMtsApp* pApp = IMS_NULL;
    if (nIndex >= 0)
    {
        pApp = m_objMtsApp.GetValueAt(nIndex);
        return DYNAMIC_CAST(MtsApp*, pApp);
    }

    return IMS_NULL;
}

PUBLIC
IMS_UINT32 MtsFactory::GetMtsAppListSize()
{
    IMS_UINT32 nSize = m_objMtsApp.GetSize();

    IMS_TRACE_D("GetMtsAppListSize: %d", nSize, 0, 0);

    return nSize;
}

PRIVATE
IMtsApp* MtsFactory::CreateMtsApp(IN IMS_SINT32 nSlotId)
{
    IMS_TRACE_D("CreateMtsApp[%d]", nSlotId, 0, 0);

    IMtsApp* piMtsApp = new MtsApp(nSlotId);
    m_objMtsApp.Add(nSlotId, piMtsApp);

    return piMtsApp;
}
