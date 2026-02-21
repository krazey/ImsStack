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
import android.telephony.ims.feature.RcsFeature;

import androidx.annotation.NonNull;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.imsservice.base.ImsContext;
import com.android.imsstack.imsservice.mmtel.ImsMmTelService;
import com.android.imsstack.imsservice.mmtel.ImsServiceManager;
import com.android.imsstack.imsservice.mmtel.ImsServiceRecord;
import com.android.imsstack.imsservice.uce.RcsFeatureImpl;
import com.android.imsstack.internal.imsservice.ProvisioningStatusTracker;
import com.android.imsstack.util.IndentingPrintWriter;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MessageExecutor;

import java.io.PrintWriter;
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
        logi(this, "+ISC");

        int simCount = DeviceConfig.getSupportedSimCount();

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
            logi(sImsServiceController, "ISC: create");

            sImsServiceController.mReady = true;

            // For global usage, a current ImsServiceManager will be set as a default.
            ImsServiceManager.setDefault(new ImsServiceManager(
                    appContext, sImsServiceController.mExecutor));
        } else {
            logi(sImsServiceController, "ISC: already created");
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

        logi(sImsServiceController, "ISC: start - s" + slotId);

        boolean isrReconfigurationRequired = false;

        if (!sImsServiceController.mReady) {
            sImsServiceController.mReady = true;

            // For global usage, a current ImsServiceManager will be set as a default.
            ImsServiceManager.setDefault(new ImsServiceManager(
                    appContext, sImsServiceController.mExecutor));
        } else {
            isrReconfigurationRequired = true;
        }

        ProvisioningStatusTracker.getInstance(slotId).start();

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
            sImsServiceController.startRcsServices();
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

    private void startRcsServices() {
        mExecutor.execute(() -> {
            for (RcsFeatureImpl rcsFeature : mRcsFeature) {
                rcsFeature.start();
            }
        });
    }

    public Executor getExecutor() {
        return mExecutor;
    }

    private static void logi(Object o, String s) {
        Log.i(o, "[ISIL] " + s);
    }

    public RcsFeature getRcsFeature(int slotId) {
        logi(this, "getRcsFeature: slotId=" + slotId + ", features=" + mRcsFeature.length);
        if (slotId >= 0 && slotId < mRcsFeature.length) {
            return mRcsFeature[slotId];
        }
        return null;
    }

    /**
     * Dump this instance into a readable format for dumpsys usage.
     */
    public void dump(@NonNull PrintWriter printWriter) {
        IndentingPrintWriter pw = new IndentingPrintWriter(printWriter, "  ");

        for (int i = 0; i < mMmTelServices.length; ++i) {
            pw.printf("Slot%d:\n", i);
            pw.increaseIndent();

            mMmTelServices[i].dump(pw);
            mRcsFeature[i].dump(pw);

            ImsServiceRecord isr = ImsServiceManager.getServiceRecord(i);
            if (isr != null) {
                isr.dump(pw);
            }

            pw.decreaseIndent();
        }
    }
}
