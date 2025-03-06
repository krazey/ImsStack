/*
 * Copyright (C) 2025 The Android Open Source Project
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

package com.android.imsstack.enabler.ssc;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;

import android.content.Context;

import androidx.test.filters.SmallTest;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;

@RunWith(JUnit4.class)
public class SscPreferenceHelperTest {
    private static final int SLOT_0 = 0;

    private SscPreferenceHelper mSscPreferenceHelper;

    @Before
    public void setup() {
        Context context = new ContextFixture().getTestDouble();
        AppContext.init(context);
    }

    @After
    public void tearDown() {
        AppContext.deinit();
    }

    @Test
    @SmallTest
    public void queryClirWithInvalidSlotId_returnDefault() {
        mSscPreferenceHelper = new SscPreferenceHelper(MSimUtils.INVALID_SLOT_ID);

        int clirMode = mSscPreferenceHelper.queryClir();

        assertEquals(-1, clirMode);
    }

    @Test
    @SmallTest
    public void updateClirWithInvalidSlotId_returnFalse() {
        mSscPreferenceHelper = new SscPreferenceHelper(MSimUtils.INVALID_SLOT_ID);

        boolean result = mSscPreferenceHelper.updateClir(SscConstant.OIR_DEFAULT);

        assertFalse(result);
    }

    @Test
    @SmallTest
    public void updateAndQueryClir_oirDefault() {
        mSscPreferenceHelper = new SscPreferenceHelper(SLOT_0);

        boolean result = mSscPreferenceHelper.updateClir(SscConstant.OIR_DEFAULT);
        int clirMode = mSscPreferenceHelper.queryClir();

        assertTrue(result);
        assertEquals(SscConstant.OIR_DEFAULT, clirMode);
    }

    @Test
    @SmallTest
    public void updateAndQueryClir_oirSuppression() {
        mSscPreferenceHelper = new SscPreferenceHelper(SLOT_0);

        boolean result = mSscPreferenceHelper.updateClir(SscConstant.OIR_SUPPRESSION);
        int clirMode = mSscPreferenceHelper.queryClir();

        assertTrue(result);
        assertEquals(SscConstant.OIR_SUPPRESSION, clirMode);
    }
}
