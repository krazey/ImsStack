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
#include <utils/String8.h>

#include "IDigestAkaListener.h"
#include "IIsimListener.h"
#include "ImsMessageDef.h"
#include "OsUtil.h"
#include "ServiceMemory.h"
#include "ServicePhoneInfo.h"
#include "ServiceThread.h"
#include "ServiceTrace.h"
#include "device/OsIsim.h"
#include "system-intf/System.h"
#include "system-intf/SystemConstants.h"

__IMS_TRACE_TAG_ADAPT__;

// ISIM system interface parameters for event notification
class OsIsimParam
{
public:
    inline OsIsimParam(IN IMS_SINT32 nType = 0)
        : m_nType(nType)
    {}
    inline virtual ~OsIsimParam()
    {}

public:
    enum
    {
        TYPE_FILE_ATTRIBUTES = 1,
        TYPE_RECORD,
        TYPE_AUTHENTICATION,
        TYPE_STATE
    };

    IMS_SINT32 m_nType;
};

class OsIsimFileAttributesParam
    : public OsIsimParam
{
public:
    inline OsIsimFileAttributesParam()
        : OsIsimParam(TYPE_FILE_ATTRIBUTES)
        , m_nFileId(0)
        , m_nRecordCount(1)
    {}
    inline virtual ~OsIsimFileAttributesParam()
    {}

public:
    IMS_SINT32 m_nFileId;
    IMS_SINT32 m_nRecordCount;
};

class OsIsimRecordParam
    : public OsIsimParam
{
public:
    inline OsIsimRecordParam()
        : OsIsimParam(TYPE_RECORD)
        , m_nFileId(0)
        , m_nIndex(0)
    {}
    inline virtual ~OsIsimRecordParam()
    {}

public:
    IMS_SINT32 m_nFileId;
    IMS_SINT32 m_nIndex;
    ByteArray m_objRecord;
};

class OsIsimAuthResponseParam
    : public OsIsimParam
{
public:
    inline OsIsimAuthResponseParam()
        : OsIsimParam(TYPE_AUTHENTICATION)
        , m_nOwner(0)
    {}
    inline virtual ~OsIsimAuthResponseParam()
    {}

public:
    IMS_SINTP m_nOwner;
    ByteArray m_objResponse;
};

