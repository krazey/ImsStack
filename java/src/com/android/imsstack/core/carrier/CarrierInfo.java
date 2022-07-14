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

import android.telephony.TelephonyManager;
import android.util.SparseArray;

import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MSimUtils;

/**
 * This class helps to load carrier codes from "config/carrier_code.xml".
 */
public final class CarrierInfo {
    private static CarrierInfo sCarrierInfo = null;
    private final SparseArray<SimCarrierId> mSimCarrierIds = new SparseArray<>(2);

    private CarrierInfo() {
        int maxSimSlot = MSimUtils.getMaxSimSlot();

        for (int i = 0; i < maxSimSlot; ++i) {
            mSimCarrierIds.put(i, new SimCarrierId.Builder().build());
        }
    }

    /** Returns CarrierInfo instance. */
    public static CarrierInfo getInstance() {
        if (sCarrierInfo == null) {
            sCarrierInfo = new CarrierInfo();
        }

        return sCarrierInfo;
    }

    /**
     * Returns the SimCarrierId object for a specified slot.
     *
     * @param slotId The slot-id to be returned.
     * @return A SimCarrierId object.
     */
    public SimCarrierId getCarrierId(int slotId) {
        return mSimCarrierIds.get(slotId);
    }

    /**
     * Updates the SimCarrierId object newly for a specified slot.
     *
     * @param slotId The slot-id to be updated.
     * @return true if the new SimCarrierId is different from the previous one, false otherwise.
     */
    public boolean updateCarrierId(int slotId) {
        final SimCarrierId oldCid = mSimCarrierIds.get(slotId);

        if (oldCid == null) {
            Log.e(Log.TAG, "Invalid slot: " + slotId);
            return false;
        }

        final SimCarrierId newCid = getCarrierIdFromSim(slotId);

        Log.i(Log.TAG, "CarrierId: old=" + oldCid + ", new=" + newCid);

        mSimCarrierIds.put(slotId, newCid);

        return !(oldCid.getCarrierId() == newCid.getCarrierId()
                && oldCid.getSpecificCarrierId() == newCid.getSpecificCarrierId());
    }

    /**
     * Creates a new SimCarrierId object using the information from the specified SIM slot.
     *
     * @param slotId The slot-id to be created.
     * @return A SimCarrierId object.
     */
    public static SimCarrierId getCarrierIdFromSim(int slotId) {
        SimCarrierId.Builder builder = new SimCarrierId.Builder();
        TelephonyManager tm = getTelephonyManager(slotId);

        if (tm != null) {
            int simState = Sim.getSimStateFromTelephonySimState(tm.getSimApplicationState());

            if (simState == Sim.STATE_LOCKED) {
                builder.setIccId(tm.getSimSerialNumber());
                builder.setSimState(SimCarrierId.SIM_LOCKED);
            } else if (simState == Sim.STATE_LOADED) {
                int testCarrierId = ImsPrivateProperties.Persistent.getInt(
                        ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID, slotId);
                builder.setCarrierId((testCarrierId > 0) ? testCarrierId : tm.getSimCarrierId());
                builder.setSpecificCarrierId(tm.getSimSpecificCarrierId());

                String simOperator = tm.getSimOperator();

                if (simOperator != null && simOperator.length() >= 3) {
                    builder.setMcc(emptyIfNull(simOperator.substring(0, 3)));
                    builder.setMnc(emptyIfNull(simOperator.substring(3)));
                }

                builder.setImsi(emptyIfNull(tm.getSubscriberId()));
                builder.setGid1(emptyIfNull(tm.getGroupIdLevel1()));
                builder.setSpn(emptyIfNull(tm.getSimOperatorName()));
                builder.setIccId(emptyIfNull(tm.getSimSerialNumber()));
                builder.setSimState(SimCarrierId.SIM_LOADED);
            }
        }

        return builder.build();
    }

    /** Sets the current operator/country string. */
    public static void setSimOperatorCountry(
            String operator, String operatorSub, String country, int slotId) {
        Log.i(Log.TAG, "CarrierInfo(" + slotId + "): "
                + "op=" + operator + ", opSub=" + operatorSub + ", co=" + country);

        operator = emptyIfNull(operator);
        operatorSub = emptyIfNull(operatorSub);
        country = emptyIfNull(country);

        ImsPrivateProperties.Persistent.set(
                ImsPrivateProperties.Persistent.KEY_SIM_OPERATOR,
                operator, slotId);
        ImsPrivateProperties.Persistent.set(
                ImsPrivateProperties.Persistent.KEY_SIM_OPERATOR_SUB,
                operatorSub, slotId);
        ImsPrivateProperties.Persistent.set(
                ImsPrivateProperties.Persistent.KEY_SIM_COUNTRY,
                country, slotId);
    }

    private static TelephonyManager getTelephonyManager(int slotId) {
        if (MSimUtils.isMultiSimEnabled()) {
            int subId = MSimUtils.getSubId(slotId);
            if (MSimUtils.isValidSubId(subId)) {
                return AppContext.getTelephonyManager(subId);
            }

            return null;
        }

        return AppContext.getTelephonyManager();
    }

    private static String emptyIfNull(String s) {
        return s == null ? "" : s;
    }
}
