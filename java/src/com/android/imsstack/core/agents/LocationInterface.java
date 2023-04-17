/*
 * Copyright (C) 2023 The Android Open Source Project
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
package com.android.imsstack.core.agents;

import android.annotation.IntDef;
import android.annotation.NonNull;
import android.location.Location;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * This provides an interface to access the location information and control the location settings
 * and rules to obtain a correct location information.
 */
public interface LocationInterface extends IAgent {
    /** Cached locations */
    int MAX_LOCATIONS = 3;
    int CACHE_I_GPS = 0;
    int CACHE_I_NLP = 1;
    int CACHE_I_FLP = 2;

    /** Indicates all the location information. */
    int LOCATION_CATEGORY_ALL = 0;
    /** Indicates the positioning and country information only. */
    int LOCATION_CATEGORY_POSITION_N_COUNTRY = 1;
    /** Indicates the positioning information only. */
    int LOCATION_CATEGORY_POSITION = 2;

    @IntDef(value = {
        LOCATION_CATEGORY_ALL,
        LOCATION_CATEGORY_POSITION_N_COUNTRY,
        LOCATION_CATEGORY_POSITION
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface LocationCategory {}

    /**
     * Initializes last known location information.
     */
    void initLastKnownLocation();

    /**
     * Returns the location policy for this location agent.
     *
     * @return The current {@link LocationPolicy} instance.
     */
    LocationPolicy getLocationPolicy();

    /**
     * Sets the location policy for this location agent.
     *
     * @param lp The location policy.
     */
    void setLocationPolicy(LocationPolicy lp);

    /**
     * Returns the cached location information.
     */
    @NonNull
    Location[] getCachedLocations();

    /**
     * Returns last known country code.
     */
    String getLastKnownCountryCode();

    /**
     * Returns the best location information from the last known location.
     *
     * @param category The location category. Possible values are:
     *                 {@link #LOCATION_CATEGORY_ALL},
     *                 {@link #LOCATION_CATEGORY_POSITION_N_COUNTRY},
     *                 {@link #LOCATION_CATEGORY_POSITION}
     */
    String[] getLastKnownLocation(@LocationCategory int category);

    /**
     * Starts listening the location information with the given interval.
     *
     * @param updateIntervalSec The location update interval in seconds.
     */
    void startListeningForLocation(int updateIntervalSec);

    /**
     * Stops listening the location information.
     */
    void stopListeningForLocation();

    /**
     * Starts an instant location update (one-time update).
     */
    void startInstantLocationUpdate();
}
