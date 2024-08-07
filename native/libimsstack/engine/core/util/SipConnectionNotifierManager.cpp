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
#include "ImsMap.h"
#include "ServiceMemory.h"
#include "ServiceMutex.h"
#include "ServiceTrace.h"
#include "SystemConfig.h"

#include "Feature.h"

#include "Connector.h"
#include "EngineActivity.h"
#include "ISipConnectionNotifier.h"
#include "ISipDialog.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipServerConnection.h"
#include "ISipServerConnectionListener.h"
#include "ImsCoreContext.h"
#include "Service.h"
#include "ServiceManager.h"
#include "Sip.h"
#include "SipConfigProxy.h"
#include "SipDebug.h"
#include "SipFactoryProxy.h"
#include "SipFeatures.h"
#include "SipParameter.h"
#include "SipParsingHelper.h"
#include "SipRoutingRejectNotifier.h"
#include "SipStatusCode.h"
#include "util/CallerPreference.h"
#include "util/CancellableMethodManager.h"
#include "util/DialogMethodManager.h"
#include "util/ForkedDialogMethodManager.h"
#include "util/PreferenceHeader.h"
#include "util/SipConnectionNotifierManager.h"
#include "util/UserAgentHeader.h"

__IMS_TRACE_TAG_IMS__;

class SipServerConnectionListenerProxy : public EngineActivity, public ISipServerConnectionListener
{
public:
    SipServerConnectionListenerProxy(
            IN const AString& strName, IN ISipServerConnectionListener* piListener);
    virtual ~SipServerConnectionListenerProxy();

    SipServerConnectionListenerProxy(IN const SipServerConnectionListenerProxy&) = delete;
    SipServerConnectionListenerProxy& operator=(
            IN const SipServerConnectionListenerProxy&) = delete;

private:
    // Activity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    // ISipServerConnectionListener interface implementations
    void ServerConnection_NotifyRequest(
            IN ISipConnectionNotifier* piScn, IN IMS_BOOL bIsForked = IMS_FALSE) override;

private:
    enum
    {
        AMSG_SIP_REQUEST_RECEIVED = AMSG_USER,
        AMSG_SIP_FORKED_REQUEST_RECEIVED
    };

    ISipServerConnectionListener* m_piListener;
};

PUBLIC
SipServerConnectionListenerProxy::SipServerConnectionListenerProxy(
        IN const AString& strName, IN ISipServerConnectionListener* piListener) :
        EngineActivity(strName),
        m_piListener(piListener)
{
}

PUBLIC VIRTUAL SipServerConnectionListenerProxy::~SipServerConnectionListenerProxy() {}

PRIVATE VIRTUAL IMS_BOOL SipServerConnectionListenerProxy::DispatchMessage(IN ImsMessage& objMsg)
{
    switch (objMsg.GetName())
    {
        case AMSG_SIP_REQUEST_RECEIVED:
            m_piListener->ServerConnection_NotifyRequest(
                    reinterpret_cast<ISipConnectionNotifier*>(objMsg.nLparam));
            return IMS_TRUE;
        case AMSG_SIP_FORKED_REQUEST_RECEIVED:
            m_piListener->ServerConnection_NotifyRequest(
                    reinterpret_cast<ISipConnectionNotifier*>(objMsg.nLparam), IMS_TRUE);
            return IMS_TRUE;
        default:
            break;
    }

    return EngineActivity::DispatchMessage(objMsg);
}

PRIVATE VIRTUAL void SipServerConnectionListenerProxy::ServerConnection_NotifyRequest(
        IN ISipConnectionNotifier* piScn, IN IMS_BOOL bIsForked /*= IMS_FALSE*/)
{
    // 4 Use an internal queue to pass the SIP connection notifier

    if (bIsForked)
    {
        PostMessage(AMSG_SIP_FORKED_REQUEST_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(piScn));
    }
    else
    {
        PostMessage(AMSG_SIP_REQUEST_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(piScn));
    }
}

class SipConnectionNotifierManagerPrivate : public ISipServerConnectionListener
{
public:
    SipConnectionNotifierManagerPrivate();
    virtual ~SipConnectionNotifierManagerPrivate();

    SipConnectionNotifierManagerPrivate(IN const SipConnectionNotifierManagerPrivate&) = delete;
    SipConnectionNotifierManagerPrivate& operator=(
            IN const SipConnectionNotifierManagerPrivate&) = delete;

public:
    void Init(IN IMS_SINT32 nSlotId);

    ISipConnectionNotifier* CreateConnectionNotifier(IN const AString& strScheme,
            IN const IpAddress& objIpAddr, IN IMS_SINT32 nPortS, IN IMS_SINT32 nPortC,
            IN IMS_SINT32 nPortFlowControl, IN const AString& strParams,
            IN const SipAddress& objUserId);

    ISipConnectionNotifier* GetConnectionNotifier(
            IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort);

    void AddConnectionNotifier(IN const AString& strKey, IN ISipConnectionNotifier* piScn);
    void ReleaseConnectionNotifier(IN ISipConnectionNotifier* piScn);

private:
    // ISipServerConnectionListener interface
    void ServerConnection_NotifyRequest(
            IN ISipConnectionNotifier* piScn, IN IMS_BOOL bIsForked = IMS_FALSE) override;

    void AddReference(IN const AString& strKey);
    IMS_SINT32 RemoveReference(IN const AString& strKey);
    ISipConnectionNotifier* GetConnectionNotifier(IN const AString& strKey);
    SipServerConnectionListenerProxy* GetServerConnectionListener(IN IMS_SINT32 nSlotId);
    IMS_BOOL IsConnectionNotifierPresent(IN const ISipConnectionNotifier* piScn) const;

