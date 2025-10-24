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
#include <binder/Parcel.h>
#include <utils/String8.h>

#include "IDigestAkaListener.h"
#include "IIsimListener.h"
#include "ImsMessageDef.h"
#include "OsUtil.h"
#include "PlatformContext.h"
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "device/OsIsim.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_IPL__;

// ISIM system interface parameters for event notification
class OsIsimParam
{
public:
    inline explicit OsIsimParam(IN IMS_SINT32 nType) :
            m_nType(nType)
    {
    }
    inline virtual ~OsIsimParam() {}

public:
    enum
    {
        TYPE_STATE = 1,
        TYPE_AUTHENTICATION
    };

    IMS_SINT32 m_nType;
};

class OsIsimStateParam : public OsIsimParam
{
public:
    inline OsIsimStateParam() :
            OsIsimParam(TYPE_STATE),
            m_nState(0)
    {
    }
    ~OsIsimStateParam() override = default;

public:
    enum
    {
        STATE_NOT_PRESENT = 0,
        STATE_NOT_READY,
        STATE_LOADED,
        STATE_REFRESH_STARTED,
        STATE_REFRESH_COMPLETED,
        // It's to identify whether SIM agent is stopped.
        // If SIM agent is stopped by any reasons, it will be delivered to Native ISIM module.
        //   e.g. SIM Removed
        STATE_SIM_REMOVED
    };

    IMS_SINT32 m_nState;
};

class OsIsimAuthResponseParam : public OsIsimParam
{
public:
    inline OsIsimAuthResponseParam() :
            OsIsimParam(TYPE_AUTHENTICATION),
            m_nOwner(0)
    {
    }
    ~OsIsimAuthResponseParam() override = default;

    OsIsimAuthResponseParam(IN const OsIsimAuthResponseParam&) = delete;
    OsIsimAuthResponseParam& operator=(IN const OsIsimAuthResponseParam&) = delete;

public:
    IMS_SINTP m_nOwner;
    ByteArray m_objResponse;
};

class OsIsimStateMap
{
public:
    IMS_SINT32 nState;
    const IMS_CHAR* pszState;
};

static const OsIsimStateMap SIM_STATE_MAP[] = {
        {OsIsimStateParam::STATE_NOT_PRESENT,       "NOT_PRESENT"      },
        {OsIsimStateParam::STATE_NOT_READY,         "NOT_READY"        },
        {OsIsimStateParam::STATE_LOADED,            "LOADED"           },
        {OsIsimStateParam::STATE_REFRESH_STARTED,   "REFRESH_STARTED"  },
        {OsIsimStateParam::STATE_REFRESH_COMPLETED, "REFRESH_COMPLETED"},
        {OsIsimStateParam::STATE_SIM_REMOVED,       "SIM_REMOVED"      },
        {(-1),                                      IMS_NULL           }
};

static OsIsim* osIsim_GetInstance(IN IMS_SINT32 nSlotId)
{
    return DYNAMIC_CAST(OsIsim*, PhoneInfoService::GetPhoneInfoService()->GetIsim(nSlotId));
}

