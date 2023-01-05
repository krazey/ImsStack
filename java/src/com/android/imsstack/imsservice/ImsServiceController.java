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

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.telephony.ims.feature.RcsFeature;

import com.android.imsstack.imsservice.base.ImsContext;
import com.android.imsstack.imsservice.mmtel.ImsMmTelService;
import com.android.imsstack.imsservice.mmtel.ImsServiceManager;
import com.android.imsstack.imsservice.mmtel.ImsServiceRecord;
import com.android.imsstack.imsservice.uce.RcsFeatureImpl;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.imsstack.util.Log;

import java.util.concurrent.Executor;

/**
 * Implements a singleton class to manage multiple ImsFeatures(MMTel, RCS).
 */
public class ImsServiceController {
    private static ImsServiceController sImsServiceController = null;

    private final MessageExecutor mExecutor
            = new MessageExecutor(ImsServiceController.class.getSimpleName());
    private final ImsMmTelService mMmTelServices[];
    //Make service for RCS feature support
    private final RcsFeatureImpl mRcsFeature[];

    private boolean mReady = false;
    private boolean mStarted = false;

    private ImsServiceController(Context context) {
        logi("ImsServiceController");

        int simCount = getSimCount(context);

        mMmTelServices = new ImsMmTelService[simCount];
        mRcsFeature = new RcsFeatureImpl[simCount];

        for (int i = 0; i < simCount; i++) {
            mMmTelServices[i] = new ImsMmTelService(new ImsContext(context, mExecutor, i));
            mRcsFeature[i] = new RcsFeatureImpl(new ImsContext(context, mExecutor, i));
        }
    }

    public static synchronized void create(Context appContext) {
        if (sImsServiceController == null) {
            sImsServiceController = new ImsServiceController(appContext);
        }

        if (!sImsServiceController.mReady) {
            logi("ImsServiceController :: create");

            sImsServiceController.mReady = true;

            // For global usage, a current ImsServiceManager will be set as a default.
            ImsServiceManager.setDefault(new ImsServiceManager(
                    appContext, sImsServiceController.mExecutor));
        } else {
            logi("ImsServiceController :: already created");
        }
    }

    public static synchronized void destroy(Context appContext) {
        if ((sImsServiceController != null) && sImsServiceController.mReady) {
            sImsServiceController.binderDied();
        }
    }

    public static synchronized void start(Context appContext, int slotId) {
        if (sImsServiceController == null) {
            sImsServiceController = new ImsServiceController(appContext);
        }

        logi("ImsServiceController :: start - s" + slotId);

        boolean isrReconfigurationRequired = false;

        if (!sImsServiceController.mReady) {
            sImsServiceController.mReady = true;

            // For global usage, a current ImsServiceManager will be set as a default.
            ImsServiceManager.setDefault(new ImsServiceManager(
                    appContext, sImsServiceController.mExecutor));
        } else {
            isrReconfigurationRequired = true;
        }

        if (!sImsServiceController.mStarted) {
            sImsServiceController.mStarted = true;

            // Start IMS service (MMTelFeature)
            ImsServiceRecord isr = ImsServiceManager.getServiceRecord(slotId);

            if (isr != null) {
                isr.broadcastServiceUp();

                if (isrReconfigurationRequired) {
                    isr.reconfigure();
                }
            }

            sImsServiceController.startMmTelServices();
        } else {
            ImsServiceRecord isr = ImsServiceManager.getServiceRecord(slotId);

            if (isr != null) {
                isr.broadcastServiceUp();
            }
        }
    }

    public static synchronized ImsServiceController getInstance() {
        return sImsServiceController;
    }

    public static int getSimCount(Context c) {
        TelephonyManager tm = c.getSystemService(TelephonyManager.class);

        int count = (tm != null) ? tm.getActiveModemCount() : 1;

        return (count == 0) ? 1 : count;
    }

    public static synchronized boolean isReady() {
        if (sImsServiceController == null) {
            return false;
        }

        return sImsServiceController.mReady;
    }

    private void binderDied() {
        mExecutor.execute(() -> {
                for (int i = 0; i < mMmTelServices.length; i++) {
                    mMmTelServices[i].binderDied();
                }
            });
    }

    public ImsMmTelService getMmTelService(int slotId) {
        if (slotId >= 0 && slotId < mMmTelServices.length) {
            return mMmTelServices[slotId];
        }

        return null;
    }

    private void startMmTelServices() {
        mExecutor.execute(() -> {
                for (int i = 0; i < mMmTelServices.length; i++) {
                    mMmTelServices[i].start();
                }
            });
    }

    private static Looper createLooper(String name) {
        HandlerThread thread = new HandlerThread(name);
        thread.start();

        Looper looper = thread.getLooper();

        if (looper == null) {
            return Looper.getMainLooper();
        }

        return looper;
    }

    private static void log(String s) {
        Log.d(Log.TAG, "[GII-IMPL] " + s);
    }

    private static void logi(String s) {
        Log.i(Log.TAG, "[GII-IMPL] " + s);
    }

    private static void loge(String s) {
        Log.e(Log.TAG, "[GII-IMPL] " + s);
    }

    public RcsFeature getRcsFeature(int slotId) {
        logi("getRcsFeature for slotId:" + slotId);
        if (slotId >= 0 && slotId < mRcsFeature.length) {
            return mRcsFeature[slotId];
        }
        loge("getRcsFeature is null for slotId:" + slotId);
        return null;
    }

    /**
     * Executes the tasks in the other thread rather than the calling thread.
     */
    private static class MessageExecutor extends Handler implements Executor {
        public MessageExecutor(String name) {
            super(createLooper(name));
        }

        @Override
        public void execute(Runnable r) {
            Message m = Message.obtain(this, 0 /* don't care */, r);
            m.sendToTarget();
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg.obj instanceof Runnable) {
                executeInternal((Runnable)msg.obj);
            } else {
                log("[MessageExecutor] handleMessage :: "
                    + "Not runnable object; ignore the msg=" + msg);
            }
        }

        private void executeInternal(Runnable r) {
            try {
                r.run();
            } catch (Throwable t) {
                loge("[MessageExecutor] executeInternal :: run task=" + r);
                t.printStackTrace();
            } finally {
            }
        }
    }

    /** Dump this instance into a readable format for dumpsys usage. */
    public void dump(IndentingPrintWriter pw) {
        pw.println("ImsServiceController:");
        pw.increaseIndent();
        for (ImsMmTelService mmTel : mMmTelServices) {
            mmTel.dump(pw);
        }
        for (RcsFeatureImpl rcs : mRcsFeature) {
            rcs.dump(pw);
        }
        pw.decreaseIndent();
    }
}
