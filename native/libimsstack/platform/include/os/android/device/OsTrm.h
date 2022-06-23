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
#ifndef OS_TRM_H_
#define OS_TRM_H_

#include "IIpcan.h"
#include "ITrm.h"
#include "ImsMap.h"
#include "OsTrmTimer.h"
#include "system-intf/ISystemListener.h"

class IMutex;
class System;

class TrmInfo
{
public:
    TrmInfo(IN IMS_UINT32 nSlotId, IN ITrmTimerListener* piTimerListener) :
            m_nSlotId(nSlotId),
            m_nEmergencyServices(ITrm::SERVICE_NONE),
            m_nUpdatedEmergencyService(ITrm::SERVICE_NONE),
            m_nServices(ITrm::SERVICE_NONE),
            m_nUpdatedService(ITrm::SERVICE_NONE),
            m_nIpcanCategory(IIpcan::CATEGORY_MOBILE),
            m_bStarted(IMS_FALSE),
            m_objTimers(IMSMap<IMS_UINT32, ITrmTimer*>())
    {
        ITrmTimer* piTimer = new OsTrmTimer(nSlotId, ITrm::SERVICE_REG, TIMER_REG_DURATION);
        piTimer->SetListener(piTimerListener);
        m_objTimers.Add(ITrm::SERVICE_REG, piTimer);

        piTimer = new OsTrmTimer(nSlotId, ITrm::SERVICE_SMS, TIMER_SMS_DURATION);
        piTimer->SetListener(piTimerListener);
        m_objTimers.Add(ITrm::SERVICE_SMS, piTimer);

        piTimer = new OsTrmTimer(nSlotId, ITrm::SERVICE_UT, TIMER_UT_DURATION);
        piTimer->SetListener(piTimerListener);
        m_objTimers.Add(ITrm::SERVICE_UT, piTimer);
    }

    virtual ~TrmInfo()
    {
        for (IMS_UINT32 i = 0; i < m_objTimers.GetSize(); ++i)
        {
            ITrmTimer* piTimer = m_objTimers.GetValueAt(i);
            if (piTimer != IMS_NULL)
            {
                piTimer->SetListener(IMS_NULL);
                delete DYNAMIC_CAST(OsTrmTimer*, piTimer);
            }
        }

        m_objTimers.Clear();
    }

    inline void ClearEmergency()
    {
        m_nEmergencyServices = ITrm::SERVICE_NONE;
        m_nUpdatedEmergencyService = ITrm::SERVICE_NONE;
    }

    inline void ClearServices() { m_nServices = ITrm::SERVICE_NONE; }

    inline ITrmTimer* GetTimer(IN IMS_UINT32 nType)
    {
        IMS_SLONG nIndex = m_objTimers.GetIndexOfKey(nType);

        if (nIndex < 0)
        {
            return IMS_NULL;
        }

        return m_objTimers.GetValueAt(nIndex);
    }

    inline void StopAllTimers()
    {
        for (IMS_UINT32 i = 0; i < m_objTimers.GetSize(); i++)
        {
            ITrmTimer* piTimer = m_objTimers.GetValueAt(i);
            if (piTimer != IMS_NULL)
            {
                piTimer->Stop();
            }
        }
    }

    inline void StartTimerAndStopRestTimers(IN IMS_UINT32 nType)
    {
        StopAllTimers();

        IMS_SLONG nIndex = m_objTimers.GetIndexOfKey(nType);

        if (nIndex < 0)
        {
            return;
        }

        ITrmTimer* piTimer = m_objTimers.GetValueAt(nIndex);
        if (piTimer != IMS_NULL)
        {
            piTimer->Start();
        }
    }

    inline void Enable(IN IMS_BOOL bIsEnabled) { m_bStarted = bIsEnabled; }

    inline IMS_BOOL IsEnabled() const { return m_bStarted; }

    inline IMS_BOOL IsStarted(IN IMS_UINT32 nService) const { return (m_nServices & nService); }

    inline IMS_BOOL IsEStarted(IN IMS_UINT32 nService) const
    {
        return (m_nEmergencyServices & nService);
    }

    inline IMS_BOOL IsWlan() const { return (m_nIpcanCategory == IIpcan::CATEGORY_WLAN); }

    inline void SetIpcan(IN IMS_UINT32 nCategory) { m_nIpcanCategory = nCategory; }

    inline void SetUpdatedEmergencyService(IN IMS_UINT32 nService)
    {
        m_nUpdatedEmergencyService = nService;
    }

    inline void SetUpdatedService(IN IMS_UINT32 nService) { m_nUpdatedService = nService; }

    inline void Start(IN IMS_UINT32 nService) { m_nServices |= nService; }

    inline void StartEmergency(IN IMS_UINT32 nService) { m_nEmergencyServices |= nService; }

    inline void Stop(IN IMS_UINT32 nService) { m_nServices &= (~nService); }