static void osIsim_HandleAuthResponse(IN IMS_SINT32 nSlotId, IN OsIsimAuthResponseParam* pParam)
{
    OsIsim* pIsim = osIsim_GetInstance(nSlotId);
    OsIsimDigestAka* pDigestAka = reinterpret_cast<OsIsimDigestAka*>(pParam->m_nOwner);

    IMS_TRACE_I("ISIM: AuthResponse - owner=%" PFLS_x ", res_len=%d", pParam->m_nOwner,
            pParam->m_objResponse.GetLength(), 0);

    if (!pIsim->IsDigestAkaPresent(pDigestAka))
    {
        IMS_TRACE_D("ISIM: Digest AKA (%p) is not present; ignore...", pDigestAka, 0, 0);
        return;
    }

    if (pParam->m_objResponse.GetLength() < 1)
    {
        pDigestAka->NotifyMacFailed();
        return;
    }

    if (OsUtil::GetInstance()->IsDebugMode())
    {
        const ByteArray& objRes = pParam->m_objResponse;
        AString strHex;
        AString strBuffer = AString::ConstEmpty();

        for (IMS_SINT32 i = 0; i < objRes.GetLength(); ++i)
        {
            strHex.Sprintf("%02X ", objRes[i]);
            strBuffer.Append(strHex);

            if ((i != 0) && (((i + 1) % 16) == 0))
            {
                IMS_TRACE_D("Authentication: %s", strBuffer.GetStr(), 0, 0);
                strBuffer = AString::ConstEmpty();
            }
        }

        IMS_TRACE_D("Authentication: %s", strBuffer.GetStr(), 0, 0);
    }

    IMS_SINT32 nPos = 0;
    const IMS_BYTE* pbyData = pParam->m_objResponse.GetData();

    if (pbyData[nPos] == 0xDB)
    {
        // RES, CK, IK
        ByteArray objRes;
        ByteArray objCk;
        ByteArray objIk;

        // RES
        ++nPos;
        IMS_SINT32 nTmpLen = pbyData[nPos];

        ++nPos;
        objRes.Append(&pbyData[nPos], nTmpLen);

        nPos += nTmpLen;

        // CK
        nTmpLen = pbyData[nPos];

        ++nPos;
        objCk.Append(&pbyData[nPos], nTmpLen);

        nPos += nTmpLen;

        // IK
        nTmpLen = pbyData[nPos];

        ++nPos;
        objIk.Append(&pbyData[nPos], nTmpLen);

        nPos += nTmpLen;

        pDigestAka->NotifyResponse(objRes, objIk, objCk);
    }
    else if (pbyData[nPos] == 0xDC)
    {
        // AUTS
        ByteArray objAuts;

        ++nPos;
        objAuts.Append(&pbyData[nPos + 1], pbyData[nPos]);

        nPos += pbyData[nPos];

        pDigestAka->NotifyAutsFailed(objAuts);
    }
    else
    {
        pDigestAka->NotifyMacFailed();
    }

    if (pParam->m_objResponse.GetLength() < nPos)
    {
        IMS_TRACE_D("ISIM: Invalid AuthRes(res-len=%d, len-from-data=%d)",
                pParam->m_objResponse.GetLength(), nPos, 0);
    }
}

static void osIsim_HandleIsimState(IN IMS_SINT32 nSlotId, IN const OsIsimStateParam* pParam)
{
    OsIsim* pIsim = osIsim_GetInstance(nSlotId);
    IMS_SINT32 nState = pIsim->GetState();

    IMS_TRACE_I("ISIM: HandleIsimState - state=%s, isimState=%s", OsIsim::StateToString(nState),
            OsIsim::SystemStateToString(pParam->m_nState), 0);

    switch (pParam->m_nState)
    {
        case OsIsimStateParam::STATE_NOT_PRESENT:
        {
            pIsim->SetState(IIsim::STATE_NOT_PRESENT);
            break;
        }
        case OsIsimStateParam::STATE_NOT_READY:
        {
            if (nState == IIsim::STATE_LOADED || nState == IIsim::STATE_NOT_PRESENT)
            {
                pIsim->SetState(IIsim::STATE_REFRESHING);
            }
            break;
        }
        case OsIsimStateParam::STATE_LOADED:
        {
            pIsim->SetState(IIsim::STATE_LOADED);
            break;
        }
        case OsIsimStateParam::STATE_REFRESH_STARTED:
        {
            pIsim->SetState(IIsim::STATE_REFRESHING);
            break;
        }
        case OsIsimStateParam::STATE_REFRESH_COMPLETED:
        {
            pIsim->SetState(IIsim::STATE_LOADED);
            break;
        }
        case OsIsimStateParam::STATE_SIM_REMOVED:
        {
            pIsim->SetState(IIsim::STATE_IDLE);
            break;
        }
        default:
        {
            // no-op
            break;
        }
    }
}

