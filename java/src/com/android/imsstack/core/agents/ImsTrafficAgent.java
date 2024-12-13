/*
 * Copyright (C) 2023 The Android Open Source Project
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
import android.util.SparseArray;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.util.ImsLog;

import java.util.Map;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * This provides the implementation of the IMS traffic interface for checking if IMS traffic can be
 * sent or not for DSDS
 */
public class ImsTrafficAgent implements ImsTrafficInterface {
    public static final int TRAFFIC_PRIORITY_NONE = 0x0;
    public static final int TRAFFIC_PRIORITY_REGISTRATION = 0x1;
    public static final int TRAFFIC_PRIORITY_SMS = 0x2;
    public static final int TRAFFIC_PRIORITY_VIDEO = 0x4;
    public static final int TRAFFIC_PRIORITY_VOICE = 0x8;
    public static final int TRAFFIC_PRIORITY_EMERGENCY_SMS = 0x10;
    public static final int TRAFFIC_PRIORITY_EMERGENCY = 0x20;

    private static final Map<Integer, String> PRIORITY_TYPE_TO_STRING = Map.ofEntries(
            Map.entry(TRAFFIC_PRIORITY_NONE, "TRAFFIC_PRIORITY_NONE"),
            Map.entry(TRAFFIC_PRIORITY_REGISTRATION, "TRAFFIC_PRIORITY_REGISTRATION"),
            Map.entry(TRAFFIC_PRIORITY_SMS, "TRAFFIC_PRIORITY_SMS"),
            Map.entry(TRAFFIC_PRIORITY_VIDEO, "TRAFFIC_PRIORITY_VIDEO"),
            Map.entry(TRAFFIC_PRIORITY_VOICE, "TRAFFIC_PRIORITY_VOICE"),
            Map.entry(TRAFFIC_PRIORITY_EMERGENCY_SMS, "TRAFFIC_PRIORITY_EMERGENCY_SMS"),
            Map.entry(TRAFFIC_PRIORITY_EMERGENCY, "TRAFFIC_PRIORITY_EMERGENCY"));

    private Handler mHandler;

    private final SparseArray<Traffic> mTraffics;
    private final Set<PriorityListener> mPriorityListeners =
            new CopyOnWriteArraySet<PriorityListener>();

    public ImsTrafficAgent() {
        int supportedSimCount = DeviceConfig.getSupportedSimCount();
        mTraffics = new SparseArray<>(supportedSimCount);

        for (int i = 0; i < supportedSimCount; i++) {
            mTraffics.put(i, new Traffic());
        }
    }

    @Override
    public void init(Context context) {
        mHandler = new Handler(AppContext.getInstance().getMainLooper());
    }

    @Override
    public void cleanup() {
        mHandler.removeCallbacksAndMessages(null);
        mPriorityListeners.clear();
    }

    @Override
    public boolean isAllowed(int trafficType, int slotId) {
        synchronized (mTraffics) {
            if (isSimultaneousCallingSupported(slotId)) {
                return true;
            }

            if (isEmergency(slotId)) {
                ImsLog.d(slotId, "emergency is ongoing");
                return true;
            }

            if (isEmergencyInOtherSlot(slotId)) {
                ImsLog.d(slotId, "emergency is ongoing in other slot");
                return false;
            }

            if (isWlan(slotId) || isWlanInOtherSlot(slotId)) {
                ImsLog.d(slotId, "wlan is enabled");
                return true;
            }

            if (isIdle() || getTopPrioritizedSlot() == slotId) {
                return true;
            }

            if (hasHighPriorityInOtherSlot(getPriorityType(trafficType), slotId)) {
                ImsLog.d(slotId, "priority is low");
                return false;
            }
        }

        return true;
    }

    @Override
    public void setTrafficPriority(int priorityType, int slotId) {
        ImsLog.d(slotId, "type=" + PRIORITY_TYPE_TO_STRING.get(priorityType));

        synchronized (mTraffics) {
            Traffic traffic = mTraffics.get(slotId);
            if (traffic != null) {
                traffic.setPriorityType(priorityType);
                invokePriorityListener();
            }
        }
    }

    @Override
    public void setSimultaneousCallingSupported(boolean supported, int slotId) {
        ImsLog.d(slotId, "setSimultaneousCallingSupported=" + supported);

        synchronized (mTraffics) {
            Traffic traffic = mTraffics.get(slotId);
            if (traffic != null) {
                traffic.setSimultaneousCallingSupported(supported);
            }
        }
    }

    @Override
    public void setWlan(boolean enabled, int slotId) {
        ImsLog.d(slotId, "enable=" + enabled);

        synchronized (mTraffics) {
            Traffic traffic = mTraffics.get(slotId);
            if (traffic != null) {
                traffic.setWlan(enabled);
            }
        }
    }

    @Override
    public void addListener(PriorityListener listener) {
        mPriorityListeners.add(listener);
    }

