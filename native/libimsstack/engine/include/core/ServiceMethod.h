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
#ifndef SERVICE_METHOD_H_
#define SERVICE_METHOD_H_

#include "ImsMap.h"

#include "Message.h"
#include "Service.h"
#include "base/Method.h"

class PreviousMessage;

class ServiceMethod : public Method
{
public:
    explicit ServiceMethod(IN Service* pService);
    ~ServiceMethod() override;

    ServiceMethod(IN const ServiceMethod&) = delete;
    ServiceMethod& operator=(IN const ServiceMethod&) = delete;

public:
    // IMethod interface
    void Destroy() override;

    // IServiceMethod interface
    Message* GetNextRequest();
    Message* GetNextResponse();
    Message* GetPreviousRequest(IN IMS_SINT32 nServiceMethod) const;
    Message* GetPreviousResponse(IN IMS_SINT32 nServiceMethod) const;
    ImsList<Message*> GetPreviousResponses(IN IMS_SINT32 nServiceMethod) const;
    inline ImsList<AString> GetRemoteUserId() const { return GetRemoteUserIds(); }

protected:
    inline const AString& GetSubscriberId() const override { return m_pService->GetSubscriberId(); }

    void ClearConnection(IN IMS_SINT32 nServiceMethod);
    void CloseConnection(IN IMS_SINT32 nServiceMethod);
    void CopyPreviousMessage(IN IMS_SINT32 nSrcServiceMethod, IN IMS_SINT32 nDstServiceMethod);
    ISipClientConnection* CreateCancelConnection(IN ISipClientConnection* piScc);
    ISipClientConnection* CreateConnection(IN const SipMethod& objMethod);
    ISipClientConnection* CreateConnection(IN ISipDialog* piDialog, IN const SipMethod& objMethod);
    IMS_BOOL CreateResponse(IN_OUT ISipServerConnection* piSsc, IN IMS_SINT32 nStatusCode,
            IN const AString& strPhrase = AString::ConstNull());
    inline ISipClientConnection* GetClientConnection(IN IMS_SINT32 nServiceMethod) const
    {
        return DYNAMIC_CAST(ISipClientConnection*, GetConnection(nServiceMethod));
    }
    inline ISipServerConnection* GetServerConnection(IN IMS_SINT32 nServiceMethod) const
    {
        return DYNAMIC_CAST(ISipServerConnection*, GetConnection(nServiceMethod));
    }
    inline Service* GetService() const { return m_pService; }
    IMS_BOOL IsPrivacyRequested(IN IMS_BOOL bRequest = IMS_TRUE) const;
    inline IMS_BOOL IsServiceOpen() const { return m_pService->IsImsConnected(); }
    IMS_BOOL RemovePreviousMessage(IN IMS_SINT32 nServiceMethod);
    inline IMS_BOOL SendNUpdateRequest(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc)
    {
        return SendNUpdateRequestEx(nServiceMethod, piSc, MESSAGE_CLASS_NORMAL);
    }
    inline IMS_BOOL SendNUpdateResponse(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc)
    {
        return SendNUpdateResponseEx(nServiceMethod, piSc, MESSAGE_CLASS_NORMAL);
    }
    IMS_BOOL SendNUpdateRequestEx(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc,
            IN IMS_SINT32 nMessageClass = MESSAGE_CLASS_NORMAL);
    IMS_BOOL SendNUpdateResponseEx(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc,
            IN IMS_SINT32 nMessageClass = MESSAGE_CLASS_NORMAL);
    void UpdateConnection(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc);
    IMS_BOOL UpdateRequestOnReceived(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc);
    IMS_BOOL UpdateRequestOnSent(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc);
    IMS_BOOL UpdateResponseOnReceived(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc);
    IMS_BOOL UpdateResponseOnSent(IN IMS_SINT32 nServiceMethod, IN ISipConnection* piSc);

private:
    IMS_BOOL AddPreviousResponse(IN IMS_SINT32 nServiceMethod, IN Message* pMessage);
    IMS_BOOL SetPreviousRequest(
            IN IMS_SINT32 nServiceMethod, IN Message* pMessage, IN ISipConnection* piSc);
    ISipConnection* GetConnection(IN IMS_SINT32 nServiceMethod) const;

private:
    // Reference to Service
    Service* m_pService;
    // Message object to outgoing SIP request message
    Message* m_pNextRequest;
    // Message object to outgoing SIP response message
    Message* m_pNextResponse;
    // Storage for the previous SIP message according to the method type
    ImsMap<IMS_SINT32, PreviousMessage*> m_objPreviousMessages;
};

#endif
