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

package com.android.imsstack.imsservice.mmtel;

import static org.junit.Assert.assertEquals;

import android.telephony.ims.ImsCallProfile;

import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class ImsSuppInfoUtilsTest {
    private static final String TAG = "[ImsSuppInfoUtilsTest]";
    private ImsSuppInfoUtils mImsSuppInfoUtils;

    @Mock ICallContext mContext;

    private int mInfo = 1;
    private SuppInfo mSuppInfo;
    private ImsCallProfile mProfile;
    int mCode = 1;

    @Before
    public void setUp() throws Exception {
        mImsSuppInfoUtils = new ImsSuppInfoUtils();
        mProfile = new ImsCallProfile();
        mSuppInfo = new SuppInfo();
        mContext = Mockito.mock(ICallContext.class);
    }

    @Test
    public void test_getCallExtraNameForBoolean() {
        assertEquals(null, ImsSuppInfoUtils.getCallExtraNameForBoolean(mContext, mInfo));
    }

    @Test
    public void test_getCallExtraNameForInt() {
        assertEquals(null, ImsSuppInfoUtils.getCallExtraNameForInt(mContext, mInfo));
    }

    @Test
    public void test_getCallExtraNameForString() {
        assertEquals(null, ImsSuppInfoUtils.getCallExtraNameForString(mContext, mInfo));
    }

    @After
    public void tearDown() throws Exception {
        mImsSuppInfoUtils = null;
        mProfile = null;
        mSuppInfo = null;
        mContext = null;
    }
}
