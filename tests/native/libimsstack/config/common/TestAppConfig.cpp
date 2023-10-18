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
        "Framed=1\n"
        "Basic=1\n"
        "Event=1\n"
        "CoreService=2\n"
        "Qos=2\n"
        "Reg=2\n"
        "Write=1\n"
        "Read=1\n"
        "Cap=2\n"
        "Mprof=2\n"
        "Connection=2\n"
        "\n"
        "[IMSRegistry]\n"
        "IMSRegistry=" TEST_CONFIG_APP_NAME "\n"
        "\n"
        "[Stream]\n"
        "media_types=audio video text\n"
        "\n"
        "[Framed]\n"
        "media_types=text/plain image/png\n"
        "max_size=4096\n"
        "\n"
        "[Basic]\n"
        "media_types=application/test\n"
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
        "[Qos_0]\n"
        "service_id=" TEST_CONFIG_SERVICE_NAME_1 "\n"
        "media_types=media/test\n"
        "send_flow_spec=1 2 3 4 5\n"
        "receive_flow_spec=1 2 3\n"
        "\n"
        "[Qos_1]\n"
        "service_id=" TEST_CONFIG_SERVICE_NAME_2 "\n"
        "media_types=media/test\n"
        "send_flow_spec=1 2 3 4 5\n"
        "receive_flow_spec=1 2 3\n"
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
        "[Cap_0]\n"
        "sector_id=Session\n"
        "message_type=Req\n"
        "sdp_count=1\n"
        "sdp_0=a=attribute:value\n"
        "\n"
        "\n"
        "[Cap_1]\n"
        "sector_id=StreamAudio\n"
        "message_type=Resp\n"
        "sdp_count=1\n"
        "sdp_0=a=attribute:value\n"
        "\n"
        "\n"
        "[Mprof_0]\n"
        "service_id=" TEST_CONFIG_SERVICE_NAME_1 "\n"
        "profile=" TEST_CONFIG_MEDIA "\n"
        "\n"
        "[Mprof_1]\n"
        "service_id=" TEST_CONFIG_SERVICE_NAME_2 "\n"
        "profile=" TEST_CONFIG_MEDIA "\n"
        "\n"
        "[Connection_0]\n"
        "service_id=" TEST_CONFIG_SERVICE_NAME_1 "\n"
        "\n"
        "[Connection_1]\n"
        "service_id=" TEST_CONFIG_SERVICE_NAME_2 "\n"
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
