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
#ifndef INTERFACE_AOS_REGISTRATION_CONTROL_LISTENER_H_
#define INTERFACE_AOS_REGISTRATION_CONTROL_LISTENER_H_

#include "ImsMap.h"
#include "ImsTypeDef.h"
#include "AString.h"

enum class AosPcscfOrder;
enum class AosRegRequestType;
enum class AosControlCause;
enum class AosReasonCode;

class IAosRegistrationControlListener
{
public:
    virtual ~IAosRegistrationControlListener(){};

    virtual void RegistrationControl_UpdateSipDelegateRegistration() = 0;
    virtual void RegistrationControl_TriggerSipDelegateDeregistration() = 0;
    virtual void RegistrationControl_TriggerFullNetworkRegistration(
            IN IMS_SINT32 nSipCode, IN const AString& strTarget) = 0;
    virtual void RegistrationControl_NotifyCapabilitiesChanged(
            IN const ImsMap<IMS_UINT32, IMS_UINT32>& objCapabilities) = 0;
    virtual void RegistrationControl_ControlRegistration(
            IN AosRegRequestType eType, IN AosPcscfOrder eOrder, IN AosControlCause eCause) = 0;
    virtual void RegistrationControl_UpdateDataFailureReason(IN AosReasonCode eReason) = 0;
};

class AosRegistrationControlListener : public IAosRegistrationControlListener
{
public:
    inline void RegistrationControl_UpdateSipDelegateRegistration() override{};
    inline void RegistrationControl_TriggerSipDelegateDeregistration() override{};
    inline void RegistrationControl_TriggerFullNetworkRegistration(
            IN IMS_SINT32 /*nSipCode*/, IN const AString& /*strTarget*/) override{};
    inline void RegistrationControl_NotifyCapabilitiesChanged(
            IN const ImsMap<IMS_UINT32, IMS_UINT32>& /*objCapabilities*/) override{};
    inline void RegistrationControl_ControlRegistration(IN AosRegRequestType /*eType*/,
            IN AosPcscfOrder /*eOrder*/, IN AosControlCause /*eCause*/) override{};
    inline void RegistrationControl_UpdateDataFailureReason(
            IN AosReasonCode /*eReason*/) override {};
};

/**
 * PCSCF order used in registration request
 */
enum class AosPcscfOrder
{
    FIRST = 0,
    CURRENT = 1,
    NEXT = 2
};

/**
 * Registration request type
 */
enum class AosRegRequestType
{
    START = 0,
    REFRESH = 1,
    STOP = 2,
    START_IMS_EST_TIMER = 3
};

/**
 * Registration control cause
 */
enum class AosControlCause
{
    UNKNOWN = 0,
    DATA = 1,
    RADIO = 2,
    IMS_SERVICE = 3,
    IMS_SUBSCRIBER = 4,
    DATA_CONNECTING = 5,

    /* From modem */
    RADIO_SIM_REMOVED = 11,
    RADIO_SIM_REFRESH = 12,
    RADIO_ALLOWED_NETWORK_TYPES_CHANGED = 13,

    /* From framework */
    RADIO_POWER_OFF = 21,
    NON_IMS_CAPABLE_NETWORK = 22,
    DATA_STALL = 23,
    HANDOVER_FAILED = 24,
    VOPS_NOT_SUPPORTED = 25,
    WIFI_OFF = 26
};

#endif  // INTERFACE_AOS_REGISTRATION_CONTROL_LISTENER_H_