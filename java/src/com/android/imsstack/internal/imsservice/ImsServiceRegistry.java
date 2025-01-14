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

package com.android.imsstack.internal.imsservice;

import android.annotation.NonNull;
import android.telephony.ims.feature.MmTelFeature;
import android.telephony.ims.stub.ImsConfigImplBase;
import android.util.SparseArray;

import com.android.imsstack.util.ImsLog;

import java.util.Objects;
import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

/**
 * A storage of ImsService's states.
 * This manages the ImsService related data such as MmTelFeature,
 * terminal based call waiting state, and notifies the components
 * who monitor this class that its related data has changed.
 */
public class ImsServiceRegistry {
    /**
     * Notifies the components who monitor this class that any states have changed.
     */
    public interface Listener {
        /**
         * Notifies the components who monitor the ImsService related data that
         * the MmTelFeature has changed.
         */
        default void onMmTelFeatureChanged() {
        }

        /**
         * Notifies the components who monitor the ImsService related data that
         * IMS on/off is changed.
         */
        default void onImsOnOffChanged() {
        }
    }

    // Maps a slot-id to a list of ImsServiceRegistry.
    private static SparseArray<ImsServiceRegistry> sImsServiceRegistrys = new SparseArray<>(2);

    private final int mSlotId;
    private volatile boolean mImsEnabled;
    private volatile MmTelFeature mMmTelFeature;
    private volatile ImsConfigImplBase mImsConfig;
    private final MmTelFeatureRegistry mMmTelFeatureRegistry;
    private final MmTelMediaRegistry mMmTelMediaRegistry;
    private final Set<Listener> mListeners = new CopyOnWriteArraySet<>();

    ImsServiceRegistry(int slotId) {
        mSlotId = slotId;
        mImsEnabled = true;
        mMmTelFeature = null;
        mImsConfig = null;
        mMmTelFeatureRegistry = new MmTelFeatureRegistry(slotId);
        mMmTelMediaRegistry = new MmTelMediaRegistry();
    }

    /**
     * Returns an ImsServiceRegistry for the given slot-id.
     *
     * @param slotId The slot-id to be retrieved.
     * @return An ImsServiceRegistry.
     */
    public static ImsServiceRegistry getInstance(int slotId) {
        ImsServiceRegistry imsServiceRegistry;

        synchronized (sImsServiceRegistrys) {
            imsServiceRegistry = sImsServiceRegistrys.get(slotId);
            if (imsServiceRegistry == null) {
                imsServiceRegistry = new ImsServiceRegistry(slotId);
                sImsServiceRegistrys.put(slotId, imsServiceRegistry);
            }
        }

        return imsServiceRegistry;
    }

    /**
     * Returns a MmTelFeature to access the IMS radio interface.
     *
     * @return A MmTelFeature.
     */
    public MmTelFeature getMmTelFeature() {
        return mMmTelFeature;
    }

    /**
     * Sets a MmTelFeature to enable access to the IMS radio interface.
     *
     * This is called by the ImsService implementation layer when MmTelFeature is instantiated,
     * and it's called only for a normal MmTelFeature.
     *
     * @param feature A MmTelFeature to be set.
     */
    public void setMmTelFeature(MmTelFeature feature) {
        if (!Objects.equals(mMmTelFeature, feature)) {
            mMmTelFeature = feature;
            notifyMmTelFeatureChanged();
        }
    }

    /**
     * Returns a ImsConfigImplBase to access the IMS configurations.
     *
     * @return A ImsConfigImplBase.
     */
    public ImsConfigImplBase getImsConfig() {
        return mImsConfig;
    }

    /**
     * Sets a ImsConfigImplBase to access of the IMS configurations.
     *
     * This is called by the ImsService implementation layer when ImsConfigImpl is instantiated.
     *
     * @param imsconfig A ImsConfigImplBase to be set.
     */
    public void setImsConfig(ImsConfigImplBase imsconfig) {
        if (!Objects.equals(mImsConfig, imsconfig)) {
            mImsConfig = imsconfig;
        }
    }

    /**
     * Checks if IMS is enabled or disabled.
     *
     * @return true if IMS is enabled, false otherwise.
     */
    public boolean isImsEnabled() {
        return mImsEnabled;
    }

    /**
     * Sets the IMS on/off status.
     *
     * @param enabled The IMS status to be turned on or off.
     */
    public void setImsEnabled(boolean enabled) {
        if (mImsEnabled != enabled) {
            ImsLog.i(mSlotId, "setImsEnabled: " + mImsEnabled + " >> " + enabled);
            mImsEnabled = enabled;
            notifyImsOnOffChanged();
        }
    }

    /**
     * Returns the {@link MmTelMediaRegistry} to access the {@link MediaThreshold} value.
     *
     * @return {@link MmTelMediaRegistry}
     */
    @NonNull
    public MmTelMediaRegistry getMmTelMediaRegistry() {
        return mMmTelMediaRegistry;
    }

    /**
     * Creates and returns the {@link MmTelMediaQualityReporter} to
     * notify media quality status changed.
     *
     * @return  {@link MmTelMediaQualityReporter}
     */
    @NonNull
    public MmTelMediaQualityReporter createMediaQualityReporter(@NonNull String callId) {
        return new MmTelMediaQualityReporter(mMmTelMediaRegistry, mMmTelFeature, callId);
    }

    /**
     * Adds the listener to monitor the state of this class.
     *
     * @param listener The listener to be added.
     */
    public void addListener(Listener listener) {
        mListeners.add(listener);
    }

    /**
     * Removes the listener that was previously set.
     *
     * @param listener The listener to be removed.
     */
    public void removeListener(Listener listener) {
        mListeners.remove(listener);
    }

    /**
     * Returns a MmTelFeatureRegistry of this registry.
     *
     * @return A MmTelFeatureRegistry.
     */
    public MmTelFeatureRegistry getMmTelFeatureRegistry() {
        return mMmTelFeatureRegistry;
    }

    private void notifyMmTelFeatureChanged() {
        for (Listener l : mListeners) {
            l.onMmTelFeatureChanged();
        }
    }

    private void notifyImsOnOffChanged() {
        for (Listener l : mListeners) {
            l.onImsOnOffChanged();
        }
    }
}