class OsIsimStateParam
    : public OsIsimParam
{
public:
    inline OsIsimStateParam()
        : OsIsimParam(TYPE_STATE)
        , m_nState(0)
    {}
    inline virtual ~OsIsimStateParam()
    {}

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

class OsIsimStateMap
{
public:
    IMS_SINT32 nState;
    const IMS_CHAR* pszState;
};

LOCAL const OsIsimStateMap SIM_STATE_MAP[] =
{
    { OsIsimStateParam::STATE_NOT_PRESENT, "NOT_PRESENT" },
    { OsIsimStateParam::STATE_NOT_READY, "NOT_READY" },
    { OsIsimStateParam::STATE_LOADED, "LOADED" },
    { OsIsimStateParam::STATE_REFRESH_STARTED, "REFRESH_STARTED" },
    { OsIsimStateParam::STATE_REFRESH_COMPLETED, "REFRESH_COMPLETED" },
    { OsIsimStateParam::STATE_SIM_REMOVED, "SIM_REMOVED" },
    { (-1), IMS_NULL }
};



LOCAL
OsIsim* osIsim_GetInstance(IN IMS_SINT32 nSlotId)
{
    return DYNAMIC_CAST(OsIsim*, PhoneInfoService::GetPhoneInfoService()->GetISIM(nSlotId));
}

LOCAL
const IMS_CHAR* osIsim_ConvertFileIdToString(IN IMS_SINT32 nFileId)
{
    switch (nFileId)
    {
        case OsIsim::EF_ID_IMPI:
            return "IMPI";
        case OsIsim::EF_ID_DOMAIN:
            return "DOMAIN";
        case OsIsim::EF_ID_IMPU:
            return "IMPU";
        case OsIsim::EF_ID_IST:
            return "IST";
        case OsIsim::EF_ID_PCSCF:
            return "P-CSCF";
        default:
            return "__INVALID__";
    }
}

LOCAL
void osIsim_HandleAuthResponse(IN IMS_SINT32 nSlotId, IN OsIsimAuthResponseParam* pParam)
{
    OsIsim* pIsim= osIsim_GetInstance(nSlotId);
    OsIsimDigestAka* pDigestAka= reinterpret_cast<OsIsimDigestAka*>(pParam->m_nOwner);

    IMS_TRACE_I("ISIM :: AuthResponse - owner=%" PFLS_x ", res_len=%d",
            pParam->m_nOwner, pParam->m_objResponse.GetLength(), 0);

    if (!pIsim->IsDigestAkaPresent(pDigestAka))
    {
        IMS_TRACE_D("ISIM :: Digest AKA (%p) is not present; ignore...", pDigestAka, 0, 0);
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
        objAuts.Append(&pbyData[nPos+1], pbyData[nPos]);

        nPos += pbyData[nPos];

        pDigestAka->NotifyAutsFailed(objAuts);
    }
    else
    {
        pDigestAka->NotifyMacFailed();
    }

    if (pParam->m_objResponse.GetLength() < nPos)
    {
        IMS_TRACE_D("ISIM :: Invalid AuthRes(res-len=%d, len-from-data=%d)",
                pParam->m_objResponse.GetLength(), nPos, 0);
    }
}

LOCAL
void osIsim_HandleFileAttributes(IN IMS_SINT32 nSlotId, IN OsIsimFileAttributesParam* pParam)
{
    OsIsim* pIsim = osIsim_GetInstance(nSlotId);

    IMS_TRACE_I("ISIM :: FileAttributes - fileId=%X, recordCount=%d",
            pParam->m_nFileId, pParam->m_nRecordCount, 0);

    if (pParam->m_nRecordCount > 0)
    {
        pIsim->SetRecordAttributes(pParam->m_nFileId, IMS_TRUE, pParam->m_nRecordCount);
    }
    else
    {
        pIsim->SetRecordAttributes(pParam->m_nFileId, IMS_FALSE);
    }
}

LOCAL
void osIsim_HandleRecord(IN IMS_SINT32 nSlotId, IN OsIsimRecordParam* pParam)
{
    OsIsim* pIsim = osIsim_GetInstance(nSlotId);

    IMS_TRACE_I("ISIM :: Record - fileId=%X, record=%s",
            pParam->m_nFileId, OsUtil::GetInstance()->IsDebugMode() ?\
            pParam->m_objRecord.ToString().GetStr() :\
            ((pParam->m_objRecord.GetLength() != 0) ? "xxx" : ""), 0);

    pIsim->SetRecord(pParam->m_nFileId,
            pParam->m_objRecord.GetData(),
            pParam->m_objRecord.GetLength());
}

LOCAL
void osIsim_HandleIsimState(IN IMS_SINT32 nSlotId, IN OsIsimStateParam* pParam)
{
    OsIsim* pIsim = osIsim_GetInstance(nSlotId);
    IMS_SINT32 nState = pIsim->GetState();

    IMS_TRACE_I("ISIM :: ISIMState - state=%d, isimState=%d", nState, pParam->m_nState, 0);

    switch (pParam->m_nState)
    {
        case OsIsimStateParam::STATE_NOT_PRESENT: {
            pIsim->SetState(IISIM::STATE_IDLE);
            pIsim->NotifyError(IISIM::ERROR_NO_ISIM_APPLICATION);
            break;
        }
        case OsIsimStateParam::STATE_NOT_READY: {
            // Case 1) Abnormal ISIM state notification (system crash, ...)
            if (nState == IISIM::STATE_READY)
            {
                pIsim->SetState(IISIM::STATE_REFRESHING);
            }
            // Case 1) ISIM state notification when reading the file attributes
            else if (nState == IISIM::STATE_INIT)
            {
                pIsim->SetState(IISIM::STATE_IDLE);
            }
            // Case: Refresh is completed, but reading ISIM records fails.
            else if (nState == IISIM::STATE_REFRESHED)
            {
                pIsim->SetState(IISIM::STATE_REFRESHING);
            }
            break;
        }
        case OsIsimStateParam::STATE_LOADED: {
            if (nState == IISIM::STATE_IDLE)
            {
                pIsim->SetState(IISIM::STATE_INIT);
            }
            // Case 1) Abnormal ISIM state notification (system crash, ...)
            else if (nState == IISIM::STATE_REFRESHING)
            {
                pIsim->SetState(IISIM::STATE_REFRESHED);
            }
            break;
        }
        case OsIsimStateParam::STATE_REFRESH_STARTED: {
            // Case 1) Normal ISIM refresh state notification
            if (nState == IISIM::STATE_READY)
            {
                pIsim->SetState(IISIM::STATE_REFRESHING);
            }
            // Refresh is completed, but reading ISIM records fails.
            else if (nState == IISIM::STATE_REFRESHED)
            {
                pIsim->SetState(IISIM::STATE_REFRESHING);
            }
            // Refresh is started on INIT state (Normal SIM activation)
            else if (nState == IISIM::STATE_INIT)
            {
                pIsim->SetState(IISIM::STATE_REFRESHING);
            }
            break;
        }
        case OsIsimStateParam::STATE_REFRESH_COMPLETED: {
            if (nState == IISIM::STATE_REFRESHING)
            {
                pIsim->SetState(IISIM::STATE_REFRESHED);
            }
            break;
        }
        case OsIsimStateParam::STATE_SIM_REMOVED: {
            pIsim->SetState(IISIM::STATE_IDLE);
            break;
        }
        default: {
            // no-op
            break;
        }
    }
}

LOCAL
void osIsim_HandleIsimEvent(IN IMS_SINT32 nSlotId, IN OsIsimParam* pParam)
{
    if (pParam == IMS_NULL)
    {
        return;
    }

    IMS_TRACE_D("ISIM :: Event - slotId=%d, type=%d", nSlotId, pParam->m_nType, 0);

    switch (pParam->m_nType)
    {
        case OsIsimParam::TYPE_AUTHENTICATION:
            osIsim_HandleAuthResponse(nSlotId,
                    DYNAMIC_CAST(OsIsimAuthResponseParam*, pParam));
            break;
        case OsIsimParam::TYPE_FILE_ATTRIBUTES:
            osIsim_HandleFileAttributes(nSlotId,
                    DYNAMIC_CAST(OsIsimFileAttributesParam*, pParam));
            break;
        case OsIsimParam::TYPE_RECORD:
            osIsim_HandleRecord(nSlotId,
                    DYNAMIC_CAST(OsIsimRecordParam*, pParam));
            break;
        case OsIsimParam::TYPE_STATE:
            osIsim_HandleIsimState(nSlotId,
                    DYNAMIC_CAST(OsIsimStateParam*, pParam));
            break;
        default:
            break;
    }

    delete pParam;
}

LOCAL
void osIsim_SendMessage(IN IThread* piThread, IN IMS_SINT32 nSlotId, IN OsIsimParam* pParam)
{
    if (piThread == IMS_NULL)
    {
        delete pParam;
        return;
    }

    if (!piThread->PostMessageI(IMS_MSG_ISIM,
            nSlotId, reinterpret_cast<IMS_UINTP>(pParam)))
    {
        delete pParam;
    }
}



PUBLIC
OsIsimDigestAka::OsIsimDigestAka(IN ImsIsim* pIsim)
    : m_pIsim(pIsim)
    , m_pDigestAkaListener(IMS_NULL)
{
}

PUBLIC VIRTUAL
OsIsimDigestAka::~OsIsimDigestAka()
{
}

PUBLIC VIRTUAL
void OsIsimDigestAka::Destroy()
{
    m_pDigestAkaListener = IMS_NULL;
    osIsim_GetInstance(m_pIsim->GetSlotId())->DestroyDigestAka(this);
}

PUBLIC VIRTUAL
IMS_RESULT OsIsimDigestAka::GetAuthResponse(IN const ByteArray& objChallenge)
{
    OsIsim* pIsim = osIsim_GetInstance(m_pIsim->GetSlotId());

    if (pIsim->GetState() != IISIM::STATE_READY)
    {
        IMS_TRACE_E(0, "Allowed in READY; state=%d", pIsim->GetState(), 0, 0);
        if (pIsim->GetState() == IISIM::STATE_IDLE)
        {
            pIsim->NotifyAuthResult(IMS_FAILURE);
        }
        return IMS_FAILURE;
    }

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
    strNonce.Attach(reinterpret_cast<const IMS_CHAR*>(objChallenge.GetData()),
            objChallenge.GetLength());
    strNonce = strNonce.ToBase64();

    if (System::GetInstance()->RequestIsimAuthentication(
            strNonce, reinterpret_cast<IMS_SINTP>(this), m_pIsim->GetSlotId()) != IMS_SUCCESS)
    {
        pIsim->NotifyAuthResult(IMS_FAILURE);
        return IMS_FAILURE;
    }

    pIsim->NotifyAuthResult(IMS_SUCCESS);

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL
void OsIsimDigestAka::SetListener(IN IDigestAKAListener* piListener)
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

    m_pDigestAkaListener->AKA_OnAUTSFailed(objAuts);
}

PUBLIC
void OsIsimDigestAka::NotifyMacFailed()
{
    if (m_pDigestAkaListener == IMS_NULL)
    {
        IMS_TRACE_D("DigestAkaListener is null", 0, 0, 0);
        return;
    }

    m_pDigestAkaListener->AKA_OnMACFailed();
}

PUBLIC
void OsIsimDigestAka::NotifyResponse(IN const ByteArray& objRes,
        IN const ByteArray& objIk, IN const ByteArray& objCk)
{
    if (m_pDigestAkaListener == IMS_NULL)
    {
        IMS_TRACE_D("DigestAkaListener is null", 0, 0, 0);
        return;
    }

    m_pDigestAkaListener->AKA_OnResponse(objRes, objIk, objCk);
}



PUBLIC
OsIsim::OsIsim(IN IMS_SINT32 nSlotId)
    : ImsIsim(nSlotId)
    , m_objIsimListeners(IMSList<IISIMListener*>())
    , m_objDigestAkas(IMSList<OsIsimDigestAka*>())
    , m_bInitialized(IMS_FALSE)
    , m_piOwnerThread(IMS_NULL)
    , m_nState(STATE_IDLE)
    , m_objEfContents(IMSMap<IMS_SINT32, IsimEfContent>())
    , m_nCountForAuthFailed(0)
{
    IMS_TRACE_D("Constructor :: ISIM%02d", nSlotId, 0, 0);

    m_objEfContents.Add(EF_ID_IMPI, IsimEfContent());
    m_objEfContents.Add(EF_ID_DOMAIN, IsimEfContent());
    m_objEfContents.Add(EF_ID_IMPU, IsimEfContent(IsimEfContent::EF_LINEAR_FIXED));
    //m_objEfContents.Add(EF_ID_IST, IsimEfContent());
    //m_objEfContents.Add(EF_ID_PCSCF, IsimEfContent(IsimEfContent::EF_LINEAR_FIXED));

    System::GetInstance()->AddListener(
            SystemConstants::CATEGORY_ISIM, this, GetSlotId());

    m_piOwnerThread = ThreadService::GetThreadService()->GetCurrentThread();
}

PUBLIC VIRTUAL
OsIsim::~OsIsim()
{
    IMS_TRACE_D("Destructor :: ISIM%02d", GetSlotId(), 0, 0);

    System::GetInstance()->RemoveListener(
            SystemConstants::CATEGORY_ISIM, this, GetSlotId());
}

PUBLIC VIRTUAL
void OsIsim::DispatchServiceMessage(IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    IMS_TRACE_I("ISIM :: DispatchServiceMessage - slotId=%d, wp=%" PFLS_u ", lp=%" PFLS_u,
            GetSlotId(), nWParam, nLParam);

    osIsim_HandleIsimEvent(GetSlotId(), reinterpret_cast<OsIsimParam*>(nLParam));
}

PUBLIC VIRTUAL
void OsIsim::ClearRecords()
{
    // Clear all the EF records
    for (IMS_UINT32 i = 0; i < m_objEfContents.GetSize(); ++i)
    {
        IsimEfContent& objContent = m_objEfContents.GetValueAt(i);
        objContent.ClearRecords();

        // Zero-based indexing to interwork with TEL-FRW interface
        objContent.SetIndexToRead(0);
    }
}

PUBLIC VIRTUAL
IDigestAKA* OsIsim::CreateDigestAKA()
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

    IMS_TRACE_D("ISIM :: CreateDigestAKA(%p, %d)", pDigestAka, GetSlotId(), 0);

    return pDigestAka;
}

