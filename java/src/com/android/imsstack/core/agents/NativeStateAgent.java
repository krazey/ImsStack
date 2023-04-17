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

import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.core.config.ServiceCaps;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.internal.imsservice.MmTelFeatureRegistry;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * A class for tracking the native logic's states.
 */
public class NativeStateAgent implements NativeStateInterface {
    private final Set<Listener> mListeners = new CopyOnWriteArraySet<>();
    private final int mSlotId;
    private final Handler mHandler;
    private boolean mServiceReady;

    public NativeStateAgent(int slotId) {
        mSlotId = slotId;
        mHandler = new Handler(AppContext.getInstance().getMainLooper());
    }

    @Override
    public void init(Context context) {
    }

    @Override
    public void cleanup() {
        ICellInfo ci = (ICellInfo) AgentFactory.getAgent(AgentFactory.CELL_INFO, mSlotId);
        if (ci != null) {
            ci.stopTrackingCellInfo(AppContext.getInstance());
            ci.cleanup();
        }

        mHandler.removeCallbacksAndMessages(null);
        mListeners.clear();
    }

    @Override
    public boolean isServiceReady() {
        return mServiceReady;
    }

    @Override
    public void addListener(Listener listener) {
        mListeners.add(listener);

        if (isServiceReady()) {
            mHandler.post(() -> {
                if (mListeners.contains(listener)) {
                    listener.onNativeServiceReady();
                }
            });
        }
    }

    @Override
    public void removeListener(Listener listener) {
        mListeners.remove(listener);
    }

    /**
     * Updates the native service ready state.
     *
     * @param serviceReady The flag specifying whether the native service is ready or not.
     */
    public void updateServiceReady(boolean serviceReady) {
        mHandler.post(() -> {
            setServiceReady(serviceReady);
        });
    }

    private void initState() {
        IBatteryState bs = (IBatteryState) AgentFactory.getAgent(AgentFactory.BATTERY_STATE);

        if (bs != null) {
            bs.notifyLowBatteryState(mSlotId);
        }

        ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

        if (system != null) {
            IDcNetWatcher dcnw =
                    (IDcNetWatcher) DcFactory.getDc(DcFactory.NETWORK_WATCHER, mSlotId);

            if (dcnw != null) {
                system.notifyEvent(ImsEventDef.IMS_EVENT_IMS_VOICE_OVER_PS_STATE,
                        dcnw.isVops()
                        ? ImsEventDef.IMS_VOICE_OVER_PS_SUPPORTED
                        : ImsEventDef.IMS_VOICE_OVER_PS_NOT_SUPPORTED, 0);
            }

            MmTelFeatureRegistry mtfr =
                    ImsServiceRegistry.getInstance(mSlotId).getMmTelFeatureRegistry();

            system.notifyEvent(ImsEventDef.IMS_EVENT_VOLTE_SETTING,
                    mtfr.isAdvancedCallingSettingEnabled()
                    ? ImsEventDef.IMS_VOLTE_SETTING_ON
                    : ImsEventDef.IMS_VOLTE_SETTING_OFF, 0);

            system.notifyEvent(ImsEventDef.IMS_EVENT_WFC_SETTING_CHANGED,
                    mtfr.isVoWiFiSettingEnabled()
                    ? ImsEventDef.IMS_WFC_ON
                    : ImsEventDef.IMS_WFC_OFF, mtfr.getVoWiFiModeSetting());

            system.notifyEvent(ImsEventDef.IMS_EVENT_RTT_SETTING, mtfr.getRttMode(), 0);
        }

        if (ServiceCaps.isWfcEnabledByPlatform(mSlotId)) {
            ICellInfo ci = (ICellInfo) AgentFactory.getAgent(AgentFactory.CELL_INFO, mSlotId);
            if (ci != null) {
                ci.init(AppContext.getInstance());
                ci.startTrackingCellInfo(AppContext.getInstance());
                ci.setLastCellInfoStorage(true);
            }
        }
    }

    private void notifyNativeServiceReady() {
        ImsLog.i(mSlotId, "NativeState#notifyNativeServiceReady");
        for (Listener l : mListeners) {
            l.onNativeServiceReady();
        }
    }

    private void setServiceReady(boolean serviceReady) {
        if (mServiceReady != serviceReady) {
            ImsLog.i(mSlotId, "NativeState#setServiceReady: "
                    + mServiceReady + " >> " + serviceReady);
            mServiceReady = serviceReady;
        }

        if (isServiceReady()) {
            initState();
        }

        notifyNativeServiceReady();
    }
}
