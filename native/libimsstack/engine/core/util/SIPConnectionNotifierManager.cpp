/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100603  hwangoo.park@             Created
    </table>

    Description

*/

#include "ServiceMemory.h"
#include "ServiceTrace.h"
#include "ServiceMutex.h"
#include "SystemConfig.h"
#include "IMSMap.h"
#include "EngineActivity.h"
#include "Connector.h"
#include "ISipHeader.h"
#include "ISipMessage.h"
#include "ISipDialog.h"
#include "ISipServerConnection.h"
#include "ISipConnectionNotifier.h"
#include "ISipServerConnectionListener.h"
#include "Sip.h"
#include "SipDebug.h"
#include "SipFeatures.h"
#include "SipParsingHelper.h"
#include "SipStatusCode.h"
#include "SipHeaderName.h"
#include "SipParameter.h"
#include "SipConfigProxy.h"
#include "SIPFactoryProxy.h"
#include "SipRoutingRejectNotifier.h"
#include "Feature.h"
#include "util/CallerPreference.h"
#include "util/PreferenceHeader.h"
#include "util/UserAgentHeader.h"
#include "Service.h"
#include "ServiceManager.h"
#include "util/CancellableMethodManager.h"
#include "util/DialogMethodManager.h"
#include "util/ForkedDialogMethodManager.h"
#include "util/SIPConnectionNotifierManager.h"

__IMS_TRACE_TAG_IMS__;



class SIPServerConnectionListenerProxy
    : public EngineActivity
    , public ISIPServerConnectionListener
{
public:
    SIPServerConnectionListenerProxy(
            IN const AString& strName, IN ISIPServerConnectionListener *piListener_);
    virtual ~SIPServerConnectionListenerProxy();

private:
    // Activity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG &objMSG);

    // ISIPServerConnectionListener interface implementations
    virtual void ServerConnection_NotifyRequest(IN ISIPConnectionNotifier *piSCN,
            IN IMS_BOOL bIsForked = IMS_FALSE);

private:
    // MSGs
    enum
    {
        AMSG_SIP_REQUEST_RECEIVED = AMSG_USER,
        AMSG_SIP_FORKED_REQUEST_RECEIVED
    };

    ISIPServerConnectionListener *piListener;
};

PUBLIC
SIPServerConnectionListenerProxy::SIPServerConnectionListenerProxy(
        IN const AString& strName,
        IN ISIPServerConnectionListener *piListener_)
    : EngineActivity(strName)
    , piListener(piListener_)
{
}

PUBLIC VIRTUAL
SIPServerConnectionListenerProxy::~SIPServerConnectionListenerProxy()
{
}

PRIVATE VIRTUAL
IMS_BOOL SIPServerConnectionListenerProxy::DispatchMessage(IN IMSMSG &objMSG)
{
    //---------------------------------------------------------------------------------------------

    switch (objMSG.GetName())
    {
    case AMSG_SIP_REQUEST_RECEIVED:
        piListener->ServerConnection_NotifyRequest(
                reinterpret_cast<ISIPConnectionNotifier*>(objMSG.nLparam));
        return IMS_TRUE;

    case AMSG_SIP_FORKED_REQUEST_RECEIVED:
        piListener->ServerConnection_NotifyRequest(
                reinterpret_cast<ISIPConnectionNotifier*>(objMSG.nLparam), IMS_TRUE);
        return IMS_TRUE;

    default:
        break;
    }

    return EngineActivity::DispatchMessage(objMSG);
}

PRIVATE VIRTUAL
void SIPServerConnectionListenerProxy::ServerConnection_NotifyRequest(IN ISIPConnectionNotifier *piSCN,
        IN IMS_BOOL bIsForked/* = IMS_FALSE*/)
{
    //---------------------------------------------------------------------------------------------

    //4 Use an internal queue to pass the SIP connection notifier

    if (bIsForked)
    {
        PostMessage(AMSG_SIP_FORKED_REQUEST_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(piSCN));
    }
    else
    {
        PostMessage(AMSG_SIP_REQUEST_RECEIVED, 0, reinterpret_cast<IMS_UINTP>(piSCN));
    }
}



