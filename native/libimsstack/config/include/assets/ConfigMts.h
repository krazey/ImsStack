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
#ifndef CONFIG_MTS_H_
#define CONFIG_MTS_H_

#include "ImsTypeDef.h"

/**
 * @brief Configuration for MTS (Multimedia Telephony SMS).
 */
class ConfigMts
{
public:
    ConfigMts() = delete;

public:
    static const IMS_CHAR APP_NAME[];
    static const IMS_CHAR SERVICE_NAME[];
    static const IMS_CHAR EMERGENCY_SERVICE_NAME[];

    static const IMS_CHAR APP_CONFIG[];
};

#endif
