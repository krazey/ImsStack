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
package com.android.imsstack.imsservice.mmtel;

/**
 * Listener for IMS Registration feature capability change.
 */
public interface IRegistrationFeatureListener {
    /**
     * This is invoked when IMS Registration state changed.
     */
    void onRegistrationFeatureChanged();

    /**
     * This is invoked when available IMS capabilities changed.
     *
     * @param enabledFeatures Features that have been enabled as capability has changed.
     * @param disabledFeatures Features that have been disabled as capability has changed.
     */
    void onAvailableFeatureChanged(int enabledFeatures, int disabledFeatures);
}
