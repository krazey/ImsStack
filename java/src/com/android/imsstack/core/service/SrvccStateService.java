/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20150526    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.core.service;

import android.content.Context;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.IPhoneState;
import com.android.imsstack.core.agents.agentif.IPhoneStateNotifier;
import com.android.imsstack.core.agents.agentif.ImsPhoneStateListener;
import com.android.imsstack.core.service.serviceif.IService;
import com.android.imsstack.core.service.serviceif.IVoLteService;
import com.android.imsstack.internal.enabler.ImsStateStore;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ISystemAPISRVCC;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.ImsLog;

public class SrvccStateService implements ISystemAPISRVCC, IService {
    /** SRVCC state */
    private static final int STATE_IDLE = 0;
    private static final int STATE_PENDING = 1;
    private static final int STATE_ACTIVE = 2;

    private final Object mLock = new Object();
    private CallStateTracker mCallStateTracker = null;
    private final VoLteServiceStateListener mVoLteServiceStateListener
            = new VoLteServiceStateListener();

    private IVoLteService mVoLteService = null;
    private int mState = STATE_IDLE;
    private boolean mSrvccEventNotificationRequired = false;

    public SrvccStateService() {
    }

    @Override
    public boolean start(IVoLteService voLteService) {
        mVoLteService = voLteService;

        ImsLog.i(getSlotId(), "");

        if (mCallStateTracker == null) {
            mCallStateTracker = new CallStateTracker();
        }

        setVoLteServiceStateListener(true);

        ISystem system = SystemInterface.getInstance().getSystem(getSlotId());

        if (system != null) {
            system.setISystemAPISRVCC(this);
        }

        return true;
    }

    @Override
    public void cleanup(Context context) {
        ImsLog.i(getSlotId(), "");

        setVoLteServiceStateListener(false);
        setState(STATE_IDLE);

        if (mCallStateTracker != null) {
            mCallStateTracker.clear();
            mCallStateTracker = null;
        }

        ISystem system = SystemInterface.getInstance().getSystem(getSlotId());

        if (system != null) {
            system.setISystemAPISRVCC(null);
        }
    }

    @Override
    public void update(Context context) {
    }

    @Override
    public int listenSrvccEvent4Sys() {
        synchronized (mLock) {
            mSrvccEventNotificationRequired = true;
        }

        return 1;
    }

    @Override
    public int unlistenSrvccEvent4Sys() {
        synchronized (mLock) {
            mSrvccEventNotificationRequired = false;
        }

        return 1;
    }

    public void clearState() {
        setState(STATE_IDLE);
    }

    public void dispose() {
        setVoLteServiceStateListener(false);
    }

    public boolean isSrvccCompleted() {
        return getState() == STATE_ACTIVE;
    }

    public boolean isSrvccPending() {
        return getState() == STATE_PENDING;
    }

    private int getState() {
        synchronized (mLock) {
            return mState;
        }
    }

    private boolean isSrvccEventNotificationRequired() {
        synchronized (mLock) {
            return mSrvccEventNotificationRequired;
        }
    }

    private void notifySrvccState(int state) {
        if (!isSrvccEventNotificationRequired()) {
            return;
        }

        ISystem system = SystemInterface.getInstance().getSystem(getSlotId());

        if (system != null) {
            system.notifyEvent(ImsEventDef.IMS_EVENT_SRVCC_NOTIFICATION, state, 0);
        }
    }

    private void setState(int state) {
        synchronized (mLock) {
            if (mState != state) {
                ImsLog.d(getSlotId(), "SRVCCState :: "
                    + stateToString(mState) + " >> " + stateToString(state));

                mState = state;
            }
        }
    }

    private void setVoLteServiceStateListener(boolean listen) {
        if (listen) {
            mVoLteServiceStateListener.setListener();
        } else {
            mVoLteServiceStateListener.dispose();
        }
    }

    private void startInternal() {
        if (!mVoLteServiceStateListener.isRegistered()) {
            setVoLteServiceStateListener(true);
        }
    }

    private static String stateToString(int state) {
        switch (state) {
        case STATE_IDLE:
            return "IDLE";
        case STATE_PENDING:
            return "PENDING";
        case STATE_ACTIVE:
            return "ACTIVE";
        default:
            return "__UNKNOWN__";
        }
    }

