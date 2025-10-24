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
#include "OsParcel.h"
#include "PlatformContext.h"
#include "ServiceImsRadio.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "device/OsImsRadio.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_IPL__;

class OsImsRadioParam
{
public:
    inline explicit OsImsRadioParam(IN IMS_UINT32 nEvent) :
            m_nEvent(nEvent)
    {
    }
    inline virtual ~OsImsRadioParam() {}

public:
    enum
    {
        EVENT_CONNECTION_FAILED = 1,
        EVENT_CONNECTION_SETUP_PREPARED = 2,
        EVENT_SSAC_STATE_CHANGED = 3,
        EVENT_SIMULTANEOUS_CALLING_SUPPORT_CHANGED = 4,
    };

    IMS_UINT32 m_nEvent;
};

class ConnectionFailedParam : public OsImsRadioParam
{
public:
    inline ConnectionFailedParam() :
            OsImsRadioParam(EVENT_CONNECTION_FAILED),
            m_nId(0),
            m_nFailureReason(0),
            m_nCauseCode(0),
            m_nWaitTimeMillis(0)
    {
    }
    ~ConnectionFailedParam() override = default;

    IMS_UINT32 m_nId;
    IMS_UINT32 m_nFailureReason;
    IMS_UINT32 m_nCauseCode;
    IMS_UINT32 m_nWaitTimeMillis;
};

class ConnectionSetupPreparedParam : public OsImsRadioParam
{
public:
    inline ConnectionSetupPreparedParam() :
            OsImsRadioParam(EVENT_CONNECTION_SETUP_PREPARED),
            m_nId(0)
    {
    }
    ~ConnectionSetupPreparedParam() override = default;

    IMS_UINT32 m_nId;
};

class SsacInfoParam : public OsImsRadioParam
{
public:
    inline SsacInfoParam() :
            OsImsRadioParam(EVENT_SSAC_STATE_CHANGED),
            m_nBarringFactorForVoice(100),
            m_nBarringTimeSecForVoice(0),
            m_nBarringFactorForVideo(100),
            m_nBarringTimeSecForVideo(0)
    {
    }
    ~SsacInfoParam() override = default;

    IMS_SINT32 m_nBarringFactorForVoice;
    IMS_SINT32 m_nBarringTimeSecForVoice;
    IMS_SINT32 m_nBarringFactorForVideo;
    IMS_SINT32 m_nBarringTimeSecForVideo;
};

class SccsInfoParam : public OsImsRadioParam
{
public:
    inline SccsInfoParam() :
            OsImsRadioParam(EVENT_SIMULTANEOUS_CALLING_SUPPORT_CHANGED),
            m_bIsSupported(IMS_FALSE)
    {
    }
    ~SccsInfoParam() override = default;

    IMS_BOOL m_bIsSupported;
};

static void osImsRadio_SendMessage(
        IN IThread* piThread, IN IMS_SINT32 nSlotId, IN OsImsRadioParam* pParam)
{
    if (piThread == IMS_NULL)
    {
        delete pParam;
        return;
    }

    if (!piThread->PostMessageI(IMS_MSG_RADIO, nSlotId, reinterpret_cast<IMS_UINTP>(pParam)))
    {
        delete pParam;
    }
}

PUBLIC
OsImsRadio::OsImsRadio(IN IMS_SINT32 nSlotId) :
        ImsRadio(nSlotId),
        m_nId(0),
        m_piOwnerThread(IMS_NULL),
        m_objSsacInfo(),
        m_objTrafficStartedCount(ImsMap<IMS_UINT32, IMS_UINT32>()),
        m_objConnectionListeners(ImsList<ConnectionListener*>()),
        m_objSsacListeners(ImsList<IImsRadioSsacListener*>()),
        m_objTrafficPriorityListeners(ImsList<IImsRadioTrafficPriorityListener*>())
{
    m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();

    PlatformContext::GetInstance()->GetSystem()->AddListener(
            SystemConstants::CATEGORY_RADIO, this, GetSlotId());

    IImsTraffic* piImsTraffic = ImsRadioService::GetImsRadioService()->GetImsTraffic();
    if (piImsTraffic != IMS_NULL)
    {
        piImsTraffic->AddListener(this);
    }

    m_objSsacInfo.nBarringFactorForVoice = 100;
    m_objSsacInfo.nBarringTimeSecForVoice = 0;
    m_objSsacInfo.nBarringFactorForVideo = 100;
    m_objSsacInfo.nBarringTimeSecForVideo = 0;

    m_objTrafficStartedCount.Add(TRAFFIC_TYPE_EMERGENCY, 0);
    m_objTrafficStartedCount.Add(TRAFFIC_TYPE_EMERGENCY_SMS, 0);
    m_objTrafficStartedCount.Add(TRAFFIC_TYPE_VOICE, 0);
    m_objTrafficStartedCount.Add(TRAFFIC_TYPE_VIDEO, 0);
    m_objTrafficStartedCount.Add(TRAFFIC_TYPE_SMS, 0);
    m_objTrafficStartedCount.Add(TRAFFIC_TYPE_REGISTRATION, 0);
    m_objTrafficStartedCount.Add(TRAFFIC_TYPE_UT_XCAP, 0);
}

