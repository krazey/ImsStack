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
#ifndef INTERFACE_METHOD_H_
#define INTERFACE_METHOD_H_

#include "ImsTypeDef.h"

class IMessageMediator;

/**
 * @brief This class is a base interface for service method of Engine.
 */
class IMethod
{
public:
    /**
     * @brief Destroys IMethod interface.
     */
    virtual void Destroy() = 0;

    /**
     * @brief Sets the SIP message mediator.
     *
     * @param piMediator SIP message mediator
     * @see IMessageMediator
     * @note SIP_MESSAGE_MEDIATOR
     */
    virtual void SetMessageMediator(IN IMessageMediator* piMediator) = 0;
};

#endif