    inline void StopEmergency(IN IMS_UINT32 nService) { m_nEmergencyServices &= (~nService); }

public:
    IMS_UINT32 m_nSlotId;
    IMS_UINT32 m_nEmergencyServices;
    IMS_UINT32 m_nUpdatedEmergencyService;
    IMS_UINT32 m_nServices;
    IMS_UINT32 m_nUpdatedService;
    IMS_UINT32 m_nIpcanCategory;
    IMS_BOOL m_bStarted;

    IMSMap<IMS_UINT32, ITrmTimer*> m_objTimers;

    static const IMS_UINT32 TIMER_REG_DURATION = 10000;  // 10s
    static const IMS_UINT32 TIMER_SMS_DURATION = 18000;  // 10s
    static const IMS_UINT32 TIMER_UT_DURATION = 10000;   // 10s
};

class OsTrm : public ITrm, public ITrmTimerListener, public ISystemListener
{
public:
    OsTrm();
    virtual ~OsTrm();

    OsTrm(IN const OsTrm&) = delete;
    OsTrm& operator=(IN const OsTrm&) = delete;

public:
    // ITrm
    void Enable(IN IMS_UINT32 nSlotId) override;
    void Disable(IN IMS_UINT32 nSlotId) override;

    IMS_BOOL IsServiceAvailable(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType) override;
    IMS_BOOL IsTrmSupported() override;
    void SetEmergencyService(
            IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType, IN IMS_UINT32 nMode) override;
    void SetIpcan(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nCategory) override;
    IMS_BOOL SetService(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType, IN IMS_UINT32 nMode) override;

    // ITrmTimerListener
    void TrmTimer_TimerExpired(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType) override;

    // ISystemListener
    void System_NotifyEvent(
            IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) override;

private:
    IMS_UINT32 GetHighPriorityService(IN IMS_UINT32 nServices);
    IMS_UINT32 GetTopPrioritySlot();
    IMS_UINT32 GetUpdatedTopPrioritySlot();
    TrmInfo* GetTrmInfo(IN IMS_UINT32 nSlotId);
    IMS_UINT32 GetTrmType(IN IMS_UINT32 nType);
    IMS_BOOL IsEnabled(IN IMS_UINT32 nSlotId);
    IMS_BOOL IsEmergency();
    IMS_BOOL IsEmergency(IN IMS_UINT32 nSlotId);
    IMS_BOOL IsEmergencyInOtherSlot(IN IMS_UINT32 nSlotId);
    IMS_BOOL IsHighPriortyExistInOtherSlot(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType);
    IMS_BOOL IsIdle();
    IMS_BOOL IsNoTrmReport(IN TrmInfo* pTrm, IN IMS_UINT32 nType, IN IMS_UINT32 nMode);
    IMS_BOOL IsSameRequest(IN TrmInfo* pTrm, IN IMS_UINT32 nType, IN IMS_UINT32 nMode);
    IMS_BOOL IsSameERequest(IN TrmInfo* pTrm, IN IMS_UINT32 nType, IN IMS_UINT32 nMode);
    IMS_BOOL IsWlan(IN IMS_UINT32 nSlotId);
    IMS_BOOL IsWlanInOtherSlot(IN IMS_UINT32 nSlotId);

    IMS_BOOL ResetService(IN IMS_UINT32 nSlotId);
    void ResetServiceInOtherSlot(IN IMS_UINT32 nSlotId);
    void SetEmergencyService_Start(IN TrmInfo* pTrm, IN IMS_UINT32 nType, IN IMS_UINT32 nSlotId);
    void SetEmergencyService_Stop(IN TrmInfo* pTrm, IN IMS_UINT32 nType, IN IMS_UINT32 nSlotId);
    void SetIpcan_Mobile(IN TrmInfo* pTrm, IN IMS_UINT32 nSlotId);
    void SetIpcan_Wlan(IN IMS_UINT32 nSlotId);
    void SetService_Start(IN TrmInfo* pTrm, IN IMS_UINT32 nType, IN IMS_UINT32 nSlotId);
    void SetService_Stop(IN TrmInfo* pTrm, IN IMS_UINT32 nType, IN IMS_UINT32 nSlotId);
    void SetTrmInfo(IN TrmInfo* pTrm, IN IMS_UINT32 nType, IN IMS_UINT32 nSlotId,
            IN IMS_BOOL bIsTimerRequired = IMS_TRUE);
    IMS_BOOL UpdateHighPrioritySlotAndResetOtherSlot(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType);
    void UpdateServiceStateChanged(
            IN IMS_UINT32 nServiceType, IN IMS_UINT32 nMode, IN IMS_UINT32 nSlotId);

    enum
    {
        TRM_TYPE_NONE = 0,
        TRM_TYPE_VOLTE = 1,
        TRM_TYPE_SMS = 2,
        TRM_TYPE_MMS = 3,
        TRM_TYPE_REGISTRATION = 4,
        TRM_TYPE_UT = 5
    };

    static const IMS_CHAR* ServiceToString(IN IMS_SINT32 nType);

private:
    IMutex* m_piLock;
    IMSMap<IMS_UINT32, TrmInfo*> m_objTrms;
};

#endif
