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

package com.android.imsstack.enabler.acs.impl;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.ISubscription;
import com.android.imsstack.enabler.acs.IAcServiceImplCallback;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;

/**
 * This class handles the multiple IAcServiceImplCallback regardless of the Slot ID and Sub ID.
 */
public class CallbackManager {
    private final ArrayList<IAcServiceImplCallback> mCallbackList =
            new ArrayList<IAcServiceImplCallback>();
    private final Object mLock = new Object();

    // TODO : need to check, mSubId is required?
    private int mSlotId;
    private int mSubId;

    @VisibleForTesting
    public CallbackManager(int slotId, ISubscription iSubscription) {
        mSlotId = slotId;
        try {
            mSubId = iSubscription.getSubId();
        } catch (NullPointerException e) {
            ImsLog.e(e.getMessage());
            mSubId = MSimUtils.INVALID_SUB_ID;
        }
    }

    /**
     * create CallbackManager instance
     * @param slotId SIM slot ID which will be used for trace.
     */
    public CallbackManager(int slotId) {
        new CallbackManager(slotId,
                (ISubscription) AgentFactory.getInstance().getAgent(AgentFactory.SUBSCRIPTION,
                        slotId));
    }

    /**
     * delete all callback references were registered.
     */
    public void clear() {
        synchronized (mLock) {
            mCallbackList.clear();
        }
        ImsLog.i("[" + mSlotId + "] " + "removed all callbacks");
    }

    /**
     * register callback object.
     * @param callback callback instance to be registered
     * @return true if registering is success, otherwise is false
     */
    public boolean registerCallback(IAcServiceImplCallback callback) {
        synchronized (mLock) {
            if (!mCallbackList.contains(callback)) {
                mCallbackList.add(callback);
                return true;
            }
        }

        ImsLog.i("[" + mSlotId + "] " + "callback was registered already");
        return false;
    }

    /**
     * unregister callback object. if there is no registered callback, throw the
     * IllegalArgumentException.
     * @param callback callback instance to be unregistered
     */
    public void unregisterCallback(IAcServiceImplCallback callback) throws
            IllegalArgumentException {
        synchronized (mLock) {
            if (mCallbackList.contains(callback)) {
                mCallbackList.remove(callback);
                return;
            }
        }

        throw new IllegalArgumentException("callback was not registered");
    }

    /**
     * notify receiving provisioning data
     * @param data has provisioning.xml
     * @param isDeProvision indicates Provisioning or De-Provisioning
     */
    public void notifyOnReceivedProvisioning(byte[] data, boolean isDeProvision) {
        ImsLog.i("[" + mSlotId + "] " + "notify onReceivedProvisioning " + isDeProvision);

        synchronized (mLock) {
            for (IAcServiceImplCallback cb : mCallbackList) {
                if (cb != null) {
                    cb.onReceivedProvisioning(data, isDeProvision);
                }
            }
        }
    }

    /**
     * notify receiving pre-provisioning data
     * @param data has pre-provisioning.xml
     */
    public void notifyOnReceivedPreProvisioning(byte[] data) {
        ImsLog.i("[" + mSlotId + "] " + "notify onReceivedPreProvisioning");

        synchronized (mLock) {
            for (IAcServiceImplCallback cb : mCallbackList) {
                if (cb != null) {
                    cb.onReceivedPreProvisioning(data);
                }
            }
        }
    }

    /**
     * notify receiving error response
     * @param errorCode integer type error code, 2xx/4xx/5xx
     * @param errorString string representing error code
     */
    public void notifyOnReceivedError(int errorCode, String errorString) {
        ImsLog.i("[" + mSlotId + "] " + "notify onReceivedError error code : "
                + errorCode + " error string : " + errorString);

        synchronized (mLock) {
            for (IAcServiceImplCallback cb : mCallbackList) {
                if (cb != null) {
                    cb.onReceivedError(errorCode, errorString);
                }
            }
        }
    }


    /**
     * get number of callbacks
     * @return count of callbacks
     */
    @VisibleForTesting
    public int getCallbackCount() {
        synchronized (mLock) {
            return mCallbackList.size();
        }
    }
}
