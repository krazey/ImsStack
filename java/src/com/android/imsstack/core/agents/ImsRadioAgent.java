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

import android.annotation.NonNull;
import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.telephony.AccessNetworkConstants;
import android.telephony.BarringInfo;
import android.telephony.BarringInfo.BarringServiceInfo;
import android.telephony.TelephonyCallback;
import android.telephony.ims.feature.ConnectionFailureInfo;
import android.telephony.ims.feature.ImsTrafficSessionCallback;
import android.telephony.ims.feature.MmTelFeature;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.Executor;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * This provides the implementation of the ims radio interface for notifying the IMS traffic
 * activities to modem, getting NAS/RRC connection setup result details from modem and
 * triggering EPS fallback to modem.
 */
public class ImsRadioAgent implements ImsRadioInterface {
    private final int mSlotId;
    private AtomicInteger mIdGenerator = new AtomicInteger(ID_MIN);
    private Handler mHandler;
    private ImsRadioPhoneStateListener mPhoneStateListener;
    private ImsTrafficListener mImsTrafficListener;
    private ImsSccsListener mImsSccsListener;
    private SsacInfo mSsacInfo;
    private Map<Integer, ConnectionListener> mConnectionListeners =
                new HashMap<Integer, ConnectionListener>();
    private Map<Integer, TrafficCallback> mTrafficCallbacks =
                new HashMap<Integer, TrafficCallback>();
    private NativeStateInterface.Listener mNativeStateListener;
    private Set<TrafficPriorityListener> mPriorityListeners =
                new CopyOnWriteArraySet<TrafficPriorityListener>();

    private static final int EVENT_CONNECTION_FAILED = 1;
    private static final int EVENT_CONNECTION_SETUP_PREPARED = 2;
    private static final int EVENT_SSAC_STATE_CHANGED = 3;
    @VisibleForTesting
    protected static final int EVENT_SIMULTANEOUS_CALLING_SUPPORT_CHANGED = 4;
    @VisibleForTesting
    protected boolean mSimulatedImsHalEnabled;

    private static final int ID_MIN = 1000000;
    private static final int ID_MAX = 1100000;

    public ImsRadioAgent(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        mHandler = new Handler(Looper.myLooper());
        mPhoneStateListener = new ImsRadioPhoneStateListener();
        mPhoneStateListener.setListener();
        mSsacInfo = new SsacInfo();

        NativeStateInterface nsi =
                AgentFactory.getInstance().getAgent(NativeStateInterface.class, mSlotId);
        if (nsi != null) {
            mNativeStateListener = new NativeStateInterface.Listener() {
                @Override
                public void onNativeServiceReady() {
                    ImsLog.i(this, mSlotId, "NativeState: service ready.");
                    notifySsacInfoToNative();
                }
            };
            nsi.addListener(mNativeStateListener);
        }

        mImsTrafficListener = new ImsTrafficListener();
        ImsTrafficInterface imsTraffic =
                AgentFactory.getInstance().getAgent(ImsTrafficInterface.class);
        if (imsTraffic != null) {
            imsTraffic.addListener(mImsTrafficListener);
        }

        int subId = MSimUtils.getSubId(mSlotId);
        if (MSimUtils.isValidSubId(subId)) {
            mImsSccsListener = new ImsSccsListener(subId);
            mImsSccsListener.register();
        }

        mSimulatedImsHalEnabled = ImsPrivateProperties.Persistent.getBoolean(
                ImsPrivateProperties.Persistent.KEY_TEST_SIMULATED_IMS_HAL, false, mSlotId);
    }

