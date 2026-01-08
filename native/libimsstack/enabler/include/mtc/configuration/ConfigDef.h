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

#ifndef CONFIG_DEF_H_
#define CONFIG_DEF_H_

enum class TerminateType
{
    USER_ENDS_CALL = 0,
    RTP_TIMEOUT = 1,
    USER_ENDS_CALL_AND_RTP_TIMEOUT = 2,
    MEDIA_BEARER_LOSS = 3,
    SIP_TIMEOUT = 4,
    SIP_RESPONSE_TIMEOUT = 5,
    USER_ENDS_AND_SIP_RESPONSE_TIMEOUT = 6,
    CALL_SETUP_TIMEOUT = 7,
    TERMINATING_EARLY_DIALOG = 8,
    SESSION_REFRESH_FAILURE = 9,
    CONFERENCE_CALL_JOINED = 10,
    NETWORK_LOST = 11,
    MEDIA_NOT_SUPPORTED = 12,
    MEDIA_BEARER_NOT_MET = 13,
};

enum class RejectType
{
    ON_CS_CALL = 0,
    ON_VI_LTE_AND_NO_LTE = 1,
    ON_CONNECTING_CALL = 2,
    EXCEEDS_MAX_CALL = 3,
    ON_CONVERTING = 4,
    NEGOTIATION_FAILURE = 5,
    NO_ANSWER_BY_USER = 6,
    VOWIFI_OFF = 7,
    USER_REJECT = 8,
    ACCESS_CLASS_BLOCKED = 9,
    VOPS_OFF = 10,
};

enum class MessageTypeForGeolocationPidf
{
    INVITE = 0,
    PROVISIONAL_RESPONSE = 1,
    FINAL_SUCCESS_RESPONSE = 2,
    FINAL_FAILURE_RESPONSE = 3,
};

#endif
