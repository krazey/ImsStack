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

package com.android.imsstack.enabler.sipnativecontroller;

/**
 * SipControllerInternalMsgDef
 * Native CMD and Indication
 */
public class SipControllerInternalMsgDef {

    //temp value
    private static final int IMS_INTERNAL_MSG = 10000;

    // Send Message
    public static final int SENDMESSAGE_CMD                 = (IMS_INTERNAL_MSG + 51);
    // Colse Ongoing Session
    public static final int CLOSEONGOINGSESSION_CMD         = (IMS_INTERNAL_MSG + 52);
    // Notify Error that Received Message from Native
    public static final int NOTIFYMESSAGERECEIVEERROR_CMD   = (IMS_INTERNAL_MSG + 53);
    // Received Message
    public static final int MESSAGERECEIVED_IND             = (IMS_INTERNAL_MSG + 54);
    // Message Sent
    public static final int MESSAGESENT_IND                 = (IMS_INTERNAL_MSG + 55);
    // Receive that Send Message Failure
    public static final int SENDMESSAGEFAILURE_IND          = (IMS_INTERNAL_MSG + 56);
}
