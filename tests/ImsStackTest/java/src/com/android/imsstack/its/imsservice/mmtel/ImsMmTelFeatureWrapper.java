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
package com.android.imsstack.its.imsservice.mmtel;

import android.annotation.CallbackExecutor;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.telephony.AccessNetworkConstants;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.MediaQualityStatus;
import android.telephony.ims.MediaThreshold;
import android.telephony.ims.RtpHeaderExtensionType;
import android.telephony.ims.aidl.IImsCallSessionListener;
import android.telephony.ims.aidl.IImsCapabilityCallback;
import android.telephony.ims.aidl.IImsMmTelFeature;
import android.telephony.ims.aidl.IImsTrafficSessionCallback;
import android.telephony.ims.aidl.ISrvccStartedCallback;
import android.telephony.ims.feature.CapabilityChangeRequest;
import android.telephony.ims.feature.ConnectionFailureInfo;
import android.telephony.ims.feature.ImsFeature;
import android.telephony.ims.feature.MmTelFeature;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.ims.internal.IImsCallSession;
import com.android.ims.internal.IImsUt;
import com.android.imsstack.its.imsservice.mmtel.call.ImsCallSessionWrapper;
import com.android.imsstack.util.Log;

import java.util.List;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.Executor;

/**
 * IMS MMTEL feature interface wrapper.
 */
public final class ImsMmTelFeatureWrapper {
    private IImsMmTelFeature mIImsMmTelFeature;
    private IncomingCallListener mIncomingCallListener;
    private final Set<MmTelListener> mMmTelListeners = new CopyOnWriteArraySet<MmTelListener>();
    private final Handler mHandler;

    public interface IncomingCallListener {
        /**
         * Notifies an incoming call.
         */
        @Nullable IImsCallSessionListener onIncomingCall(
                @NonNull IImsCallSession c, @Nullable String callId, @Nullable Bundle extras);
    }

    interface MmTelListener {
        /**
         * Notifies the request for EPS fallback.
         */
        default void onTriggerEpsFallback(
                @MmTelFeature.EpsFallbackReason int reason) {
        }

        /**
         * Notifies the start for IMS traffic session.
         */
        default void onStartImsTrafficSession(int token,
                @MmTelFeature.ImsTrafficType int trafficType,
                @AccessNetworkConstants.RadioAccessNetworkType int accessNetworkType,
                @MmTelFeature.ImsTrafficDirection int trafficDirection,
                IImsTrafficSessionCallback callback) {
        }

        /**
         * Notifies the modification for IMS traffic session.
         */
        default void onModifyImsTrafficSession(int token,
                @AccessNetworkConstants.RadioAccessNetworkType int accessNetworkType) {
        }

        /**
         * Notifies the stop for IMS traffic session.
         */
        default void onStopImsTrafficSession(int token) {
        }
    }

