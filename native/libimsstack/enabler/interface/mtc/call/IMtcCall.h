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

#include "ImsTypeDef.h"

class AString;
class ISession;
class SuppService;
class IMtcCallContext;
struct CallReasonInfo;
struct ConfUser;
struct MediaInfo;
template <class T>
class ImsList;

using CallKey = IMS_ULONG;

/**
 * Defines the type of a call, such as voice, video, or RTT.
 */
enum class CallType
{
    UNKNOWN = 0,
    VOIP = 1,
    VT = 2,
    RTT = 3,
    VIDEO_RTT = 4,
};

/**
 * Defines the type of an emergency call.
 */
enum class EmergencyType
{
    // This applies when the service type of ImsCallProfile is set to
    // ImsCallProfile.SERVICE_TYPE_NORMAL.
    NONE = 0,
    // This applies when the service type of ImsCallProfile is set to
    // ImsCallProfile.SERVICE_TYPE_EMERGENCY and the emergency routing is set to either
    // EmergencyNumber.EMERGENCY_CALL_ROUTING_UNKNOWN or
    // EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY.
    EMERGENCY_ROUTING = 1,
    // This applies when the service type of ImsCallProfile is set to
    // ImsCallProfile.SERVICE_TYPE_EMERGENCY and the emergency routing is set to
    // EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL.
    NORMAL_ROUTING = 2,
};

/**
 * Defines the peer type of a call, indicating whether it is mobile-originated or mobile-terminated.
 */
enum class PeerType
{
    MO,
    MT,
};

/**
 * This class represents a single call and provides an interface for managing its operations.
 */
class IMtcCall
{
public:
    /** Defines the state of a call. */
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

    /** Represents an invalid call key. */
    static inline const CallKey CALL_KEY_INVALID = 0;

    virtual ~IMtcCall(){};

    /**
     * @brief Attaches the call to the framework, enabling interaction with the Java layer.
     * This is typically called for an incoming call when it is ready to be handled.
     */
    virtual void Attach() = 0;

    /**
     * @brief Starts an outgoing call.
     *
     * @param eCallType The type of the call to start.
     * @param strTarget The remote party's URI.
     * @param objMediaInfo The media information for the call.
     * @param objSuppServices A list of supplementary services to be applied.
     */
    virtual void Start(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo, IN const ImsList<SuppService*>& objSuppServices) = 0;

