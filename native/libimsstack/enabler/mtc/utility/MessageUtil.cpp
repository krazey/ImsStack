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

#include "ImsTypeDef.h"
#include "utility/MessageUtil.h"

const IMS_CHAR MessageUtil::STR_199[] = "199";
const IMS_CHAR MessageUtil::STR_ALERT_URN_CALL_WAITING[] = "<urn:alert:service:call-waiting>";
const IMS_CHAR MessageUtil::STR_ANONYMOUS[] = "Anonymous";
const IMS_CHAR MessageUtil::STR_CONTENT_DISPOSITION_RECIPIENT_LIST[] = "recipient-list";
const IMS_CHAR MessageUtil::STR_CONTENT_ID[] = "Content-ID";
const IMS_CHAR MessageUtil::STR_CONTENT_TYPE_3GPP_IMS_XML[] = "3gpp-ims+xml";
const IMS_CHAR MessageUtil::STR_CONTENT_TYPE_APPLICATION[] = "application";
const IMS_CHAR MessageUtil::STR_CONTENT_TYPE_RESOURCE_LISTS_XML[] =
        "application/resource-lists+xml";
const IMS_CHAR MessageUtil::STR_CONTENT_TYPE_SIP_FRAG[] = "message/sipfrag";
const IMS_CHAR MessageUtil::STR_ACCEPT_TYPE_APPLICATION_SDP[] = "application/sdp";
const IMS_CHAR MessageUtil::STR_ACCEPT_TYPE_APPLICATION_3GPP_CURRENT_LOCATION_DISCOVERY_XML[] =
        "application/vnd.3gpp.current-location-discovery+xml";
const IMS_CHAR MessageUtil::STR_ACCEPT_TYPE_APPLICATION_3GPP_IMS_XML[] = "application/3gpp-ims+xml";
const IMS_CHAR MessageUtil::STR_HEADER[] = "header";
const IMS_CHAR MessageUtil::STR_HISTINFO[] = "histinfo";
const IMS_CHAR MessageUtil::STR_ICSI[] = "+g.3gpp.icsi-ref=";
const IMS_CHAR MessageUtil::STR_ID[] = "id";
const IMS_CHAR MessageUtil::STR_NONE[] = "none";
const IMS_CHAR MessageUtil::STR_PACKAGE_CURRENT_LOCATION_DISCOVERY[] =
        "g.3gpp.current-location-discovery";
const IMS_CHAR MessageUtil::STR_PARAMETER_IS_FOCUS[] = "isfocus";
const IMS_CHAR MessageUtil::STR_PRECONDITION[] = "precondition";
const IMS_CHAR MessageUtil::STR_REASON_FAILURE_TO_TRANSITION[] =
        "SIP;cause=487;text=\"failure to transition to CS domain\"";
const IMS_CHAR MessageUtil::STR_REASON_HANDOVER_CANCELLED[] =
        "SIP;cause=487;text=\"handover cancelled\"";
const IMS_CHAR MessageUtil::STR_RELEASE_CAUSE_1[] = "RELEASE_CAUSE;cause=1;text=\"User ends call\"";
const IMS_CHAR MessageUtil::STR_RELEASE_CAUSE_2[] =
        "RELEASE_CAUSE;cause=2;text=\"RTP/RTCP time-out\"";
const IMS_CHAR MessageUtil::STR_RELEASE_CAUSE_3[] =
        "RELEASE_CAUSE;cause=3;text=\"Media bearer loss\"";
const IMS_CHAR MessageUtil::STR_RELEASE_CAUSE_4[] =
        "RELEASE_CAUSE;cause=4;text=\"SIP timeout - no ACK\"";
const IMS_CHAR MessageUtil::STR_RELEASE_CAUSE_5[] =
        "RELEASE_CAUSE;cause=5;text=\"SIP response time-out\"";
const IMS_CHAR MessageUtil::STR_RELEASE_CAUSE_6[] =
        "RELEASE_CAUSE;cause=6;text=\"Call-setup time-out\"";
const IMS_CHAR MessageUtil::STR_REPLACES[] = "replaces";
const IMS_CHAR MessageUtil::STR_SERVICE[] = "service";
const IMS_CHAR MessageUtil::STR_SOS[] = "sos";
const IMS_CHAR MessageUtil::STR_SOS_AMBULANCE[] = "sos.ambulance";
const IMS_CHAR MessageUtil::STR_SOS_ANIMAL_CONTROL[] = "sos.animal-control";
const IMS_CHAR MessageUtil::STR_SOS_COUNTRY_SPECIFIC[] = "sos.country-specific";
const IMS_CHAR MessageUtil::STR_SOS_FIRE[] = "sos.fire";
const IMS_CHAR MessageUtil::STR_SOS_GAS[] = "sos.gas";
const IMS_CHAR MessageUtil::STR_SOS_MARINE[] = "sos.marine";
const IMS_CHAR MessageUtil::STR_SOS_MOUNTAIN[] = "sos.mountain";
const IMS_CHAR MessageUtil::STR_SOS_PHYSICIAN[] = "sos.physician";
const IMS_CHAR MessageUtil::STR_SOS_POISON[] = "sos.poison";
const IMS_CHAR MessageUtil::STR_SOS_POLICE[] = "sos.police";
const IMS_CHAR MessageUtil::STR_SRVCC_FEATURE_A[] = "+g.3gpp.srvcc-alerting";
const IMS_CHAR MessageUtil::STR_SRVCC_FEATURE_B[] = "+g.3gpp.ps2cs-srvcc-orig-pre-alerting";
const IMS_CHAR MessageUtil::STR_SRVCC_FEATURE_M[] = "+g.3gpp.mid-call";
const IMS_CHAR MessageUtil::STR_SUPPORTED[] = "supported";
const IMS_CHAR MessageUtil::STR_TIMER[] = "timer";
const IMS_CHAR MessageUtil::STR_URN[] = "urn";
const IMS_CHAR MessageUtil::STR_VIDEO[] = "video";
const IMS_CHAR MessageUtil::STR_TEXT[] = "text";

// carrier specific
const IMS_CHAR MessageUtil::STR_P_SKT_BYE_CAUSE[] = "P-SKT-BYE-CAUSE";
const IMS_CHAR MessageUtil::STR_P_TTA_VOLTE_INFO[] = "P-TTA-VoLTE-Info";
const IMS_CHAR MessageUtil::STR_AVCHANGE[] = "avchange";
const IMS_CHAR MessageUtil::STR_REASON_USER_SESSIONEXPIRED[] = "Reason.User.SessionExpired";
const IMS_CHAR MessageUtil::STR_P_COM_ENABLETRANSCODING[] = "P-com.EnableTranscoding";
