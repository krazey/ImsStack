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

package com.android.imsstack.core.agents;

import android.annotation.IntDef;
import android.content.Intent;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.imsstack.base.MSimUtils;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;

/**
 * This interface provides the definition of listener and constant values to monitor and access
 * the SIM related information.
 */
public interface Sim {
    /**
     * SIM application types.
     */
    int APP_TYPE_UNKNOWN = TelephonyManager.APPTYPE_UNKNOWN;
    int APP_TYPE_USIM = TelephonyManager.APPTYPE_USIM;
    int APP_TYPE_ISIM = TelephonyManager.APPTYPE_ISIM;

    @IntDef(value = {
        APP_TYPE_UNKNOWN,
        APP_TYPE_USIM,
        APP_TYPE_ISIM
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface AppType {}

    /**
     * SIM states.
     */
    int STATE_INVALID = -1; // Internal usage.
    int STATE_UNKNOWN = TelephonyManager.SIM_STATE_UNKNOWN;
    int STATE_ABSENT = TelephonyManager.SIM_STATE_ABSENT;
    int STATE_PIN_REQUIRED = TelephonyManager.SIM_STATE_PIN_REQUIRED;
    int STATE_PUK_REQUIRED = TelephonyManager.SIM_STATE_PUK_REQUIRED;
    int STATE_NETWORK_LOCKED = TelephonyManager.SIM_STATE_NETWORK_LOCKED;
    int STATE_READY = TelephonyManager.SIM_STATE_READY;
    int STATE_NOT_READY = TelephonyManager.SIM_STATE_NOT_READY;
    int STATE_PERM_DISABLED = TelephonyManager.SIM_STATE_PERM_DISABLED;
    int STATE_CARD_IO_ERROR = TelephonyManager.SIM_STATE_CARD_IO_ERROR;
    int STATE_CARD_RESTRICTED = TelephonyManager.SIM_STATE_CARD_RESTRICTED;
    int STATE_LOADED = TelephonyManager.SIM_STATE_LOADED;
    int STATE_PRESENT = TelephonyManager.SIM_STATE_PRESENT;
    int STATE_LOCKED = 21;

    @IntDef(value = {
        STATE_INVALID,
        STATE_UNKNOWN,
        STATE_ABSENT,
        STATE_PIN_REQUIRED,
        STATE_PUK_REQUIRED,
        STATE_NETWORK_LOCKED,
        STATE_READY,
        STATE_NOT_READY,
        STATE_PERM_DISABLED,
        STATE_CARD_IO_ERROR,
        STATE_CARD_RESTRICTED,
        STATE_LOCKED,
        STATE_LOADED,
        STATE_PRESENT
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface State {}

    /**
     * ISIM states.
     */
    // Initial ISIM state
    int ISIM_STATE_UNKNOWN = -1;
    // When ISIM application is not present.
    int ISIM_STATE_NOT_PRESENT = 0;
    // When ISIM application is present, but ISIM records are not ready.
    int ISIM_STATE_NOT_READY = 1;
    // When all the ISIM records are available.
    int ISIM_STATE_LOADED = 2;
    // When ISIM refresh is started.
    int ISIM_STATE_REFRESH_STARTED = 3;
    // When ISIM refresh is completed. Used for internal and native layer.
    int ISIM_STATE_REFRESH_COMPLETED = 4;
    // When SIM is removed.
    int ISIM_STATE_REMOVED = 5;

    @IntDef(value = {
        ISIM_STATE_UNKNOWN,
        ISIM_STATE_NOT_PRESENT,
        ISIM_STATE_NOT_READY,
        ISIM_STATE_LOADED,
        ISIM_STATE_REFRESH_STARTED,
        ISIM_STATE_REFRESH_COMPLETED,
        ISIM_STATE_REMOVED
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface IsimState {}

    /**
     * ISIM EF types.
     */
    int ISIM_FILE_ID_IMPI = 0x6F02;
    int ISIM_FILE_ID_DOMAIN = 0x6F03;
    int ISIM_FILE_ID_IMPU = 0x6F04;

    @IntDef(value = {
        ISIM_FILE_ID_IMPI,
        ISIM_FILE_ID_DOMAIN,
        ISIM_FILE_ID_IMPU
    })
    @Retention(RetentionPolicy.SOURCE)
    public @interface IsimFileId {}

    /**
     * Listener interface to receive the change notification of SIM state.
     */
    public interface Listener {
        /**
         * Notifies the application that SIM card state is changed.
         */
        default void onSimCardStateChanged() {
        }

        /**
         * Notifies the application that SIM state is changed.
         */
        default void onSimStateChanged() {
        }
    }

    /**
     * Listener interface to receive the change notification of ISIM state.
     */
    public interface IsimListener {
        /**
         * Notifies the application that ISIM state is changed.
         */
        default void onIsimStateChanged() {
        }
    }

    /**
     * Returns the SIM state of this interface represented by
     * the given telephony SIM card state.
     *
     * @param cardState The telephony SIM card state.
     * @return A SIM state.
     */
    static @Sim.State int getSimCardStateFromTelephonySimState(int cardState) {
        int simCardState = getSimStateFromTelephonySimState(cardState);

        switch (simCardState) {
            case Sim.STATE_UNKNOWN:
                // If the card state is unknown, it is handled as an ABSENT.
                return Sim.STATE_ABSENT;
            case Sim.STATE_ABSENT: // FALL-THROUGH
            case Sim.STATE_PRESENT:
                return simCardState;
            default:
                return Sim.STATE_INVALID;
        }
    }

    /**
     * Returns the SIM state of this interface represented by
     * the given telephony SIM application state.
     *
     * @param state The telephony SIM application state.
     * @return A SIM state.
     */
    static @Sim.State int getSimStateFromTelephonySimState(int state) {
        switch (state) {
            case TelephonyManager.SIM_STATE_UNKNOWN: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_CARD_IO_ERROR: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_CARD_RESTRICTED:
                return Sim.STATE_UNKNOWN;
            case TelephonyManager.SIM_STATE_ABSENT:
                return Sim.STATE_ABSENT;
            case TelephonyManager.SIM_STATE_PRESENT:
                return Sim.STATE_PRESENT;
            case TelephonyManager.SIM_STATE_NOT_READY: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_READY:
                return Sim.STATE_NOT_READY;
            case TelephonyManager.SIM_STATE_PIN_REQUIRED: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_PUK_REQUIRED: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_NETWORK_LOCKED: // FALL-THROUGH
            case TelephonyManager.SIM_STATE_PERM_DISABLED:
                return Sim.STATE_LOCKED;
            case TelephonyManager.SIM_STATE_LOADED:
                return Sim.STATE_LOADED;
            default:
                return Sim.STATE_INVALID;
        }
    }

    /** Gets the SIM state from the given intent. */
    static int getExtraSimState(Intent intent) {
        return intent.getIntExtra(TelephonyManager.EXTRA_SIM_STATE, Sim.STATE_INVALID);
    }

    /** Gets the slot index from the given intent. */
    static int getExtraSlotIndex(Intent intent) {
        return getExtraSlotIndex(intent, MSimUtils.INVALID_SLOT_ID);
    }

    /** Gets the slot index from the given intent. */
    static int getExtraSlotIndex(Intent intent, int defaultValue) {
        return intent.getIntExtra(SubscriptionManager.EXTRA_SLOT_INDEX, defaultValue);
    }

    /** Gets the subscription index from the given intent. */
    static int getExtraSubscriptionIndex(Intent intent) {
        return getExtraSubscriptionIndex(intent, MSimUtils.INVALID_SUB_ID);
    }

    /** Gets the subscription index from the given intent. */
    static int getExtraSubscriptionIndex(Intent intent, int defaultValue) {
        return intent.getIntExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, defaultValue);
    }

    /** Returns a string represented by the given SIM state. */
    static String stateToString(@Sim.State int state) {
        switch (state) {
            case Sim.STATE_UNKNOWN:
                return "UNKNOWN";
            case Sim.STATE_ABSENT:
                return "ABSENT";
            case Sim.STATE_PIN_REQUIRED:
                return "PIN_REQUIRED";
            case Sim.STATE_PUK_REQUIRED:
                return "PUK_REQUIRED";
            case Sim.STATE_NETWORK_LOCKED:
                return "NETWORK_LOCKED";
            case Sim.STATE_PERM_DISABLED:
                return "PERM_DISABLED";
            case Sim.STATE_READY:
                return "READY";
            case Sim.STATE_NOT_READY:
                return "NOT_READY";
            case Sim.STATE_CARD_IO_ERROR:
                return "CARD_IO_ERROR";
            case Sim.STATE_CARD_RESTRICTED:
                return "CARD_RESTRICTED";
            case Sim.STATE_LOCKED:
                return "LOCKED";
            case Sim.STATE_LOADED:
                return "LOADED";
            case Sim.STATE_PRESENT:
                return "PRESENT";
            default:
                return "INVALID";
        }
    }

    /** Returns a string representation by the given ISIM file id. */
    static String isimFileIdToString(@Sim.IsimFileId int fileId) {
        switch (fileId) {
            case Sim.ISIM_FILE_ID_IMPI:
                return "IMPI";
            case Sim.ISIM_FILE_ID_DOMAIN:
                return "DOMAIN";
            case Sim.ISIM_FILE_ID_IMPU:
                return "IMPU";
            default:
                return "UNKNOWN";
        }
    }
}
