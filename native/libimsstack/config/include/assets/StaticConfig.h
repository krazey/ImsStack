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
#ifndef STATIC_CONFIG_H_
#define STATIC_CONFIG_H_

#include "AString.h"

class StaticConfig
{
public:
    StaticConfig() = delete;

public:
    /**
     * @brief Returns the static configuration that matches with the specified configuration name.
     *
     * @param strName The configuration name to retrieve
     */
    static const IMS_CHAR* GetConfig(IN const AString& strName);

    /**
     * @brief Returns the media configuration.
     */
    static const IMS_CHAR* GetMediaConfig();
};

#endif
