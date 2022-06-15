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
#include "ServiceMutex.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"

#include "private/ConfigurationManager.h"
#include "private/SubscriberConfig.h"

#include "SipDebug.h"
#include "base/SubscriberTracker.h"

__IMS_TRACE_TAG_IMS__;

PRIVATE
SubscriberTracker::SubscriberTracker() :
        m_piLock(IMS_NULL),
        m_pSubscriberMaps(IMS_NULL)
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();
    m_pSubscriberMaps = new IMSMap<AString, IMSList<SipAddress*>>[SystemConfig::GetMaxSimSlot()];
}

PUBLIC VIRTUAL SubscriberTracker::~SubscriberTracker()
{
    if (m_pSubscriberMaps != IMS_NULL)
    {
        delete[] m_pSubscriberMaps;
    }

    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC
const AString& SubscriberTracker::GetSubscriberId(
        IN IMS_SINT32 nSlotId, IN const AString& strAor) const
{
    //// Supports a multiple subscriber configurations
    SipAddress objAor;

    if (!objAor.Create(strAor))
    {
        return AString::ConstNull();
    }

    return GetSubscriberId(nSlotId, &objAor);
}

PUBLIC
const AString& SubscriberTracker::GetSubscriberId(
        IN IMS_SINT32 nSlotId, IN const SipAddress* pAor) const
{
    if (pAor == IMS_NULL)
    {
        return AString::ConstNull();
    }

    LockGuard objLock(m_piLock);

    IMSMap<AString, IMSList<SipAddress*>>* pSubscribers = GetSubscribers(nSlotId);

    if (pSubscribers == IMS_NULL)
    {
        return AString::ConstNull();
    }

    //// If there is no subscribers, it will return the default subscriber id.
    if (pSubscribers->IsEmpty())
    {
        return SubscriberConfig::GetDefaultId();
    }

    // Supports a multiple subscriber configurations
    // Find the matched AOR & IMPU
    for (IMS_UINT32 i = 0; i < pSubscribers->GetSize(); ++i)
    {
        const AString& strId = pSubscribers->GetKeyAt(i);
        const IMSList<SipAddress*>& objImpus = pSubscribers->GetValueAt(i);

        for (IMS_UINT32 j = 0; j < objImpus.GetSize(); ++j)
        {
            const SipAddress* pImpu = objImpus.GetAt(j);

            if (pImpu->Equals(*pAor))
            {
                return strId;
            }
        }
    }

    return AString::ConstNull();
}

PUBLIC GLOBAL SubscriberTracker* SubscriberTracker::GetInstance()
{
    static SubscriberTracker* s_pSubsTracker = IMS_NULL;

    if (s_pSubsTracker == IMS_NULL)
    {
        s_pSubsTracker = new SubscriberTracker();
    }

    return s_pSubsTracker;
}

PROTECTED VIRTUAL void SubscriberTracker::SubscriberInfo_UpdateIMPU(IN IMS_SINT32 nSlotId,
        IN const AString& strId, IN const AString& strOld, IN const AString& strNew)
{
    IMS_TRACE_I("Subscriber :: ID (%s), OLD (%s), NEW (%s)", strId.GetStr(),
            SipDebug::GetUri1(strOld).GetStr(), SipDebug::GetUri2(strNew).GetStr());

    LockGuard objLock(m_piLock);

    IMSMap<AString, IMSList<SipAddress*>>* pSubscribers = GetSubscribers(nSlotId);

    if (pSubscribers == IMS_NULL)
    {
        return;
    }

    IMS_SLONG nIndex = pSubscribers->GetIndexOfKey(strId);

    if (nIndex < 0)
    {
        if ((strOld.GetLength() == 0) && (strNew.GetLength() == 0))
        {
            // Ignore the issue...
            return;
        }

        // New subscriber configuration
        SipAddress* pImpu = new SipAddress();

        if (pImpu == IMS_NULL)
        {
            return;
        }

        if (!pImpu->Create(strNew))
        {
            delete pImpu;

            IMS_TRACE_E(0, "Creating SIP address (%s) failed", SipDebug::GetUri1(strNew).GetStr(),
                    0, 0);
            return;
        }

        IMSList<SipAddress*> objImpus;

        objImpus.Append(pImpu);

        pSubscribers->Add(strId, objImpus);

        return;
    }

    if ((strOld.GetLength() == 0) && (strNew.GetLength() == 0))
    {
        // Remove all the IMPUs
        IMSList<SipAddress*>& objImpus = pSubscribers->GetValueAt(nIndex);

        while (!objImpus.IsEmpty())
        {
            SipAddress* pImpu = objImpus.GetAt(0);

            if (pImpu != IMS_NULL)
            {
                delete pImpu;
            }

            objImpus.RemoveAt(0);
        }

        return;
    }

    // If OLD & NEW is same value, then ignore the below procedure
    if (strOld.Equals(strNew))
    {
        return;
    }

    IMSList<SipAddress*>& objImpus = pSubscribers->GetValueAt(nIndex);

    // OLD : empty, NEW : not empty
    // OLD : not empty, NEW : empty
    // OLD : not empty, NEW : not empty

    // First, check an old IMPU and remove it if present
    if ((strOld.GetLength() != 0) && !objImpus.IsEmpty())
    {
        SipAddress objOld(strOld);

        for (IMS_UINT32 i = 0; i < objImpus.GetSize(); ++i)
        {
            SipAddress* pImpu = objImpus.GetAt(i);

            if (pImpu == IMS_NULL)
            {
                continue;
            }

            if (pImpu->Equals(objOld))
            {
                // Remove and escape the loop
                delete pImpu;

                objImpus.RemoveAt(i);
                break;
            }
        }
    }

    // Adds a new IMPU
    if (strNew.GetLength() != 0)
    {
        SipAddress* pImpu = new SipAddress();

        if (pImpu == IMS_NULL)
        {
            return;
        }

        if (!pImpu->Create(strNew))
        {
            delete pImpu;

            IMS_TRACE_E(0, "Creating SIP address (%s) failed", SipDebug::GetUri1(strNew).GetStr(),
                    0, 0);
            return;
        }

        objImpus.Append(pImpu);
    }
}

PRIVATE
IMSMap<AString, IMSList<SipAddress*>>* SubscriberTracker::GetSubscribers(
        IN IMS_SINT32 nSlotId) const
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return IMS_NULL;
    }

    return &m_pSubscriberMaps[nSlotId];
}