PUBLIC VIRTUAL OsImsRadio::~OsImsRadio()
{
    m_objTrafficStartedCount.Clear();

    IImsTraffic* piImsTraffic = ImsRadioService::GetImsRadioService()->GetImsTraffic();
    if (piImsTraffic != IMS_NULL)
    {
        piImsTraffic->RemoveListener(this);
    }

    PlatformContext::GetInstance()->GetSystem()->RemoveListener(
            SystemConstants::CATEGORY_RADIO, this, GetSlotId());
}

PUBLIC VIRTUAL IMS_BOOL OsImsRadio::IsImsTrafficAllowed(IN IMS_UINT32 nTrafficType)
{
    IImsTraffic* piImsTraffic = ImsRadioService::GetImsRadioService()->GetImsTraffic();

    if (piImsTraffic != IMS_NULL)
    {
        return piImsTraffic->IsAllowed(GetSlotId(), nTrafficType);
    }

    return IMS_TRUE;
}

PUBLIC VIRTUAL void OsImsRadio::StartImsTraffic(IN IMS_UINT32 nTrafficType,
        IN IMS_UINT32 nAccessNetworkType, IN IMS_UINT32 nDirection,
        IN IImsRadioConnectionListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    IMS_SLONG nIndex = -1;

    for (IMS_UINT32 i = 0; i < m_objConnectionListeners.GetSize(); i++)
    {
        const ConnectionListener* pConnectionListener = m_objConnectionListeners.GetAt(i);
        if (pConnectionListener == IMS_NULL)
        {
            continue;
        }

        const IImsRadioConnectionListener* piCurrListener = pConnectionListener->GetListener();
        if (piCurrListener == piListener)
        {
            nIndex = i;
            break;
        }
    }

    IMS_UINT32 nId = 0;
    IMS_BOOL bFirstStartTraffic = IMS_FALSE;

    if (nIndex < 0)
    {
        IMS_UINT32 nStartedCount = m_objTrafficStartedCount.GetValue(nTrafficType);
        if (nStartedCount == 0)
        {
            bFirstStartTraffic = IMS_TRUE;
        }

        nId = GetId();
        m_objConnectionListeners.Append(new ConnectionListener(nId, nTrafficType, piListener));
        m_objTrafficStartedCount.SetValue(nTrafficType, ++nStartedCount);

        IMS_TRACE_D("OsImsRadio :: StartImsTraffic - slotId=%d, size=%d, count=%d", GetSlotId(),
                m_objConnectionListeners.GetSize(), nStartedCount);
    }
    else
    {
        const ConnectionListener* pConnectionListener = m_objConnectionListeners.GetAt(nIndex);
        if (pConnectionListener == IMS_NULL)
        {
            return;
        }

        nId = pConnectionListener->GetId();
    }

    if (PlatformContext::GetInstance()->GetSystem()->StartImsTraffic(
                nId, nTrafficType, nAccessNetworkType, nDirection, GetSlotId()) <= 0)
    {
        IMS_TRACE_I("OsImsRadio :: [%d] StartImsTraffic is failed", GetSlotId(), 0, 0);
    }

    IImsTraffic* piImsTraffic = ImsRadioService::GetImsRadioService()->GetImsTraffic();
    if (bFirstStartTraffic && piImsTraffic != IMS_NULL)
    {
        piImsTraffic->Start(GetSlotId(), nTrafficType);
    }
}

