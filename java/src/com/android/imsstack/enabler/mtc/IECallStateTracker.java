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

package com.android.imsstack.enabler.mtc;

public interface IECallStateTracker {
    /**
     * Adds a listener for ECBM (Emergency CallBack Mode).
     */
    public void addEcbmListener(EcbmListener listener);

    /**
     * Removes a listener for ECBM (Emergency CallBack Mode).
     */
    public void removeEcbmListener(EcbmListener listener);

    /**
     * Exits emergency callback mode.
     */
    public void exitEmergencyCallbackMode(boolean bExitedByNewCall);

    /**
     * Checks if ECBM is entered.
     */
    public boolean isEcbmEntered();
}