class SIPConnectionNotifierManagerPrivate
    : public ISIPServerConnectionListener
{
public:
    SIPConnectionNotifierManagerPrivate();
    virtual ~SIPConnectionNotifierManagerPrivate();

private:
    SIPConnectionNotifierManagerPrivate(IN const SIPConnectionNotifierManagerPrivate& objRHS);
    SIPConnectionNotifierManagerPrivate& operator=(
            IN const SIPConnectionNotifierManagerPrivate& objRHS);

public:
    void Init(IN IMS_SINT32 nSlotId);

    ISIPConnectionNotifier* CreateConnectionNotifier(
            IN CONST AString &strScheme, IN CONST IPAddress &objIPA, IN IMS_SINT32 nPortS,
            IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl,
            IN CONST AString &strParams, IN CONST SIPAddress &objUserId);

    ISIPConnectionNotifier* GetConnectionNotifier(IN CONST IPAddress &objIP,
            IN IMS_SINT32 nPort);

    void AddConnectionNotifier(IN const AString& strKey, IN ISIPConnectionNotifier *piSCN);
    void ReleaseConnectionNotifier(IN ISIPConnectionNotifier *piSCN);

private:
    // ISIPServerConnectionListener interface implementations
    virtual void ServerConnection_NotifyRequest(IN ISIPConnectionNotifier *piSCN,
            IN IMS_BOOL bIsForked = IMS_FALSE);

    void AddReference(IN CONST AString &strKey);
    IMS_SINT32 RemoveReference(IN CONST AString &strKey);
    ISIPConnectionNotifier* GetConnectionNotifier(IN CONST AString &strKey);
    SIPServerConnectionListenerProxy* GetServerConnectionListener(IN IMS_SINT32 nSlotId);
    IMS_BOOL IsConnectionNotifierPresent(IN ISIPConnectionNotifier *piSCN) const;

    static IMS_BOOL CheckMessageValidity(IN ISIPMessage *piSIPMsg, OUT AString &strReason);
    static AString CreateConnectionNotifierKey(IN CONST IPAddress &objIP, IN IMS_SINT32 nPort);
    static void CreateExtraFeatures(IN Service *pService, OUT IMSList<FeatureSet*> &objFeatures);
    static void CreatePreferenceHeaders(IN CONST AStringArray &objAcceptContacts,
            OUT IMSList<PreferenceHeader*> &objHeaders);
    static void DestroyExtraFeatures(OUT IMSList<FeatureSet*> &objFeatures);
    static void DestroyPreferenceHeaders(OUT IMSList<PreferenceHeader*> &objHeaders);
    static void GetCalleePreferenceSupportedServices(IN CONST IMSList<Service*> &objServices,
            IN CONST SIPMethod &objMethod,
            OUT IMSList<Service*> &objCalleePreferenceServices);
    static void GetRejectCode(IN ISIPServerConnection *piSSC,
            IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nLogInfo, OUT SIPStatusCode &objStatusCode);
    static void HandleSIPRequest(IN ISIPConnectionNotifier *piSCN,
            IN IMS_BOOL bIsForked = IMS_FALSE);
    static IMS_BOOL HasService(IN CONST IMSList<Service*> &objServices,
            IN CONST Service *pEvaluatedService);
    static IMS_BOOL IsCalleePreferenceSupported(IN CONST IMSList<Service*> &objServices,
            IN CONST SIPMethod &objMethod);
    static Service* RouteSIPRequest(IN ISIPServerConnection *piSSC,
            OUT SIPStatusCode &objStatusCode);
    static Service* RouteSIPRequestByIFC(
            IN CONST IMSList<Service*> &objServices, IN ISIPServerConnection *piSSC);
    static void SendResponse(IN ISIPConnectionNotifier *piSCN,
            IN ISIPServerConnection *piSSC, IN IMS_SINT32 nStatusCode,
            IN CONST AString &strReasonPhrase = AString::ConstNull(),
            IN IMS_BOOL bDebuggableToTag = IMS_FALSE);
    static void SetServerHeader(IN ISIPConnectionNotifier *piSCN, IN ISIPServerConnection *piSSC);

private:
    IMutex *piLock;
    SIPServerConnectionListenerProxy **ppListenerProxy;
    // < (IP + Port), ISIPConnectionNotifier* >
    IMSMap<AString, ISIPConnectionNotifier*> objConnectionNotifiers;
    // < (IP + Port), Count >
    IMSMap<AString, IMS_SINT32> objReferenceCounts;
};



PUBLIC
SIPConnectionNotifierManagerPrivate::SIPConnectionNotifierManagerPrivate()
    : piLock(IMS_NULL)
    , ppListenerProxy(IMS_NULL)
{
    piLock = MutexService::GetMutexService()->CreateMutex();

    IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

    ppListenerProxy = new SIPServerConnectionListenerProxy*[nSimCount];

    for (IMS_SINT32 i = 0; i < nSimCount; ++i)
    {
        ppListenerProxy[i] = IMS_NULL;
    }
}

PUBLIC VIRTUAL
SIPConnectionNotifierManagerPrivate::~SIPConnectionNotifierManagerPrivate()
{
    if (ppListenerProxy != IMS_NULL)
    {
        IMS_SINT32 nSimCount = SystemConfig::GetMaxSimSlot();

        for (IMS_SINT32 i = 0; i < nSimCount; ++i)
        {
            if (ppListenerProxy[i] != IMS_NULL)
            {
                delete ppListenerProxy[i];
            }
        }

        delete[] ppListenerProxy;
    }

    MutexService::GetMutexService()->DestroyMutex(piLock);
}

/*

Remarks

*/
PUBLIC
void SIPConnectionNotifierManagerPrivate::Init(IN IMS_SINT32 nSlotId)
{
    //---------------------------------------------------------------------------------------------

    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return;
    }

    if (ppListenerProxy[nSlotId] == IMS_NULL)
    {
        AString strName;
        strName.Sprintf("SSCLP_%02d", nSlotId);

        IMS_TRACE_I("SCN :: %s is created", strName.GetStr(), 0, 0);

        ppListenerProxy[nSlotId] = new SIPServerConnectionListenerProxy(strName, this);
    }
}

/*

Remarks

*/
PUBLIC
ISIPConnectionNotifier* SIPConnectionNotifierManagerPrivate::CreateConnectionNotifier(
        IN CONST AString &strScheme, IN CONST IPAddress &objIPA, IN IMS_SINT32 nPortS,
        IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl,
        IN CONST AString &strParams, IN CONST SIPAddress &objUserId)
{
    AString strKey = CreateConnectionNotifierKey(objIPA, nPortS);

    //---------------------------------------------------------------------------------------------

    ISIPConnectionNotifier *piSCN = GetConnectionNotifier(strKey);

    if (piSCN != IMS_NULL)
    {
        // Add the reference count
        AddReference(strKey);

        return piSCN;
    }

    AString strName;
    strName.SetNumber(nPortS);

    piSCN = DYNAMIC_CAST(ISIPConnectionNotifier*, Connector::Open(strScheme, strName, strParams));

    if (piSCN == IMS_NULL)
    {
        IMS_TRACE_E(0, "Creating SIP connection notifier failed", 0, 0, 0);
        return IMS_NULL;
    }

    piSCN->SetListener(GetServerConnectionListener(piSCN->GetSlotId()));

    if (piSCN->ReserveTransportResource(objIPA, nPortS, nPortC, nPortFlowControl) != IMS_SUCCESS)
    {
        piSCN->Close();

        IMS_TRACE_E(0, "Creating a transport resource failed", 0, 0, 0);
        return IMS_NULL;
    }

    // Set a default From & Contact information
    piSCN->SetFromAndContact(objUserId.GetURI(), objUserId.GetDisplayName(), objUserId.GetUser());

    AddConnectionNotifier(strKey, piSCN);
    AddReference(strKey);

    IMS_TRACE_D("SIP Connection Notifier (%s, %d) is created",
            SIPDebug::GetCharA1(strKey.GetStr(), 5), piSCN->GetSlotId(), 0);

    return piSCN;
}