    static IMS_BOOL CheckMessageValidity(IN ISipMessage* piSipMsg, OUT AString& strReason);
    static AString CreateConnectionNotifierKey(IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort);
    static void CreateExtraFeatures(IN Service* pService, OUT ImsList<FeatureSet*>& objFeatures);
    static void CreatePreferenceHeaders(
            IN const AStringArray& objAcceptContacts, OUT ImsList<PreferenceHeader*>& objHeaders);
    static void DestroyExtraFeatures(OUT ImsList<FeatureSet*>& objFeatures);
    static void DestroyPreferenceHeaders(OUT ImsList<PreferenceHeader*>& objHeaders);
    static void GetCalleePreferenceSupportedServices(IN const ImsList<Service*>& objServices,
            IN const SipMethod& objMethod, OUT ImsList<Service*>& objCalleePreferenceServices);
    static void GetRejectCode(IN ISipServerConnection* piSsc, IN IMS_SINT32 nStatusCode,
            IN IMS_SINT32 nLogInfo, OUT SipStatusCode& objStatusCode);
    static void HandleSipRequest(
            IN ISipConnectionNotifier* piScn, IN IMS_BOOL bIsForked = IMS_FALSE);
    static IMS_BOOL HasService(
            IN const ImsList<Service*>& objServices, IN const Service* pEvaluatedService);
    static IMS_BOOL IsCalleePreferenceSupported(
            IN const ImsList<Service*>& objServices, IN const SipMethod& objMethod);
    static Service* RouteSipRequest(
            IN ISipServerConnection* piSsc, OUT SipStatusCode& objStatusCode);
    static Service* RouteSipRequestByIfc(
            IN const ImsList<Service*>& objServices, IN ISipServerConnection* piSsc);
    static void SendResponse(IN ISipConnectionNotifier* piScn, IN ISipServerConnection* piSsc,
            IN IMS_SINT32 nStatusCode, IN const AString& strReasonPhrase = AString::ConstNull(),
            IN IMS_BOOL bDebuggableToTag = IMS_FALSE);
    static void SetServerHeader(IN ISipConnectionNotifier* piScn, IN ISipServerConnection* piSsc);

private:
    IMutex* m_piLock;
    SipServerConnectionListenerProxy** m_ppListenerProxy;
    // < (IP + Port), ISipConnectionNotifier* >
    ImsMap<AString, ISipConnectionNotifier*> m_objConnectionNotifiers;
    // < (IP + Port), Count >
    ImsMap<AString, IMS_SINT32> m_objReferenceCounts;
};

PUBLIC
SipConnectionNotifierManagerPrivate::SipConnectionNotifierManagerPrivate() :
        m_piLock(IMS_NULL),
        m_ppListenerProxy(IMS_NULL)
{
    m_piLock = MutexService::GetMutexService()->CreateMutex();

    IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

    m_ppListenerProxy = new SipServerConnectionListenerProxy*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        m_ppListenerProxy[i] = IMS_NULL;
    }
}

PUBLIC VIRTUAL SipConnectionNotifierManagerPrivate::~SipConnectionNotifierManagerPrivate()
{
    if (m_ppListenerProxy != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetSupportedSimCount();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (m_ppListenerProxy[i] != IMS_NULL)
            {
                delete m_ppListenerProxy[i];
            }
        }

        delete[] m_ppListenerProxy;
    }

    MutexService::GetMutexService()->DestroyMutex(m_piLock);
}

PUBLIC
void SipConnectionNotifierManagerPrivate::Init(IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetSupportedSimCount()))
    {
        return;
    }

    if (m_ppListenerProxy[nSlotId] == IMS_NULL)
    {
        AString strName;
        strName.Sprintf("SSCLP_%02d", nSlotId);

        IMS_TRACE_I("SCN :: %s is created", strName.GetStr(), 0, 0);

        m_ppListenerProxy[nSlotId] = new SipServerConnectionListenerProxy(strName, this);
    }
}

PUBLIC
ISipConnectionNotifier* SipConnectionNotifierManagerPrivate::CreateConnectionNotifier(
        IN const AString& strScheme, IN const IpAddress& objIpAddr, IN IMS_SINT32 nPortS,
        IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl, IN const AString& strParams,
        IN const SipAddress& objUserId)
{
    AString strKey = CreateConnectionNotifierKey(objIpAddr, nPortS);
    ISipConnectionNotifier* piScn = GetConnectionNotifier(strKey);

    if (piScn != IMS_NULL)
    {
        // Add the reference count
        AddReference(strKey);

        return piScn;
    }

    AString strName;
    strName.SetNumber(nPortS);

    piScn = DYNAMIC_CAST(ISipConnectionNotifier*, Connector::Open(strScheme, strName, strParams));

    if (piScn == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating SIP connection notifier failed", 0, 0, 0);
        return IMS_NULL;
    }

    piScn->SetListener(GetServerConnectionListener(piScn->GetSlotId()));

    if (piScn->ReserveTransportResource(objIpAddr, nPortS, nPortC, nPortFlowControl) != IMS_SUCCESS)
    {
        piScn->Close();

        IMS_TRACE_E(0, "Creating a transport resource failed", 0, 0, 0);
        return IMS_NULL;
    }

    // Set a default From & Contact information
    piScn->SetFromAndContact(objUserId.GetUri(), objUserId.GetDisplayName(), objUserId.GetUser());

    AddConnectionNotifier(strKey, piScn);
    AddReference(strKey);

    IMS_TRACE_D("SIP Connection Notifier (%s, %d) is created",
            SipDebug::GetCharA1(strKey.GetStr(), 5), piScn->GetSlotId(), 0);

    return piScn;
}