PUBLIC VIRTUAL
IMS_RESULT OsIsim::GetField(IN IMS_SINT32 nField)
{
    IMS_TRACE_D("ISIM :: GetField - fieldId=%d", nField, 0, 0);

    if (nField == FIELD_IST)
    {
        return GetRecord(EF_ID_IST);
    }
    else if (nField == FIELD_PCSCF_ADDRESS)
    {
        return GetRecord(EF_ID_PCSCF);
    }
    else
    {
        return IMS_FAILURE;
    }
}

PUBLIC VIRTUAL
IMS_RESULT OsIsim::GetHomeDomainName()
{
    return GetRecord(EF_ID_DOMAIN);
}

PUBLIC VIRTUAL
IMS_RESULT OsIsim::GetIMPI()
{
    return GetRecord(EF_ID_IMPI);
}

PUBLIC VIRTUAL
IMS_RESULT OsIsim::GetIMPU()
{
    return GetRecord(EF_ID_IMPU);
}

PUBLIC VIRTUAL
IMS_SINT32 OsIsim::GetState() const
{
    return m_nState;
}

PUBLIC VIRTUAL
IMS_BOOL OsIsim::IsReady()
{
    return (m_nState == STATE_READY) ? IMS_TRUE : IMS_FALSE;
}

PUBLIC VIRTUAL
void OsIsim::AddListener(IN IISIMListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objIsimListeners.GetSize(); ++i)
    {
        IISIMListener* piTmpListener = m_objIsimListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            return;
        }
    }

    m_objIsimListeners.Append(piListener);
}

