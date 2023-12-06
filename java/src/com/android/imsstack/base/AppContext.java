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
package com.android.imsstack.base;

import static android.provider.Settings.Global.DEVICE_NAME;

import android.content.Context;
import android.content.ContextWrapper;
import android.os.Environment;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;

import androidx.annotation.NonNull;
import androidx.annotation.VisibleForTesting;

import com.android.imsstack.util.MessageExecutor;

import java.util.Objects;
import java.util.concurrent.Executor;

/**
 * This class holds the application context.
 */
public final class AppContext extends ContextWrapper {
    private static AppContext sAppContext = null;

    private final Handler mMainHandler;
    private final MessageExecutor mMainExecutor;
    private BroadcastReceiverProxy mBroadcastReceiverProxy;
    private ContentProviderProxy mContentProviderProxy;
    private SystemServiceProxy mSystemServiceProxy;

    AppContext(Context context, Looper looper) {
        super(context);
        mMainHandler = new Handler(looper);
        mMainExecutor = new MessageExecutor(looper);
        mBroadcastReceiverProxy = new BroadcastReceiverProxyImpl(context, mMainHandler);
        mContentProviderProxy = new ContentProviderProxyImpl(context);
        mSystemServiceProxy = new SystemServiceProxyImpl(context);
    }

    private void releaseInstance() {
        mMainHandler.getLooper().quit();
        mSystemServiceProxy = null;
        mContentProviderProxy = null;
    }

    /**
     * Initializes an instance of the application context.
     *
     * NOTE: This method is invoked when IMS application is created.
     */
    public static void init(Context context) {
        if (sAppContext == null) {
            HandlerThread handlerThread = new HandlerThread(AppContext.class.getSimpleName());
            handlerThread.start();
            sAppContext = new AppContext(context, handlerThread.getLooper());
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
     * Runs the specified task through the main handler.
     *
     * @param task The task to be run.
     * @param millis The delayed time as milli-seconds.
     */
    public static void runTask(@NonNull Runnable task, long millis) {
        Objects.requireNonNull(task, "task must not be null");

        if (sAppContext == null) {
            task.run();
        } else {
            sAppContext.runTaskAsync(task, millis);
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
     * Creates a new {@link TelephonyManagerProxy} object pinned to the given {@code subId}.
     *
     * @return A {@link TelephonyManagerProxy} object that uses the given {@code subId}.
     */
    public static @NonNull TelephonyManagerProxy getTelephonyManagerProxy(int subId) {
        TelephonyManagerProxy tmp = getInstance()
                .getSystemServiceProxy(TelephonyManagerProxy.class);
        return tmp.createForSubscriptionId(subId);
    }

    /**
     * Creates a new {@link SystemServiceProxy.SmsManagerProxy} object pinned to the given
     * {@code subId}.
     *
     * @return A {@link SystemServiceProxy.SmsManagerProxy} object that uses the given
     *         {@code subId}.
     */
    public static @NonNull SystemServiceProxy.SmsManagerProxy getSmsManagerProxy(int subId) {
        SystemServiceProxy.SmsManagerProxy smp = getInstance()
                .getSystemServiceProxy(SystemServiceProxy.SmsManagerProxy.class);
        return smp.createForSubscriptionId(subId);
    }

    /**
     * Returns a specific system service corresponding to the given class.
     *
     * @param clazz A requested class name.
     * @return A system service object corresponding to the given class.
     */
    public @NonNull <T> T getSystemServiceProxy(Class<T> clazz) {
        return mSystemServiceProxy.getSystemService(clazz);
    }

    /**
     * Returns the {@link BroadcastReceiverProxy} to register and unregister the broadcast intent
     * receiver.
     *
     * @return The {@link BroadcastReceiverProxy} instance.
     */
    public @NonNull BroadcastReceiverProxy getBroadcastReceiverProxy() {
        return mBroadcastReceiverProxy;
    }

    /**
     * Returns the {@link ContentProviderProxy} to access the Settings provider or
     * register/unregister the content observer.
     *
     * @return The {@link ContentProviderProxy} instance.
     */
    public @NonNull ContentProviderProxy getContentProviderProxy() {
        return mContentProviderProxy;
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
        return mMainHandler.getLooper();
    }

    /**
     * Runs the specified task asynchronously on the main handler.
     *
     * @param task The task to be run.
     * @param millis The delayed time as milli-seconds.
     */
    public void runTaskAsync(@NonNull Runnable task, long millis) {
        Objects.requireNonNull(task, "task must not be null");

        if (millis > 0) {
            mMainHandler.postDelayed(task, millis);
        } else {
            mMainHandler.post(task);
        }
    }

    /**
     * Sets the fake {@link BroadcastReceiverProxy} object for a test purpose.
     */
    @VisibleForTesting
    public void setBroadcastReceiverProxy(BroadcastReceiverProxy broadcastReceiverProxy) {
        mBroadcastReceiverProxy = broadcastReceiverProxy;
    }

    /**
     * Sets the fake {@link ContentProviderProxy} object for a test purpose.
     */
    @VisibleForTesting
    public void setContentProviderProxy(ContentProviderProxy contentProviderProxy) {
        mContentProviderProxy = contentProviderProxy;
    }

    /**
     * Sets the fake {@link SystemServiceProxy} object for a test purpose.
     */
    @VisibleForTesting
    public void setSystemServiceProxy(SystemServiceProxy systemServiceProxy) {
        mSystemServiceProxy = systemServiceProxy;
    }

    /**
     * Returns the current device name.
     *
     * @return A device name.
     */
    public String getDeviceName() {
        return getContentProviderProxy().getGlobalSettings().getString(DEVICE_NAME, "");
    }

    /**
     * Returns the external storage path.
     *
     * @return An external storage path.
     */
    public static String getExternalStoragePath() {
        String state = Environment.getExternalStorageState();

        if (!Environment.MEDIA_MOUNTED.equalsIgnoreCase(state)) {
            return "";
        }

        return Environment.getExternalStorageDirectory().getAbsolutePath();
    }
}
