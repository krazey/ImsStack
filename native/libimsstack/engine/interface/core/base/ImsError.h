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
#ifndef IMS_ERROR_H_
#define IMS_ERROR_H_

#include "ImsTypeDef.h"

/**
 * @brief This class defines IMS error values.
 */
class ImsError
{
public:
    ImsError() = delete;

public:
    /**
     * @brief Returns the last error code.
     *
     * @return It returns the last error code.
     */
    static IMS_SINT32 GetLastError();

    /**
     * @brief Returns the string format of the last error code.
     *
     * @return It returns the string format of the last error code.
     */
    static const IMS_CHAR* GetLastErrorString();

    /**
     * @brief Returns the string format of the specified error code.
     *
     * @param nError the error code
     * @return It returns the string format of the specified error code.
     */
    static const IMS_CHAR* GetString(IN IMS_SINT32 nError);

public:
    enum
    {
        NO_ERROR = 10001,
        GENERAL_ERROR = (NO_ERROR + 1),
        SERVICE_CLOSED,
        ILLEGAL_STATE,
        ILLEGAL_ARGUMENT,
        PROFILE_MISSED,
        PROFILE_CORRUPTED,
        AUTHENTICATION_FAILED,
        REGISTRATION_REQUEST_TIMEOUT,
        REGISTRATION_FAILED_RESPONSE,
        // AppId is not an installed IMS app. or Protocol type is not supported
        CONNECTION_NOT_FOUND,
        INVALID_OPERATION,

        REFRESH_FAILED,

        NO_MESSAGE,
        NO_MEMORY,
        NOT_FOUND,
        ALREADY_EXISTS,
        LIST_OPERATION_FAILED,
        PARSING_ERROR,

        NO_SIP_CONNECTION
    };
};

#endif