    @Override
    public void removeListener(PriorityListener listener) {
        mPriorityListeners.remove(listener);
    }

    private int getTopPrioritizedSlot() {
        int topSlot = 0;

        for (int i = 1; i < mTraffics.size(); i++) {
            if (mTraffics.valueAt(topSlot).isWlan()
                    || mTraffics.valueAt(topSlot).getPriorityType()
                            <  mTraffics.valueAt(i).getPriorityType()) {
                topSlot = mTraffics.keyAt(i);
            }
        }

        return topSlot;
    }

    private boolean hasHighPriorityInOtherSlot(int type, int slotId) {
        for (int i = 0; i < mTraffics.size(); i++) {
            Traffic traffic = mTraffics.valueAt(i);
            if (mTraffics.keyAt(i) == slotId || traffic.isWlan()) {
                continue;
            }

            if (traffic.getPriorityType() != TRAFFIC_PRIORITY_NONE
                    && traffic.getPriorityType() >= type) {
                return true;
            }
        }

        return false;
    }

    private void invokePriorityListener() {
        mHandler.post(new Runnable() {
            @Override
            public void run() {
                for (PriorityListener l : mPriorityListeners) {
                    l.onTrafficPriorityChanged();
                }
            }
        });
    }

    private boolean isEmergency(int slotId) {
        Traffic traffic = mTraffics.get(slotId);
        return (traffic != null) ? traffic.isEmergency() : false;
    }

    private boolean isEmergencyInOtherSlot(int slotId) {
        for (int i = 0; i < mTraffics.size(); i++) {
            if (mTraffics.keyAt(i) == slotId) {
                continue;
            }
            if (mTraffics.valueAt(i).isEmergency()) {
                return true;
            }
        }

        return false;
    }

    private boolean isIdle() {
        for (int i = 0; i < mTraffics.size(); i++) {
            if (mTraffics.valueAt(i).getPriorityType() != TRAFFIC_PRIORITY_NONE) {
                return false;
            }
        }

        return true;
    }

    private boolean isSimultaneousCallingSupported(int slotId) {
        Traffic traffic = mTraffics.get(slotId);
        return (traffic != null) ? traffic.isSimultaneousCallingSupported() : false;
    }

    private boolean isWlan(int slotId) {
        Traffic traffic = mTraffics.get(slotId);
        return (traffic != null) ? traffic.isWlan() : false;
    }

    private boolean isWlanInOtherSlot(int slotId) {
        if (mTraffics.size() == 1) {
            return false;
        }

        for (int i = 0; i < mTraffics.size(); i++) {
            if (mTraffics.keyAt(i) == slotId) {
                continue;
            }
            if (!mTraffics.valueAt(i).isWlan()) {
                return false;
            }
        }

        return true;
    }

    private static int getPriorityType(int trafficType) {
        switch (trafficType) {
            case ImsRadioInterface.TRAFFIC_TYPE_REGISTRATION:
                return TRAFFIC_PRIORITY_REGISTRATION;
            case ImsRadioInterface.TRAFFIC_TYPE_SMS:
                return TRAFFIC_PRIORITY_SMS;
            case ImsRadioInterface.TRAFFIC_TYPE_VIDEO:
                return TRAFFIC_PRIORITY_VIDEO;
            case ImsRadioInterface.TRAFFIC_TYPE_VOICE:
                return TRAFFIC_PRIORITY_VOICE;
            case ImsRadioInterface.TRAFFIC_TYPE_EMERGENCY_SMS:
                return TRAFFIC_PRIORITY_EMERGENCY_SMS;
            case ImsRadioInterface.TRAFFIC_TYPE_EMERGENCY:
                return TRAFFIC_PRIORITY_EMERGENCY;
            case ImsRadioInterface.TRAFFIC_TYPE_UT_XCAP: // fall-through
            default:
                return TRAFFIC_PRIORITY_NONE;
        }
    }

    /**
     * This class provides the top priority type of the IMS traffic for a specified slot.
     */
    private static final class Traffic {
        private int mPriorityType = TRAFFIC_PRIORITY_NONE;
        private boolean mWlan = false;
        private boolean mIsSimultaneousCallingSupported = false;

        Traffic() {
        }

        public int getPriorityType() {
            return mPriorityType;
        }

        public boolean isEmergency() {
            return (mPriorityType == TRAFFIC_PRIORITY_EMERGENCY
                    || mPriorityType == TRAFFIC_PRIORITY_EMERGENCY_SMS);
        }

        public boolean isSimultaneousCallingSupported() {
            return mIsSimultaneousCallingSupported;
        }

        public boolean isWlan() {
            return mWlan;
        }

        public void setPriorityType(int type) {
            mPriorityType = type;
        }

        public void setSimultaneousCallingSupported(boolean supported) {
            mIsSimultaneousCallingSupported = supported;
        }

        public void setWlan(boolean enabled) {
            mWlan = enabled;
        }
    }
}
