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
#include "ConfigUce.h"

#define CONFIG_APP_NAME     "ims.app.uce"
#define CONFIG_SERVICE_NAME "ims.service.uce"

PUBLIC GLOBAL const IMS_CHAR ConfigUce::APP_NAME[] = CONFIG_APP_NAME;
PUBLIC GLOBAL const IMS_CHAR ConfigUce::SERVICE_NAME[] = CONFIG_SERVICE_NAME;

// clang-format off
PUBLIC GLOBAL const IMS_CHAR ConfigUce::APP_CONFIG[] = {
        "[Uniqueness]\n"
        "Stream=0\n"
        "Framed=0\n"
        "Basic=0\n"
        "Event=1\n"
        "CoreService=1\n"
        "Qos=0\n"
        "Reg=1\n"
        "Write=1\n"
        "Read=1\n"
        "Cap=0\n"
        "Mprof=0\n"
        "Connection=0\n"
        "\n"
        "[IMSRegistry]\n"
        "IMSRegistry=" CONFIG_APP_NAME "\n"
        "\n"
        "[Event]\n"
        "package_names=presence\n"
        "\n"
        "[CoreService_0]\n"
        "service_id=" CONFIG_SERVICE_NAME "\n"
        "iari=\n"
        "icsi=\n"
        "feature_tag=\n"
        "\n"
        "[Reg_0]\n"
        "service_id=" CONFIG_SERVICE_NAME "\n"
        "header_count=1\n"
        "header_0=Supported: path, eventlist\n"
        "\n"
        "[Write]\n"
        "header_names=\n"
        "\n"
        "[Read]\n"
        "header_names=\n"
        "\n"
};
// clang-format on
