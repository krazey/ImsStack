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
#include "ImsMessageDef.h"
#include "OsUtil.h"
#include "PlatformContext.h"
#include "ServicePhoneInfo.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "device/OsUsim.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_IPL__;

// USIM system interface parameters for event notification
class OsUsimParam
{
public:
    inline explicit OsUsimParam(IN IMS_SINT32 nType = 0) :
            m_nType(nType)
    {
    }
    inline virtual ~OsUsimParam() {}

public:
    enum
    {
        TYPE_AUTHENTICATION,
    };

    IMS_SINT32 m_nType;
};

class OsUsimAuthResponseParam : public OsUsimParam
{
public:
    inline OsUsimAuthResponseParam() :
            OsUsimParam(TYPE_AUTHENTICATION),
            m_nOwner(0),
            m_objResponse(ByteArray::ConstNull())
    {
    }
    ~OsUsimAuthResponseParam() override = default;

public:
    IMS_SINTP m_nOwner;
    ByteArray m_objResponse;
};

static OsUsim* osUsim_GetInstance(IN IMS_SINT32 nSlotId)
{
    return DYNAMIC_CAST(OsUsim*, PhoneInfoService::GetPhoneInfoService()->GetUsim(nSlotId));
}

static void osUsim_HandleAuthResponse(IN IMS_SINT32 nSlotId, IN OsUsimAuthResponseParam* pParam)
{
    OsUsim* pUsim = osUsim_GetInstance(nSlotId);
    OsUsimDigestAka* pDigestAka = reinterpret_cast<OsUsimDigestAka*>(pParam->m_nOwner);

    IMS_TRACE_I("USIM :: AuthResponse - owner=%" PFLS_x ", res_len=%d", pParam->m_nOwner,
            pParam->m_objResponse.GetLength(), 0);

    if (!pUsim->IsDigestAkaPresent(pDigestAka))
    {
        IMS_TRACE_D("USIM :: DigestAKA(%p) is not present", pDigestAka, 0, 0);
        return;
    }

    pDigestAka->OnAuthResponseReceived(pParam->m_objResponse);
}

