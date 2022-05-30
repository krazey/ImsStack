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
import android.telephony.TelephonyManager;

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
}
