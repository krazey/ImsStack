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

#include "IMtsContext.h"
#include "ServiceTrace.h"
#include "utility/MtsDynamicLoader.h"

__IMS_TRACE_TAG_COM_MTS__;

PUBLIC
MtsDynamicLoader::MtsDynamicLoader(IN IMtsContext& objContext) :
        m_objContext(objContext),
        m_pMtsSipFormUtils(IMS_NULL),
        m_pMtsSmUtils(IMS_NULL),
        m_pMtsGeolocationUtils(IMS_NULL),
        m_pMtsAosUtils(IMS_NULL)
{
    IMS_TRACE_I("+MtsDynamicLoader [slot_%d]", m_objContext.GetSlotId(), 0, 0);
    Initialize();
}

PUBLIC
MtsDynamicLoader::~MtsDynamicLoader()
{
    DestroyAll();
}

PRIVATE
void MtsDynamicLoader::Initialize()
{
    IMS_TRACE_I("Initialize : nSlotId[%d]", m_objContext.GetSlotId(), 0, 0);

    m_pMtsSipFormUtils = new MtsSipFormUtils(m_objContext.GetSlotId());
    m_pMtsSmUtils = new MtsSmUtils();
    m_pMtsGeolocationUtils = new MtsGeolocationUtils();
    m_pMtsAosUtils = new MtsAosUtils();
}

PRIVATE
void MtsDynamicLoader::DestroyAll()
{
    IMS_TRACE_I("DestroyAll : nSlotId[%d]", m_objContext.GetSlotId(), 0, 0);

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

    if (m_pMtsGeolocationUtils != IMS_NULL)
    {
        delete m_pMtsGeolocationUtils;
        m_pMtsGeolocationUtils = IMS_NULL;
    }

    if (m_pMtsAosUtils != IMS_NULL)
    {
        delete m_pMtsAosUtils;
        m_pMtsAosUtils = IMS_NULL;
    }
}
