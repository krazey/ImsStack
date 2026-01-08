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
package com.android.imsstack.internal.enabler;

import android.annotation.NonNull;
import android.telephony.TelephonyManager;
import android.util.ArraySet;
import android.util.SparseArray;

import com.android.imsstack.base.AppContext;

/**
 * This class provides the cache information of ImsStack's internal states.
 */
public final class ImsStateStore {
    /** The network types for Ims registration state. */
    public static final int NETWORK_TYPE_UNKNOWN = TelephonyManager.NETWORK_TYPE_UNKNOWN;
    public static final int NETWORK_TYPE_NR = TelephonyManager.NETWORK_TYPE_NR;
    public static final int NETWORK_TYPE_LTE = TelephonyManager.NETWORK_TYPE_LTE;
    public static final int NETWORK_TYPE_IWLAN = TelephonyManager.NETWORK_TYPE_IWLAN;

    public static final int STATE_INACTIVE = 0;
    public static final int STATE_ACTIVE = 1;

    private static SparseArray<ImsState> sImsStates = new SparseArray<>();

    /**
     * Returns the registration state for the given phone-id.
     *
     * @param phoneId The phone-id to be retrieved.
     * @return The registration state.
     */
    public static @NonNull RegState getRegState(int phoneId) {
        ImsState imsState = getImsState(phoneId);
        return imsState.getRegState();
    }

    /**
     * Returns the MmTel state for the given phone-id.
     *
     * @param phoneId The phone-id to be retrieved.
     * @return The MmTel state.
     */
    public static @NonNull MmTelState getMmTelState(int phoneId) {
        ImsState imsState = getImsState(phoneId);
        return imsState.getMmTelState();
    }

    /**
     * Returns the call (MmTel) state for the given phone-id.
     *
     * @param phoneId The phone-id to be retrieved.
     * @return The call state.
     */
    public static @NonNull CallState getCallState(int phoneId) {
        ImsState imsState = getImsState(phoneId);
        return imsState.getCallState();
    }

    /**
     * Initializes all the state information to a default value.
     *
     * @param phoneId The phone-id to be initialized.
     */
    public static void init(int phoneId) {
        ImsState imsState = getImsState(phoneId);

        imsState.getRegState().init();
        imsState.getMmTelState().init();
        imsState.getCallState().init();
    }

    /**
     * Listenter to monitor the change of IMS states (registration / call).
     */
    public interface Listener {
        /**
         * Invoked when any IMS state is changed.
         */
        void onStateChanged();
    }

    /**
     * RegState stores the IMS registration states.
     */
    public static class RegState {
        private int mState = STATE_INACTIVE;
        private int mNetworkType = NETWORK_TYPE_UNKNOWN;
        private final ArraySet<Listener> mListeners = new ArraySet<>();

        RegState() {
        }

        /**
         * Initializes the registration state.
         */
        public void init() {
            boolean changed = false;

            if (mState != STATE_INACTIVE) {
                mState = STATE_INACTIVE;
                changed = true;
            }

            if (mNetworkType != NETWORK_TYPE_UNKNOWN) {
                mNetworkType = NETWORK_TYPE_UNKNOWN;
                changed = true;
            }

            if (changed) {
                notifyStateChanged();
            }
        }

        /**
         * Returns the state of IMS registration.
         *
         * @return The registration state. Valid values are
         *         {@link #STATE_ACTIVE},
         *         {@link #STATE_INACTIVE}.
         */
        public int getState() {
            return mState;
        }

        /**
         * Returns the network type when IMS is registered.
         *
         * @return The registered network type. Valid values are
         *         {@link #NETWORK_TYPE_UNKNOWN},
         *         {@link #NETWORK_TYPE_NR},
         *         {@link #NETWORK_TYPE_LTE},
         *         {@link #NETWORK_TYPE_IWLAN}.
         */
        public int getNetworkType() {
            return mNetworkType;
        }

        /**
         * Checks if the IMS registration was done over Wi-Fi network or not.
         *
         * @return true if the IMS registration was done over Wi-Fi network, false otherwise.
         */
        public boolean isNetworkTypeWifi() {
            return mNetworkType == NETWORK_TYPE_IWLAN;
        }

        /**
         * Sets the IMS registration state.
         *
         * @param state The reigstration state.
         */
        public void setState(int state) {
            if (mState != state) {
                mState = state;
                notifyStateChanged();
            }
        }

        /**
         * Sets the network type when IMS is registered.
         *
         * @param networkType The registered network type.
         */
        public void setNetworkType(int networkType) {
            if (mNetworkType != networkType) {
                mNetworkType = networkType;
                notifyStateChanged();
            }
        }

        /**
         * Adds the listener to monitor the registration state.
         *
         * @param listener The listener to be added.
         */
        public void addListener(@NonNull Listener listener) {
            synchronized (mListeners) {
                mListeners.add(listener);
            }
        }

