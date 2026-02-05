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

import android.telephony.ims.stub.ImsEcbmImplBase;

import com.android.imsstack.enabler.mtc.EcbmListener;
import com.android.imsstack.enabler.mtc.IECallStateTracker;
import com.android.imsstack.imsservice.mmtel.base.ICallContext;
import com.android.imsstack.util.ImsLog;

public final class ImsEcbmImpl extends ImsEcbmImplBase {
    private final ICallContext mCallContext;
    private final EcbmListenerProxy mListenerProxy = new EcbmListenerProxy();

    public ImsEcbmImpl(ICallContext callContext) {
        mCallContext = callContext;

        init();
    }

    public void dispose() {
        logi("dispose");

        clear();
    }

    public void init() {
        IECallStateTracker ecst = mCallContext.getECallStateTracker();

        if (ecst != null) {
            ecst.addEcbmListener(mListenerProxy);
        }
    }

    public void clear() {
        IECallStateTracker ecst = mCallContext.getECallStateTracker();

        if (ecst != null) {
            ecst.removeEcbmListener(mListenerProxy);
        }
    }

    @Override
    public void exitEmergencyCallbackMode() {
        logi("exitEmergencyCallbackMode");

        IECallStateTracker ecst = mCallContext.getECallStateTracker();

        if (ecst != null) {
            ecst.exitEmergencyCallbackMode(false);
        }
    }

    private void loge(Throwable t, String message) {
        ImsLog.e("[ISIL] " + message + t, t);
    }

    private static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }

    private class EcbmListenerProxy implements EcbmListener {

        public EcbmListenerProxy() {
        }

        /**
         * This is invoked if emergency callback mode is entered.
         */
        @Override
        public void onEcbmEntered() {
            logi("onEcbmEntered");

            try {
                enteredEcbm();
            } catch (Throwable t) {
                loge(t, "enteredEcbm: " + t.getMessage());
            }
        }

        /**
         * This is invoked if emergency callback mode is exited.
         */
        @Override
        public void onEcbmExited() {
            logi("onEcbmExited");

            try {
                exitedEcbm();
            } catch (Throwable t) {
                loge(t, "exitedEcbm: " + t.getMessage());
            }
        }
    }

    // @Override
    // @PRIVATE_IMS_API
    public void exitEmergencyCallbackModeByESMS(boolean bExitedByESMS) {
        logi("exitEmergencyCallbackMode by ESMS, bExitedByESMS=" + bExitedByESMS);

        IECallStateTracker ecst = mCallContext.getECallStateTracker();

        if (ecst != null) {
            ecst.exitEmergencyCallbackMode(bExitedByESMS);
        }
    }
}
