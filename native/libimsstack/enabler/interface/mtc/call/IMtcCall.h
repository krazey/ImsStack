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

#ifndef INTERFACE_MTC_CALL_H_
#define INTERFACE_MTC_CALL_H_

#include "AString.h"
#include "IMtcService.h"
#include "ImsList.h"
#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "MtcDef.h"

class ISession;
class SuppService;
class IMtcCallContext;
struct CallReasonInfo;
struct ConfUser;
struct MediaInfo;

using CallKey = IMS_ULONG;

enum class CallType
{
    UNKNOWN = 0,
    VOIP = 1,
    VT = 2,
    RTT = 3,
    VIDEO_RTT = 4,
};

enum class PeerType
{
    MO,
    MT,
};

class IMtcCall
{
public:
    enum class State
    {
        IDLE,
        OUTGOING,
        INCOMING,
        ALERTING,
        ESTABLISHED,
        UPDATING,
        TERMINATING,
    };

    static inline const CallKey CALL_KEY_INVALID = 0;

    virtual ~IMtcCall(){};

    // Sets thread to interact with the Java layer. Nothing happens if the thread is null.

    /**
     * @brief Attachs
     *
     */
    virtual void Attach() = 0;

    // Starts an outgoing call.

    /**
     * @brief Starts
     *
     * @param eCallType
     * @param strTarget
     * @param objMediaInfo
     * @param objSuppServices
     */
    virtual void Start(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo,
            IN const ImsMap<SuppType, SuppService*>& objSuppServices) = 0;

    /**
     * @brief Starts
     *
     * @param eCallType
     * @param strTarget
     * @param objMediaInfo
     * @param objSuppServices
     * @param objUsers
     */
    virtual void StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo, IN const ImsMap<SuppType, SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& objUsers) = 0;

    /**
     * @brief Starts
     *
     * @param eCallType
     * @param strTarget
     * @param objUsers
     */
    virtual void StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN const ImsList<ConfUser*>& objUsers) = 0;

    // Handles an incoming call.

    /**
     * @brief Handles
     *
     * @param piSession
     */
    virtual void HandleIncoming(IN ISession* piSession) = 0;

    // Notifies that the user alerting for this call is started.

    /**
     * @brief Handles
     *
     */
    virtual void HandleUserAlert() = 0;

    // Accepts the incoming call.

    /**
     * @brief Accepts
     *
     * @param eCallType
     * @param objMediaInfo
     */
    virtual void Accept(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    // Rejects the incoming call.

    /**
     * @brief Rejects
     *
     * @param objReason
     */
    virtual void Reject(IN const CallReasonInfo& objReason) = 0;

    // Holds the call.

    /**
     * @brief Holds
     *
     * @param objMediaInfo
     */
    virtual void Hold(IN MediaInfo& objMediaInfo) = 0;

    // Resumes the call.

    /**
     * @brief Resumes
     *
     * @param objMediaInfo
     */
    virtual void Resume(IN MediaInfo& objMediaInfo) = 0;

    // Accepts the resume request from the remote.

    /**
     * @brief Accepts
     *
     * @param eCallType
     * @param objMediaInfo
     */
    virtual void AcceptResume(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    // Rejects the resume request from the remote.

    /**
     * @brief Rejects
     *
     * @param objReason
     */
    virtual void RejectResume(IN const CallReasonInfo& objReason) = 0;

    // Requests call updating(converting) to the remote.

    /**
     * @brief Updates
     *
     * @param eCallType
     * @param objMediaInfo
     */
    virtual void Update(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    // Accepts the call updating(converting) request from the remote.

    /**
     * @brief Accepts
     *
     * @param eCallType
     * @param objMediaInfo
     */
    virtual void AcceptUpdate(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    // Rejects the call updating(converting) request from the remote.

    /**
     * @brief Rejects
     *
     * @param objReason
     */
    virtual void RejectUpdate(IN const CallReasonInfo& objReason) = 0;

    // Cancels the ongoing call updating(converting) request.

    /**
     * @brief Cancels
     *
     * @param objReason
     */
    virtual void CancelUpdate(IN const CallReasonInfo& objReason) = 0;

    // Terminates the call.

    /**
     * @brief Terminates
     *
     * @param objReason
     */
    virtual void Terminate(IN const CallReasonInfo& objReason) = 0;

    // Sends USSD. Does nothing if the call isn't a USSI call.

    /**
     * @brief Sends
     *
     * @param strUssd
     */
    virtual void SendUssd(IN const AString& strUssd) = 0;

    // Returns a key to uniquely identify this call.

    /**
     * @brief Gets
     *
     * @return
     */
    virtual CallKey GetKey() const = 0;

    /**
     * Returns the current call type.
     *
     * @return Current call type.
     */
    virtual CallType GetCallType() const = 0;

    /**
     * Returns the current call state.
     *
     * @return Current state.
     */
    virtual State GetState() const = 0;

    /**
     * @brief Gets
     *
     * @return
     */
    virtual IMtcCallContext& GetCallContext() = 0;
};

struct CallInfo
{
public:
    explicit CallInfo() :
            ePeerType(PeerType::MO),
            eInitialCallType(CallType::VOIP),
            bEmergency(IMS_FALSE),
            bOffline(IMS_FALSE),
            bUssi(IMS_FALSE),
            bConference(IMS_FALSE)
    {
    }

    explicit CallInfo(IN const CallInfo& objRhs) :
            ePeerType(objRhs.ePeerType),
            eInitialCallType(objRhs.eInitialCallType),
            bEmergency(objRhs.bEmergency),
            bOffline(objRhs.bOffline),
            bUssi(objRhs.bUssi),
            bConference(objRhs.bConference)
    {
    }

    CallInfo& operator=(IN const CallInfo& objRhs)
    {
        if (this != &objRhs)
        {
            ePeerType = objRhs.ePeerType;
            eInitialCallType = objRhs.eInitialCallType;
            bEmergency = objRhs.bEmergency;
            bOffline = objRhs.bOffline;
            bUssi = objRhs.bUssi;
            bConference = objRhs.bConference;
        }

        return *this;
    }

    IMS_BOOL operator==(const CallInfo& objRhs) const
    {
        if (this == &objRhs)
        {
            return IMS_TRUE;
        }

        return ePeerType == objRhs.ePeerType && eInitialCallType == objRhs.eInitialCallType &&
                bEmergency == objRhs.bEmergency && bOffline == objRhs.bOffline &&
                bUssi == objRhs.bUssi && bConference == objRhs.bConference;
    }

    IMS_BOOL operator!=(const CallInfo& objRhs) const { return !(*this == objRhs); }

public:
    PeerType ePeerType;
    CallType eInitialCallType;
    IMS_BOOL bEmergency;
    IMS_BOOL bOffline;
    IMS_BOOL bUssi;
    IMS_BOOL bConference;
};

#endif
