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
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "ServiceUtil.h"
#include "SystemConfigManager.h"
#include "device/OsVoNr.h"
#include "network/OsNetworkConstants.h"
#include "system-intf/System.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_ADAPT__;

class OsVoNrParam
{
public:
    inline OsVoNrParam(IN IMS_UINT32 nEvent) :
            m_nEvent(nEvent)
    {
    }
    inline virtual ~OsVoNrParam() {}

public:
    // VoNRAgent sets this notification type
    enum
    {
        EVENT_CALL_READY = 1,
        EVENT_HANDOFF_INFORMATION = 2,
        EVENT_UAC_RESPONSE = 3
    };

    IMS_UINT32 m_nEvent;
};

class UacResponseParam : public OsVoNrParam
{
public:
    inline UacResponseParam() :
            OsVoNrParam(EVENT_UAC_RESPONSE),
            m_nSysMode(-1),
            m_nCallType(0),
            m_nResult(0),
            m_nBarringTime(0)
    {
    }
    inline virtual ~UacResponseParam() {}

public:
    IMS_SINT32 m_nSysMode;
    IMS_UINT32 m_nCallType;
    IMS_UINT32 m_nResult;
    IMS_UINT32 m_nBarringTime;
};

class CallReadyParam : public OsVoNrParam
{
public:
    inline CallReadyParam() :
            OsVoNrParam(EVENT_CALL_READY),
            m_nSysMode(-1)
    {
    }
    inline virtual ~CallReadyParam() {}

public:
    IMS_SINT32 m_nSysMode;
};

class HandoffInformationParam : public OsVoNrParam
{
public:
    inline HandoffInformationParam() :
            OsVoNrParam(EVENT_HANDOFF_INFORMATION),
            m_nStatus(0),
            m_nSourceRat(0),
            m_nTargetRat(0),
            m_nReasonType(0),
            m_nReason(-1)
    {
    }
    inline virtual ~HandoffInformationParam() {}

public:
    IMS_UINT32 m_nStatus;
    IMS_UINT32 m_nSourceRat;
    IMS_UINT32 m_nTargetRat;
    IMS_UINT32 m_nReasonType;
    IMS_SINT32 m_nReason;
};

LOCAL
void osVoNr_SendMessage(IN IThread* piThread, IN IMS_SINT32 nSlotId, IN OsVoNrParam* pParam)
{
    if (piThread == IMS_NULL)
    {
        delete pParam;
        return;
    }

    if (!piThread->PostMessageI(IMS_MSG_VONR, nSlotId, reinterpret_cast<IMS_UINTP>(pParam)))
    {
        delete pParam;
    }
}

PUBLIC
OsVoNr::OsVoNr(IN IMS_SINT32 nSlotId) :
        ImsVoNr(nSlotId),
        m_piOwnerThread(IMS_NULL),
        m_bVoNrSupported(IMS_FALSE),
        m_bIsMtkChipset(IMS_FALSE),
        m_objUacListeners(IMSList<IVoNrUacListener*>()),
        m_objCallPreferenceListeners(IMSList<IVoNrCallPreferenceListener*>()),
        m_objHandoffListeners(IMSList<IVoNrHandoffListener*>()),
        m_objMoCallStates(IMSMap<IMS_UINT32, CallStateList*>())
{
    m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();
    System::GetInstance()->AddListener(SystemConstants::CATEGORY_VONR, this, GetSlotId());

    Initialize();
}

PUBLIC VIRTUAL OsVoNr::~OsVoNr()
{
    CleanUp();

    System::GetInstance()->RemoveListener(SystemConstants::CATEGORY_VONR, this, GetSlotId());
}

PUBLIC VIRTUAL IMS_BOOL OsVoNr::IsVoNrSupported() const
{
    return m_bVoNrSupported;
}

PUBLIC VIRTUAL IMS_BOOL OsVoNr::IsUacCheckRequired(IN IMS_UINT32 nType)
{
    if (!IsNrRat() || !IsCallTypeForUac(nType))
    {
        return IMS_FALSE;
    }

    if (!IsMtkChipset())
    {
        CallStateList* pList = GetCallStateList(nType);
        return (pList != IMS_NULL) ? pList->IsUacCheckNeeded() : IMS_TRUE;
    }
    else
    {
        return IMS_TRUE;
    }
}

PUBLIC VIRTUAL IMS_BOOL OsVoNr::IsUeCapabilityVoNrEnabled() const
{
    const SystemConfig* pSysConfig = SystemConfigManager::GetInstance()->GetConfig(GetSlotId());
    return (pSysConfig != IMS_NULL && pSysConfig->IsUeCapabilityVoNrEnabled());
}

