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

package com.android.imsstack.internal.imsservice;

import android.telephony.ims.ProvisioningManager.FeatureProvisioningCallback;
import android.util.SparseArray;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.SystemServiceProxy.ImsManagerProxy;
import com.android.imsstack.base.SystemServiceProxy.ProvisioningManagerProxy;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Objects;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;
import java.util.concurrent.Executor;

public class ProvisioningStatusTracker {
    public interface Listener {
        /**
         * Notifies the IMS MMTEL provisioning has changed for a specific capability and IMS
         * registration technology.
         */
        default void onFeatureProvisioningChanged(int capability, int tech, boolean isProvisioned) {
        }
    }

    private int mSubId = MSimUtils.INVALID_SUB_ID;
    private final int mSlotId;
    private final Executor mExecutor;
    private final Sim.Listener mSimListener = new Sim.Listener() {
        @Override
        public void onSimStateChanged() {
            updateCallback();
        }
    };
    private static final SparseArray<ProvisioningStatusTracker>
            sProvisioningStatusTracker = new SparseArray<>();
    private final Set<Listener> mListeners = new CopyOnWriteArraySet<>();
    private final FeatureProvisioningCallback mCallback = new FeatureProvisioningCallback() {
        @Override
        public void onFeatureProvisioningChanged(int capability, int tech, boolean isProvisioned) {
            ImsLog.i(mSlotId, "onFeatureProvisioningChanged : Capability: " + capability
                    + ", Tech: " + tech + ", Provisioned: " + isProvisioned);
            handleProvisioningChanged(capability, tech, isProvisioned);
        }

        @Override
        public void onRcsFeatureProvisioningChanged(int capability, int tech,
                boolean isProvisioned) {
            ImsLog.i(mSlotId, "onRcsFeatureProvisioningChanged : Capability: " + capability
                    + ", Tech: " + tech + ", Provisioned: " + isProvisioned);
        }
    };

    ProvisioningStatusTracker(int slotId) {
        mSlotId = slotId;
        mExecutor = AppContext.getInstance().getMainExecutor();
    }

    /**
     * Returns a ProvisioningStatusTracker for the given slot-id.
     *
     * @param slotId The slot-id to be retrieved.
     * @return A ProvisioningStatusTracker.
     */
    public static ProvisioningStatusTracker getInstance(int slotId) {
        ProvisioningStatusTracker provisioningStatusTracker;

        synchronized (sProvisioningStatusTracker) {
            provisioningStatusTracker = sProvisioningStatusTracker.get(slotId);
            if (provisioningStatusTracker == null) {
                provisioningStatusTracker = new ProvisioningStatusTracker(slotId);
                sProvisioningStatusTracker.put(slotId, provisioningStatusTracker);
            }
        }
        return provisioningStatusTracker;
    }

    /**
     * Sets a ProvisioningStatusTracker for the given slot-id.
     *
     * @param slotId The slot-id to update ProvisioningStatusTracker.
     * @param instance The ProvisioningStatusTracker instance to be set.
     */
    @VisibleForTesting
    public static void setInstance(int slotId, ProvisioningStatusTracker instance) {
        synchronized (sProvisioningStatusTracker) {
            sProvisioningStatusTracker.put(slotId, instance);
        }
    }

    /**
     * Removes a ProvisioningStatusTracker for the given slot-id.
     *
     * @param slotId The slot-id to be retrieved.
     */
    public static void releaseInstance(int slotId) {
        synchronized (sProvisioningStatusTracker) {
            ProvisioningStatusTracker provisioningStatusTracker =
                    sProvisioningStatusTracker.get(slotId);
            if (provisioningStatusTracker != null) {
                provisioningStatusTracker.unregisterFeatureProvisioningCallback();
                sProvisioningStatusTracker.remove(slotId);
                provisioningStatusTracker = null;
            }
        }
    }

    /**
     * Starts monitoring sim state to register or unregister FeatureProvisioningCallback.
     * If the sim is already loaded, register FeatureProvisioningCallback immediately.
     */
    public void start() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotId);
        if (sim != null) {
            sim.addListener(mSimListener);
        }

        updateCallback();
    }

    /**
     * Adds the listener to monitor the feature provisioning status change.
     *
     * @param listener The listener to be added.
     * @throws NullPointerException if the listener is null.
     */
    public void addListener(@NonNull Listener listener) {
        Objects.requireNonNull(listener, "listener must not be null");
        mListeners.add(listener);
    }

    /**
     * Removes the listener that was previously set.
     *
     * @param listener The listener to be removed.
     * @throws NullPointerException if the listener is null.
     */
    public void removeListener(@NonNull Listener listener) {
        Objects.requireNonNull(listener, "listener must not be null");
        mListeners.remove(listener);
    }

    /**
     * Get the provisioning status for the IMS MmTel capability specified.
     */
    public boolean getProvisioningStatusForCapability(int capability, int tech) {
        ProvisioningManagerProxy pmp = getProvisioningManagerProxy();
        return (pmp != null) ? pmp.getProvisioningStatusForCapability(capability, tech) : true;
    }

    /**
     * Registers feature provisioning change callback for current subscriber if subId changed.
     * If the old subId is still valid, unregister it first.
     */
    private void updateCallback() {
        int subId = MSimUtils.getSubId(mSlotId);
        if (subId == mSubId) {
            ImsLog.i(mSlotId, "do not update callback for subId " + mSubId);
            return;
        }

        ImsLog.i(mSlotId, "updateCallback old subId is " + mSubId + ", new subId is " + subId);

        // unregister callback for old subscriber
        if (mSubId != MSimUtils.INVALID_SUB_ID) {
            unregisterFeatureProvisioningCallback();
        }

        // update subscriber id
        mSubId = subId;

        // register callback for updated subscriber
        if (mSubId != MSimUtils.INVALID_SUB_ID) {
            registerFeatureProvisioningCallback();
        }
    }

    private void registerFeatureProvisioningCallback() {
        ProvisioningManagerProxy pmp = getProvisioningManagerProxy();
        if (pmp != null) {
            pmp.registerFeatureProvisioningChangedCallback(mExecutor, mCallback);
        }
    }

    private void unregisterFeatureProvisioningCallback() {
        ProvisioningManagerProxy pmp = getProvisioningManagerProxy();
        if (pmp != null) {
            pmp.unregisterFeatureProvisioningChangedCallback(mCallback);
        }
    }

    @Nullable
    private ProvisioningManagerProxy getProvisioningManagerProxy() {
        ImsManagerProxy imp = AppContext.getInstance().getSystemServiceProxy(ImsManagerProxy.class);
        return imp.getProvisioningManagerProxy(mSubId);
    }

    private void handleProvisioningChanged(int capability, int tech, boolean isProvisioned) {
        for (Listener l : mListeners) {
            l.onFeatureProvisioningChanged(capability, tech, isProvisioned);
        }
    }
}