static void osIsim_HandleIsimEvent(IN IMS_SINT32 nSlotId, IN OsIsimParam* pParam)
{
    if (pParam == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("ISIM: HandleIsimEvent - slotId=%d, type=%d", nSlotId, pParam->m_nType, 0);

    switch (pParam->m_nType)
    {
        case OsIsimParam::TYPE_STATE:
            osIsim_HandleIsimState(nSlotId, DYNAMIC_CAST(OsIsimStateParam*, pParam));
            break;
        case OsIsimParam::TYPE_AUTHENTICATION:
            osIsim_HandleAuthResponse(nSlotId, DYNAMIC_CAST(OsIsimAuthResponseParam*, pParam));
            break;
        default:
            break;
    }

    delete pParam;
}

static void osIsim_SendMessage(IN IThread* piThread, IN IMS_SINT32 nSlotId, IN OsIsimParam* pParam)
{
    if (piThread == IMS_NULL)
    {
        delete pParam;
        return;
    }

    if (!piThread->PostMessageI(IMS_MSG_ISIM, nSlotId, reinterpret_cast<IMS_UINTP>(pParam)))
    {
        delete pParam;
    }
}

PUBLIC
OsIsimDigestAka::OsIsimDigestAka(IN ImsIsim* pIsim) :
        m_pIsim(pIsim),
        m_pDigestAkaListener(IMS_NULL)
{
}

PUBLIC VIRTUAL OsIsimDigestAka::~OsIsimDigestAka() {}

PUBLIC VIRTUAL void OsIsimDigestAka::Destroy()
{
    m_pDigestAkaListener = IMS_NULL;
    osIsim_GetInstance(m_pIsim->GetSlotId())->DestroyDigestAka(this);
}

PUBLIC VIRTUAL IMS_RESULT OsIsimDigestAka::GetAuthResponse(IN const ByteArray& objChallenge)
{
    if (OsUtil::GetInstance()->IsDebugMode())
    {
        AString strHex;
        AString strBuffer = AString::ConstEmpty();

        for (IMS_SINT32 i = 0; i < objChallenge.GetLength(); ++i)
        {
            strHex.Sprintf("%02X ", objChallenge[i]);
            strBuffer.Append(strHex);

            if (i == 16)
            {
                IMS_TRACE_D("Challenge: %s", strBuffer.GetStr(), 0, 0);
                strBuffer = AString::ConstEmpty();
            }
        }

        IMS_TRACE_D("Challenge: %s", strBuffer.GetStr(), 0, 0);
    }

    AString strNonce;
    strNonce.Attach(
            reinterpret_cast<const IMS_CHAR*>(objChallenge.GetData()), objChallenge.GetLength());
    strNonce = strNonce.ToBase64();

    if (PlatformContext::GetInstance()->GetSystem()->RequestIsimAuthentication(
                strNonce, reinterpret_cast<IMS_SINTP>(this), m_pIsim->GetSlotId()) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL void OsIsimDigestAka::SetListener(IN IDigestAkaListener* piListener)
{
    m_pDigestAkaListener = piListener;
}

PUBLIC
void OsIsimDigestAka::NotifyAutsFailed(IN const ByteArray& objAuts)
{
    if (m_pDigestAkaListener == IMS_NULL)
    {
        IMS_TRACE_D("DigestAkaListener is null", 0, 0, 0);
        return;
    }

    m_pDigestAkaListener->DigestAka_OnAutsFailed(objAuts);
}

PUBLIC
void OsIsimDigestAka::NotifyMacFailed()
{
    if (m_pDigestAkaListener == IMS_NULL)
    {
        IMS_TRACE_D("DigestAkaListener is null", 0, 0, 0);
        return;
    }

    m_pDigestAkaListener->DigestAka_OnMacFailed();
}

PUBLIC
void OsIsimDigestAka::NotifyResponse(
        IN const ByteArray& objRes, IN const ByteArray& objIk, IN const ByteArray& objCk)
{
    if (m_pDigestAkaListener == IMS_NULL)
    {
        IMS_TRACE_D("DigestAkaListener is null", 0, 0, 0);
        return;
    }

    m_pDigestAkaListener->DigestAka_OnResponse(objRes, objIk, objCk);
}

PUBLIC
OsIsim::OsIsim(IN IMS_SINT32 nSlotId) :
        ImsIsim(nSlotId),
        m_objIsimListeners(ImsList<IIsimListener*>()),
        m_objDigestAkas(ImsList<OsIsimDigestAka*>()),
        m_piOwnerThread(IMS_NULL),
        m_nState(STATE_IDLE)
{
    IMS_TRACE_D("C-ISIM%02d", nSlotId, 0, 0);

    PlatformContext::GetInstance()->GetSystem()->AddListener(
            SystemConstants::CATEGORY_ISIM, this, GetSlotId());

    m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();
}

PUBLIC VIRTUAL OsIsim::~OsIsim()
{
    IMS_TRACE_D("D-ISIM%02d", GetSlotId(), 0, 0);

    PlatformContext::GetInstance()->GetSystem()->RemoveListener(
            SystemConstants::CATEGORY_ISIM, this, GetSlotId());
}

PUBLIC VIRTUAL void OsIsim::DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam)
{
    IMS_TRACE_I("ISIM: DispatchServiceMessage - slotId=%d, wp=%" PFLS_u ", lp=%" PFLS_u,
            GetSlotId(), nWparam, nLparam);

    osIsim_HandleIsimEvent(GetSlotId(), reinterpret_cast<OsIsimParam*>(nLparam));
}

PUBLIC VIRTUAL IDigestAka* OsIsim::CreateDigestAka()
{
    OsIsimDigestAka* pDigestAka = new OsIsimDigestAka(this);

    if (pDigestAka == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!m_objDigestAkas.Append(pDigestAka))
    {
        delete pDigestAka;
        return IMS_NULL;
    }

    IMS_TRACE_D("ISIM: CreateDigestAKA(%p, %d)", pDigestAka, GetSlotId(), 0);

    return pDigestAka;
}

PUBLIC VIRTUAL AString OsIsim::GetHomeDomainName() const
{
    AStringArray objRecord =
            PlatformContext::GetInstance()->GetSystem()->GetIsimRecord(EF_ID_DOMAIN, GetSlotId());
    return objRecord.IsEmpty() ? AString::ConstNull() : objRecord.GetElementAt(0);
}

PUBLIC VIRTUAL AString OsIsim::GetImpi() const
{
    AStringArray objRecord =
            PlatformContext::GetInstance()->GetSystem()->GetIsimRecord(EF_ID_IMPI, GetSlotId());
    return objRecord.IsEmpty() ? AString::ConstNull() : objRecord.GetElementAt(0);
}

PUBLIC VIRTUAL AStringArray OsIsim::GetImpu() const
{
    return PlatformContext::GetInstance()->GetSystem()->GetIsimRecord(EF_ID_IMPU, GetSlotId());
}

PUBLIC VIRTUAL AStringArray OsIsim::GetPcscf() const
{
    return PlatformContext::GetInstance()->GetSystem()->GetIsimRecord(EF_ID_PCSCF, GetSlotId());
}

PUBLIC VIRTUAL void OsIsim::AddListener(IN IIsimListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objIsimListeners.GetSize(); ++i)
    {
        const IIsimListener* piTmpListener = m_objIsimListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            return;
        }
    }

    m_objIsimListeners.Append(piListener);
}

