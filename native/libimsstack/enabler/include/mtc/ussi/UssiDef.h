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

#ifndef USSI_DEF_H_
#define USSI_DEF_H_

enum class UssiModeType
{
    ERROR = -1,
    NOTIFY = 0,
    REQUEST = 1,
    NONE = 2
};

enum class UssiError
{
    CODE_NONE = 0,  // no error-code specified
    CODE_1 = 1,     // unspecified. default error-code
    CODE_2 = 2,     // language/alphabet not supported
    CODE_3 = 3,     // unexpected data value
    CODE_4 = 4      // USSD-busy
};

enum class UssiNextAction
{
    NOTHING = 0,
    SEND_INFO_WITH_NOTIFY_ELEMENT = 1,
    SEND_INFO_WITH_ERROR_CODE = 2,
    SEND_INFO_WITH_ERROR_CODE_AND_TERMINATE = 3
};

struct UssiResult
{
public:
    UssiResult(UssiNextAction _eAction, UssiError _eErrorCode) :
            eAction(_eAction),
            eErrorCode(_eErrorCode)
    {
    }

    UssiResult& operator=(const UssiResult& objRhs)
    {
        if (this != &objRhs)
        {
            eAction = objRhs.eAction;
            eErrorCode = objRhs.eErrorCode;
        }

        return *this;
    }

    IMS_BOOL operator==(const UssiResult& objRhs) const
    {
        return eAction == objRhs.eAction && eErrorCode == objRhs.eErrorCode;
    }

    UssiNextAction eAction;
    UssiError eErrorCode;
};

#endif
