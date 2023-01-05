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

import com.android.imsstack.enabler.sipcontroller.impl.SipControllerAgent;
import com.android.imsstack.imsservice.sipcontroller.ImsSipTransport;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.util.AppContext;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.concurrent.Executor;

public class ImsServiceRecordTest {
    protected Context mMockContext;
    private ImsCallApp mImsCallApp;
    private ImsServiceRegistry mImsServiceRegistry;
    private Executor mExecutor;
    private ImsMmTelService mImsMmTelService;
    private TestImsServiceRecord mImsServiceRecord;
    private SipControllerAgent mSipControllerAgent;
    @Before
    public void setUp() {
        mMockContext = Mockito.mock(Context.class);
        mExecutor = Mockito.mock(Executor.class);
        mImsCallApp = Mockito.mock(ImsCallApp.class);
        mImsMmTelService = Mockito.mock(ImsMmTelService.class);
        mImsServiceRegistry = Mockito.mock(ImsServiceRegistry.class);
        AppContext.init(mMockContext);
        int phoneId = 1;
        mImsServiceRecord = new TestImsServiceRecord(mMockContext, mExecutor, phoneId);
        mImsServiceRecord.setListener(mImsMmTelService);
        mSipControllerAgent = Mockito.mock(SipControllerAgent.class);
    }

    @Test
    public void broadcastServiceDownUPTest() {
        mImsServiceRecord.setListener(null);
        mImsServiceRecord.broadcastServiceDown();
        mImsServiceRecord.broadcastServiceUp();
        Mockito.verify(mImsMmTelService, Mockito.never()).onServiceRecordStateChanged();

        mImsServiceRecord.setListener(mImsMmTelService);
        mImsServiceRecord.broadcastServiceDown();
        mImsServiceRecord.broadcastServiceUp();
        Mockito.verify(mImsMmTelService, Mockito.times(2)).onServiceRecordStateChanged();
    }

    @Test
    public void broadcastServiceUPTest() {
        mImsServiceRecord.setListener(mImsMmTelService);
        mImsServiceRecord.broadcastServiceUp();
        Mockito.verify(mImsMmTelService).onServiceRecordStateChanged();
    }

    @Test
    public void setAndGetCallAppTest() {
        Assert.assertNull(mImsServiceRecord.getCallApp());
        mImsServiceRecord.setCallApp(mImsCallApp);
        Assert.assertNotNull(mImsServiceRecord.getCallApp());
    }

    @Test
    public void getConfigTest() {
        ImsConfigImpl Config = mImsServiceRecord.getConfig();
        Assert.assertNotNull(Config);
    }

    @Test
    public void getSipTransportTest() {
        ImsSipTransport Config = mImsServiceRecord.getSipTransport();
        Assert.assertNotNull(Config);
    }

    @Test
    public void getRegistration() {
        ImsRegistrationImpl registration = mImsServiceRecord.getRegistration();
        Assert.assertNotNull(registration);
    }

    @Test
    public void getRegistrationTrackerTest() {
        ImsRegistrationTracker tracker = mImsServiceRecord.getRegistrationTracker();
        Assert.assertNotNull(tracker);
    }

    @Test
    public void isServiceUp() {
        mImsServiceRecord.broadcastServiceUp();
        boolean serviceUp = mImsServiceRecord.isServiceUp();
        Assert.assertTrue(serviceUp);
    }

    @Test
    public void disableImsTest() {
        mImsServiceRecord.setImsService(mImsServiceRegistry);
        mImsServiceRecord.disableIms();
        Mockito.verify(mImsServiceRegistry).setImsEnabled(false);
    }

    @Test
    public void enableImsTest() {
        mImsServiceRecord.setImsService(mImsServiceRegistry);
        mImsServiceRecord.enableIms();
        Mockito.verify(mImsServiceRegistry).setImsEnabled(true);
    }

    @After
    public void cleanUp() {
        mImsServiceRecord = null;
    }

    class TestImsServiceRecord extends ImsServiceRecord {
        private final Object mLock = new Object();
        ImsSipTransport mSipTransport;
        int mPhoneId = -1;
        TestImsServiceRecord(Context context, Executor executor, int phoneId) {
            super(context, executor, phoneId);
            mPhoneId = phoneId;
        }

        public ImsSipTransport getSipTransport() {
            synchronized (mLock) {
                if (mSipTransport == null) {
                    mSipTransport = ImsSipTransport.createImsSipTransport(mPhoneId,
                            mMockContext, mExecutor, getRegistration(), mSipControllerAgent);
                }
                return mSipTransport;
            }
        }
    }

}
