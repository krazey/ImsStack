/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.servercontrol;

import java.util.HashMap;
import java.util.Map;

/**
 * Manages configuration settings for SIP messages.
 */
public class MessageConfig {
    private final Map<String, String> mConfigs = new HashMap<>();

    /**
     * Adds a configuration key-value pair to the message config.
     *
     * @param key The config key (e.g., "Delay")
     * @param value The config value associated with the key
     */
    public void addConfig(String key, String value) {
        mConfigs.put(key, value);
    }

    /**
     * Gets the map of configuration settings.
     *
     * @return A map containing the message config data.
     */
    public Map<String, String> getConfigs() {
        return mConfigs;
    }
}