    @Override
    public void cleanup() {
        if (mNativeStateListener != null) {
            NativeStateInterface nsi =
                    AgentFactory.getInstance().getAgent(NativeStateInterface.class, mSlotId);
            if (nsi != null) {
                nsi.removeListener(mNativeStateListener);
            }
            mNativeStateListener = null;
        }

        if (mImsTrafficListener != null) {
            ImsTrafficInterface imsTraffic =
                    AgentFactory.getInstance().getAgent(ImsTrafficInterface.class);
            if (imsTraffic != null) {
                imsTraffic.removeListener(mImsTrafficListener);
            }
            mImsTrafficListener = null;
        }

        if (mImsSccsListener != null) {
            mImsSccsListener.unregister();
            mImsSccsListener = null;
        }

        mSsacInfo = null;
        mPhoneStateListener.dispose();
        mHandler.removeCallbacksAndMessages(null);

        mConnectionListeners.clear();
        mTrafficCallbacks.clear();
        mPriorityListeners.clear();
    }

    @Override
    public boolean isImsTrafficAllowed(int trafficType) {
        ImsTrafficInterface imsTraffic =
                AgentFactory.getInstance().getAgent(ImsTrafficInterface.class);
        return (imsTraffic != null) ? imsTraffic.isAllowed(trafficType, mSlotId) : true;
    }

    @Override
    public void startImsTraffic(int trafficType, int accessNetworkType, int direction,
            ConnectionListener listener) {
        int id = -1;

        for (Integer i : mConnectionListeners.keySet()) {
            if (mConnectionListeners.get(i) == listener) {
                id = i;
                break;
            }
        }

        if (id < 0) {
            id = getId();
            mConnectionListeners.put(id, listener);

            TrafficCallback tcb = new TrafficCallback(id);
            mTrafficCallbacks.put(id, tcb);

            invokeStartImsTrafficSession(convertTrafficType(trafficType),
                    convertAccessNetworkType(accessNetworkType),
                    convertTrafficDirection(direction), mHandler::post, tcb);

        } else {
            TrafficCallback tcb = mTrafficCallbacks.get(id);

            if (tcb != null) {
                invokeModifyImsTrafficSession(convertAccessNetworkType(accessNetworkType), tcb,
                        mHandler::post);
            }
        }

        ImsLog.d(this, mSlotId, "startImsTraffic - type=" + trafficType + ", network type="
                + accessNetworkType + ", id=" + id + ", size=" + mConnectionListeners.size());
    }

    @Override
    public void stopImsTraffic(ConnectionListener listener) {
        for (Integer i : mConnectionListeners.keySet()) {
            if (mConnectionListeners.get(i) == listener) {
                TrafficCallback tcb = mTrafficCallbacks.get(i);

                if (tcb != null) {
                    invokeStopImsTrafficSession(tcb);
                    mTrafficCallbacks.remove(i);
                }
                mConnectionListeners.remove(i);
                break;
            }
        }

        ImsLog.d(this, mSlotId, "stopImsTraffic - size=" + mConnectionListeners.size());
    }

    @Override
    public void addListenerForTrafficPriority(TrafficPriorityListener listener) {
        mPriorityListeners.add(listener);
    }

    @Override
    public void removeListenerForTrafficPriority(TrafficPriorityListener listener) {
        mPriorityListeners.remove(listener);
    }

    @Override
    public void startImsTraffic(int id, @TrafficType int trafficType,
            @AccessNetworkType int accessNetworkType, @Direction int direction) {
        ImsLog.d(this, mSlotId, "startImsTraffic - id=" + id + ", type=" + trafficType
                + ", network type=" + accessNetworkType + ", dir=" + direction);

        for (Integer i : mTrafficCallbacks.keySet()) {
            if (i == id) {
                TrafficCallback tcb = mTrafficCallbacks.get(i);

                if (tcb != null) {
                    invokeModifyImsTrafficSession(convertAccessNetworkType(accessNetworkType),
                            tcb, mHandler::post);
                }
                return;
            }
        }

        TrafficCallback tcb = new TrafficCallback(id);
        mTrafficCallbacks.put(id, tcb);

        invokeStartImsTrafficSession(convertTrafficType(trafficType),
                convertAccessNetworkType(accessNetworkType),
                convertTrafficDirection(direction), mHandler::post, tcb);
    }

    @Override
    public void stopImsTraffic(int id) {
        ImsLog.d(this, mSlotId, "stopImsTraffic - id=" + id);

        TrafficCallback tcb = mTrafficCallbacks.get(id);

        if (tcb != null) {
            invokeStopImsTrafficSession(tcb);
            mTrafficCallbacks.remove(id);
        }
    }

