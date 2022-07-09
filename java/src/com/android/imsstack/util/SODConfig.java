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
package com.android.imsstack.util;

import android.content.Context;
import android.content.SharedPreferences;
import android.content.SharedPreferences.Editor;

import java.util.Locale;
import java.util.Objects;

/**
 * This class provides the constant values and utility methods
 * for SIMOperatorDetector / OperatorInfo related operations.
 */
public final class SODConfig {
    /** supported services */
    public static final int SUPPORT_NONE = 0x00000000;
    public static final int SUPPORT_VOLTE = 0x00000001;
    public static final int SUPPORT_VOWIFI = 0x00000002;
    public static final int SUPPORT_VT = 0x00000004;
    public static final int SUPPORT_VOLTE_EMERGENCY = 0x00000008;

    /** SIM states */
    public static final int SIM_STATE_NONE = 0x00;
    public static final int SIM_STATE_UNKNOWN = 0x01;
    public static final int SIM_STATE_ABSENT = 0x02;
    public static final int SIM_STATE_LOADED = 0x04;
    public static final int SIM_STATE_LOCKED = 0x08;
    public static final int SIM_STATE_REMOVED = 0x10;

    /** device information */
    public static final int TARGET_BASED = 0x00010000;
    public static final int SIM_BASED = 0x00020000;
    public static final int SUPPORT_SIM_MOVED = 0x00000001;
    public static final int TARGET_OPEN = 0x00000002;
    public static final int NA_OPEN = 0x00000004;
    public static final int SMS_ONLY = 0x00000008;
    public static final int KR_OPEN = 0x00000010;

    public static class Operator {
        private final int mSlotId;
        private boolean mAvailable = false;
        private boolean mActive = false;
        private boolean mInboundRoaming = false;

        // Copied from Sim class and managed differently comparing to Sim's information
        private int mSubId = (-1);
        private String mMccMnc = "";

        private String mOperator = "";
        private String mCountry = "";
        private String mRegion = "";
        private String mGroupId = "";
        private String mCategory ="";
        private String mEnablerType = ENABLER_TYPE_GLOBAL;
        private boolean mUnknownOperator = false;
        private boolean mConfigPerModel = false;
        // Internal usage to identify whether SIM-MOVED feature is enabled
        private boolean mSimMoved = false;

        private int mSupportedServices = SUPPORT_NONE;

        public Operator(int slotId) {
            mSlotId = slotId;
        }

        public int getSlotId() {
            return mSlotId;
        }

        public boolean isAvailable() {
            return mAvailable;
        }

        public boolean isActive() {
            return mActive;
        }

        public boolean isInboundRoaming() {
            return mInboundRoaming;
        }

        public int getSubId() {
            return mSubId;
        }

        public String getMccMnc() {
            return mMccMnc;
        }

        public String getOperator() {
            return mOperator;
        }

        public String getCountry() {
            return mCountry;
        }

        public String getRegion() {
            return mRegion;
        }

        public String getGroupId() {
            return mGroupId;
        }

        public String getCategory() {
            return mCategory;
        }

        public String getEnablerType() {
            return mEnablerType;
        }

        public boolean isUnknownOperator() {
            return mUnknownOperator;
        }

        public boolean isConfigPerModel() {
            return mConfigPerModel;
        }

        public boolean isSimMoved() {
            return mSimMoved;
        }

        public int getServices() {
            return mSupportedServices;
        }

        public void setAvailable(boolean available) {
            mAvailable = available;
        }

        public void setActive(boolean active) {
            mActive = active;
        }

        public void setInboundRoaming(boolean inboundRoaming) {
            mInboundRoaming = inboundRoaming;
        }

        public void setSubId(int subId) {
            mSubId = subId;
        }

        public void setMccMnc(String mccmnc) {
            mMccMnc = mccmnc;
        }

        public void setOperator(String operator) {
            mOperator = operator;
        }

        public void setCountry(String country) {
            mCountry = country;
        }

        public void setRegion(String region) {
            mRegion = region;
        }

        public void setGroupId(String groupId) {
            mGroupId = groupId;
        }

        public void setCategory(String category) {
            mCategory = category;
        }

        public void setEnablerType(String enablerType) {
            mEnablerType = enablerType;
        }

        public void setUnknownOperator(boolean unknownOperator) {
            mUnknownOperator = unknownOperator;
        }

        public void setConfigPerModel(boolean configPerModel) {
            mConfigPerModel = configPerModel;
        }

        public void setSimMoved(boolean simMoved) {
            mSimMoved = simMoved;
        }

        public void setServices(int services) {
            mSupportedServices = services;
        }

        public void setService(int service) {
            mSupportedServices |= service;
        }

        public void resetService(int service) {
            mSupportedServices &= (~service);
        }

        public boolean isServiceOn(int service) {
            return (mSupportedServices & service) == service;
        }

