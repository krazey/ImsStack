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

import android.util.SparseArray;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.ImsPrivateProperties;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.util.Log;
import com.android.internal.annotations.VisibleForTesting;

/**
 * This class helps to load carrier codes from "config/carrier_code.xml".
 */
public final class CarrierInfo {
    private static CarrierInfo sCarrierInfo = null;
    private final SparseArray<SimCarrierId> mSimCarrierIds;

    CarrierInfo() {
        int supportedSimCount = DeviceConfig.getSupportedSimCount();

        mSimCarrierIds = new SparseArray<>(supportedSimCount);

        for (int i = 0; i < supportedSimCount; ++i) {
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
     * Clears all the resources for this class.
     */
    @VisibleForTesting
    public static void clear() {
        sCarrierInfo = null;
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
            Log.e(this, "Invalid slot: " + slotId);
            return false;
        }

        final SimCarrierId newCid = getCarrierIdFromSim(slotId);

        Log.i(this, "CarrierId: old=" + oldCid + ", new=" + newCid);

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
        TelephonyManagerProxy tmp = getTelephonyManagerProxy(slotId);

        if (tmp != null) {
            int simState = Sim.getSimStateFromTelephonySimState(tmp.getSimApplicationState());

            if (simState == Sim.STATE_LOCKED) {
                builder.setIccId(tmp.getSimSerialNumber());
                builder.setSimState(SimCarrierId.SIM_LOCKED);
            } else if (simState == Sim.STATE_LOADED) {
                int testCarrierId = ImsPrivateProperties.Persistent.getInt(
                        ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID, slotId);
                int testSpecificCarrierId = ImsPrivateProperties.Persistent.getInt(
                        ImsPrivateProperties.Persistent.KEY_TEST_SPECIFIC_CARRIER_ID, slotId);
                if (testCarrierId > 0) {
                    builder.setCarrierId(testCarrierId);
                    builder.setSpecificCarrierId(
                            (testSpecificCarrierId > 0)
                            ? testSpecificCarrierId
                            : SimCarrierId.UNKNOWN_ID);
                } else {
                    builder.setCarrierId(tmp.getSimCarrierId());
                    builder.setSpecificCarrierId(
                            (testSpecificCarrierId > 0)
                            ? testSpecificCarrierId
                            : tmp.getSimSpecificCarrierId());
                }

                String simOperator = tmp.getSimOperator();

                if (simOperator != null && simOperator.length() >= 3) {
                    builder.setMcc(emptyIfNull(simOperator.substring(0, 3)));
                    builder.setMnc(emptyIfNull(simOperator.substring(3)));
                }

                builder.setImsi(emptyIfNull(tmp.getSubscriberId()));
                builder.setGid1(emptyIfNull(tmp.getGroupIdLevel1()));
                builder.setSpn(emptyIfNull(tmp.getSimOperatorName()));
                builder.setIccId(emptyIfNull(tmp.getSimSerialNumber()));
                builder.setSimState(SimCarrierId.SIM_LOADED);
            }
        }

        return builder.build();
    }

    /** Sets the current operator/country string. */
    public static void setSimOperatorCountry(
            String operator, String operatorSub, String country, int slotId) {
        Log.i(CarrierInfo.class, "CarrierInfo(" + slotId + "): "
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

    private static TelephonyManagerProxy getTelephonyManagerProxy(int slotId) {
        if (DeviceConfig.isMultiSimEnabled()) {
            int subId = MSimUtils.getSubId(slotId);
            if (MSimUtils.isValidSubId(subId)) {
                return AppContext.getTelephonyManagerProxy(subId);
            }

            return null;
        }

        return AppContext.getInstance().getSystemServiceProxy(TelephonyManagerProxy.class);
    }

    private static String emptyIfNull(String s) {
        return s == null ? "" : s;
    }
}
