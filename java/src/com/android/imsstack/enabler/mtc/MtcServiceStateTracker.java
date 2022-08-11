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

import android.os.Handler;
import android.os.Registrant;
import android.os.RegistrantList;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.IUIMS;
import com.android.imsstack.enabler.mtc.reg.ImsServiceState;
import com.android.internal.annotations.VisibleForTesting;

public final class MtcServiceStateTracker extends MtcApp.ServiceStateListener
        implements IServiceStateTracker {
    private final IBaseContext mContext;
    private RegistrantList mEServiceStateChangedRegistrants = new RegistrantList();
    private RegistrantList mServiceStateChangedRegistrants = new RegistrantList();
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

        if (mEServiceStateChangedRegistrants.size() > 0) {
            mEServiceStateChangedRegistrants.notifyResult(
                    new ImsServiceState(IUIMS.APP_MTC,
                        IUMtcService.SERVICE_EMERGENCY, state, reason));
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

        if (mServiceStateChangedRegistrants.size() > 0) {
            mServiceStateChangedRegistrants.notifyResult(
                    new ImsServiceState(IUIMS.APP_MTC, state, -1, reason));
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

    // The Message object contains AsyncResult object in "obj" field.
    // The AsyncResult object contains ImsServiceState object in "result" field.
    @Override
    public void registerForEmergencyServiceStateChanged(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);

        mEServiceStateChangedRegistrants.add(r);

        // Notify the service state if a service is already connected
        notifyServiceStateIfEmergencyServiceRegistered(r);
    }

    @Override
    public void unregisterForEmergencyServiceStateChanged(Handler h) {
        mEServiceStateChangedRegistrants.remove(h);
    }

    // The Message object contains AsyncResult object in "obj" field.
    // The AsyncResult object contains ImsServiceState object in "result" field.
    @Override
    public void registerForServiceStateChanged(Handler h, int what, Object obj) {
        Registrant r = new Registrant(h, what, obj);

        mServiceStateChangedRegistrants.add(r);

        // Notify the service state if a service is already connected
        notifyServiceStateIfServiceRegistered(r);
    }

    @Override
    public void unregisterForServiceStateChanged(Handler h) {
        mServiceStateChangedRegistrants.remove(h);
    }

    private void notifyServiceStateIfEmergencyServiceRegistered(Registrant r) {
        if (isServiceRegistered(IUMtcService.SERVICE_EMERGENCY)
                || isEmergencyServiceState(IUMtcService.ES_UNAVAILABLE)) {
            // To guard timing issue: emergency service is already unavailable
            r.notifyResult(new ImsServiceState(IUIMS.APP_MTC,
                    IUMtcService.SERVICE_EMERGENCY,
                    mEmergencyServiceState, mEmergencyServiceReason));
        }
    }

    @VisibleForTesting
    protected void setServiceState(int serviceState) {
        mServiceState = serviceState;
    }

    @VisibleForTesting
    protected void setEmergencyServiceState(int emergencyServiceState) {
        mEmergencyServiceState = emergencyServiceState;
    }

    @VisibleForTesting
    protected void setEmergencyServiceReason(int emergencyServiceReason) {
        mEmergencyServiceReason = emergencyServiceReason;
    }

    @VisibleForTesting
    protected boolean isServiceState(int serviceState) {
        if (mServiceState == serviceState) {
            return true;
        }
        return false;
    }

    @VisibleForTesting
    protected boolean isEmergencyServiceState(int emergencyServiceState) {
        if (mEmergencyServiceState == emergencyServiceState) {
            return true;
        }
        return false;
    }

    private void notifyServiceStateIfServiceRegistered(Registrant r) {
        if (isServiceRegistered(IUMtcService.SERVICE_UC)) {
            r.notifyResult(new ImsServiceState(IUIMS.APP_MTC, IUMtcService.SERVICE_UC));
        } else {
            if (isServiceRegistered(IUMtcService.SERVICE_VOIP)) {
                r.notifyResult(new ImsServiceState(IUIMS.APP_MTC, IUMtcService.SERVICE_VOIP));
            }

            if (isServiceRegistered(IUMtcService.SERVICE_VT)) {
                r.notifyResult(new ImsServiceState(IUIMS.APP_MTC, IUMtcService.SERVICE_VT));
            }
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

        MtcStateUtils.notifyRegState(mContext.getContext(),
                mContext.getSlotId(), mServiceState, MtcStateUtils.SERVICE_UC);
    }
}
