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
#ifndef INTERFACE_REFERENCE_LISTENER_H_
#define INTERFACE_REFERENCE_LISTENER_H_

#include "ImsTypeDef.h"

class IMessage;
class IReference;

/**
 * @brief This class provides a listener interface to notify an application
 *        about events regarding an IReference.
 *
 * @see IReference
 */
class IReferenceListener
{
protected:
    virtual ~IReferenceListener() = default;

public:
    /**
     * @brief Notifies the application that the reference was successfully delivered.
     *
     * @param piReference Pointer to IReference
     */
    virtual void ReferenceDelivered(IN IReference* piReference) = 0;

    /**
     * @brief Notifies the application that the reference was not successfully delivered.
     *
     * @param piReference Pointer to IReference
     */
    virtual void ReferenceDeliveryFailed(IN IReference* piReference) = 0;

    /**
     * @brief Notifies the application with status reports regarding the Reference.
     *
     * @param piReference Pointer to IReference
     * @param piNotify Pointer to IMessage (including NOTIFY request)
     */
    virtual void ReferenceNotify(IN IReference* piReference, IN IMessage* piNotify) = 0;

    /**
     * @brief Notifies the application that a reference has been terminated.
     *
     * @param piReference Pointer to IReference
     */
    virtual void ReferenceTerminated(IN IReference* piReference) = 0;
};

#endif
