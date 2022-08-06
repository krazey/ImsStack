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
 * This provides an interface to control the wake lock while processing SIP signalling.
 */
public interface IWakeLock extends IAgent {
    /**
     * Sets the device stay on until timer expired.
     */
    void acquire(int timeout);

    /**
     * Sets the device stay on until timer expired when screen is off.
     */
    void acquire(int timeout, boolean screenOffOnly);
}
