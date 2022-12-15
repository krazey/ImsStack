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
import android.telephony.BarringInfo;
import android.telephony.BarringInfo.BarringServiceInfo;

import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.system.IJNIUpCallEvt;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.JNIUpCallEvtManager;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.system.SystemRadioInterface;
import com.android.imsstack.util.ImsLog;

import java.util.HashMap;
import java.util.Map;
import java.util.concurrent.atomic.AtomicInteger;

/**
 * This provides the implementation of the ims radio interface for notifying the IMS traffic
 * activities to modem, getting NAS/RRC connection setup result details from modem and
 * triggering EPS fallback to modem.
 */
public class ImsRadioAgent implements ImsRadioInterface, SystemRadioInterface {
    private final int mSlotId;
    private AtomicInteger mIdGenerator = new AtomicInteger(ID_MIN);
    private ImsRadioHandler mHandler;
    private ImsRadioPhoneStateListener mPhoneStateListener;
    private SsacInfo mSsacInfo;
    private Map<Integer, ConnectionListener> mConnectionListeners =
                new HashMap<Integer, ConnectionListener>();

    //private static final int EVENT_CONNECTION_FAILED = 1;
    private static final int EVENT_CONNECTION_SETUP_PREPARED = 2;
    private static final int EVENT_SSAC_STATE_CHANGED = 3;

    private static final int EVENT_RAT_CHANGED = 101;
    private static final int EVENT_NATIVE_BOOT_COMPLETED = 102;

    private static final int ID_MIN = 1000000;
    private static final int ID_MAX = 1100000;

    public ImsRadioAgent(int slotId) {
        mSlotId = slotId;
    }

    /**
     * Set the system radio interface.
     *
     * @param systemRadio The system radio interface
     */
    public void setSystemRadioInterface(SystemRadioInterface systemRadio) {
        ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

        if (system != null) {
            system.setSystemRadioInterface(systemRadio);
        }
    }

    @Override
    public void init(Context context) {
        mHandler = new ImsRadioHandler(Looper.myLooper());
        mPhoneStateListener = new ImsRadioPhoneStateListener();
        mPhoneStateListener.setListener();
        mSsacInfo = new SsacInfo();
        setSystemRadioInterface(this);

        IJNIUpCallEvt juce = JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(mSlotId);
        if (juce != null) {
            juce.registerForNativeBootComplete(mHandler, EVENT_NATIVE_BOOT_COMPLETED, null);
        }
    }

    @Override
    public void cleanup() {
        IJNIUpCallEvt juce = JNIUpCallEvtManager.getInstance().getJNIUpCallEvt(mSlotId);
        if (juce != null) {
            juce.unregisterForNativeBootComplete(mHandler);
        }

        IDcNetWatcher dcnw = getDcNetWatcher(mSlotId);

        if (dcnw != null) {
            dcnw.unregisterForRatChanged(mHandler);
        }

        setSystemRadioInterface(null);
        mSsacInfo = null;
        mPhoneStateListener.dispose();
        mHandler.removeCallbacksAndMessages(null);
    }

    @Override
    public boolean isImsTrafficAllowed(int trafficType) {
        return true;
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
        }

        ImsLog.d(mSlotId, "startImsTraffic - type=" + trafficType + ", network type="
                + accessNetworkType + ", id=" + id + ", size=" + mConnectionListeners.size());

        final int key = id;

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                ConnectionListener listener = mConnectionListeners.get(key);

