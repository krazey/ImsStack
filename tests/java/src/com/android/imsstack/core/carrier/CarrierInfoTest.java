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
package com.android.imsstack.core.carrier;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doReturn;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.times;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.SharedPreferences;
import android.telephony.TelephonyManager;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.SubscriptionManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.base.TestAppContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.List;

@RunWith(JUnit4.class)
public class CarrierInfoTest {
    private static final int MAX_SIM_SLOT = 2;
    private static final int SIM_CARRIER_ID = 100;
    private static final int SIM_SPECIFIC_CARRIER_ID = 101;
    private static final String SIM_MCC = "001";
    private static final String SIM_MNC = "01";
    private static final String SIM_OPERATOR = SIM_MCC + SIM_MNC;
    private static final String SIM_IMSI = "2040439303";
    private static final String SIM_GID1 = "A00020";
    private static final String SIM_OPERATOR_NAME = "Test-Carrier";
    private static final String SIM_ICCID = "89011201";

    private static final String TEST_OPERATOR = "TEST";
    private static final String TEST_OPERATOR_SUB = "TEST-SUB";
    private static final String TEST_COUNTRY = "COM";

    @Mock private SharedPreferences mSp;

    private TestAppContext mTestAppContext;
    private TelephonyManagerProxy mTelephonyManagerProxy;
    private CarrierInfo mCi;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestAppContext = new TestAppContext();
        mTestAppContext.setUp();
        DeviceConfig.setSimCount(MAX_SIM_SLOT, MAX_SIM_SLOT);

        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        when(mTelephonyManagerProxy.getSimSerialNumber()).thenReturn(SIM_ICCID);
        when(mTelephonyManagerProxy.getSimCarrierId()).thenReturn(SIM_CARRIER_ID);
        when(mTelephonyManagerProxy.getSimSpecificCarrierId()).thenReturn(SIM_SPECIFIC_CARRIER_ID);
        when(mTelephonyManagerProxy.getSimOperator()).thenReturn(SIM_OPERATOR);
        when(mTelephonyManagerProxy.getSubscriberId()).thenReturn(SIM_IMSI);
        when(mTelephonyManagerProxy.getGroupIdLevel1()).thenReturn(SIM_GID1);
        when(mTelephonyManagerProxy.getSimOperatorName()).thenReturn(SIM_OPERATOR_NAME);
        doReturn(mSp).when(mTestAppContext.getContext())
                .getSharedPreferences(anyString(), anyInt());