/*

Remarks

*/
PUBLIC
ISIPConnectionNotifier* SIPConnectionNotifierManagerPrivate::GetConnectionNotifier(
        IN CONST IPAddress &objIP, IN IMS_SINT32 nPort)
{
    AString strKey = CreateConnectionNotifierKey(objIP, nPort);
    ISIPConnectionNotifier *piSCN = GetConnectionNotifier(strKey);

    //---------------------------------------------------------------------------------------------

    if (piSCN == IMS_NULL)
    {
        IMS_TRACE_D("SIP Connection Notifier (%s) is not found",
                SIPDebug::GetCharA1(strKey.GetStr(), 5), 0, 0);
        return IMS_NULL;
    }

    AddReference(strKey);

    return piSCN;
}

/*

Remarks

*/
PUBLIC
void SIPConnectionNotifierManagerPrivate::AddConnectionNotifier(
        IN const AString& strKey, IN ISIPConnectionNotifier *piSCN)
{
    LockGuard objLock(piLock);

    //---------------------------------------------------------------------------------------------

    objConnectionNotifiers.Add(strKey, piSCN);
}

/*

Remarks

*/
PUBLIC
void SIPConnectionNotifierManagerPrivate::ReleaseConnectionNotifier(IN ISIPConnectionNotifier *piSCN)
{
    //---------------------------------------------------------------------------------------------

    if (piSCN == IMS_NULL)
    {
        return;
    }

    AString strKey = CreateConnectionNotifierKey(piSCN->GetLocalAddress(), piSCN->GetLocalPort());
    IMS_SINT32 nCount = RemoveReference(strKey);

    if (nCount <= 0)
    {
        LockGuard objLock(piLock);
        objConnectionNotifiers.Remove(strKey);
        objReferenceCounts.Remove(strKey);

        // Close the SIP connection notifier
        piSCN->Close();
    }
}

/*

Remarks

*/
PRIVATE VIRTUAL
void SIPConnectionNotifierManagerPrivate::ServerConnection_NotifyRequest(
        IN ISIPConnectionNotifier *piSCN, IN IMS_BOOL bIsForked /* = IMS_FALSE */)
{
    if (!IsConnectionNotifierPresent(piSCN))
    {
        IMS_TRACE_E(0, "SIPConnectionNotifier(%" PFLS_X ") is not present", piSCN, 0, 0);
        return;
    }

    HandleSIPRequest(piSCN, bIsForked);
}

/*

Remarks

*/
PRIVATE
void SIPConnectionNotifierManagerPrivate::AddReference(IN CONST AString &strKey)
{
    LockGuard objLock(piLock);
    IMS_SLONG nIndex = objReferenceCounts.GetIndexOfKey(strKey);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        // It is just created, so add a new reference count
        objReferenceCounts.Add(strKey, 1);
        IMS_TRACE_D("SCN_REF (%s) :: Created",
                SIPDebug::GetCharA1(strKey.GetStr(), 5), 0, 0);
        return;
    }

    IMS_SINT32 &nValue = objReferenceCounts.GetValueAt(nIndex);

    ++nValue;

    IMS_TRACE_D("SCN_REF (%s) :: %d >>> %d",
            SIPDebug::GetCharA1(strKey.GetStr(), 5), nValue - 1, nValue);
}

/*

Remarks

*/
PRIVATE
IMS_SINT32 SIPConnectionNotifierManagerPrivate::RemoveReference(IN CONST AString &strKey)
{
    LockGuard objLock(piLock);
    IMS_SLONG nIndex = objReferenceCounts.GetIndexOfKey(strKey);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return 0;
    }

    IMS_SINT32 &nValue = objReferenceCounts.GetValueAt(nIndex);

    --nValue;

    IMS_TRACE_D("SCN_REF (%s) :: %d >>> %d",
            SIPDebug::GetCharA1(strKey.GetStr(), 5), nValue + 1, nValue);

    return nValue;
}

/*

Remarks

*/
PRIVATE
ISIPConnectionNotifier* SIPConnectionNotifierManagerPrivate::GetConnectionNotifier(
        IN CONST AString &strKey)
{
    LockGuard objLock(piLock);
    IMS_SLONG nIndex = objConnectionNotifiers.GetIndexOfKey(strKey);

    //---------------------------------------------------------------------------------------------

    if (nIndex < 0)
    {
        return IMS_NULL;
    }

    return objConnectionNotifiers.GetValueAt(nIndex);
}

/*

Remarks

*/
PRIVATE
SIPServerConnectionListenerProxy* SIPConnectionNotifierManagerPrivate::GetServerConnectionListener(
        IN IMS_SINT32 nSlotId)
{
    //---------------------------------------------------------------------------------------------

    if ((nSlotId < IMS_SLOT_0) || (nSlotId >= SystemConfig::GetMaxSimSlot()))
    {
        return IMS_NULL;
    }

    return ppListenerProxy[nSlotId];
}