        /**
         * Removes the listener that was previously set.
         *
         * @param listener The listener to be removed.
         */
        public void removeListener(@NonNull Listener listener) {
            synchronized (mListeners) {
                mListeners.remove(listener);
            }
        }

        @Override
        public String toString() {
            return new StringBuilder("[ RegState:")
                    .append(" state=").append(mState)
                    .append(" networkType=").append(mNetworkType)
                    .append(" ]").toString();
        }

        private void notifyStateChanged() {
            AppContext.runTask(() -> {
                synchronized (mListeners) {
                    mListeners.forEach(l -> l.onStateChanged());
                }
            }, 0);
        }
    }

    /**
     * MmTelState stores the MmTel related states.
     */
    public static class MmTelState {
        /** IUMtcService.SERVICE_* */
        private int mRegisteredServiceType = STATE_INACTIVE;
        private int mVoLteProvisioned = STATE_INACTIVE;
        private int mVtProvisioned = STATE_INACTIVE;
        private int mWfcProvisioned = STATE_INACTIVE;
        private final ArraySet<Listener> mListeners = new ArraySet<>();

        MmTelState() {
        }

        /**
         * Initializes the MmTel state.
         */
        public void init() {
            boolean changed = false;

            if (mRegisteredServiceType != STATE_INACTIVE) {
                mRegisteredServiceType = STATE_INACTIVE;
                changed = true;
            }

            if (mVoLteProvisioned != STATE_INACTIVE) {
                mVoLteProvisioned = STATE_INACTIVE;
                changed = true;
            }

            if (mVtProvisioned != STATE_INACTIVE) {
                mVtProvisioned = STATE_INACTIVE;
                changed = true;
            }

            if (mWfcProvisioned != STATE_INACTIVE) {
                mWfcProvisioned = STATE_INACTIVE;
                changed = true;
            }

            if (changed) {
                notifyStateChanged();
            }
        }

        /**
         * Returns the registered service type.
         *
         * @return The registered service type.
         */
        public int getRegisteredServiceType() {
            return mRegisteredServiceType;
        }

        /**
         * Checks if VoLTE is provisioned or not.
         *
         * @return true if VoLte is provisioned, false otherwise.
         */
        public boolean isVoLteProvisioned() {
            return mVoLteProvisioned == STATE_ACTIVE;
        }

        /**
         * Checks if VT is provisioned or not.
         *
         * @return true if VT is provisioned, false otherwise.
         */
        public boolean isVtProvisioned() {
            return mVtProvisioned == STATE_ACTIVE;
        }

        /**
         * Checks if WFC is provisioned or not.
         *
         * @return true if WFC is provisioned, false otherwise.
         */
        public boolean isWfcProvisioned() {
            return mWfcProvisioned == STATE_ACTIVE;
        }

        /**
         * Sets the registered service type.
         *
         * @param serviceType The registered service type. (IUMtcService.SERVICE_*)
         */
        public void setRegisteredServiceType(int serviceType) {
            if (mRegisteredServiceType != serviceType) {
                mRegisteredServiceType = serviceType;
                notifyStateChanged();
            }
        }

        /**
         *  Sets the provisioned status of MmTel service.
         *
         * @param voLteProvisioned The VoLTE provisioned status.
         * @param vtProvisioned The VT provisioned status.
         * @param wfcProvisioned The WFC provisioned status.
         */
        public void setProvisioned(int voLteProvisioned,
                int vtProvisioned, int wfcProvisioned) {
            boolean changed = false;

            if (mVoLteProvisioned != voLteProvisioned) {
                mVoLteProvisioned = voLteProvisioned;
                changed = true;
            }

            if (mVtProvisioned != vtProvisioned) {
                mVtProvisioned = vtProvisioned;
                changed = true;
            }

            if (mWfcProvisioned != wfcProvisioned) {
                mWfcProvisioned = wfcProvisioned;
                changed = true;
            }

            if (changed) {
                notifyStateChanged();
            }
        }

        /**
         * Sets the VoLTE provisioned status.
         *
         * @param voLteProvisioned The VoLTE provisioned status.
         */
        public void setVoLteProvisioned(int voLteProvisioned) {
            if (mVoLteProvisioned != voLteProvisioned) {
                mVoLteProvisioned = voLteProvisioned;
                notifyStateChanged();
            }
        }

        /**
         * Sets the VT provisioned status.
         *
         * @param vtProvisioned The VT provisioned status.
         */
        public void setVtProvisioned(int vtProvisioned) {
            if (mVtProvisioned != vtProvisioned) {
                mVtProvisioned = vtProvisioned;
                notifyStateChanged();
            }
        }

        /**
         * Sets the WFC provisioned status.
         *
         * @param wfcProvisioned The WFC provisioned status.
         */
        public void setWfcProvisioned(int wfcProvisioned) {
            if (mWfcProvisioned != wfcProvisioned) {
                mWfcProvisioned = wfcProvisioned;
                notifyStateChanged();
            }
        }

