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

/**
 * Provides APIs for keeping, tracking calls.
 */
public interface IMtcCallManager {
    /**
     * Initializes this object.
     */
    void init();

    /**
     * Clears this object.
     */
    void clear();

    /**
     * Disposes this object.
     */
    void dispose();

    /**
     * Gets {@code MtcCallTracker}.
     */
    CallTracker getCallTracker();

    /**
     * gets an index that is not owned by other calls.
     */
    int getVacantCallIndex();

    /**
     * Attachs {@code Call} for originating call.
     *
     * @param call a call which wants to attach
     */
    void attachCall(Call call);

    /**
     * Attachs pending {@code Call} for incoming call.
     *
     * @param call a call which wants to attach
     */
    void attachPreIncomingCall(Call call);

    /**
     * Gets pending {@code Call}
     *
     * @param callId a ID which wants to match
     */
    Call getPendingCall(long callId);

    /**
     * Gets {@code MtcECallStateTracker}
     */
    MtcECallStateTracker getECallStateTracker();

    /**
     * Gets {@code Call}
     *
     * @param callId a ID which wants to match
     */
    Call getCall(long callId);
}
