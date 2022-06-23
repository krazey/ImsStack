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
#ifndef OS_VONR_H_
#define OS_VONR_H_

#include "ISystemConfigListener.h"
#include "ImsMap.h"
#include "ImsVoNr.h"
#include "ServiceTimer.h"
#include "system-intf/ISystemListener.h"

class IVoNrTimerListener
{
public:
    virtual void VoNrTimer_TimerExpired(IN IMS_UINT32 nType) = 0;
};

class OsVoNr :
        public ImsVoNr,
        public ISystemListener,
        public IVoNrTimerListener,
        public ISystemConfigListener
{
public:
    OsVoNr(IN IMS_SINT32 nSlotId);
    virtual ~OsVoNr();

    OsVoNr(IN const OsVoNr&) = delete;
    OsVoNr& operator=(IN const OsVoNr&) = delete;

public:
    // IVoNr
    IMS_BOOL IsVoNrSupported() const override;
    IMS_BOOL IsUacCheckRequired(IN IMS_UINT32 nType) override;
    IMS_BOOL IsUeCapabilityVoNrEnabled() const override;

    IMS_BOOL NotifyCallState(IN IMS_UINT32 nModule, IN IMS_UINT32 nType, IN IMS_UINT32 nState,
            IN IMS_UINT32 nSysMode, IN IMS_UINT32 nDirection) override;
    IMS_SINT32 RequestCallPreference(IN IMS_UINT32 nRAT, IN IMS_UINT32 nType) override;
    IMS_BOOL SetImsSession(IN IMS_UINT32 nType, IN IMS_UINT32 nState) override;
    IMS_BOOL SetImsSignaling(IN IMS_UINT32 nType) override;
    IMS_BOOL SetImsVoice(IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode) override;
    IMS_BOOL SetUacCheck(IN IMS_UINT32 nType, IN IMS_UINT32 nState) override;
    IMS_BOOL SetVoice(IN IMS_UINT32 nState, IN IMS_BOOL bIsEmergency) override;

    void AddListenerForUac(IN IVoNrUacListener* piListener) override;
    void RemoveListenerForUac(IN IVoNrUacListener* piListener) override;

    void AddListenerForCallPreference(IN IVoNrCallPreferenceListener* piListener) override;
    void RemoveListenerForCallPreference(IN IVoNrCallPreferenceListener* piListener) override;

    void AddListenerForHandoff(IN IVoNrHandoffListener* piListener) override;
    void RemoveListenerForHandoff(IN IVoNrHandoffListener* piListener) override;

protected:
    // ImsVoNr
    void DispatchServiceMessage(IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) override;

    // ISystemListener
    void System_NotifyEvent(
            IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam) override;

    // IVoNrTimerListener
    void VoNrTimer_TimerExpired(IN IMS_UINT32 nType) override;

    // ISystemConfigListener class
    void SystemConfig_ConfigurationChanged(
            IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId = IMS_SLOT_ANY) override;

private:
    void Initialize();
    void CleanUp();

    IMS_UINT32 GetHandoffStatus(IN IMS_UINT32 nStatus) const;
    IMS_UINT32 GetImsCallType(IN IMS_UINT32 nType) const;
    IMS_UINT32 GetMtkRatForCallPreference(IN IMS_UINT32 nRAT) const;
    IMS_UINT32 GetNasStatusForCallState(IN IMS_UINT32 nState) const;
    IMS_UINT32 GetNasRatForCallPreference(IN IMS_UINT32 nRAT) const;
    IMS_UINT32 GetRatForCallPreference(IN IMS_UINT32 nRAT) const;
    IMS_UINT32 GetRatFromBearer(IN IMS_UINT32 nRAT) const;
    IMS_SINT32 GetReasonForUac(IN IMS_UINT32 nResponse, OUT IMS_RESULT& nResult);
    IMS_UINT32 GetSysMode(IN IMS_SINT32 nSysMode) const;
    IMS_UINT32 GetSysModeForMtk(IN IMS_UINT32 nSysMode) const;
    IMS_UINT32 GetSysModeForNas(IN IMS_UINT32 nSysMode) const;
    IMS_UINT32 GetTypeForMtk(IN IMS_UINT32 nType) const;
    IMS_UINT32 GetWdsLteCallType(IN IMS_UINT32 nType) const;

    IMS_BOOL IsMtkChipset() const;
    IMS_BOOL IsNrRat();
    IMS_BOOL IsStatusValidForHandoff(IN IMS_UINT32 nStatus) const;
    IMS_BOOL IsSysModeValidForCallState(IN IMS_UINT32 nSysMode) const;
    IMS_BOOL IsRatValidForCallPreference(IN IMS_UINT32 nRat) const;
    IMS_BOOL IsTypeValidForCallPreference(IN IMS_UINT32 nType) const;
    IMS_BOOL IsCallTypeForUac(IN IMS_UINT32 nType) const;

    void NotifyCallState_Emergency(IN IMS_UINT32 nModule, IN IMS_UINT32 nType, IN IMS_UINT32 nState,
            IN IMS_UINT32 nSysMode);
    void NotifyCallState_Mo(IN IMS_UINT32 nModule, IN IMS_UINT32 nType, IN IMS_UINT32 nState,
            IN IMS_UINT32 nSysMode);
    void NotifyCallState_Mt(IN IMS_UINT32 nModule, IN IMS_UINT32 nType, IN IMS_UINT32 nState,
            IN IMS_UINT32 nSysMode);

    void NotifyCallReady(IN IMS_UINT32 nSysMode);
    void NotifyHandoffInformation(IN IMS_UINT32 nInformation, IN IMS_UINT32 nSourceRat,
            IN IMS_UINT32 nTargetRat, IN IMS_UINT32 nReasonType, IN IMS_SINT32 nReason);
    void NotifyUacResponse(IN IMS_UINT32 nCallType, IN IMS_UINT32 nResult, IN IMS_SINT32 nSysMode,
            IN IMS_UINT32 nBarringTime);

    void SetVoNrSupported();

private:
    class CallState
    {
    public:
        inline CallState() :
                m_nState(STATE_IDLE),
                m_nSysMode(SYS_MODE_UNKNOWN)
        {
        }
        inline virtual ~CallState() {}

    public:
        void Clear();

        inline IMS_UINT32 GetState() const { return m_nState; }
        inline IMS_UINT32 GetSysMode() const { return m_nSysMode; }
        inline IMS_BOOL IsIdle(IN IMS_UINT32 nState) const { return nState == STATE_IDLE; }

        IMS_BOOL IsNotificationRequired(IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode) const;
        void Set(IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode);

    private:
        IMS_UINT32 m_nState;
        IMS_UINT32 m_nSysMode;
    };

    class CallStateList
    {
    public:
        inline CallStateList() :
                m_objCallState(IMSMap<IMS_UINT32, CallState*>()),
                m_nNotifiedCallState(STATE_IDLE),
                m_nNotifiedSysMode(SYS_MODE_UNKNOWN),
                m_nUacResult(UAC_RESULT_IDLE)
        {
        }
        inline virtual ~CallStateList() { m_objCallState.Clear(); }

        CallStateList(IN const CallStateList&) = delete;
        CallStateList& operator=(IN const CallStateList&) = delete;

    public:
        IMS_BOOL Add(IN IMS_UINT32 nModule, IN CallState* pState);
        void Remove(IN IMS_UINT32 nModule);

        inline IMSMap<IMS_UINT32, CallState*>& GetCallState() { return m_objCallState; }

        inline CallState* GetCallState(IN IMS_UINT32 nModule)
        {
            IMS_SLONG nIndex = m_objCallState.GetIndexOfKey(nModule);
            return ((nIndex >= 0) ? m_objCallState.GetValueAt(nIndex) : IMS_NULL);
        }

        IMS_BOOL IsStateStopInOtherModule(IN IMS_UINT32 nModule);
        IMS_BOOL IsNotificationRequired(
                IN IMS_UINT32 nModule, IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode);
        IMS_BOOL IsUacCheckNeeded() const;
        void SetForReport(IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode);
        void SetUacResult(IN IMS_RESULT nResult);

    public:
        enum
        {
            UAC_RESULT_IDLE = 0,
            UAC_RESULT_SUCCESS,
            UAC_RESULT_FAILURE
        };

    private:
        IMSMap<IMS_UINT32, CallState*> m_objCallState;
        IMS_UINT32 m_nNotifiedCallState;
        IMS_UINT32 m_nNotifiedSysMode;
        IMS_UINT32 m_nUacResult;
    };

    class CallPreference
    {
    public:
        inline CallPreference() :
                m_nRat(RAT_INVALID),
                m_nState(IDLE)
        {
        }
        inline virtual ~CallPreference() {}

    public:
        inline void Clear()
        {
            m_nState = IDLE;
            m_nRat = RAT_INVALID;
        }
        inline void SetRat(IN IMS_UINT32 nRat) { m_nRat = nRat; }

        inline void SetState(IN IMS_UINT32 nState) { m_nState = nState; }

        inline IMS_UINT32 GetState() const { return m_nState; }

        inline IMS_UINT32 GetRat() const { return m_nRat; }

        inline IMS_BOOL IsIdle() const { return (m_nState == IDLE); }

    public:
        enum
        {
            IDLE = 0,
            REQUEST
        };

    private:
        IMS_UINT32 m_nRat;
        IMS_UINT32 m_nState;
    };

    class CallPreferenceList
    {
    public:
        inline CallPreferenceList() :
                m_objCallPreference(IMSMap<IMS_UINT32, CallPreference*>()),
                m_nRequestedRat(RAT_INVALID),
                m_nState(CallPreference::IDLE)
        {
        }
        inline virtual ~CallPreferenceList() {}

    public:
        IMS_BOOL Add(IN IMS_UINT32 nType, IN CallPreference* pPreference);
        void Remove(IN IMS_UINT32 nType);
        void ClearAll();

        inline CallPreference* GetCallPreference(IN IMS_UINT32 nType)
        {
            IMS_SLONG nIndex = m_objCallPreference.GetIndexOfKey(nType);
            return ((nIndex >= 0) ? m_objCallPreference.GetValueAt(nIndex) : IMS_NULL);
        }

        inline IMS_UINT32 GetRat() const { return m_nRequestedRat; }

        inline IMS_BOOL IsIdle() const { return (m_nState == CallPreference::IDLE); }

        void Set(IN IMS_UINT32 nType, IN IMS_UINT32 nRat, IN IMS_UINT32 nState);
        void SetForReport(IN IMS_UINT32 nRat, IN IMS_UINT32 nState);

    private:
        IMSMap<IMS_UINT32, CallPreference*> m_objCallPreference;
        IMS_UINT32 m_nRequestedRat;
        IMS_UINT32 m_nState;
    };

    class VoNrTimer : public ITimerListener
    {
    public:
        inline VoNrTimer() :
                m_piCallReadyTimer(IMS_NULL),
                m_piListener(IMS_NULL)
        {
        }
        inline virtual ~VoNrTimer() { Stop(TIMER_CALL_READY); }

        VoNrTimer(IN const VoNrTimer&) = delete;
        VoNrTimer& operator=(IN const VoNrTimer&) = delete;

    public:
        void Clear();
        void SetListener(IN IVoNrTimerListener* piListener);
        void Start(IN IMS_UINT32 nType);
        void Stop(IN IMS_UINT32 nType);

    private:
        // ITimerListener
        void Timer_TimerExpired(IN ITimer* piTimer) override;

    public:
        enum
        {
            TIMER_CALL_READY = 0
        };

    private:
        ITimer* m_piCallReadyTimer;
        IVoNrTimerListener* m_piListener;

        static const IMS_UINT32 TIMER_CALL_READY_DURATION = 5000;  // 5s
    };

private:
    CallStateList* GetCallStateList(IN IMS_UINT32 nType);

    static const IMS_CHAR* CallTypeToString(IN IMS_UINT32 nType);
    static const IMS_CHAR* EventToString(IN IMS_UINT32 nEvent);
    static const IMS_CHAR* HandoffStatusToString(IN IMS_UINT32 nStatus);
    static const IMS_CHAR* ModuleToString(IN IMS_UINT32 nModule);
    static const IMS_CHAR* RatToString(IN IMS_UINT32 nRAT);
    static const IMS_CHAR* StateToString(IN IMS_UINT32 nState);
    static const IMS_CHAR* SysModeToString(IN IMS_UINT32 nSysMode);
    static const IMS_CHAR* UacReasonToString(IN IMS_UINT32 nReason);

    // IVoNr::STATE_XXX
    enum
    {
        NAS_IMS_CALL_STATUS_START = 0x0,
        NAS_IMS_CALL_STATUS_STOP = 0x1,
        NAS_IMS_CALL_STATUS_CONNECTED = 0x2
    };

    // IVoNr::SYS_MODE_XXX
    enum
    {
        NAS_RADIO_IF_NO_SVC = 0x0,
        NAS_RADIO_IF_WLAN = 0x6,
        NAS_RADIO_IF_LTE = 0x8,
        NAS_RADIO_IF_NR5G = 0x0C
    };

    // IVoNr::SYS_MODE_XXX for MTK Ims Session
    enum
    {
        MTK_RADIO_IF_LTE = 0,
        MTK_RADIO_IF_WLAN = 1,
        MTK_RADIO_IF_EHRPD = 2,
        MTK_RADIO_IF_GSM = 3,
        MTK_RADIO_IF_UMTS = 4,
        MTK_RADIO_IF_NR5G = 5
    };

    // IVoNrUacListener::REASON_XXX
    enum
    {
        NAS_MMTEL_SUCCESS = 1,
        NAS_MMTEL_ACCESS_BARRED = 2,
        NAS_MMTEL_INVALID_RAT = 3,
        NAS_MMTEL_INVALID_STATE = 4,
        NAS_MMTEL_NO_SERVICE = 5,
        NAS_MMTEL_T3346_ACTIVE = 6,
        NAS_MMTEL_SERVICE_AREA_RESTRICTION = 7
    };

    // IVoNrUacListener::REASON_XXX
    // Result of UAC Check for MTK
    enum
    {
        MTK_UAC_CHECK_NONE = 0,
        MTK_UAC_CHECK_OK = 1,
        MTK_UAC_CHECK_BARRED = 2,
        MTK_UAC_CHECK_NO_COVERAGE = 3,
        MTK_UAC_CHECK_GEMINI_SUSPEND = 4,
        MTK_UAC_CHECK_DEREGISTERED = 5
    };

    // IVoNr::RAT_XXX for handoff information
    enum
    {
        WDS_BEARER_TECH_RAT_EX_3GPP_LTE = 0x03,
        WDS_BEARER_TECH_RAT_EX_3GPP_5G = 0x06
    };

    // IVoNrHandoffListener::STATUS_HANDOFF_XXX
    enum
    {
        RIL_HANDOFF_STATUS_PREFSYS_SUCCESS = 0,
        RIL_HANDOFF_STATUS_PREFSYS_FAILURE = 1,
        RIL_HANDOFF_STATUS_INIT = 2,
        RIL_HANDOFF_STATUS_SUCCESS = 3,
        RIL_HANDOFF_STATUS_FAILURE = 4
    };

    // IVoNr::RAT_XXX for call preference
    enum
    {
        NAS_RAT_MODE_PREF_NO = 0x0,
        NAS_RAT_MODE_PREF_LTE = 0x10,
        NAS_RAT_MODE_PREF_NR5G = 0x40
    };

    // IVoNr::RAT_XXX for MTK call preference
    enum
    {
        MTK_RAT_MODE_PREF_NONE = 0,
        MTK_RAT_MODE_PREF_LTE = 4,
        MTK_RAT_MODE_PREF_NR5G = 128
    };

    // MTK response for RequestCallPreference()
    enum
    {
        MTK_MODE_PREF_RESP_NO_STATUS = 1,
        MTK_MODE_PREF_RESP_STARTED = 2,
        MTK_MODE_PREF_RESP_SUCCESSFUL = 3,
        MTK_MODE_PREF_RESP_FAILED = 4,
        MTK_MODE_PREF_RESP_REJECTED = 5,
        MTK_MODE_PREF_RESP_NEED_RETRY = 6
    };

    // IVoNr::SIGNALING_TYPE_XXX for IMS signaling UAC
    enum
    {
        WDS_LTE_CALL_TYPE_DEFAULT = 0x0,
        WDS_LTE_CALL_TYPE_VOLTE = 0x1
    };

    IThread* m_piOwnerThread;

    CallPreferenceList m_objCallPreferenceList;
    VoNrTimer m_objTimer;
    IMS_BOOL m_bVoNrSupported;
    IMS_BOOL m_bIsMtkChipset;

    IMSList<IVoNrUacListener*> m_objUacListeners;
    IMSList<IVoNrCallPreferenceListener*> m_objCallPreferenceListeners;
    IMSList<IVoNrHandoffListener*> m_objHandoffListeners;

    IMSMap<IMS_UINT32, CallStateList*> m_objMoCallStates;
};

#endif