        mCi = new CarrierInfo();
    }

    @After
    public void tearDown() throws Exception {
        mCi = null;
        mSp = null;
        mTelephonyManagerProxy = null;
        DeviceConfig.setSimCount(1, 1);
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void testGetCarrierId() {
        for (int i = 0; i < MAX_SIM_SLOT; ++i) {
            assertNotNull(mCi.getCarrierId(i));
        }

        assertNull(mCi.getCarrierId(MAX_SIM_SLOT));
    }

    @Test
    @SmallTest
    public void testUpdateCarrierIdWhenSimAbsent() {
        initCarrierIdsFor(TelephonyManager.SIM_STATE_ABSENT);

        for (int i = 0; i < MAX_SIM_SLOT; ++i) {
            assertFalse(mCi.updateCarrierId(i));
        }

        for (int i = 0; i < MAX_SIM_SLOT; ++i) {
            SimCarrierId cid = mCi.getCarrierId(i);

            assertTrue(cid.isSimAbsent());

            assertEquals(SimCarrierId.UNKNOWN_ID, cid.getCarrierId());
            assertEquals(SimCarrierId.UNKNOWN_ID, cid.getSpecificCarrierId());

            assertEquals("", cid.getMcc());
            assertEquals("", cid.getMnc());
            assertEquals("", cid.getImsi());
            assertEquals("", cid.getGid1());
            assertEquals("", cid.getSpn());
            assertEquals("", cid.getIccId());
        }
    }

    @Test
    @SmallTest
    public void testUpdateCarrierIdWhenSimLocked() {
        initCarrierIdsFor(TelephonyManager.SIM_STATE_PIN_REQUIRED);

        for (int i = 0; i < MAX_SIM_SLOT; ++i) {
            assertFalse(mCi.updateCarrierId(i));
        }

        for (int i = 0; i < MAX_SIM_SLOT; ++i) {
            SimCarrierId cid = mCi.getCarrierId(i);

            assertTrue(cid.isSimLocked());

            assertEquals(SimCarrierId.UNKNOWN_ID, cid.getCarrierId());
            assertEquals(SimCarrierId.UNKNOWN_ID, cid.getSpecificCarrierId());

            assertEquals("", cid.getMcc());
            assertEquals("", cid.getMnc());
            assertEquals("", cid.getImsi());
            assertEquals("", cid.getGid1());
            assertEquals("", cid.getSpn());
            assertEquals(SIM_ICCID, cid.getIccId());
        }
    }

    @Test
    @SmallTest
    public void testUpdateCarrierIdWhenSimLoaded() {
        when(mSp.getString(eq(ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID), anyString()))
                .thenReturn("0");
        when(mSp.getString(eq(ImsPrivateProperties.Persistent.KEY_TEST_SPECIFIC_CARRIER_ID),
                anyString()))
                .thenReturn("0");

        initCarrierIdsFor(TelephonyManager.SIM_STATE_LOADED);

        for (int i = 0; i < MAX_SIM_SLOT; ++i) {
            assertTrue(mCi.updateCarrierId(i));
        }

        for (int i = 0; i < MAX_SIM_SLOT; ++i) {
            SimCarrierId cid = mCi.getCarrierId(i);

            assertTrue(cid.isSimLoaded());

            assertEquals(SIM_CARRIER_ID, cid.getCarrierId());
            assertEquals(SIM_SPECIFIC_CARRIER_ID, cid.getSpecificCarrierId());

            assertEquals(SIM_MCC, cid.getMcc());
            assertEquals(SIM_MNC, cid.getMnc());
            assertEquals(SIM_IMSI, cid.getImsi());
            assertEquals(SIM_GID1, cid.getGid1());
            assertEquals(SIM_OPERATOR_NAME, cid.getSpn());
            assertEquals(SIM_ICCID, cid.getIccId());
        }
    }

    @Test
    @SmallTest
    public void testUpdateCarrierIdWhenSimLoadedUsingTestCarrierIds() {
        final int testCarrierId = SIM_CARRIER_ID + 1;
        final int testSpecificCarrierId = SIM_SPECIFIC_CARRIER_ID + 1;
        when(mSp.getString(eq(ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID), anyString()))
                .thenReturn(String.valueOf(testCarrierId));
        when(mSp.getString(eq(ImsPrivateProperties.Persistent.KEY_TEST_SPECIFIC_CARRIER_ID),
                anyString()))
                .thenReturn(String.valueOf(testSpecificCarrierId));

        initCarrierIdsFor(TelephonyManager.SIM_STATE_LOADED);

        for (int i = 0; i < MAX_SIM_SLOT; ++i) {
            assertTrue(mCi.updateCarrierId(i));
        }

        for (int i = 0; i < MAX_SIM_SLOT; ++i) {
            SimCarrierId cid = mCi.getCarrierId(i);

            assertTrue(cid.isSimLoaded());

            assertEquals(testCarrierId, cid.getCarrierId());
            assertEquals(testSpecificCarrierId, cid.getSpecificCarrierId());

            assertEquals(SIM_MCC, cid.getMcc());
            assertEquals(SIM_MNC, cid.getMnc());
            assertEquals(SIM_IMSI, cid.getImsi());
            assertEquals(SIM_GID1, cid.getGid1());
            assertEquals(SIM_OPERATOR_NAME, cid.getSpn());
            assertEquals(SIM_ICCID, cid.getIccId());
        }
    }

    @Test
    @SmallTest
    public void testUpdateCarrierIdWithInvalidSlotId() {
        assertFalse(mCi.updateCarrierId(MAX_SIM_SLOT));
    }

    @Test
    @SmallTest
    public void testGetCarrierIdFromSimWhenTelephonyManagerNull() {
        SubscriptionManagerProxy smp =
                mTestAppContext.getSystemServiceProxy(SubscriptionManagerProxy.class);
        when(smp.getSubscriptionId(anyInt())).thenReturn(MSimUtils.INVALID_SUB_ID);

        SimCarrierId simCarrierId = mCi.getCarrierIdFromSim(TestAppContext.SLOT0);

        assertEquals(simCarrierId, new SimCarrierId.Builder().build());
    }

    @Test
    @SmallTest
    public void testSetSimOperatorCountryWhenNullString() {
        SharedPreferences.Editor editor = mock(SharedPreferences.Editor.class);
        ArgumentCaptor<String> valueCaptor = ArgumentCaptor.forClass(String.class);

        when(mSp.edit()).thenReturn(editor);

        CarrierInfo.setSimOperatorCountry(null, null, null, TestAppContext.SLOT0);
        verify(editor, times(3)).putString(anyString(), valueCaptor.capture());

        List<String> values = valueCaptor.getAllValues();

        assertEquals("", values.get(0));
        assertEquals("", values.get(1));
        assertEquals("", values.get(2));
    }

    @Test
    @SmallTest
    public void testSetSimOperatorCountryWhenEmptyString() {
        SharedPreferences.Editor editor = mock(SharedPreferences.Editor.class);
        ArgumentCaptor<String> valueCaptor = ArgumentCaptor.forClass(String.class);

        when(mSp.edit()).thenReturn(editor);

        CarrierInfo.setSimOperatorCountry("", "", "", TestAppContext.SLOT0);
        verify(editor, times(3)).putString(anyString(), valueCaptor.capture());

        List<String> values = valueCaptor.getAllValues();

        assertEquals("", values.get(0));
        assertEquals("", values.get(1));
        assertEquals("", values.get(2));
    }

    @Test
    @SmallTest
    public void testSetSimOperatorCountryWhenNonEmptyOrNullString() {
        SharedPreferences.Editor editor = mock(SharedPreferences.Editor.class);
        ArgumentCaptor<String> valueCaptor = ArgumentCaptor.forClass(String.class);

        when(mSp.edit()).thenReturn(editor);

        CarrierInfo.setSimOperatorCountry(
                TEST_OPERATOR, TEST_OPERATOR_SUB, TEST_COUNTRY, TestAppContext.SLOT0);
        verify(editor, times(3)).putString(anyString(), valueCaptor.capture());

        List<String> values = valueCaptor.getAllValues();

        assertEquals(TEST_OPERATOR, values.get(0));
        assertEquals(TEST_OPERATOR_SUB, values.get(1));
        assertEquals(TEST_COUNTRY, values.get(2));
    }

    private void initCarrierIdsFor(int testSimState) {
        if (MAX_SIM_SLOT == 1) {
            when(mTelephonyManagerProxy.getSimApplicationState())
                    .thenReturn(TelephonyManager.SIM_STATE_ABSENT, testSimState);
        } else {
            when(mTelephonyManagerProxy.getSimApplicationState())
                    .thenReturn(TelephonyManager.SIM_STATE_ABSENT,
                            TelephonyManager.SIM_STATE_ABSENT, testSimState);
        }

        for (int i = 0; i < MAX_SIM_SLOT; ++i) {
            mCi.updateCarrierId(i);
        }
    }
}
