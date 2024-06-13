/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.imsservice;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.RemoteException;
import android.telephony.ims.aidl.IImsMmTelFeature;
import android.telephony.ims.aidl.IImsRegistration;
import android.telephony.ims.aidl.IImsServiceController;

import androidx.annotation.NonNull;

import com.android.ims.internal.IImsUt;
import com.android.imsstack.base.AppContext;
import com.android.imsstack.imsservice.ImsService;
import com.android.imsstack.its.base.TestConstants;
import com.android.imsstack.its.imsservice.mmtel.ImsMmTelFeatureWrapper;
import com.android.imsstack.its.imsservice.mmtel.sms.ImsSmsWrapper;
import com.android.imsstack.its.imsservice.mmtel.ut.ImsUtWrapper;
import com.android.imsstack.its.imsservice.reg.ImsRegistrationWrapper;
import com.android.imsstack.util.Log;

/**
 * IMS service connector
 */
public final class ImsServiceConnector {
    private static final String IMS_PACKAGE_NAME = "com.android.imsstack.its";

    private static ImsServiceConnector sImsServiceConnector = null;
    private static TestImsService sImsService;

    private IImsServiceController mImsServiceBinder;
    private ImsServiceConnection mServiceConnection;
    private ImsMmTelFeatureWrapper mMmTelFeatureWrapper;
    private ImsRegistrationWrapper mRegistrationWrapper;
    private ImsSmsWrapper mSmsWrapper;
    private ImsUtWrapper mUtWrapper;

    /**
     * Gets static instance.
     */
    public static ImsServiceConnector getInstance() {
        if (sImsServiceConnector == null) {
            sImsServiceConnector = new ImsServiceConnector();
        }
        return sImsServiceConnector;
    }

    /**
     * Start.
     */
    public void start() {
        bindImsService();
    }

    /**
     * Stop.
     */
    public void stop() {
        destroyWrappers();

        try {
            if (mImsServiceBinder != null) {
                mImsServiceBinder.disableIms(TestConstants.SLOT0, TestConstants.SUB_ID_1);
            }
        } catch (RemoteException e) {
            loge("stop:" + e.toString());
        } finally {
            mImsServiceBinder = null;
            sImsService = null;
        }

        unbindImsService();
    }

    /**
     * Gets MMTEL feature wrapper.
     */
    public ImsMmTelFeatureWrapper getMmTelFeature() {
        return mMmTelFeatureWrapper;
    }

    /**
     * Gets Registration interface wrapper.
     */
    public ImsRegistrationWrapper getRegistration() {
        return mRegistrationWrapper;
    }

    /**
     * Gets SMS interface wrapper.
     */
    public ImsSmsWrapper getSms() {
        return mSmsWrapper;
    }

    /**
     * Gets UT interface wrapper.
     */
    public ImsUtWrapper getUt() {
        return mUtWrapper;
    }

    private void bindImsService() {
        mServiceConnection = new ImsServiceConnection();
        Intent intent = new Intent(ImsService.SERVICE_INTERFACE);
        intent.setClassName(IMS_PACKAGE_NAME, TestImsService.class.getName());
        AppContext.getInstance().bindService(intent, mServiceConnection,
                Context.BIND_AUTO_CREATE);
    }

    private void unbindImsService() {
        if (mServiceConnection != null) {
            AppContext.getInstance().unbindService(mServiceConnection);
            mServiceConnection = null;
        }
    }

    private void createMmTelWrappers(@NonNull IImsMmTelFeature mmtelFeature) {
        if (mMmTelFeatureWrapper != null) {
            mMmTelFeatureWrapper.destroy();
        }
        mMmTelFeatureWrapper = new ImsMmTelFeatureWrapper(mmtelFeature);

        if (mSmsWrapper != null) {
            mSmsWrapper.destroy();
        }
        mSmsWrapper = new ImsSmsWrapper(mmtelFeature);

        if (mUtWrapper != null) {
            mUtWrapper.destroy();
            mUtWrapper = null;
        }
        IImsUt imsUt = mMmTelFeatureWrapper.getUtInterface();
        if (imsUt != null) {
            mUtWrapper = new ImsUtWrapper(imsUt);
        }
    }

    private void createRegistrationWrapper(@NonNull IImsRegistration registration) {
        if (mRegistrationWrapper != null) {
            mRegistrationWrapper.destroy();
        }
        mRegistrationWrapper = new ImsRegistrationWrapper(registration);
    }

    private void destroyWrappers() {
        if (mUtWrapper != null) {
            mUtWrapper.destroy();
            mUtWrapper = null;
        }

        if (mSmsWrapper != null) {
            mSmsWrapper.destroy();
            mSmsWrapper = null;
        }

        if (mMmTelFeatureWrapper != null) {
            mMmTelFeatureWrapper.destroy();
            mMmTelFeatureWrapper = null;
        }

        if (mRegistrationWrapper != null) {
            mRegistrationWrapper.destroy();
            mRegistrationWrapper = null;
        }
    }

    private void notifyOnServiceConnected(IBinder service) {
        mImsServiceBinder = IImsServiceController.Stub.asInterface(service);

        try {
            mImsServiceBinder.enableIms(TestConstants.SLOT0, TestConstants.SUB_ID_1);

            IImsRegistration registration = mImsServiceBinder.getRegistration(
                    TestConstants.SLOT0, TestConstants.SUB_ID_1);
            if (registration != null) {
                createRegistrationWrapper(registration);
            }

            IImsMmTelFeature mmtelFeature = mImsServiceBinder.createMmTelFeature(
                    TestConstants.SLOT0, TestConstants.SUB_ID_1);
            if (mmtelFeature != null) {
                createMmTelWrappers(mmtelFeature);
            }
        } catch (RemoteException e) {
            loge("notifyOnServiceConnected:" + e.toString());
        }
    }

    private static void logi(String s) {
        Log.i(Log.TAG, "ImsServiceConnector: " + s);
    }

    private static void loge(String s) {
        Log.e(Log.TAG, "ImsServiceConnector: " + s);
    }

    private class ImsServiceConnection implements ServiceConnection {
        public void onServiceConnected(ComponentName className, IBinder service) {
            logi("onServiceConnected = " + className + " / " + service);
            notifyOnServiceConnected(service);
        }

        public void onServiceDisconnected(ComponentName className) {
            logi("onServiceDisconnected = " + className);
            stop();
        }
    };

    private static class TestImsService extends ImsService {
        TestImsService() {
            sImsService = this;
        }
    }
}
