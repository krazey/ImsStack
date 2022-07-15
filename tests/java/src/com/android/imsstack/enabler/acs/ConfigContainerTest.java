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
import static org.junit.Assert.assertNotEquals;
import static org.mockito.Mockito.doReturn;

import android.content.Context;
import android.test.suitebuilder.annotation.SmallTest;
import android.util.Log;

import androidx.test.InstrumentationRegistry;

import com.android.imsstack.enabler.acs.impl.ConfigContainer;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

public class ConfigContainerTest {
    private static final String TAG = ConfigContainerTest.class.getSimpleName();
    private static final String VERSION = "6.0";
    private static final String PROFILE = "UP_1.0";
    private static final String CLIENT_VENDOR = "Google";
    private static final String CLIENT_VERSION = "1.0";
    private static final boolean ENABLED_BY_USER = true;

    @Mock Context mContext;

    private TestConfigContainer mConfigContainer;
    private int mSlotId0 = 0;
    private int mSlotId1 = 1;
    private int mSubId0 = 1234;
    private int mSubId1 = 5678;

    private class TestConfigContainer extends ConfigContainer {
        private long mCurrentTime;

        TestConfigContainer(Context context, int slotId, int subId) {
            super(context, slotId, subId);
            mCurrentTime = 0L;
        }
        protected long getCurrentTimeMillis() {
            return mCurrentTime;
        }

        public void setCurrentTime(long time) {
            mCurrentTime = time;
        }
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        doReturn(InstrumentationRegistry.getContext().getFilesDir()).when(mContext).getFilesDir();
    }

    @After
    public void tearDown() throws Exception {
        mConfigContainer = null;
    }


    @Test
    @SmallTest
    public void getFileName_withSubId() throws Exception {
        // for SubId 0
        mConfigContainer = new TestConfigContainer(mContext, mSlotId0, mSubId0);

        String expectedFileName = "rcs_provisioning_config_" + mSubId0 + ".xml";
        String fileName = mConfigContainer.getFileName();
        assertEquals(expectedFileName, fileName);

        // for SubId 1
        mConfigContainer = new TestConfigContainer(mContext, mSlotId1, mSubId1);
        expectedFileName = "rcs_provisioning_config_" + mSubId1 + ".xml";
        fileName = mConfigContainer.getFileName();
        assertEquals(expectedFileName, fileName);
    }

    @Test
    @SmallTest
    public void acVersionValidityToken_ReadWrite() throws Exception {
        // for SubId 0
        mConfigContainer = new TestConfigContainer(mContext, mSlotId0, mSubId0);
        int expectedAcVersion = 0;
        long expectedAcValidity = 0;
        String expectedAcToken = null;

        int acVersion = mConfigContainer.getAcVersion(0);
        long acValidity = mConfigContainer.getAcValidity();
        String acToken = mConfigContainer.getAcToken();

        assertEquals(expectedAcVersion, acVersion);
        assertEquals(expectedAcValidity, acValidity);
        assertEquals(expectedAcToken, acToken);

        expectedAcVersion = 2;
        expectedAcValidity = 36000L;
        expectedAcToken = "qwert12345";

        mConfigContainer.updateAcVersionValidity(expectedAcVersion, expectedAcValidity);
        mConfigContainer.updateAcToken(expectedAcToken);

        // verify data from cached
        acVersion = mConfigContainer.getAcVersion(0);
        acValidity = mConfigContainer.getAcValidity();
        acToken = mConfigContainer.getAcToken();

        assertEquals(expectedAcVersion, acVersion);
        assertEquals(expectedAcValidity, acValidity);
        assertEquals(expectedAcToken, acToken);

        // verify data from stored file
        mConfigContainer = new TestConfigContainer(mContext, mSlotId0, mSubId0);
        acVersion = mConfigContainer.getAcVersion(0);
        acValidity = mConfigContainer.getAcValidity();
        acToken = mConfigContainer.getAcToken();

        assertEquals(expectedAcVersion, acVersion);
        assertEquals(expectedAcValidity, acValidity);
        assertEquals(expectedAcToken, acToken);
    }

    @Test
    @SmallTest
    public void acVersionValidityToken_afterReset() throws Exception {
        // for SubId 0
        mConfigContainer = new TestConfigContainer(mContext, mSlotId0, mSubId0);
        int expectedAcVersion = 2;
        long expectedAcValidity = 36000L;
        String expectedAcToken = "zxcvb67890";

        mConfigContainer.updateAcVersionValidity(expectedAcVersion, expectedAcValidity);
        mConfigContainer.updateAcToken(expectedAcToken);

        // verify data from cached
        int acVersion = mConfigContainer.getAcVersion(0);
        long acValidity = mConfigContainer.getAcValidity();
        String acToken = mConfigContainer.getAcToken();

        assertEquals(expectedAcVersion, acVersion);
        assertEquals(expectedAcValidity, acValidity);
        assertEquals(expectedAcToken, acToken);

        // reset
        mConfigContainer.resetAcValue();
        expectedAcVersion = 0;
        expectedAcValidity = 0L;
        expectedAcToken = null;

        // verify data from cached
        acVersion = mConfigContainer.getAcVersion(0);
        acValidity = mConfigContainer.getAcValidity();
        acToken = mConfigContainer.getAcToken();

        assertEquals(expectedAcVersion, acVersion);
        assertEquals(expectedAcValidity, acValidity);
        assertEquals(expectedAcToken, acToken);

        // verify data from stored file
        mConfigContainer = new TestConfigContainer(mContext, mSlotId0, mSubId0);
        acVersion = mConfigContainer.getAcVersion(0);
        acValidity = mConfigContainer.getAcValidity();
        acToken = mConfigContainer.getAcToken();

        assertEquals(expectedAcVersion, acVersion);
        assertEquals(expectedAcValidity, acValidity);
        assertEquals(expectedAcToken, acToken);
    }