    /** Constructor. */
    public ImsMmTelFeatureWrapper(@NonNull IImsMmTelFeature mmtelFeature) {
        mIImsMmTelFeature = mmtelFeature;
        mHandler = new Handler(Looper.myLooper());

        try {
            setMmTelFeatureListener(new MmTelFeatureListener());
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /** Destroy. */
    public void destroy() {
        logi("destroy");

        mHandler.removeCallbacksAndMessages(null);
        mIImsMmTelFeature = null;
    }

    /** Sets the listener for getting incoming call. */
    public void setIncomingCallListener(@Nullable IncomingCallListener listener) {
        mIncomingCallListener = listener;
    }

    /** Adds MMTEL feature listener. */
    public void addMmTelListener(@NonNull MmTelListener listener) {
        mMmTelListeners.add(listener);
    }

    /** Removes MMTEL feature listener. */
    public void removeMmTelListener(@NonNull MmTelListener listener) {
        mMmTelListeners.remove(listener);
    }

    /**
     * @return The current state of the ImsFeature.
     */
    public int getFeatureState() {
        try {
            return mIImsMmTelFeature.getFeatureState();
        } catch (RemoteException e) {
            loge(e.toString());
            return ImsFeature.STATE_UNAVAILABLE;
        } catch (NullPointerException e) {
            loge(e.toString());
            return ImsFeature.STATE_UNAVAILABLE;
        }
    }

    /**
     * Creates a {@link ImsCallProfile} from the service capabilities & IMS registration state.
     *
     * @param serviceType a service type that is specified in {@link ImsCallProfile}.
     * @param callType a call type that is specified in {@link ImsCallProfile}.
     * @return a {@link ImsCallProfile} object.
     */
    public @Nullable ImsCallProfile createCallProfile(int serviceType, int callType) {
        try {
            return mIImsMmTelFeature.createCallProfile(serviceType, callType);
        } catch (RemoteException e) {
            loge(e.toString());
            return null;
        } catch (NullPointerException e) {
            loge(e.toString());
            return null;
        }
    }

    /**
     * Creates an {@link ImsCallSessionWrapper} with the specified call profile.
     *
     * @param profile a call profile to make the call.
     */
    public @Nullable ImsCallSessionWrapper createCallSession(@NonNull ImsCallProfile profile,
            @NonNull IImsCallSessionListener listener) {
        IImsCallSession callSession = createImsCallSession(profile);
        return (callSession != null) ? new ImsCallSessionWrapper(callSession, listener) : null;
    }

    /**
     * For reporting a change to the RTP header extension types which should be offered
     * during SDP negotiation (see RFC8285 for more information).
     *
     * @param types The RTP header extensions to offer during outgoing and incoming call setup.
     */
    public void changeOfferedRtpHeaderExtensionTypes(@NonNull List<RtpHeaderExtensionType> types) {
        try {
            mIImsMmTelFeature.changeOfferedRtpHeaderExtensionTypes(types);
        } catch (RemoteException e) {
            loge(e.toString());
        } catch (NullPointerException e) {
            loge(e.toString());
        }
    }

    /**
     * For determining if the outgoing call, designated by the outgoing {@link String}s,
     * should be processed as an IMS call or CSFB call.
     *
     * @param numbers An array of {@link String}s that will be used for placing the call. There can
     *         be multiple {@link String}s listed in the case when we want to place an outgoing
     *         call as a conference.
     * @return a {@link ProcessCallResult} to the framework, which will be used to determine if the
     *        call will be placed over IMS or via CSFB.
     */
    public int shouldProcessCall(@NonNull String[] numbers) {
        try {
            return handleShouldProcessCall(numbers);
        } catch (RemoteException e) {
            loge(e.toString());
            return MmTelFeature.PROCESS_CALL_IMS;
        }
    }

    /**
     * @return The Ut interface for the supplementary service configuration.
     */
    public IImsUt getUtInterface() {
        try {
            return mIImsMmTelFeature.getUtInterface();
        } catch (RemoteException e) {
            loge(e.toString());
            return null;
        } catch (NullPointerException e) {
            loge(e.toString());
            return null;
        }
    }

    /**
     * Sets the current UI TTY mode for the MmTelFeature.
     *
     * @param uiTtyMode An integer containing the new UI TTY Mode.
     * @param onCompleteMessage If non-null, this MmTelFeature should call this {@link Message} when
     *         the operation is complete by using the associated {@link android.os.Messenger} in
     *         {@link Message#replyTo}.
     */
    public void setUiTtyMode(int uiTtyMode, @Nullable Message onCompleteMessage) {
        try {
            mIImsMmTelFeature.setUiTtyMode(uiTtyMode, onCompleteMessage);
        } catch (RemoteException e) {
            loge(e.toString());
        } catch (NullPointerException e) {
            loge(e.toString());
        }
    }

    /**
     * The current capability status that this MmTelFeature has defined is available. This
     * configuration will be used by the platform to figure out which capabilities are CURRENTLY
     * available to be used.
     *
     * @return The current MmTelFeature capability status.
     */
    public int queryCapabilityStatus() {
        try {
            return handleQueryCapabilityStatus();
        } catch (RemoteException e) {
            loge(e.toString());
            return 0;
        }
    }

    /**
     * Notifies the MmTelFeature of the enablement status of terminal based call waiting.
     *
     * If the terminal based call waiting is provisioned,
     * IMS controls the enablement of terminal based call waiting which is defined
     * in 3GPP TS 24.615.
     *
     * @param enabled user setting controlling whether or not call waiting is enabled.
     */
    public void setTerminalBasedCallWaitingStatus(boolean enabled) {
        try {
            mIImsMmTelFeature.setTerminalBasedCallWaitingStatus(enabled);
        } catch (RemoteException e) {
            loge(e.toString());
        } catch (NullPointerException e) {
            loge(e.toString());
        }
    }

    /**
     * Adds the callback for getting the capability change.
     *
     * @param c Sees ImsFeature#CapabilityCallback for more information.
     */
    public void addCapabilityCallback(@NonNull IImsCapabilityCallback c) {
        try {
            handleAddCapabilityCallback(c);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Removes the callback for getting the capability change.
     *
     * @param c Sees ImsFeature#CapabilityCallback for more information.
     */
    public void removeCapabilityCallback(@NonNull IImsCapabilityCallback c) {
        try {
            handleRemoveCapabilityCallback(c);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Called internally to request the change of enabled capabilities.
     *
     * @param request Used by the framework to enable and disable MMTEL and RCS capabilities.
     * Sees MmTelFeature#changeEnabledCapabilities.
     * @param c Sees ImsFeature#CapabilityCallback for more information.
     */
    public void changeCapabilitiesConfiguration(@NonNull CapabilityChangeRequest request,
            @NonNull IImsCapabilityCallback c) {
        try {
            handleChangeCapabilitiesConfiguration(request, c);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Queries whether configuration is enabled or disabled for given capability and
     * radio technology.
     */
    public void queryCapabilityConfiguration(int capability, int radioTech,
            @NonNull IImsCapabilityCallback c) {
        try {
            handleQueryCapabilityConfiguration(capability, radioTech, c);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Notifies the MmTelFeature that the network has initiated an SRVCC (Single radio voice
     * call continuity) for all IMS calls.
     */
    public void notifySrvccStarted(@NonNull ISrvccStartedCallback c) {
        try {
            handleNotifySrvccStarted(c);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Notifies the MmTelFeature that the SRVCC is completed and the calls have been moved
     * over to the circuit-switched domain.
     */
    public void notifySrvccCompleted() {
        try {
            handleNotifySrvccCompleted();
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Notifies the MmTelFeature that the SRVCC has failed.
     */
    public void notifySrvccFailed() {
        try {
            handleNotifySrvccFailed();
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Notifies the MmTelFeature that the SRVCC has been canceled.
     */
    public void notifySrvccCanceled() {
        try {
            handleNotifySrvccCanceled();
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Passes {@link MediaThreshold}.
     *
     * @param mediaSessionType media session type for this Threshold info.
     * @param mediaThreshold media threshold information.
     */
    public void setMediaQualityThreshold(int mediaSessionType,
            @Nullable MediaThreshold mediaThreshold) {
        try {
            handleSetMediaQualityThreshold(mediaSessionType, mediaThreshold);
        } catch (RemoteException e) {
            loge(e.toString());
        }
    }

    /**
     * Returns currently measured media quality status.
     *
     * @param sessionType media session type
     * @return Current media quality status. It could be null if media quality status is not
     *         measured yet or {@link MediaThreshold} was not set corresponding to the media session
     *         type.
     */
    public @Nullable MediaQualityStatus queryMediaQualityStatus(int sessionType) {
        try {
            return mIImsMmTelFeature.queryMediaQualityStatus(sessionType);
        } catch (RemoteException e) {
            loge(e.toString());
            return null;
        } catch (NullPointerException e) {
            loge(e.toString());
            return null;
        }
    }

    /**
     * Invokes the ready state for the IMS traffic session.
     */
    public void invokeOnReadyForImsTrafficSession(@NonNull @CallbackExecutor Executor executor,
            @NonNull IImsTrafficSessionCallback c) {
        executor.execute(() -> {
            try {
                c.onReady();
            } catch (RemoteException e) {
                loge(e.toString());
            }
        });
    }

    /**
     * Invokes the error state for the IMS traffic session.
     */
    public void invokeOnErrorForImsTrafficSession(@NonNull @CallbackExecutor Executor executor,
            @NonNull IImsTrafficSessionCallback c, @NonNull ConnectionFailureInfo info) {
        executor.execute(() -> {
            try {
                c.onError(info);
            } catch (RemoteException e) {
                loge(e.toString());
            }
        });
    }

    /**
     * Creates an {@link ImsCallSession} with the specified call profile.
     * Use other methods, if applicable, instead of interacting with
     * {@link ImsCallSession} directly.
     *
     * @param profile a call profile to make the call.
     */
    private @Nullable IImsCallSession createImsCallSession(@NonNull ImsCallProfile profile) {
        try {
            return mIImsMmTelFeature.createCallSession(profile);
        } catch (RemoteException e) {
            loge(e.toString());
            return null;
        } catch (NullPointerException e) {
            loge(e.toString());
            return null;
        }
    }

    private void handleAddCapabilityCallback(@NonNull IImsCapabilityCallback c)
            throws RemoteException {
        try {
            mIImsMmTelFeature.addCapabilityCallback(c);
        } catch (NullPointerException e) {
            loge(e.toString());
        }
    }

    private void handleRemoveCapabilityCallback(@NonNull IImsCapabilityCallback c)
            throws RemoteException {
        try {
            mIImsMmTelFeature.removeCapabilityCallback(c);
        } catch (NullPointerException e) {
            loge(e.toString());
        }
    }

    private void handleChangeCapabilitiesConfiguration(@NonNull CapabilityChangeRequest request,
            @NonNull IImsCapabilityCallback c) throws RemoteException {
        try {
            mIImsMmTelFeature.changeCapabilitiesConfiguration(request, c);
        } catch (NullPointerException e) {
            loge(e.toString());
        }
    }

    private int handleShouldProcessCall(@NonNull String[] numbers) throws RemoteException {
        try {
            return mIImsMmTelFeature.shouldProcessCall(numbers);
        } catch (NullPointerException e) {
            loge(e.toString());
            return MmTelFeature.PROCESS_CALL_IMS;
        }
    }

    private int handleQueryCapabilityStatus() throws RemoteException {
        try {
            return mIImsMmTelFeature.queryCapabilityStatus();
        } catch (NullPointerException e) {
            loge(e.toString());
            return 0;
        }
    }

    private void handleQueryCapabilityConfiguration(int capability, int radioTech,
            @NonNull IImsCapabilityCallback c) throws RemoteException {
        try {
            mIImsMmTelFeature.queryCapabilityConfiguration(capability, radioTech, c);
        } catch (NullPointerException e) {
            loge(e.toString());
        }
    }

    private void handleNotifySrvccStarted(@NonNull ISrvccStartedCallback c) throws RemoteException {
        try {
            mIImsMmTelFeature.notifySrvccStarted(c);
        } catch (NullPointerException e) {
            loge(e.toString());
        }
    }

    private void handleNotifySrvccCompleted() throws RemoteException {
        try {
            mIImsMmTelFeature.notifySrvccCompleted();
        } catch (NullPointerException e) {
            loge(e.toString());
        }
    }

    private void handleNotifySrvccFailed() throws RemoteException {
        try {
            mIImsMmTelFeature.notifySrvccFailed();
        } catch (NullPointerException e) {
            loge(e.toString());
        }
    }

    private void handleNotifySrvccCanceled() throws RemoteException {
        try {
            mIImsMmTelFeature.notifySrvccCanceled();
        } catch (NullPointerException e) {
            loge(e.toString());
        }
    }

    private void handleSetMediaQualityThreshold(int mediaSessionType,
            @Nullable MediaThreshold mediaThreshold) throws RemoteException {
        try {
            mIImsMmTelFeature.setMediaQualityThreshold(mediaSessionType, mediaThreshold);
        } catch (NullPointerException e) {
            loge(e.toString());
        }
    }

    private void invokeOnTriggerEpsFallback(@MmTelFeature.EpsFallbackReason int reason) {
        if (mMmTelListeners.isEmpty()) {
            return;
        }

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                for (MmTelListener l : mMmTelListeners) {
                    l.onTriggerEpsFallback(reason);
                }
            }
        });
    }

    private void invokeOnStartImsTrafficSession(int token,
            @MmTelFeature.ImsTrafficType int trafficType,
            @AccessNetworkConstants.RadioAccessNetworkType int accessNetworkType,
            @MmTelFeature.ImsTrafficDirection int trafficDirection,
            IImsTrafficSessionCallback callback) {
        if (mMmTelListeners.isEmpty()) {
            return;
        }

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                for (MmTelListener l : mMmTelListeners) {
                    l.onStartImsTrafficSession(token, trafficType, accessNetworkType,
                            trafficDirection, callback);
                }
            }
        });
    }

    private void invokeOnModifyImsTrafficSession(int token,
            @AccessNetworkConstants.RadioAccessNetworkType int accessNetworkType) {
        if (mMmTelListeners.isEmpty()) {
            return;
        }

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                for (MmTelListener l : mMmTelListeners) {
                    l.onModifyImsTrafficSession(token, accessNetworkType);
                }
            }
        });
    }

    private void invokeOnStopImsTrafficSession(int token) {
        if (mMmTelListeners.isEmpty()) {
            return;
        }

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                for (MmTelListener l : mMmTelListeners) {
                    l.onStopImsTrafficSession(token);
                }
            }
        });
    }

    private void setMmTelFeatureListener(@NonNull MmTelFeatureListener listener)
            throws RemoteException {
        try {
            mIImsMmTelFeature.setListener(listener);
        } catch (NullPointerException e) {
            loge(e.toString());
        }
    }

    private void logi(String s) {
        Log.i(this, s);
    }

    private void loge(String s) {
        Log.e(this, s);
    }

    public class MmTelFeatureListener extends MmTelFeature.Listener {
        @Override
        public @Nullable IImsCallSessionListener onIncomingCall(
                @NonNull IImsCallSession c, @Nullable String callId, @Nullable Bundle extras) {
            if (mIncomingCallListener != null) {
                return mIncomingCallListener.onIncomingCall(c, callId, extras);
            }

            return null;
        }

        @Override
        public void onTriggerEpsFallback(@MmTelFeature.EpsFallbackReason int reason) {
            invokeOnTriggerEpsFallback(reason);
        }

        @Override
        public void onStartImsTrafficSession(int token,
                @MmTelFeature.ImsTrafficType int trafficType,
                @AccessNetworkConstants.RadioAccessNetworkType int accessNetworkType,
                @MmTelFeature.ImsTrafficDirection int trafficDirection,
                IImsTrafficSessionCallback callback) {
            invokeOnStartImsTrafficSession(token, trafficType, accessNetworkType, trafficDirection,
                    callback);
        }

        @Override
        public void onModifyImsTrafficSession(int token,
                @AccessNetworkConstants.RadioAccessNetworkType int accessNetworkType) {
            invokeOnModifyImsTrafficSession(token, accessNetworkType);
        }

        @Override
        public void onStopImsTrafficSession(int token) {
            invokeOnStopImsTrafficSession(token);
        }
    }
}
