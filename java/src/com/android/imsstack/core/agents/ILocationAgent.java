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
package com.android.imsstack.core.agents;

import android.location.Location;

import com.android.imsstack.system.ISystemAPILocation;

/**
 * This provides an interface to access the location information and control the location settings
 * and rules to obtain a correct location information.
 */
public interface ILocationAgent extends ISystemAPILocation {
    /** Cached locations */
    int MAX_LOCATIONS = 3;
    int CACHE_I_GPS = 0;
    int CACHE_I_NLP = 1;
    int CACHE_I_FLP = 2;

    // Location type
    int LOCATION_ALL = 0;
    int LOCATION_POSITION_N_COUNTRY = 1;
    int LOCATION_POSITION = 2;

    /**
     * Initializes last known location information.
     */
    void initLastKnownLocation();

    /**
     * Returns the location policy for this location agent.
     */
    LocationPolicy getLocationPolicy();

    /**
     * Sets the location policy for this location agent.
     */
    void setLocationPolicy(LocationPolicy lp);

    /**
     * Returns the cached location information.
     */
    Location[] getCachedLocations();

    /**
     * Returns last known country code.
     */
    String getLastKnownCountryCode();

    /**
     * Returns the best location information from the last known location.
     */
    String[] getLastKnownLocation(int type);
}