PUBLIC
ISipConnectionNotifier* SipConnectionNotifierManagerPrivate::GetConnectionNotifier(
        IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort)
{
    AString strKey = CreateConnectionNotifierKey(objIpAddr, nPort);
    ISipConnectionNotifier* piScn = GetConnectionNotifier(strKey);

    if (piScn == IMS_NULL)
    {
        IMS_TRACE_D("SIP Connection Notifier (%s) is not found",
                SipDebug::GetCharA1(strKey.GetStr(), 5), 0, 0);
        return IMS_NULL;
    }

    AddReference(strKey);

    return piScn;
}

PUBLIC
void SipConnectionNotifierManagerPrivate::AddConnectionNotifier(
        IN const AString& strKey, IN ISipConnectionNotifier* piScn)
{
    LockGuard objLock(m_piLock);

    m_objConnectionNotifiers.Add(strKey, piScn);
}

PUBLIC
void SipConnectionNotifierManagerPrivate::ReleaseConnectionNotifier(
        IN ISipConnectionNotifier* piScn)
{
    if (piScn == IMS_NULL)
    {
        return;
    }

    AString strKey = CreateConnectionNotifierKey(piScn->GetLocalAddress(), piScn->GetLocalPort());
    IMS_SINT32 nCount = RemoveReference(strKey);

    if (nCount <= 0)
    {
        LockGuard objLock(m_piLock);
        m_objConnectionNotifiers.Remove(strKey);
        m_objReferenceCounts.Remove(strKey);

        // Close the SIP connection notifier
        piScn->Close();
    }
}

PRIVATE VIRTUAL void SipConnectionNotifierManagerPrivate::ServerConnection_NotifyRequest(
        IN ISipConnectionNotifier* piScn, IN IMS_BOOL bIsForked /*= IMS_FALSE*/)
{
    if (!IsConnectionNotifierPresent(piScn))
    {
        IMS_TRACE_E(0, "SipConnectionNotifier(%" PFLS_X ") is not present", piScn, 0, 0);
        return;
    }

    HandleSipRequest(piScn, bIsForked);
}

PRIVATE
void SipConnectionNotifierManagerPrivate::AddReference(IN const AString& strKey)
{
    LockGuard objLock(m_piLock);
    IMS_SLONG nIndex = m_objReferenceCounts.GetIndexOfKey(strKey);

    if (nIndex < 0)
    {
        // It is just created, so add a new reference count
        m_objReferenceCounts.Add(strKey, 1);
        IMS_TRACE_D("SCN_REF (%s) :: Created", SipDebug::GetCharA1(strKey.GetStr(), 5), 0, 0);
        return;
    }

    IMS_SINT32& nValue = m_objReferenceCounts.GetValueAt(nIndex);

    ++nValue;

    IMS_TRACE_D("SCN_REF (%s) :: %d >>> %d", SipDebug::GetCharA1(strKey.GetStr(), 5), nValue - 1,
            nValue);
}

PRIVATE
IMS_SINT32 SipConnectionNotifierManagerPrivate::RemoveReference(IN const AString& strKey)
{
    LockGuard objLock(m_piLock);
    IMS_SLONG nIndex = m_objReferenceCounts.GetIndexOfKey(strKey);

    if (nIndex < 0)
    {
        return 0;
    }

    IMS_SINT32& nValue = m_objReferenceCounts.GetValueAt(nIndex);

    --nValue;

    IMS_TRACE_D("SCN_REF (%s) :: %d >>> %d", SipDebug::GetCharA1(strKey.GetStr(), 5), nValue + 1,
            nValue);

    return nValue;
}

PRIVATE
ISipConnectionNotifier* SipConnectionNotifierManagerPrivate::GetConnectionNotifier(
        IN const AString& strKey)
{
    LockGuard objLock(m_piLock);
    IMS_SLONG nIndex = m_objConnectionNotifiers.GetIndexOfKey(strKey);

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return m_objConnectionNotifiers.GetValueAt(nIndex);
}

PRIVATE
SipServerConnectionListenerProxy* SipConnectionNotifierManagerPrivate::GetServerConnectionListener(
        IN IMS_SINT32 nSlotId)
{
    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetSupportedSimCount()))
    {
        return IMS_NULL;
    }

    return m_ppListenerProxy[nSlotId];
}