    @Override
    public boolean triggerEpsFallback(int reason) {
        ImsLog.d(this, mSlotId, "triggerEpsFallback - reason=" + reason);

        ImsServiceRegistry isr = ImsServiceRegistry.getInstance(mSlotId);
        MmTelFeature mtf = isr.getMmTelFeature();

        if (mtf != null) {
            try {
                mtf.triggerEpsFallback(reason);
                return true;
            } catch (IllegalStateException e) {
                ImsLog.e(this, mSlotId, "triggerEpsFallback: " + e.toString());
            }
        }

        return false;
    }

    private static IDcNetWatcher getDcNetWatcher(int slotId) {
        return DcFactory.getDcAgent(IDcNetWatcher.class, slotId);
    }

    private static int convertConnectionFailureReason(int reason) {
        switch (reason) {
            case ConnectionFailureInfo.REASON_ACCESS_DENIED:
                return REASON_ACCESS_DENIED;
            case ConnectionFailureInfo.REASON_NAS_FAILURE:
                return REASON_NAS_FAILURE;
            case ConnectionFailureInfo.REASON_RACH_FAILURE:
                return REASON_RACH_FAILURE;
            case ConnectionFailureInfo.REASON_RLC_FAILURE:
                return REASON_RLC_FAILURE;
            case ConnectionFailureInfo.REASON_RRC_REJECT:
                return REASON_RRC_REJECT;
            case ConnectionFailureInfo.REASON_RRC_TIMEOUT:
                return REASON_RRC_TIMEOUT;
            case ConnectionFailureInfo.REASON_NO_SERVICE:
                return REASON_NO_SERVICE;
            case ConnectionFailureInfo.REASON_PDN_NOT_AVAILABLE:
                return REASON_PDN_NOT_AVAILABLE;
            case ConnectionFailureInfo.REASON_RF_BUSY:
                return REASON_RF_BUSY;
            default:
                return REASON_INTERNAL_ERROR;
        }
    }

    private static int convertTrafficDirection(int direction) {
        if (direction == DIRECTION_MO) {
            return MmTelFeature.IMS_TRAFFIC_DIRECTION_OUTGOING;
        }

        return MmTelFeature.IMS_TRAFFIC_DIRECTION_INCOMING;
    }

    private static int convertTrafficType(int type) {
        switch (type) {
            case TRAFFIC_TYPE_EMERGENCY:
                return MmTelFeature.IMS_TRAFFIC_TYPE_EMERGENCY;
            case TRAFFIC_TYPE_EMERGENCY_SMS:
                return MmTelFeature.IMS_TRAFFIC_TYPE_EMERGENCY_SMS;
            case TRAFFIC_TYPE_VOICE:
                return MmTelFeature.IMS_TRAFFIC_TYPE_VOICE;
            case TRAFFIC_TYPE_VIDEO:
                return MmTelFeature.IMS_TRAFFIC_TYPE_VIDEO;
            case TRAFFIC_TYPE_SMS:
                return MmTelFeature.IMS_TRAFFIC_TYPE_SMS;
            case TRAFFIC_TYPE_REGISTRATION:
                return MmTelFeature.IMS_TRAFFIC_TYPE_REGISTRATION;
            case TRAFFIC_TYPE_UT_XCAP:
                return MmTelFeature.IMS_TRAFFIC_TYPE_UT_XCAP;
            default:
                return MmTelFeature.IMS_TRAFFIC_TYPE_NONE;
        }
    }

