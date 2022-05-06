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
import android.telephony.ims.ImsService;
import android.telephony.ims.aidl.IImsRcsFeature;
import android.telephony.ims.aidl.IImsServiceController;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.stub.ImsFeatureConfiguration;

import androidx.test.core.app.ApplicationProvider;

import com.android.ims.internal.IImsFeatureStatusCallback;
import com.android.imsstack.enabler.IContext;
import com.android.imsstack.util.Log;

import org.junit.After;
import org.junit.Assert;
import org.junit.Before;
import org.junit.Test;
import org.junit.runner.RunWith;
import org.junit.runners.JUnit4;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;

import java.util.concurrent.TimeUnit;

@RunWith(JUnit4.class)
public class ImsServiceTest {
    IImsServiceController mImsServiceBinder;
    @Mock
    private Context mContext;
    @Mock
    IContext mIContext;
    private int mSlotId = 0;
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
    public void setUp() {
        MockitoAnnotations.initMocks(this);
        mContext = spy(ApplicationProvider.getApplicationContext());
        Intent intent = new Intent(ImsService.SERVICE_INTERFACE);
        intent.setClassName(IMS_PACKAGE_NAME, CLASS_NAME);
        mContext.bindService(intent, mConnection, Context.BIND_AUTO_CREATE);
    }

    @Test
    public void querySupportedImsFeaturesTest() throws Exception {
        ImsFeatureConfiguration config = new ImsFeatureConfiguration.Builder()
                .addFeature(mSlotId, ImsFeature.FEATURE_MMTEL)
                .addFeature(mSlotId, ImsFeature.FEATURE_EMERGENCY_MMTEL)
                .addFeature(mSlotId, ImsFeature.FEATURE_RCS)
                .build();
        //added delay for service binding
        TimeUnit.MINUTES.sleep(1);
        ImsFeatureConfiguration resultConfig = mImsServiceBinder.querySupportedImsFeatures();
        Assert.assertEquals(config, resultConfig);
    }

    @Test
    public void createRcsFeatureTest() throws RemoteException {
        IImsFeatureStatusCallback callback =  new IImsFeatureStatusCallback() {
            @Override
            public void notifyImsFeatureStatus(int featureStatus) throws RemoteException {
                Log.i("service", "notifyImsFeatureStatus()");
            }
            @Override
            public IBinder asBinder() {
                Log.i("service", "binder()");
                return null;
            }
        };
        mImsServiceBinder.addFeatureStatusCallback(0, ImsFeature.FEATURE_RCS, callback);
        IImsRcsFeature result = mImsServiceBinder.createRcsFeature(mSlotId, mIContext.getSubId());
        Assert.assertNotNull(result);
    }

    @After
    public void tearDown() throws Exception {
    }

    @After
    public void cleanUp() {
        mImsServiceBinder = null;
        mContext = null;
    }
}