    private Context getContext() {
        return (mVoLteService != null) ? mVoLteService.getContext() : null;
    }

    private int getSlotId() {
        return (mVoLteService != null) ? mVoLteService.getSlotID() : 0;
    }

    private final class CallStateTracker implements ImsStateStore.Listener {
        private int mCallState = ImsStateStore.STATE_INACTIVE;

        public CallStateTracker() {
            init();
        }

        public void clear() {
            ImsStateStore.getCallState(getSlotId()).removeListener(this);
            setCallState(ImsStateStore.STATE_INACTIVE);
        }

        public void init() {
            ImsStateStore.CallState callState = ImsStateStore.getCallState(getSlotId());
            setCallState(callState.getState());
            callState.addListener(this);

            if (isInCall()) {
                startInternal();
            }
        }

        @Override
        public void onStateChanged() {
            int state = ImsStateStore.getCallState(getSlotId()).getState();
            boolean oldInCall = isInCall();

            setCallState(state);

            if (isInCall() && !oldInCall) {
                startInternal();
            } else if (!isInCall() && oldInCall) {
                // To resolve the state mis-matched situation
                clearState();
            }
        }

        public boolean isInCall() {
            synchronized (mLock) {
                return mCallState != ImsStateStore.STATE_INACTIVE;
            }
        }

        private void setCallState(int state) {
            if (mCallState != state) {
                ImsLog.d(getSlotId(), "CallState :: " + mCallState + " >> " + state);

                synchronized (mLock) {
                    mCallState = state;
                }
            }
        }
    }

    private final class VoLteServiceStateListener extends ImsPhoneStateListener {
        private IPhoneStateNotifier mNotifier = null;

        public VoLteServiceStateListener() {
        }

        public void dispose() {
            synchronized (mLock) {
                if (mNotifier == null) {
                    return;
                }

                IPhoneState ips = getPhoneState();

                if (ips != null) {
                    ips.removeNotifier(mNotifier);
                }

                mNotifier.setListener(null);
                mNotifier = null;
            }
        }

        public boolean isRegistered() {
            synchronized (mLock) {
                return mNotifier != null;
            }
        }

        public void setListener() {
            synchronized (mLock) {
                if (mNotifier != null) {
                    return;
                }

                IPhoneState ips = getPhoneState();

                if (ips != null) {
                    mNotifier = ips.createNotifier(this,
                            ImsGlobal.getInstance().getDefaultLooper());
                    mNotifier.setEvents(LISTEN_SRVCC_STATE);

                    ips.addNotifier(mNotifier);
                }
            }
        }

        /**
         * Invokes when SRVCC state is changed.
         */
        public void onSrvccStateChanged(int state) {
            ImsLog.i(getSlotId(), "onSrvccStateChanged :: state=" + state);

            switch (state) {
                case TelephonyManager.SRVCC_STATE_HANDOVER_STARTED:
                    if (getState() == STATE_IDLE) {
                        setState(STATE_PENDING);
                        notifySrvccState(ImsEventDef.IMS_SRVCC_EVT_START);
                    }
                    break;

                case TelephonyManager.SRVCC_STATE_HANDOVER_COMPLETED:
                    if (getState() == STATE_PENDING) {
                        setState(STATE_ACTIVE);
                        notifySrvccState(ImsEventDef.IMS_SRVCC_EVT_SUCCESS);
                    }
                    break;

                case TelephonyManager.SRVCC_STATE_HANDOVER_FAILED:
                    if (getState() == STATE_PENDING) {
                        setState(STATE_IDLE);
                        notifySrvccState(ImsEventDef.IMS_SRVCC_EVT_FAILURE);
                    }
                    break;

                case TelephonyManager.SRVCC_STATE_HANDOVER_CANCELED:
                    setState(STATE_IDLE);
                    notifySrvccState(ImsEventDef.IMS_SRVCC_EVT_CANCEL);
                    break;

                default:
                    break;
            }
        }

        private IPhoneState getPhoneState() {
            return (IPhoneState)AgentFactory.getAgent(AgentFactory.PHONE_STATE, getSlotId());
        }
    }

}