PRIVATE
void SubscriberTracker::Initialize()
{
    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    for (IMS_SINT32 i = 0; i < nSimCount; i++)
    {
        InitForSlot(i);
    }
}

PRIVATE
void SubscriberTracker::InitForSlot(IN IMS_SINT32 nSlotId)
{
    const IMSList<SubscriberConfig*>& objSubsConfigs =
            ConfigurationManager::GetInstance()->GetSubscriberConfigs(nSlotId);

    if (objSubsConfigs.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objSubsConfigs.GetSize(); ++i)
    {
        SubscriberConfig* pSubsConfig = objSubsConfigs.GetAt(i);

        if (pSubsConfig == IMS_NULL)
        {
            continue;
        }

        // Set the subscriber info. change listener
        pSubsConfig->SetSubscriberInfoListener(this);

        if (!pSubsConfig->IsProvisioningDone())
        {
            // SubscriberConfig will notify of the information change
            continue;
        }

        IMS_SINT32 nSubsCount = pSubsConfig->GetSubscriberCount();

        // Add all the subscriber information
        for (IMS_SINT32 j = 0; j < nSubsCount; ++j)
        {
            const ImsSubscriberInfo* pSubsInfo = pSubsConfig->GetSubscriberInfoEx(j);

            if (pSubsInfo == IMS_NULL)
            {
                continue;
            }

            const AStringArray& objImpus = pSubsInfo->GetPublicUserIds();

            for (IMS_SINT32 k = 0; k < objImpus.GetCount(); ++k)
            {
                const AString& strImpu = objImpus.GetElementAt(k);

                if (strImpu.GetLength() == 0)
                {
                    continue;
                }

                SubscriberInfo_UpdateIMPU(
                        nSlotId, pSubsConfig->GetId(), AString::ConstNull(), strImpu);
            }
        }
    }
}
