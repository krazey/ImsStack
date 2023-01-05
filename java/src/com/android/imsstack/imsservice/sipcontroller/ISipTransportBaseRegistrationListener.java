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

package com.android.imsstack.imsservice.sipcontroller;
/**
 * This interface is used to listen for the registration APIs called by framework to SipTransport
 * {@link com.android.imsstack.imsservice.mmtel.ImsRegistrationImpl }
 */
public interface ISipTransportBaseRegistrationListener {
    /**
     * This method will request registration for all sip delegates belongs to this sip transport
     */
    void triggerSipTransportDelegateRegistration();

    /**
     * This method will request de-registration for all sip delegates belongs to this sip transport
     */
    void triggerSipTransportDelegateDeregistration();
}
