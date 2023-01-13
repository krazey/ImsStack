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
package com.android.imsstack.enabler.sipcontroller.impl;

/**
 * SipControllerConstants
 * Native CMD and Indication
 */
public class SipControllerConstants {

    // Base Event
    private static final int IMS_MSG_APP_INTERNAL = 10000;
    private static final int IMS_MSG_SIP_DELEGATE = (IMS_MSG_APP_INTERNAL + 6000);

    // Send Message
    public static final int SEND_MESSAGE_CMD                 = (IMS_MSG_SIP_DELEGATE + 11);
    // Colse Ongoing Session
    public static final int CLOSE_SESSION_CMD                = (IMS_MSG_SIP_DELEGATE + 12);
    // Notify Error that Received Message from Native
    public static final int NOTIFY_MESSAGE_RECEIVE_ERROR_CMD   = (IMS_MSG_SIP_DELEGATE + 13);
    // Received Message
    public static final int MESSAGE_RECEIVED_IND             = (IMS_MSG_SIP_DELEGATE + 21);
    // Message Sent
    public static final int MESSAGE_SENT_IND                 = (IMS_MSG_SIP_DELEGATE + 22);
    // Receive that Send Message Failure
    public static final int SEND_MESSAGE_FAILURE_IND          = (IMS_MSG_SIP_DELEGATE + 23);

    // Update sip registration
    public static final int UPDATE_DELEGATE_REGISTRATION_CMD = (IMS_MSG_SIP_DELEGATE + 101);
    // Trigger sip de-registration
    public static final int TRIGGER_DELEGATEDE_REGISTRATION_CMD = (IMS_MSG_SIP_DELEGATE + 102);

    // received registration update
    public static final int ONREGISTRATION_UPDATED_IND = (IMS_MSG_SIP_DELEGATE + 111);
    // received configuration update
    public static final int ONCONFIGURATION_UPDATED_IND = (IMS_MSG_SIP_DELEGATE + 121);

    // Registration state
    public static final int STATE_DEREGISTERING = 0;
    public static final int STATE_DEREGISTERED = 1;
    public static final int STATE_REGISTERING = 2;
    public static final int STATE_REGISTERED = 3;
}
