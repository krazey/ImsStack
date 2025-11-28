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
#ifndef SDP_OFFER_ANSWER_H_
#define SDP_OFFER_ANSWER_H_

class SdpOfferAnswer
{
public:
    // Result type of SDP offer/answer
    enum
    {
        RESULT_NOT_CHANGED,
        RESULT_NOT_DONE,
        RESULT_SUCCESS,
        RESULT_FAILURE,
        RESULT_QOS_PRECONDITION_PRESENT,
        RESULT_NOT_FOUND
    };

    // Comparison flag for SDP offer/answer
    enum
    {
        F_MEDIA_GROUP = 0x01,
        F_MEDIA_PARAM = 0x02,
    };

    enum
    {
        MEDIA_UPDATE_OFFER_SENT,
        MEDIA_UPDATE_OFFER_SENT_ACCEPTED,
        MEDIA_UPDATE_OFFER_SENT_REJECTED,
        MEDIA_UPDATE_OFFER_RECEIVED,
        MEDIA_UPDATE_OFFER_RECEIVED_ACCEPTED,
        MEDIA_UPDATE_OFFER_RECEIVED_REJECTED
    };
};

#endif