    private int convertAccessNetworkType(int type) {
        if (type == ACCESS_NETWORK_TYPE_UNKNOWN) {
            IDcNetWatcher dcnw = getDcNetWatcher(mSlotId);

            if (dcnw != null) {
                if (dcnw.is4G()) {
                    return AccessNetworkConstants.AccessNetworkType.EUTRAN;
                }
                if (dcnw.is5G()) {
                    return AccessNetworkConstants.AccessNetworkType.NGRAN;
                }
                if (dcnw.is3G()) {
                    return AccessNetworkConstants.AccessNetworkType.UTRAN;
                }
            }
        }

        switch (type) {
            case ACCESS_NETWORK_TYPE_UTRAN:
                return AccessNetworkConstants.AccessNetworkType.UTRAN;
            case ACCESS_NETWORK_TYPE_EUTRAN:
                return AccessNetworkConstants.AccessNetworkType.EUTRAN;
            case ACCESS_NETWORK_TYPE_NGRAN:
                return AccessNetworkConstants.AccessNetworkType.NGRAN;
            case ACCESS_NETWORK_TYPE_IWLAN:
                return AccessNetworkConstants.AccessNetworkType.IWLAN;
            default:
                return AccessNetworkConstants.AccessNetworkType.UNKNOWN;
        }
    }

    private int getId() {
        int id = mIdGenerator.incrementAndGet();

        if (id > ID_MAX) {
            mIdGenerator.set(ID_MIN);
            return ID_MIN;
        }

        return id;
    }

    private void handleBarringInfo(BarringInfo barringInfo) {
        SsacInfo ssacInfo = new SsacInfo();

        BarringServiceInfo mmtelVoice = barringInfo.getBarringServiceInfo(
                BarringInfo.BARRING_SERVICE_TYPE_MMTEL_VOICE);
        int voiceBarringFactor = mmtelVoice.getConditionalBarringFactor();
        int voiceBarringTime = mmtelVoice.getConditionalBarringTimeSeconds();

        if (mmtelVoice.isBarred() || (voiceBarringFactor > 0 && voiceBarringFactor < 100)) {
            ssacInfo.setForVoice(voiceBarringFactor, voiceBarringTime);
        }

        BarringServiceInfo mmtelVideo = barringInfo.getBarringServiceInfo(
                BarringInfo.BARRING_SERVICE_TYPE_MMTEL_VIDEO);
        int videoBarringFactor = mmtelVideo.getConditionalBarringFactor();
        int videoBarringTime = mmtelVideo.getConditionalBarringTimeSeconds();

        if (mmtelVideo.isBarred() || (videoBarringFactor > 0 && videoBarringFactor < 100)) {
            ssacInfo.setForVideo(videoBarringFactor, videoBarringTime);
        }

        ImsLog.d(this, mSlotId,
                "voice - factor=" + voiceBarringFactor + ", time=" + voiceBarringTime
                + " : video - factor=" + videoBarringFactor + ", time=" + videoBarringTime);

        notifySsacInfo(ssacInfo);
    }

    private void handleTrafficCallbackOnError(
            int id, int reason, int causeCode, int waitTimeMillis) {
        ImsLog.d(this, mSlotId, "handleTrafficCallbackOnError - id=" + id + ", reason=" + reason
                + " , cause=" + causeCode + " , timeM=" + waitTimeMillis);

        if (reason == REASON_INTERNAL_ERROR) {
            handleTrafficCallbackOnReady(id);
            return;
        }

        if (waitTimeMillis < 0) {
            waitTimeMillis = 0;
        }

        if (id >= ID_MIN) {
            ConnectionListener listener = mConnectionListeners.get(id);

            if (listener != null) {
                listener.onConnectionFailed(reason, causeCode, waitTimeMillis);
            }
        } else {
            ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

            if (system != null) {
                system.notifyRadioConnectionFailed(EVENT_CONNECTION_FAILED, id, reason, causeCode,
                        waitTimeMillis);
            }
        }
    }

    private void handleTrafficCallbackOnReady(int id) {
        ImsLog.d(this, mSlotId, "handleTrafficCallbackOnReady - id=" + id);

        if (id >= ID_MIN) {
            ConnectionListener listener = mConnectionListeners.get(id);

            if (listener != null) {
                listener.onConnectionSetupPrepared();
            }
        } else {
            ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

            if (system != null) {
                system.notifyRadioConnectionSetupPrepared(EVENT_CONNECTION_SETUP_PREPARED, id);
            }
        }
    }

