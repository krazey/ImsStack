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
#include "ConfigMedia.h"
#include "ConfigMtc.h"

#define CONFIG_APP_NAME               "ims.app.mtc"
#define CONFIG_SERVICE_NAME           "ims.service.mtc"
#define CONFIG_EMERGENCY_SERVICE_NAME "ims.service.mtc.emergency"

PUBLIC GLOBAL const IMS_CHAR ConfigMtc::APP_NAME[] = CONFIG_APP_NAME;
PUBLIC GLOBAL const IMS_CHAR ConfigMtc::SERVICE_NAME[] = CONFIG_SERVICE_NAME;
PUBLIC GLOBAL const IMS_CHAR ConfigMtc::EMERGENCY_SERVICE_NAME[] = CONFIG_EMERGENCY_SERVICE_NAME;

// clang-format off
PUBLIC GLOBAL const IMS_CHAR ConfigMtc::APP_CONFIG[] = {
        "[Uniqueness]\n"
        "Stream=1\n"
        "Framed=0\n"
        "Basic=0\n"
        "Event=1\n"
        "CoreService=2\n"
        "Qos=0\n"
        "Reg=2\n"
        "Write=1\n"
        "Read=1\n"
        "Cap=0\n"
        "Mprof=2\n"
        "Connection=0\n"
        "\n"
        "[IMSRegistry]\n"
        "IMSRegistry=" CONFIG_APP_NAME "\n"
        "\n"
        "[Stream]\n"
        "media_types=audio video text\n"
        "\n"
        "[Event]\n"
        "package_names=conference dialog presence\n"
        "\n"
        "[CoreService_0]\n"
        "service_id=" CONFIG_SERVICE_NAME "\n"
        "iari=\n"
        "icsi=urn:urn-7:3gpp-service.ims.icsi.mmtel\n"
        "feature_tag=\n"
        "\n"
        "[CoreService_1]\n"
        "service_id=" CONFIG_EMERGENCY_SERVICE_NAME "\n"
        "iari=\n"
        "icsi=urn:urn-7:3gpp-service.ims.icsi.mmtel\n"
        "feature_tag=\n"
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
        "header_names=Accept Alert-Info Content-Disposition Content-ID Content-Type History-Info "
        "P-Early-Media P-Preferred-Service Privacy Referred-By Refer-Sub Require Supported Reason "
        "Recv-Info In-Reply-To Geolocation Geolocation-Routing Priority Subject Call-Info "
        "Organization"
        "\n"
        "\n"
        "[Read]\n"
        "header_names=Contact Content-Type From History-Info P-Early-Media Privacy Reason Require "
        "Server Subscription-State Supported To Refer-To Replaces Call-ID Priority Subject "
        "Call-Info Organization"
        "\n"
        "\n"
        "[Mprof_0]\n"
        "service_id=" CONFIG_SERVICE_NAME "\n"
        "profile=" CONFIG_MEDIA_MTC "\n"
        "\n"
        "[Mprof_1]\n"
        "service_id=" CONFIG_EMERGENCY_SERVICE_NAME "\n"
        "profile=" CONFIG_MEDIA_MTC "\n"
        "\n"
};
// clang-format on
