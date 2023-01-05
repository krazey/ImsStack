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
package com.android.imsstack.core.agents;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;

import com.android.imsstack.system.IJNIUpCallEvt;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.JNIUpCallEvtManager;
import com.android.imsstack.util.ImsLog;

public class SharedStateAgent implements ISharedState {
    // Constants--------------------------------------------------
    // Variables--------------------------------------------------
    private Handler mSharedStateHandler;
    private RegistrantList mNativeBootCompleteRegistrants = new RegistrantList();
    private boolean mNativeBootCompleted = false;

    private int mSlotId = 0;

    // Public methods --------------------------------------------
    public SharedStateAgent(int slotId) {
        mSlotId = slotId;
    }

    // Interface implementation methods --------------------------
    @Override
    public void init(Context context) {
        ImsLog.d(mSlotId, "");

        mSharedStateHandler = new Handler(Looper.myLooper()) {
           public void handleMessage(Message msg) {
                ImsLog.i(mSlotId, "SharedStateAgentHandler :: what=" + msg.what);

                switch (msg.what) {
                    case ImsEventDef.IMS_EVENT_NATIVE_BOOT_COMPLETED:
                        mNativeBootCompleted = true;
                        mNativeBootCompleteRegistrants.notifyRegistrants();
                        break;
                    default:
                        break;
                }
           }
        };

        IJNIUpCallEvt jniEvt = JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(mSlotId);
        if (jniEvt != null) {
            jniEvt.registerForNativeBootComplete(mSharedStateHandler,
                ImsEventDef.IMS_EVENT_NATIVE_BOOT_COMPLETED, null);
        }
    }

    @Override
    public void cleanup() {
        if (mSharedStateHandler != null) {
            IJNIUpCallEvt jniEvt = JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(mSlotId);
            if (jniEvt != null) {
                jniEvt.unregisterForNativeBootComplete(mSharedStateHandler);
            }

            mSharedStateHandler.removeCallbacksAndMessages(null);
            mSharedStateHandler = null;
        }

        mNativeBootCompleted = false;
    }

    @Override
    public boolean isNativeBootCompleted() {
        return mNativeBootCompleted;
    }

    @Override
    public void registerForNativeBootComplete(Handler h, int what, Object obj) {
        mNativeBootCompleteRegistrants.add(new Registrant(h, what, obj));

        if (isNativeBootCompleted()) {
            mNativeBootCompleteRegistrants.notifyRegistrants();
        }
    }

    @Override
    public void unregisterForNativeBootComplete(Handler h) {
        mNativeBootCompleteRegistrants.remove(h);
    }
}
