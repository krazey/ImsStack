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
#ifndef METHOD_H_
#define METHOD_H_

#include "ImsMap.h"

#include "EngineActivity.h"
#include "ISipClientConnection.h"
#include "ISipClientConnectionListener.h"
#include "ISipErrorListener.h"
#include "Sip.h"
#include "SipAddress.h"
#include "base/IMessageMediator.h"

class IReferredMessageListener;
class ISipGenericChallenge;
class ISipServerConnection;

class Method : public EngineActivity, public ISipClientConnectionListener, public ISipErrorListener
{
public:
    Method();
    ~Method() override;

    Method(IN const Method&) = delete;
    Method& operator=(IN const Method&) = delete;

public:
    // IMethod interface
    virtual void Destroy();
    inline virtual void SetMessageMediator(IN IMessageMediator* piMediator)
    {
        m_piMessageMediator = piMediator;
    }
    /**
     * @brief When any error occurs in the Service, the Service notifies the error
     *        to the specific Method.
     */
    inline virtual void Exception_NotifyError(IN IMS_SINT32 /*nErrorCode*/)
    {
        // The subclass MUST implement this method if the error needs to be handled.
    }
    inline virtual IMS_BOOL SetReferredMessageListener(IN IReferredMessageListener* /*piListener*/)
    {
        // The subclass MUST implement this method if the referred message needs to be handled.
        return IMS_FALSE;
    }

    IMS_BOOL Equals(IN const Method* pMethod) const;
    IMS_BOOL InitMethod(IN const AString& strFrom, IN const AString& strTo,
            IN const SipAddress& objUserAor, IN IMS_BOOL bMobileOriginated = IMS_TRUE);
    IMS_BOOL InitMethod(IN const Method* pMethod, IN IMS_BOOL bMobileOriginated = IMS_TRUE);
    inline ISipDialog* GetDialog() const { return m_piDialog; }
    IMS_BOOL ServerConnection_NotifyRequest(IN ISipServerConnection* piSsc);

protected:
    // Overridable methods
    inline virtual IMS_BOOL InitInstance()
    {
        // The subclass MUST implement this method if an additional initialization work needs.
        return IMS_TRUE;
    }
    inline virtual IMS_BOOL NotifySipRequest(IN ISipServerConnection* /*piSsc*/)
    {
        // The subclass MUST implement this method if an incoming SIP request needs to be handled.
        return IMS_FALSE;
    }
    virtual IMS_BOOL NotifySipForkedResponse(
            IN ISipClientConnection* piScc, IN ISipClientConnection* piForkedScc);
    virtual void NotifySipResponse(IN ISipClientConnection* piScc) = 0;
    virtual void NotifySipError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) = 0;

    inline virtual const AString& GetSubscriberId() const { return AString::ConstNull(); }
    virtual IMS_BOOL SendRequestToChallenge(IN ISipClientConnection* piScc);

    IMS_RESULT AdjustMessage(
            IN_OUT ISipMessage* piSipMsg, IN IMS_SINT32 nMessage = MESSAGE_CLASS_NORMAL);
    void CheckNCreateDialog(IN ISipConnection* piSc, IN IMS_BOOL bDestroy = IMS_FALSE,
            IN IMS_BOOL bTerminatedDialogRequired = IMS_FALSE);
    void DestroyDialog();
    inline const SipAddress* GetUserAor() const { return m_pUserAor; }
    inline const SipAddress* GetRemoteUserAor() const { return m_pRemoteUserAor; }
    inline const ImsList<AString>& GetRemoteUserIds() const { return m_objRemoteUserIds; }
    IMS_BOOL HandleAllSipResponse(IN ISipClientConnection* piScc);
    inline IMS_BOOL IsMobileOriginated() const { return m_bMobileOriginated; }

    // IMS_AUTH_SIP_DIGEST
    void ResetChallengeCount(IN ISipClientConnection* piScc);
    IMS_BOOL RespondToChallenge(IN ISipClientConnection* piScc);
    IMS_BOOL SetChallengeNCredentials(IN ISipClientConnection* piScc);

    void UpdateRemoteUserIds(IN ISipConnection* piSc);

private:
    // ISipClientConnectionListener interface
    void ClientConnection_NotifyResponse(IN ISipClientConnection* piScc,
            IN ISipClientConnection* piForkedScc = IMS_NULL) override;
    // ISipErrorListener interface
    void Error_NotifyError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) override;

public:
    class SccListener : public ISipErrorListener, public ISipClientConnectionListener
    {
    public:
        SccListener();
        ~SccListener() override;

    protected:
        void Error_NotifyError(IN ISipConnection* piSc, IN IMS_SINT32 nCode,
                IN const AString& strMessage) override;

        void ClientConnection_NotifyResponse(IN ISipClientConnection* piScc,
                IN ISipClientConnection* piForkedScc = IMS_NULL) override;
    };

public:
    /// Re-define the message category for Method class
    enum
    {
        /// Message for standalone or mid-dialog
        MESSAGE_CLASS_NORMAL = IMessageMediator::MESSAGE_NORMAL,
        /// Message for re-submitted request only (request for auth. challenge)
        MESSAGE_CLASS_RESUBMIT = IMessageMediator::MESSAGE_RESUBMIT,
        /// Message for refresh operation
        MESSAGE_CLASS_REFRESH = IMessageMediator::MESSAGE_REFRESH,
        /// Message sent automatically by Engine
        MESSAGE_CLASS_AUTOMATIC = IMessageMediator::MESSAGE_AUTOMATIC,
        /// Message sent internally by Engine
        MESSAGE_CLASS_INTERNAL_BYE = IMessageMediator::MESSAGE_INTERNAL_BYE
    };

private:
    // Direction of method
    IMS_BOOL m_bMobileOriginated;

    // Logical identity of the initiator of the request; From header field in SIP message
    SipAddress* m_pUserAor;
    // Logical identity of the recipient of the request; To header field in SIP message
    SipAddress* m_pRemoteUserAor;
    // Remote asserted user identities; from P-Asserted-Identity header in SIP message
    ImsList<AString> m_objRemoteUserIds;
    // Pointer to ISipDialog object
    ISipDialog* m_piDialog;
    // If the authentication is failed for the consecutive three times,
    // then it considers that the method can't be progressing anymore.
    static const IMS_SINT32 MAX_CHALLENGE_COUNT = 2;
    // Authentication challenge which is received from 401/407 response
    // when SIP digest authentication is used
    ISipGenericChallenge* m_piAuthChallenge;
    // Authentication challenge counts
    ImsMap<IMS_SINT32, IMS_SINT32> m_objAuthChallengeMap;
    // It gives a chance to modify the message before sending the SIP message to the network.
    // The application can control the SIP header & message body part using this.
    IMessageMediator* m_piMessageMediator;
};

#endif