/*

Remarks

*/
PRIVATE
IMS_BOOL SIPConnectionNotifierManagerPrivate::IsConnectionNotifierPresent(
        IN ISIPConnectionNotifier *piSCN) const
{
    LockGuard objLock(piLock);

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objConnectionNotifiers.GetSize(); ++i)
    {
        ISIPConnectionNotifier *piTmpSCN = objConnectionNotifiers.GetValueAt(i);

        if (piTmpSCN == IMS_NULL)
            continue;

        if (piTmpSCN == piSCN)
        {
            return IMS_TRUE;
        }
    }

    return IMS_FALSE;
}

/*

Remarks

*/
PRIVATE GLOBAL
IMS_BOOL SIPConnectionNotifierManagerPrivate::CheckMessageValidity(IN ISIPMessage *piSIPMsg,
        OUT AString &strReason)
{
    // According to the SIP method, check the mandatory header or parameters ...
    const SIPMethod &objMethod = piSIPMsg->GetMethod();

    //---------------------------------------------------------------------------------------------

    // RFC 3891 : Replaces header requirements
    if (objMethod.ToInt() != SIPMethod::INVITE)
    {
        if (piSIPMsg->IsHeaderPresent(ISIPHeader::REPLACES))
        {
            strReason = "Replaces header is present in non-INVITE";
            return IMS_FALSE;
        }
    }

    switch (objMethod.ToInt())
    {
    case SIPMethod::INVITE:
        {
            // Check Replaces header
            if (piSIPMsg->GetHeaderCount(ISIPHeader::REPLACES) > 1)
            {
                strReason = "Multiple Replaces headers are present";
                return IMS_FALSE;
            }
        }
        break;

    case SIPMethod::REFER:
        {
            // Check Refer-To header
            AString strReferTo = piSIPMsg->GetHeader(ISIPHeader::REFER_TO);

            if (strReferTo.IsNULL() || strReferTo.IsEmpty())
            {
                strReason = "Mandatory Header Missing";
                return IMS_FALSE;
            }

            ISIPHeader *piHeader
                    = SIPParsingHelper::CreateHeader(ISIPHeader::REFER_TO, strReferTo);

            if (piHeader == IMS_NULL)
            {
                strReason = "Bad Request";
                return IMS_FALSE;
            }

            const SIPAddress *pAddress = piHeader->GetSIPAddress();

            if (pAddress == IMS_NULL)
            {
                piHeader->Destroy();

                strReason = "Bad Request";
                return IMS_FALSE;
            }

            const SIPParameter *pParameter = pAddress->GetParameter(SIP::STR_METHOD);

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
        }
        break;

    default:
        break;
    }

    return IMS_TRUE;
}

/*

Remarks

*/
PRIVATE GLOBAL
AString SIPConnectionNotifierManagerPrivate::CreateConnectionNotifierKey(IN CONST IPAddress &objIP,
        IN IMS_SINT32 nPort)
{
    AString strKey;

    //---------------------------------------------------------------------------------------------

    strKey.Sprintf("%s:%d", objIP.ToString().GetStr(), nPort);

    return strKey;
}

/*

Remarks

*/
PRIVATE GLOBAL
void SIPConnectionNotifierManagerPrivate::CreateExtraFeatures(IN Service *pService,
        OUT IMSList<FeatureSet*> &objFeatures)
{
    //---------------------------------------------------------------------------------------------

    if (pService == IMS_NULL)
    {
        return;
    }

    // Provide the "+sip.instance" feature if it is supported in the device
    // Q-OS: always enabled
    {
        const SIPParameter *pParameter = pService->GetInstanceParameter();

        if (pParameter != IMS_NULL)
        {
            FeatureSet *pFeatureSet
                    = new FeatureSet(pParameter->GetName(), pParameter->GetValue());

            if (pFeatureSet != IMS_NULL)
            {
                objFeatures.Append(pFeatureSet);
            }
        }
    }
}

/*

Remarks

*/
PRIVATE GLOBAL
void SIPConnectionNotifierManagerPrivate::CreatePreferenceHeaders(
        IN CONST AStringArray &objAcceptContacts, OUT IMSList<PreferenceHeader*> &objHeaders)
{
    //---------------------------------------------------------------------------------------------

    if (objAcceptContacts.IsEmpty())
        return;

    for (IMS_SINT32 i = 0; i < objAcceptContacts.GetCount(); ++i)
    {
        const AString &strHeader = objAcceptContacts.GetElementAt(i);
        ISIPHeader *piSIPHeader
                = SIPParsingHelper::CreateHeader(ISIPHeader::ACCEPT_CONTACT, strHeader);

        if (piSIPHeader == IMS_NULL)
            continue;

        PreferenceHeader *pHeader = new PreferenceHeader(piSIPHeader);

        if (pHeader == IMS_NULL)
        {
            piSIPHeader->Destroy();
            continue;
        }

        if (!objHeaders.Append(pHeader))
        {
            delete pHeader;
        }

        piSIPHeader->Destroy();
    }
}

/*

Remarks

*/
PRIVATE GLOBAL
void SIPConnectionNotifierManagerPrivate::DestroyExtraFeatures(
        OUT IMSList<FeatureSet*> &objFeatures)
{
    //---------------------------------------------------------------------------------------------

    if (objFeatures.IsEmpty())
        return;

    for (IMS_UINT32 i = 0; i < objFeatures.GetSize(); ++i)
    {
        FeatureSet *pFeatureSet = objFeatures.GetAt(i);

        if (pFeatureSet != IMS_NULL)
            delete pFeatureSet;
    }

    objFeatures.Clear();
}

