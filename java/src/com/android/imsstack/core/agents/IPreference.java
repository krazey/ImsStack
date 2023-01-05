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

/**
 * This provides an interface to get and set the preference.
 */
public interface IPreference extends IAgent {
    /**
     * Returns Boolean type value from common ims shared preference file
     */
    boolean getPreferenceBoolValue(String key);

    /**
     * Returns Boolean type value from common ims shared preference file
     */
    boolean getPreferenceBoolValue(String fileName, String key);

    /**
     * Returns Boolean type value from common ims shared preference file
     */
    boolean getPreferenceBoolValue(String key, int slotId);

    /**
     * Returns Boolean type value from common ims shared preference file
     */
    boolean getPreferenceBoolValue(String fileName, String key, int slotId);

    /**
     * Returns String type value from common ims shared preference file
     */
    String getPreferenceStrValue(String key);

    /**
     * Returns Boolean type value from target shared preference file
     */
    String getPreferenceStrValue(String fileName, String key);

    /**
     * Returns String type value from common ims shared preference file
     */
    String getPreferenceStrValue(String key, int slotId);

    /**
     * Returns Boolean type value from target shared preference file
     */
    String getPreferenceStrValue(String fileName, String key, int slotId);

    /**
     * Set String value to common ims shared preference file
     */
    void setPreferenceStrValue(String key, String value);

    /**
     * Set String value to target shared preference file
     */
    void setPreferenceStrValue(String fileName, String key, String value);

    /**
     * Set String value to common ims shared preference file
     */
    void setPreferenceStrValue(String key, String value, int slotId);

    /**
     * Set String value to target shared preference file
     */
    void setPreferenceStrValue(String fileName, String key, String value, int slotId);

    /**
     * Remove String value from common ims shared preference file
     */
    //2015-11-12 , joengtae.kim@ , add method for remove preference data
    void removePreferenceValue(String key);

    /**
     * Remove String value from target shared preference file
     */
    void removePreferenceValue(String fileName, String key);

    /**
     * Remove String value from common ims shared preference file
     */
    //2015-11-12 , joengtae.kim@ , add method for remove preference data
    void removePreferenceValue(String key, int slotId);

    /**
     * Remove String value from target shared preference file
     */
    void removePreferenceValue(String fileName, String key, int slotId);
}
