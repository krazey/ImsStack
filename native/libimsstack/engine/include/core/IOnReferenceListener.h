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
#ifndef INTERFACE_ON_REFERENCE_LISTENER_H_
#define INTERFACE_ON_REFERENCE_LISTENER_H_

#include "ImsTypeDef.h"

class Message;
class Reference;

/**
 * @brief This listener type is used to notify an application about events regarding a Reference.
 *
 * @see Reference
 */
class IOnReferenceListener
{
protected:
    virtual ~IOnReferenceListener() = default;

public:
    /**
     * @brief Notifies the application that the reference was successfully delivered.
     *
     * @param pReference The Reference object
     */
    virtual void OnReference_Delivered(IN Reference* pReference) = 0;

    /**
     * @brief Notifies the application that the reference was not successfully delivered.
     *
     * @param pReference The Reference object
     */
    virtual void OnReference_DeliveryFailed(IN Reference* pReference) = 0;

    /**
     * @brief Notifies the application with status reports regarding the Reference.
     *
     * @param pReference The Reference object
     * @param pNotify The Message object (including NOTIFY request)
     */
    virtual void OnReference_NotifyReceived(IN Reference* pReference, IN Message* pNotify) = 0;

    /**
     * @brief Notifies the application that a reference has been terminated.
     *
     * @param pReference The Reference object
     */
    virtual void OnReference_Terminated(IN Reference* pReference) = 0;
};

#endif