PUBLIC VIRTUAL void OsIsim::RemoveListener(IN IIsimListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objIsimListeners.GetSize(); ++i)
    {
        const IIsimListener* piTmpListener = m_objIsimListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            m_objIsimListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC VIRTUAL void OsIsim::Init()
{
    // In case that ISIM STATUS is LOADED or NOT_PRESENT
    IMS_SINT32 nIsimState = ConvertSystemStateToEnum(
            PlatformContext::GetInstance()->GetSystem()->GetIsimState(GetSlotId()));

    if (nIsimState == OsIsimStateParam::STATE_LOADED ||
            nIsimState == OsIsimStateParam::STATE_NOT_PRESENT)
    {
        OsIsimStateParam* pParam = new OsIsimStateParam();
        pParam->m_nState = nIsimState;
        osIsim_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
    }
}

PUBLIC VIRTUAL void OsIsim::Release()
{
    IMS_TRACE_I("ISIM: Release", 0, 0, 0);
    m_nState = IIsim::STATE_IDLE;
}

PUBLIC VIRTUAL void OsIsim::System_NotifyEvent(
        IN IMS_UINT32 nEvent, IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    (void)nWParam;

    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(nLParam);

    if (pParcel == IMS_NULL)
    {
        return;
    }

    switch (nEvent)
    {
        case NOTIFICATION_ISIM_STATE_CHANGED:
        {
            // SIM state is notified when receiving the intent
            android::String8 strTmpState(pParcel->readString16());
            AString strState(strTmpState.c_str(), strTmpState.length());

            IMS_TRACE_I("ISIM: IsimStateChanged (%s)", strState.GetStr(), 0, 0);

            OsIsimStateParam* pParam = new OsIsimStateParam();

            pParam->m_nType = OsIsimParam::TYPE_STATE;
            pParam->m_nState = ConvertSystemStateToEnum(strState);

            osIsim_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        case NOTIFICATION_ISIM_AUTH:
        {
            android::String8 strResponse(pParcel->readString16());
            AString strAuth(strResponse.c_str());
            strAuth = AString::FromBase64(strAuth);

            OsIsimAuthResponseParam* pParam = new OsIsimAuthResponseParam();

            pParam->m_nType = OsIsimParam::TYPE_AUTHENTICATION;
            pParam->m_nOwner = INT64_TO_SINTP(pParcel->readInt64());
            pParam->m_objResponse.Append(reinterpret_cast<const IMS_BYTE*>(strAuth.GetStr()),
                    static_cast<IMS_SINT32>(strAuth.GetLength()));

            IMS_TRACE_D("ISIM: Auth - owner=%" PFLS_x ", res=%s", pParam->m_nOwner,
                    strResponse.c_str(), 0);

            osIsim_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        default:
        {
            IMS_TRACE_D("ISIM: Invalid event (%d)", nEvent, 0, 0);
            break;
        }
    }
}

PUBLIC
void OsIsim::NotifyStateChanged(IN IMS_SINT32 nState)
{
    for (IMS_UINT32 i = 0; i < m_objIsimListeners.GetSize(); ++i)
    {
        IIsimListener* piListener = m_objIsimListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->Isim_OnStateChanged(nState);
        }
    }
}

PUBLIC
void OsIsim::SetState(IN IMS_SINT32 nState)
{
    if (m_nState == nState)
    {
        return;
    }

    IMS_TRACE_I("ISIM: %s >> %s", StateToString(m_nState), StateToString(nState), 0);

    m_nState = nState;
    NotifyStateChanged(m_nState);
}

PUBLIC
void OsIsim::DestroyDigestAka(IN OsIsimDigestAka* pDigestAka)
{
    if (pDigestAka == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objDigestAkas.GetSize(); ++i)
    {
        const OsIsimDigestAka* pTmp = m_objDigestAkas.GetAt(i);

        if (pDigestAka == pTmp)
        {
            m_objDigestAkas.RemoveAt(i);
            break;
        }
    }

    IMS_TRACE_D("ISIM: DestroyDigestAKA(%p)", pDigestAka, 0, 0);

    delete pDigestAka;
}

PUBLIC
IMS_BOOL OsIsim::IsDigestAkaPresent(IN const OsIsimDigestAka* pDigestAka)
{
    if (pDigestAka == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objDigestAkas.GetSize(); ++i)
    {
        const OsIsimDigestAka* pTmp = m_objDigestAkas.GetAt(i);

        if (pDigestAka == pTmp)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PUBLIC GLOBAL IMS_SINT32 OsIsim::ConvertSystemStateToEnum(IN const AString& strState)
{
    const IMS_SINT32 nCount = (sizeof(SIM_STATE_MAP) / sizeof(SIM_STATE_MAP[0])) - 1;

    for (IMS_SINT32 i = 0; i < nCount; ++i)
    {
        if (SIM_STATE_MAP[i].pszState == IMS_NULL)
        {
            continue;
        }

        if (strState.Equals(SIM_STATE_MAP[i].pszState))
        {
            return SIM_STATE_MAP[i].nState;
        }
    }

    return OsIsimStateParam::STATE_NOT_READY;
}

PUBLIC GLOBAL const IMS_CHAR* OsIsim::SystemStateToString(IN IMS_SINT32 nSystemState)
{
    const IMS_SINT32 nCount = (sizeof(SIM_STATE_MAP) / sizeof(SIM_STATE_MAP[0])) - 1;

    for (IMS_SINT32 i = 0; i < nCount; ++i)
    {
        if (SIM_STATE_MAP[i].nState == nSystemState)
        {
            return SIM_STATE_MAP[i].pszState != IMS_NULL ? SIM_STATE_MAP[i].pszState : "UNKNOWN";
        }
    }

    return "UNKNOWN";
}

PUBLIC GLOBAL const IMS_CHAR* OsIsim::StateToString(IN IMS_SINT32 nState)
{
    switch (nState)
    {
        case STATE_NOT_PRESENT:
            return "NOT_PRESENT";
        case STATE_LOADED:
            return "LOADED";
        case STATE_REFRESHING:
            return "REFRESHING";
        case STATE_IDLE:
        default:
            return "IDLE";
    }
}
