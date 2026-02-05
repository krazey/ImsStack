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
package com.android.imsstack.imsservice.mmtel.internal;

import android.telephony.Annotation.SrvccState;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.IPhoneStateNotifier;
import com.android.imsstack.core.agents.ImsPhoneStateListener;
import com.android.imsstack.core.agents.PhoneStateInterface;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.imsservice.mmtel.base.ISrvccStateListener;
import com.android.imsstack.imsservice.mmtel.base.ISrvccStateTracker;
import com.android.imsstack.util.ImsLog;

import java.util.Set;
import java.util.concurrent.CopyOnWriteArraySet;

public final class SrvccStateTracker implements ISrvccStateTracker {
    private static final boolean DYNAMIC_BINDING = false;

    private static final int STATE_IDLE = 0;
    private static final int STATE_PENDING = 1;
    private static final int STATE_ACTIVE = 2;

    private final IBaseContext mContext;
    private final Set<ISrvccStateListener> mListeners
            = new CopyOnWriteArraySet<ISrvccStateListener>();
    private VoLteServiceStateListener mVoLteServiceStateListener;
    private int mState = STATE_IDLE;

    public SrvccStateTracker(IBaseContext context) {
        mContext = context;

        mVoLteServiceStateListener = new VoLteServiceStateListener();

        if (!DYNAMIC_BINDING) {
            setVoLteServiceStateListener(true);
        }
    }

    public void dispose() {
        if (!DYNAMIC_BINDING) {
            setVoLteServiceStateListener(false);
        }

        mListeners.clear();
        mVoLteServiceStateListener.dispose();
    }

    @Override
    public void addListener(ISrvccStateListener listener) {
        mListeners.add(listener);
    }

    @Override
    public void removeListener(ISrvccStateListener listener) {
        mListeners.remove(listener);
    }

    @Override
    public boolean isSrvccCompleted() {
        return getState() == STATE_ACTIVE;
    }

    @Override
    public boolean isSrvccPending() {
        return getState() == STATE_PENDING;
    }

    public void start() {
        if (DYNAMIC_BINDING) {
            setVoLteServiceStateListener(true);
        }
    }

    public void stop() {
        if (DYNAMIC_BINDING) {
            setVoLteServiceStateListener(false);
        } else {
            clearState();
        }
    }

    public void clearState() {
        setState(STATE_IDLE);
    }

    private int getState() {
        return mState;
    }

    private void setState(int state) {
        if (mState != state) {
            logi("setState :: "
                + stateToString(mState) + " >> " + stateToString(state));

            mState = state;
        }
    }

    private void onHandoverStarted() {
        for (ISrvccStateListener l : mListeners) {
            l.onHandoverStarted();
        }
    }

    private void onHandoverCompleted() {
        for (ISrvccStateListener l : mListeners) {
            l.onHandoverCompleted();
        }
    }

    private void onHandoverCanceled() {
        for (ISrvccStateListener l : mListeners) {
            l.onHandoverCanceled();
        }
    }

    private void onHandoverFailed() {
        for (ISrvccStateListener l : mListeners) {
            l.onHandoverFailed();
        }
    }

    private void setVoLteServiceStateListener(boolean listen) {
        if (listen) {
            mVoLteServiceStateListener.setListener();
        } else {
            mVoLteServiceStateListener.dispose();
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

    private static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }

    private final class VoLteServiceStateListener implements ImsPhoneStateListener {
        private IPhoneStateNotifier mNotifier = null;

        public VoLteServiceStateListener() {
        }

        public void dispose() {
            if (mNotifier != null) {
                PhoneStateInterface phoneState = AgentFactory.getInstance().getAgent(
                        PhoneStateInterface.class, mContext.getSlotId());

                if (phoneState != null) {
                    phoneState.removeNotifier(mNotifier);
                }

                mNotifier.setListener(null);
                mNotifier = null;
            }
        }

        public void setListener() {
            PhoneStateInterface phoneState = AgentFactory.getInstance().getAgent(
                    PhoneStateInterface.class, mContext.getSlotId());

            if (phoneState != null) {
                mNotifier = phoneState.createNotifier(this, mContext.getDefaultLooper());
                mNotifier.setEvents(LISTEN_SRVCC_STATE);

                phoneState.addNotifier(mNotifier);
            }
        }

        /**
         * Invokes when SRVCC state is changed.
         */
        public void onSrvccStateChanged(@SrvccState int state) {
            logi("onSrvccStateChanged :: slotId=" + mContext.getSlotId() + ", state=" + state);

            switch (state) {
            case TelephonyManager.SRVCC_STATE_HANDOVER_STARTED: {
                if (getState() == STATE_IDLE) {
                    setState(STATE_PENDING);
                    onHandoverStarted();
                }
                break;
            }
            case TelephonyManager.SRVCC_STATE_HANDOVER_COMPLETED: {
                if (getState() == STATE_PENDING) {
                    setState(STATE_ACTIVE);
                    onHandoverCompleted();
                }
                break;
            }
            case TelephonyManager.SRVCC_STATE_HANDOVER_FAILED: {
                if (getState() == STATE_PENDING) {
                    setState(STATE_IDLE);
                    onHandoverFailed();
                }
                break;
            }
            case TelephonyManager.SRVCC_STATE_HANDOVER_CANCELED: {
                setState(STATE_IDLE);
                onHandoverCanceled();
                break;
            }
            default:
                log("Unknown SRVCC state");
                break;
            }
        }
    }
}
