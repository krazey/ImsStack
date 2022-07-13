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

#include "MtsServiceState.h"
#include "utility/MtsDynamicLoader.h"
#include "utility/MtsStrName.h"
#include "utility/MtsSipFormUtils.h"

__IMS_TRACE_TAG_COM_SMS__;

PUBLIC
MtsDynamicLoader::MtsDynamicLoader(IN IMS_SINT32 nSlotId) :
        m_nSlotId(nSlotId),
        m_pMtsServiceState(IMS_NULL),
        m_pMtsSipFormUtils(IMS_NULL),
        m_pMtsSmUtils(IMS_NULL),
        m_pMtsStrName(IMS_NULL)
{
    IMS_TRACE_D("+MtsDynamicLoader [slot_%d]", m_nSlotId, 0, 0);
}

PUBLIC
MtsDynamicLoader::~MtsDynamicLoader()
{
    DestroyAll();
}

PUBLIC
void MtsDynamicLoader::Initialize()
{
    IMS_TRACE_D("Initialize nSlotId : [%d]", m_nSlotId, 0, 0);

    m_pMtsServiceState = new MtsServiceState(m_nSlotId);
    m_pMtsSipFormUtils = new MtsSipFormUtils(m_nSlotId);
    m_pMtsSmUtils = new MtsSmUtils();
    m_pMtsStrName = new MtsStrName();
}

PRIVATE
void MtsDynamicLoader::DestroyAll()
{
    IMS_TRACE_I("DestroyAll", 0, 0, 0);

    if (m_pMtsServiceState != IMS_NULL)
    {
        delete m_pMtsServiceState;
        m_pMtsServiceState = IMS_NULL;
    }

    if (m_pMtsSipFormUtils != IMS_NULL)
    {
        delete m_pMtsSipFormUtils;
        m_pMtsSipFormUtils = IMS_NULL;
    }

    if (m_pMtsSmUtils != IMS_NULL)
    {
        delete m_pMtsSmUtils;
        m_pMtsSmUtils = IMS_NULL;
    }

    if (m_pMtsStrName != IMS_NULL)
    {
        delete m_pMtsStrName;
        m_pMtsStrName = IMS_NULL;
    }
}
