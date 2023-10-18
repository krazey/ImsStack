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
#include "config/IMConstants.h"

PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_OMA_IM[] = "+g.oma.sip-im";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CHAT_IM[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.im\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_FILETRANSFER[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.ft\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_HTTP_FILETRANSFER[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_SMS_FILETRANSFER[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.ftsms\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_FILE_TRANSFER[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.filetransfer\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_GEOLOCATIONPUSH[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_SMS_GEOLOCATIONPUSH[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geosms\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_SESSION[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_STANDALONE_PAGER[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_STANDALONE_DEFERRED[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.deferred\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_STANDALONE_LARGE[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_STANDALONE_PAGER_LARGE[] =
        "+g.gsma.rcs.cpm.pager-large";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_SYSTEM_MSG[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.systemmsg\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CPM_CFS[] = "+g.gsma.rcs.msgrevoke";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CPM_NFS[] = "+g.gsma.rcs.msgfallback";
PUBLIC GLOBAL const IMS_CHAR IMConstants::STR_UP_URN_REVOKE[] =
        "urn:gsma:params:xml:ns:rcs:rcs:rcsrevoke";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CALL_COMPOSER_ENRICHED_CALLING[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callcomposer\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CALL_COMPOSER_VIA_TELEPHONY[] =
        "+g.gsma.callcomposer";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_POST_CALL[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.callunanswered\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_SHARED_MAP[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedmap\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_SHARED_SKETCH[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.gsma.sharedsketch\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CHATBOT_COMMUNICATION_USING_SESSION[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CHATBOT_COMMUNICATION_USING_STANDALONE_MSG[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.chatbot.sa\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CHATBOT_VERSION_SUPPORTED[] =
        "+g.gsma.rcs.botversion=\"#=1\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CHATBOT_VERSION_V2_SUPPORTED[] =
        "+g.gsma.rcs.botversion=\"#=1,#=2\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CHATBOT_ROLE[] = "+g.gsma.rcs.isbot";
