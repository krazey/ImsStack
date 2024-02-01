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

package com.android.imsstack.enabler.mtc.reg;

public class MtcServiceState {
    // IUMtcService.SERVICE_XXX
    public int mServiceType = 0;
    // Additional information for each service types
    // As of now, it's only for SERVICE_EMERGENCY
    public int mExtraState = (-1);
    // Reason code if the service is not available
    public int mReason = 0;

    public MtcServiceState(int serviceType) {
        mServiceType = serviceType;
    }

    public MtcServiceState(int serviceType, int extraState, int reason) {
        this(serviceType);
        mExtraState = extraState;
        mReason = reason;
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("[ MtcServiceState: serviceType=");
        sb.append(mServiceType);
        sb.append(", extraState=");
        sb.append(mExtraState);
        sb.append(", reason=");
        sb.append(mReason);
        sb.append(" ]");

        return sb.toString();
    }
}