PUBLIC VIRTUAL
void OsIsim::RemoveListener(IN IISIMListener* piListener)
{
    if (piListener == IMS_NULL)
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < m_objIsimListeners.GetSize(); ++i)
    {
        IISIMListener* piTmpListener = m_objIsimListeners.GetAt(i);

        if (piTmpListener == piListener)
        {
            m_objIsimListeners.RemoveAt(i);
            return;
        }
    }
}

PUBLIC VIRTUAL
IMS_RESULT OsIsim::Init()
{
    if (m_bInitialized)
    {
        IMS_TRACE_D("Init :: ISIM is already initialized", 0, 0, 0);
        return IMS_SUCCESS;
    }

    if (System::GetInstance() == IMS_NULL)
    {
        return IMS_FAILURE;
    }

    if (m_nState != STATE_IDLE)
    {
        IMS_TRACE_E(0, "Init() can be only invoked in IDLE state; state(%d)", m_nState, 0, 0);
        return IMS_FAILURE;
    }

    m_bInitialized = IMS_TRUE;
    m_nCountForAuthFailed = 0;

    // In case that ISIM STATUS is LOADED or NOT_PRESENT
    IMS_SINT32 nIsimState = ConvertSimStateToEnum(
            System::GetInstance()->GetIsimState(GetSlotId()));

    if (nIsimState == OsIsimStateParam::STATE_LOADED
            || nIsimState == OsIsimStateParam::STATE_NOT_PRESENT)
    {
        OsIsimStateParam* pParam = new OsIsimStateParam();
        pParam->m_nState = nIsimState;
        osIsim_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL
void OsIsim::Release()
{
    ReleaseUimClient(IMS_TRUE);
}

PUBLIC VIRTUAL
IMS_RESULT OsIsim::Start(IN IMS_SINT32 nEFs /*= EF_ALL*/)
{
    IMS_SINT32 nState = GetState();

    if ((nState != STATE_INIT) && (nState != STATE_REFRESHED))
    {
        IMS_TRACE_E(0, "Start() can be only invoked in INIT or REFRESHED state; state(%d)",
                nState, 0, 0);
        return IMS_FAILURE;
    }

    // Clear all the EF contents
    for (IMS_UINT32 i = 0; i < m_objEfContents.GetSize(); ++i)
    {
        IsimEfContent& objContent = m_objEfContents.GetValueAt(i);
        objContent.Clear();
    }

    if (GetFileAttributes(nEFs, EF_IMPI, EF_ID_IMPI) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    if (GetFileAttributes(nEFs, EF_IMPU, EF_ID_IMPU) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    if (GetFileAttributes(nEFs, EF_DOMAIN, EF_ID_DOMAIN) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    if (GetFileAttributes(nEFs, EF_IST, EF_ID_IST,
            IMS_TRUE, IsimEfContent::EF_TRANSPARENT) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    if (GetFileAttributes(nEFs, EF_PCSCF, EF_ID_PCSCF,
            IMS_TRUE, IsimEfContent::EF_LINEAR_FIXED) != IMS_SUCCESS)
    {
        return IMS_FAILURE;
    }

    return IMS_SUCCESS;
}

PUBLIC VIRTUAL
void OsIsim::System_NotifyEvent(IN IMS_UINT32 nEvent,
        IN IMS_UINTP nWParam, IN IMS_UINTP nLParam)
{
    (void) nWParam;

    android::Parcel* pParcel = reinterpret_cast<android::Parcel*>(nLParam);

    if (pParcel == IMS_NULL)
    {
        return;
    }

    switch (nEvent)
    {
        case NOTIFICATION_ISIM_READ_FILE_ATTRIBUTE: {
            OsIsimFileAttributesParam* pParam = new OsIsimFileAttributesParam();

            pParam->m_nType = OsIsimParam::TYPE_FILE_ATTRIBUTES;
            pParam->m_nFileId = pParcel->readInt32();
            pParam->m_nRecordCount = pParcel->readInt32();

            osIsim_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        case NOTIFICATION_ISIM_READ_RECORD: {
            OsIsimRecordParam* pParam = new OsIsimRecordParam();

            pParam->m_nType = OsIsimParam::TYPE_RECORD;
            pParam->m_nFileId = pParcel->readInt32();
            pParam->m_nIndex = pParcel->readInt32();

            android::String8 strRecord(pParcel->readString16());
            pParam->m_objRecord.Append(reinterpret_cast<const IMS_BYTE*>(strRecord.string()),
                    static_cast<IMS_SINT32>(strRecord.size()));

            osIsim_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        case NOTIFICATION_ISIM_AUTH: {
            android::String8 strResponse(pParcel->readString16());
            AString strAuth(strResponse.string());
            strAuth = AString::FromBase64(strAuth);

            OsIsimAuthResponseParam* pParam = new OsIsimAuthResponseParam();

            pParam->m_nType = OsIsimParam::TYPE_AUTHENTICATION;
            pParam->m_nOwner = INT64_TO_SINTP(pParcel->readInt64());
            pParam->m_objResponse.Append(reinterpret_cast<const IMS_BYTE*>(strAuth.GetStr()),
                    static_cast<IMS_SINT32>(strAuth.GetLength()));

            IMS_TRACE_D("ISIM :: Auth - owner=%" PFLS_x ", res=%s",
                    pParam->m_nOwner, strResponse.string(), 0);

            osIsim_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        case NOTIFICATION_ISIM_STATE_CHANGED: {
            // SIM state is notified when receiving the intent
            android::String8 strTmpState(pParcel->readString16());
            AString strState(strTmpState.string(), strTmpState.length());

            IMS_TRACE_I("ISIM :: SIM state (%s)", strState.GetStr(), 0, 0);

            OsIsimStateParam* pParam = new OsIsimStateParam();

            pParam->m_nType = OsIsimParam::TYPE_STATE;
            pParam->m_nState = ConvertSimStateToEnum(strState);

            osIsim_SendMessage(m_piOwnerThread, GetSlotId(), pParam);
            break;
        }
        default: {
            IMS_TRACE_D("ISIM :: Invalid event (%d)", nEvent, 0, 0);
            break;
        }
    }
}

PUBLIC
IMS_RESULT OsIsim::GetFileAttributes(IN IMS_SINT32 nRequiredEFs, IN IMS_SINT32 nEF,
        IN IMS_SINT32 nFileId, IN IMS_BOOL bOptionalField /*= IMS_FALSE*/,
        IN IMS_SINT32 nEFContentType /*= IsimEfContent::EF_TRANSPARENT*/)
{
    if ((nRequiredEFs & nEF) != 0)
    {
        if (bOptionalField)
        {
            m_objEfContents.SetValue(nFileId, IsimEfContent(nEFContentType));
        }

        if (System::GetInstance()->ReadIsimFileAttributes(
                nFileId, GetSlotId()) != IMS_SUCCESS)
        {
            return IMS_FAILURE;
        }
    }
    else
    {
        if (bOptionalField)
        {
            m_objEfContents.Remove(nFileId);
        }
    }

    return IMS_SUCCESS;
}

PUBLIC
IMS_RESULT OsIsim::GetRecord(IN IMS_SINT32 nFileId)
{
    const IMS_CHAR* pszRecordName = osIsim_ConvertFileIdToString(nFileId);

    IMS_TRACE_I("ISIM :: GetRecord - %s(%X)", _TRACE_S_(pszRecordName), nFileId, 0);

    if (m_nState != STATE_READY)
    {
        IMS_TRACE_E(0, "Invalid state (%d)", m_nState, 0, 0);
        return IMS_FAILURE;
    }

    IMS_SLONG nIndex = m_objEfContents.GetIndexOfKey(nFileId);

    if (nIndex < 0)
    {
        IMS_TRACE_D("ISIM :: %s record is not required", _TRACE_S_(pszRecordName), 0, 0);
        return RESULT_NO_RECORDS;
    }

    IsimEfContent& objContent = m_objEfContents.GetValueAt(nIndex);

    if (objContent.GetCount() <= 0)
    {
        IMS_TRACE_D("ISIM :: No %s records", _TRACE_S_(pszRecordName), 0, 0);
        return RESULT_NO_RECORDS;
    }

    objContent.SetReadRequested(IMS_TRUE);

    if (System::GetInstance()->ReadIsimRecord(
            nFileId, objContent.GetIndexToRead(), GetSlotId()) != IMS_SUCCESS)
    {
        objContent.SetReadRequested(IMS_FALSE);

        IMS_TRACE_E(0, "Reading %s failed", _TRACE_S_(pszRecordName), 0, 0);
        return IMS_FAILURE;
    }

    if (objContent.GetType() == IsimEfContent::EF_LINEAR_FIXED)
    {
        objContent.SetIndexToRead(objContent.GetIndexToRead() + 1);
    }

    IMS_TRACE_D("%s read request is done", _TRACE_S_(pszRecordName), 0, 0);

    return IMS_SUCCESS;
}

PUBLIC
IMS_BOOL OsIsim::IsAllAttributesReady() const
{
    for (IMS_UINT32 i = 0; i < m_objEfContents.GetSize(); ++i)
    {
        const IsimEfContent& objContent = m_objEfContents.GetValueAt(i);

        if (!objContent.IsReady())
        {
            return IMS_FALSE;
        }
    }

    return IMS_TRUE;
}

PUBLIC
void OsIsim::NotifyAuthResult(IN IMS_RESULT nResult)
{
    if (nResult == IMS_SUCCESS)
    {
        m_nCountForAuthFailed = 0;
    }
    else
    {
        ++m_nCountForAuthFailed;
    }

    if (m_nCountForAuthFailed == MAX_AUTH_FAILURE_COUNT_FOR_RECOVERY)
    {
        IMS_TRACE_D("ISIM :: auth-request is failed for 3 times continuously...",
                0, 0, 0);

        m_nCountForAuthFailed = 0;
        ReleaseUimClient();

        NotifyError(ERROR_INTERFACE_CHANNEL_ERROR);
    }
}

PUBLIC
void OsIsim::NotifyError(IN IMS_UINT32 nError)
{
    for (IMS_UINT32 i = 0; i < m_objIsimListeners.GetSize(); ++i)
    {
        IISIMListener* piListener = m_objIsimListeners.GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }
        piListener->ISIM_OnError(nError);
    }
}

PUBLIC
void OsIsim::NotifyError(IN IMS_SINT32 nFileId, IN IMS_SINT32 nOperation)
{
    IMS_SLONG nIndex = m_objEfContents.GetIndexOfKey(nFileId);

    if (nIndex < 0)
    {
        IMS_TRACE_D("EF(%X) record is not supported", nFileId, 0, 0);
    }
    else
    {
        IsimEfContent& objContent = m_objEfContents.GetValueAt(nIndex);

        if (nOperation == OPERATION_READ_RECORD)
        {
            objContent.SetReadRequested(IMS_FALSE);
        }
    }

    IMS_SINT32 nErrorReason = -1;

    switch (nOperation)
    {
        case OPERATION_READ_RECORD:
        {
            if (nFileId == EF_ID_IMPI)
            {
                nErrorReason = IISIM::ERROR_READ_IMPI_FAILED;
            }
            else if (nFileId == EF_ID_DOMAIN)
            {
                nErrorReason = IISIM::ERROR_READ_DOMAIN_FAILED;
            }
            else if (nFileId == EF_ID_IMPU)
            {
                nErrorReason = IISIM::ERROR_READ_IMPU_FAILED;
            }
            else if (nFileId == EF_ID_IST)
            {
                nErrorReason = IISIM::ERROR_READ_IST_FAILED;
            }
            else if (nFileId == EF_ID_PCSCF)
            {
                nErrorReason = IISIM::ERROR_READ_PCSCF_ADDRESS_FAILED;
            }
            break;
        }
        default:
            break;
    }

    for (IMS_UINT32 i = 0; i < m_objIsimListeners.GetSize(); ++i)
    {
        IISIMListener* piListener = m_objIsimListeners.GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }

        if (nErrorReason != -1)
        {
            piListener->ISIM_OnError(nErrorReason);
        }
    }
}

PUBLIC
void OsIsim::NotifyRecordReadCompleted(IN IMS_SINT32 nFileId,
        IN const IsimEfContent& objContent)
{
    for (IMS_UINT32 i = 0; i < m_objIsimListeners.GetSize(); ++i)
    {
        IISIMListener* piListener = m_objIsimListeners.GetAt(i);

        if (piListener == IMS_NULL)
        {
            continue;
        }

        switch (nFileId)
        {
        case EF_ID_IMPI:
            piListener->ISIM_OnIMPI(objContent.GetRecords().GetAt(0));
            break;
        case EF_ID_DOMAIN:
            piListener->ISIM_OnHomeDomainName(objContent.GetRecords().GetAt(0));
            break;
        case EF_ID_IMPU:
            piListener->ISIM_OnIMPU(objContent.GetRecords());
            break;
        case EF_ID_IST:
            piListener->ISIM_OnField(FIELD_IST, objContent.GetRecords());
            break;
        case EF_ID_PCSCF:
            piListener->ISIM_OnField(FIELD_PCSCF_ADDRESS, objContent.GetRecords());
            break;
        default:
            break;
        }
    }
}

PUBLIC
void OsIsim::ReleaseUimClient(IN IMS_BOOL bFromApp /*= IMS_FALSE*/)
{
    IMS_TRACE_I("ReleaseUIMClient - initialized=%s, fromApp=%s",
            _TRACE_B_(m_bInitialized), _TRACE_B_(bFromApp), 0);

    m_nState = IISIM::STATE_IDLE;
    m_nCountForAuthFailed = 0;
    m_bInitialized = IMS_FALSE;
}

PUBLIC
void OsIsim::SetRecord(IN IMS_SINT32 nFileId, IN const IMS_BYTE* pbyData, IN IMS_SINT32 nSize)
{
    IMS_TRACE_D("ISIM :: SetRecord - fileId=%X, size=%d", nFileId, nSize, 0);

    IMS_SLONG nIndex = m_objEfContents.GetIndexOfKey(nFileId);

    if (nIndex < 0)
    {
        IMS_TRACE_E(0, "EF(%X) record is not required", nFileId, 0, 0);
        return;
    }

    if (nFileId == EF_ID_IST)
    {
        IMS_TRACE_D("SetRecord :: %s=0x%02X, %d",
                osIsim_ConvertFileIdToString(nFileId), pbyData[0], nSize);
    }
    else
    {
        IMS_TRACE_D("SetRecord :: %s=%s, %d",
                osIsim_ConvertFileIdToString(nFileId),
                OsUtil::GetInstance()->IsDebugMode() ?\
                    reinterpret_cast<const IMS_CHAR*>(pbyData) : "xxx", nSize);
    }

    IsimEfContent& objContent = m_objEfContents.GetValueAt(nIndex);

    if (!objContent.IsReadRequested())
    {
        IMS_TRACE_D("EF(%X) record is not read by the application", nFileId, 0, 0);
        return;
    }

    objContent.AddRecord(pbyData, nSize);

    if (objContent.IsAllRecordsReadCompleted())
    {
        objContent.SetReadRequested(IMS_FALSE);
        NotifyRecordReadCompleted(nFileId, objContent);
    }
    else
    {
        if ((GetState() == STATE_REFRESHING) || (GetState() == STATE_REFRESHED))
        {
            IMS_TRACE_D("Do not request the linear fixed records any more in this state,"
                    " REFRESHING or REFRESHED", 0, 0, 0);
            return;
        }

        if (objContent.GetType() == IsimEfContent::EF_LINEAR_FIXED)
        {
            if (System::GetInstance()->ReadIsimRecord(
                    nFileId, objContent.GetIndexToRead(), GetSlotId()) != IMS_SUCCESS)
            {
                IMS_TRACE_E(0, "Reading EF(%X) failed", nFileId, 0, 0);
                NotifyError(nFileId, OPERATION_READ_RECORD);
                return;
            }

            objContent.SetIndexToRead(objContent.GetIndexToRead() + 1);
        }
    }
}

PUBLIC
void OsIsim::SetRecordAttributes(IN IMS_SINT32 nFileId,
        IN IMS_BOOL bReady, IN IMS_SINT32 nRecordCount /*= 0*/)
{
    IMS_SINT32 nState = GetState();

    if ((nState == IISIM::STATE_IDLE) || (nState == IISIM::STATE_REFRESHING))
    {
        IMS_TRACE_D("EF(%X) can't be set in the state (IDLE or REFRESHING)", nFileId, 0, 0);
        return;
    }

    if (!bReady
            && ((nFileId == EF_ID_IMPI)
                || (nFileId == EF_ID_DOMAIN)
                || (nFileId == EF_ID_IMPU)))
    {
        IMS_TRACE_E(0, "EF(%X) is not ready", nFileId, 0, 0);
        NotifyError(ERROR_START_FAILED);
        return;
    }

    IMS_SLONG nIndex = m_objEfContents.GetIndexOfKey(nFileId);

    if (nIndex < 0)
    {
        IMS_TRACE_D("EF(%X) record is not required", nFileId, 0, 0);
        return;
    }

    IsimEfContent& objContent = m_objEfContents.GetValueAt(nIndex);

    objContent.SetReady(bReady);

    if (objContent.GetType() == IsimEfContent::EF_TRANSPARENT)
    {
        objContent.SetCount(1);
    }
    else
    {
        objContent.SetCount(nRecordCount);
    }

    // Zero-based indexing to interwork with TEL-FRW interface
    objContent.SetIndexToRead(0);

    if (!IsAllAttributesReady())
    {
        return;
    }

    if (GetState() == STATE_REFRESHED)
    {
        IMS_TRACE_I("All ISIM attributes are ready in REFRESHED", 0, 0, 0);
        SetState(STATE_READY);
    }
    else
    {
        IMS_TRACE_I("All ISIM attributes are ready in INIT", 0, 0, 0);
        SetState(STATE_READY);

        // ISIM refresh callback will be registered automatically by the Java layer
    }
}

PUBLIC
void OsIsim::SetState(IN IMS_SINT32 nState)
{
    if (m_nState == nState)
    {
        return;
    }

    if (nState == STATE_IDLE)
    {
        m_nState = STATE_IDLE;
        IMS_TRACE_I("ISIM state is IDLE", 0, 0, 0);
    }
    else if (nState == STATE_INIT)
    {
        // ignore the state change "init" if current state is "ready" or "refreshing"
        if ((m_nState == STATE_READY) || (m_nState == STATE_REFRESHING))
        {
            return;
        }

        m_nState = STATE_INIT;
        IMS_TRACE_I("ISIM state is INIT", 0, 0, 0);
    }
    else if (nState == STATE_READY)
    {
        m_nState = STATE_READY;
        IMS_TRACE_I("ISIM state is READY", 0, 0, 0);
    }
    else if (nState == STATE_REFRESHING)
    {
        m_nState = STATE_REFRESHING;
        IMS_TRACE_I("ISIM state is REFRESHING", 0, 0, 0);
    }
    else if (nState == STATE_REFRESHED)
    {
        m_nState = STATE_REFRESHED;
        IMS_TRACE_I("ISIM state is REFRESHED", 0, 0, 0);
    }

    for (IMS_UINT32 i = 0; i < m_objIsimListeners.GetSize(); ++i)
    {
        IISIMListener* piListener = m_objIsimListeners.GetAt(i);

        if (piListener != IMS_NULL)
        {
            piListener->ISIM_OnStateChanged(m_nState);
        }
    }
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
        OsIsimDigestAka* pTmp = m_objDigestAkas.GetAt(i);

        if (pDigestAka == pTmp)
        {
            m_objDigestAkas.RemoveAt(i);
            break;
        }
    }

    IMS_TRACE_D("ISIM :: DestroyDigestAKA(%p)", pDigestAka, 0, 0);

    delete pDigestAka;
}

PUBLIC
IMS_BOOL OsIsim::IsDigestAkaPresent(IN OsIsimDigestAka* pDigestAka)
{
    if (pDigestAka == IMS_NULL)
    {
        return IMS_FALSE;
    }

    for (IMS_UINT32 i = 0; i < m_objDigestAkas.GetSize(); ++i)
    {
        OsIsimDigestAka* pTmp = m_objDigestAkas.GetAt(i);

        if (pDigestAka == pTmp)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL
IMS_SINT32 OsIsim::ConvertSimStateToEnum(IN const AString& strState)
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
