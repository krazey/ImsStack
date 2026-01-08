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
package com.android.imsstack.core.carrier;

import android.annotation.NonNull;
import android.telephony.TelephonyManager;

import com.android.imsstack.util.Log;

import java.util.Objects;

/**
 * This class provides the carrier information from SIM card.
 */
public final class SimCarrierId {
    /** Unknown carrier id. */
    public static final int UNKNOWN_ID = TelephonyManager.UNKNOWN_CARRIER_ID;
    /** Carrier id for Google Fi SIM. */
    public static final int GOOGLE_FI = 1989;
    /** SIM states */
    public static final int SIM_ABSENT = 0;
    public static final int SIM_LOCKED = 1;
    public static final int SIM_LOADED = 2;

    private final int mCarrierId;
    private final int mSpecificCarrierId;
    private final String mMcc;
    private final String mMnc;
    private final String mImsi;
    private final String mGid1;
    private final String mSpn;
    private final String mIccId;
    private final int mSimState;

    private SimCarrierId(int carrierId, int specificCarrierId,
            @NonNull String mcc, @NonNull String mnc, @NonNull String imsi,
            @NonNull String gid1, @NonNull String spn, @NonNull String iccId,
            int simState) {
        mCarrierId = carrierId;
        mSpecificCarrierId = specificCarrierId;
        mMcc = mcc;
        mMnc = mnc;
        mImsi = imsi;
        mGid1 = gid1;
        mSpn = spn;
        mIccId = iccId;
        mSimState = simState;
    }

    /**
     * Returns the carrier id.
     */
    public int getCarrierId() {
        return mCarrierId;
    }

    /**
     * Returns the specific carrier id.
     */
    public int getSpecificCarrierId() {
        return mSpecificCarrierId;
    }

    /**
     * Returns the ICCID string.
     */
    public @NonNull String getIccId() {
        return mIccId;
    }

    /**
     * Returns the MCC (3 digits) string.
     */
    public @NonNull String getMcc() {
        return mMcc;
    }

    /**
     * Returns the MNC (2 or 3 digits) string.
     */
    public @NonNull String getMnc() {
        return mMnc;
    }

    /**
     * Returns the IMSI string.
     */
    public @NonNull String getImsi() {
        return mImsi;
    }

    /**
     * Returns the GID1 string.
     */
    public @NonNull String getGid1() {
        return mGid1;
    }

    /**
     * Returns the SPN string.
     */
    public @NonNull String getSpn() {
        return mSpn;
    }

    /**
     * Checks if the SIM is in LOCKED state.
     */
    public boolean isSimLocked() {
        return mSimState == SIM_LOCKED;
    }

    /**
     * Checks if the SIM is in ABSENT state.
     */
    public boolean isSimAbsent() {
        return mSimState == SIM_ABSENT;
    }

    /**
     * Checks if the SIM is in LOADED state.
     */
    public boolean isSimLoaded() {
        return mSimState == SIM_LOADED;
    }

    @Override
    public int hashCode() {
        return Objects.hash(mCarrierId, mSpecificCarrierId,
                mMcc, mMnc, mImsi, mGid1, mSpn, mIccId);
    }

    @Override
    public boolean equals(Object other) {
        if (this == other) {
            return true;
        }

        if (other == null || getClass() != other.getClass()) {
            return false;
        }

        SimCarrierId id = (SimCarrierId) other;

        if (mCarrierId == UNKNOWN_ID && id.mCarrierId == UNKNOWN_ID
                && mSpecificCarrierId == UNKNOWN_ID && id.mSpecificCarrierId == UNKNOWN_ID) {
            return Objects.equals(mMcc, id.mMcc)
                    && Objects.equals(mMnc, id.mMnc)
                    && Objects.equals(mImsi, id.mImsi)
                    && Objects.equals(mGid1, id.mGid1)
                    && Objects.equals(mSpn, id.mSpn)
                    && Objects.equals(mIccId, id.mIccId);
        }

        return (mCarrierId == id.mCarrierId)
                && (mSpecificCarrierId == id.mSpecificCarrierId);
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();

        sb.append("[ SimCarrierId: carrierId=");
        sb.append(mCarrierId);
        sb.append(", specificCarrierId=");
        sb.append(mSpecificCarrierId);
        sb.append(", mcc=");
        sb.append(mMcc);
        sb.append(", mnc=");
        sb.append(mMnc);
        sb.append(", imsi=");
        sb.append(Log.pii(mImsi));
        sb.append(", gid1=");
        sb.append(mGid1);
        sb.append(", spn=");
        sb.append(mSpn);
        sb.append(", iccId=");
        sb.append(Log.pii(mIccId));
        sb.append(", simState=");
        sb.append(mSimState);
        sb.append(" ]");

        return sb.toString();
    }