/*

Remarks

*/
PRIVATE GLOBAL
void SIPConnectionNotifierManagerPrivate::DestroyPreferenceHeaders(
        OUT IMSList<PreferenceHeader*> &objHeaders)
{
    //---------------------------------------------------------------------------------------------

    if (objHeaders.IsEmpty())
        return;

    for (IMS_UINT32 i = 0; i < objHeaders.GetSize(); ++i)
    {
        PreferenceHeader *pHeader = objHeaders.GetAt(i);

        if (pHeader != IMS_NULL)
            delete pHeader;
    }

    objHeaders.Clear();
}

/*

Remarks

*/
PRIVATE GLOBAL
void SIPConnectionNotifierManagerPrivate::GetCalleePreferenceSupportedServices(
        IN CONST IMSList<Service*> &objServices, IN CONST SIPMethod &objMethod,
        OUT IMSList<Service*> &objCalleePreferenceServices)
{
    //---------------------------------------------------------------------------------------------

    // In the moment, OPTIONS method will be only handled for this request.
    if (!objMethod.Equals(SIPMethod::OPTIONS))
    {
        // no-op
        return;
    }

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        Service *pService = objServices.GetAt(i);

        if (pService == IMS_NULL)
        {
            continue;
        }

        ServiceFilterCriteria *pSFC = pService->GetFilterCriteria();

        if (pSFC == IMS_NULL)
        {
            continue;
        }

        if (pSFC->IsCalleePreferenceSupported(objMethod))
        {
            objCalleePreferenceServices.Append(pService);
        }
    }
}

/*

Remarks

*/
PRIVATE GLOBAL
void SIPConnectionNotifierManagerPrivate::GetRejectCode(IN ISIPServerConnection *piSSC,
        IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nLogInfo, OUT SIPStatusCode &objStatusCode)
{
    //---------------------------------------------------------------------------------------------

    objStatusCode = nStatusCode;

    SIPFactoryProxy* pFactoryProxy = SIPFactoryProxy::GetInstance();

    if (pFactoryProxy->IsRoutingRejectNotifierEnabled(piSSC->GetSlotId()))
    {
        SIPRoutingRejectNotifier* pRoutingRejectNotifier
                = pFactoryProxy->GetRoutingRejectNotifier(piSSC->GetSlotId());
        pRoutingRejectNotifier->NotifyRequestReject(piSSC, objStatusCode);

        if (objStatusCode != nStatusCode)
        {
            IMS_TRACE_D("SIPRoutingReject(%d) :: Status code is overwritten (%d >> %d)",
                    nLogInfo, nStatusCode, objStatusCode.ToInt());
        }
    }
}

/*

Remarks

*/
PRIVATE GLOBAL
void SIPConnectionNotifierManagerPrivate::HandleSIPRequest(IN ISIPConnectionNotifier *piSCN,
        IN IMS_BOOL bIsForked /* = IMS_FALSE */)
{
    //---------------------------------------------------------------------------------------------

    if (piSCN == IMS_NULL)
        return;

    ISIPDialog *piOrigDialog = IMS_NULL;
    ISIPServerConnection *piSSC = IMS_NULL;

    if (bIsForked)
    {
        piSSC = piSCN->AcceptAndOpen(piOrigDialog);
    }
    else
    {
        piSSC = piSCN->AcceptAndOpen();
    }

    if (piSSC == IMS_NULL)
    {
        IMS_TRACE_E(0, "Accepting & opening SIP server connection is failed", 0, 0, 0);
        return;
    }

    IMS_TRACE_I("___ %s REQUEST RECEIVED ___", piSSC->GetMethod().ToString().GetStr(), 0, 0);

    // Check the message validity
    AString strReasonPhrase(AString::ConstNull());

    if (!CheckMessageValidity(piSSC->GetMessage(), strReasonPhrase))
    {
        SendResponse(piSCN, piSSC, SIPStatusCode::SC_400, strReasonPhrase);
        piSSC->Close();
        return;
    }

    // Parse the message body if it is a multipart body
    if (!SIPParsingHelper::CreateMessageBodyParts(piSSC->GetMessage()))
    {
        IMS_TRACE_E(0, "Parsing a message body part failed", 0, 0, 0);

        SendResponse(piSCN, piSSC, SIPStatusCode::SC_400);
        piSSC->Close();
        return;
    }

    // First, check if the message is CANCEL request ...
    if (piSSC->GetMethod().Equals(SIPMethod::CANCEL))
    {
        IMS_BOOL bHandled = CancellableMethodManager::GetInstance()->HandleCancelRequest(piSSC);

        if (!bHandled)
        {
            // Send 481 response
            IMS_TRACE_D("SIPConnectionNotifierManager :: Sending 481 response " \
                    "to CANCEL request ...", 0, 0, 0);

            SendResponse(piSCN, piSSC, SIPStatusCode::SC_481);
            piSSC->Close();
        }

        return;
    }

    IMS_BOOL bRequestWithinDialog = IMS_FALSE;
    ISIPDialog *piDialog = piSSC->GetDialog();

    if (piDialog != IMS_NULL)
    {
        IMS_SINT32 nDState = piDialog->GetState();

        if ((nDState == ISIPDialog::STATE_EARLY)
                || (nDState == ISIPDialog::STATE_CONFIRMED))
        {
            bRequestWithinDialog = IMS_TRUE;
        }
        else if (nDState == ISIPDialog::STATE_TERMINATED)
        {
            const SIPMethod &objMethod = piSSC->GetMethod();

            // To guard the race condition of re-INVITE/non-2xx & BYE request
            if (objMethod.Equals(SIPMethod::BYE)
                    || objMethod.Equals(SIPMethod::NOTIFY)
                    || objMethod.Equals(SIPMethod::INVITE)
                    || objMethod.Equals(SIPMethod::UPDATE)
                    || objMethod.Equals(SIPMethod::REFER))
            {
                IMS_TRACE_D("SIPConnectionNotifierManager :: Dialog is already terminated, "
                        "but try to route the message to the proper service method", 0, 0, 0);
                bRequestWithinDialog = IMS_TRUE;
            }
        }
    }

    // SIP request received within a SIP dialog
    if (bRequestWithinDialog)
    {
        IMS_BOOL bHandled = IMS_FALSE;

        if (bIsForked)
        {
            bHandled = ForkedDialogMethodManager::GetInstance()->HandleRequestWithinDialog(
                            piSSC, piOrigDialog);
            piOrigDialog->Destroy();
        }
        else
        {
            bHandled = DialogMethodManager::GetInstance()->HandleRequestWithinDialog(piSSC);
        }

        if (!bHandled)
        {
            if (piSSC->GetMethod().Equals(SIPMethod::ACK))
            {
                IMS_TRACE_D("SIPConnectionNotifierManager :: ACK request is received, " \
                        "but no dialog(%s) matched ...",
                        SIPDebug::GetStr1(piDialog->GetDialogID(), 8, '@').GetStr(), 0, 0);
            }
            else
            {
                // Send 481 response
                IMS_TRACE_D("SIPConnectionNotifierManager :: Sending 481 response " \
                        "to %s request within the dialog (%s) ...",
                        piSSC->GetMethod().ToString().GetStr(),
                        SIPDebug::GetStr1(piDialog->GetDialogID(), 8, '@').GetStr(), 0);

                SendResponse(piSCN, piSSC, SIPStatusCode::SC_481);
            }

            piSSC->Close();
        }
    }
    // New incoming SIP request received
    else
    {
        SIPStatusCode objStatusCode;

        // Find an appropriate service from the incoming SIP request message
        Service *pService = RouteSIPRequest(piSSC, objStatusCode);

        if (pService != IMS_NULL)
        {
            if (!pService->NotifyRequest(piSSC))
            {
                IMS_TRACE_D("REQUEST (%s) IS NOT HANDLED or GOT AN ERROR DURING PROCESSING",
                        piSSC->GetMethod().ToString().GetStr(), 0, 0);

                piSSC->Close();
            }
        }
        else
        {
            if (piSSC->GetMethod().Equals(SIPMethod::ACK))
            {
                piSSC->Close();

                IMS_TRACE_D("SIPConnectionNotifierManager :: ACK request is received, ignored...",
                        0, 0, 0);
                return;
            }

            if (objStatusCode == SIPStatusCode::SC_INVALID)
            {
                // Default status code is 404
                objStatusCode = SIPStatusCode::SC_404;
            }

            // Send 404 response (480 ???)
            IMS_TRACE_D("SIPConnectionNotifierManager :: Sending %d response to %s request ...",
                    objStatusCode.ToInt(), piSSC->GetMethod().ToString().GetStr(), 0);

            SendResponse(piSCN, piSSC, objStatusCode.ToInt(),
                    objStatusCode.GetReasonPhrase(), IMS_TRUE);

            piSSC->Close();
        }
    }
}

