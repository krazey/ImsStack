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
#ifndef INTERFACE_VONR_H_
#define INTERFACE_VONR_H_

#include "ImsTypeDef.h"

class IVoNrUacListener
{
public:
    /*
        This indication is sent to notify the UAC response
        nType : TYPE_XXX (TYPE_VOICE, ...)
        nResult : IMS_FAILURE, IMS_SUCCESS
        nReason : REASON_XXX (REASON_NO, ...)
        nSysMode : SYS_MODE_XXX (SYS_MODE_LTE, ...)
        nBarringTime : N/A
    */
    virtual void VoNrUac_NotifyResponse(IN IMS_UINT32 nType, IN IMS_RESULT nResult,
        IN IMS_SINT32 nReason, IN IMS_UINT32 nSysMode, IN IMS_UINT32 nBarringTime) = 0;

    enum
    {
        REASON_NO = 0,

        REASON_ACCESS_BARRED = -1,
        REASON_INVALID_RAT = -2,
        REASON_INVALID_STATE = -3,
        REASON_NO_SERVICE = -4,
        REASON_T3346_ACTIVE = -5,
        REASON_SERVICE_AREA_RESTRICTION = -6,

        REASON_UNKNOWN = -11,
        REASON_NO_RESPONSE = -12
    };
};

class IVoNrCallPreferenceListener
{
public:
    /*
        This indication is sent when the UE has found service to redial the voice call/video call
        nSysMode : SYS_MODE_LTE, SYS_MODE_NR5G (Two types are only valid)
    */
    virtual void VoNrCallPreference_NotifyCallReady(IN IMS_UINT32 nSysMode) = 0;
};

class IVoNrHandoffListener
{
public:
    /*
        This indication is that a handoff is in progress or has been completed.
        nStatus : STATUS_HANDOFF_XXX (STATUS_HANDOFF_INIT, ...)
        nSourceRAT, nTargetRAT : RAT_LTE, RAT_NR5G (Two types are only valid)
        nReason : if need, parameters will be defined
    */
    virtual void VoNrHandoff_NotifyInformation(IN IMS_UINT32 nStatus,
        IN IMS_UINT32 nSourceRAT, IN IMS_UINT32 nTargetRAT, IN IMS_SINT32 nReason) = 0;

    enum
    {
        STATUS_HANDOFF_INIT = 0,
        STATUS_HANDOFF_SUCCESS,
        STATUS_HANDOFF_FAILURE
    };
};


class IVoNr
{
public:
    /*
        Determine if VoNR is supported
    */
    virtual IMS_BOOL IsVoNrSupported() const = 0;

    /*
        Determine if UAC check is required
    */
    virtual IMS_BOOL IsUacCheckRequired(IN IMS_UINT32 nType) = 0;

    /**
     * Checks if VoNR is enabled as UE capabilities in modem.
     */
    virtual IMS_BOOL IsUeCapabilityVoNrEnabled() const = 0;

    /*
        Notify ims call state to modem
        nModule : MODULE_XXX (MODULE_UC, ...)
        nType : TYPE_XXX (TYPY_VOICE, ...)
        nState : STATE_XXX (STATE_START, ...)
        nSysMode : SYS_MODE_LTE, SYS_MODE_NR5G, SYS_MODE_WLAN
        nDirection : DIRECTION_MO, DIRECTION_MT
        return : IMS_FALSE - invalid information
    */
    virtual IMS_BOOL NotifyCallState(IN IMS_UINT32 nModule, IN IMS_UINT32 nType, IN IMS_UINT32 nState,
        IN IMS_UINT32 nSysMode, IN IMS_UINT32 nDirection) = 0;

    /*
        This command sets the RATs to be scanned to perform silent redial
        for the MMTEL voice call/video call originated.
        response is called as VoNR_NotifyCallReady() function.
        nRAT : RAT_LTE, RAT_NR5G
        nType : TYPE_VOICE, TYPE_VIDEO (Two types are only allowed)
        return : CALL_PREF_REQUEST_XXX (CALL_PREF_REQUEST_SUCCESS, ...)
    */
    virtual IMS_SINT32 RequestCallPreference(IN IMS_UINT32 nRAT, IN IMS_UINT32 nType) = 0;