PRIVATE
IMS_BOOL SipConnectionNotifierManagerPrivate::IsConnectionNotifierPresent(
        IN const ISipConnectionNotifier* piScn) const
{
    LockGuard objLock(m_piLock);

    for (IMS_UINT32 i = 0; i < m_objConnectionNotifiers.GetSize(); ++i)
    {
        ISipConnectionNotifier* piTmpScn = m_objConnectionNotifiers.GetValueAt(i);

        if (piTmpScn == IMS_NULL)
        {
            continue;
        }

        if (piTmpScn == piScn)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL SipConnectionNotifierManagerPrivate::CheckMessageValidity(
        IN ISipMessage* piSipMsg, OUT AString& strReason)
{
    // According to the SIP method, check the mandatory header or parameters ...
    const SipMethod& objMethod = piSipMsg->GetMethod();

    // RFC 3891 : Replaces header requirements
    if (objMethod.ToInt() != SipMethod::INVITE)
    {
        if (piSipMsg->IsHeaderPresent(ISipHeader::REPLACES))
        {
            strReason = "Replaces header is present in non-INVITE";
            return IMS_FALSE;
        }
    }

    switch (objMethod.ToInt())
    {
        case SipMethod::INVITE:
        {
            // Check Replaces header
            if (piSipMsg->GetHeaderCount(ISipHeader::REPLACES) > 1)
            {
                strReason = "Multiple Replaces headers are present";
                return IMS_FALSE;
            }
            break;
        }
        case SipMethod::REFER:
        {
            // Check Refer-To header
            AString strReferTo = piSipMsg->GetHeader(ISipHeader::REFER_TO);

            if (strReferTo.IsNULL() || strReferTo.IsEmpty())
            {
                strReason = "Mandatory Header Missing";
                return IMS_FALSE;
            }

            ISipHeader* piHeader = SipParsingHelper::CreateHeader(ISipHeader::REFER_TO, strReferTo);

            if (piHeader == IMS_NULL)
            {
                strReason = "Bad Request";
                return IMS_FALSE;
            }

            const SipAddress* pAddress = piHeader->GetSipAddress();

            if (pAddress == IMS_NULL)
            {
                piHeader->Destroy();

                strReason = "Bad Request";
                return IMS_FALSE;
            }

            const SipParameter* pParameter = pAddress->GetParameter(Sip::STR_METHOD);

            if (pParameter == IMS_NULL)
            {
                IMS_TRACE_D("Refer-To :: method parameter does not exist", 0, 0, 0);
                piHeader->Destroy();
                break;
            }

            if (pParameter->GetValue().IsNULL() || pParameter->GetValue().IsEmpty())
            {
                piHeader->Destroy();

                strReason = "Mandatory Parameter Missing";
                return IMS_FALSE;
            }

            piHeader->Destroy();
            break;
        }
        default:
            break;
    }

    return IMS_TRUE;
}

PRIVATE GLOBAL AString SipConnectionNotifierManagerPrivate::CreateConnectionNotifierKey(
        IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort)
{
    AString strKey;

    strKey.Sprintf("%s:%d", objIpAddr.ToString().GetStr(), nPort);

    return strKey;
}

PRIVATE GLOBAL void SipConnectionNotifierManagerPrivate::CreateExtraFeatures(
        IN Service* pService, OUT ImsList<FeatureSet*>& objFeatures)
{
    if (pService == IMS_NULL)
    {
        return;
    }

    // Provide the "+sip.instance" feature if it is supported in the device
    const SipParameter* pParameter = pService->GetInstanceParameter();

    if (pParameter != IMS_NULL)
    {
        FeatureSet* pFeatureSet = new FeatureSet(pParameter->GetName(), pParameter->GetValue());

        if (pFeatureSet != IMS_NULL)
        {
            objFeatures.Append(pFeatureSet);
        }
    }
}

PRIVATE GLOBAL void SipConnectionNotifierManagerPrivate::CreatePreferenceHeaders(
        IN const AStringArray& objAcceptContacts, OUT ImsList<PreferenceHeader*>& objHeaders)
{
    if (objAcceptContacts.IsEmpty())
    {
        return;
    }

    for (IMS_SINT32 i = 0; i < objAcceptContacts.GetCount(); ++i)
    {
        const AString& strHeader = objAcceptContacts.GetElementAt(i);
        ISipHeader* piHeader =
                SipParsingHelper::CreateHeader(ISipHeader::ACCEPT_CONTACT, strHeader);

        if (piHeader == IMS_NULL)
        {
            continue;
        }

        PreferenceHeader* pHeader = new PreferenceHeader(piHeader);

        if (pHeader == IMS_NULL)
        {
            piHeader->Destroy();
            continue;
        }

        if (!objHeaders.Append(pHeader))
        {
            delete pHeader;
        }

        piHeader->Destroy();
    }
}

PRIVATE GLOBAL void SipConnectionNotifierManagerPrivate::DestroyExtraFeatures(
        OUT ImsList<FeatureSet*>& objFeatures)
{
    if (objFeatures.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objFeatures.GetSize(); ++i)
    {
        FeatureSet* pFeatureSet = objFeatures.GetAt(i);

        if (pFeatureSet != IMS_NULL)
        {
            delete pFeatureSet;
        }
    }

    objFeatures.Clear();
}

PRIVATE GLOBAL void SipConnectionNotifierManagerPrivate::DestroyPreferenceHeaders(
        OUT ImsList<PreferenceHeader*>& objHeaders)
{
    if (objHeaders.IsEmpty())
    {
        return;
    }

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        PreferenceHeader* pHeader = objHeaders.GetAt(i);

        if (pHeader != IMS_NULL)
        {
            delete pHeader;
        }
    }

    objHeaders.Clear();
}

PRIVATE GLOBAL void SipConnectionNotifierManagerPrivate::GetCalleePreferenceSupportedServices(
        IN const ImsList<Service*>& objServices, IN const SipMethod& objMethod,
        OUT ImsList<Service*>& objCalleePreferenceServices)
{
    // In the moment, OPTIONS method will be only handled for this request.
    if (!objMethod.Equals(SipMethod::OPTIONS))
    {
        // no-op
        return;
    }

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        Service* pService = objServices.GetAt(i);

        if (pService == IMS_NULL)
        {
            continue;
        }

        ServiceFilterCriteria* pSfc = pService->GetFilterCriteria();

        if (pSfc == IMS_NULL)
        {
            continue;
        }

        if (pSfc->IsCalleePreferenceSupported(objMethod))
        {
            objCalleePreferenceServices.Append(pService);
        }
    }
}

