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
#include "provider/AosString.h"

const IMS_CHAR AosString::STR_SEC_AGREE[] = "sec-agree";
const IMS_CHAR AosString::STR_APPLICATION[] = "application";
const IMS_CHAR AosString::STR_3GPP_IMS_XML[] = "3gpp-ims+xml";
const IMS_CHAR AosString::STR_VERSTAT_FEATURE[] = "+g.3gpp.verstat";
const IMS_CHAR AosString::STR_NW_INIT_USSI_FEATURE[] = "+g.3gpp.nw-init-ussi";
const IMS_CHAR AosString::STR_RTT_FEATURE[] = "text";
const IMS_CHAR AosString::STR_ACCESS_TYPE_FEATURE[] = "+g.3gpp.accesstype";
const IMS_CHAR AosString::STR_ACCESS_TYPE_CELLULAR[] = "\"cellular\"";
const IMS_CHAR AosString::STR_ACCESS_TYPE_CELLULAR2[] = "\"cellular2\"";
const IMS_CHAR AosString::STR_ACCESS_TYPE_WLAN[] = "\"wlan\"";
const IMS_CHAR AosString::STR_ACCESS_TYPE_WLAN1[] = "\"wlan1\"";
const IMS_CHAR AosString::STR_P_CELLULAR_NETWORK_INFO[] = "P-Cellular-Network-Info";
const IMS_CHAR AosString::STR_P_LAST_ACCESS_NETWORK_INFO[] = "P-Last-Access-Network-Info";
const IMS_CHAR AosString::STR_CS[] = "cs";
const IMS_CHAR AosString::STR_CS_WITH_DQ[] = "\"cs\"";
const IMS_CHAR AosString::STR_VOLTE[] = "volte";
const IMS_CHAR AosString::STR_REG_RETRY_TIME0[] = "vendor.ims.reg_retry_time0";
const IMS_CHAR AosString::STR_REG_RETRY_TIME1[] = "vendor.ims.reg_retry_time1";
const IMS_CHAR AosString::STR_EMERGENCY_CALL_FAIL_CAUSE[] = "vendor.ims.emergency_call_fail_cause";

const IMS_CHAR* FeatureTags::STANDALONE_MSG[4] = {
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg\"",
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg\"",
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.deferred\"",
        "+g.gsma.rcs.cpm.pager-large"};
const IMS_CHAR FeatureTags::CHAT_IM[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im\"";
const IMS_CHAR FeatureTags::CHAT_SESSION[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session\"";
const IMS_CHAR FeatureTags::FILE_TRANSFER[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp\"";
const IMS_CHAR FeatureTags::FILE_TRANSFER_VIA_SMS[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftsms\"";
const IMS_CHAR FeatureTags::CALL_COMPOSER_ENRICHED_CALLING[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callcomposer\"";
const IMS_CHAR FeatureTags::CALL_COMPOSER_VIA_TELEPHONY[] = "+g.gsma.callcomposer";
const IMS_CHAR FeatureTags::POST_CALL[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callunanswered\"";
const IMS_CHAR FeatureTags::SHARED_MAP[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedmap\"";
const IMS_CHAR FeatureTags::SHARED_SKETCH[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedsketch\"";
const IMS_CHAR FeatureTags::GEO_PUSH[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush\"";
const IMS_CHAR FeatureTags::GEO_PUSH_VIA_SMS[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geosms\"";
const IMS_CHAR FeatureTags::CHATBOT_COMMUNICATION_USING_SESSION[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot\"";
const IMS_CHAR FeatureTags::CHATBOT_COMMUNICATION_USING_STANDALONE_MSG[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot.sa\"";
const IMS_CHAR FeatureTags::CHATBOT_VERSION_SUPPORTED[] = "+g.gsma.rcs.botversion=\"#=1\"";
const IMS_CHAR FeatureTags::CHATBOT_VERSION_V2_SUPPORTED[] = "+g.gsma.rcs.botversion=\"#=1,#=2\"";
const IMS_CHAR FeatureTags::CHATBOT_ROLE[] = "+g.gsma.rcs.isbot";
const IMS_CHAR FeatureTags::MMTEL[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.mmtel\"";
const IMS_CHAR FeatureTags::VIDEO[] = "video";
const IMS_CHAR FeatureTags::PRESENCE[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp\"";
const IMS_CHAR FeatureTags::RCS_TELEPHONY[] = "+g.gsma.rcs.telephony";
const IMS_CHAR FeatureTags::CDMALESS[] = "+cdmaless";
