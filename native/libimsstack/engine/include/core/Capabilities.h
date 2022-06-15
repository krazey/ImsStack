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
#ifndef CAPABILITIES_H_
#define CAPABILITIES_H_

#include "SdpDescription.h"
#include "ServiceMethod.h"

class IOnCapabilitiesListener;
class RemoteCapabilities;

class Capabilities : public ServiceMethod
{
public:
    explicit Capabilities(IN Service* pService);
    virtual ~Capabilities();

    Capabilities(IN const Capabilities&) = delete;
    Capabilities& operator=(IN const Capabilities&) = delete;

public:
    // Method class
    inline void Destroy() override { ServiceMethod::Destroy(); }

    // ICapabilities interface
    IMSList<AString> GetRemoteUserIdentities() const;
    inline IMS_SINT32 GetState() const { return m_nState; }
    IMS_BOOL HasCapabilities(IN const AString& strConnection) const;
    IMS_RESULT QueryCapabilities(IN IMS_BOOL bSdpInRequest,
            IN IMS_BOOL bContactInRequest = IMS_TRUE, IN IMS_BOOL bCheckSupport = IMS_TRUE);
    IMS_RESULT QueryCapabilitiesEx();
    inline void SetListener(IN IOnCapabilitiesListener* piListener) { m_piListener = piListener; }
    IMS_RESULT Accept(
            IN IMS_BOOL bFeatureInContact = IMS_TRUE, IN IMS_BOOL bCheckSupport = IMS_TRUE);
    IMS_RESULT AcceptEx();
    IMS_RESULT Reject(IN IMS_SINT32 nStatusCode, IN IMS_SINT32 nRetryAfter = 0);

    static IMS_RESULT HandleOptionsRequestWithinDialog(
            IN Service* pService, IN const Method* pOwnerMethod, IN ISipServerConnection* piSsc);

protected:
    // Activity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;
    // Method class
    IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piScc) override;
    // Handle the incoming request / outgoing response message
    IMS_BOOL NotifySipRequest(IN ISipServerConnection* piSsc) override;
    // Handle to the outgoing request / incoming response message
    void NotifySipResponse(IN ISipClientConnection* piScc);
    void NotifySipError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

private:
    IMS_BOOL CreateContactHeader(OUT AString& strContactHeader, OUT IMS_BOOL& bIsContactGruu,
            IN IMS_BOOL bWithFeature = IMS_TRUE) const;
    IMS_BOOL CreateSdp(OUT AString& strSdp, IN IMS_BOOL bCheckSupport = IMS_TRUE,
            IN IMS_BOOL bRequest = IMS_FALSE) const;
    void HandleCapabilities(IN ISipClientConnection* piScc);
    IMS_BOOL ParseConnectionName(IN const AString& strConnection, OUT AString& strAppId,
            OUT AString& strServiceId) const;
    void SetState(IN IMS_SINT32 nState);

    static void CollectSdpFieldsFromRegistry(IN const AppConfig* pAppConfig, IN IMS_BOOL bRequest,
            IN IMS_SINT32 nSector, IN_OUT SdpDescription& objSdpDesc);
    static void CopySdpFields(
            IN const SdpDescription& objSdpDesc, IN_OUT SdpDescription& objConcreteSdpDesc);
    static void SetSdpFields(IN const AStringArray& objSdpLines, IN_OUT SdpDescription& objSdpDesc);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    /// Refer to ICapabilities class
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

    IMS_SINT32 m_nState;
    IMSList<AString> m_objRemoteUserIdentities;
    IOnCapabilitiesListener* m_piListener;
    RemoteCapabilities* m_pRemoteCapabilities;
};

#endif
