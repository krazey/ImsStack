/*
 * Copyright (C) 2021 The Android Open Source Project
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

package com.android.imsstack.imsservice.sipcontroller.remote;

import android.annotation.NonNull;
import android.telephony.ims.DelegateRegistrationState;

/**
 * The interface to get delegate registration updates
 */
public interface ISipTransportRemoteRegistrationListener {
    /**
     * Update the ims registration with new values
     * @param updatedRegistrationValues after registration update
     * @param slotId for which registration is changed
     */
    void updateRegistration(@NonNull DelegateRegistrationState updatedRegistrationValues,
           int slotId);
}
