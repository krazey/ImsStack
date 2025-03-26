/**
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

package com.android.imsstack.enabler.media;

import android.annotation.NonNull;
import android.annotation.Nullable;
import android.content.Context;
import android.os.HandlerThread;
import android.os.Looper;
import android.telephony.imsmedia.ImsMediaManager;
import android.telephony.imsmedia.ImsMediaSession;
import android.telephony.imsmedia.RtpConfig;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.net.DatagramSocket;
import java.util.concurrent.Executor;
import java.util.concurrent.Executors;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * This instantiates ImsMediaManager and interacts with ImsMedia framework
 */
public class MediaManagerHelper {

    private final Context mContext;
    private static AtomicBoolean sIsImsMediaConnected = new AtomicBoolean(false);
    private static ImsMediaManager sImsMediaManager;
    private static Executor sExecutor;
    private static IMediaConnectionObserver sMediaObserver;
    private static HandlerThread sMediaHandlerThread;

    @SuppressWarnings("StaticAssignmentInConstructor")
    public MediaManagerHelper(IMediaConnectionObserver mediaObserver) {

        sMediaObserver = mediaObserver;
        mContext = AppContext.getInstance();
        createImsMediaManagerInstance();
    }

    @VisibleForTesting
    @SuppressWarnings("StaticAssignmentInConstructor")
    public MediaManagerHelper(Context context, IMediaConnectionObserver mediaObserver,
        ImsMediaManager imsMediaManager, Executor executor) {

        mContext = context;
        sMediaObserver = mediaObserver;
        if (sImsMediaManager == null) {
            sImsMediaManager = imsMediaManager;
            sExecutor = executor;
            sMediaHandlerThread = new HandlerThread(MediaManagerHelper.class.getSimpleName());
            if (sMediaHandlerThread.getState() == Thread.State.NEW) {
                sMediaHandlerThread.start();
            }
        }
    }

    private void createImsMediaManagerInstance() {

        if (sImsMediaManager == null) {
            ImsLog.d("ImsMediaManager instance created");
            sMediaHandlerThread = new HandlerThread(MediaManagerHelper.class.getSimpleName());
            sExecutor = Executors.newSingleThreadExecutor();
            setImsMediaConnected(false);
            sImsMediaManager =
                    new ImsMediaManager(mContext, sExecutor, new ImsMediaManagerCallback());
            if (sMediaHandlerThread.getState() == Thread.State.NEW) {
                sMediaHandlerThread.start();
            }
        }
    }

    /**
     * Returns ImsMediaManager instance
     */
    private ImsMediaManager getImsMediaManagerInstance() {
        createImsMediaManagerInstance();
        return sImsMediaManager;
    }

    /**
     * Returns Executor instance
     */
    private Executor getExecutor() {
        return sExecutor;
    }

    /**
     * Returns whether ImsMedia is connected or not
     */
    public static boolean isImsMediaConnected() {
        return sIsImsMediaConnected.get();
    }

    /**
     * Sets whether ImsMedia is connected or not
     */
    @VisibleForTesting
    static void setImsMediaConnected(boolean isConnected) {
        sIsImsMediaConnected.set(isConnected);
        ImsLog.i("ImsMediaManager is connected[" + isImsMediaConnected() + "]");
    }

    /**
     * clears the resources
     */
    @VisibleForTesting
    void close() {
        if (sImsMediaManager != null) {
            ImsLog.i("ImsMediaManager - close");
            sImsMediaManager.release();
            sImsMediaManager = null;
            sExecutor = null;
            sMediaObserver = null;
            setImsMediaConnected(false);
        }

        if (sMediaHandlerThread != null) {
            sMediaHandlerThread.quitSafely();
            sMediaHandlerThread = null;
        }
    }

    /**
     * Opens a RTP session based on the local sockets with the associated initial remote
     * configuration if there is a valid {@link RtpConfig} passed. It starts the media flow if the
     * media direction in the {@link RtpConfig} is set to any value other than {@link
     * RtpConfig#NO_MEDIA_FLOW}. If the open session is successful then a new {@link
     * ImsMediaSession} object will be returned using the {@link
     * SessionCallback#onOpenSessionSuccess(ImsMediaSession)} API. If the open session is failed
     * then an error code {@link SessionOperationResult} will be returned using {@link
     * SessionCallback#onOpenSessionFailure(int)} API.
     *
     * @param rtpSocket local UDP socket to send and receive incoming RTP packets
     * @param rtcpSocket local UDP socket to send and receive incoming RTCP packets
     * @param rtpConfig provides remote endpoint info and codec details. This could be null
     *     initially and the application may update this later using {@link
     *     ImsMediaSession#modifySession()} API.
     * @param callback callbacks to receive session specific notifications.
     */
    public void openSession(
            @NonNull final DatagramSocket rtpSocket,
            @NonNull final DatagramSocket rtcpSocket,
            @NonNull final @ImsMediaSession.SessionType int sessionType,
            @Nullable final RtpConfig rtpConfig,
            @NonNull final ImsMediaManager.SessionCallback callback) {
        getImsMediaManagerInstance()
                .openSession(
                        rtpSocket, rtcpSocket, sessionType, rtpConfig, getExecutor(), callback);
    }

    /**
     * Closes the RTP session including cleanup of all the resources associated with the session.
     * This will also close the session object and associated callback.
     *
     * @param session RTP session to be closed.
     */
    public void closeSession(@NonNull final ImsMediaSession session) {
        getImsMediaManagerInstance().closeSession(session);
    }

    /**
     * Return the Looper for the media Handler thread.
     *
     * @return The media main looper.
     */
    Looper getMediaLooper() {
        return sMediaHandlerThread.getLooper();
    }

    /**
     * Handle ImsMedia onConnected callback
     */
    private void handleConnected() {
        if (!isImsMediaConnected()) {
            setImsMediaConnected(true);
            if (sMediaObserver != null) {
                sMediaObserver.onMediaConnected();
            }
        }
    }

    /**
     * Handle ImsMedia onDisconnected callback
     */
    private void handleDisconnected() {
        if (isImsMediaConnected()) {
            if (sMediaObserver != null) {
                sMediaObserver.onMediaDisconnected();
            }
            close();
        }
    }

    /**
     * Implements Interface to receive callbacks when the ImsMediaManager is connected or
     * disconnected.
     */
    private class ImsMediaManagerCallback implements ImsMediaManager.OnConnectedCallback {

        @Override
        public void onConnected() {
            ImsLog.i("ImsMediaManager - connected");
            handleConnected();
        }

        @Override
        public void onDisconnected() {
            ImsLog.i("ImsMediaManager - disconnected");
            handleDisconnected();
        }
    }
}
