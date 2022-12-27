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

import static org.mockito.Mockito.spy;

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

import com.android.imsstack.enabler.IContext;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Ignore;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.Mockito;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class ImsServiceTest {
    IImsServiceController mImsServiceBinder;
    private TestImsService mImsService;
    private Context mContext;
    @Mock
    IContext mIContext;
    private static final String IMS_PACKAGE_NAME = "com.android.imsstack";
    private static final String CLASS_NAME = "com.android.imsstack.imsservice.ImsService";

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            mImsServiceBinder = IImsServiceController.Stub.asInterface(service);
        }

        public void onServiceDisconnected(ComponentName className) {
            mImsServiceBinder = null;
        }
    };

    @Before
    public void setUp() throws InterruptedException {
        MockitoAnnotations.initMocks(this);
        mContext = spy(ApplicationProvider.getApplicationContext());
        Mockito.when(mIContext.getSlotId()).thenReturn(0);
        Mockito.when(mIContext.getSubId()).thenReturn(1);

        Intent intent = new Intent(ImsService.SERVICE_INTERFACE);
        intent.setClassName(IMS_PACKAGE_NAME, CLASS_NAME);
        mContext.bindService(intent, mConnection, Context.BIND_AUTO_CREATE);

        mImsService = new TestImsService(mContext);
        mImsService.setImsControllerReady(true);
        //added delay for service binding
        TimeUnit.MILLISECONDS.sleep(10);
    }

    @Test
    @Ignore // TODO(b/263768170): Binder should not return null
    public void querySupportedImsFeaturesTest() throws Exception {
        Assert.assertNotNull(mImsServiceBinder);
        ImsFeatureConfiguration resultConfig = mImsServiceBinder.querySupportedImsFeatures();
        Assert.assertNotNull(resultConfig);
    }

    @Test
    public void querySupportedImsFeaturesWithoutBinderTest() {
        //set ImsController ready state false
        mImsService.setImsControllerReady(false);
        ImsFeatureConfiguration result = mImsService.querySupportedImsFeatures();
        Assert.assertEquals(0, result.getServiceFeatures().size());

        ImsServiceController.create(mContext);
        mImsService.setImsControllerReady(true);
        ImsFeatureConfiguration resultConfig = mImsService.querySupportedImsFeatures();
        Assert.assertTrue(ImsServiceController.isReady());
        Assert.assertNotNull(resultConfig);
    }

    @Test
    @Ignore // TODO(b/263768170): Binder should not return null
    public void createRcsFeatureTest() throws RemoteException, InterruptedException {
        Assert.assertNotNull(mImsServiceBinder);
        //verify for invalid slot id
        Mockito.when(mIContext.getSlotId()).thenReturn(-1);
        IImsRcsFeature result = mImsServiceBinder.createRcsFeature(
                mIContext.getSlotId(), mIContext.getSubId());
        Assert.assertNull(result);

        Mockito.when(mIContext.getSlotId()).thenReturn(0);
        result = mImsServiceBinder.createRcsFeature(mIContext.getSlotId(),
                mIContext.getSubId());
        Assert.assertNotNull(result);
    }

    @Test
    public void createRcsFeatureWithoutBinderTest() {
        ImsServiceController.create(mContext);
        RcsFeature rcs = mImsService.createRcsFeature(mIContext.getSlotId());
        Assert.assertNotNull(rcs);
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
        Assert.assertNotNull(mImsServiceBinder);
        Mockito.when(mIContext.getSlotId()).thenReturn(-1);
        IImsMmTelFeature iImsMMTelFeature = mImsServiceBinder.createMmTelFeature(
                mIContext.getSlotId(), mIContext.getSubId());
        Assert.assertNull(iImsMMTelFeature);

        Mockito.when(mIContext.getSlotId()).thenReturn(0);
        iImsMMTelFeature = mImsServiceBinder.createMmTelFeature(
                mIContext.getSlotId(), mIContext.getSubId());
        Assert.assertNotNull(iImsMMTelFeature);
    }

    @Test
    public void createMmTelFeatureWithOutBinderTest() throws RemoteException {
        ImsServiceController.create(mContext);
        MmTelFeature iImsMMTelFeature = mImsService.createMmTelFeature(mIContext.getSlotId());
        Assert.assertNotNull(iImsMMTelFeature);
    }

    @After
    public void cleanUp() {
        mImsServiceBinder = null;
        mContext = null;
        mConnection = null;
        mIContext = null;
    }
}