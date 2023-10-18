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
#include "RegInfoConst.h"

// elements
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ELEMENT_CONTACT[] = "contact";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ELEMENT_REGINFO[] = "reginfo";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ELEMENT_REGISTRATION[] = "registration";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ELEMENT_URI[] = "uri";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ELEMENT_DISPLAY_NAME[] = "display-name";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ELEMENT_PUB_GRUU[] = "pub-gruu";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ELEMENT_TEMP_GRUU[] = "temp-gruu";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ELEMENT_UNKNOWN_PARAM[] = "unknown-param";

// attributes
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_AOR[] = "aor";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_CALLID[] = "callid";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_CSEQ[] = "cseq";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_DURATION_REGISTERED[] = "duration-registered";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_EVENT[] = "event";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_EXPIRES[] = "expires";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_FIRST_CSEQ[] = "first-cseq";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_ID[] = "id";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_NAME[] = "name";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_Q[] = "q";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_RETRY_AFTER[] = "retry-after";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_STATE[] = "state";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_URI[] = "uri";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_VERSION[] = "version";

// values of "event" attribute
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_EVENT_REGISTERED[] = "registered";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_EVENT_CREATED[] = "created";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_EVENT_REFRESHED[] = "refreshed";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_EVENT_SHORTENED[] = "shortened";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_EVENT_EXPIRED[] = "expired";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_EVENT_DEACTIVATED[] = "deactivated";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_EVENT_PROBATION[] = "probation";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_EVENT_UNREGISTERED[] = "unregistered";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_EVENT_REJECTED[] = "rejected";

// values of "state" attribute
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_STATE_FULL[] = "full";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_STATE_PARTIAL[] = "partial";

PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_STATE_INIT[] = "init";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_STATE_ACTIVE[] = "active";
PUBLIC GLOBAL const IMS_CHAR RegInfoConst::ATTR_STATE_TERMINATED[] = "terminated";
