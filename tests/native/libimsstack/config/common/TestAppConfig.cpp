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

#include "private/ImsRegistryLoader.h"
#include "private/TestAppConfig.h"

#define TEST_CONFIG_APP_NAME       "ims.test.app"
#define TEST_CONFIG_SERVICE_NAME_1 "ims.test.service1"
#define TEST_CONFIG_SERVICE_NAME_2 "ims.test.service2"
#define TEST_CONFIG_MEDIA          "ims.test.media"

const IMS_CHAR TestAppConfig::TEST_APP_NAME[] = TEST_CONFIG_APP_NAME;
const IMS_CHAR TestAppConfig::TEST_SERVICE_NAME_1[] = TEST_CONFIG_SERVICE_NAME_1;
const IMS_CHAR TestAppConfig::TEST_SERVICE_NAME_2[] = TEST_CONFIG_SERVICE_NAME_2;
const IMS_CHAR TestAppConfig::TEST_MEDIA[] = TEST_CONFIG_MEDIA;
const IMS_CHAR TestAppConfig::TEST_CONFIG[] = {
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
        "IMSRegistry=" TEST_CONFIG_APP_NAME "\n"
        "\n"
        "[Stream]\n"
        "media_types=audio video text\n"
        "\n"
        "[Event]\n"
        "package_names=conference\n"
        "\n"
        "[CoreService_0]\n"
        "service_id=" TEST_CONFIG_SERVICE_NAME_1 "\n"
        "iari=\n"
        "icsi=urn:urn-7:3gpp-service.ims.icsi.mmtel\n"
        "feature_tag=\n"
        "\n"
        "[CoreService_1]\n"
        "service_id=" TEST_CONFIG_SERVICE_NAME_2 "\n"
        "iari=\n"
        "icsi=urn:urn-7:3gpp-service.ims.icsi.mmtel\n"
        "feature_tag=\n"
        "\n"
        "[Reg_0]\n"
        "service_id=" TEST_CONFIG_SERVICE_NAME_1 "\n"
        "header_count=1\n"
        "header_0=Supported: eventlist\n"
        "\n"
        "[Reg_1]\n"
        "service_id=" TEST_CONFIG_SERVICE_NAME_2 "\n"
        "header_count=1\n"
        "header_0=Supported: eventlist\n"
        "\n"
        "[Write]\n"
        "header_names=Accept Alert-Info Content-Disposition Content-ID Content-Type History-Info "
        "P-Early-Media P-Preferred-Service Privacy Referred-By Refer-Sub Require Supported Reason "
        "Recv-Info In-Reply-To Geolocation Geolocation-Routing"
        "\n"
        "\n"
        "[Read]\n"
        "header_names=Contact Content-Type From History-Info P-Early-Media Privacy Reason Require "
        "Server Subscription-State Supported To Refer-To Replaces Call-ID"
        "\n"
        "\n"
        "[Mprof_0]\n"
        "service_id=" TEST_CONFIG_SERVICE_NAME_1 "\n"
        "profile=" TEST_CONFIG_MEDIA "\n"
        "\n"
        "[Mprof_1]\n"
        "service_id=" TEST_CONFIG_SERVICE_NAME_2 "\n"
        "profile=" TEST_CONFIG_MEDIA "\n"
        "\n"};

AppConfig TestAppConfig::Create(IN const IMS_CHAR* pszAppId, IN const IMS_CHAR* pszConfig,
        IN IMS_SINT32 nFlags /*= FLAG_STREAM_ALL*/)
{
    const AString strAppId(pszAppId);
    AString strConfig(pszConfig);

    SetConfig(strConfig, nFlags);

    ImsRegistry objRegistry;
    ImsRegistryLoader::GetRegistryFromContent(strAppId, strConfig, objRegistry);

    AppConfig objAppConfig(strAppId);
    objAppConfig.Create(objRegistry, IMS_SLOT_0);

    return objAppConfig;
}

AppConfig* TestAppConfig::CreateP(IN const IMS_CHAR* pszAppId, IN const IMS_CHAR* pszConfig,
        IN IMS_SINT32 nFlags /*= FLAG_STREAM_ALL*/)
{
    const AString strAppId(pszAppId);
    AString strConfig(pszConfig);

    SetConfig(strConfig, nFlags);

    ImsRegistry objRegistry;
    ImsRegistryLoader::GetRegistryFromContent(strAppId, strConfig, objRegistry);

    AppConfig* pAppConfig = new AppConfig(strAppId);
    pAppConfig->Create(objRegistry, IMS_SLOT_0);

    return pAppConfig;
}

void TestAppConfig::SetConfig(IN_OUT AString& strConfig, IN IMS_SINT32 nFlags)
{
    if ((nFlags & FLAG_STREAM_AUDIO_VIDEO) != 0)
    {
        strConfig.Replace("audio video text", "audio video");
    }
    else if ((nFlags & FLAG_STREAM_AUDIO) != 0)
    {
        strConfig.Replace("audio video text", "audio");
    }
    else if ((nFlags & FLAG_STREAM_EMPTY) != 0)
    {
        strConfig.Replace("audio video text", "");
    }
}
