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
 * SipControllerInternalMsgDef
 * Native CMD and Indication
 */
public class SipControllerInternalMsgDef {

    //Base Event
    private static final int IMS_MSG_APP_INTERNAL = 10000;
    private static final int IMS_MSG_SIP_DELEGATE = (IMS_MSG_APP_INTERNAL + 6000);

    /*
     * Commands from SipController(Java) and Jni
     */
    // Send Message
    public static final int SENDMESSAGE_CMD                 = (IMS_MSG_SIP_DELEGATE + 11);
    // Colse Ongoing Session
    public static final int CLOSESESSION_CMD                = (IMS_MSG_SIP_DELEGATE + 12);
    // Notify Error that Received Message from Native
    public static final int NOTIFYMESSAGERECEIVEERROR_CMD   = (IMS_MSG_SIP_DELEGATE + 13);
    // Received Message
    public static final int MESSAGERECEIVED_IND             = (IMS_MSG_SIP_DELEGATE + 21);
    // Message Sent
    public static final int MESSAGESENT_IND                 = (IMS_MSG_SIP_DELEGATE + 22);
    // Receive that Send Message Failure
    public static final int SENDMESSAGEFAILURE_IND          = (IMS_MSG_SIP_DELEGATE + 23);
}