    /**
     * Builder class for {@link SimCarrierId} object.
     */
    public static final class Builder {
        private int mCarrierId = UNKNOWN_ID;
        private int mSpecificCarrierId = UNKNOWN_ID;
        private String mMcc = "";
        private String mMnc = "";
        private String mImsi = "";
        private String mGid1 = "";
        private String mSpn = "";
        private String mIccId = "";
        private int mSimState = SIM_ABSENT;

        /** A default constructor for a new Builder instance. */
        public Builder() {}

        /**
         * Sets the carrier identifier.
         *
         * @param carrierId The carrier id.
         * @return The same Builder instance.
         */
        public Builder setCarrierId(int carrierId) {
            mCarrierId = carrierId;
            return this;
        }

        /**
         * Sets the specific carrier identifier.
         *
         * @param specificCarrierId The specific carrier id.
         * @return The same Builder instance.
         */
        public Builder setSpecificCarrierId(int specificCarrierId) {
            mSpecificCarrierId = specificCarrierId;
            return this;
        }

        /**
         * Sets the MCC (3 digits) string.
         *
         * @param mcc The MCC string.
         * @return The same Builder instance.
         */
        public Builder setMcc(@NonNull String mcc) {
            mMcc = mcc;
            return this;
        }

        /**
         * Sets the MNC (2 or 3 digits) string.
         *
         * @param mnc The MNC string.
         * @return The same Builder instance.
         */
        public Builder setMnc(@NonNull String mnc) {
            mMnc = mnc;
            return this;
        }

        /**
         * Sets the IMSI string.
         *
         * @param imsi The IMSI string.
         * @return The same Builder instance.
         */
        public Builder setImsi(@NonNull String imsi) {
            mImsi = imsi;
            return this;
        }

        /**
         * Sets the GID1 string.
         *
         * @param gid1 The GID1 string.
         * @return The same Builder instance.
         */
        public Builder setGid1(@NonNull String gid1) {
            mGid1 = gid1;
            return this;
        }

        /**
         * Sets the SPN (Servicer Provider Name) string.
         *
         * @param spn The SPN string.
         * @return The same Builder instance.
         */
        public Builder setSpn(@NonNull String spn) {
            mSpn = spn;
            return this;
        }

        /**
         * Sets the ICCID string.
         *
         * @param iccId The ICCID string.
         * @return The same Builder instance.
         */
        public Builder setIccId(@NonNull String iccId) {
            mIccId = iccId;
            return this;
        }

        /**
         * Sets the current SIM state.
         *
         * @param state The current SIM state.
         *              {@link SimCarrierId#SIM_ABSENT},
         *              {@link SimCarrierId#SIM_LOCKED},
         *              {@link SimCarrierId#SIM_LOADED}
         * @return The same Builder instance.
         */
        public Builder setSimState(int state) {
            mSimState = state;
            return this;
        }

        /**
         * Creates a new instance of SimCarrierId.
         *
         * @return A new SimCarrierId object.
         */
        public SimCarrierId build() {
            return new SimCarrierId(mCarrierId, mSpecificCarrierId,
                    mMcc, mMnc, mImsi, mGid1, mSpn, mIccId, mSimState);
        }
    }
}
