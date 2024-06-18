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
package com.android.imsstack.its.tests.call;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.its.imsservice.mmtel.ImsMmTelFeatureWrapper;
import com.android.imsstack.its.imsservice.reg.ImsRegistrationWrapper;
import com.android.imsstack.its.tests.ImsStackTestBase;
import com.android.imsstack.its.util.SingleLatch;

public class CallTestBase extends ImsStackTestBase {
    protected final SingleLatch mEventLatch = new SingleLatch(CallTestBase.class.getSimpleName());

    protected ImsRegistrationWrapper mImsRegistration = null;
    protected ImsMmTelFeatureWrapper mMmTelFeature = null;

    protected PersistableBundle mConfig = null;

    protected void performRegistration() {
        startImsStack(SLOT0, mConfig);
        enableAllMmTelCapabilities();
        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS);
        mConnectivityManagerProxy.notifyNetworkAvailable(APN_IMS);

        mImsRegistration.waitForRegistered();

        // Just sleep to have a delay between registration and call.
        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS);
    }

    // TODO: Move into CallTestUtilities / CallTestConfigManager.
    protected void turnOffQosAndPrecondition() {
        if (mConfig == null) {
            mConfig = new PersistableBundle();
        }
        mConfig.putInt(
                CarrierConfig.Assets.KEY_POLICY_FOR_ALERT_NOT_USING_PRECONDITION_MECHANISM_INT, 0);
        mConfig.putBoolean(CarrierConfigManager.ImsVoice.KEY_VOICE_QOS_PRECONDITION_SUPPORTED_BOOL,
                false);
    }
}