                if (listener != null) {
                    listener.onConnectionSetupPrepared();
                }
            }
        });
    }

    @Override
    public void stopImsTraffic(ConnectionListener listener) {
        for (Integer i : mConnectionListeners.keySet()) {
            if (mConnectionListeners.get(i) == listener) {
                mConnectionListeners.remove(i);
                break;
            }
        }

        ImsLog.d(mSlotId, "stopImsTraffic - size=" + mConnectionListeners.size());
    }

    @Override
    public void addListenerForTrafficPriority(TrafficPriorityListener listener) {
    }

    @Override
    public void removeListenerForTrafficPriority(TrafficPriorityListener listener) {
    }

    @Override
    public int startImsTraffic(int id, int trafficType, int accessNetworkType, int direction) {
        ImsLog.d(mSlotId, "startImsTraffic - id=" + id + ", type=" + trafficType
                + ", network type=" + accessNetworkType + ", dir=" + direction);

        mHandler.post(new Runnable() {
            @Override
            public void run() {
                ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

                if (system != null) {
                    system.notifyRadioConnectionSetupPrepared(EVENT_CONNECTION_SETUP_PREPARED, id);
                }
            }
        });

        return SystemRadioInterface.RESULT_OK;
    }

    @Override
    public void stopImsTraffic(int id) {
        ImsLog.d(mSlotId, "stopImsTraffic - id=" + id);
    }

    @Override
    public int triggerEpsFallback(int reason) {
        ImsLog.d(mSlotId, "triggerEpsFallback - reason=" + reason);
        return SystemRadioInterface.RESULT_OK;
    }

    private static IDcNetWatcher getDcNetWatcher(int slotId) {
        return (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, slotId);
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
        IDcNetWatcher dcnw = getDcNetWatcher(mSlotId);
        SsacInfo ssacInfo = new SsacInfo();

        if (dcnw != null && dcnw.is4G()) {
            BarringServiceInfo mmtelVoice = barringInfo.getBarringServiceInfo(
                    BarringInfo.BARRING_SERVICE_TYPE_MMTEL_VOICE);

            if (mmtelVoice.isBarred()) {
                ssacInfo.setForVoice(mmtelVoice.getConditionalBarringFactor(),
                        mmtelVoice.getConditionalBarringTimeSeconds());
            } else {
                ImsLog.d(mSlotId, "voice is allowed");
            }

            BarringServiceInfo mmtelVideo = barringInfo.getBarringServiceInfo(
                    BarringInfo.BARRING_SERVICE_TYPE_MMTEL_VIDEO);

            if (mmtelVideo.isBarred()) {
                ssacInfo.setForVideo(mmtelVideo.getConditionalBarringFactor(),
                        mmtelVideo.getConditionalBarringTimeSeconds());
            } else {
                ImsLog.d(mSlotId, "video is allowed");
            }
        }

        notifySsacInfo(ssacInfo);
    }

    private void handleNativeBootCompleted() {
        IDcNetWatcher dcnw = getDcNetWatcher(mSlotId);

        if (dcnw != null) {
            dcnw.registerForRatChanged(mHandler, EVENT_RAT_CHANGED, null);
        }

        notifySsacInfoToNative();
    }

    private void handleRatChanged() {
        IDcNetWatcher dcnw = getDcNetWatcher(mSlotId);

        if (dcnw != null && !dcnw.is4G()) {
            notifySsacInfo(new SsacInfo());
        }
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

    private final class ImsRadioPhoneStateListener extends ImsPhoneStateListener {
        private IPhoneStateNotifier mNotifier = null;
        private IPhoneState mIps = null;

        ImsRadioPhoneStateListener() {
        }

        public void dispose() {
            if (mNotifier != null) {
                if (mIps != null) {
                    mIps.removeNotifier(mNotifier);
                }

                mNotifier.setListener(null);
                mNotifier = null;
            }
        }

        public void setListener() {
            mIps = (IPhoneState) AgentFactory.getAgent(
                    AgentFactory.PHONE_STATE, mSlotId);

            if (mIps != null) {
                mNotifier = mIps.createNotifier(this, Looper.myLooper());
                mNotifier.setEvents(LISTEN_BARRING_INFO);

                mIps.addNotifier(mNotifier);
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
            ImsLog.d(mSlotId, "setForVideo :: factor=" + factor + ", timeSec=" + timeSec);
            mBarringFactorForVideo = factor;
            mBarringTimeSecForVideo = timeSec;
        }

        public void setForVoice(int factor, int timeSec) {
            ImsLog.d(mSlotId, "setForVoice :: factor=" + factor + ", timeSec=" + timeSec);
            mBarringFactorForVoice = factor;
            mBarringTimeSecForVoice = timeSec;
        }

        public void update(SsacInfo ssacInfo) {
            mBarringFactorForVoice = ssacInfo.mBarringFactorForVoice;
            mBarringTimeSecForVoice = ssacInfo.mBarringTimeSecForVoice;
            mBarringFactorForVideo = ssacInfo.mBarringFactorForVideo;
            mBarringTimeSecForVideo = ssacInfo.mBarringTimeSecForVideo;
            ImsLog.d(mSlotId, "update :: voicefactor=" + mBarringFactorForVoice + ", timeSec="
                    + mBarringTimeSecForVoice + ", videofactor=" + mBarringFactorForVideo
                    + ", timeSec=" + mBarringTimeSecForVideo);
        }
    }

    private final class ImsRadioHandler extends Handler {
        ImsRadioHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            if (msg == null) {
                return;
            }

            ImsLog.d(mSlotId, "ImsRadioHandler :: msg=" + msg.what);

            switch (msg.what) {
                case EVENT_NATIVE_BOOT_COMPLETED:
                    handleNativeBootCompleted();
                    break;
                case EVENT_RAT_CHANGED:
                    handleRatChanged();
                    break;

                default:
                    break;
            }
        }
    }
}