    private void invokeModifyImsTrafficSession(int accessNetworkType,
            @NonNull ImsTrafficSessionCallback callback, @NonNull Executor executor) {
        if (mSimulatedImsHalEnabled) {
            executor.execute(() -> callback.onReady());
            return;
        }

        ImsServiceRegistry isr = ImsServiceRegistry.getInstance(mSlotId);
        MmTelFeature mtf = isr.getMmTelFeature();

        if (mtf != null) {
            try {
                mtf.modifyImsTrafficSession(accessNetworkType, callback);
            } catch (IllegalStateException e) {
                ImsLog.e(this, mSlotId, "modifyImsTrafficSession: " + e.toString());
                return;
            }
        }
    }

    private void invokeStartImsTrafficSession(int trafficType, int accessNetworkType,
            int direction, @NonNull Executor executor,
            @NonNull ImsTrafficSessionCallback callback) {
        if (mSimulatedImsHalEnabled) {
            executor.execute(() -> callback.onReady());
            return;
        }

        ImsServiceRegistry isr = ImsServiceRegistry.getInstance(mSlotId);
        MmTelFeature mtf = isr.getMmTelFeature();

        if (mtf != null) {
            try {
                mtf.startImsTrafficSession(trafficType, accessNetworkType, direction, executor,
                        callback);
            } catch (IllegalStateException e) {
                ImsLog.e(this, mSlotId, "startImsTrafficSession: " + e.toString());
                return;
            }
        }
    }

    private void invokeStopImsTrafficSession(@NonNull ImsTrafficSessionCallback callback) {
        if (mSimulatedImsHalEnabled) {
            return;
        }

        ImsServiceRegistry isr = ImsServiceRegistry.getInstance(mSlotId);
        MmTelFeature mtf = isr.getMmTelFeature();

        if (mtf != null) {
            try {
                mtf.stopImsTrafficSession(callback);
            } catch (IllegalStateException e) {
                ImsLog.e(this, mSlotId, "stopImsTrafficSession: " + e.toString());
                return;
            }
        }
    }