PUBLIC VIRTUAL void OsImsRadio::StopImsTraffic(IN IImsRadioConnectionListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objConnectionListeners.GetSize(); i++)
    {
        ConnectionListener* pConnectionListener = m_objConnectionListeners.GetAt(i);
        if (pConnectionListener == IMS_NULL)
        {
            continue;
        }

        const IImsRadioConnectionListener* piCurrListener = pConnectionListener->GetListener();
        if (piCurrListener == piListener)
        {
            IMS_UINT32 nId = pConnectionListener->GetId();
            IMS_UINT32 nTrafficType = pConnectionListener->GetTrafficType();
            IMS_UINT32 nStartedCount = m_objTrafficStartedCount.GetValue(nTrafficType);

            delete pConnectionListener;
            m_objConnectionListeners.RemoveAt(i);

            if (nStartedCount > 0)
            {
                m_objTrafficStartedCount.SetValue(nTrafficType, --nStartedCount);
            }

            IMS_TRACE_D("OsImsRadio :: StopImsTraffic - slotId=%d, id=%d, size=%d", GetSlotId(),
                    nId, m_objConnectionListeners.GetSize());

            PlatformContext::GetInstance()->GetSystem()->StopImsTraffic(nId, GetSlotId());

            IImsTraffic* piImsTraffic = ImsRadioService::GetImsRadioService()->GetImsTraffic();
            if (m_objTrafficStartedCount.GetValue(nTrafficType) == 0 && piImsTraffic != IMS_NULL)
            {
                piImsTraffic->Stop(GetSlotId(), nTrafficType);
            }
            break;
        }
    }
}

PUBLIC VIRTUAL void OsImsRadio::TriggerEpsFallback(IN IMS_UINT32 nEpsfbReason)
{
    if (PlatformContext::GetInstance()->GetSystem()->TriggerEpsFallback(
                nEpsfbReason, GetSlotId()) <= 0)
    {
        IMS_TRACE_I("OsImsRadio :: [%d] TriggerEpsFallback is failed", GetSlotId(), 0, 0);
    }
}

PUBLIC VIRTUAL const SsacInfo& OsImsRadio::GetSsacInfo() const
{
    return m_objSsacInfo;
}

PUBLIC VIRTUAL void OsImsRadio::AddListenerForSsac(IN IImsRadioSsacListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objSsacListeners.GetSize(); ++i)
    {
        const IImsRadioSsacListener* pTmpListener = m_objSsacListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            return;
        }
    }

    m_objSsacListeners.Append(piListener);
}

PUBLIC VIRTUAL void OsImsRadio::RemoveListenerForSsac(IN IImsRadioSsacListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objSsacListeners.GetSize(); ++i)
    {
        const IImsRadioSsacListener* pTmpListener = m_objSsacListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            m_objSsacListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC VIRTUAL void OsImsRadio::AddListenerForTrafficPriority(
        IN IImsRadioTrafficPriorityListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objTrafficPriorityListeners.GetSize(); ++i)
    {
        const IImsRadioTrafficPriorityListener* pTmpListener =
                m_objTrafficPriorityListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            return;
        }
    }

    m_objTrafficPriorityListeners.Append(piListener);
}

PUBLIC VIRTUAL void OsImsRadio::RemoveListenerForTrafficPriority(
        IN IImsRadioTrafficPriorityListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objTrafficPriorityListeners.GetSize(); ++i)
    {
        const IImsRadioTrafficPriorityListener* pTmpListener =
                m_objTrafficPriorityListeners.GetAt(i);

        if (pTmpListener == piListener)
        {
            m_objTrafficPriorityListeners.RemoveAt(i);
            return;
        }
    }
}