PRIVATE GLOBAL void SipConnectionNotifierManagerPrivate::GetRejectCode(
        IN ISipServerConnection* piSsc, IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nLogInfo,
        OUT SipStatusCode& objStatusCode)
{
    objStatusCode = nStatusCode;

    SipFactoryProxy* pFactoryProxy = SipFactoryProxy::GetInstance();

    if (pFactoryProxy->IsRoutingRejectNotifierEnabled(piSsc->GetSlotId()))
    {
        SipRoutingRejectNotifier* pRoutingRejectNotifier =
                pFactoryProxy->GetRoutingRejectNotifier(piSsc->GetSlotId());
        pRoutingRejectNotifier->NotifyRequestReject(piSsc, objStatusCode);

        if (objStatusCode != nStatusCode)
        {
            IMS_TRACE_D("SIPRoutingReject(%d) :: Status code is overwritten (%d >> %d)", nLogInfo,
                    nStatusCode, objStatusCode.ToInt());
        }
    }
}

PRIVATE GLOBAL void SipConnectionNotifierManagerPrivate::HandleSipRequest(
        IN ISipConnectionNotifier* piScn, IN IMS_BOOL bIsForked /*= IMS_FALSE*/)
{
    if (piScn == IMS_NULL)
    {
        return;
    }

    ISipDialog* piOrigDialog = IMS_NULL;
    ISipServerConnection* piSsc = IMS_NULL;

    if (bIsForked)
    {
        piSsc = piScn->AcceptAndOpen(piOrigDialog);
    }
    else
    {
        piSsc = piScn->AcceptAndOpen();
    }

    if (piSsc == IMS_NULL)
    {
        IMS_TRACE_E(0, "Accepting & opening SIP server connection is failed", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("___ %s REQUEST RECEIVED ___", piSsc->GetMethod().ToString().GetStr(), 0, 0);

    // Check the message validity
    AString strReasonPhrase(AString::ConstNull());

    if (!CheckMessageValidity(piSsc->GetMessage(), strReasonPhrase))
    {
        SendResponse(piScn, piSsc, SipStatusCode::SC_400, strReasonPhrase);
        piSsc->Close();
        return;
    }

    // Parse the message body if it is a multipart body
    if (!SipParsingHelper::CreateMessageBodyParts(piSsc->GetMessage()))
    {
        IMS_TRACE_E(0, "Parsing a message body part failed", 0, 0, 0);

        SendResponse(piScn, piSsc, SipStatusCode::SC_400);
        piSsc->Close();
        return;
    }

    // First, check if the message is CANCEL request ...
    if (piSsc->GetMethod().Equals(SipMethod::CANCEL))
    {
        IMS_BOOL bHandled = CancellableMethodManager::GetInstance()->HandleCancelRequest(piSsc);

        if (!bHandled)
        {
            // Send 481 response
            IMS_TRACE_D("SipConnectionNotifierManager :: Sending 481 response "
                        "to CANCEL request ...",
                    0, 0, 0);

            SendResponse(piScn, piSsc, SipStatusCode::SC_481);
            piSsc->Close();
        }

        return;
    }

    IMS_BOOL bRequestWithinDialog = IMS_FALSE;
    ISipDialog* piDialog = piSsc->GetDialog();

    if (piDialog != IMS_NULL)
    {
        IMS_SINT32 nDState = piDialog->GetState();

        if ((nDState == ISipDialog::STATE_EARLY) || (nDState == ISipDialog::STATE_CONFIRMED))
        {
            bRequestWithinDialog = IMS_TRUE;
        }
        else if (nDState == ISipDialog::STATE_TERMINATED)
        {
            const SipMethod& objMethod = piSsc->GetMethod();

            // To guard the race condition of re-INVITE/non-2xx & BYE request
            if (objMethod.Equals(SipMethod::BYE) || objMethod.Equals(SipMethod::NOTIFY) ||
                    objMethod.Equals(SipMethod::INVITE) || objMethod.Equals(SipMethod::UPDATE) ||
                    objMethod.Equals(SipMethod::REFER))
            {
                IMS_TRACE_D("SipConnectionNotifierManager :: Dialog is already terminated, "
                            "but try to route the message to the proper service method",
                        0, 0, 0);
                bRequestWithinDialog = IMS_TRUE;
            }
        }
    }

    // SIP request received within a SIP dialog
    if (bRequestWithinDialog)
    {
        IMS_BOOL bHandled;

        if (bIsForked)
        {
            bHandled = ForkedDialogMethodManager::GetInstance()->HandleRequestWithinDialog(
                    piSsc, piOrigDialog);
            piOrigDialog->Destroy();
        }
        else
        {
            bHandled = DialogMethodManager::GetInstance()->HandleRequestWithinDialog(piSsc);
        }

        if (!bHandled)
        {
            if (piSsc->GetMethod().Equals(SipMethod::ACK))
            {
                IMS_TRACE_D("SipConnectionNotifierManager :: ACK request is received, "
                            "but no dialog(%s) matched ...",
                        SipDebug::GetStr1(piDialog->GetDialogId(), 8, '@').GetStr(), 0, 0);
            }
            else
            {
                // Send 481 response
                IMS_TRACE_D("SipConnectionNotifierManager :: Sending 481 response "
                            "to %s request within the dialog (%s) ...",
                        piSsc->GetMethod().ToString().GetStr(),
                        SipDebug::GetStr1(piDialog->GetDialogId(), 8, '@').GetStr(), 0);

                SendResponse(piScn, piSsc, SipStatusCode::SC_481);
            }

            piSsc->Close();
        }
    }
    // New incoming SIP request received
    else
    {
        SipStatusCode objStatusCode;

        // Find an appropriate service from the incoming SIP request message
        Service* pService = RouteSipRequest(piSsc, objStatusCode);

        if (pService != IMS_NULL)
        {
            if (!pService->NotifyRequest(piSsc))
            {
                IMS_TRACE_D("REQUEST (%s) IS NOT HANDLED or GOT AN ERROR DURING PROCESSING",
                        piSsc->GetMethod().ToString().GetStr(), 0, 0);

                piSsc->Close();
            }
        }
        else
        {
            if (piSsc->GetMethod().Equals(SipMethod::ACK))
            {
                piSsc->Close();

                IMS_TRACE_D("SipConnectionNotifierManager :: ACK request is received, ignored...",
                        0, 0, 0);
                return;
            }

            if (objStatusCode == SipStatusCode::SC_INVALID)
            {
                // Default status code is 404
                objStatusCode = SipStatusCode::SC_404;
            }

            // Send 404 response (480 ???)
            IMS_TRACE_D("SipConnectionNotifierManager :: Sending %d response to %s request ...",
                    objStatusCode.ToInt(), piSsc->GetMethod().ToString().GetStr(), 0);

            SendResponse(
                    piScn, piSsc, objStatusCode.ToInt(), objStatusCode.GetReasonPhrase(), IMS_TRUE);

            piSsc->Close();
        }
    }
}

PRIVATE GLOBAL IMS_BOOL SipConnectionNotifierManagerPrivate::HasService(
        IN const ImsList<Service*>& objServices, IN const Service* pEvaluatedService)
{
    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        const Service* pService = objServices.GetAt(i);

        if (pService == IMS_NULL)
        {
            continue;
        }

        if (pService == pEvaluatedService)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

PRIVATE GLOBAL IMS_BOOL SipConnectionNotifierManagerPrivate::IsCalleePreferenceSupported(
        IN const ImsList<Service*>& objServices, IN const SipMethod& objMethod)
{
    IMS_BOOL bCalleePreference = IMS_FALSE;

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        const Service* pService = objServices.GetAt(i);

        if (pService == IMS_NULL)
        {
            continue;
        }

        ServiceFilterCriteria* pSfc = pService->GetFilterCriteria();

        if (pSfc == IMS_NULL)
        {
            continue;
        }

        if (pSfc->IsCalleePreferenceSupported(objMethod))
        {
            IMS_TRACE_D("Service (%s) supports the callee preference",
                    pService->GetServiceId().GetStr(), 0, 0);

            bCalleePreference = IMS_TRUE;
        }
    }

    return bCalleePreference;
}

PRIVATE GLOBAL Service* SipConnectionNotifierManagerPrivate::RouteSipRequest(
        IN ISipServerConnection* piSsc, OUT SipStatusCode& objStatusCode)
{
    ImsList<Service*> objServices =
            ImsCoreContext::GetInstance()->GetServiceManager()->GetServices();

    if (objServices.IsEmpty())
    {
        GetRejectCode(piSsc, SipStatusCode::SC_404, 1, objStatusCode);
        IMS_TRACE_E(0, "There are no installed services", 0, 0, 0);
        return IMS_NULL;
    }

    // First, check the Request-URI if it matches with the Contact address of the service.
    const SipMethod& objMethod = piSsc->GetMethod();
    const AString& strRequestUri = piSsc->GetRequestUri();
    SipAddress objRequestUri(strRequestUri);

    IMS_TRACE_D(
            "RouteSipRequest :: Request-URI (%s)", SipDebug::GetUri1(strRequestUri).GetStr(), 0, 0);

    if (SipFeatures::IsTransportParameterIgnoredForIncomingRequestRouting(piSsc->GetSlotId()))
    {
        const AString strTransport("transport");
        objRequestUri.RemoveParameter(strTransport);
    }

    for (IMS_UINT32 i = 0; i < objServices.GetSize();)
    {
        Service* pService = objServices.GetAt(i);

        if (pService == IMS_NULL)
        {
            ++i;
            continue;
        }

        // If Service is not yet opened, then skips this service...
        if (!pService->IsImsConnected())
        {
            IMS_TRACE_I("IMS is not connected (%s, %s)", pService->GetAppId().GetStr(),
                    pService->GetServiceId().GetStr(), 0);

            // Drop the service since it is not connected to the IMS network
            objServices.RemoveAt(i);
            continue;
        }

        // Check the method if it supports or not
        if (!pService->ValidateMethod(objMethod))
        {
            IMS_TRACE_D("Method (%s) is not supported in the service (%s)",
                    objMethod.ToString().GetStr(), pService->GetServiceId().GetStr(), 0);

            // Drop the service since the request-uri validation fails.
            objServices.RemoveAt(i);
            continue;
        }

        // Check the Request-URI validation
        IMS_BOOL bValidity = pService->ValidateRequestUri(objRequestUri);

        if (!bValidity &&
                SipFeatures::IsHostPartValidationRequiredForIncomingRequestRouting(
                        piSsc->GetSlotId()))
        {
            // Checks if IP and port is matched with those in the Contact address.
            bValidity = pService->ValidateRequestUriForIpAndPort(objRequestUri);
        }

        if (!bValidity)
        {
            IMS_TRACE_D("Request-URI is not matched (%s)", pService->GetServiceId().GetStr(), 0, 0);

            // Drop the service since the request-uri validation fails.
            objServices.RemoveAt(i);
            continue;
        }

        ++i;
    }

    if (objServices.IsEmpty())
    {
        GetRejectCode(piSsc, SipStatusCode::SC_404, 2, objStatusCode);
        IMS_TRACE_E(0, "There are no matched services", 0, 0, 0);
        return IMS_NULL;
    }

    AStringArray objAcceptContacts = piSsc->GetMessage()->GetHeaders(ISipHeader::ACCEPT_CONTACT);

    if (objAcceptContacts.IsEmpty())
    {
        // No Accept-Contact headers in the incoming SIP request
        Service* pService = RouteSipRequestByIfc(objServices, piSsc);

        if (pService == IMS_NULL)
        {
            GetRejectCode(piSsc, SipStatusCode::SC_404, 3, objStatusCode);
            return IMS_NULL;
        }

        return pService;
    }

    ImsList<Service*> objCalleePreferenceServices;
    GetCalleePreferenceSupportedServices(objServices, objMethod, objCalleePreferenceServices);

    ImsList<PreferenceHeader*> objPreferenceHeaders;
    CreatePreferenceHeaders(objAcceptContacts, objPreferenceHeaders);

    ImsList<Service*> objCandidates;
    ImsList<FeatureSet*> objExtraFeatures;
    IMS_SINT32* pnScore = new IMS_SINT32[objServices.GetSize()];

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        pnScore[i] = 0;

        Service* pService = objServices.GetAt(i);

        if (pService == IMS_NULL)
        {
            continue;
        }

        // If Service is not yet opened, then skips this service...
        if (!pService->IsImsConnected())
        {
            IMS_TRACE_I("CallerPreference :: IMS is not connected (%s, %s)",
                    pService->GetAppId().GetStr(), pService->GetServiceId().GetStr(), 0);
            continue;
        }

        AppConfig* pAppConfig = pService->GetAppConfig();
        const CoreServiceConfig* pServiceConfig = pService->GetServiceConfig();

        CreateExtraFeatures(pService, objExtraFeatures);

        IMS_SINT32 nScore = CallerPreference::GetCandidateScore(
                pAppConfig, pServiceConfig, objPreferenceHeaders, objExtraFeatures);

        if (nScore != CallerPreference::SCORE_INVALID)
        {
            if (objCandidates.Append(pService))
            {
                pnScore[objCandidates.GetSize() - 1] = nScore;
            }
        }
        else if (HasService(objCalleePreferenceServices, pService))
        {
            IMS_TRACE_D("Service(%s) is candidated by callee preference",
                    pService->GetServiceId().GetStr(), 0, 0);

            if (objCandidates.Append(pService))
            {
                pnScore[objCandidates.GetSize() - 1] = 0;
            }
        }

        DestroyExtraFeatures(objExtraFeatures);
    }

    // Destroy the preference headers
    DestroyPreferenceHeaders(objPreferenceHeaders);

    if (objCandidates.IsEmpty())
    {
        delete[] pnScore;

        IMS_TRACE_I("CallerPreference :: No candidated services", 0, 0, 0);

        Service* pService = RouteSipRequestByIfc(objServices, piSsc);

        if (pService == IMS_NULL)
        {
            GetRejectCode(piSsc, SipStatusCode::SC_480, 4, objStatusCode);
            return IMS_NULL;
        }

        return pService;
    }

    Service* pService = objCandidates.GetAt(0);
    IMS_SINT32 nMaxScore = 0;
    IMS_SINT32 nTotalScore = 0;

    for (IMS_UINT32 j = 0; j < objCandidates.GetSize(); ++j)
    {
        nTotalScore += pnScore[j];

        if (pnScore[j] > nMaxScore)
        {
            nMaxScore = pnScore[j];
            pService = objCandidates.GetAt(j);
        }
    }

    delete[] pnScore;

    // If the candidated services are the same priority(0) and greater than one,
    // then the SCN manager try to evaluate the provisioned iFC and
    // if one of them is matched, it overrides the selected service to handle
    // this SIP message.
    if ((nTotalScore == 0) && (objCandidates.GetSize() > 1))
    {
        Service* pServiceByIfc = RouteSipRequestByIfc(objCandidates, piSsc);

        if (pServiceByIfc != IMS_NULL)
        {
            IMS_TRACE_I("CallerPreference :: Service is overridden by iFC", 0, 0, 0);
            pService = pServiceByIfc;
        }
    }

    if (IsCalleePreferenceSupported(objCandidates, objMethod))
    {
        Service* pPreferredService = RouteSipRequestByIfc(objCandidates, piSsc);

        if (pPreferredService != IMS_NULL)
        {
            IMS_TRACE_I("CallerPreference :: Service is overridden by callee preference", 0, 0, 0);
            pService = pPreferredService;
        }
    }

    IMS_TRACE_I("CallerPreference :: SERVICE (%s, %s) IS SELECTED", pService->GetAppId().GetStr(),
            pService->GetServiceId().GetStr(), 0);

    return pService;
}

PRIVATE GLOBAL Service* SipConnectionNotifierManagerPrivate::RouteSipRequestByIfc(
        IN const ImsList<Service*>& objServices, IN ISipServerConnection* piSsc)
{
    if (objServices.IsEmpty())
    {
        return IMS_NULL;
    }

    IMS_UINT32 nScore = 0;
    IMS_UINT32 nMaxScore = 0;
    Service* pBestService = IMS_NULL;

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        Service* pService = objServices.GetAt(i);

        if (pService == IMS_NULL)
        {
            continue;
        }

        if (!pService->IsImsConnected())
        {
            IMS_TRACE_I("iFC :: IMS is not connected (%s, %s)", pService->GetAppId().GetStr(),
                    pService->GetServiceId().GetStr(), 0);
            continue;
        }

        nScore = pService->EvaluateFilterCriteria(piSsc->GetMessage());

        if (nScore == 0)
        {
            continue;
        }

        ServiceFilterCriteria* pSfc = pService->GetFilterCriteria();

        if (pSfc != IMS_NULL)
        {
            if (pSfc->IsCalleePreferenceSupported(piSsc->GetMethod()))
            {
                nScore += CallerPreference::SCORE_CALLEE_PREFERENCE;
            }
        }

        if (nScore > nMaxScore)
        {
            nMaxScore = nScore;
            pBestService = pService;
        }
    }

    if (pBestService != IMS_NULL)
    {
        IMS_TRACE_I("iFC :: SERVICE ([%s, %s]|%d) IS SELECTED", pBestService->GetAppId().GetStr(),
                pBestService->GetServiceId().GetStr(), nMaxScore);
    }

    return pBestService;
}