    /**
     * @brief Starts an outgoing conference call.
     *
     * @param eCallType The type of the conference call.
     * @param strTarget The conference factory URI.
     * @param objMediaInfo The media information for the call.
     * @param objSuppServices A list of supplementary services.
     * @param objUsers A list of users to be included in the conference.
     */
    virtual void StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN MediaInfo& objMediaInfo, IN const ImsList<SuppService*>& objSuppServices,
            IN const ImsList<ConfUser*>& objUsers) = 0;

    /**
     * @brief Starts an outgoing conference call with default media information.
     *
     * @param eCallType The type of the conference call.
     * @param strTarget The conference factory URI.
     * @param objUsers A list of users to be included in the conference.
     */
    virtual void StartConference(IN CallType eCallType, IN const AString& strTarget,
            IN const ImsList<ConfUser*>& objUsers) = 0;

    /**
     * @brief Handles an incoming call.
     *
     * @param piSession The SIP session associated with the incoming call.
     */
    virtual void HandleIncoming(IN ISession* piSession) = 0;

    /**
     * @brief Notifies that the user is being alerted for this incoming call.
     */
    virtual void HandleUserAlert() = 0;

    /**
     * @brief Accepts an incoming call.
     *
     * @param eCallType The desired call type for the accepted call.
     * @param objMediaInfo The media information for the accepted call.
     */
    virtual void Accept(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Rejects an incoming call.
     *
     * @param objReason The reason for rejecting the call.
     */
    virtual void Reject(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Puts the call on hold.
     *
     * @param objMediaInfo The media information to be used for the hold operation.
     */
    virtual void Hold(IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Resumes a held call.
     *
     * @param objMediaInfo The media information to be used for the resume operation.
     */
    virtual void Resume(IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Accepts a resume request from the remote party.
     *
     * @param eCallType The call type after resuming.
     * @param objMediaInfo The media information for the resumed call.
     */
    virtual void AcceptResume(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Rejects a resume request from the remote party.
     *
     * @param objReason The reason for rejecting the resume request.
     */
    virtual void RejectResume(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Requests a call update (e.g., upgrade to video) to the remote party.
     *
     * @param eCallType The new call type.
     * @param objMediaInfo The new media information.
     */
    virtual void Update(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Accepts a call update request from the remote party.
     *
     * @param eCallType The call type after the update.
     * @param objMediaInfo The media information for the updated call.
     */
    virtual void AcceptUpdate(IN CallType eCallType, IN MediaInfo& objMediaInfo) = 0;

    /**
     * @brief Rejects a call update request from the remote party.
     *
     * @param objReason The reason for rejecting the update.
     */
    virtual void RejectUpdate(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Cancels an ongoing call update request.
     *
     * @param objReason The reason for canceling the update.
     */
    virtual void CancelUpdate(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Terminates the call.
     *
     * @param objReason The reason for terminating the call.
     */
    virtual void Terminate(IN const CallReasonInfo& objReason) = 0;

    /**
     * @brief Sends a USSD message.
     *
     * @param strUssd The USSD string to send.
     */
    virtual void SendUssd(IN const AString& strUssd) = 0;

    /**
     * @brief Gets the log tag of the call.
     *
     * @return The log tag.
     */
    virtual const AString& GetLogTag() const = 0;

    /**
     * @brief Gets a key that uniquely identifies this call.
     *
     * @return The unique call key.
     */
    virtual CallKey GetKey() const = 0;

    /**
     * @brief Returns the current call type.
     *
     * @return Current call type.
     */
    virtual CallType GetCallType() const = 0;

    /**
     * @brief Returns the current call state.
     *
     * @return Current state.
     */
    virtual State GetState() const = 0;

    /**
     * @brief Gets the context of this call.
     *
     * @return A reference to the call context object.
     */
    virtual IMtcCallContext& GetCallContext() = 0;
};

/**
 * Holds basic information about a call.
 */
struct CallInfo
{
public:
    explicit CallInfo() :
            ePeerType(PeerType::MO),
            eInitialCallType(CallType::VOIP),
            eEmergencyType(EmergencyType::NONE),
            bOffline(IMS_FALSE),
            bUssi(IMS_FALSE),
            bConference(IMS_FALSE)
    {
    }

    explicit CallInfo(IN const CallInfo& objRhs) :
            ePeerType(objRhs.ePeerType),
            eInitialCallType(objRhs.eInitialCallType),
            eEmergencyType(objRhs.eEmergencyType),
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
            eEmergencyType = objRhs.eEmergencyType;
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
                eEmergencyType == objRhs.eEmergencyType && bOffline == objRhs.bOffline &&
                bUssi == objRhs.bUssi && bConference == objRhs.bConference;
    }

    IMS_BOOL operator!=(const CallInfo& objRhs) const { return !(*this == objRhs); }

    /**
     * @brief Checks if the call is an emergency call with emergency routing.
     *
     * @return True if it is an emergency call, false otherwise.
     */
    inline IMS_BOOL IsEmergency() const
    {
        return eEmergencyType == EmergencyType::EMERGENCY_ROUTING;
    }

public:
    /** The peer type of the call (MO or MT). */
    PeerType ePeerType;
    /** The initial call type when the call was started. */
    CallType eInitialCallType;
    /** The emergency type of the call. */
    EmergencyType eEmergencyType;
    /** Indicates if the call is an offline call (Triggered when there's no IMS registration). */
    IMS_BOOL bOffline;
    /** Indicates if the call is a USSI. */
    IMS_BOOL bUssi;
    /** Indicates if the call is a conference call. */
    IMS_BOOL bConference;
};

#endif
