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
#include "ConfigMts.h"

#define CONFIG_APP_NAME               "ims.app.mts"
#define CONFIG_SERVICE_NAME           "ims.service.mts"
#define CONFIG_EMERGENCY_SERVICE_NAME "ims.service.mts.emergency"

PUBLIC GLOBAL const IMS_CHAR ConfigMts::APP_NAME[] = CONFIG_APP_NAME;
PUBLIC GLOBAL const IMS_CHAR ConfigMts::SERVICE_NAME[] = CONFIG_SERVICE_NAME;
PUBLIC GLOBAL const IMS_CHAR ConfigMts::EMERGENCY_SERVICE_NAME[] = CONFIG_EMERGENCY_SERVICE_NAME;

// clang-format off
PUBLIC GLOBAL const IMS_CHAR ConfigMts::APP_CONFIG[] = {
        "[Uniqueness]\n"
        "Stream=0\n"
        "Framed=0\n"
        "Basic=0\n"
        "Event=0\n"
        "CoreService=2\n"
        "Qos=0\n"
        "Reg=2\n"
        "Write=1\n"
        "Read=1\n"
        "Cap=0\n"
        "Mprof=0\n"
        "Connection=0\n"
        "\n"
        "[IMSRegistry]\n"
        "IMSRegistry=" CONFIG_APP_NAME "\n"
        "\n"
        "[CoreService_0]\n"
        "service_id=" CONFIG_SERVICE_NAME "\n"
        "iari=\n"
        "icsi=\n"
        "feature_tag=+g.3gpp.smsip\n"
        "\n"
        "[CoreService_1]\n"
        "service_id=" CONFIG_EMERGENCY_SERVICE_NAME "\n"
        "iari=\n"
        "icsi=\n"
        "feature_tag=+g.3gpp.smsip\n"
        "\n"
        "[Reg_0]\n"
        "service_id=" CONFIG_SERVICE_NAME "\n"
        "header_count=1\n"
        "header_0=Supported: path\n"
        "\n"
        "[Reg_1]\n"
        "service_id=" CONFIG_EMERGENCY_SERVICE_NAME "\n"
        "header_count=1\n"
        "header_0=Supported: path\n"
        "\n"
        "[Write]\n"
        "header_names=Request-Disposition In-Reply-To Geolocation\n"
        "\n"
        "[Read]\n"
        "header_names=Content-Type From Request-Disposition Retry-After To Call-ID\n"
        "\n"
};
// clang-format on
