
/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef OS_IMS_TRAFFIC_H_
#define OS_IMS_TRAFFIC_H_

#include "ImsList.h"
#include "ImsMap.h"
#include "IImsTraffic.h"
#include "OsImsTrafficTimer.h"

#include "ServiceThread.h"

class IMutex;

class Traffic
{
public:
    Traffic(IN IMS_SINT32 nSlotId, IN IImsTrafficTimerListener* piTimerListener) :
            m_nSlotId(nSlotId),
            m_nTopPriorityTraffic(IImsTraffic::TRAFFIC_PRIORITY_NONE),
            m_nTraffics(IImsTraffic::TRAFFIC_PRIORITY_NONE),
            m_bEnabled(IMS_FALSE),
            m_bSimultaneousCallingSupported(IMS_FALSE),
            m_bWlan(IMS_FALSE),
            m_objTimer(ImsMap<IMS_UINT32, IImsTrafficTimer*>())
    {
        IImsTrafficTimer* piTimer = new OsImsTrafficTimer(
                nSlotId, IImsTraffic::TRAFFIC_PRIORITY_REGISTRATION, TIMER_REG_DURATION);
        piTimer->SetListener(piTimerListener);
        m_objTimer.Add(IImsTraffic::TRAFFIC_PRIORITY_REGISTRATION, piTimer);

        piTimer = new OsImsTrafficTimer(
                nSlotId, IImsTraffic::TRAFFIC_PRIORITY_SMS, TIMER_SMS_DURATION);
        piTimer->SetListener(piTimerListener);
        m_objTimer.Add(IImsTraffic::TRAFFIC_PRIORITY_SMS, piTimer);
    }

    virtual ~Traffic()
    {
        for (IMS_UINT32 i = 0; i < m_objTimer.GetSize(); i++)
        {
            IImsTrafficTimer* piTimer = m_objTimer.GetValueAt(i);
            if (piTimer != IMS_NULL)
            {
                piTimer->SetListener(IMS_NULL);
                delete DYNAMIC_CAST(OsImsTrafficTimer*, piTimer);
            }
        }

        m_objTimer.Clear();
    }

    inline void ClearTraffics() { m_nTraffics = IImsTraffic::TRAFFIC_PRIORITY_NONE; }

    inline IImsTrafficTimer* GetTimer(IN IMS_UINT32 nType) const
    {
        IMS_SLONG nIndex = m_objTimer.GetIndexOfKey(nType);

        if (nIndex < 0)
        {
            return IMS_NULL;
        }

        return m_objTimer.GetValueAt(nIndex);
    }

    inline void SetTimer(IN IMS_UINT32 nType, IN IMS_BOOL bStart)
    {
        IMS_SLONG nIndex = m_objTimer.GetIndexOfKey(nType);

        if (nIndex < 0)
        {
            return;
        }

        IImsTrafficTimer* piTimer = m_objTimer.GetValueAt(nIndex);
        if (piTimer != IMS_NULL)
        {
            if (bStart)
            {
                piTimer->Start();
            }
            else
            {
                piTimer->Stop();
            }
        }
    }

    inline void StopAllTimers()
    {
        for (IMS_UINT32 i = 0; i < m_objTimer.GetSize(); i++)
        {
            IImsTrafficTimer* piTimer = m_objTimer.GetValueAt(i);

            if (piTimer != IMS_NULL)
            {
                piTimer->Stop();
            }
        }
    }

    inline void Enable(IN IMS_BOOL bIsEnabled) { m_bEnabled = bIsEnabled; }

    inline IMS_BOOL IsEmergency() const
    {
        return (IsStarted(IImsTraffic::TRAFFIC_PRIORITY_EMERGENCY) ||
                       IsStarted(IImsTraffic::TRAFFIC_PRIORITY_EMERGENCY_SMS))
                ? IMS_TRUE
                : IMS_FALSE;
    }

    inline IMS_BOOL IsEnabled() const { return m_bEnabled; }

    inline IMS_BOOL IsStarted(IN IMS_UINT32 nType) const { return (m_nTraffics & nType); }

    inline IMS_BOOL IsSimultaneousCallingSupported() const
    {
        return m_bSimultaneousCallingSupported;
    }

    inline IMS_BOOL IsWlan() const { return m_bWlan; }

