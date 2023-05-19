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

/**
 * A class to provide an interface to read/write the preferences of ImsStack.
 */
public interface PreferenceInterface extends IAgent {
    /**
     * Returns a string value from the default preference file for a specified slot.
     *
     * @param key The key to retrieve.
     * @param slotId The slot-id to find a correct file.
     * @return A string value.
     */
    String getString(String key, int slotId);

    /**
     * Returns a string value from the specified preference file for a specified slot.
     * If {@code fileName} is empty, then returns a string value from the default preference file.
     *
     * @param fileName The shared preference file name.
     * @param key The key to retrieve.
     * @param slotId The slot-id to find a correct file.
     * @return A string value.
     */
    String getString(String fileName, String key, int slotId);

    /**
     * Puts a string value to the default preference file for a specified slot.
     *
     * @param key The key to update.
     * @param value The string value.
     * @param slotId The slot-id to find a correct file.
     * @return {@code true} if the key and value is successfully set, {@code false} otherwise.
     */
    boolean putString(String key, String value, int slotId);

    /**
     * Puts a string value to the specified preference file for a specified slot.
     * If {@code fileName} is empty, then puts a string value to the default preference file.
     *
     * @param fileName The shared preference file name.
     * @param key The key to update.
     * @param value The string value.
     * @param slotId The slot-id to find a correct file.
     * @return {@code true} if the key and value is successfully set, {@code false} otherwise.
     */
    boolean putString(String fileName, String key, String value, int slotId);

    /**
     * Removes a preference key from the default preference file for a specified slot.
     *
     * @param key The key to remove.
     * @param slotId The slot-id to find a correct file.
     */
    void remove(String key, int slotId);

    /**
     * Returns a preference key from the specified preference file for a specified slot.
     * If {@code fileName} is empty, then removes a preference key from the default preference file.
     *
     * @param fileName The shared preference file name.
     * @param key The key to remove.
     * @param slotId The slot-id to find a correct file.
     */
    void remove(String fileName, String key, int slotId);
}
