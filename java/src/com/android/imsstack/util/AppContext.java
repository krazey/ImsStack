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
package com.android.imsstack.util;

import android.content.Context;
import android.content.ContextWrapper;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.telephony.TelephonyManager;

import java.util.concurrent.Executor;

/**
 * This class holds the application context.
 */
public final class AppContext extends ContextWrapper {
    private static AppContext sAppContext = null;

    private final Handler mMainHandler;
    private final HandlerThread mMainHandlerThread =
            new HandlerThread(AppContext.class.getSimpleName());
    private final MessageExecutor mMainExecutor;

    private AppContext(Context context) {
        super(context);
        mMainHandlerThread.start();
        mMainHandler = new Handler(mMainHandlerThread.getLooper());
        mMainExecutor = new MessageExecutor(mMainHandlerThread.getLooper());
    }

    private void releaseInstance() {
        mMainHandlerThread.quit();
    }

    /**
     * Initializes an instance of the application context.
     *
     * NOTE: This method is invoked when IMS application is created.
     */
    public static void init(Context context) {
        if (sAppContext == null) {
            sAppContext = new AppContext(context);
        }
    }

    /**
     * Deinitializes all the resources that were allocated by the AppContext.
     */
    public static void deinit() {
        if (sAppContext != null) {
            sAppContext.releaseInstance();
            sAppContext = null;
        }
    }

    /**
     * Returns the Telephony Manager that is running as a system service.
     */
    public static TelephonyManager getTelephonyManager() {
        return (sAppContext != null) ? sAppContext.getSystemService(TelephonyManager.class) : null;
    }

    /**
     * Returns the Telephony Manager with the specified subscription.
     *
     * @param subId The subscription id.
     */
    public static TelephonyManager getTelephonyManager(int subId) {
        TelephonyManager tm = getTelephonyManager();
        return (tm != null) ? tm.createForSubscriptionId(subId) : null;
    }

    /**
     * Runs any tasks using the main handler.
     *
     * @param task The task to be run.
     * @param millis The delayed time as milli-seconds.
     */
    public static void runTask(Runnable task, long millis) {
        if (task == null) {
            return;
        }

        Handler h = (sAppContext != null) ? sAppContext.getMainHandler() : null;

        if (h != null) {
            if (millis > 0) {
                h.postDelayed(task, millis);
            } else {
                h.post(task);
            }
        } else {
            task.run();
        }
    }

    /**
     * Returns the AppContext instance of ImsStack.
     */
    public static AppContext getInstance() {
        if (sAppContext == null) {
            throw new IllegalStateException("AppContext is not initialized!");
        }

        return sAppContext;
    }

    /**
     * Returns the main executor for ImsStack.
     */
    public Executor getMainExecutor() {
        return mMainExecutor;
    }

    /**
     * Returns the main handler for ImsStack.
     */
    public Handler getMainHandler() {
        return mMainHandler;
    }

    /**
     * Returns the main looper for ImsStack.
     */
    public Looper getMainLooper() {
        return mMainHandlerThread.getLooper();
    }
}
