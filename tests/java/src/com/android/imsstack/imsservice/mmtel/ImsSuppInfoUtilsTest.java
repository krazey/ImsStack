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
import static org.junit.Assert.assertNotNull;
import static org.mockito.Mockito.when;

import android.telephony.ims.ImsCallProfile;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

@RunWith(JUnit4.class)
public class ImsSuppInfoUtilsTest {
    private static final String TAG = "[ImsSuppInfoUtilsTest]";
    private static final int SLOT_ID = 0;

    private int mInfo = 1;
    private SuppInfo mSuppInfo;
    private ImsCallProfile mCallProfile;
    private ICallContext mMockContext;
    private CarrierConfig mMockCarrierConfig;
    private ConfigInterface mMockConfigInterface;

    @Before
    public void setUp() throws Exception {
        mCallProfile = new ImsCallProfile();
        mSuppInfo = new SuppInfo();
        mMockContext = Mockito.mock(ICallContext.class);
        mMockCarrierConfig = Mockito.mock(CarrierConfig.class);
        mMockConfigInterface = Mockito.mock(ConfigInterface.class);
        when(mMockConfigInterface.getCarrierConfig()).thenReturn(mMockCarrierConfig);
        AgentFactory.getInstance().setAgent(ConfigInterface.class, mMockConfigInterface, SLOT_ID);
    }

    @After
    public void tearDown() throws Exception {
        AgentFactory.getInstance().setAgent(ConfigInterface.class, null, SLOT_ID);
        mCallProfile = null;
        mSuppInfo = null;
    }

    @Test
    public void test_getCallExtraNameForBoolean() {
        assertEquals(null, ImsSuppInfoUtils.getCallExtraNameForBoolean(mMockContext, mInfo));
    }

    @Test
    public void test_getCallExtraNameForInt() {
        assertEquals(null, ImsSuppInfoUtils.getCallExtraNameForInt(mMockContext, mInfo));
    }

    @Test
    public void test_getCallExtraNameForString() {
        assertEquals(null, ImsSuppInfoUtils.getCallExtraNameForString(mMockContext, mInfo));
    }

    @Test
    public void test_addCallExtraForApp() {
        mSuppInfo.addService_int(SuppInfo.TYPE_CDIV_CAUSE, 1);
        when(mMockCarrierConfig.getBoolean(
                CarrierConfig.Assets.KEY_SUPPINFO_CDIV_CAUSE_REQUIRED_BOOL)).thenReturn(true);
        ImsSuppInfoUtils.addCallExtraForApp(mMockContext, mSuppInfo, mCallProfile);
        assertEquals(1, mCallProfile.getCallExtraInt(ImsCallUtils.EXTRA_CDIV_CAUSE));

        mSuppInfo = new SuppInfo();
        ImsSuppInfoUtils.addCallExtraForApp(mMockContext, mSuppInfo, mCallProfile);
        assertEquals(-1, mCallProfile.getCallExtraInt(ImsCallUtils.EXTRA_CDIV_CAUSE));
    }

    @Test
    public void test_addSuppInfoForIms() {
        mCallProfile.setCallExtraBoolean(ImsSuppInfoUtils.EXTRA_GEOLOCATION, true);
        ImsSuppInfoUtils.addSuppInfoForIms(mMockContext, mCallProfile, mSuppInfo);
        SuppInfo.SuppService ss = mSuppInfo.getService(SuppInfo.TYPE_GEOLOCATION);
        assertNotNull(ss);
        assertEquals(true, ss.boolValue);
    }
}
