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
#ifndef INTERFACE_SIP_ERROR_LISTENER_H_
#define INTERFACE_SIP_ERROR_LISTENER_H_

#include "ISipConnection.h"

/**
 * @brief This class provides a listener interface for notifications
 *        about failure of asynchronous send operations.
 *
 * The application implementing this interface has to register it with
 * a ISipConnection instance using the ISipTransaction::SetErrorListener method.
 *
 * @see ISipConnection
 */
class ISipErrorListener
{
protected:
    virtual ~ISipErrorListener() = default;

public:
    /**
     * @brief Called when an asynchronous send operation fails or any error occurs.
     *
     * @param piSc Pointer to ISipConnection object which error occurs
     * @param nCode Reason code of error
     * @param strMessage Reason phrase of error\n
     *                   Implementation dependent non-localized information about the error
     */
    virtual void Error_NotifyError(
            IN ISipConnection* piSc, IN IMS_SINT32 nCode, IN const AString& strMessage) = 0;
};

#endif