/*

Remarks

*/
PRIVATE GLOBAL
IMS_BOOL SIPConnectionNotifierManagerPrivate::HasService(
        IN CONST IMSList<Service*> &objServices, IN CONST Service *pEvaluatedService)
{
    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        const Service *pService = objServices.GetAt(i);

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

/*

Remarks

*/
PRIVATE GLOBAL
IMS_BOOL SIPConnectionNotifierManagerPrivate::IsCalleePreferenceSupported(
        IN CONST IMSList<Service*> &objServices, IN CONST SIPMethod &objMethod)
{
    IMS_BOOL bCalleePreference = IMS_FALSE;

    //---------------------------------------------------------------------------------------------

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        const Service *pService = objServices.GetAt(i);

        if (pService == IMS_NULL)
            continue;

        ServiceFilterCriteria *pSFC = pService->GetFilterCriteria();

        if (pSFC == IMS_NULL)
            continue;

        if (pSFC->IsCalleePreferenceSupported(objMethod))
        {
            IMS_TRACE_D("Service (%s) supports the callee preference",
                    pService->GetServiceId().GetStr(), 0, 0);

            bCalleePreference = IMS_TRUE;
        }
    }

    return bCalleePreference;
}

/*

Remarks

*/
PRIVATE GLOBAL
Service* SIPConnectionNotifierManagerPrivate::RouteSIPRequest(IN ISIPServerConnection *piSSC,
        OUT SIPStatusCode &objStatusCode)
{
    IMSList<Service*> objServices = ServiceManager::GetInstance()->GetServices();

    //---------------------------------------------------------------------------------------------

    if (objServices.IsEmpty())
    {
        GetRejectCode(piSSC, SIPStatusCode::SC_404, 1, objStatusCode);
        IMS_TRACE_E(0, "There are no installed services", 0, 0, 0);
        return IMS_NULL;
    }

    // First, check the Request-URI if it matches with the Contact address of the service.
    const SIPMethod &objMethod = piSSC->GetMethod();
    const AString &strRequestURI = piSSC->GetRequestURI();
    SIPAddress objRequestURI(strRequestURI);

    IMS_TRACE_D("RouteSIPRequest :: Request-URI (%s)",
            SIPDebug::GetUri1(strRequestURI).GetStr(), 0, 0);

    if (SIPFeatures::IsTransportParameterIgnoredForIncomingRequestRouting(piSSC->GetSlotId()))
    {
        const AString strTransport("transport");
        objRequestURI.RemoveParameter(strTransport);
    }

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); )
    {
        Service *pService = objServices.GetAt(i);

        if (pService == IMS_NULL)
        {
            ++i;
            continue;
        }

        // If Service is not yet opened, then skips this service...
        if (!pService->IsImsConnected())
        {
            IMS_TRACE_I("IMS is not connected (%s, %s)",
                    pService->GetAppId().GetStr(), pService->GetServiceId().GetStr(), 0);

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
        IMS_BOOL bValidity = pService->ValidateRequestURI(objRequestURI);

        if (!bValidity && SIPFeatures::IsHostPartValidationRequiredForIncomingRequestRouting(
                piSSC->GetSlotId()))
        {
            // Checks if IP and port is matched with those in the Contact address.
            bValidity = pService->ValidateRequestURIForIPAndPort(objRequestURI);
        }

        if (!bValidity)
        {
            IMS_TRACE_D("Request-URI is not matched (%s)",
                    pService->GetServiceId().GetStr(), 0, 0);

            // Drop the service since the request-uri validation fails.
            objServices.RemoveAt(i);
            continue;
        }

        ++i;
    }

    if (objServices.IsEmpty())
    {
        GetRejectCode(piSSC, SIPStatusCode::SC_404, 2, objStatusCode);
        IMS_TRACE_E(0, "There are no matched services", 0, 0, 0);
        return IMS_NULL;
    }

    AStringArray objAcceptContacts = piSSC->GetMessage()->GetHeaders(ISIPHeader::ACCEPT_CONTACT);

    if (objAcceptContacts.IsEmpty())
    {
        // No Accept-Contact headers in the incoming SIP request
        Service *pService = RouteSIPRequestByIFC(objServices, piSSC);

        if (pService == IMS_NULL)
        {
            GetRejectCode(piSSC, SIPStatusCode::SC_404, 3, objStatusCode);
            return IMS_NULL;
        }

        return pService;
    }

    IMSList<Service*> objCalleePreferenceServices;
    GetCalleePreferenceSupportedServices(objServices, objMethod, objCalleePreferenceServices);

    IMSList<PreferenceHeader*> objPreferenceHeaders;
    CreatePreferenceHeaders(objAcceptContacts, objPreferenceHeaders);

    IMSList<Service*> objCandidates;
    IMSList<FeatureSet*> objExtraFeatures;
    IMS_SINT32 *pnScore = new IMS_SINT32[objServices.GetSize()];

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        pnScore[i] = 0;

        Service *pService = objServices.GetAt(i);

        if (pService == IMS_NULL)
            continue;

        // If Service is not yet opened, then skips this service...
        if (!pService->IsImsConnected())
        {
            IMS_TRACE_I("CallerPreference :: IMS is not connected (%s, %s)",
                    pService->GetAppId().GetStr(), pService->GetServiceId().GetStr(), 0);
            continue;
        }

        AppConfig *pAppConfig = pService->GetAppConfig();
        const CoreServiceConfig *pServiceConfig = pService->GetServiceConfig();

        CreateExtraFeatures(pService, objExtraFeatures);

        IMS_SINT32 nScore = CallerPreference::GetCandidateScore(pAppConfig,
                                pServiceConfig, objPreferenceHeaders, objExtraFeatures);

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

        Service *pService = RouteSIPRequestByIFC(objServices, piSSC);

        if (pService == IMS_NULL)
        {
            GetRejectCode(piSSC, SIPStatusCode::SC_480, 4, objStatusCode);
            return IMS_NULL;
        }

        return pService;
    }

    Service *pService = objCandidates.GetAt(0);
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
        Service *pServiceByIFC = RouteSIPRequestByIFC(objCandidates, piSSC);

        if (pServiceByIFC != IMS_NULL)
        {
            IMS_TRACE_I("CallerPreference :: Service is overridden by iFC", 0, 0, 0);
            pService = pServiceByIFC;
        }
    }

    if (IsCalleePreferenceSupported(objCandidates, objMethod))
    {
        Service *pPreferredService = RouteSIPRequestByIFC(objCandidates, piSSC);

        if (pPreferredService != IMS_NULL)
        {
            IMS_TRACE_I("CallerPreference :: Service is overridden by callee preference", 0, 0, 0);
            pService = pPreferredService;
        }
    }

    IMS_TRACE_I("CallerPreference :: SERVICE (%s, %s) IS SELECTED",
            pService->GetAppId().GetStr(), pService->GetServiceId().GetStr(), 0);

    return pService;
}