PUBLIC VIRTUAL IMS_BOOL OsVoNr::NotifyCallState(IN IMS_UINT32 nModule, IN IMS_UINT32 nType,
        IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode, IN IMS_UINT32 nDirection)
{
    if ((nState != STATE_IDLE) && (!IsSysModeValidForCallState(nSysMode)))
    {
        IMS_TRACE_I("[%d] NotifyCallState :: sys mode is invalid(type=%d, mode=%d)", GetSlotId(),
                nType, nSysMode);
        return IMS_FALSE;
    }

    if (nType == TYPE_EMERGENCY)
    {
        NotifyCallState_Emergency(nModule, nType, nState, nSysMode);
        return IMS_TRUE;
    }

    if (nDirection == DIRECTION_MO)
    {
        NotifyCallState_Mo(nModule, nType, nState, nSysMode);
    }
    else
    {
        // DIRECTION_MT
        NotifyCallState_Mt(nModule, nType, nState, nSysMode);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_SINT32 OsVoNr::RequestCallPreference(IN IMS_UINT32 nRat, IN IMS_UINT32 nType)
{
    if (!IsRatValidForCallPreference(nRat) || !IsTypeValidForCallPreference(nType))
    {
        return CALL_PREF_REQUEST_FAILURE_INVALID_PARAMETER;
    }

    if (m_objCallPreferenceList.IsIdle())
    {
        m_objCallPreferenceList.Set(nType, nRat, CallPreference::REQUEST);
        m_objCallPreferenceList.SetForReport(nRat, CallPreference::REQUEST);
        System::GetInstance()->RequestCallPreference(
                GetRatForCallPreference(nRat), nType, GetSlotId());
        m_objTimer.Start(VoNrTimer::TIMER_CALL_READY);
        return CALL_PREF_REQUEST_SUCCESS;
    }
    else  // REQUEST
    {
        if (m_objCallPreferenceList.GetRat() == nRat)
        {
            m_objCallPreferenceList.Set(nType, nRat, CallPreference::REQUEST);
            return CALL_PREF_REQUEST_SUCCESS;
        }
        else
        {
            return CALL_PREF_REQUEST_FAILURE_OTHER_RAT_REQUESTED;
        }
    }
}

PUBLIC VIRTUAL IMS_BOOL OsVoNr::SetImsSession(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    IMS_TRACE_I("[%d] SetImsSession :: type(%s) , state(%d)", GetSlotId(), CallTypeToString(nType),
            nState);
    System::GetInstance()->SetImsSession(GetTypeForMtk(nType), nState, GetSlotId());
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsVoNr::SetImsSignaling(IN IMS_UINT32 nType)
{
    IMS_TRACE_I("[%d] SetImsSignaling :: type(%d)", GetSlotId(), nType, 0);
    System::GetInstance()->SetImsSignaling(GetWdsLteCallType(nType), GetSlotId());
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsVoNr::SetImsVoice(IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode)
{
    IMS_TRACE_I("[%d] SetImsVoice :: state(%d), rat(%d)", GetSlotId(), nState, nSysMode);
    System::GetInstance()->SetImsVoice(nState, GetSysModeForMtk(nSysMode), GetSlotId());
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsVoNr::SetUacCheck(IN IMS_UINT32 nType, IN IMS_UINT32 nState)
{
    IMS_TRACE_I("[%d] SetUacCheck :: type(%s) , state(%d)", GetSlotId(), CallTypeToString(nType),
            nState);
    System::GetInstance()->SetUacCheck(GetTypeForMtk(nType), nState, GetSlotId());
    return IMS_TRUE;
}

PUBLIC VIRTUAL IMS_BOOL OsVoNr::SetVoice(IN IMS_UINT32 nState, IN IMS_BOOL bIsEmergency)
{
    IMS_TRACE_I("[%d] SetVoice :: state(%d) , emergency(%d)", GetSlotId(), nState, bIsEmergency);
    System::GetInstance()->SetVoice(nState, bIsEmergency, GetSlotId());
    return IMS_TRUE;
}

PUBLIC VIRTUAL void OsVoNr::AddListenerForUac(IN IVoNrUacListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objUacListeners.GetSize(); ++i)
    {
        IVoNrUacListener* pTmpListener = m_objUacListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            return;
        }
    }

    m_objUacListeners.Append(piListener);
}

PUBLIC VIRTUAL void OsVoNr::RemoveListenerForUac(IN IVoNrUacListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objUacListeners.GetSize(); ++i)
    {
        IVoNrUacListener* pTmpListener = m_objUacListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            m_objUacListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC VIRTUAL void OsVoNr::AddListenerForCallPreference(IN IVoNrCallPreferenceListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objCallPreferenceListeners.GetSize(); ++i)
    {
        IVoNrCallPreferenceListener* pTmpListener = m_objCallPreferenceListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            return;
        }
    }

    m_objCallPreferenceListeners.Append(piListener);
}

PUBLIC VIRTUAL void OsVoNr::RemoveListenerForCallPreference(
        IN IVoNrCallPreferenceListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objCallPreferenceListeners.GetSize(); ++i)
    {
        IVoNrCallPreferenceListener* pTmpListener = m_objCallPreferenceListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            m_objCallPreferenceListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC VIRTUAL void OsVoNr::AddListenerForHandoff(IN IVoNrHandoffListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objHandoffListeners.GetSize(); ++i)
    {
        IVoNrHandoffListener* pTmpListener = m_objHandoffListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            return;
        }
    }

    m_objHandoffListeners.Append(piListener);
}

PUBLIC VIRTUAL void OsVoNr::RemoveListenerForHandoff(IN IVoNrHandoffListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objHandoffListeners.GetSize(); ++i)
    {
        IVoNrHandoffListener* pTmpListener = m_objHandoffListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            m_objHandoffListeners.RemoveAt(i);
            return;
        }
    }
}

PROTECTED VIRTUAL void OsVoNr::DispatchServiceMessage(IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    (void)nWParam;

    OsVoNrParam* pVoNrParam = reinterpret_cast<OsVoNrParam*>(nLParam);

    if (pVoNrParam != IMS_NULL)
    {
        IMS_TRACE_I("VoNR :: DispatchServiceMessage - slotId=%d, event=%s, lp=%" PFLS_u,
                GetSlotId(), EventToString(pVoNrParam->m_nEvent), nLParam);

        if (pVoNrParam->m_nEvent == OsVoNrParam::EVENT_UAC_RESPONSE)
        {
            UacResponseParam* pParam = reinterpret_cast<UacResponseParam*>(pVoNrParam);
            NotifyUacResponse(pParam->m_nCallType, pParam->m_nResult, pParam->m_nSysMode,
                    pParam->m_nBarringTime);
        }
        else if (pVoNrParam->m_nEvent == OsVoNrParam::EVENT_CALL_READY)
        {
            CallReadyParam* pParam = reinterpret_cast<CallReadyParam*>(pVoNrParam);
            NotifyCallReady(pParam->m_nSysMode);
        }
        else if (pVoNrParam->m_nEvent == OsVoNrParam::EVENT_HANDOFF_INFORMATION)
        {
            HandoffInformationParam* pParam =
                    reinterpret_cast<HandoffInformationParam*>(pVoNrParam);
            NotifyHandoffInformation(pParam->m_nStatus, pParam->m_nSourceRat, pParam->m_nTargetRat,
                    pParam->m_nReasonType, pParam->m_nReason);
        }

        delete pVoNrParam;
    }
}

