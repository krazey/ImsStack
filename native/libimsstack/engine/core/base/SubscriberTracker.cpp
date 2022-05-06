/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20110517  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMutex.h"
#include "SystemConfig.h"
#include "private/ConfigurationManager.h"
#include "private/SubscriberConfig.h"
#include "SipDebug.h"
#include "base/SubscriberTracker.h"

__IMS_TRACE_TAG_IMS__;

PRIVATE
SubscriberTracker::SubscriberTracker() :
        piLock(IMS_NULL),
        pSubscriberMaps(IMS_NULL)
{
    piLock = MutexService::GetMutexService()->CreateMutex();
    pSubscriberMaps = new IMSMap<AString, IMSList<SipAddress*>>[SystemConfig::GetMaxSimSlot()];
}

PUBLIC VIRTUAL SubscriberTracker::~SubscriberTracker()
{
    if (pSubscriberMaps != IMS_NULL)
    {
        delete[] pSubscriberMaps;
    }

    MutexService::GetMutexService()->DestroyMutex(piLock);
}

/*

Remarks

*/
PUBLIC
const AString& SubscriberTracker::GetSubscriberId(
        IN IMS_SINT32 nSlotId, IN const AString& strAOR) const
{
    //// Supports a multiple subscriber configurations
    SipAddress objAOR;

    if (!objAOR.Create(strAOR))
    {
        return AString::ConstNull();
    }

    return GetSubscriberId(nSlotId, &objAOR);
}

/*

Remarks

*/
PUBLIC
const AString& SubscriberTracker::GetSubscriberId(
        IN IMS_SINT32 nSlotId, IN const SipAddress* pAOR) const
{
    if (pAOR == IMS_NULL)
    {
        return AString::ConstNull();
    }

    LockGuard objLock(piLock);

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
        const IMSList<SipAddress*>& objIMPUs = pSubscribers->GetValueAt(i);

        for (IMS_UINT32 j = 0; j < objIMPUs.GetSize(); ++j)
        {
            const SipAddress* pIMPU = objIMPUs.GetAt(j);

            if (pIMPU->Equals(*pAOR))
            {
                return strId;
            }
        }
    }

    return AString::ConstNull();
}

/*

Remarks

*/
PUBLIC GLOBAL SubscriberTracker* SubscriberTracker::GetInstance()
{
    static SubscriberTracker* pSubsTracker = IMS_NULL;

    if (pSubsTracker == IMS_NULL)
    {
        pSubsTracker = new SubscriberTracker();
    }

    return pSubsTracker;
}

/*

Remarks

*/
PROTECTED VIRTUAL void SubscriberTracker::SubscriberInfo_UpdateIMPU(IN IMS_SINT32 nSlotId,
        IN const AString& strId, IN const AString& strOLD, IN const AString& strNEW)
{
    IMS_TRACE_I("Subscriber :: ID (%s), OLD (%s), NEW (%s)", strId.GetStr(),
            SipDebug::GetUri1(strOLD).GetStr(), SipDebug::GetUri2(strNEW).GetStr());

    LockGuard objLock(piLock);

    IMSMap<AString, IMSList<SipAddress*>>* pSubscribers = GetSubscribers(nSlotId);

    if (pSubscribers == IMS_NULL)
    {
        return;
    }

    IMS_SLONG nIndex = pSubscribers->GetIndexOfKey(strId);

    if (nIndex < 0)
    {
        if ((strOLD.GetLength() == 0) && (strNEW.GetLength() == 0))
        {
            // Ignore the issue...
            return;
        }

        // New subscriber configuration
        SipAddress* pIMPU = new SipAddress();

        if (pIMPU == IMS_NULL)
        {
            return;
        }

        if (!pIMPU->Create(strNEW))
        {
            delete pIMPU;

            IMS_TRACE_E(0, "Creating SIP address (%s) failed", SipDebug::GetUri1(strNEW).GetStr(),
                    0, 0);
            return;
        }

        IMSList<SipAddress*> objIMPUs;

        objIMPUs.Append(pIMPU);

        pSubscribers->Add(strId, objIMPUs);

        return;
    }

    if ((strOLD.GetLength() == 0) && (strNEW.GetLength() == 0))
    {
        // Remove all the IMPUs
        IMSList<SipAddress*>& objIMPUs = pSubscribers->GetValueAt(nIndex);

        while (!objIMPUs.IsEmpty())
        {
            SipAddress* pIMPU = objIMPUs.GetAt(0);

            if (pIMPU != IMS_NULL)
            {
                delete pIMPU;
            }

            objIMPUs.RemoveAt(0);
        }

        return;
    }

    // If OLD & NEW is same value, then ignore the below procedure
    if (strOLD.Equals(strNEW))
    {
        return;
    }

    IMSList<SipAddress*>& objIMPUs = pSubscribers->GetValueAt(nIndex);

    // OLD : empty, NEW : not empty
    // OLD : not empty, NEW : empty
    // OLD : not empty, NEW : not empty

    // First, check an old IMPU and remove it if present
    if ((strOLD.GetLength() != 0) && !objIMPUs.IsEmpty())
    {
        SipAddress objOLD(strOLD);

        for (IMS_UINT32 i = 0; i < objIMPUs.GetSize(); ++i)
        {
            SipAddress* pIMPU = objIMPUs.GetAt(i);

            if (pIMPU == IMS_NULL)
            {
                continue;
            }

            if (pIMPU->Equals(objOLD))
            {
                // Remove and escape the loop
                delete pIMPU;

                objIMPUs.RemoveAt(i);
                break;
            }
        }
    }

    // Adds a new IMPU
    if (strNEW.GetLength() != 0)
    {
        SipAddress* pIMPU = new SipAddress();

        if (pIMPU == IMS_NULL)
        {
            return;
        }

        if (!pIMPU->Create(strNEW))
        {
            delete pIMPU;

            IMS_TRACE_E(0, "Creating SIP address (%s) failed", SipDebug::GetUri1(strNEW).GetStr(),
                    0, 0);
            return;
        }

        objIMPUs.Append(pIMPU);
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

    return &pSubscriberMaps[nSlotId];
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

            const AStringArray& objIMPUs = pSubsInfo->GetPublicUserIds();

            for (IMS_SINT32 k = 0; k < objIMPUs.GetCount(); ++k)
            {
                const AString& strIMPU = objIMPUs.GetElementAt(k);

                if (strIMPU.GetLength() == 0)
                {
                    continue;
                }

                SubscriberInfo_UpdateIMPU(
                        nSlotId, pSubsConfig->GetId(), AString::ConstNull(), strIMPU);
            }
        }
    }
}