PROTECTED VIRTUAL void OsImsRadio::DispatchServiceMessage(IN IMS_UINTP /* nWparam */
        ,
        IN IMS_UINTP nLparam)
{
    OsImsRadioParam* pImsRadioParam = reinterpret_cast<OsImsRadioParam*>(nLparam);

    if (pImsRadioParam != IMS_NULL)
    {
        IMS_TRACE_D("OsImsRadio :: DispatchServiceMessage - slotId=%d, event=%s, lp=%" PFLS_u,
                GetSlotId(), EventToString(pImsRadioParam->m_nEvent), nLparam);

        if (pImsRadioParam->m_nEvent == OsImsRadioParam::EVENT_CONNECTION_SETUP_PREPARED)
        {
            const ConnectionSetupPreparedParam* pParam =
                    reinterpret_cast<ConnectionSetupPreparedParam*>(pImsRadioParam);

            NotifyConnectionSetupPrepared(pParam->m_nId);
        }
        else if (pImsRadioParam->m_nEvent == OsImsRadioParam::EVENT_CONNECTION_FAILED)
        {
            const ConnectionFailedParam* pParam =
                    reinterpret_cast<ConnectionFailedParam*>(pImsRadioParam);

            NotifyConnectionFailed(pParam->m_nId, pParam->m_nFailureReason, pParam->m_nCauseCode,
                    pParam->m_nWaitTimeMillis);
        }
        else if (pImsRadioParam->m_nEvent == OsImsRadioParam::EVENT_SSAC_STATE_CHANGED)
        {
            const SsacInfoParam* pParam = reinterpret_cast<SsacInfoParam*>(pImsRadioParam);

            NotifySsacInfoChanged(pParam->m_nBarringFactorForVoice,
                    pParam->m_nBarringTimeSecForVoice, pParam->m_nBarringFactorForVideo,
                    pParam->m_nBarringTimeSecForVideo);
        }
        else if (pImsRadioParam->m_nEvent ==
                OsImsRadioParam::EVENT_SIMULTANEOUS_CALLING_SUPPORT_CHANGED)
        {
            const SccsInfoParam* pParam = reinterpret_cast<SccsInfoParam*>(pImsRadioParam);

            NotifySimultaneousCallingSupportChanged(pParam->m_bIsSupported);
        }

        delete pImsRadioParam;
    }
}

PROTECTED VIRTUAL void OsImsRadio::ImsTraffic_OnPriorityChanged()
{
    IMS_TRACE_D("OsImsRadio :: ImsTraffic_OnPriorityChanged - slotId=%d", GetSlotId(), 0, 0);

    for (IMS_UINT32 i = 0; i < m_objTrafficPriorityListeners.GetSize(); ++i)
    {
        IImsRadioTrafficPriorityListener* piListener = m_objTrafficPriorityListeners.GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }

        piListener->ImsRadio_OnTrafficPriorityChanged();
    }
}

