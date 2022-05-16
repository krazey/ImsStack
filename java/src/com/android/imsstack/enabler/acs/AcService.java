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

package com.android.imsstack.enabler.acs;

import android.annotation.IntDef;
import android.annotation.NonNull;
import android.util.SparseArray;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.concurrent.Executor;

/**
 * Provide interface for ACService(Auto Configuration Service) client to retrieve Provisioning xml
 * from service provider.
  */
public class AcService {
    @Retention(RetentionPolicy.SOURCE)
    @IntDef(prefix = "STATE_TYPE_", flag = true, value = {
            STATE_TYPE_NONE,
            STATE_TYPE_READY,
            STATE_TYPE_PROGRESS
    })
    public @interface AcServiceStateFlag {}

    public static final int STATE_TYPE_NONE = 0;
    public static final int STATE_TYPE_READY = 1;
    public static final int STATE_TYPE_PROGRESS = 2;

    /**
     * The callback for Provisioning changes.
     */
    public static class AcServiceCallback {
        private static class Callback implements IAcServiceImplCallback {
            private final AcServiceCallback mLocalCallback;
            private Executor mExecutor;

            private Callback(AcServiceCallback serviceCallback) {
                mLocalCallback = serviceCallback;
            }

            private void setExecutor(Executor executor) {
                mExecutor = executor;
            }

            @Override
            public void onReceivedProvisioning(byte[] data, boolean isDeProvision) {
                mExecutor.execute(() -> mLocalCallback.onReceivedProvisioning(data, isDeProvision));
            }

            @Override
            public void onReceivedPreProvisioning(byte[] data) {
                mExecutor.execute(() -> mLocalCallback.onReceivedPreProvisioning(data));
            }

            @Override
            public void onReceivedError(int errorCode, String errorString) {
                mExecutor.execute(() -> mLocalCallback.onReceivedError(errorCode, errorString));
            }
        }

        private final Callback mCallback = new Callback(this);

        public final IAcServiceImplCallback getCallback() {
            return mCallback;
        }

        /**
         * Set an executor to call registered callback
         * @param executor The executor of the caller that registered the callback
         */
        public void setExecutor(Executor executor) {
            mCallback.setExecutor(executor);
        }

        /**
         * If override method, then the notification will be transferred when the device receives
         * the provisioning data from service provider server
         * @param data has provisioning.xml
         * @param isDeProvision indicates Provisioning or De-Provisioning
         */
        public void onReceivedProvisioning(byte[] data, boolean isDeProvision) {}

        /**
         * If override method, then the notification will be transferred when the device receives
         * the pre-provisioning data (self-provisioning) from service provider server
         * @param data has provisioning.xml
         */
        public void onReceivedPreProvisioning(byte[] data) {}

        /**
         * If override method, then the notification will be transferred when the device receives
         * the error response regarding request of provisioning
         * @param errorCode HTTP response code
         * @param errorString Text of response code
         */
        public void onReceivedError(int errorCode, String errorString) {}
    }

    private static final String TAG = "AcServiceImplBase";
    private static final SparseArray<AcService> INSTANCES = new SparseArray<AcService>();

    //private AcServiceImpl mAcServiceImpl;
    private int mPhoneId;

    private AcService(int phoneId) {
        mPhoneId = phoneId;

        //mAcServiceImpl = new AcServiceImpl(phoneId);
    }

    /**
     * Returns a AcService for phoneId specified and IllegalArgumentException will be
     * thrown if the phoneId is not valid.
     * @param phoneId The ID of the Phone or SIM Slot that this AcService will use.
     * @return Instance of the AcService
     */
    public static AcService getInstance(int phoneId) {
        AcService acService;
        synchronized (INSTANCES) {
            acService = INSTANCES.get(phoneId);
            if (acService == null) {
                acService = new AcService(phoneId);
                INSTANCES.put(phoneId, acService);
            }
        }

        return acService;
    }

    /**
     * Register a new AcServiceCallback to listen to changes of provisioning
     * @param executor The executor to call the callback method
     * @param serviceCallback The callback to be registered.
     * @return true if the registration callback process is success, or false otherwise.
     */
    public boolean setCallback(@NonNull Executor executor,
            @NonNull AcServiceCallback serviceCallback) {
        serviceCallback.setExecutor(executor);
        //mAcServiceImpl.setCallback(mSubId, serviceCallback.getCallback());
        return true;
    }

    /**
     * Unregister an existing callback. When the subscription associated with this callback is
     * removed, this callback will automatically be removed.
     * @param serviceCallback The callback to be removed.
     */
    public void removeCallback(@NonNull AcServiceCallback serviceCallback) {
        //mAcServiceImpl.removeCallBack(mSubId, serviceCallback.getCallback());
    }

    /**
     * Provide the client configuration parameters which to be included the RCS auto configuration
     * request.
     * @param clientInfo RCS client configuration
     * @return true if the operation is success, or false otherwise.
     */
    public boolean setClientInfo(@NonNull AcServiceClientInfo clientInfo) {
        //mAcServiceImpl.setClientInfo(mSubId, clientInfo);
        return true;
    }

    /**
     * Reconfiguration triggered by the caller
     * @return true if the AcService module can to request provisioning and the result will be
     * notified by callback, or false otherwise.
     */
    public boolean start() {
        // return mAcServiceImpl.start();
        return false;
    }

    /**
     * Stop the configuration process, it is only available when the provisioning process is
     * running. (before called the registered callback)
     */
    public void stop() {
        // mAcServiceImpl.stop();
    }

    /**
     * Notify the provisioning xml has been received
     * @param data Provisioning xml
     * @param isCompressed The xml file is compressed in gzip format
     */
    public void notifyProvisioningReceived(byte[] data, boolean isCompressed) {
        // mAcServiceImpl.notifyConfigDataReceived(data, isCompressed);
    }

    /**
     * The provisioning xml through notifyConfigDataReceived() is not available anymore.
     */
    public void notifyProvisioningRemoved() {
        // mAcServiceImpl.notifyConfigDataRemoved();
    }

    /**
     * Get the state of AcService module
     * @return The state is defined in AcServiceSateFlag
     */
    public @AcServiceStateFlag int getState() {
        // return mAcServiceImpl.getState()
        return STATE_TYPE_NONE;
    }
}
