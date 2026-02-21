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

import com.android.imsstack.util.ImsLog;

public abstract class ImsApp {
    private final int mPhoneId;

    public ImsApp(int phoneId) {
        mPhoneId = phoneId;
    }

    /**
     * Clear objects created within session.
     */
    public abstract void close();

    /**
     * Clear all sessions and initialize Ims state to default.
     */
    public abstract void onBinderDied();

    public final int getPhoneId() {
        return mPhoneId;
    }

    /**
     * Check if the specified service and call type is connected (registered).
     *
     * @param serviceType ImsCallProfile#SERVICE_TYPE_XXX
     * @param serviceType ImsCallProfile#CALL_TYPE_XXX
     */
    public boolean isConnected(int serviceType, int callType) {
        return false;
    }

    protected static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }
}