PROTECTED VIRTUAL void OsVoNr::System_NotifyEvent(
        IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    (void)nWParam;

    IMS_TRACE_D("System_NotifyEvent :: event=%s", EventToString(nEvent), 0, 0);

    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(nLParam);

    if (pParcel == IMS_NULL)
    {
        return;
    }

    switch (nEvent)
    {
        case OsVoNrParam::EVENT_UAC_RESPONSE:
        {
            UacResponseParam* pParam = new UacResponseParam();
            pParam->m_nCallType = pParcel->readInt32();
            pParam->m_nSysMode = pParcel->readInt32();
            pParam->m_nResult = pParcel->readInt32();
            pParam->m_nBarringTime = pParcel->readInt32();

            osVoNr_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        case OsVoNrParam::EVENT_CALL_READY:
        {
            CallReadyParam* pParam = new CallReadyParam();
            pParam->m_nSysMode = pParcel->readInt32();

            osVoNr_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        case OsVoNrParam::EVENT_HANDOFF_INFORMATION:
        {
            HandoffInformationParam* pParam = new HandoffInformationParam();
            pParam->m_nStatus = pParcel->readInt32();
            pParam->m_nSourceRat = pParcel->readInt32();
            pParam->m_nTargetRat = pParcel->readInt32();
            pParam->m_nReasonType = pParcel->readInt32();
            pParam->m_nReason = pParcel->readInt32();

            osVoNr_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        default:
        {
            // no-op
            break;
        }
    }
}

PROTECTED VIRTUAL void OsVoNr::VoNrTimer_TimerExpired(IN IMS_UINT32 nType)
{
    if (nType == VoNrTimer::TIMER_CALL_READY)
    {
        m_objCallPreferenceList.ClearAll();
    }
}

PROTECTED VIRTUAL void OsVoNr::SystemConfig_ConfigurationChanged(
        IN IMS_SINT32 nEvent, IN IMS_SINT32 nSlotId /*= IMS_SLOT_ANY*/)
{
    (void)nEvent;

    if ((nSlotId == IMS_SLOT_ANY) || (nSlotId == GetSlotId()))
    {
        SetVoNrSupported();
    }
}

PRIVATE
void OsVoNr::Initialize()
{
    SetVoNrSupported();

    m_bIsMtkChipset =
            UtilService::GetUtilService()->GetSystemProperty()->GetChipsetVendor().EqualsIgnoreCase(
                    "MediaTek");

    // objMoCallState
    CallStateList* objVoiceList = new CallStateList();
    objVoiceList->Add(MODULE_UC, new CallState());
    objVoiceList->Add(MODULE_AOS, new CallState());
    m_objMoCallStates.Add(TYPE_VOICE, objVoiceList);

    CallStateList* objVideoList = new CallStateList();
    objVideoList->Add(MODULE_UC, new CallState());
    m_objMoCallStates.Add(TYPE_VIDEO, objVideoList);

    CallStateList* objSmsList = new CallStateList();
    objSmsList->Add(MODULE_SMS, new CallState());
    objSmsList->Add(MODULE_AOS, new CallState());
    m_objMoCallStates.Add(TYPE_SMS, objSmsList);

    // objCallPreference
    m_objCallPreferenceList.Add(TYPE_VOICE, new CallPreference());
    m_objCallPreferenceList.Add(TYPE_VIDEO, new CallPreference());

    m_objTimer.SetListener(this);
}

PRIVATE
void OsVoNr::CleanUp()
{
    m_objTimer.SetListener(IMS_NULL);

    m_objCallPreferenceList.Remove(TYPE_VOICE);
    m_objCallPreferenceList.Remove(TYPE_VIDEO);

    for (IMS_UINT32 i = 0; i < m_objMoCallStates.GetSize(); ++i)
    {
        CallStateList* pCallStateList = m_objMoCallStates.GetValueAt(i);
        if (pCallStateList != IMS_NULL)
        {
            IMSMap<IMS_UINT32, CallState*>& objCallState = pCallStateList->GetCallState();
            for (IMS_UINT32 j = 0; j < objCallState.GetSize(); ++j)
            {
                CallState* pCallState = objCallState.GetValueAt(j);
                if (pCallState != IMS_NULL)
                {
                    delete pCallState;
                }
            }
            delete pCallStateList;
        }
    }

    m_objMoCallStates.Clear();

    m_objHandoffListeners.Clear();
    m_objCallPreferenceListeners.Clear();
    m_objUacListeners.Clear();

    m_objTimer.Clear();
}

PRIVATE
IMS_UINT32 OsVoNr::GetHandoffStatus(IN IMS_UINT32 nStatus) const
{
    switch (nStatus)
    {
        case RIL_HANDOFF_STATUS_INIT:
            return IVoNrHandoffListener::STATUS_HANDOFF_INIT;

        case RIL_HANDOFF_STATUS_SUCCESS:
            return IVoNrHandoffListener::STATUS_HANDOFF_SUCCESS;

        case RIL_HANDOFF_STATUS_FAILURE:
            return IVoNrHandoffListener::STATUS_HANDOFF_FAILURE;

        default:
            return IVoNrHandoffListener::STATUS_HANDOFF_FAILURE;
    }
}

PRIVATE
IMS_UINT32 OsVoNr::GetImsCallType(IN IMS_UINT32 nType) const
{
    if (IsMtkChipset())
    {
        if (nType == MTK_TYPE_REG_SIGNAL)
        {
            return TYPE_REG_SIGNAL;
        }
    }

    return nType;
}

PRIVATE
IMS_UINT32 OsVoNr::GetMtkRatForCallPreference(IN IMS_UINT32 nRat) const
{
    switch (nRat)
    {
        case RAT_LTE:
            return MTK_RAT_MODE_PREF_LTE;

        case RAT_NR5G:
            return MTK_RAT_MODE_PREF_NR5G;

        default:
            return MTK_RAT_MODE_PREF_NONE;
    }
}

PRIVATE
IMS_UINT32 OsVoNr::GetNasStatusForCallState(IN IMS_UINT32 nState) const
{
    switch (nState)
    {
        case STATE_IDLE:
            return NAS_IMS_CALL_STATUS_STOP;

        case STATE_START:
            return NAS_IMS_CALL_STATUS_START;

        case STATE_CONNECTED:
            return NAS_IMS_CALL_STATUS_CONNECTED;

        default:
            return NAS_IMS_CALL_STATUS_STOP;
    }
}

PRIVATE
IMS_UINT32 OsVoNr::GetNasRatForCallPreference(IN IMS_UINT32 nRat) const
{
    switch (nRat)
    {
        case RAT_LTE:
            return NAS_RAT_MODE_PREF_LTE;

        case RAT_NR5G:
            return NAS_RAT_MODE_PREF_NR5G;

        default:
            return NAS_RAT_MODE_PREF_NO;
    }
}

PRIVATE
IMS_UINT32 OsVoNr::GetRatForCallPreference(IN IMS_UINT32 nRat) const
{
    if (IsMtkChipset())
    {
        return GetMtkRatForCallPreference(nRat);
    }
    else
    {
        return GetNasRatForCallPreference(nRat);
    }
}

PRIVATE
IMS_UINT32 OsVoNr::GetRatFromBearer(IN IMS_UINT32 nRat) const
{
    switch (nRat)
    {
        case WDS_BEARER_TECH_RAT_EX_3GPP_LTE:
            return RAT_LTE;

        case WDS_BEARER_TECH_RAT_EX_3GPP_5G:
            return RAT_NR5G;

        default:
            return RAT_INVALID;
    }
}

PRIVATE
IMS_SINT32 OsVoNr::GetReasonForUac(IN IMS_UINT32 nResponse, OUT IMS_RESULT& nResult)
{
    if (IsMtkChipset())
    {
        nResult = (nResponse == MTK_UAC_CHECK_BARRED) ? IMS_FAILURE : IMS_SUCCESS;

        switch (nResponse)
        {
            case MTK_UAC_CHECK_NONE:  // FALL-THROUGH
            case MTK_UAC_CHECK_OK:
                return IVoNrUacListener::REASON_NO;

            case MTK_UAC_CHECK_BARRED:
                return IVoNrUacListener::REASON_ACCESS_BARRED;

            case MTK_UAC_CHECK_NO_COVERAGE:
                return IVoNrUacListener::REASON_NO_SERVICE;

            default:
                return IVoNrUacListener::REASON_UNKNOWN;
        }
    }
    else
    {
        if (nResponse == NAS_MMTEL_SUCCESS)
        {
            nResult = IMS_SUCCESS;
            return IVoNrUacListener::REASON_NO;
        }
        else
        {
            nResult = IMS_FAILURE;
        }

        switch (nResponse)
        {
            case NAS_MMTEL_ACCESS_BARRED:
                return IVoNrUacListener::REASON_ACCESS_BARRED;

            case NAS_MMTEL_INVALID_RAT:
                return IVoNrUacListener::REASON_INVALID_RAT;

            case NAS_MMTEL_INVALID_STATE:
                return IVoNrUacListener::REASON_INVALID_STATE;

            case NAS_MMTEL_NO_SERVICE:
                return IVoNrUacListener::REASON_NO_SERVICE;

            case NAS_MMTEL_T3346_ACTIVE:
                return IVoNrUacListener::REASON_T3346_ACTIVE;

            case NAS_MMTEL_SERVICE_AREA_RESTRICTION:
                return IVoNrUacListener::REASON_SERVICE_AREA_RESTRICTION;

            default:
                return IVoNrUacListener::REASON_UNKNOWN;
        }
    }
}

PRIVATE
IMS_UINT32 OsVoNr::GetSysMode(IN IMS_SINT32 nSysMode) const
{
    switch (nSysMode)
    {
        case NAS_RADIO_IF_LTE:
            return SYS_MODE_LTE;

        case NAS_RADIO_IF_NR5G:
            return SYS_MODE_NR5G;

        default:
            return SYS_MODE_UNKNOWN;
    }
}

PRIVATE
IMS_UINT32 OsVoNr::GetSysModeForMtk(IN IMS_UINT32 nSysMode) const
{
    switch (nSysMode)
    {
        case SYS_MODE_LTE:
            return MTK_RADIO_IF_LTE;

        case SYS_MODE_WLAN:
            return MTK_RADIO_IF_WLAN;

        case SYS_MODE_NR5G:
            return MTK_RADIO_IF_NR5G;

        default:
            return MTK_RADIO_IF_LTE;
    }
}

PRIVATE
IMS_UINT32 OsVoNr::GetSysModeForNas(IN IMS_UINT32 nSysMode) const
{
    switch (nSysMode)
    {
        case SYS_MODE_LTE:
            return NAS_RADIO_IF_LTE;

        case SYS_MODE_NR5G:
            return NAS_RADIO_IF_NR5G;

        default:
            return NAS_RADIO_IF_NO_SVC;
    }
}

PRIVATE
IMS_UINT32 OsVoNr::GetTypeForMtk(IN IMS_UINT32 nType) const
{
    switch (nType)
    {
        case TYPE_REG_SIGNAL:
            return MTK_TYPE_REG_SIGNAL;

        default:
            return nType;
    }
}

PRIVATE
IMS_UINT32 OsVoNr::GetWdsLteCallType(IN IMS_UINT32 nType) const
{
    switch (nType)
    {
        case SIGNALING_TYPE_IDLE:
            return WDS_LTE_CALL_TYPE_DEFAULT;

        case SIGNALING_TYPE_ACTIVE:
            return WDS_LTE_CALL_TYPE_VOLTE;

        default:
            return WDS_LTE_CALL_TYPE_DEFAULT;
    }
}

PRIVATE
IMS_BOOL OsVoNr::IsMtkChipset() const
{
    return m_bIsMtkChipset;
}

PRIVATE
IMS_BOOL OsVoNr::IsNrRat()
{
    return (System::GetInstance()->GetNetworkType(GetSlotId()) == RADIOTECH_TYPE_NR);
}

PRIVATE
IMS_BOOL OsVoNr::IsStatusValidForHandoff(IN IMS_UINT32 nStatus) const
{
    return (nStatus == RIL_HANDOFF_STATUS_INIT || nStatus == RIL_HANDOFF_STATUS_SUCCESS ||
            nStatus == RIL_HANDOFF_STATUS_FAILURE);
}

PRIVATE
IMS_BOOL OsVoNr::IsSysModeValidForCallState(IN IMS_UINT32 nSysMode) const
{
    return (nSysMode == SYS_MODE_LTE || nSysMode == SYS_MODE_NR5G || nSysMode == SYS_MODE_WLAN);
}

PRIVATE
IMS_BOOL OsVoNr::IsRatValidForCallPreference(IN IMS_UINT32 nRat) const
{
    return (nRat == RAT_LTE || nRat == RAT_NR5G);
}

PRIVATE
IMS_BOOL OsVoNr::IsTypeValidForCallPreference(IN IMS_UINT32 nType) const
{
    return (nType == TYPE_VOICE || nType == TYPE_VIDEO);
}

PRIVATE
IMS_BOOL OsVoNr::IsCallTypeForUac(IN IMS_UINT32 nType) const
{
    return (nType == TYPE_VOICE || nType == TYPE_VIDEO || nType == TYPE_SMS);
}

PRIVATE
void OsVoNr::NotifyCallState_Emergency(
        IN IMS_UINT32 nModule, IN IMS_UINT32 nType, IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode)
{
    (void)nModule;

    IMS_TRACE_I("NotifyCallState_EMERGENCY :: %s, %s, %s", ModuleToString(nModule),
            CallTypeToString(nType), StateToString(nState));

    System::GetInstance()->NotifyCallState(nType, GetNasStatusForCallState(nState),
            GetSysModeForNas(nSysMode), DIRECTION_MO, GetSlotId());
}

PRIVATE
void OsVoNr::NotifyCallState_Mo(
        IN IMS_UINT32 nModule, IN IMS_UINT32 nType, IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode)
{
    IMS_TRACE_I("NotifyCallState_MO :: %s, %s, %s", ModuleToString(nModule),
            CallTypeToString(nType), StateToString(nState));

    CallStateList* pList = GetCallStateList(nType);
    if (pList == IMS_NULL)
    {
        return;
    }

    CallState* pState = pList->GetCallState(nModule);
    if (pState == IMS_NULL)
    {
        return;
    }

    if (!pState->IsNotificationRequired(nState, nSysMode))
    {
        pState->Set(nState, nSysMode);
        return;
    }

    pState->Set(nState, nSysMode);

    if (pList->IsNotificationRequired(nModule, nState, nSysMode))
    {
        pList->SetForReport(nState, nSysMode);

        System::GetInstance()->NotifyCallState(nType, GetNasStatusForCallState(nState),
                GetSysModeForNas(nSysMode), DIRECTION_MO, GetSlotId());
    }
}

PRIVATE
void OsVoNr::NotifyCallState_Mt(
        IN IMS_UINT32 nModule, IN IMS_UINT32 nType, IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode)
{
    (void)nModule;

    IMS_TRACE_I("NotifyCallState_MO :: %s, %s, %s", ModuleToString(nModule),
            CallTypeToString(nType), StateToString(nState));

    System::GetInstance()->NotifyCallState(nType, GetNasStatusForCallState(nState),
            GetSysModeForNas(nSysMode), DIRECTION_MT, GetSlotId());
}

PRIVATE
void OsVoNr::NotifyCallReady(IN IMS_UINT32 nSysMode)
{
    IMS_UINT32 nConvertSysMode = SYS_MODE_UNKNOWN;

    if (IsMtkChipset())
    {
        if (nSysMode != MTK_MODE_PREF_RESP_SUCCESSFUL)
        {
            return;
        }

        nConvertSysMode = m_objCallPreferenceList.GetRat();
    }
    else
    {
        nConvertSysMode = GetSysMode(nSysMode);
    }

    m_objTimer.Stop(VoNrTimer::TIMER_CALL_READY);
    m_objCallPreferenceList.ClearAll();

    IMS_TRACE_I("NotifyCallReady sys mode=%s", SysModeToString(nConvertSysMode), 0, 0);

    for (IMS_UINT32 i = 0; i < m_objCallPreferenceListeners.GetSize(); i++)
    {
        IVoNrCallPreferenceListener* piListener = m_objCallPreferenceListeners.GetAt(i);
        if (piListener != IMS_NULL)
        {
            piListener->VoNrCallPreference_NotifyCallReady(nConvertSysMode);
        }
    }
}

PRIVATE
void OsVoNr::NotifyHandoffInformation(IN IMS_UINT32 nStatus, IN IMS_UINT32 nSourceRat,
        IN IMS_UINT32 nTargetRat, IN IMS_UINT32 nReasonType, IN IMS_SINT32 nReason)
{
    (void)nReasonType;

    if (!IsStatusValidForHandoff(nStatus))
    {
        return;
    }

    IMS_UINT32 nHandoffStatus = GetHandoffStatus(nStatus);
    IMS_UINT32 nSRat = GetRatFromBearer(nSourceRat);
    IMS_UINT32 nTRat = GetRatFromBearer(nTargetRat);

    AString strLog;
    AString strNumber;
    strLog.Append("status=");
    strLog.Append(HandoffStatusToString(nHandoffStatus));
    strLog.Append(", sRAT=");
    strLog.Append(RatToString(nSRat));
    strLog.Append(", tRAT=");
    strLog.Append(RatToString(nTRat));
    strLog.Append(", reason type=");
    strLog.Append(strNumber.SetNumber(nReasonType));
    strLog.Append(", reason=");
    strLog.Append(strNumber.SetNumber(nReason));

    IMS_TRACE_D("NotifyHandoffInformation :: %s", strLog.GetStr(), 0, 0);

    for (IMS_UINT32 i = 0; i < m_objHandoffListeners.GetSize(); i++)
    {
        IVoNrHandoffListener* piListener = m_objHandoffListeners.GetAt(i);
        if (piListener != IMS_NULL)
        {
            piListener->VoNrHandoff_NotifyInformation(nHandoffStatus, nSRat, nTRat, nReason);
        }
    }
}

PRIVATE
void OsVoNr::NotifyUacResponse(IN IMS_UINT32 nCallType, IN IMS_UINT32 nResult,
        IN IMS_SINT32 nSysMode, IN IMS_UINT32 nBarringTime)
{
    IMS_UINT32 nImsCallType = GetImsCallType(nCallType);
    IMS_RESULT nReportResult = IMS_FAILURE;
    IMS_SINT32 nReason = GetReasonForUac(nResult, nReportResult);
    IMS_UINT32 nConvertSysMode = GetSysMode(nSysMode);

    IMS_TRACE_I("NotifyUACResponse :: type=%s, result=%s, sys mode=%s",
            CallTypeToString(nImsCallType), UacReasonToString(nResult),
            SysModeToString(nConvertSysMode));

    if (!IsMtkChipset())
    {
        CallStateList* pList = GetCallStateList(nImsCallType);
        if (pList != IMS_NULL)
        {
            pList->SetUacResult(nReportResult);
        }
    }

    for (IMS_UINT32 i = 0; i < m_objUacListeners.GetSize(); i++)
    {
        IVoNrUacListener* piListener = m_objUacListeners.GetAt(i);
        if (piListener != IMS_NULL)
        {
            piListener->VoNrUac_NotifyResponse(
                    nImsCallType, nReportResult, nReason, nConvertSysMode, nBarringTime);
        }
    }
}

PRIVATE
void OsVoNr::SetVoNrSupported()
{
    // TODO: use carrier-config
    IMS_BOOL bFeatureSupported = IMS_FALSE;
    IMS_BOOL bNsaModeEnabled = IMS_FALSE;

    if (bFeatureSupported)
    {
        const SystemConfig* pSysConfig = SystemConfigManager::GetInstance()->GetConfig(GetSlotId());

        if (pSysConfig != IMS_NULL && pSysConfig->IsNrNsaModeEnabled())
        {
            bNsaModeEnabled = IMS_TRUE;
        }
    }
    else
    {
        bNsaModeEnabled = IMS_TRUE;
    }

    m_bVoNrSupported = bFeatureSupported && !bNsaModeEnabled;

    IMS_TRACE_I("[%d] SetVoNRSupported :: (%s), mode=%s", GetSlotId(), _TRACE_B_(m_bVoNrSupported),
            bNsaModeEnabled ? "NSA" : "SA");
}

PUBLIC
void OsVoNr::CallState::Clear()
{
    m_nState = STATE_IDLE;
    m_nSysMode = SYS_MODE_UNKNOWN;
}

PUBLIC
IMS_BOOL OsVoNr::CallState::IsNotificationRequired(
        IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode) const
{
    if (IsIdle(m_nState) && IsIdle(nState))
    {
        return IMS_FALSE;
    }

    return (nState != m_nState || nSysMode != m_nSysMode);
}

PUBLIC
void OsVoNr::CallState::Set(IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode)
{
    m_nState = nState;
    m_nSysMode = nSysMode;

    if (IsIdle(m_nState))
    {
        Clear();
    }
}

PUBLIC
IMS_BOOL OsVoNr::CallStateList::Add(IN IMS_UINT32 nModule, IN CallState* pState)
{
    if (pState == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SLONG nIndex = m_objCallState.GetIndexOfKey(nModule);

    if (nIndex < 0)
    {
        if (!m_objCallState.Add(nModule, pState))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        CallState* pCallState = m_objCallState.GetValueAt(nIndex);
        if (pCallState == pState)
        {
            return IMS_TRUE;
        }

        if (pCallState != IMS_NULL)
        {
            delete pCallState;
        }

        m_objCallState.SetValueAt(nIndex, pState);
    }

    return IMS_TRUE;
}

PUBLIC
void OsVoNr::CallStateList::Remove(IN IMS_UINT32 nModule)
{
    IMS_SLONG nIndex = m_objCallState.GetIndexOfKey(nModule);

    if (nIndex < 0)
    {
        return;
    }

    CallState* pCallState = m_objCallState.GetValueAt(nIndex);
    if (pCallState != IMS_NULL)
    {
        delete pCallState;
    }

    m_objCallState.RemoveAt(nIndex);
}

PUBLIC
IMS_BOOL OsVoNr::CallStateList::IsStateStopInOtherModule(IN IMS_UINT32 nModule)
{
    for (IMS_UINT32 i = 0; i < m_objCallState.GetSize(); ++i)
    {
        if (m_objCallState.GetKeyAt(i) == nModule)
        {
            continue;
        }

        CallState* pState = m_objCallState.GetValueAt(i);
        if (pState != IMS_NULL)
        {
            if (pState->GetState() != STATE_IDLE)
            {
                return IMS_FALSE;
            }
        }
    }

    return IMS_TRUE;
}

PUBLIC
IMS_BOOL OsVoNr::CallStateList::IsNotificationRequired(
        IN IMS_UINT32 nModule, IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode)
{
    switch (nState)
    {
        case STATE_CONNECTED:
        {
            if (m_nNotifiedCallState == STATE_CONNECTED)
            {
                return (m_nNotifiedSysMode != nSysMode);
            }
            return IMS_TRUE;
        }

        case STATE_START:
        {
            if (m_nNotifiedCallState == STATE_CONNECTED)
            {
                return IMS_FALSE;
            }
            else if (m_nNotifiedCallState == STATE_START)
            {
                return (m_nNotifiedSysMode != nSysMode);
            }
            // (nNotifiedCallState == STATE_IDLE)
            return IMS_TRUE;
        }

        case STATE_IDLE:
        {
            if (m_nNotifiedCallState == STATE_IDLE)
            {
                return IMS_FALSE;
            }
            return IsStateStopInOtherModule(nModule);
        }

        default:
            return IMS_FALSE;
    }
}

PUBLIC
IMS_BOOL OsVoNr::CallStateList::IsUacCheckNeeded() const
{
    if ((m_nNotifiedCallState == STATE_CONNECTED) ||
            (m_nNotifiedCallState == STATE_START && m_nUacResult == UAC_RESULT_SUCCESS))
    {
        return IMS_FALSE;
    }

    return IMS_TRUE;
}

PUBLIC
void OsVoNr::CallStateList::SetForReport(IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode)
{
    m_nNotifiedCallState = nState;

    if (nState == STATE_IDLE)
    {
        m_nNotifiedSysMode = SYS_MODE_UNKNOWN;
        m_nUacResult = UAC_RESULT_IDLE;
    }
    else
    {
        m_nNotifiedSysMode = nSysMode;
    }
}

PUBLIC
void OsVoNr::CallStateList::SetUacResult(IN IMS_RESULT nResult)
{
    m_nUacResult = (nResult == IMS_SUCCESS) ? UAC_RESULT_SUCCESS : UAC_RESULT_FAILURE;
}

PUBLIC
IMS_BOOL OsVoNr::CallPreferenceList::Add(IN IMS_UINT32 nType, IN CallPreference* pPreference)
{
    if (pPreference == IMS_NULL)
    {
        return IMS_FALSE;
    }

    IMS_SLONG nIndex = m_objCallPreference.GetIndexOfKey(nType);

    if (nIndex < 0)
    {
        if (!m_objCallPreference.Add(nType, pPreference))
        {
            return IMS_FALSE;
        }
    }
    else
    {
        CallPreference* pCallPreference = m_objCallPreference.GetValueAt(nIndex);
        if (pCallPreference == pPreference)
        {
            return IMS_TRUE;
        }

        if (pCallPreference != IMS_NULL)
        {
            delete pCallPreference;
        }

        m_objCallPreference.SetValueAt(nIndex, pPreference);
    }

    return IMS_TRUE;
}

PUBLIC
void OsVoNr::CallPreferenceList::Remove(IN IMS_UINT32 nType)
{
    IMS_SLONG nIndex = m_objCallPreference.GetIndexOfKey(nType);

    if (nIndex < 0)
    {
        return;
    }

    CallPreference* pCallPreference = m_objCallPreference.GetValueAt(nIndex);
    if (pCallPreference != IMS_NULL)
    {
        delete pCallPreference;
    }

    m_objCallPreference.RemoveAt(nIndex);
}

PUBLIC
void OsVoNr::CallPreferenceList::ClearAll()
{
    for (IMS_UINT32 i = 0; i < m_objCallPreference.GetSize(); i++)
    {
        CallPreference* pPreference = m_objCallPreference.GetValueAt(i);
        if (pPreference != IMS_NULL)
        {
            pPreference->Clear();
        }
    }

    m_nRequestedRat = RAT_INVALID;
    m_nState = CallPreference::IDLE;
}

PUBLIC
void OsVoNr::CallPreferenceList::Set(IN IMS_UINT32 nType, IN IMS_UINT32 nRat, IN IMS_UINT32 nState)
{
    CallPreference* pPreference = GetCallPreference(nType);
    if (pPreference != IMS_NULL)
    {
        pPreference->SetRat(nRat);
        pPreference->SetState(nState);
    }
}

PUBLIC
void OsVoNr::CallPreferenceList::SetForReport(IN IMS_UINT32 nRat, IN IMS_UINT32 nState)
{
    m_nRequestedRat = nRat;
    m_nState = nState;
}

PUBLIC
void OsVoNr::VoNrTimer::Clear()
{
    Stop(TIMER_CALL_READY);
}

PUBLIC
void OsVoNr::VoNrTimer::SetListener(IN IVoNrTimerListener* piListener)
{
    m_piListener = piListener;
}

PUBLIC
void OsVoNr::VoNrTimer::Start(IN IMS_UINT32 nType)
{
    if (nType == TIMER_CALL_READY)
    {
        Stop(nType);
        m_piCallReadyTimer = TimerService::GetTimerService()->CreateTimer();
        m_piCallReadyTimer->SetTimer(TIMER_CALL_READY_DURATION, this);
    }
}

PUBLIC
void OsVoNr::VoNrTimer::Stop(IN IMS_UINT32 nType)
{
    if (nType == TIMER_CALL_READY)
    {
        if (m_piCallReadyTimer == IMS_NULL)
        {
            return;
        }

        m_piCallReadyTimer->KillTimer();
        TimerService::GetTimerService()->DestroyTimer(m_piCallReadyTimer);
        m_piCallReadyTimer = IMS_NULL;
    }
}

PRIVATE VIRTUAL void OsVoNr::VoNrTimer::Timer_TimerExpired(IN ITimer* piTimer)
{
    if (piTimer == IMS_NULL)
    {
        return;
    }

    if (piTimer == m_piCallReadyTimer)
    {
        Stop(TIMER_CALL_READY);

        if (m_piListener != IMS_NULL)
        {
            m_piListener->VoNrTimer_TimerExpired(TIMER_CALL_READY);
        }
    }
}

PRIVATE
OsVoNr::CallStateList* OsVoNr::GetCallStateList(IN IMS_UINT32 nType)
{
    IMS_SLONG nIndex = m_objMoCallStates.GetIndexOfKey(nType);
    return (nIndex >= 0) ? m_objMoCallStates.GetValueAt(nIndex) : IMS_NULL;
}

PRIVATE GLOBAL const IMS_CHAR* OsVoNr::CallTypeToString(IN IMS_UINT32 nType)
{
    switch (nType)
    {
        case TYPE_VOICE:
            return "TYPE_VOICE";

        case TYPE_VIDEO:
            return "TYPE_VIDEO";

        case TYPE_SMS:
            return "TYPE_SMS";

        case TYPE_EMERGENCY:
            return "TYPE_EMERGENCY";

        case TYPE_REG_SIGNAL:
            return "TYPE_REG_SIGNAL";

        default:
            return "__INVALID__";
    }
}

PRIVATE GLOBAL const IMS_CHAR* OsVoNr::EventToString(IN IMS_UINT32 nEvent)
{
    switch (nEvent)
    {
        case OsVoNrParam::EVENT_CALL_READY:
            return "EVENT_CALL_READY";

        case OsVoNrParam::EVENT_HANDOFF_INFORMATION:
            return "EVENT_HANDOFF_INFORMATION";

        case OsVoNrParam::EVENT_UAC_RESPONSE:
            return "EVENT_UAC_RESPONSE";

        default:
            return "__INVALID__";
    }
}

PRIVATE GLOBAL const IMS_CHAR* OsVoNr::HandoffStatusToString(IN IMS_UINT32 nStatus)
{
    switch (nStatus)
    {
        case IVoNrHandoffListener::STATUS_HANDOFF_INIT:
            return "STATUS_HANDOFF_INIT";

        case IVoNrHandoffListener::STATUS_HANDOFF_SUCCESS:
            return "STATUS_HANDOFF_SUCCESS";

        case IVoNrHandoffListener::STATUS_HANDOFF_FAILURE:
            return "STATUS_HANDOFF_FAILURE";

        default:
            return "__INVALID__";
    }
}

PRIVATE GLOBAL const IMS_CHAR* OsVoNr::ModuleToString(IN IMS_UINT32 nModule)
{
    switch (nModule)
    {
        case MODULE_UC:
            return "MODULE_UC";

        case MODULE_AOS:
            return "MODULE_AOS";

        case MODULE_SMS:
            return "MODULE_SMS";

        default:
            return "__INVALID__";
    }
}

PRIVATE GLOBAL const IMS_CHAR* OsVoNr::RatToString(IN IMS_UINT32 nRat)
{
    switch (nRat)
    {
        case RAT_LTE:
            return "RAT_LTE";

        case RAT_NR5G:
            return "RAT_NR5G";

        default:
            return "__INVALID__";
    }
}

PRIVATE GLOBAL const IMS_CHAR* OsVoNr::StateToString(IN IMS_UINT32 nState)
{
    switch (nState)
    {
        case STATE_IDLE:
            return "STATE_IDLE";

        case STATE_START:
            return "STATE_START";

        case STATE_CONNECTED:
            return "STATE_CONNECTED";

        default:
            return "__INVALID__";
    }
}

PRIVATE GLOBAL const IMS_CHAR* OsVoNr::SysModeToString(IN IMS_UINT32 nSysMode)
{
    switch (nSysMode)
    {
        case SYS_MODE_LTE:
            return "SYS_MODE_LTE";

        case SYS_MODE_NR5G:
            return "SYS_MODE_NR5G";

        default:
            return "__INVALID__";
    }
}

PRIVATE GLOBAL const IMS_CHAR* OsVoNr::UacReasonToString(IN IMS_UINT32 nReason)
{
    switch (nReason)
    {
        case NAS_MMTEL_SUCCESS:
            return "NAS_MMTEL_SUCCESS";

        case NAS_MMTEL_ACCESS_BARRED:
            return "NAS_MMTEL_ACCESS_BARRED";

        case NAS_MMTEL_INVALID_RAT:
            return "NAS_MMTEL_INVALID_RAT";

        case NAS_MMTEL_INVALID_STATE:
            return "NAS_MMTEL_INVALID_STATE";

        case NAS_MMTEL_NO_SERVICE:
            return "NAS_MMTEL_NO_SERVICE";

        case NAS_MMTEL_T3346_ACTIVE:
            return "NAS_MMTEL_T3346_ACTIVE";

        case NAS_MMTEL_SERVICE_AREA_RESTRICTION:
            return "NAS_MMTEL_SERVICE_AREA_RESTRICTION";

        default:
            return "__UNKNOWN__";
    }
}