        /**
         * Adds the listener to monitor the MmTel state.
         *
         * @param listener The listener to be added.
         */
        public void addListener(@NonNull Listener listener) {
            synchronized (mListeners) {
                mListeners.add(listener);
            }
        }

        /**
         * Removes the listener that was previously set.
         *
         * @param listener The listener to be removed.
         */
        public void removeListener(@NonNull Listener listener) {
            synchronized (mListeners) {
                mListeners.remove(listener);
            }
        }

        @Override
        public String toString() {
            return new StringBuilder("[ MmTelState:")
                    .append(" registeredServiceType=").append(mRegisteredServiceType)
                    .append(" voLteProvisioned=").append(mVoLteProvisioned)
                    .append(" vtProvisioned=").append(mVtProvisioned)
                    .append(" wfcProvisioned=").append(mWfcProvisioned)
                    .append(" ]").toString();
        }

        private void notifyStateChanged() {
            AppContext.runTask(() -> {
                synchronized (mListeners) {
                    mListeners.forEach(l -> l.onStateChanged());
                }
            }, 0);
        }
    }

    /**
     * CallState stores the call(MmTel) states.
     */
    public static class CallState {
        private int mState = STATE_INACTIVE;
        private int mConnectedCallOnWifi = STATE_INACTIVE;
        private final ArraySet<Listener> mListeners = new ArraySet<>();

        CallState() {
        }

        /**
         * Initializes the call(MmTel) state.
         */
        public void init() {
            boolean changed = false;

            if (mState != STATE_INACTIVE) {
                mState = STATE_INACTIVE;
                changed = true;
            }

            if (mConnectedCallOnWifi != STATE_INACTIVE) {
                mConnectedCallOnWifi = STATE_INACTIVE;
                changed = true;
            }

            if (changed) {
                notifyStateChanged();
            }
        }

        /**
         * Returns the call state.
         *
         * @return The call state. Valid values are
         *         {@link #STATE_ACTIVE},
         *         {@link #STATE_INACTIVE}.
         */
        public int getState() {
            return mState;
        }

        /**
         * Returns the state that the call is connected on Wi-Fi network.
         *
         * @return The state that the call is connected on Wi-Fi network.
         */
        public int getConnectedCallOnWifi() {
            return mConnectedCallOnWifi;
        }

        /**
         * Sets the call state.
         *
         * @param state The call state. Valid values are
         *              {@link #STATE_ACTIVE},
         *              {@link #STATE_INACTIVE}.
         */
        public void setState(int state) {
            if (mState != state) {
                mState = state;
                notifyStateChanged();
            }
        }

        /**
         * Sets the state that the call is connected on Wi-Fi network.
         *
         * @param state The state that the call is connected on Wi-Fi network. Valid values are
         *              {@link #STATE_ACTIVE},
         *              {@link #STATE_INACTIVE}.
         */
        public void setConnectedCallOnWifi(int state) {
            if (mConnectedCallOnWifi != state) {
                mConnectedCallOnWifi = state;
                notifyStateChanged();
            }
        }

        /**
         * Adds the listener to monitor the call state.
         *
         * @param listener The listener to be added.
         */
        public void addListener(@NonNull Listener listener) {
            synchronized (mListeners) {
                mListeners.add(listener);
            }
        }

        /**
         * Removes the listener that was previously set.
         *
         * @param listener The listener to be removed.
         */
        public void removeListener(@NonNull Listener listener) {
            synchronized (mListeners) {
                mListeners.remove(listener);
            }
        }

        @Override
        public String toString() {
            return new StringBuilder("[ CallState:")
                    .append(" state=").append(mState)
                    .append(" connectedCallOnWifi=").append(mConnectedCallOnWifi)
                    .append(" ]").toString();
        }

        private void notifyStateChanged() {
            AppContext.runTask(() -> {
                synchronized (mListeners) {
                    mListeners.forEach(l -> l.onStateChanged());
                }
            }, 0);
        }
    }

    static @NonNull ImsState getImsState(int phoneId) {
        ImsState imsState = sImsStates.get(phoneId);

        if (imsState == null) {
            imsState = new ImsState();
            sImsStates.put(phoneId, imsState);
        }

        return imsState;
    }

    static class ImsState {
        private final RegState mRegState;
        private final MmTelState mMmTelState;
        private final CallState mCallState;

        ImsState() {
            mRegState = new RegState();
            mMmTelState = new MmTelState();
            mCallState = new CallState();
        }

        public RegState getRegState() {
            return mRegState;
        }

        public MmTelState getMmTelState() {
            return mMmTelState;
        }

        public CallState getCallState() {
            return mCallState;
        }

        @Override
        public String toString() {
            return new StringBuilder("[ ImsState:")
                    .append(" ").append(mRegState.toString())
                    .append(" ").append(mMmTelState.toString())
                    .append(" ").append(mCallState.toString())
                    .append(" ]").toString();
        }
    }
}
