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
#ifndef PUB_STATE_H_
#define PUB_STATE_H_

#include "ISipMessage.h"
#include "SipHeaderProperty.h"
#include "util/EventPackage.h"

/**
 * @brief This class defines a state & behavior for a published event state.
 */
class PubState
{
public:
    PubState();
    virtual ~PubState();

    PubState(IN const PubState&) = delete;
    PubState& operator=(IN const PubState&) = delete;

public:
    void Clear();
    IMS_BOOL CreateEventPackage(IN const AString& strEvent);
    inline IMS_SINT32 GetDuration() const { return m_nPublicationDuration; }
    inline const AString& GetEntityTag() const { return m_strEntityTag; }
    inline EventPackage* GetEventPackage() { return &m_objEventPackage; }
    inline IMS_SINT32 GetOperation() const { return m_nOperation; }
    inline IMS_SINT32 GetState() const { return m_nState; }
    inline IMS_BOOL IsTerminated() const { return (GetState() == STATE_TERMINATED); }
    IMS_BOOL SetHeaders(IN_OUT ISipMessage*& piSipMsg);
    inline void SetOperation(IN IMS_SINT32 nOperation) { m_nOperation = nOperation; }
    IMS_BOOL UpdateState(IN const ISipMessage* piSipMsg);
    IMS_BOOL UpdateStateOnTxnTimerExpired();

private:
    void SetState(IN IMS_SINT32 nState);
    void StoreMessage(IN const ISipMessage* piSipMsg);
    IMS_BOOL UpdateOnPublishRequest(IN const ISipMessage* piSipMsg);
    void UpdateOnPublishResponse(IN const ISipMessage* piSipMsg);

    static const IMS_CHAR* StateToString(IN IMS_SINT32 nState);

public:
    /// State of publication
    enum
    {
        STATE_INIT = 0,
        STATE_PENDING,
        STATE_ACTIVE,
        STATE_TERMINATED
    };

    /// Type of publication operation
    enum
    {
        NO_OPERATION = 0,

        OPERATION_CREATE,
        OPERATION_MODIFY,
        /// Refreshed by the PublicationRefreshHelper
        OPERATION_REFRESH,
        OPERATION_REMOVE
    };

    enum
    {
        NO_EXPIRES = (-1)
    };

    static const SipHeaderProperty RESTRICTED_HEADER_PROPERTIES[];

private:
    // Event package for the publication
    EventPackage m_objEventPackage;
    // State of publication
    IMS_SINT32 m_nState;
    // Current operation for the publication: ADD/MODIFY/REFRESH/REMOVE
    //    Operation        Body        SIP-If-Match        Expires
    //    INITIAL          yes         no                  > 0
    //    REFRESH          no          yes                 > 0
    //    MODIFY           yes         yes                 > 0
    //    REMOVE           no          yes                 0
    IMS_SINT32 m_nOperation;
    // Authoritative publication duration
    //    - "Expires" header in 2XX to PUBLISH request
    IMS_SINT32 m_nPublicationDuration;
    // SIP-ETag: received in the 2xx response message to the event publication
    AString m_strEntityTag;
    // Manages an initial SIP message for refresh/removal operation
    ISipMessage* m_piSipMsg;
};

#endif
