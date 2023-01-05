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
package com.android.imsstack.util;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;

import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.test.suitebuilder.annotation.SmallTest;

import com.android.imsstack.ContextFixture;

import org.junit.After;
import org.junit.AfterClass;
import org.junit.Before;
import org.junit.BeforeClass;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class MSimUtilsTest {
    private static final int[] MAX_SUPPORTED_SIM_COUNT = { 1, 2 };
    private static final int[] MAX_ACTIVE_SIM_COUNT = { 1, 2 };
    private static final boolean[] HAS_ICC_CARD = { true, false };
    private static final int SLOT0 = 0;
    private static final int SLOT1 = 1;
    private static final int[] SUB_ID_SLOT0 = { 1 };
    private static final int[] SUB_ID_SLOT1 = { 2 };

    static ContextFixture sContext;

    private TelephonyManager mTelephonyManager;

    @BeforeClass
    public static void setUpOnce() {
        sContext = new ContextFixture();
        AppContext.init(sContext.getTestDouble());
    }

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        SubscriptionManager sm =
                sContext.getTestDouble().getSystemService(SubscriptionManager.class);
        when(sm.getSubscriptionIds(eq(SLOT0))).thenReturn(SUB_ID_SLOT0);
        when(sm.getSubscriptionIds(eq(SLOT1))).thenReturn(SUB_ID_SLOT1);

        mTelephonyManager = sContext.getTestDouble().getSystemService(TelephonyManager.class);
        when(mTelephonyManager.createForSubscriptionId(anyInt())).thenReturn(mTelephonyManager);
        when(mTelephonyManager.getActiveModemCount())
                .thenReturn(MAX_ACTIVE_SIM_COUNT[0], MAX_ACTIVE_SIM_COUNT[1]);
        when(mTelephonyManager.getSupportedModemCount())
                .thenReturn(MAX_SUPPORTED_SIM_COUNT[0], MAX_SUPPORTED_SIM_COUNT[1]);
        when(mTelephonyManager.hasIccCard()).thenReturn(HAS_ICC_CARD[0], HAS_ICC_CARD[1]);
    }

    @After
    public void tearDown() throws Exception {
    }

    @AfterClass
    public static void tearDownOnce() {
        AppContext.deinit();
        sContext = null;
    }

    @Test
    @SmallTest
    public void getActiveSimCount() throws Exception {
        assertEquals(MAX_ACTIVE_SIM_COUNT[0], MSimUtils.getActiveSimCount());
        assertEquals(MAX_ACTIVE_SIM_COUNT[1], MSimUtils.getActiveSimCount());
    }

    @Test
    @SmallTest
    public void getSupportedSimCount() throws Exception {
        assertEquals(MAX_SUPPORTED_SIM_COUNT[0], MSimUtils.getSupportedSimCount());
        assertEquals(MAX_SUPPORTED_SIM_COUNT[1], MSimUtils.getSupportedSimCount());
    }

    @Test
    @SmallTest
    public void getSubId() throws Exception {
        assertEquals(SUB_ID_SLOT0[0], MSimUtils.getSubId(SLOT0));
        assertEquals(SUB_ID_SLOT1[0], MSimUtils.getSubId(SLOT1));
    }

    @Test
    @SmallTest
    public void hasIccCard() throws Exception {
        assertTrue(MSimUtils.hasIccCard(SLOT0));
        assertFalse(MSimUtils.hasIccCard(SLOT0));
    }

    @Test
    @SmallTest
    public void isMultiSimEnabled() throws Exception {
        assertFalse(MSimUtils.isMultiSimEnabled());
        assertTrue(MSimUtils.isMultiSimEnabled());
    }

    @Test
    @SmallTest
    public void isValidSubId() throws Exception {
        assertTrue(MSimUtils.isValidSubId(SUB_ID_SLOT0[0]));
        assertTrue(MSimUtils.isValidSubId(SUB_ID_SLOT1[0]));
        assertFalse(MSimUtils.isValidSubId(MSimUtils.INVALID_SUB_ID));
    }
}
