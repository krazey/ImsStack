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

package com.android.imsstack.enabler.acs;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.Mockito.doReturn;

import android.os.Build;
import android.telephony.TelephonyManager;
import android.util.Log;

import androidx.test.filters.SmallTest;

import com.android.imsstack.enabler.acs.impl.RequestInfo;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.net.URL;

public class RequestInfoTest {
    private static final String TAG = RequestInfoTest.class.getSimpleName();
    private static final String VERSION = "6.0";
    private static final String PROFILE = "UP_1.0";
    private static final String CLIENT_VENDOR = "Google";
    private static final String CLIENT_VERSION = "1.0";
    private static final boolean ENABLED_BY_USER = true;
    // TODO : need to test with 6 digits
    private static final String MCC = "310";
    private static final String MNC = "38";
    private static final String MCCMNC = MCC + MNC;
    private static final String IMSI = "3103801012345678";
    private static final String IMEI = "googlepixel123456789";

    private int mSlotId = 0;
    private int mSubId = 1234;

    private RequestInfo mRequestInfo;

    @Mock
    TelephonyManager mTelephonyManager;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        doReturn(MCCMNC).when(mTelephonyManager).getSimOperator();
        doReturn(IMSI).when(mTelephonyManager).getSubscriberId();
        doReturn(IMEI).when(mTelephonyManager).getImei(anyInt());

    }

    @After
    public void tearDown() throws Exception {
        mRequestInfo = null;
    }

    @Test
    @SmallTest
    public void userAgent_test() throws Exception {
        mRequestInfo = createRequestInfo();

        String expectedUserAgent = "IM-client/OMA1.0 " + Build.MANUFACTURER + "/"
                + Build.MODEL + "-" + Build.DISPLAY + " " + CLIENT_VENDOR + "/" + CLIENT_VERSION;

        String userAgent = mRequestInfo.generateUserAgent();
        assertEquals(expectedUserAgent, userAgent);

        String gbaToken = "1a2s3d4f5g";
        expectedUserAgent += ";" + gbaToken;
        mRequestInfo.setGBAProductToken(gbaToken);

        userAgent = mRequestInfo.generateUserAgent();
        assertEquals(expectedUserAgent, userAgent);
    }

    @Test
    @SmallTest
    public void getHttpUrl_test() throws Exception {
        mRequestInfo = createRequestInfo();

        String expectedHost = "config.rcs.mnc" + MNC + ".mcc" + MCC + ".pub.3gppnetwork.org";

        URL httpUrl = mRequestInfo.getHttpUrl();

        assertEquals("http", httpUrl.getProtocol());
        assertEquals(expectedHost, httpUrl.getHost());
    }

    @Test
    @SmallTest
    public void getHttpsUrl_test() throws Exception {
        mRequestInfo = createRequestInfo();

        String expectedHost = "config.rcs.mnc" + MNC + ".mcc" + MCC + ".pub.3gppnetwork.org";

        URL httpsUrl = mRequestInfo.getHttpsUrl();
        String query = httpsUrl.getQuery();

        assertEquals("https", httpsUrl.getProtocol());
        assertEquals(expectedHost, httpsUrl.getHost());

        assertTrue(query.contains("vers="));
        assertTrue(query.contains("IMSI="));
        assertTrue(query.contains("rcs_version="));
        assertTrue(query.contains("rcs_profile="));
        assertTrue(query.contains("client_vendor="));
        assertTrue(query.contains("client_version="));
        assertTrue(query.contains("terminal_vendor="));
        assertTrue(query.contains("terminal_model="));
        assertTrue(query.contains("terminal_sw_version="));
        assertTrue(query.contains("IMEI="));
        assertTrue(query.contains("default_sms_app="));
        assertTrue(query.contains("rcs_state="));
        assertTrue(query.contains("provisioning_version="));
        assertTrue(query.contains("SMS_port="));
    }

    @Test
    @SmallTest
    public void requestInfoBuilder_withValidValue() throws Exception {
        AcServiceClientInfo acServiceClientInfo = new AcServiceClientInfo(
                VERSION, PROFILE, CLIENT_VENDOR, CLIENT_VERSION, ENABLED_BY_USER);
        RequestInfo.RequestInfoBuilder requestInfoBuilder = new RequestInfo.RequestInfoBuilder(
                mSlotId, mSubId, acServiceClientInfo, mTelephonyManager);

        String acVersion = "100";
        String rcsProfile = "UP_1.0";
        String rcsVersion = "7.0";
        String smsPort = "9898";

        requestInfoBuilder.setAcVersion(acVersion)
                .setRcsProfile(rcsProfile)
                .setRcsVersion(rcsVersion)
                .setSmsPort(smsPort);
        mRequestInfo = requestInfoBuilder.build();
        String query = mRequestInfo.getHttpsUrl().getQuery();

        assertTrue(query.contains(acVersion));
        assertTrue(query.contains(rcsVersion));
        assertTrue(query.contains(rcsProfile));
        assertTrue(query.contains(smsPort));

        acVersion = "200";
        rcsProfile = "UP_2.0";
        rcsVersion = "6.0";

        requestInfoBuilder.setAcVersion(acVersion)
                .setRcsProfile(rcsProfile)
                .setRcsVersion(rcsVersion);
        mRequestInfo = requestInfoBuilder.build();
        query = mRequestInfo.getHttpsUrl().getQuery();

        assertTrue(query.contains(acVersion));
        assertTrue(query.contains(rcsVersion));
        assertTrue(query.contains(rcsProfile));

        Log.i(TAG, requestInfoBuilder.toString());
    }

    @Test
    @SmallTest
    public void requestInfoBuilder_withEmptyValue() throws Exception {
        AcServiceClientInfo acServiceClientInfo = new AcServiceClientInfo(
                VERSION, PROFILE, CLIENT_VENDOR, CLIENT_VERSION, ENABLED_BY_USER);
        RequestInfo.RequestInfoBuilder requestInfoBuilder = new RequestInfo.RequestInfoBuilder(
                mSlotId, mSubId, acServiceClientInfo, mTelephonyManager);

        String acVersion = "300";
        // default value
        String rcsProfile = "UP_1.0";
        // default value
        String rcsVersion = "6.0";

        requestInfoBuilder.setAcVersion(acVersion)
                .setRcsProfile("")
                .setRcsVersion("");
        mRequestInfo = requestInfoBuilder.build();
        String query = mRequestInfo.getHttpsUrl().getQuery();

        assertTrue(query.contains(acVersion));
        assertTrue(query.contains(rcsVersion));
        assertTrue(query.contains(rcsProfile));

        requestInfoBuilder.setTerminalName("")
                .setTerminalVersion("");
        try {
            mRequestInfo = requestInfoBuilder.build();
            throw new AssertionError("not expected");
        } catch (IllegalArgumentException e) {
            // expected result
        }
    }

    @Test
    @SmallTest
    public void requestInfo_withEmptyValue() throws Exception {
        AcServiceClientInfo acServiceClientInfo = new AcServiceClientInfo(
                VERSION, PROFILE, CLIENT_VENDOR, CLIENT_VERSION, ENABLED_BY_USER);
        RequestInfo.RequestInfoBuilder requestInfoBuilder = new RequestInfo.RequestInfoBuilder(
                mSlotId, mSubId, acServiceClientInfo, mTelephonyManager);

        mRequestInfo = requestInfoBuilder.build();

        String otp = "13579";
        String token = "abcdefghij";
        String defaultSmsApp = "google_sms";
        String defaultVvmApp = "google_vvm";
        String smsPort = "9898";
        String rcsState = "-4";
        String acVersion = "7171";

        mRequestInfo.setOtp(otp);
        mRequestInfo.setToken(token);
        mRequestInfo.setDefaultSmsApp(defaultSmsApp);
        mRequestInfo.setDefaultVvmApp(defaultVvmApp);
        mRequestInfo.updateSmsPort(smsPort);
        mRequestInfo.updateRcsState(rcsState);
        mRequestInfo.updateAcVersion(acVersion);

        String query = mRequestInfo.getHttpsUrl().getQuery();

        assertTrue(query.contains(otp));
        assertTrue(query.contains(token));
        assertTrue(query.contains(defaultSmsApp));
        assertTrue(query.contains(defaultVvmApp));
        assertTrue(query.contains(smsPort));
        assertTrue(query.contains(rcsState));
        assertTrue(query.contains(acVersion));
    }

    private RequestInfo createRequestInfo() {
        AcServiceClientInfo acServiceClientInfo = new AcServiceClientInfo(
                VERSION, PROFILE, CLIENT_VENDOR, CLIENT_VERSION, ENABLED_BY_USER);
        RequestInfo.RequestInfoBuilder requestInfoBuilder = new RequestInfo.RequestInfoBuilder(
                mSlotId, mSubId, acServiceClientInfo, mTelephonyManager);
        return requestInfoBuilder.build();
    }
}