    /*
        Set Ims Session State for MTK UAC operation
        nType : TYPE_VOICE, TYPE_VIDEO, TYPE_SMS, TYPE_REG_SIGNAL (Four types are only allowed)
        nState : MTK_CALL_START, MTK_CALL_STOP
    */
    virtual IMS_BOOL SetImsSession(IN IMS_UINT32 nType, IN IMS_UINT32 nState) = 0;

    /*
        Set IMS Voice State to VDM for MTK
        nState : MTK_CALL_START, MTK_CALL_STOP
        nSysMode : SYS_MODE_XXX (SYS_MODE_LTE, SYS_MODE_NR5G, SYS_MODE_WLAN, ...)
    */
    virtual IMS_BOOL SetImsVoice(IN IMS_UINT32 nState, IN IMS_UINT32 nSysMode) = 0;

    /*
        Set Ims Signaling for UAC
        nType : SIGNALING_TYPE_XXX (SIGNALING_TYPE_IDLE, ...)
    */
    virtual IMS_BOOL SetImsSignaling(IN IMS_UINT32 nType) = 0;

    /*
        Set UAC Check for MTK
        nType : TYPE_VOICE, TYPE_VIDEO, TYPE_SMS, TYPE_REG_SIGNAL (Four types are only allowed)
        nState : MTK_CALL_START, MTK_CALL_STOP
    */
    virtual IMS_BOOL SetUacCheck(IN IMS_UINT32 nType, IN IMS_UINT32 nState) = 0;

    /*
        Set Voice State to VDM for MTK
        nState : MTK_CALL_START, MTK_CALL_STOP
        bIsEmergency : IMS_TRUE, IMS_FALSE
    */
    virtual IMS_BOOL SetVoice(IN IMS_UINT32 nState, IN IMS_BOOL bIsEmergency) = 0;

    /*
        Adds the listener for UAC
    */
    virtual void AddListenerForUac(IN IVoNrUacListener* piListener) = 0;

    /*
        Removes the listener for UAC
    */
    virtual void RemoveListenerForUac(IN IVoNrUacListener* piListener) = 0;

    /*
        Adds the listener for call preference
    */
    virtual void AddListenerForCallPreference(
            IN IVoNrCallPreferenceListener* piListener) = 0;

    /*
        Removes the listener for call preference
    */
    virtual void RemoveListenerForCallPreference(
            IN IVoNrCallPreferenceListener* piListener) = 0;

    /*
        Adds the listener for handoff
    */
    virtual void AddListenerForHandoff(IN IVoNrHandoffListener* piListener) = 0;

    /*
        Removes the listener for handoff
    */
    virtual void RemoveListenerForHandoff(IN IVoNrHandoffListener* piListener) = 0;

    enum
    {
        TYPE_VOICE = 0x01,
        TYPE_VIDEO = 0x02,
        TYPE_SMS = 0x04,
        TYPE_EMERGENCY = 0x08,
        TYPE_REG_SIGNAL = 0x10
    };

    enum
    {
        MODULE_UC = 0,
        MODULE_AOS,
        MODULE_SMS
    };

    enum
    {
        STATE_IDLE = 0,
        STATE_START,
        STATE_CONNECTED
    };

    enum
    {
        SYS_MODE_UNKNOWN = 0,

        SYS_MODE_LTE,
        SYS_MODE_NR5G,
        SYS_MODE_WLAN,

        SYS_MODE_NO_SVC,
        SYS_MODE_NO_CHANGE,
        SYS_MODE_TIMEOUT
    };

    enum
    {
        DIRECTION_MO = 0,
        DIRECTION_MT
    };

    enum
    {
        RAT_INVALID = 0,
        RAT_LTE,
        RAT_NR5G
    };

    enum
    {
        CALL_PREF_REQUEST_SUCCESS = 0,

        CALL_PREF_REQUEST_FAILURE_INVALID_PARAMETER = -1,
        CALL_PREF_REQUEST_FAILURE_OTHER_RAT_REQUESTED = -2,
        CALL_PREF_REQUEST_FAILURE_NOT_SUPPORTED = -3,
    };

    enum
    {
        SIGNALING_TYPE_IDLE = 0,
        SIGNALING_TYPE_ACTIVE
    };

    // MTK only
    enum
    {
        MTK_CALL_START = 0,
        MTK_CALL_STOP
    };

    // MTK only
    enum
    {
        MTK_TYPE_REG_SIGNAL = 8
    };
};

#endif // INTERFACE_VONR_H_
