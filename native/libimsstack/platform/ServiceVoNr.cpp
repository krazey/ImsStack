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
#include "ImsMessageDef.h"
#include "ImsVoNr.h"
#include "PlatformFactory.h"
#include "ServiceVoNr.h"
#include "SystemConfig.h"

class VoNrHolder
{
public:
    inline VoNrHolder()
        : m_piVoNr(IMS_NULL)
    {}
    inline ~VoNrHolder()
    {
        PlatformFactory::DestroyVoNr(m_piVoNr);
    }

    VoNrHolder(IN const VoNrHolder&) = delete;
    VoNrHolder& operator=(IN const VoNrHolder&) = delete;

public:
    inline IVoNr* GetVoNr(IN IMS_SINT32 nSlotId)
    {
        if (m_piVoNr == IMS_NULL)
        {
            m_piVoNr = PlatformFactory::CreateVoNr(nSlotId);
        }

        return m_piVoNr;
    }

private:
    IVoNr* m_piVoNr;
};

class VoNrServicePrivate
{
public:
    VoNrServicePrivate();
    ~VoNrServicePrivate();

    VoNrServicePrivate(IN const VoNrServicePrivate&) = delete;
    VoNrServicePrivate& operator=(IN const VoNrServicePrivate&) = delete;

public:
    VoNrHolder* GetHolder(IN IMS_SINT32 nSlotId);

private:
    VoNrHolder** m_ppHolder;
};

PUBLIC
VoNrServicePrivate::VoNrServicePrivate()
    : m_ppHolder(IMS_NULL)
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    m_ppHolder = new VoNrHolder*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        m_ppHolder[i] = new VoNrHolder();
    }
}

PUBLIC
VoNrServicePrivate::~VoNrServicePrivate()
{
    if (m_ppHolder != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (m_ppHolder[i] != IMS_NULL)
            {
                delete m_ppHolder[i];
            }
        }

        delete[] m_ppHolder;
    }
}

PUBLIC
VoNrHolder* VoNrServicePrivate::GetHolder(IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        nSlotId = IMS_SLOT_0;
    }

    return m_ppHolder[nSlotId];
}

PRIVATE
VoNrService::VoNrService()
    : m_pPrivate(new VoNrServicePrivate())
{
}

PRIVATE
VoNrService::~VoNrService()
{
    if (m_pPrivate != IMS_NULL)
    {
        delete m_pPrivate;
    }
}

PUBLIC
IVoNr* VoNrService::GetVoNr(IN IMS_SINT32 nSlotId)
{
    VoNrHolder* pHolder = m_pPrivate->GetHolder(nSlotId);
    return pHolder->GetVoNr(nSlotId);
}

PUBLIC
void VoNrService::DispatchServiceMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case IMS_MSG_VONR: {
            IMS_SINT32 nSlotId = LONG_TO_SINT(objMsg.nWparam);
            ImsVoNr* pVoNr = DYNAMIC_CAST(ImsVoNr*, GetVoNr(nSlotId));

            if (pVoNr != IMS_NULL)
            {
                pVoNr->DispatchServiceMessage(objMsg.nWparam, objMsg.nLparam);
            }
            break;
        }

        default:
            // no-op
            break;
    }
}

PUBLIC GLOBAL
VoNrService* VoNrService::GetVoNrService()
{
    static VoNrService* s_pVoNrService = IMS_NULL;

    if (s_pVoNrService == IMS_NULL)
    {
        s_pVoNrService = new VoNrService();
    }

    return s_pVoNrService;
}