    private void invokeTrafficPriorityListener() {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                for (TrafficPriorityListener l : mPriorityListeners) {
                    l.onTrafficPriorityChanged();
                }
            }
        });
    }

    private void notifySsacInfo(SsacInfo ssacInfo) {
        if (mSsacInfo.isChanged(ssacInfo)) {
            mSsacInfo.update(ssacInfo);
            notifySsacInfoToNative();
        }
    }

    private void notifySsacInfoToNative() {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

                if (system != null) {
                    system.notifySsacInfo(EVENT_SSAC_STATE_CHANGED,
                            mSsacInfo.mBarringFactorForVoice,
                            mSsacInfo.mBarringTimeSecForVoice,
                            mSsacInfo.mBarringFactorForVideo,
                            mSsacInfo.mBarringTimeSecForVideo);
                }
            }
        });
    }

    private final class ImsRadioPhoneStateListener implements ImsPhoneStateListener {
        private IPhoneStateNotifier mNotifier;
        private PhoneStateInterface mPhoneState;

        ImsRadioPhoneStateListener() {
        }

        public void dispose() {
            if (mNotifier != null) {
                if (mPhoneState != null) {
                    mPhoneState.removeNotifier(mNotifier);
                }

                mNotifier.setListener(null);
                mNotifier = null;
            }
        }

        public void setListener() {
            mPhoneState = AgentFactory.getInstance().getAgent(PhoneStateInterface.class, mSlotId);

            if (mPhoneState != null) {
                mNotifier = mPhoneState.createNotifier(this, Looper.myLooper());
                mNotifier.setEvents(LISTEN_BARRING_INFO);

                mPhoneState.addNotifier(mNotifier);
            }
        }

        /**
         * Invokes when barring information is changed.
         */
        @Override
        public void onBarringInfoChanged(BarringInfo barringInfo) {
            handleBarringInfo(barringInfo);
        }
    }

    private final class SsacInfo {
        private int mBarringFactorForVoice = 100;
        private int mBarringTimeSecForVoice = 0;
        private int mBarringFactorForVideo = 100;
        private int mBarringTimeSecForVideo = 0;

        SsacInfo() {
        }

        public boolean isChanged(SsacInfo ssacInfo) {
            if (mBarringFactorForVoice != ssacInfo.mBarringFactorForVoice
                    || mBarringTimeSecForVoice != ssacInfo.mBarringTimeSecForVoice
                    || mBarringFactorForVideo != ssacInfo.mBarringFactorForVideo
                    || mBarringTimeSecForVideo != ssacInfo.mBarringTimeSecForVideo) {
                return true;
            }

            return false;
        }

        public void setForVideo(int factor, int timeSec) {
            mBarringFactorForVideo = factor;
            mBarringTimeSecForVideo = timeSec;
        }

        public void setForVoice(int factor, int timeSec) {
            mBarringFactorForVoice = factor;
            mBarringTimeSecForVoice = timeSec;
        }

        public void update(SsacInfo ssacInfo) {
            mBarringFactorForVoice = ssacInfo.mBarringFactorForVoice;
            mBarringTimeSecForVoice = ssacInfo.mBarringTimeSecForVoice;
            mBarringFactorForVideo = ssacInfo.mBarringFactorForVideo;
            mBarringTimeSecForVideo = ssacInfo.mBarringTimeSecForVideo;
            ImsLog.d(this, mSlotId, "ssac-update: voicefactor=" + mBarringFactorForVoice
                    + ", timeSec=" + mBarringTimeSecForVoice
                    + ", videofactor=" + mBarringFactorForVideo
                    + ", timeSec=" + mBarringTimeSecForVideo);
        }
    }

    private final class TrafficCallback implements ImsTrafficSessionCallback {
        private final int mId;

        TrafficCallback(int id) {
            mId = id;
        }

        @Override
        public void onReady() {
            handleTrafficCallbackOnReady(mId);
        }

        @Override
        public void onError(@NonNull ConnectionFailureInfo info) {
            handleTrafficCallbackOnError(mId, convertConnectionFailureReason(info.getReason()),
                    info.getCauseCode(), info.getWaitTimeMillis());
        }
    }

    private final class ImsTrafficListener implements ImsTrafficInterface.PriorityListener {
        @Override
        public void onTrafficPriorityChanged() {
            invokeTrafficPriorityListener();
        }
    }

    private final class ImsSccsListener extends TelephonyCallback implements
            TelephonyCallback.SimultaneousCellularCallingSupportListener {
        private final int mSubId;
        private boolean mIsSimultaneousCallingSupported = false;

        ImsSccsListener(int subId) {
            mSubId = subId;
        }

        public void register() {
            TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(mSubId);
            tmp.registerTelephonyCallback(mHandler::post, this);
        }

        public void unregister() {
            TelephonyManagerProxy tmp = AppContext.getTelephonyManagerProxy(mSubId);
            tmp.unregisterTelephonyCallback(this);
        }

        @Override
        public void onSimultaneousCellularCallingSubscriptionsChanged(
                @NonNull Set<Integer> simultaneousCallingSubscriptionIds) {
            boolean isSupported = simultaneousCallingSubscriptionIds.contains(mSubId);
            if (isSupported == mIsSimultaneousCallingSupported) {
                return;
            }

            ImsLog.d(this, mSlotId,
                    "Simultaneous calling support for SubId[" + mSubId + "]=" + isSupported);

            mIsSimultaneousCallingSupported = isSupported;

            // notifying to ImsTrafficAgent
            ImsTrafficInterface imsTraffic =
                    AgentFactory.getInstance().getAgent(ImsTrafficInterface.class);
            if (imsTraffic != null) {
                imsTraffic.setSimultaneousCallingSupported(isSupported, mSlotId);
            }

            // notifying to native
            ISystem system = SystemInterface.getInstance().getSystem(mSlotId);
            if (system != null) {
                system.notifySimultaneousCallingSupportChanged(
                        EVENT_SIMULTANEOUS_CALLING_SUPPORT_CHANGED, isSupported);
            }
        }
    }
}
