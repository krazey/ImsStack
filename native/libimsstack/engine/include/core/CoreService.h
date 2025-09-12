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
#ifndef CORE_SERVICE_H_
#define CORE_SERVICE_H_

#include "ReasonInfo.h"
#include "Service.h"

class IOnCoreServiceListener;
class IOnDirectCoreServiceListener;
class ISipConnectionFactory;
class Publication;
class Subscription;
class SessionEx;

class CoreService : public Service
{
public:
    CoreService(IN const AString& strAppId, IN const AString& strServiceId,
            IN const SipAddress* pImpu = IMS_NULL);
    ~CoreService() override;

    CoreService(IN const CoreService&) = delete;
    CoreService& operator=(IN const CoreService&) = delete;

public:
    // IConnection interface
    void Close() override;

    // Service class
    void Abort() override;
    void HandleSessionInvitationReceived(IN Session* pSession) override;
    void HandlePageMessageReceived(IN PageMessage* pPageMessage) override;
    void HandleReferenceReceived(IN Reference* pReference) override;
    void HandleCapabilityQueryReceived(IN Capabilities* pCapabilities) override;

    // ICoreService interface
    Capabilities* CreateCapabilities(IN const AString& strFrom, IN const AString& strTo);
    PageMessage* CreatePageMessage(IN const AString& strFrom, IN const AString& strTo);
    Publication* CreatePublication(
            IN const AString& strFrom, IN const AString& strTo, IN const AString& strEvent);
    Reference* CreateReference(IN const AString& strFrom, IN const AString& strTo,
            IN const AString& strReferTo, IN const AString& strReferMethod);
    Session* CreateSession(IN const AString& strFrom, IN const AString& strTo);
    SessionEx* CreateSessionEx(IN const AString& strFrom, IN const AString& strTo);
    Subscription* CreateSubscription(
            IN const AString& strFrom, IN const AString& strTo, IN const AString& strEvent);
    inline AString GetLocalUserId() const { return GetAuthorizedUserId().ToString(); }
    inline void SetListener(IN IOnCoreServiceListener* piListener)
    {
        m_piCoreServiceListener = piListener;
    }
    ISipConnectionFactory* CreateSipConnectionFactory();
    inline void SetDirectListener(IN IOnDirectCoreServiceListener* piListener)
    {
        m_piDirectCoreServiceListener = piListener;
    }

private:
    void Exception_NotifyError(IN IMS_SINT32 nErrorCode) override;
    IMS_BOOL ServerConnection_NotifyRequest(IN ISipServerConnection* piSsc) override;

    IMS_SINT32 CheckAndHandleDirectSipRequest(IN ISipServerConnection* piSsc);

private:
    /// Refer to ICoreService class
    enum
    {
        /// The transaction will be handled by the owner of direct listener.
        /// The owner has a responsibility of the resource release to SipServerConnection.
        RESULT_DIRECT_TXN_HANDLED = 0,
        /// The transaction is not handled by the owner of direct listener.
        /// The invoker should release the SIP server connection calling close() method.
        RESULT_DIRECT_TXN_NOT_HANDLED = 1,
        /// The transaction is handled by the owner of direct listener.
        /// But, it also should be handled by the default listener.
        /// It's usage is only for SIP message modification after receiving the message.
        RESULT_DIRECT_TXN_BYPASS = 2
    };

    ReasonInfo m_objReasonInfo;
    IOnCoreServiceListener* m_piCoreServiceListener;
    IOnDirectCoreServiceListener* m_piDirectCoreServiceListener;
};

#endif
