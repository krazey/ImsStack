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

package com.android.imsstack.imsservice.mmtel;

import static org.junit.Assert.assertEquals;

import androidx.annotation.NonNull;

import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;

import java.util.concurrent.CountDownLatch;

class MockIAosRegistration implements IAosRegistration {
    private IAosRegistrationListener mAosRegListener;
    private CountDownLatch mLatch;

    public IAosRegistrationListener getListener() {
        return mAosRegListener;
    }

    @Override
    public void addListener(@NonNull IAosRegistrationListener listener) {
        mAosRegListener = listener;
    }

    @Override
    public void removeListener(@NonNull IAosRegistrationListener listener) {
        mAosRegListener = null;
    }

    @Override
    public void updateSipDelegateRegistration() {
        mLatch.countDown();
    }

    @Override
    public void triggerSipDelegateDeregistration() {
        mLatch.countDown();
    }

    @Override
    public void triggerFullNetworkRegistration(int sipCode, String sipReason) {
        mLatch.countDown();
    }

    @Override
    public int getRegisteredNetworkType() {
        return IAosRegistrationListener.NetworkType.LTE;
    }

    @Override
    public int getRegistrationState() {
        return IAosRegistrationListener.RegistrationState.REGISTERED;
    }

    @Override
    public void controlRegistration(int requestType, int pcscfOrder, int cause) {
        assertEquals(IAosRegistration.RequestType.STOP, requestType);
        assertEquals(IAosRegistration.Pcscf.CURRENT, pcscfOrder);
        assertEquals(IAosRegistration.Cause.RADIO_SIM_REMOVED.getValue(), cause);
    }

    @Override
    public void changeCapabilities(@NonNull CapabilityPairs pairs) {
    }

    public void setCountDownLatch(CountDownLatch cl) {
        mLatch = cl;
    }
}
