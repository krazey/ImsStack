/*
 * Copyright (C) 2025 The Android Open Source Project
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
package com.android.imsstack.its.base;

import com.android.imsstack.base.SystemServiceProxy.TelecomManagerProxy;

/**
 * An implementation class to access the {@link TelecomManager}.
 */
public class TelecomManagerProxyImpl implements TelecomManagerProxy {
    private boolean mIsInEmergencyCall;

    @Override
    public boolean isInEmergencyCall() {
        return mIsInEmergencyCall;
    }

    /**
     * Sets the emergency call state.
     *
     * @param isInEmergencyCall {@code true} if there is an emergency call, {@code false} otherwise.
     */
    public void setInEmergencyCall(boolean isInEmergencyCall) {
        mIsInEmergencyCall = isInEmergencyCall;
    }
}
