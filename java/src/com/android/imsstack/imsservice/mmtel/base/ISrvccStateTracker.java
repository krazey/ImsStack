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

public interface ISrvccStateTracker {
    /**
     * Adds a listener for SRVCC events.
     */
    public void addListener(ISrvccStateListener listener);

    /**
     * Removes a listener for SRVCC events.
     */
    public void removeListener(ISrvccStateListener listener);

    /**
     * Checks if SRVCC is successfully completed.
     */
    public boolean isSrvccCompleted();

    /**
     * Checks if SRVCC is ongoing.
     */
    public boolean isSrvccPending();
}
