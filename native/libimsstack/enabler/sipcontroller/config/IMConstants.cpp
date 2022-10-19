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
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_FILETRANSFER[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.ft\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_HTTP_FILETRANSFER[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.fthttp\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_GEOLOCATIONPUSH[] =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcs.geopush\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CPM_SESSION[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CPM_MSG[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.msg\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CPM_DEFERRED[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.deferred\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CPM_LARGE_MSG[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CPM_FILE_TRANSFER[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.filetransfer\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CPM_SYSTEM_MSG[] =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.systemmsg\"";
PUBLIC GLOBAL const IMS_CHAR IMConstants::TAG_CPM_CFS[] = "+g.gsma.rcs.msgrevoke";