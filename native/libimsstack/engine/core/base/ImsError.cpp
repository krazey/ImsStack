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
#include "base/Ims.h"
#include "base/ImsError.h"

#define IMS_ERROR_CODE(CODE)  \
    {                         \
        ImsError::CODE, #CODE \
    }
#define IMS_ERROR_CODE_END() \
    {                        \
        0, IMS_NULL          \
    }

struct ImsErrorCode
{
    IMS_SINT32 nCode;
    const IMS_CHAR* pszMessage;
};

// clang-format off
static const ImsErrorCode s_objErrorCode[] =
{
    IMS_ERROR_CODE(NO_ERROR),
    IMS_ERROR_CODE(GENERAL_ERROR),
    IMS_ERROR_CODE(SERVICE_CLOSED),
    IMS_ERROR_CODE(ILLEGAL_STATE),
    IMS_ERROR_CODE(ILLEGAL_ARGUMENT),
    IMS_ERROR_CODE(PROFILE_MISSED),
    IMS_ERROR_CODE(PROFILE_CORRUPTED),
    IMS_ERROR_CODE(AUTHENTICATION_FAILED),
    IMS_ERROR_CODE(REGISTRATION_REQUEST_TIMEOUT),
    IMS_ERROR_CODE(REGISTRATION_FAILED_RESPONSE),
    // AppId is not an installed IMS app. or Protocol type is not supported
    IMS_ERROR_CODE(CONNECTION_NOT_FOUND),
    IMS_ERROR_CODE(INVALID_OPERATION),
    IMS_ERROR_CODE(REFRESH_FAILED),
    IMS_ERROR_CODE(NO_MESSAGE),
    IMS_ERROR_CODE(NO_MEMORY),
    IMS_ERROR_CODE(NOT_FOUND),
    IMS_ERROR_CODE(ALREADY_EXISTS),
    IMS_ERROR_CODE(LIST_OPERATION_FAILED),
    IMS_ERROR_CODE(PARSING_ERROR),
    IMS_ERROR_CODE(NO_SIP_CONNECTION),

    IMS_ERROR_CODE_END()
};
// clang-format on

PUBLIC GLOBAL IMS_SINT32 ImsError::GetLastError()
{
    return Ims::GetLastError();
}

PUBLIC GLOBAL const IMS_CHAR* ImsError::GetLastErrorString()
{
    return GetString(Ims::GetLastError());
}

PUBLIC GLOBAL const IMS_CHAR* ImsError::GetString(IN IMS_SINT32 nError)
{
    IMS_UINT32 nCount = sizeof(s_objErrorCode) / sizeof(s_objErrorCode[0]);

    for (IMS_UINT32 i = 0; i < nCount; ++i)
    {
        const ImsErrorCode* pCode = &(s_objErrorCode[i]);

        if ((pCode->nCode == 0) || (pCode->pszMessage == IMS_NULL))
        {
            break;
        }

        if (pCode->nCode == nError)
        {
            return pCode->pszMessage;
        }
    }

    return "__UNKNOWN__";
}
