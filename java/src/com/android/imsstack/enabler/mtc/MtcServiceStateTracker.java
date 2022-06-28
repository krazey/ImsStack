/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20141129    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.enabler.mtc;

import android.os.Handler;
import android.os.Registrant;
import android.os.RegistrantList;

import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.IUIMS;
import com.android.imsstack.enabler.mtc.reg.ImsServiceState;

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
        mServiceState = IUMtcService.SERVICE_NONE;
        mEmergencyServiceState = IUMtcService.SERVICE_NONE;
        mEmergencyServiceReason = IUMtcService.ES_IDLE_REASON_NONE;
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
            return (mEmergencyServiceState == IUMtcService.ES_OPENED)
                    || (mEmergencyServiceState == IUMtcService.ES_IN_CALL);
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
                || (mEmergencyServiceState == IUMtcService.ES_UNAVAILABLE)) {
            // To guard timing issue: emergency service is already unavailable
            r.notifyResult(new ImsServiceState(IUIMS.APP_MTC,
                    IUMtcService.SERVICE_EMERGENCY,
                    mEmergencyServiceState, mEmergencyServiceReason));
        }
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
        mEmergencyServiceState = state;
        mEmergencyServiceReason = reason;

        if (mEmergencyServiceState == IUMtcService.ES_IDLE) {
            mEmergencyServiceReason = IUMtcService.ES_IDLE_REASON_NONE;
        }
    }

    private void updateState(int state) {
        if (mServiceState == state) {
            // REG. state is not changed; ignore it.
            return;
        }

        if ((state < IUMtcService.SERVICE_NONE) || (state > IUMtcService.SERVICE_OPENING)) {
            // Invalid state value; ignore it
            return;
        }

        mServiceState = state;

        // This is not to update the OPENING state when offline-call is supported.
        if ((mServiceState == IUMtcService.SERVICE_NONE)
                || (mServiceState == IUMtcService.SERVICE_VOIP)
                || (mServiceState == IUMtcService.SERVICE_VT)
                || (mServiceState == IUMtcService.SERVICE_UC)) {
            MtcStateUtils.updateRegState(mContext.getContext(),
                    mContext.getSlotId(), mServiceState);
        }

        MtcStateUtils.notifyRegState(mContext.getContext(),
                mContext.getSlotId(), mServiceState, MtcStateUtils.SERVICE_UC);
    }
}
