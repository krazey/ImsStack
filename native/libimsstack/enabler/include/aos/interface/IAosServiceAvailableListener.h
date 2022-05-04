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
#ifndef INTERFACE_AOS_SERVICE_AVAILABLE_LISTENER_H_
#define INTERFACE_AOS_SERVICE_AVAILABLE_LISTENER_H_

/**
 * @brief
 *
 * @see
 */
class IAosServiceAvailableListener
{
public:
    /**
     * @brief Notifies if any service is available under the current conditions.
     *
     * @param
     */
    virtual void ServiceAvailable_Changed() = 0;

    /**
     * @brief Notifies if there are specific commands under the current conditions.
     *
     * @param
     */
    virtual void ServiceAvailable_RequestCommand(IN IMS_UINT32 nCommand, IN IMS_UINT32 nReason) = 0;
};

#endif  // INTERFACE_AOS_SERVICE_AVAILABLE_LISTENER_H_