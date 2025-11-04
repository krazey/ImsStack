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
package com.android.imsstack.its.imsservice.mmtel.ut;

import android.os.Bundle;
import android.os.RemoteException;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsData;
import android.telephony.ims.ImsSsInfo;

import androidx.annotation.NonNull;

import com.android.ims.internal.IImsUt;
import com.android.ims.internal.IImsUtListener;
import com.android.imsstack.util.Log;

/**
 * IMS UT interface wrapper.
 */
public final class ImsUtWrapper {

    public static final int INVALID_TID = -1;
    private static final String TAG = "ImsUtWrapper";

    @NonNull private final IImsUt mIImsUt;
    @NonNull private final IImsUtListenerProxy mImsUtListenerProxy;

    /** Constructor. */
    public ImsUtWrapper(@NonNull IImsUt imsUt) {
        mIImsUt = imsUt;

        mImsUtListenerProxy = new IImsUtListenerProxy();
        try {
            setUtListener(mImsUtListenerProxy);
        } catch (RemoteException e) {
            loge("setUtListener failed", e);
            throw new IllegalStateException("Failed to create ImsUtWrapper: " + e, e);
        }
    }

    /**
    * Closes the object. This object is not usable after being closed.
    */
    public void destroy() {
        try {
            mIImsUt.close();
        } catch (RemoteException e) {
            loge("close failed", e);
        }
    }

    /**
     * Retrieves the configuration of the call barring.
     */
    public int queryCallBarring(int cbType) {
        try {
            return mIImsUt.queryCallBarring(cbType);
        } catch (RemoteException e) {
            loge("queryCallBarring failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Retrieves the configuration of the call forward.
     */
    public int queryCallForward(int condition, String number) {
        try {
            return mIImsUt.queryCallForward(condition, number);
        } catch (RemoteException e) {
            loge("queryCallForward failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Retrieves the configuration of the call waiting.
     */
    public int queryCallWaiting() {
        try {
            return mIImsUt.queryCallWaiting();
        } catch (RemoteException e) {
            loge("queryCallWaiting failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Retrieves the default CLIR setting.
     */
    public int queryCLIR() {
        try {
            return mIImsUt.queryCLIR();
        } catch (RemoteException e) {
            loge("queryCLIR failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Retrieves the CLIP call setting.
     */
    public int queryCLIP() {
        try {
            return mIImsUt.queryCLIP();
        } catch (RemoteException e) {
            loge("queryCLIP failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Retrieves the COLR call setting.
     */
    public int queryCOLR() {
        try {
            return mIImsUt.queryCOLR();
        } catch (RemoteException e) {
            loge("queryCOLR failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Retrieves the COLP call setting.
     */
    public int queryCOLP() {
        try {
            return mIImsUt.queryCOLP();
        } catch (RemoteException e) {
            loge("queryCOLP failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Updates the configuration of the call barring.
     */
    public int updateCallBarring(int cbType, int action, String[] barrList) {
        try {
            return mIImsUt.updateCallBarring(cbType, action, barrList);
        } catch (RemoteException e) {
            loge("updateCallBarring failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Updates the configuration of the call forward.
     */
    public int updateCallForward(int action, int condition, String number, int serviceClass,
            int timeSeconds) {
        try {
            return mIImsUt.updateCallForward(action, condition, number, serviceClass, timeSeconds);
        } catch (RemoteException e) {
            loge("updateCallForward failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Updates the configuration of the call waiting.
     */
    public int updateCallWaiting(boolean enable, int serviceClass) {
        try {
            return mIImsUt.updateCallWaiting(enable, serviceClass);
        } catch (RemoteException e) {
            loge("updateCallWaiting failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Updates the configuration of the CLIR supplementary service.
     */
    public int updateCLIR(int clirMode) {
        try {
            return mIImsUt.updateCLIR(clirMode);
        } catch (RemoteException e) {
            loge("updateCLIR failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Updates the configuration of the CLIP supplementary service.
     */
    public int updateCLIP(boolean enable) {
        try {
            return mIImsUt.updateCLIP(enable);
        } catch (RemoteException e) {
            loge("updateCLIP failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Updates the configuration of the COLR supplementary service.
     */
    public int updateCOLR(int presentation) {
        try {
            return mIImsUt.updateCOLR(presentation);
        } catch (RemoteException e) {
            loge("updateCOLR failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Updates the configuration of the COLP supplementary service.
     */
    public int updateCOLP(boolean enable) {
        try {
            return mIImsUt.updateCOLP(enable);
        } catch (RemoteException e) {
            loge("updateCOLP failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Retrieves the configuration of the call barring for specified service class.
     */
    public int queryCallBarringForServiceClass(int cbType, int serviceClass) {
        try {
            return mIImsUt.queryCallBarringForServiceClass(cbType, serviceClass);
        } catch (RemoteException e) {
            loge("queryCallBarringForServiceClass failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Updates the configuration of the call barring for specified service class.
     */
    public int updateCallBarringForServiceClass(int cbType, int action, String[] barrList,
            int serviceClass) {
        try {
            return mIImsUt.updateCallBarringForServiceClass(cbType, action, barrList, serviceClass);
        } catch (RemoteException e) {
            loge("updateCallBarringForServiceClass failed", e);
            return INVALID_TID;
        }
    }

    /**
     * Updates the configuration of the call barring for specified service class with password.
     */
    public int updateCallBarringWithPassword(int cbType, int action, String[] barrList,
            int serviceClass, String password) {
        try {
            return mIImsUt.updateCallBarringWithPassword(
                    cbType, action, barrList, serviceClass, password);
        } catch (RemoteException e) {
            loge("updateCallBarringWithPassword failed", e);
            return INVALID_TID;
        }
    }

    private void setUtListener(@NonNull IImsUtListenerProxy listener) throws RemoteException {
        mIImsUt.setListener(listener);
    }

    private void loge(String s, Throwable tr) {
        Log.e(TAG, s, tr);
    }

    /**
     * A proxy class implementing the IImsUtListener interface to handle Ims supplementary service
     * events. This class acts as a bridge between IImsUtListener interface and the actual listener
     * implementation
     */
    private class IImsUtListenerProxy extends IImsUtListener.Stub {

        @Override
        public void utConfigurationUpdated(IImsUt ut, int id) {}
        @Override
        public void utConfigurationUpdateFailed(IImsUt ut, int id, ImsReasonInfo error) {}

        @Override
        public void utConfigurationQueried(IImsUt ut, int id, Bundle ssInfo) {}
        @Override
        public void utConfigurationQueryFailed(IImsUt ut, int id, ImsReasonInfo error) {}
        @Override
        public void lineIdentificationSupplementaryServiceResponse(int id, ImsSsInfo config) {}
        @Override
        public void utConfigurationCallBarringQueried(IImsUt ut,
                int id, ImsSsInfo[] cbInfo) {}

        @Override
        public void utConfigurationCallForwardQueried(IImsUt ut,
                int id, ImsCallForwardInfo[] cfInfo) {}

        @Override
        public void utConfigurationCallWaitingQueried(IImsUt ut,
                int id, ImsSsInfo[] cwInfo) {}

        @Override
        public void onSupplementaryServiceIndication(ImsSsData ssData) {}
    }
}