PROTECTED VIRTUAL void OsImsRadio::System_NotifyEvent(
        IN IMS_UINT32 nEvent, IN IMS_UINTP /* nWParam */, IN IMS_UINTP nLParam)
{
    IMS_TRACE_D("OsImsRadio :: System_NotifyEvent - slotId=%d, event=%s", GetSlotId(),
            EventToString(nEvent), 0);

    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(nLParam);

    if (pParcel == IMS_NULL)
    {
        return;
    }

    switch (nEvent)
    {
        case OsImsRadioParam::EVENT_CONNECTION_FAILED:
        {
            ConnectionFailedParam* pParam = new ConnectionFailedParam();
            pParam->m_nId = pParcel->readInt32();
            pParam->m_nFailureReason = pParcel->readInt32();
            pParam->m_nCauseCode = pParcel->readInt32();
            pParam->m_nWaitTimeMillis = pParcel->readInt32();

            osImsRadio_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        case OsImsRadioParam::EVENT_CONNECTION_SETUP_PREPARED:
        {
            ConnectionSetupPreparedParam* pParam = new ConnectionSetupPreparedParam();
            pParam->m_nId = pParcel->readInt32();

            osImsRadio_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        case OsImsRadioParam::EVENT_SSAC_STATE_CHANGED:
        {
            SsacInfoParam* pParam = new SsacInfoParam();
            pParam->m_nBarringFactorForVoice = pParcel->readInt32();
            pParam->m_nBarringTimeSecForVoice = pParcel->readInt32();
            pParam->m_nBarringFactorForVideo = pParcel->readInt32();
            pParam->m_nBarringTimeSecForVideo = pParcel->readInt32();

            osImsRadio_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        case OsImsRadioParam::EVENT_SIMULTANEOUS_CALLING_SUPPORT_CHANGED:
        {
            SccsInfoParam* pParam = new SccsInfoParam();
            pParam->m_bIsSupported = (pParcel->readInt32() > 0) ? IMS_TRUE : IMS_FALSE;

            osImsRadio_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        default:
        {
            // no-op
            break;
        }
    }
}

PRIVATE IImsRadioConnectionListener* OsImsRadio::GetConnectionListener(IN IMS_UINT32 nId)
{
    for (IMS_UINT32 i = 0; i < m_objConnectionListeners.GetSize(); i++)
    {
        const ConnectionListener* pConnectionListener = m_objConnectionListeners.GetAt(i);
        if (pConnectionListener == IMS_NULL)
        {
            continue;
        }

        if (nId == pConnectionListener->GetId())
        {
            return pConnectionListener->GetListener();
        }
    }

    return IMS_NULL;
}

PRIVATE IMS_UINT32 OsImsRadio::GetId()
{
    if (m_nId == ID_MAX)
    {
        m_nId = 0;
    }

    return ++m_nId;
}

PRIVATE void OsImsRadio::NotifyConnectionSetupPrepared(IN IMS_UINT32 nId)
{
    IImsRadioConnectionListener* piListener = GetConnectionListener(nId);

    if (piListener != IMS_NULL)
    {
        piListener->ImsRadio_OnConnectionSetupPrepared();
    }
}

PRIVATE void OsImsRadio::NotifyConnectionFailed(IN IMS_UINT32 nId, IN IMS_UINT32 nFailureReason,
        IN IMS_UINT32 nCauseCode, IN IMS_UINT32 nWaitTimeMillis)
{
    IImsRadioConnectionListener* piListener = GetConnectionListener(nId);

    if (piListener != IMS_NULL)
    {
        piListener->ImsRadio_OnConnectionFailed(nFailureReason, nCauseCode, nWaitTimeMillis);
    }
}

PRIVATE void OsImsRadio::NotifySsacInfoChanged(IN IMS_SINT32 nFactorForVoice,
        IN IMS_SINT32 nTimeSecForVoice, IN IMS_SINT32 nFactorForVideo,
        IN IMS_SINT32 nTimeSecForVideo)
{
    AString strSsacForLog;
    strSsacForLog.Sprintf("slotId=%d, voice=%d,%d , video=%d,%d", GetSlotId(), nFactorForVoice,
            nTimeSecForVoice, nFactorForVideo, nTimeSecForVideo);

    IMS_TRACE_D("OsImsRadio :: NotifySsacInfoChanged -  %s", strSsacForLog.GetStr(), 0, 0);

    m_objSsacInfo.nBarringFactorForVoice = nFactorForVoice;
    m_objSsacInfo.nBarringTimeSecForVoice = nTimeSecForVoice;
    m_objSsacInfo.nBarringFactorForVideo = nFactorForVideo;
    m_objSsacInfo.nBarringTimeSecForVideo = nTimeSecForVideo;

    for (IMS_UINT32 i = 0; i < m_objSsacListeners.GetSize(); ++i)
    {
        IImsRadioSsacListener* pTmpListener = m_objSsacListeners.GetAt(i);

        if (pTmpListener == IMS_NULL)
        {
            continue;
        }

        pTmpListener->ImsRadio_OnSsacChanged(m_objSsacInfo);
    }
}

PRIVATE void OsImsRadio::NotifySimultaneousCallingSupportChanged(IN IMS_BOOL bSupported)
{
    IImsTraffic* piImsTraffic = ImsRadioService::GetImsRadioService()->GetImsTraffic();

    if (piImsTraffic != IMS_NULL)
    {
        piImsTraffic->SetSimultaneousCallingSupported(GetSlotId(), bSupported);
    }
}

PRIVATE GLOBAL const IMS_CHAR* OsImsRadio::EventToString(IN IMS_UINT32 nEvent)
{
    switch (nEvent)
    {
        case OsImsRadioParam::EVENT_CONNECTION_FAILED:
            return "EVENT_CONNECTION_FAILED";

        case OsImsRadioParam::EVENT_CONNECTION_SETUP_PREPARED:
            return "EVENT_CONNECTION_SETUP_PREPARED";

        case OsImsRadioParam::EVENT_SSAC_STATE_CHANGED:
            return "EVENT_SSAC_STATE_CHANGED";

        case OsImsRadioParam::EVENT_SIMULTANEOUS_CALLING_SUPPORT_CHANGED:
            return "EVENT_SIMULTANEOUS_CALLING_SUPPORT_CHANGED";

        default:
            return "__INVALID__";
    }
}