PRIVATE GLOBAL void SipConnectionNotifierManagerPrivate::SendResponse(
        IN ISipConnectionNotifier* piScn, IN ISipServerConnection* piSsc, IN IMS_SINT32 nStatusCode,
        IN const AString& strReasonPhrase /*= AString::ConstNull()*/,
        IN IMS_BOOL bDebuggableToTag /*= IMS_FALSE*/)
{
    if (piSsc == IMS_NULL)
    {
        return;
    }

    if (piSsc->InitResponse(nStatusCode) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Initializing the response(%d) failed", nStatusCode, 0, 0);
        return;
    }

    if (!strReasonPhrase.IsNULL())
    {
        piSsc->SetReasonPhrase(strReasonPhrase);
    }

    // DEBUG -- starts
    if (bDebuggableToTag)
    {
        // It's for out-of-dialog message.
        AString strTo = piSsc->GetMessage()->GetHeader(ISipHeader::TO);

        if (strTo.GetLength() > 0)
        {
            strTo.Append("_core_sr");
            piSsc->GetMessage()->SetHeader(ISipHeader::TO, strTo);
        }
    }
    // DEBUG -- ends

    SetServerHeader(piScn, piSsc);

    if (piSsc->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending the response failed", 0, 0, 0);
        return;
    }
}

