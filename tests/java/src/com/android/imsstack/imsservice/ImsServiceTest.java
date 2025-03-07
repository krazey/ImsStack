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

package com.android.imsstack.imsservice;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.when;

import android.app.ActivityManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.telephony.ims.aidl.IImsMmTelFeature;
import android.telephony.ims.aidl.IImsRcsFeature;
import android.telephony.ims.aidl.IImsServiceController;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.feature.RcsFeature;
import android.telephony.ims.stub.ImsFeatureConfiguration;
import android.util.Singleton;

import androidx.test.platform.app.InstrumentationRegistry;

import com.android.imsstack.ImsStackTest;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.imsservice.mmtel.ImsMmTelService;

import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class ImsServiceTest extends ImsStackTest {
    private static final String IMS_PACKAGE_NAME = "com.android.imsstack.tests";
    private static final int AWAIT_TIMEOUT = 5 * 1000;
    private static final int SUB_ID = 1;

    @Mock private ImsMmTelService mMmTelService;
    @Mock private RcsFeature mRcsFeature;
    @Mock private ImsServiceController mServiceController;

    private static TestImsService sImsService;
    private IImsServiceController mImsServiceBinder;
    private ImsServiceConnection mServiceConnection;

    private class ImsServiceConnection implements ServiceConnection {
        private CountDownLatch mLatch;

        ImsServiceConnection(CountDownLatch latch) {
            mLatch = latch;
        }

        public void onServiceConnected(ComponentName className, IBinder service) {
            mImsServiceBinder = IImsServiceController.Stub.asInterface(service);
            mLatch.countDown();
        }

        public void onServiceDisconnected(ComponentName className) {
            mImsServiceBinder = null;
        }
    };

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);
        AppContext.init(mContext);
        when(mServiceController.getMmTelService(eq(MSimUtils.DEFAULT_SLOT_ID)))
                .thenReturn(mMmTelService);
        when(mServiceController.getRcsFeature(eq(MSimUtils.DEFAULT_SLOT_ID)))
                .thenReturn(mRcsFeature);
        replaceInstance(ImsServiceController.class,
                "sImsServiceController", null, mServiceController);

        // Restore the ActivityManager to bind a real Service.
        restoreInstance(Singleton.class, "mInstance", mIActivityManagerSingleton);
        restoreInstance(ActivityManager.class, "IActivityManagerSingleton", null);

        CountDownLatch latch = new CountDownLatch(1);
        mServiceConnection = new ImsServiceConnection(latch);
        Intent intent = new Intent(ImsService.SERVICE_INTERFACE);
        intent.setClassName(IMS_PACKAGE_NAME, TestImsService.class.getName());
        InstrumentationRegistry.getInstrumentation().getTargetContext()
                .bindService(intent, mServiceConnection, Context.BIND_AUTO_CREATE);

        try {
            assertTrue("Timeout to bind to service: " + intent.getComponent(),
                    latch.await(AWAIT_TIMEOUT, TimeUnit.MILLISECONDS));
        } catch (InterruptedException e) {
            fail("Unable to bind to service: " + intent.getComponent());
        }

        sImsService.setImsControllerReady(true);
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown();
        mMmTelService = null;
        mRcsFeature = null;
        mServiceController = null;
        mImsServiceBinder = null;
        mServiceConnection = null;
        AppContext.deinit();
    }

    @Test
    @Ignore // TODO(b/263768170): Binder should not return null
    public void querySupportedImsFeaturesTest() throws Exception {
        assertNotNull(mImsServiceBinder);
        ImsFeatureConfiguration resultConfig = mImsServiceBinder.querySupportedImsFeatures();
        assertNotNull(resultConfig);
    }

    @Test
    public void querySupportedImsFeaturesWithoutBinderTest() {
        //set ImsController ready state false
        sImsService.setImsControllerReady(false);
        ImsFeatureConfiguration result = sImsService.querySupportedImsFeatures();
        assertEquals(0, result.getServiceFeatures().size());

        sImsService.setImsControllerReady(true);
        ImsFeatureConfiguration resultConfig = sImsService.querySupportedImsFeatures();
        assertNotNull(resultConfig);
    }

    @Test
    @Ignore // TODO(b/263768170): Binder should not return null
    public void createRcsFeatureTest() throws RemoteException, InterruptedException {
        assertNotNull(mImsServiceBinder);
        //verify for invalid slot id
        IImsRcsFeature result = mImsServiceBinder
                .createRcsFeature(MSimUtils.INVALID_SLOT_ID, SUB_ID);
        assertNull(result);

        result = mImsServiceBinder.createRcsFeature(MSimUtils.DEFAULT_SLOT_ID, SUB_ID);
        assertNotNull(result);
    }

    @Test
    public void createRcsFeatureWithoutBinderTest() {
        RcsFeature rcs = sImsService.createRcsFeature(MSimUtils.DEFAULT_SLOT_ID);
        assertEquals(mRcsFeature, rcs);
    }

    public static class TestImsService extends ImsService {
        private boolean mReady;

        public TestImsService() {
            sImsService = this;
        }

        @Override
        public boolean isImsControllerReady() {
            return mReady;
        }

        protected void setImsControllerReady(boolean ready) {
            mReady = ready;
        }
    }

    @Test
    @Ignore // TODO(b/263768170): Binder should not return null
    public void createMmTelFeatureTest() throws RemoteException {
        assertNotNull(mImsServiceBinder);
        IImsMmTelFeature iImsMMTelFeature = mImsServiceBinder.createMmTelFeature(
                MSimUtils.INVALID_SLOT_ID, SUB_ID);
        assertNull(iImsMMTelFeature);

        iImsMMTelFeature = mImsServiceBinder.createMmTelFeature(MSimUtils.DEFAULT_SLOT_ID, SUB_ID);
        assertNotNull(iImsMMTelFeature);
    }

    @Test
    public void createMmTelFeatureWithOutBinderTest() throws RemoteException {
        MmTelFeature mmTelFeature = sImsService.createMmTelFeature(MSimUtils.DEFAULT_SLOT_ID);
        assertEquals(mMmTelService, mmTelFeature);
    }
}