    inline void SetWlan(IN IMS_BOOL bEnabled) { m_bWlan = bEnabled; }

    inline void SetTopPriorityTraffic(IN IMS_UINT32 nType) { m_nTopPriorityTraffic = nType; }

    inline void SetSimultaneousCallingSupported(IN IMS_BOOL bSupported)
    {
        m_bSimultaneousCallingSupported = bSupported;
    }

    inline void Start(IN IMS_UINT32 nType) { m_nTraffics |= nType; }

    inline void Stop(IN IMS_UINT32 nType) { m_nTraffics &= (~nType); }

public:
    IMS_SINT32 m_nSlotId;
    IMS_UINT32 m_nTopPriorityTraffic;
    IMS_UINT32 m_nTraffics;

    IMS_BOOL m_bEnabled;
    IMS_BOOL m_bSimultaneousCallingSupported;
    IMS_BOOL m_bWlan;

    ImsMap<IMS_UINT32, IImsTrafficTimer*> m_objTimer;

    static const IMS_UINT32 TIMER_REG_DURATION = 10000;  // 10s
    static const IMS_UINT32 TIMER_SMS_DURATION = 18000;  // 18s
};

class OsImsTraffic : public IImsTraffic, public IImsTrafficTimerListener
{
public:
    explicit OsImsTraffic();
    virtual ~OsImsTraffic();

    OsImsTraffic(IN const OsImsTraffic&) = delete;
    OsImsTraffic& operator=(IN const OsImsTraffic&) = delete;

public:
    // IImsTaffic class
    void Disable(IN IMS_SINT32 nSlotId) override;
    IMS_BOOL IsAllowed(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nTrafficType) override;
    void Start(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nTrafficType) override;
    void Stop(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nTrafficType) override;
    void SetSimultaneousCallingSupported(IN IMS_SINT32 nSlotId, IN IMS_BOOL bSupported) override;
    void SetWlan(IN IMS_SINT32 nSlotId, IN IMS_BOOL bEnabled) override;
    void AddListener(IN IImsTrafficListener* piListener) override;
    void RemoveListener(IN IImsTrafficListener* piListener) override;

    // IImsTrafficTimerListener
    void ImsTrafficTimer_Expired(IN IMS_UINT32 nSlotId, IN IMS_UINT32 nType) override;

    void DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam) override;

private:
    IMS_SINT32 GetTopPrioritizedSlot();
    Traffic* GetTraffic(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL HasHighPriorityInOtherSlot(IN IMS_SINT32 nSlotId, IN IMS_UINT32 nType) const;
    IMS_BOOL IsEmergency(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL IsEmergencyInOtherSlot(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL IsEnabled(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL IsIdle() const;
    IMS_BOOL IsSimultaneousCallingSupported(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL IsWlan(IN IMS_SINT32 nSlotId) const;
    IMS_BOOL IsWlanInOtherSlot(IN IMS_SINT32 nSlotId) const;
    void PostMessage();
    void SetEnabled(IN IMS_SINT32 nSlotId);
    void SetTimer(IN IMS_SINT32 nType, IN Traffic* pTraffic, IN IMS_BOOL bStart);

    static IMS_UINT32 GetHighPriorityType(IN IMS_UINT32 nTraffics);
    static IMS_UINT32 GetPriorityType(IN IMS_UINT32 nTrafficType);
    static IMS_UINT32 GetTrafficType(IN IMS_UINT32 nPriorityType);
    static const IMS_CHAR* PriorityTypeToString(IN IMS_UINT32 nType);

    class TrafficListeners
    {
    public:
        inline TrafficListeners(IN IImsTrafficListener* piListener) :
                piOwnerThread(IMS_NULL),
                objListeners(ImsList<IImsTrafficListener*>())
        {
            piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();

            objListeners.Append(piListener);
        }

        inline ~TrafficListeners() { objListeners.Clear(); }

        inline IMS_BOOL operator==(IN IThread* piThread) { return piThread == piOwnerThread; }

    public:
        IThread* piOwnerThread;
        ImsList<IImsTrafficListener*> objListeners;
    };

private:
    IMutex* m_piLock;
    ImsMap<IMS_SINT32, Traffic*> m_objTraffics;
    ImsList<TrafficListeners*> m_objThreadListeners;
};

#endif