PRIVATE GLOBAL void SipConnectionNotifierManagerPrivate::SetServerHeader(
        IN ISipConnectionNotifier* piScn, IN ISipServerConnection* piSsc)
{
    if (piScn == IMS_NULL)
    {
        return;
    }

    if (piSsc == IMS_NULL)
    {
        return;
    }

    ISipMessage* piSipMsg = piSsc->GetMessage();

    if (piSipMsg == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nSlotId = piSsc->GetSlotId();
    SipProfile* pProfile = piScn->GetSipProfile();

    if (!SipConfigProxy::IsUserAgentConfigured(nSlotId, pProfile))
    {
        return;
    }

    const IpAddress& objIpAddr = piScn->GetLocalAddress();

    if (SipConfigProxy::IsUserAgentSetByContext(nSlotId, pProfile))
    {
        UserAgentHeader::SetHeader(
                ISipHeader::SERVER, pProfile, AString::ConstNull(), objIpAddr, nSlotId, piSipMsg);
    }
    else
    {
        UserAgentHeader::SetHeader(ISipHeader::USER_AGENT, pProfile, AString::ConstNull(),
                objIpAddr, nSlotId, piSipMsg);
    }
}

PUBLIC
SipConnectionNotifierManager::SipConnectionNotifierManager() :
        m_pScnMngrPrivate(new SipConnectionNotifierManagerPrivate())
{
}

PUBLIC VIRTUAL SipConnectionNotifierManager::~SipConnectionNotifierManager()
{
    if (m_pScnMngrPrivate != IMS_NULL)
    {
        delete m_pScnMngrPrivate;
    }
}

PUBLIC VIRTUAL ISipConnectionNotifier* SipConnectionNotifierManager::CreateConnectionNotifier(
        IN const AString& strScheme, IN const IpAddress& objIpAddr, IN IMS_SINT32 nPortS,
        IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl, IN const AString& strParams,
        IN const SipAddress& objUserId)
{
    return m_pScnMngrPrivate->CreateConnectionNotifier(
            strScheme, objIpAddr, nPortS, nPortC, nPortFlowControl, strParams, objUserId);
}

PUBLIC VIRTUAL ISipConnectionNotifier* SipConnectionNotifierManager::GetConnectionNotifier(
        IN const IpAddress& objIpAddr, IN IMS_SINT32 nPort)
{
    return m_pScnMngrPrivate->GetConnectionNotifier(objIpAddr, nPort);
}

PUBLIC VIRTUAL void SipConnectionNotifierManager::ReleaseConnectionNotifier(
        IN ISipConnectionNotifier*& piScn)
{
    m_pScnMngrPrivate->ReleaseConnectionNotifier(piScn);
    piScn = IMS_NULL;
}

PUBLIC VIRTUAL void SipConnectionNotifierManager::Init(IN IMS_SINT32 nSlotId)
{
    m_pScnMngrPrivate->Init(nSlotId);
}
