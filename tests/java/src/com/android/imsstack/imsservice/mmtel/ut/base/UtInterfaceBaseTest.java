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

package com.android.imsstack.imsservice.mmtel.util;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.verify;

import android.telephony.ims.ImsReasonInfo;
import android.util.Log;

import com.android.imsstack.enabler.ssc.SscConstant;
import com.android.imsstack.enabler.ssc.SscServiceClassUtil;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtServiceStateListener;
import com.android.imsstack.imsservice.mmtel.ut.base.UtInterfaceBase;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class UtInterfaceBaseTest {
    private UtInterfaceBase mUtInterfaceBase;
    @Mock
    public IUtServiceStateListener mockUtServiceStateListener;
    public static final String TAG = "UtInterfaceBaseTest";
    public int result = ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED * (-1);

    @Before
    public void setUp() throws Exception {
        mUtInterfaceBase = new UtInterfaceBase();
        mockUtServiceStateListener =  Mockito.mock(IUtServiceStateListener.class);
    }

    @Test
    public void test_isUtAvailable() {
        Log.d(TAG, " Unit Test");
        assertEquals(false , mUtInterfaceBase.isUtAvailable());
    }

    @Test
    public void test_onServiceStateChanged() {
        mockUtServiceStateListener.onServiceStateChanged();
        verify(mockUtServiceStateListener, Mockito.times(1)).onServiceStateChanged();
    }

    @Test
    public void test_queryCallBarring() {
        int queryCondition = SscConstant.CONDITION_BIC_WR;
        assertEquals(result, mUtInterfaceBase.queryCallBarring(queryCondition));
    }

    @Test
    public void test_queryCallBarringForServiceClass() {
        int queryCondition = SscConstant.CONDITION_BAOC;
        assertEquals(result, mUtInterfaceBase.queryCallBarringForServiceClass(queryCondition,
                SscServiceClassUtil.SERVICE_CLASS_VOICE));
    }

    @Test
    public void test_queryCallForward() {
        int queryCondition = SscConstant.CONDITION_CFU;
        assertEquals(result, mUtInterfaceBase.queryCallForward(queryCondition, " "));
    }

    @Test
    public void test_queryCallWaiting() {
        assertEquals(result, mUtInterfaceBase.queryCallWaiting());
    }

    @Test
    public void test_queryCLIR() {
        assertEquals(result, mUtInterfaceBase.queryCLIR());
    }

    @Test
    public void test_queryCLIP() {
        assertEquals(result, mUtInterfaceBase.queryCLIP());
    }

    @Test
    public void test_queryCOLR() {
        assertEquals(result, mUtInterfaceBase.queryCOLR());
    }

    @Test
    public void test_queryCOLP() {
        assertEquals(result, mUtInterfaceBase.queryCOLP());
    }

    @Test
    public void test_updateCallBarring() {
        int queryCondition = SscConstant.CONDITION_BAOC;
        assertEquals(result, mUtInterfaceBase.updateCallBarring(queryCondition,
                SscConstant.STATUS_ENABLE, null));
    }

    @Test
    public void test_updateCallBarringForServiceClass() {
        int cbType = SscConstant.CONDITION_BAIC;
        assertEquals(result, mUtInterfaceBase.updateCallBarringForServiceClass(cbType,
                SscConstant.STATUS_ENABLE, null, SscServiceClassUtil.SERVICE_CLASS_VOICE));
    }

    @Test
    public void test_updateCallBarringWithPassword() {
        assertEquals(result,
                mUtInterfaceBase.updateCallBarringWithPassword(SscConstant.CONDITION_BIC_WR,
                SscConstant.STATUS_ENABLE, null, SscServiceClassUtil.SERVICE_CLASS_VOICE, null));
    }

    @Test
    public void test_updateCallForward() {
        assertEquals(result, mUtInterfaceBase.updateCallForward(SscConstant.ACTION_REGISTRATION,
                SscConstant.CONDITION_CFNRC, "+1234567890", 0, 0));
    }

    @Test
    public void test_updateCallWaiting() {
        assertEquals(result, mUtInterfaceBase.updateCallWaiting(true,
                SscServiceClassUtil.SERVICE_CLASS_VOICE));
    }

    @Test
    public void test_updateCLIR() {
        assertEquals(result, mUtInterfaceBase.updateCLIR(SscConstant.OIR_INVOCATION));
    }

    @Test
    public void test_updateCLIP() {
        assertEquals(result, mUtInterfaceBase.updateCLIP(true));
    }

    @Test
    public void test_updateCOLR() {
        assertEquals(result, mUtInterfaceBase.updateCOLR(SscConstant.TIR_PROVISIONED));
    }

    @Test
    public void test_updateCOLP() {
        assertEquals(result, mUtInterfaceBase.updateCOLP(true));
    }

    @After
    public void tearDown() throws Exception {
        mUtInterfaceBase = null;
    }
}
