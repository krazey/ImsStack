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
#ifndef PUBLICATION_H_
#define PUBLICATION_H_

#include "ServiceMethod.h"
#include "util/IRefreshable.h"

class IOnPublicationListener;
class IRefreshListener;
class PubState;
class PublicationRefreshHelper;

class Publication : public ServiceMethod, public IRefreshable
{
public:
    Publication(IN Service* pService, IN const AString& strEvent,
            IN IMS_BOOL bImplicitRoutingRequired = IMS_TRUE);
    ~Publication() override;

    Publication(IN const Publication&) = delete;
    Publication& operator=(IN const Publication&) = delete;

public:
    // Method class
    void Destroy() override;
    void SetMessageMediator(IN IMessageMediator* piMediator) override;

    // IPublication interface
    inline const AString& GetEvent() const { return m_strEvent; }
    inline IMS_SINT32 GetState() const { return m_nState; }
    IMS_RESULT Publish(IN const ByteArray& objState, IN const AString& strContentType);
    inline void SetListener(IN IOnPublicationListener* piListener) { m_piListener = piListener; }
    IMS_RESULT Unpublish();
    inline void SetRefreshListener(IN IRefreshListener* piListener)
    {
        m_piRefreshListener = piListener;
    }
    void SetRefreshPolicy(IN IMS_SINT32 nPolicy, IN IMS_SINT32 nCriteriaInterval,
            IN IMS_SINT32 nValueEorLt, IN IMS_SINT32 nValueGt);

protected:
    // Activity class
    IMS_BOOL DispatchMessage(IN ImsMessage& objMsg) override;

    // Method class
    IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piScc) override;
    // Handle the exceptions
    void Exception_NotifyError(IN IMS_SINT32 nErrorCode) override;
    IMS_BOOL InitInstance() override;
    // Handle to the outgoing request / incoming response message
    void NotifySipResponse(IN ISipClientConnection* piScc) override;
    void NotifySipError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

    // IRefreshable interface
    void Refreshable_RefreshCompleted(
            IN ISipClientConnection* piScc, IN IMS_SINT32 nCode = 0) override;
    IMS_BOOL Refreshable_RefreshStarted() override;
    void Refreshable_RefreshTerminated() override;

private:
    void CloseConnection();
    ISipClientConnection* CreateConnectionWithDialog(
            IN ISipDialog* piDialog, IN const SipMethod& objMethod);
    void ReceiveResponse(IN ISipClientConnection* piScc);
    void SetState(IN IMS_SINT32 nState);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    /// Refer to IPublication interface
    enum
    {
        STATE_INACTIVE = 1,
        STATE_PENDING = 2,
        STATE_ACTIVE = 3
    };

    /// Policy for publication refresh
    enum
    {
        /// No refresh by engine
        REFRESH_POLICY_NO_REFRESH = (-1),

        /// Default policy; Select the refresh time according to 3GPP spec.
        ///     nCriteriaInterval : Criteria value for the refresh duration
        ///    nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100; default 50)
        ///    nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        REFRESH_POLICY_SPEC = 0,

        /// Set the remain time before it is expired
        ///    nCriteriaInterval : Criteria value for the refresh duration
        ///    nValueEorLT : Interval value when the refresh duration is equal or less
        ///              than the criteria interval
        ///    nValueGT : Interval value when the refresh duration is greater
        ///              than the criteria interval
        REFRESH_POLICY_REMAIN_TIME,

        /// Set the ratio before it is expired
        ///    nCriteriaInterval : Criteria value for the refresh duration
        ///    nValueEorLT : Ratio when the refresh duration is equal or less
        ///              than the criteria interval (1 ~ 100)
        ///    nValueGT : Ratio when the refresh duration is greater
        ///              than the criteria interval (1 ~ 100)
        /// Ex) Expires: 3600, Ratio: 10
        ///        -> Refresh timer is expired after 3240s
        REFRESH_POLICY_RATIO
    };

protected:
    enum
    {
        AMSG_PUBLICATION_DELIVERED = AMSG_USER,
        AMSG_PUBLICATION_DELIVERY_FAILED,
        AMSG_PUBLICATION_TERMINATED,
        AMSG_PUBLICATION_REFRESH_STARTED,
        AMSG_PUBLICATION_REFRESH_COMPLETED,
        AMSG_PUBLICATION_MAX
    };

private:
    // State of Publication
    IMS_SINT32 m_nState;
    // Event package name
    AString m_strEvent;
    // Listener for this publication
    IOnPublicationListener* m_piListener;
    // Publication state information
    PubState* m_pPubState;
    // Publication refresh timer
    IRefreshListener* m_piRefreshListener;
    PublicationRefreshHelper* m_pRefreshHelper;
    // IMPLICIT_ROUTING_FOR_MID_DIALOG
    IMS_BOOL m_bImplicitRoutingRequired;
};

#endif
