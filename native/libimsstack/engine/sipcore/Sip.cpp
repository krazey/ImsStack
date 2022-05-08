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
#include "ServiceMemory.h"

#include "SIPPrivate.h"

PUBLIC GLOBAL const IMS_CHAR Sip::CONNECTION_SCHEME_SIP[] = "sip";
PUBLIC GLOBAL const IMS_CHAR Sip::CONNECTION_SCHEME_SIPS[] = "sips";

PUBLIC GLOBAL const IMS_CHAR Sip::STR_SIP_VERSION[] = "SIP/2.0";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_SIP_VERSION_ONLY[] = "2.0";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_SIP[] = "sip";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_SIPS[] = "sips";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_TEL[] = "tel";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_UDP[] = "udp";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_TCP[] = "tcp";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_TLS[] = "tls";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_UDP_CAPS[] = "UDP";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_TCP_CAPS[] = "TCP";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_TLS_CAPS[] = "TLS";

PUBLIC GLOBAL const IMS_CHAR Sip::STR_BRANCH_MAGIC_COOKIE[] = "z9hG4bK";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_100REL[] = "100rel";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_ACTIVE[] = "active";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_APPLICATION_SDP[] = "application/sdp";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_EARLY_SESSION[] = "early-session";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_FROM_CHANGE[] = "from-change";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_GRUU[] = "gruu";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_MULTIPART[] = "multipart";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_MIXED[] = "mixed";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_MULTIPART_MIXED[] = "multipart/mixed";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_REFER[] = "refer";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_SEC_AGREE[] = "sec-agree";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_PENDING[] = "pending";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_TERMINATED[] = "terminated";

PUBLIC GLOBAL const IMS_CHAR Sip::STR_BOUNDARY[] = "boundary";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_BRANCH[] = "branch";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_EXPIRES[] = "expires";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_KEEP[] = "keep";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_LR[] = "lr";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_METHOD[] = "method";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_OB[] = "ob";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_RPORT[] = "rport";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_RECEIVED[] = "received";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_REG_ID[] = "reg-id";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_SIP_INSTANCE[] = "+sip.instance";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_TYPE[] = "type";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_TRANSPORT_EXT[] = "transport-ext";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_FROM_TAG[] = "from-tag";
PUBLIC GLOBAL const IMS_CHAR Sip::STR_TO_TAG[] = "to-tag";
PUBLIC GLOBAL const AString Sip::STR_TAG("tag");
