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

package com.android.imsstack.imsservice.mmtel.base;

import android.telephony.ims.ImsCallProfile;

/**
 * A class that provides any policy to determine whether geolocation is required for the call.
 */
public interface ICallLocationPolicy {
    /**
     * Checks if location information is required for the specified callee and call profile.
     */
    public boolean isLocationRequired(String callee, ImsCallProfile profile);

    /**
     * Checks if GPS location is required.
     */
    public boolean isPositionInfoRequired();

    /**
     * Returns the validity period of the last known location information.
     * Milli-seconds
     */
    public long getValidityPeriod();

    /**
     * Returns the waiting time after location information is requested.
     * Milli-seconds
     */
    public long getWaitingTimeForLocationFix();
}
