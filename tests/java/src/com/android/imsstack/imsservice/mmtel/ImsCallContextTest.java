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

import android.content.Context;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcServiceStateTracker;
import com.android.imsstack.imsservice.mmtel.internal.WfcSettingTracker;
import com.android.imsstack.util.AppContext;

import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mockito;

import java.util.concurrent.Executor;

@RunWith(JUnit4.class)
public class ImsCallContextTest {

    private Context mContext;
    private IBaseContext mIBaseContext;
    private Executor mExecutor;
    private ImsCallApp mImsCallApp;
    private ImsCallContext mImsCallContext;
    private WfcSettingTracker mWfcsettingtracker;
    private MtcServiceStateTracker mStateTracker;
    private MtcApp mMtcapp;

    @Before
    public void setUp() {
        mContext = Mockito.mock(Context.class);
        AppContext.init(mContext);
        mExecutor = Mockito.mock(Executor.class);
        mWfcsettingtracker = Mockito.mock(WfcSettingTracker.class);
        mStateTracker = new MtcServiceStateTracker(mIBaseContext);
        mMtcapp = Mockito.mock(MtcApp.class);
        mImsCallApp = Mockito.mock(ImsCallApp.class);
        mImsCallContext = new ImsCallContext(mContext, mExecutor, mImsCallApp,
                mWfcsettingtracker, mStateTracker, mMtcapp);
    }

    @Test
    public void initTest() {
        mImsCallContext.init();
        Mockito.verify(mMtcapp).init();
        Mockito.verify(mMtcapp, Mockito.times(2)).setServiceStateListener(
                Mockito.any(MtcServiceStateTracker.class));
        Mockito.verify(mWfcsettingtracker).init();
    }

    @Test
    public void clearTest() {
        mImsCallContext.clear();
        Mockito.verify(mMtcapp).setServiceStateListener(null);
        Mockito.verify(mMtcapp).clear();
        Mockito.verify(mWfcsettingtracker).clear();
    }

    @Test
    public void disposeTest() {
        mImsCallContext.dispose();
        Mockito.verify(mMtcapp).setServiceStateListener(null);
        Mockito.verify(mWfcsettingtracker).dispose();
        Mockito.verify(mMtcapp).close();
    }

    @Test
    public void getContextTest() {
        Assert.assertNotNull(mImsCallContext.getContext());
    }

    @Test
    public void getExecutorTest() {
        Assert.assertNotNull(mImsCallContext.getExecutor());
    }

    @Test
    public void getPhoneidTest() {
        Assert.assertEquals(0, mImsCallContext.getPhoneId());
    }

    @Test
    public void getSlotIdTest() {
        Assert.assertNotNull(mImsCallContext.getSlotId());
    }

    @Test
    public void getSubIdTest() {
        Assert.assertNotNull(mImsCallContext.getSubId());
    }

    @Test
    public void getServiceStateTracker() {
        Assert.assertEquals(mStateTracker, mImsCallContext.getServiceStateTracker());
    }

    @Test
    public void getLocationAgentTest() {
        Assert.assertNull(mImsCallContext.getLocationAgent());
    }
}

