/*
 * Copyright (C) 2023 The Android Open Source Project
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
#ifndef INTERFACE_AOS_EMERGENCY_LISTENER_H_
#define INTERFACE_AOS_EMERGENCY_LISTENER_H_

enum class EmergencyCallbackModeType;
enum class EmergencyCallbackMode;

/**
 * @brief This class provides a listener interface for Aos
 * The IAosEmergencyListener is the listener interface getting the information about
 * Emergency.
 */
class IAosEmergencyListener
{
public:
    virtual ~IAosEmergencyListener() {}
    /**
     * @brief Notifies an emergency callback mode changed information by AosService (Java).
     *
     * @param eType The emergency callback mode type.
     * @param eState The emergency callback mode state.
     * @param nDuration The timer duration of emergency callback mode for call or sms.
     */
    virtual void CallbackModeChanged(IN EmergencyCallbackModeType eType,
            IN EmergencyCallbackMode eState, IN IMS_ULONG nDuration) = 0;
};

/**
 * Emergency Callback mode type for Aos
 */
enum class EmergencyCallbackModeType
{
    CALL = 1,
    SMS = 2
};

/**
 * Emergency Callback mode for Aos
 */
enum class EmergencyCallbackMode
{
    STOP = 0,
    START = 1,
    STOP_BY_EMERGENCY = 2
};

#endif  // INTERFACE_AOS_EMERGENCY_LISTENER_H_