        public void copyFrom(Operator op) {
            if (op == null) {
                return;
            }

            mAvailable = op.mAvailable;
            mActive = op.mActive;
            mInboundRoaming = op.mInboundRoaming;

            mSubId = op.mSubId;
            mMccMnc = op.mMccMnc;

            mOperator = op.mOperator;
            mCountry = op.mCountry;
            mRegion = op.mRegion;
            mGroupId = op.mGroupId;
            mCategory = op.mCategory;
            mEnablerType = op.mEnablerType;
            mUnknownOperator = op.mUnknownOperator;
            mConfigPerModel = op.mConfigPerModel;
            mSimMoved = op.mSimMoved;

            mSupportedServices = op.mSupportedServices;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();

            sb.append("[ OP: ");
            sb.append("slotId=");
            sb.append(getSlotId());
            sb.append(", available=");
            sb.append(isAvailable() ? 1 : 0);
            sb.append(", active=");
            sb.append(isActive() ? 1 : 0);
            sb.append(", inboundRoaming=");
            sb.append(isInboundRoaming() ? 1 : 0);
            sb.append(", subId=");
            sb.append(getSubId());
            sb.append(", mccmnc=");
            sb.append(getMccMnc());
            sb.append(", operator=");
            sb.append(getOperator());
            sb.append(", country=");
            sb.append(getCountry());

            sb.append(", volte=");
            sb.append(isServiceOn(SODConfig.SUPPORT_VOLTE) ? 1 : 0);
            sb.append(", vt=");
            sb.append(isServiceOn(SODConfig.SUPPORT_VT) ? 1 : 0);
            sb.append(", vowifi=");
            sb.append(isServiceOn(SODConfig.SUPPORT_VOWIFI) ? 1 : 0);

            sb.append(", region=");
            sb.append(getRegion());
            sb.append(", groupId=");
            sb.append(getGroupId());
            sb.append(", category=");
            sb.append(getCategory());
            sb.append(", enablerType=");
            sb.append(getEnablerType());
            sb.append(", unknownOperator=");
            sb.append(isUnknownOperator() ? 1 : 0);
            sb.append(", configPerModel=");
            sb.append(isConfigPerModel() ? 1 : 0);
            sb.append(", simMoved=");
            sb.append(isSimMoved() ? 1 : 0);
            sb.append(" ]");

            return sb.toString();
        }
    }

    public static class Sim {
        private int mSubId = (-1);
        private int mState = SODConfig.SIM_STATE_NONE;
        private String mOperator = "";
        private String mCountry = "";
        private String mMccMnc = "";
        private String mGid = "";
        private String mSpn = "";
        private String mImsi = "";

        public Sim() {
        }

        public int getSubId() {
            return mSubId;
        }

        public int getState() {
            return mState;
        }

        public String getOperator() {
            return mOperator;
        }

        public String getCountry() {
            return mCountry;
        }

        public String getMccMnc() {
            return mMccMnc;
        }

        public String getGid() {
            return mGid;
        }

        public String getSpn() {
            return mSpn;
        }

        public String getImsi() {
            return mImsi;
        }

        public void setSubId(int subId) {
            mSubId = subId;
        }

        public void clearState() {
            mState = SIM_STATE_NONE;
        }

        public void setState(int state) {
            mState |= state;
        }

        public void setOperator(String operator) {
            mOperator = operator;
        }

        public void setCountry(String country) {
            mCountry = country;
        }

        public void setMccMnc(String mccmnc) {
            mMccMnc = mccmnc;
        }

        public void setGid(String gid) {
            mGid = gid;
        }

        public void setSpn(String spn) {
            mSpn = spn;
        }

        public void setImsi(String imsi) {
            mImsi = imsi;
        }

        public boolean isStateOn(int state) {
            return (mState & state) == state;
        }

        public void copyFrom(Sim sim) {
            if (sim == null) {
                return;
            }

            mSubId = sim.mSubId;
            mState = sim.mState;
            mOperator = sim.mOperator;
            mCountry = sim.mCountry;
            mMccMnc = sim.mMccMnc;
            mGid = sim.mGid;
            mSpn = sim.mSpn;
            mImsi = sim.mImsi;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();

            sb.append("[ SIM: ");
            sb.append("subId=");
            sb.append(getSubId());
            sb.append(", state=0x");
            sb.append(Integer.toHexString(getState()));
            sb.append(", operator=");
            sb.append(getOperator());
            sb.append(", country=");
            sb.append(getCountry());
            sb.append(", mccmnc=");
            sb.append(getMccMnc());
            sb.append(", gid=");
            sb.append(getGid());
            sb.append(", spn=");
            sb.append(getSpn());
            sb.append(", imsi=");
            sb.append(Log.pii(getImsi()));
            sb.append(" ]");

            return sb.toString();
        }
    }

