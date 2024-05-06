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

package com.android.imsstack.enabler.mtc;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.mtc.reg.MtcServiceState;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

public final class MtcServiceStateTracker extends MtcApp.ServiceStateListener
        implements IServiceStateTracker {
    private final IBaseContext mContext;
    private final Set<Listener> mListeners = new CopyOnWriteArraySet<>();
    private int mServiceState = IUMtcService.SERVICE_NONE;
    private int mEmergencyServiceState = IUMtcService.SERVICE_NONE;
    private int mEmergencyServiceReason = IUMtcService.ES_IDLE_REASON_NONE;

    public MtcServiceStateTracker(IBaseContext context) {
        mContext = context;
    }

    public void dispose() {
        clear();
    }

    public void clear() {
        setServiceState(IUMtcService.SERVICE_NONE);
        setEmergencyServiceState(IUMtcService.SERVICE_NONE);
        setEmergencyServiceReason(IUMtcService.ES_IDLE_REASON_NONE);
    }

    /**
     * Notifies the application when the emergency service state is changed.
     */
    @Override
    public void onEmergencyServiceStateChanged(MtcApp app, int state, int reason) {
        updateEmergencyState(state, reason);

        for (Listener l : mListeners) {
            l.onEmergencyServiceStateChanged(
                    new MtcServiceState(IUMtcService.SERVICE_EMERGENCY, state, reason));
        }
    }

    /**
     * Notifies the application when the normal service state is changed.
     */
    @Override
    public void onServiceStateChanged(MtcApp app, int state, int reason) {
        if (state == IUMtcService.SERVICE_EMERGENCY) {
            // no-op
            return;
        }

        updateState(state);

        for (Listener l : mListeners) {
            l.onNormalServiceStateChanged(new MtcServiceState(state, -1, reason));
        }
    }

    @Override
    public boolean isServiceRegistered(int serviceType) {
        if (serviceType == IUMtcService.SERVICE_EMERGENCY) {
            return isEmergencyServiceState(IUMtcService.ES_OPENED)
                    || isEmergencyServiceState(IUMtcService.ES_IN_CALL);
        } else {
            int regServiceType = MtcStateUtils.getRegisteredServiceType(
                    mContext.getContext(), mContext.getPhoneId());

            if (serviceType == IUMtcService.SERVICE_VOIP) {
                return (regServiceType == IUMtcService.SERVICE_UC)
                        || (regServiceType == IUMtcService.SERVICE_VOIP);
            } else if (serviceType == IUMtcService.SERVICE_UC) {
                return regServiceType == IUMtcService.SERVICE_UC;
            } else if (serviceType == IUMtcService.SERVICE_VT) {
                if (regServiceType == IUMtcService.SERVICE_VT) {
                    return true;
                } else if (regServiceType == IUMtcService.SERVICE_UC) {
                    // If the service type is UC and VT is disabled,
                    // then VT is not supported.
                    return MtcStateUtils.isVtProvisioned(
                            mContext.getContext(), mContext.getPhoneId());
                }
            }
        }

        return false;
    }

    @Override
    public void addListener(Listener listener) {
        mListeners.add(listener);

        if (isServiceRegistered(IUMtcService.SERVICE_EMERGENCY)
                || isEmergencyServiceState(IUMtcService.ES_UNAVAILABLE)) {
            // To guard timing issue: emergency service is already unavailable
            notifyServiceStateIfEmergencyServiceRegistered();
        }

        if (isServiceRegistered(IUMtcService.SERVICE_UC)) {
            notifyServiceStateIfServiceRegistered(IUMtcService.SERVICE_UC);
        } else {
            if (isServiceRegistered(IUMtcService.SERVICE_VOIP)) {
                notifyServiceStateIfServiceRegistered(IUMtcService.SERVICE_VOIP);
            }

            if (isServiceRegistered(IUMtcService.SERVICE_VT)) {
                notifyServiceStateIfServiceRegistered(IUMtcService.SERVICE_VT);
            }
        }
    }

    @Override
    public void removeListener(Listener listener) {
        mListeners.remove(listener);
    }

    private void notifyServiceStateIfEmergencyServiceRegistered() {
        for (Listener l : mListeners) {
            l.onEmergencyServiceStateChanged(new MtcServiceState(IUMtcService.SERVICE_EMERGENCY,
                    mEmergencyServiceState, mEmergencyServiceReason));
        }
    }

    @VisibleForTesting
    void setServiceState(int serviceState) {
        mServiceState = serviceState;
    }

    @VisibleForTesting
    void setEmergencyServiceState(int emergencyServiceState) {
        mEmergencyServiceState = emergencyServiceState;
    }

    @VisibleForTesting
    void setEmergencyServiceReason(int emergencyServiceReason) {
        mEmergencyServiceReason = emergencyServiceReason;
    }

    @VisibleForTesting
    boolean isServiceState(int serviceState) {
        if (mServiceState == serviceState) {
            return true;
        }
        return false;
    }

    @VisibleForTesting
    boolean isEmergencyServiceState(int emergencyServiceState) {
        if (mEmergencyServiceState == emergencyServiceState) {
            return true;
        }
        return false;
    }

    private void notifyServiceStateIfServiceRegistered(int serviceType) {
        for (Listener l : mListeners) {
            l.onNormalServiceStateChanged(new MtcServiceState(serviceType));
        }
    }

    private void updateEmergencyState(int state, int reason) {
        setEmergencyServiceState(state);
        setEmergencyServiceReason(reason);

        if (isEmergencyServiceState(IUMtcService.ES_IDLE)) {
            setEmergencyServiceReason(IUMtcService.ES_IDLE_REASON_NONE);
        }
    }

    private void updateState(int state) {
        if (isServiceState(state)) {
            // REG. state is not changed; ignore it.
            return;
        }

        if ((state < IUMtcService.SERVICE_NONE) || (state > IUMtcService.SERVICE_OPENING)) {
            // Invalid state value; ignore it
            return;
        }

        setServiceState(state);

        // This is not to update the OPENING state when offline-call is supported.
        if (isServiceState(IUMtcService.SERVICE_NONE)
                || isServiceState(IUMtcService.SERVICE_VOIP)
                || isServiceState(IUMtcService.SERVICE_VT)
                || isServiceState(IUMtcService.SERVICE_UC)) {
            MtcStateUtils.updateRegState(mContext.getContext(),
                    mContext.getSlotId(), mServiceState);
        }
    }
}
