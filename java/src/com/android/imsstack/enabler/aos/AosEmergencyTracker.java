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
package com.android.imsstack.enabler.aos;

/**
 * This class provides information for emergency.
 */
public class AosEmergencyTracker {

    private int mSlotId = 0;

    AosEmergencyTracker(int slotId) {
        mSlotId = slotId;
    }

    /**
     * Updating callback mode information for emergency.
     *
     * @param type {@code type} is callback mode entry {@link EmcCallbackType}
     * @param state {@code state} is type of {@link EmcCallbackMode}.
     * @param duration is the number of milliseconds remaining in the emergency callback mode.
     */
    public void updateEmcCallbackMode(int type, int state, long duration) {
        IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(mSlotId);
        if (aosInfo != null) {
            aosInfo.notifyEmcCallbackModeChanged(type, state, duration);
        }
    }

}