    public static class SimProperties {
        private String mCountry = "";
        private String mOperator = "";
        private String mOperatorSub = "";

        public SimProperties() {
        }

        public String getCountry() {
            return mCountry;
        }

        public String getOperator() {
            return mOperator;
        }

        public String getOperatorSub() {
            return mOperatorSub;
        }

        public void setCountry(String country) {
            mCountry = country;
        }

        public void setOperator(String operator) {
            mOperator = operator;
        }

        public void setOperatorSub(String operatorSub) {
            mOperatorSub = operatorSub;
        }
    }

    public static class Device {
        private int mOperatorBasedOn = 0x00000000;
        private boolean mSupportSimMoved = false;

        public Device() {
        }

        public int getOperatorBasedOn() {
            return mOperatorBasedOn;
        }

        public boolean isOperatorBasedOn(int operatorBasedOn) {
            return (mOperatorBasedOn & operatorBasedOn) == operatorBasedOn;
        }

        public boolean isSimMovedSupported() {
            return mSupportSimMoved;
        }

        public void clearOperatorBasedOn() {
            mOperatorBasedOn = 0x00000000;
        }

        public void setOperatorBasedOn(int operatorBasedOn) {
            mOperatorBasedOn |= operatorBasedOn;
        }

        public void resetOperatorBasedOn(int operatorBasedOn) {
            mOperatorBasedOn &= (~operatorBasedOn);
        }

        public void setSimMovedSupported(boolean supported) {
            mSupportSimMoved = supported;
        }

        public void copyFrom(Device dev) {
            if (dev == null) {
                return;
            }

            mOperatorBasedOn = dev.mOperatorBasedOn;
            mSupportSimMoved = dev.mSupportSimMoved;
        }
    }

    public static boolean equalsOperator(String op1, String op2) {
        return Objects.equals(op1, op2);
    }

    public static boolean equalsCountry(String co1, String co2) {
        return Objects.equals(co1, co2);
    }

    public static boolean equalsOperatorCountry(String op1, String co1, String op2, String co2) {
        return equalsOperator(op1, op2) && equalsCountry(co1, co2);
    }

    public static boolean isEnablerTypeCanada(String type) {
        return ENABLER_TYPE_CANADA.equals(type);
    }

    public static boolean isEnablerTypeGlobal(String type) {
        return ENABLER_TYPE_GLOBAL.equals(type);
    }

    public static boolean isEnablerTypeForNonOperator(String type) {
        return isEnablerTypeCanada(type) || isEnablerTypeGlobal(type);
    }

    /** key names */
    public static final String KEY_OPERATOR = "operator";
    public static final String KEY_COUNTRY = "country";
    public static final String KEY_SIM_OPERATOR = "sim_operator";
    public static final String KEY_SIM_COUNTRY = "sim_country";
    public static final String KEY_MCCMNC = "mccmnc";
    public static final String KEY_GID = "gid";
    public static final String KEY_SPN = "spn";
    public static final String KEY_IMSI = "imsi";

    /** enabler types */
    public static final String ENABLER_TYPE_OPERATOR = "operator";
    public static final String ENABLER_TYPE_GLOBAL = "global";
    public static final String ENABLER_TYPE_CANADA = "canada";

    /** additional information */
    public static final String OPERATOR_BASED_ON_SIM = "sim";
    public static final String OPERATOR_BASED_ON_TARGET = "target";

    /** preference file name for operator-info */
    private static final String PREFERENCE_OPERATOR_INFO = "operator_info";

    public static String getKeyForSlot(int slotId, String key) {
        return String.format(Locale.US, "%s_%d", key, slotId);
    }

    public static String getValueForCachedOperatorInfo(String key) {
        SharedPreferences pref = AppContext.getInstance().getSharedPreferences(
                PREFERENCE_OPERATOR_INFO, Context.MODE_PRIVATE);

        try {
            return (pref != null) ? pref.getString(key, "") : "";
        } catch (Exception e) {
            e.printStackTrace();
        }

        return null;
    }

    public static String getValueForCachedOperatorInfo(int slotId, String key) {
        String keyForSlot = getKeyForSlot(slotId, key);
        return getValueForCachedOperatorInfo(keyForSlot);
    }

    public static void setValueForCachedOperatorInfo(String key, String value) {
        SharedPreferences pref = AppContext.getInstance().getSharedPreferences(
                PREFERENCE_OPERATOR_INFO, Context.MODE_PRIVATE);

        if (pref == null) {
            return;
        }

        Editor editor = pref.edit();

        if (editor != null) {
            if (value != null) {
                editor.putString(key, value);
            } else {
                editor.remove(key);
            }
            editor.commit();
        }
    }

    public static void setValueForCachedOperatorInfo(int slotId, String key, String value) {
        String keyForSlot = getKeyForSlot(slotId, key);
        setValueForCachedOperatorInfo(keyForSlot, value);
    }
}
