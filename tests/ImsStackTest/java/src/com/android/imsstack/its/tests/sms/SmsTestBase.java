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
package com.android.imsstack.its.tests.sms;

import static android.telephony.SmsMessage.FORMAT_3GPP;

import static com.android.imsstack.its.base.TestConstants.SLOT0;

import android.os.PersistableBundle;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.its.base.SystemProxyResolver;
import com.android.imsstack.its.base.TelephonyManagerProxyImpl;
import com.android.imsstack.its.imsservice.ImsServiceConnector;
import com.android.imsstack.its.imsservice.mmtel.sms.ImsSmsWrapper;
import com.android.imsstack.its.tests.ImsStackTestBase;
import com.android.imsstack.its.util.SingleLatch;

import java.util.Objects;

public class SmsTestBase extends ImsStackTestBase {
    private static final String TEST_FORMAT = FORMAT_3GPP;
    private static final String TEST_SMSC = "07913121139418F0";
    // destination:1234567890, text:Hello Google
    private static final String TEST_PDU_IN_HEX_STRING =
            "01000A81214365870900000CC8329BFD061DDFEF33BB0C";

    protected final @NonNull SingleLatch mEventLatch =
            new SingleLatch(SmsTestBase.class.getSimpleName());

    protected @NonNull ImsSmsWrapper mImsSms;
    protected @Nullable PersistableBundle mConfig;

    protected final void setUpSmsBase(int slotId) {
        TelephonyManagerProxyImpl telephony =
                SystemProxyResolver.getTelephonyManagerProxy(getSubId(SLOT0));
        telephony.setHalVersion(-2, -2);

        setUpBase(slotId);

        mImsSms = ImsServiceConnector.getInstance().getSms();
    }

    protected final void performRegistration() {
        startImsStack(SLOT0, mConfig);
        enableAllMmTelCapabilities();
        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS);

        mConnectivityManagerProxy.notifyNetworkAvailable(APN_IMS);
        mImsServiceConnector.getRegistration().waitForRegistered();
        mEventLatch.sleep(SingleLatch.SHORT_SLEEP_MS);
    }

    // TODO: Move into SmsTestConfigurationHelper(tentative).
    protected void enableDefaultConfiguration() {
        if (mConfig == null) {
            mConfig = new PersistableBundle();
        }
        mConfig.putInt(
                CarrierConfig.ImsSms.KEY_SMS_PREFERRED_PSI_URI_TYPE_INT, 0);
        mConfig.putBoolean(CarrierConfig.ImsSms.KEY_SMS_USE_DIALED_NUMBER_FOR_REQUEST_URI_BOOL,
                false);
    }

    protected void sendSms() {
        mImsSms.sendSms(
                1,
                0,
                TEST_FORMAT,
                TEST_SMSC,
                false,
                getPdu(TEST_PDU_IN_HEX_STRING));
    }

    protected void acknowledgeSms() {
        mImsSms.acknowledgeSms(mImsSms.getLastReceivedToken(), 0, 1, null);
    }

    protected void expectResultWithin(int timeInMillis) {
        mImsSms.waitForSmsResult(timeInMillis);
    }

    protected void expectStatusReportWithin(int timeInMillis) {
        mImsSms.waitForStatusReport(timeInMillis);
    }

    protected void expectIncomingSmsWithin(int timeInMillis) {
        mImsSms.waitForIncomingSms(timeInMillis);
    }

    protected void expectContent(@NonNull String content) {
        Objects.requireNonNull(content, "content must not be null.");
        // TODO: check the equality of getPdu(content) and mImsSms.getmLastReceivedPdu() and fail
        // if the check is failed.
    }

    private @NonNull byte[] getPdu(@NonNull String hexString) {
        Objects.requireNonNull(hexString, "hexString must not be null.");

        byte[] byteArray = new byte[hexString.length() / 2];
        for (int i = 0; i < byteArray.length; i++) {
            int index = i * 2;
            byteArray[i] = (byte) Integer.parseInt(hexString.substring(index, index + 2), 16);
        }
        return byteArray;
    }
}
