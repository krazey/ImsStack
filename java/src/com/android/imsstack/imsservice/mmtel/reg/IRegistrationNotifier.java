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

package com.android.imsstack.imsservice.mmtel.reg;

import android.annotation.NonNull;
import android.net.Uri;

import com.android.imsstack.enabler.aos.IAosRegistrationListener.ReasonCode;

import java.util.Set;

public interface IRegistrationNotifier {
    /**
     * Notify the application that the device is connected to the IMS network.
     *
     * @param regType a type of the registration.(@see IAosRegistrationListener.RegistrationType)
     * @param networkType the radio access technology.
     * @param featureTags set of strings containing the MMTEL feature tags associated with the IMS
     * registration.
     */
    void notifyRegistered(int regType, int networkType, @NonNull Set<String> featureTags);

    /**
     * Notify the application that the device is trying to connect to the IMS network.
     *
     * @param regType a type of the registration.(@see IAosRegistrationListener.RegistrationType)
     * @param networkType the radio access technology.
     * @param featureTags set of strings containing the MMTEL feature tags associated with the IMS
     * registration.
     */
    void notifyRegistering(int regType, int networkType, @NonNull Set<String> featureTags);

    /**
     * Notify the application that the device is disconnected from the IMS network.
     *
     * @param regType a type of the registration.(@see IAosRegistrationListener.RegistrationType)
     * @param networkType the radio access technology.
     * @param reason the disconnected reason.
     * @param message the disconnected message.
     */
    void notifyDeregistered(int regType, int networkType, ReasonCode reason, String message);

    /**
     * Notify the framework that the handover from the current radio technology
     * to the other technology has failed.
     *
     * @param regType a type of the registration.(@see IAosRegistrationListener.RegistrationType)
     * @param networkType the current network type (before handover)
     * @param reason the handover failure reason.
     * @param message the handover failure message.
     */
    void notifyTechnologyChangeFailed(
            int regType, int networkType, ReasonCode reason, String message);

    /**
     * This device's subscriber associated {@link Uri}s have changed, which are used to filter
     * out this device's {@link Uri}s during conference calling.
     *
     * @param uris the network provisioned public user identities.
     */
    void notifyAssociatedUriChanged(Uri[] uris);
}