/*

Remarks

*/
PRIVATE GLOBAL
Service* SIPConnectionNotifierManagerPrivate::RouteSIPRequestByIFC(
        IN CONST IMSList<Service*> &objServices, IN ISIPServerConnection *piSSC)
{
    //---------------------------------------------------------------------------------------------

    if (objServices.IsEmpty())
    {
        return IMS_NULL;
    }

    IMS_UINT32 nScore = 0;
    IMS_UINT32 nMaxScore = 0;
    Service *pBestService = IMS_NULL;

    for (IMS_UINT32 i = 0; i < objServices.GetSize(); ++i)
    {
        Service *pService = objServices.GetAt(i);

        if (pService == IMS_NULL)
            continue;

        if (!pService->IsImsConnected())
        {
            IMS_TRACE_I("iFC :: IMS is not connected (%s, %s)",
                    pService->GetAppId().GetStr(), pService->GetServiceId().GetStr(), 0);
            continue;
        }

        nScore = pService->EvaluateFilterCriteria(piSSC->GetMessage());

        if (nScore == 0)
            continue;

        if (nScore > nMaxScore)
        {
            nMaxScore = nScore;
            pBestService = pService;
        }
    }

    if (pBestService != IMS_NULL)
    {
        IMS_TRACE_I("iFC :: SERVICE (%s, %s) IS SELECTED",
                pBestService->GetAppId().GetStr(), pBestService->GetServiceId().GetStr(), 0);
    }

    return pBestService;
}

