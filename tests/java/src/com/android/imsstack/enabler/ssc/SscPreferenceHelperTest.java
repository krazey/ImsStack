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
    public void queryCbWithInvalidSlotId_returnDefault() {
        mSscPreferenceHelper = new SscPreferenceHelper(MSimUtils.INVALID_SLOT_ID);

        int status = mSscPreferenceHelper.queryCb(SscConstant.CONDITION_BAIC,
                SscServiceClassUtil.SERVICE_CLASS_CALL);

        assertEquals(SscConstant.STATUS_NOT_REGISTERED, status);
    }

    @Test
    @SmallTest
    public void updateCbWithInvalidSlotId_returnFalse() {
        mSscPreferenceHelper = new SscPreferenceHelper(MSimUtils.INVALID_SLOT_ID);

        boolean result = mSscPreferenceHelper.updateCb(SscConstant.CONDITION_BAIC,
                SscServiceClassUtil.SERVICE_CLASS_CALL, SscConstant.STATUS_ENABLE);

        assertFalse(result);
    }

    @Test
    @SmallTest
    public void updateAndQueryCb_enable() {
        int condition = SscConstant.CONDITION_BAIC;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_CALL;
        int updateStatus = SscConstant.STATUS_ENABLE;
        mSscPreferenceHelper = new SscPreferenceHelper(SLOT_0);

        boolean result = mSscPreferenceHelper.updateCb(condition, serviceClass, updateStatus);
        int queryStatus = mSscPreferenceHelper.queryCb(condition, serviceClass);

        assertTrue(result);
        assertEquals(updateStatus, queryStatus);
    }

    @Test
    @SmallTest
    public void updateAndQueryCb_disable() {
        int condition = SscConstant.CONDITION_BOIC_EXHC;
        int serviceClass = SscServiceClassUtil.SERVICE_CLASS_VIDEO;
        int updateStatus = SscConstant.STATUS_DISABLE;
        mSscPreferenceHelper = new SscPreferenceHelper(SLOT_0);

        boolean result = mSscPreferenceHelper.updateCb(condition, serviceClass, updateStatus);
        int queryStatus = mSscPreferenceHelper.queryCb(condition, serviceClass);

        assertTrue(result);
        assertEquals(updateStatus, queryStatus);
    }

    @Test
    @SmallTest
    public void queryOirWithInvalidSlotId_returnDefault() {
        mSscPreferenceHelper = new SscPreferenceHelper(MSimUtils.INVALID_SLOT_ID);

        int oirMode = mSscPreferenceHelper.queryOir();

        assertEquals(SscConstant.STATUS_NOT_REGISTERED, oirMode);
    }

    @Test
    @SmallTest
    public void updateOirWithInvalidSlotId_returnFalse() {
        mSscPreferenceHelper = new SscPreferenceHelper(MSimUtils.INVALID_SLOT_ID);

        boolean result = mSscPreferenceHelper.updateOir(SscConstant.OIR_DEFAULT);

        assertFalse(result);
    }

    @Test
    @SmallTest
    public void updateAndQueryOir_default() {
        int updateMode = SscConstant.OIR_DEFAULT;
        mSscPreferenceHelper = new SscPreferenceHelper(SLOT_0);

        boolean result = mSscPreferenceHelper.updateOir(SscConstant.OIR_DEFAULT);
        int queryMode = mSscPreferenceHelper.queryOir();

        assertTrue(result);
        assertEquals(updateMode, queryMode);
    }

    @Test
    @SmallTest
    public void updateAndQueryOir_suppression() {
        int updateMode = SscConstant.OIR_SUPPRESSION;
        mSscPreferenceHelper = new SscPreferenceHelper(SLOT_0);

        boolean result = mSscPreferenceHelper.updateOir(updateMode);
        int queryMode = mSscPreferenceHelper.queryOir();

        assertTrue(result);
        assertEquals(updateMode, queryMode);
    }

    @Test
    @SmallTest
    public void queryOipWithInvalidSlotId_returnDefault() {
        mSscPreferenceHelper = new SscPreferenceHelper(MSimUtils.INVALID_SLOT_ID);

        int status = mSscPreferenceHelper.queryOip();

        assertEquals(SscConstant.STATUS_NOT_REGISTERED, status);
    }

    @Test
    @SmallTest
    public void updateOipWithInvalidSlotId_returnFalse() {
        mSscPreferenceHelper = new SscPreferenceHelper(MSimUtils.INVALID_SLOT_ID);

        boolean result = mSscPreferenceHelper.updateOip(SscConstant.STATUS_ENABLE);

        assertFalse(result);
    }

    @Test
    @SmallTest
    public void updateAndQueryOip_enable() {
        int updateStatus = SscConstant.STATUS_ENABLE;
        mSscPreferenceHelper = new SscPreferenceHelper(SLOT_0);

        boolean result = mSscPreferenceHelper.updateOip(updateStatus);
        int queryStatus = mSscPreferenceHelper.queryOip();

        assertTrue(result);
        assertEquals(updateStatus, queryStatus);
    }

    @Test
    @SmallTest
    public void updateAndQueryOip_disable() {
        int updateStatus = SscConstant.STATUS_DISABLE;
        mSscPreferenceHelper = new SscPreferenceHelper(SLOT_0);

        boolean result = mSscPreferenceHelper.updateOip(updateStatus);
        int queryStatus = mSscPreferenceHelper.queryOip();

        assertTrue(result);
        assertEquals(updateStatus, queryStatus);
    }

    @Test
    @SmallTest
    public void queryTirWithInvalidSlotId_returnDefault() {
        mSscPreferenceHelper = new SscPreferenceHelper(MSimUtils.INVALID_SLOT_ID);

        int mode = mSscPreferenceHelper.queryTir();

        assertEquals(SscConstant.STATUS_NOT_REGISTERED, mode);
    }

    @Test
    @SmallTest
    public void updateTirWithInvalidSlotId_returnFalse() {
        mSscPreferenceHelper = new SscPreferenceHelper(MSimUtils.INVALID_SLOT_ID);

        boolean result = mSscPreferenceHelper.updateTir(SscConstant.TIR_NOT_PROVISIONED);

        assertFalse(result);
    }

    @Test
    @SmallTest
    public void updateAndQueryTir_notProvisioned() {
        int updateMode = SscConstant.TIR_NOT_PROVISIONED;
        mSscPreferenceHelper = new SscPreferenceHelper(SLOT_0);

        boolean result = mSscPreferenceHelper.updateTir(updateMode);
        int queryMode = mSscPreferenceHelper.queryTir();

        assertTrue(result);
        assertEquals(updateMode, queryMode);
    }

    @Test
    @SmallTest
    public void updateAndQueryTir_provisioned() {
        int updateMode = SscConstant.TIR_PROVISIONED;
        mSscPreferenceHelper = new SscPreferenceHelper(SLOT_0);

        boolean result = mSscPreferenceHelper.updateTir(updateMode);
        int queryMode = mSscPreferenceHelper.queryTir();

        assertTrue(result);
        assertEquals(updateMode, queryMode);
    }

    @Test
    @SmallTest
    public void queryTipWithInvalidSlotId_returnDefault() {
        mSscPreferenceHelper = new SscPreferenceHelper(MSimUtils.INVALID_SLOT_ID);

        int status = mSscPreferenceHelper.queryTip();

        assertEquals(SscConstant.STATUS_NOT_REGISTERED, status);
    }

    @Test
    @SmallTest
    public void updateTipWithInvalidSlotId_returnFalse() {
        mSscPreferenceHelper = new SscPreferenceHelper(MSimUtils.INVALID_SLOT_ID);

        boolean result = mSscPreferenceHelper.updateTip(SscConstant.STATUS_ENABLE);

        assertFalse(result);
    }

    @Test
    @SmallTest
    public void updateAndQueryTip_enable() {
        int updateStatus = SscConstant.STATUS_ENABLE;
        mSscPreferenceHelper = new SscPreferenceHelper(SLOT_0);

        boolean result = mSscPreferenceHelper.updateTip(updateStatus);
        int queryStatus = mSscPreferenceHelper.queryTip();

        assertTrue(result);
        assertEquals(updateStatus, queryStatus);
    }

    @Test
    @SmallTest
    public void updateAndQueryTip_disable() {
        int updateStatus = SscConstant.STATUS_DISABLE;
        mSscPreferenceHelper = new SscPreferenceHelper(SLOT_0);

        boolean result = mSscPreferenceHelper.updateTip(updateStatus);
        int queryStatus = mSscPreferenceHelper.queryTip();

        assertTrue(result);
        assertEquals(updateStatus, queryStatus);
    }
}
