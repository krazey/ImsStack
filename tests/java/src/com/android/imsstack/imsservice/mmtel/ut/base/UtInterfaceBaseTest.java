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

import com.android.imsstack.imsservice.mmtel.ut.base.IUtServiceStateListener;
import com.android.imsstack.imsservice.mmtel.ut.base.UtInterfaceBase;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
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

    @After
    public void tearDown() throws Exception {
        mUtInterfaceBase = null;
    }
}