/*

Remarks

*/
PRIVATE GLOBAL
void SIPConnectionNotifierManagerPrivate::SendResponse(IN ISIPConnectionNotifier *piSCN,
        IN ISIPServerConnection *piSSC, IN IMS_SINT32 nStatusCode,
        IN CONST AString &strReasonPhrase/* = AString::ConstNull()*/,
        IN IMS_BOOL bDebuggableToTag/* = IMS_FALSE*/)
{
    //---------------------------------------------------------------------------------------------

    if (piSSC == IMS_NULL)
        return;

    if (piSSC->InitResponse(nStatusCode) != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Initializing the response(%d) failed", nStatusCode, 0, 0);
        return;
    }

    if (!strReasonPhrase.IsNULL())
    {
        piSSC->SetReasonPhrase(strReasonPhrase);
    }

    // DEBUG -- starts
    if (bDebuggableToTag)
    {
        // It's for out-of-dialog message.
        AString strTo = piSSC->GetMessage()->GetHeader(ISIPHeader::TO);

        if (strTo.GetLength() > 0)
        {
            strTo.Append("_j281_sr");
            piSSC->GetMessage()->SetHeader(ISIPHeader::TO, strTo);
        }
    }
    // DEBUG -- ends

    SetServerHeader(piSCN, piSSC);

    if (piSSC->Send() != IMS_SUCCESS)
    {
        IMS_TRACE_E(0, "Sending the response failed", 0, 0, 0);
        return;
    }
}

/*

Remarks

*/
PRIVATE GLOBAL
void SIPConnectionNotifierManagerPrivate::SetServerHeader(IN ISIPConnectionNotifier *piSCN,
        IN ISIPServerConnection *piSSC)
{
    //---------------------------------------------------------------------------------------------

    if (piSCN == IMS_NULL)
    {
        return;
    }

    if (piSSC == IMS_NULL)
    {
        return;
    }

    ISIPMessage *piSIPMsg = piSSC->GetMessage();

    if (piSIPMsg == IMS_NULL)
    {
        return;
    }

    IMS_SINT32 nSlotId = piSSC->GetSlotId();
    SIPProfile* pSIPProfile = piSCN->GetSIPProfile();

    if (!SIPConfigProxy::IsUserAgentConfigured(nSlotId, pSIPProfile))
    {
        return;
    }

    const IPAddress& objIP = piSCN->GetLocalAddress();

    if (SIPConfigProxy::IsUserAgentSetByContext(nSlotId, pSIPProfile))
    {
        UserAgentHeader::SetHeader(SIPHeaderName::SERVER,
                pSIPProfile, AString::ConstNull(), objIP, nSlotId, piSIPMsg);
    }
    else
    {
        UserAgentHeader::SetHeader(SIPHeaderName::USER_AGENT,
                pSIPProfile, AString::ConstNull(), objIP, nSlotId, piSIPMsg);
    }
}



PRIVATE
SIPConnectionNotifierManager::SIPConnectionNotifierManager()
    : pSCNMngrP(new SIPConnectionNotifierManagerPrivate())
{
}

PRIVATE
SIPConnectionNotifierManager::~SIPConnectionNotifierManager()
{
    if (pSCNMngrP != IMS_NULL)
    {
        delete pSCNMngrP;
    }
}

/*

Remarks

*/
PUBLIC
ISIPConnectionNotifier* SIPConnectionNotifierManager::CreateConnectionNotifier(
        IN CONST AString &strScheme, IN CONST IPAddress &objIPA, IN IMS_SINT32 nPortS,
        IN IMS_SINT32 nPortC, IN IMS_SINT32 nPortFlowControl,
        IN CONST AString &strParams, IN CONST SIPAddress &objUserId)
{
    //---------------------------------------------------------------------------------------------

    if (pSCNMngrP == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pSCNMngrP->CreateConnectionNotifier(strScheme,
            objIPA, nPortS, nPortC, nPortFlowControl, strParams, objUserId);
}

/*

Remarks

*/
PUBLIC
ISIPConnectionNotifier* SIPConnectionNotifierManager::GetConnectionNotifier(IN CONST IPAddress &objIP,
        IN IMS_SINT32 nPort)
{
    //---------------------------------------------------------------------------------------------

    if (pSCNMngrP == IMS_NULL)
    {
        return IMS_NULL;
    }

    return pSCNMngrP->GetConnectionNotifier(objIP, nPort);
}

/*

Remarks

*/
PUBLIC
void SIPConnectionNotifierManager::ReleaseConnectionNotifier(IN ISIPConnectionNotifier *&piSCN)
{
    //---------------------------------------------------------------------------------------------

    if (pSCNMngrP == IMS_NULL)
    {
        return;
    }

    pSCNMngrP->ReleaseConnectionNotifier(piSCN);

    piSCN = IMS_NULL;
}

/*

Remarks

*/
PUBLIC GLOBAL
SIPConnectionNotifierManager* SIPConnectionNotifierManager::GetInstance()
{
    static SIPConnectionNotifierManager *pSCNMngr = IMS_NULL;

    //---------------------------------------------------------------------------------------------

    if (pSCNMngr == IMS_NULL)
    {
        pSCNMngr = new SIPConnectionNotifierManager();
    }

    return pSCNMngr;
}

/*

Remarks

*/
PUBLIC GLOBAL
void SIPConnectionNotifierManager::Init(IN IMS_SINT32 nSlotId)
{
    SIPConnectionNotifierManager *pSCNMngr = GetInstance();

    //---------------------------------------------------------------------------------------------

    if (pSCNMngr->pSCNMngrP != IMS_NULL)
    {
        pSCNMngr->pSCNMngrP->Init(nSlotId);
    }
}