    @Test
    @SmallTest
    public void clientInfo_ReadWrite() throws Exception {
        // for SubId 0
        mConfigContainer = new TestConfigContainer(mContext, mSlotId0, mSubId0);

        AcServiceClientInfo clientInfo0 = mConfigContainer.getClientInfo();
        checkEquals(null, clientInfo0);

        AcServiceClientInfo expected = new AcServiceClientInfo(
                VERSION, PROFILE, CLIENT_VENDOR, CLIENT_VERSION, ENABLED_BY_USER);
        mConfigContainer.updateClientInfo(expected);
        clientInfo0 = mConfigContainer.getClientInfo();
        checkEquals(expected, clientInfo0);

        // for SubId 1
        mConfigContainer = new TestConfigContainer(mContext, mSlotId1, mSubId1);

        AcServiceClientInfo clientInfo1 = mConfigContainer.getClientInfo();
        checkEquals(null, clientInfo1);

        expected = new AcServiceClientInfo(VERSION + "_1", PROFILE + "_1",
                CLIENT_VENDOR + "_1", CLIENT_VERSION + "_1", !ENABLED_BY_USER);
        mConfigContainer.updateClientInfo(expected);

        clientInfo1 = mConfigContainer.getClientInfo();
        checkEquals(expected, clientInfo1);

        // check different client info for SubId 0 and SubId 1
        checkNotEquals(clientInfo0, clientInfo1);

        mConfigContainer.resetClientInfo();
        clientInfo1 = mConfigContainer.getClientInfo();
        checkEquals(null, clientInfo1);
    }

    @Test
    @SmallTest
    public void validationTime_ReadWrite() throws Exception {
        // for SubId 0
        mConfigContainer = new TestConfigContainer(mContext, mSlotId0, mSubId0);

        long expected = 0L;
        long validationTime0 = mConfigContainer.getValidationTime(expected);

        assertEquals(expected, validationTime0);

        // sec for a week
        expected = 604800L;
        mConfigContainer.updateValidationTime(expected);

        validationTime0 = mConfigContainer.getValidationTime(0L);
        assertEquals(expected, validationTime0);

        // for SubId 1
        mConfigContainer = new TestConfigContainer(mContext, mSlotId1, mSubId1);

        expected = 0L;
        long validationTime1 = mConfigContainer.getValidationTime(expected);

        assertEquals(expected, validationTime1);

        // sec for 3 days
        expected = 10800L;
        mConfigContainer.updateValidationTime(expected);

        validationTime1 = mConfigContainer.getValidationTime(0L);
        assertEquals(expected, validationTime1);

        // check different validation time for SubId 0 and SubId 1
        assertNotEquals(validationTime0, validationTime1);
    }

    @Test
    @SmallTest
    public void lastUpdatedTime_ReadWrite() throws Exception {
        // for SubId 0
        mConfigContainer = new TestConfigContainer(mContext, mSlotId0, mSubId0);

        long expected = 0L;
        long lastUpdatedTime0 = mConfigContainer.getLastUpdatedTime(expected);

        assertEquals(expected, lastUpdatedTime0);

        expected = System.currentTimeMillis();
        mConfigContainer.setCurrentTime(expected);
        mConfigContainer.updateLastUpdatedTime();

        lastUpdatedTime0 = mConfigContainer.getLastUpdatedTime(0L);
        assertEquals(expected, lastUpdatedTime0);

        // for SubId 1
        mConfigContainer = new TestConfigContainer(mContext, mSlotId1, mSubId1);

        expected = 0L;
        long lastUpdatedTime1 = mConfigContainer.getLastUpdatedTime(expected);

        assertEquals(expected, lastUpdatedTime1);

        expected = System.currentTimeMillis();
        mConfigContainer.setCurrentTime(expected);
        mConfigContainer.updateLastUpdatedTime();

        lastUpdatedTime1 = mConfigContainer.getLastUpdatedTime(0L);
        assertEquals(expected, lastUpdatedTime1);

        // check different validation time for SubId 0 and SubId 1
        assertNotEquals(lastUpdatedTime0, lastUpdatedTime1);
    }

    private void checkEquals(AcServiceClientInfo expected, AcServiceClientInfo result)
            throws Exception {
        if (expected == result) {
            return;
        } else if (expected == null || result == null) {
            throw new AssertionError("object is null", new Exception());
        }

        Log.i(TAG, "expected : " + expected.toString());
        Log.i(TAG, "result : " + result.toString());

        assertEquals(expected.getClientVendor(), result.getClientVendor());
        assertEquals(expected.getClientVersion(), result.getClientVersion());
        assertEquals(expected.getRcsProfile(), result.getRcsProfile());
        assertEquals(expected.getRcsVersion(), result.getRcsVersion());
        assertEquals(expected.isRcsEnabledByUser(), result.isRcsEnabledByUser());
    }

    private void checkNotEquals(AcServiceClientInfo expected, AcServiceClientInfo result)
            throws Exception {
        if (expected == null || result == null) {
            throw new AssertionError("object is null", new Exception());
        }

        Log.i(TAG, "expected : " + expected.toString());
        Log.i(TAG, "result : " + result.toString());

        assertNotEquals(expected.getClientVendor(), result.getClientVendor());
        assertNotEquals(expected.getClientVersion(), result.getClientVersion());
        assertNotEquals(expected.getRcsProfile(), result.getRcsProfile());
        assertNotEquals(expected.getRcsVersion(), result.getRcsVersion());
        assertNotEquals(expected.isRcsEnabledByUser(), result.isRcsEnabledByUser());
    }
}
