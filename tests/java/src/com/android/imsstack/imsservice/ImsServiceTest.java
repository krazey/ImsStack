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
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.spy;
import static org.mockito.Mockito.when;

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

import androidx.test.core.app.ApplicationProvider;

import com.android.imsstack.ContextFixture;
import com.android.imsstack.ImsStackTest;
import com.android.imsstack.imsservice.mmtel.ImsMmTelService;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.MSimUtils;

import org.junit.After;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class ImsServiceTest extends ImsStackTest {
    private static final String IMS_PACKAGE_NAME = "com.android.imsstack";
    private static final String CLASS_NAME = "com.android.imsstack.imsservice.ImsService";
    private static final int SLOT1 = 1;
    private static final int SUB_ID = 1;

    @Mock private ImsMmTelService mMmTelService;
    @Mock private RcsFeature mRcsFeature;
    @Mock private ImsServiceController mServiceController;
    private IImsServiceController mImsServiceBinder;
    private TestImsService mImsService;
    private Context mContext;
    private ContextFixture mContextFixture;

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            mImsServiceBinder = IImsServiceController.Stub.asInterface(service);
        }

        public void onServiceDisconnected(ComponentName className) {
            mImsServiceBinder = null;
        }
    };

    @Before
    public void setUp() throws Exception {
        super.setUp(getClass().getSimpleName());
        MockitoAnnotations.initMocks(this);
        mContext = spy(ApplicationProvider.getApplicationContext());
        mContextFixture = new ContextFixture();
        AppContext.init(mContextFixture.getTestDouble());
        when(mServiceController.getMmTelService(eq(MSimUtils.DEFAULT_SLOT_ID)))
                .thenReturn(mMmTelService);
        when(mServiceController.getRcsFeature(eq(MSimUtils.DEFAULT_SLOT_ID)))
                .thenReturn(mRcsFeature);
        replaceInstance(ImsServiceController.class,
                "sImsServiceController", null, mServiceController);

        Intent intent = new Intent(ImsService.SERVICE_INTERFACE);
        intent.setClassName(IMS_PACKAGE_NAME, CLASS_NAME);
        mContext.bindService(intent, mConnection, Context.BIND_AUTO_CREATE);

        mImsService = new TestImsService(mContext);
        mImsService.setImsControllerReady(true);
        //added delay for service binding
        TimeUnit.MILLISECONDS.sleep(10);
    }

    @After
    public void tearDown() throws Exception {
        super.tearDown();
        mMmTelService = null;
        mRcsFeature = null;
        mServiceController = null;
        mImsServiceBinder = null;
        mContext = null;
        mConnection = null;
        mContextFixture = null;
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
        mImsService.setImsControllerReady(false);
        ImsFeatureConfiguration result = mImsService.querySupportedImsFeatures();
        assertEquals(0, result.getServiceFeatures().size());

        mImsService.setImsControllerReady(true);
        ImsFeatureConfiguration resultConfig = mImsService.querySupportedImsFeatures();
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
        RcsFeature rcs = mImsService.createRcsFeature(MSimUtils.DEFAULT_SLOT_ID);
        assertEquals(mRcsFeature, rcs);
    }

    private static class TestImsService extends ImsService {
        Context mContext;
        boolean mReady;
        TestImsService(Context context) {
            mContext = context;
        }
        public Context getAppContext() {
            return mContext;
        }
        private void setImsControllerReady(boolean ready) {
            mReady = ready;
        }

        public boolean isImsControllerReady() {
            return mReady;
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
        MmTelFeature mmTelFeature = mImsService.createMmTelFeature(MSimUtils.DEFAULT_SLOT_ID);
        assertEquals(mMmTelService, mmTelFeature);
    }
}