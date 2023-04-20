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

import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;

import android.content.Context;
import android.content.Intent;
import android.os.Looper;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;

import com.android.imsstack.enabler.aos.service.AosService;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class CarrierSignalReceiverTest {
    private static final int SLOT_0 = 0;
    private static final int PCO_TARGET_ID = 0xff00;
    private FakeCarrierSignalReceiver mFakeCarrierSignalReceiver;

    @Mock private Context mMockContext;
    @Mock private AosService mMockAosService;

    @Before
    public void setUp() throws Exception {
        if (Looper.myLooper() == null) {
            Looper.prepare();
        }

        MockitoAnnotations.initMocks(this);

        AosFactory.getInstance().mAosServices.put(SLOT_0, mMockAosService);

        mFakeCarrierSignalReceiver = new FakeCarrierSignalReceiver();
    }

    @After
    public void cleanUp() throws Exception {
        AosFactory.getInstance().mAosServices.remove(SLOT_0);
    }

    @Test
    public void testOnReceive_ValidIntent() {
        byte[] pcoData = {1, 2, 3, 5};

        Intent intent = new Intent();
        intent.setAction(TelephonyManager.ACTION_CARRIER_SIGNAL_PCO_VALUE);
        intent.putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, 0);
        intent.putExtra(TelephonyManager.EXTRA_APN_TYPE, ApnSetting.TYPE_IMS);
        intent.putExtra(TelephonyManager.EXTRA_PCO_ID, PCO_TARGET_ID);
        intent.putExtra(TelephonyManager.EXTRA_PCO_VALUE, pcoData);

        // Trigger broadcast
        mFakeCarrierSignalReceiver.onReceive(mMockContext, intent);

        verify(mMockAosService).notifyCarrierSignalPcoValueChanged(intent);
    }

    @Test
    public void testOnReceive_UnknownIntent() {
        byte[] pcoData = {1, 2, 3, 5};

        Intent intent = new Intent();
        intent.setAction("android.telephony.action.CARRIER_SIGNAL_PCO_VALUE_UNKNOWN");
        intent.putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, 0);
        intent.putExtra(TelephonyManager.EXTRA_APN_TYPE, ApnSetting.TYPE_IMS);
        intent.putExtra(TelephonyManager.EXTRA_PCO_ID, PCO_TARGET_ID);
        intent.putExtra(TelephonyManager.EXTRA_PCO_VALUE, pcoData);

        // Trigger broadcast
        mFakeCarrierSignalReceiver.onReceive(mMockContext, intent);

        verify(mMockAosService, times(0)).notifyCarrierSignalPcoValueChanged(intent);
    }

    private static class FakeCarrierSignalReceiver extends CarrierSignalReceiver {

        @Override
        protected int getSlotId(int subId) {
            return SLOT_0;
        }
    }
}

