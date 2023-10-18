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

#ifndef TEST_APP_CONFIG_H_
#define TEST_APP_CONFIG_H_

#include "private/AppConfig.h"

class TestAppConfig
{
public:
    static AppConfig Create(IN const IMS_CHAR* pszAppId, IN const IMS_CHAR* pszConfig,
            IN IMS_SINT32 nFlags = FLAG_STREAM_ALL);
    static AppConfig* CreateP(IN const IMS_CHAR* pszAppId, IN const IMS_CHAR* pszConfig,
            IN IMS_SINT32 nFlags = FLAG_STREAM_ALL);

private:
    static void SetConfig(IN_OUT AString& strConfig, IN IMS_SINT32 nFlags);

public:
    static const IMS_CHAR TEST_APP_NAME[];
    static const IMS_CHAR TEST_SERVICE_NAME_1[];
    static const IMS_CHAR TEST_SERVICE_NAME_2[];
    static const IMS_CHAR TEST_MEDIA[];
    static const IMS_CHAR TEST_CONFIG[];

    enum
    {
        FLAG_STREAM_ALL = 1 << 0,
        FLAG_STREAM_AUDIO = 1 << 1,
        FLAG_STREAM_AUDIO_VIDEO = 1 << 2,
        FLAG_STREAM_EMPTY = 1 << 3,
    };
};

#endif
