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
#ifndef SIP_D_STATE_H_
#define SIP_D_STATE_H_

#include "ImsTypeDef.h"

class SipDState
{
public:
    /*
    1) UAC side
        a) Init to Early
            - Receiving a 1xx response(except for 100) to INVITE(/SUBSCRIBE/REFER) request
        b) Init to Confirmed
            - Receiving 200 OK to SUBSCRIBE
            - Receiving NOTIFY request
        c) Init to Terminated
            - Receiving a failure final response to INVITE/SUBSCRIBE/REFER request
        d) Early to Early
            - Receiving a 1xx response(except for 100) to INVITE(/SUBSCRIBE/REFER) request
    - Sending PRACK request
    - Receiving a 200 response to PRACK request
    e) Early to Terminated
    - Receiving a failure final response to INVITE(/SUBSCRIBE/REFER) request
    f) Early to Confirmed
    - Receiving 2xx response to INVITE(/SUBSCRIBE/REFER) request
    g) Confirmed to Confirmed
    - Receiving a response
    h) Confirmed to Terminated
    - BYE - 200 OK
    - un-SUBSCRIBE

    2) UAS side
    a) Init to Early
    - Sending a 1xx response(except for 100) to INVITE(/SUBSCRIBE/REFER) request
    b) Init to Confirmed
    - Sending 200 OK to SUBSCRIBE
    - Sending NOTIFY request
    c) Init to Terminated
    - Sending a failure final response to INVITE/SUBSCRIBE/REFER request
    d) Early to Early
    - Sending a 1xx response(except for 100) to INVITE(/SUBSCRIBE/REFER) request
    - Receiving PRACK request
    - Sending a 200 response to PRACK request
    e) Early to Terminated
    - Sending a failure final response to INVITE(/SUBSCRIBE/REFER) request
    f) Early to Confirmed
    - Sending 2xx response to INVITE(/SUBSCRIBE/REFER) request
    g) Confirmed to Confirmed
    - Sending a response
    h) Confirmed to Terminated
    - BYE - 200 OK
    - un-SUBSCRIBE
    */

    /// State of Dialog
    enum
    {
        STATE_INIT = 0,

        STATE_TERMINATED,
        STATE_EARLY,
        STATE_CONFIRMED,

        STATE_MAX
    };

    /// MAIN events for a dialog processing when message is received
    enum
    {
        ACTION_IGNORE = 0,
        ACTION_TRANSIT_STATE = 1,
        ACTION_DESTROY_USAGE = 2,
        ACTION_DESTROY_DIALOG = 3
    };

    /// TRIGGER events for dialog state transition
    enum
    {
        TRIGGER_INIT = 0

        // invite usage & subscribe usage will define this trigger events in details.
    };
};

#endif
