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
package com.android.imsstack.base;

import static com.android.imsstack.base.TestAppContext.SLOT0;
import static com.android.imsstack.base.TestAppContext.SLOT1;
import static com.android.imsstack.base.TestAppContext.SUB_ID_1;
import static com.android.imsstack.base.TestAppContext.SUB_ID_2;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

import android.content.Context;

import androidx.test.filters.SmallTest;

import com.android.imsstack.base.SystemServiceProxy.SubscriptionManagerProxy;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

@RunWith(JUnit4.class)
public class MSimUtilsTest {
    private static final boolean[] HAS_ICC_CARD = { true, false };

    @Mock private Context mContext;

    private TelephonyManagerProxy mTelephonyManagerProxy;
    private SubscriptionManagerProxy mSubscriptionManagerProxy;
    private TestAppContext mTestAppContext;

    @Before
    public void setUp() throws Exception {
        MockitoAnnotations.initMocks(this);

        mTestAppContext = new TestAppContext(mContext);
        mTestAppContext.setUp();

        mTelephonyManagerProxy = mTestAppContext.getSystemServiceProxy(TelephonyManagerProxy.class);
        when(mTelephonyManagerProxy.hasIccCard()).thenReturn(HAS_ICC_CARD[0], HAS_ICC_CARD[1]);

        mSubscriptionManagerProxy =
                mTestAppContext.getSystemServiceProxy(SubscriptionManagerProxy.class);
    }

    @After
    public void tearDown() throws Exception {
        mContext = null;
        mSubscriptionManagerProxy = null;
        mTelephonyManagerProxy = null;
        mTestAppContext.tearDown();
        mTestAppContext = null;
    }

    @Test
    @SmallTest
    public void testGetMethods() {
        assertEquals(SLOT0, MSimUtils.getSlotId(SUB_ID_1));
        assertEquals(SLOT1, MSimUtils.getSlotId(SUB_ID_2));

        assertEquals(SUB_ID_1, MSimUtils.getSubId(SLOT0));
        assertEquals(SUB_ID_2, MSimUtils.getSubId(SLOT1));
    }

    @Test
    @SmallTest
    public void testCheckMethods() {
        assertTrue(MSimUtils.isMultiImsEnabled());

        MSimUtils.isUsableSubId(SUB_ID_1);
        verify(mSubscriptionManagerProxy).isUsableSubscriptionId(eq(SUB_ID_1));

        MSimUtils.isValidSubId(SUB_ID_1);
        verify(mSubscriptionManagerProxy).isValidSubscriptionId(eq(SUB_ID_1));

        MSimUtils.hasIccCard(SLOT0);
        verify(mTelephonyManagerProxy).hasIccCard();
    }

    @Test
    @SmallTest
    public void testGetPhoneId() {
        assertEquals(SLOT0, MSimUtils.getPhoneId(SUB_ID_1));
        assertEquals(SLOT1, MSimUtils.getPhoneId(SUB_ID_2));
        assertEquals(MSimUtils.DEFAULT_PHONE_ID, MSimUtils.getPhoneId(MSimUtils.INVALID_SUB_ID));
    }
}