static void osUsim_HandleUsimEvent(IN IMS_SINT32 nSlotId, IN OsUsimParam* pParam)
{
    if (pParam == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("USIM :: Event - slotId=%d, type=%d", nSlotId, pParam->m_nType, 0);

    switch (pParam->m_nType)
    {
        case OsUsimParam::TYPE_AUTHENTICATION:
            osUsim_HandleAuthResponse(nSlotId, DYNAMIC_CAST(OsUsimAuthResponseParam*, pParam));
            break;
        default:
            break;
    }

    delete pParam;
}

static void osUsim_SendMessage(IN IThread* piThread, IN IMS_SINT32 nSlotId, IN OsUsimParam* pParam)
{
    if (piThread == IMS_NULL)
    {
        delete pParam;
        return;
    }

    if (!piThread->PostMessageI(IMS_MSG_USIM, nSlotId, reinterpret_cast<IMS_UINTP>(pParam)))
    {
        delete pParam;
    }
}

PUBLIC
OsUsimDigestAka::OsUsimDigestAka(IN ImsUsim* pUsim) :
        m_pUsim(pUsim),
        m_objAuthResponse(ByteArray::ConstNull()),
        m_piListener(IMS_NULL)
{
}

PUBLIC VIRTUAL OsUsimDigestAka::~OsUsimDigestAka() {}

PUBLIC
void OsUsimDigestAka::OnAuthResponseReceived(IN const ByteArray& objAuthRes)
{
    if (objAuthRes.GetLength() <= 0)
    {
        IMS_TRACE_D("No auth response; MAC failure", 0, 0, 0);

        if (m_piListener != IMS_NULL)
        {
            m_piListener->DigestAka_OnMacFailed();
        }

        return;
    }

    const IMS_BYTE* pbyAuthRes = objAuthRes.GetData();

    if (OsUtil::GetInstance()->IsDebugMode())
    {
        AString strHex;
        AString strBuffer = AString::ConstEmpty();

        for (IMS_SINT32 i = 0; i < objAuthRes.GetLength(); i++)
        {
            strHex.Sprintf("%02X ", pbyAuthRes[i]);
            strBuffer.Append(strHex);

            if ((i != 0) && (((i + 1) % 16) == 0))
            {
                IMS_TRACE_D("USIM :: Authentication: %s", strBuffer.GetStr(), 0, 0);
                strBuffer = AString::ConstEmpty();
            }
        }

        IMS_TRACE_D("USIM :: Authentication: %s", strBuffer.GetStr(), 0, 0);
    }

    if (m_piListener == IMS_NULL)
    {
        IMS_TRACE_E(0, "IDigestAkaListener is null", 0, 0, 0);
        return;
    }

    IMS_SINT32 nPos = 0;

    if (pbyAuthRes[nPos] == 0xDB)
    {
        // RES, CK, IK
        ByteArray objRes;
        ByteArray objCk;
        ByteArray objIk;

        // RES
        ++nPos;
        IMS_SINT32 nTmpLen = pbyAuthRes[nPos];

        ++nPos;
        objRes.Append(&pbyAuthRes[nPos], nTmpLen);

        nPos += nTmpLen;

        // CK
        nTmpLen = pbyAuthRes[nPos];

        ++nPos;
        objCk.Append(&pbyAuthRes[nPos], nTmpLen);

        nPos += nTmpLen;

        // IK
        nTmpLen = pbyAuthRes[nPos];

        ++nPos;
        objIk.Append(&pbyAuthRes[nPos], nTmpLen);

        m_piListener->DigestAka_OnResponse(objRes, objIk, objCk);
    }
    else if (pbyAuthRes[nPos] == 0xDC)
    {
        // AUTS
        ByteArray objAuts;

        ++nPos;
        objAuts.Append(&pbyAuthRes[nPos + 1], pbyAuthRes[nPos]);

        m_piListener->DigestAka_OnAutsFailed(objAuts);
    }
    else
    {
        m_piListener->DigestAka_OnMacFailed();
    }
}

PROTECTED VIRTUAL void OsUsimDigestAka::Destroy()
{
    m_piListener = IMS_NULL;

    osUsim_GetInstance(m_pUsim->GetSlotId())->DestroyDigestAka(this);
}

PROTECTED VIRTUAL IMS_RESULT OsUsimDigestAka::GetAuthResponse(IN const ByteArray& objChallenge)
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
                IMS_TRACE_D("USIM :: Challenge: %s", strBuffer.GetStr(), 0, 0);
                strBuffer = AString::ConstEmpty();
            }
        }

        IMS_TRACE_D("USIM :: Challenge: %s", strBuffer.GetStr(), 0, 0);
    }

    AString strNonce = AString::ConstNull();
    strNonce.Attach(
            reinterpret_cast<const IMS_CHAR*>(objChallenge.GetData()), objChallenge.GetLength());
    strNonce = strNonce.ToBase64();

    if (PlatformContext::GetInstance()->GetSystem()->RequestUsimAuthentication(
                strNonce, reinterpret_cast<IMS_SINTP>(this), m_pUsim->GetSlotId()) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PROTECTED VIRTUAL void OsUsimDigestAka::SetListener(IN IDigestAkaListener* piListener)
{
    m_piListener = piListener;
}

PUBLIC
OsUsim::OsUsim(IN IMS_SINT32 nSlotId) :
        ImsUsim(nSlotId),
        m_objDigestAkas(ImsList<OsUsimDigestAka*>()),
        m_piOwnerThread(IMS_NULL)
{
    IMS_TRACE_D("Constructor :: USIM%02d", nSlotId, 0, 0);

    PlatformContext::GetInstance()->GetSystem()->AddListener(
            SystemConstants::CATEGORY_USIM, this, GetSlotId());

    m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();
}

PUBLIC VIRTUAL OsUsim::~OsUsim()
{
    IMS_TRACE_D("Destructor :: USIM%02d", GetSlotId(), 0, 0);

    PlatformContext::GetInstance()->GetSystem()->RemoveListener(
            SystemConstants::CATEGORY_USIM, this, GetSlotId());
}

PUBLIC
void OsUsim::DestroyDigestAka(IN OsUsimDigestAka* pDigestAka)
{
    if (pDigestAka == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objDigestAkas.GetSize(); ++i)
    {
        const OsUsimDigestAka* pTmp = m_objDigestAkas.GetAt(i);

        if (pDigestAka == pTmp)
        {
            m_objDigestAkas.RemoveAt(i);
            break;
        }
    }

    IMS_TRACE_D("USIM :: DestroyDigestAKA(%p)", pDigestAka, 0, 0);

    delete pDigestAka;
}

PROTECTED VIRTUAL IDigestAka* OsUsim::CreateDigestAka()
{
    OsUsimDigestAka* pDigestAka = new OsUsimDigestAka(this);

    if (pDigestAka == IMS_NULL)
    {
        return IMS_NULL;
    }

    if (!m_objDigestAkas.Append(pDigestAka))
    {
        delete pDigestAka;
        return IMS_NULL;
    }

    IMS_TRACE_D("USIM :: CreateDigestAKA(%p, %d)", pDigestAka, GetSlotId(), 0);

    return pDigestAka;
}

PROTECTED VIRTUAL void OsUsim::DispatchServiceMessage(IN IMS_UINTP nWparam, IN IMS_UINTP nLparam)
{
    IMS_TRACE_I("USIM :: DispatchServiceMessage - slotId=%d, wp=%" PFLS_u ", lp=%" PFLS_u,
            GetSlotId(), nWparam, nLparam);

    osUsim_HandleUsimEvent(GetSlotId(), reinterpret_cast<OsUsimParam*>(nLparam));
}

PROTECTED VIRTUAL void OsUsim::System_NotifyEvent(
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
        case NOTIFICATION_USIM_AUTH:
        {
            android::String8 strResponse(pParcel->readString16());
            AString strAuth(strResponse.c_str());

            OsUsimAuthResponseParam* pParam = new OsUsimAuthResponseParam();

            pParam->m_nType = OsUsimParam::TYPE_AUTHENTICATION;
            pParam->m_nOwner = INT64_TO_SINTP(pParcel->readInt64());

            IMS_TRACE_D("USIM :: Auth(B) - res(%d)=%s", strAuth.GetLength(), strAuth.GetStr(), 0);

            strAuth = AString::FromBase64(strAuth);

            IMS_TRACE_I("USIM :: Auth(A) - owner=%" PFLS_x ", res=%s", pParam->m_nOwner,
                    OsUtil::GetInstance()->IsDebugMode() ? strAuth.GetStr() : "xxx", 0);

            pParam->m_objResponse.Append(reinterpret_cast<const IMS_BYTE*>(strAuth.GetStr()),
                    static_cast<IMS_SINT32>(strAuth.GetLength()));

            osUsim_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        default:
        {
            break;
        }
    }
}

PUBLIC
IMS_BOOL OsUsim::IsDigestAkaPresent(IN const OsUsimDigestAka* pDigestAka)
{
    if (pDigestAka == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objDigestAkas.GetSize(); ++i)
    {
        const OsUsimDigestAka* pTmp = m_objDigestAkas.GetAt(i);

        if (pDigestAka == pTmp)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}
