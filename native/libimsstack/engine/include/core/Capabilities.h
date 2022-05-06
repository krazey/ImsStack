/*
    Author
    <table>
    date          author                    description
    --------      --------------            ----------
    20091201    toastops@               Created
    </table>

    Description

*/

#ifndef _CAPABILITIES_H_
#define _CAPABILITIES_H_

#include "SdpDescription.h"
#include "ServiceMethod.h"

class IOnCapabilitiesListener;
class RemoteCapabilities;

class Capabilities : public ServiceMethod
{
public:
    explicit Capabilities(IN Service* pService_);
    virtual ~Capabilities();

private:
    Capabilities(IN CONST Capabilities& objRHS);
    Capabilities& operator=(IN CONST Capabilities& objRHS);

public:
    // Method class
    virtual void Destroy();

    // ICapabilities interface
    IMSList<AString> GetRemoteUserIdentities() const;
    IMS_SINT32 GetState() const;
    IMS_BOOL HasCapabilities(IN CONST AString& strConnection) const;
    IMS_RESULT QueryCapabilities(IN IMS_BOOL bSDPInRequest,
            IN IMS_BOOL bContactInRequest = IMS_TRUE, IN IMS_BOOL bCheckSupport = IMS_TRUE);
    IMS_RESULT QueryCapabilitiesEx();
    void SetListener(IN IOnCapabilitiesListener* piListener);

    //// IMS extensions
    IMS_RESULT Accept(
            IN IMS_BOOL bFeatureInContact = IMS_TRUE, IN IMS_BOOL bCheckSupport = IMS_TRUE);
    IMS_RESULT AcceptEx();
    IMS_RESULT Reject(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter = 0);

    static IMS_RESULT HandleOPTIONSRequestWithinDialog(
            IN Service* pService, IN CONST Method* pOwnerMethod, IN ISipServerConnection* piSSC);

protected:
    // Activity class
    virtual IMS_BOOL DispatchMessage(IN IMSMSG& objMSG);

    // Method class
    // IMS_AUTH_SIP_DIGEST
    virtual IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piSCC);

    // Handle the incoming request / outgoing response message
    virtual IMS_BOOL NotifySIPRequest(IN ISipServerConnection* piSSC);

    // Handle to the outgoing request / incoming response message
    virtual void NotifySIPResponse(IN ISipClientConnection* piSCC);
    virtual void NotifySIPError(
            IN ISipConnection* piSC, IN IMS_SINT32 nCode, IN CONST AString& strMessage);

private:
    IMS_BOOL CreateContactHeader(OUT AString& strContactHeader, OUT IMS_BOOL& bIsContactGRUU,
            IN IMS_BOOL bWithFeature = IMS_TRUE) const;
    IMS_BOOL CreateSDP(OUT AString& strSDP, IN IMS_BOOL bCheckSupport = IMS_TRUE,
            IN IMS_BOOL bRequest = IMS_FALSE) const;
    void HandleCapabilities(IN ISipClientConnection* piSCC);
    IMS_BOOL ParseConnectionName(IN CONST AString& strConnection, OUT AString& strAppId,
            OUT AString& strServiceId) const;
    void SetState(IN IMS_SINT32 nState);

    static void CollectSDPFieldsFromRegistry(IN CONST AppConfig* pAppConfig, IN IMS_BOOL bRequest,
            IN IMS_SINT32 nSector, IN_OUT SdpDescription& objSDPDesc);
    static void CopySDPFields(
            IN CONST SdpDescription& objSDPDesc, IN_OUT SdpDescription& objConcreteSDPDesc);
    static void SetSDPFields(IN CONST AStringArray& objSDPLines, IN_OUT SdpDescription& objSDPDesc);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    // Refer to ICapabilities class
    enum
    {
        STATE_INACTIVE = 1,
        STATE_PENDING = 2,
        STATE_ACTIVE = 3
    };

protected:
    enum
    {
        AMSG_CAPABILITY_QUERY_RECEIVED = AMSG_USER,
        AMSG_CAPABILITY_QUERY_DELIVERED,
        AMSG_CAPABILITY_QUERY_DELIVERY_FAILED,
        AMSG_CAPABILITY_QUERY_MAX
    };

private:
    static const IMS_CHAR DEFAULT_MEDIA_TYPE[];

    IMS_SINT32 nState;
    IMSList<AString> objRemoteUserIdentities;
    IOnCapabilitiesListener* piListener;

    RemoteCapabilities* pRemoteCapabilities;
};

#endif  // _CAPABILITIES_H_
