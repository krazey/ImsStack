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

import com.android.imsstack.system.ISystem;
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
    private Map<Integer, ConnectionListener> mConnectionListeners =
                new HashMap<Integer, ConnectionListener>();

    //private static final int EVENT_CONNECTION_FAILED = 1;
    private static final int EVENT_CONNECTION_SETUP_PREPARED = 2;
    //private static final int EVENT_SSAC_STATE_CHANGED = 3;

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
        setSystemRadioInterface(this);
    }

    @Override
    public void cleanup() {
        setSystemRadioInterface(null);
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

    private int getId() {
        int id = mIdGenerator.incrementAndGet();

        if (id > ID_MAX) {
            mIdGenerator.set(ID_MIN);
            return ID_MIN;
        }

        return id;
    }

    private static final class ImsRadioHandler extends Handler {
        ImsRadioHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
        }
    }
}
