/*
 * Copyright (C) 2024 The Android Open Source Project
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
package com.android.imsstack.its.base;

import static android.telephony.AccessNetworkConstants.TRANSPORT_TYPE_WLAN;
import static android.telephony.AccessNetworkConstants.TRANSPORT_TYPE_WWAN;
import static android.telephony.DataSpecificRegistrationInfo.LTE_ATTACH_EXTRA_INFO_NONE;
import static android.telephony.DataSpecificRegistrationInfo.LTE_ATTACH_TYPE_COMBINED;
import static android.telephony.DataSpecificRegistrationInfo.LTE_ATTACH_TYPE_UNKNOWN;
import static android.telephony.LteVopsSupportInfo.LTE_STATUS_NOT_SUPPORTED;
import static android.telephony.LteVopsSupportInfo.LTE_STATUS_SUPPORTED;
import static android.telephony.NetworkRegistrationInfo.DOMAIN_CS;
import static android.telephony.NetworkRegistrationInfo.DOMAIN_PS;
import static android.telephony.NetworkRegistrationInfo.REGISTRATION_STATE_EMERGENCY;
import static android.telephony.NetworkRegistrationInfo.REGISTRATION_STATE_HOME;
import static android.telephony.NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
import static android.telephony.NrVopsSupportInfo.NR_STATUS_EMC_5GCN_ONLY;
import static android.telephony.NrVopsSupportInfo.NR_STATUS_EMC_NOT_SUPPORTED;
import static android.telephony.NrVopsSupportInfo.NR_STATUS_EMF_5GCN_ONLY;
import static android.telephony.NrVopsSupportInfo.NR_STATUS_EMF_NOT_SUPPORTED;
import static android.telephony.NrVopsSupportInfo.NR_STATUS_VOPS_3GPP_SUPPORTED;
import static android.telephony.NrVopsSupportInfo.NR_STATUS_VOPS_NOT_SUPPORTED;

import android.os.Parcel;
import android.telephony.CellIdentity;
import android.telephony.CellIdentityGsm;
import android.telephony.CellIdentityLte;
import android.telephony.CellIdentityNr;
import android.telephony.CellIdentityWcdma;
import android.telephony.DataSpecificRegistrationInfo;
import android.telephony.LteVopsSupportInfo;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.NrVopsSupportInfo;
import android.telephony.ServiceState;
import android.telephony.TelephonyManager;
import android.telephony.VopsSupportInfo;

import androidx.annotation.NonNull;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * A builder class to create the {@link ServiceState} object.
 */
public class ServiceStateBuilder {
    private int mVoiceRegState = ServiceState.STATE_OUT_OF_SERVICE;
    private int mDataRegState = ServiceState.STATE_OUT_OF_SERVICE;
    private String mOperatorAlphaLong = null;
    private String mOperatorAlphaShort = null;
    private String mOperatorNumeric = null;
    private boolean mIsEmergencyOnly;
    private final List<NetworkRegistrationInfo> mNetworkRegistrationInfos = new ArrayList<>();
    private int mChannelNumber = -1;
    private boolean mIsIwlanPreferred;

    public ServiceStateBuilder() {
    }

    public @NonNull ServiceStateBuilder addNetworkRegistrationInfoForNoService() {
        addNetworkRegistrationInfo(DOMAIN_PS, TRANSPORT_TYPE_WWAN,
                REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING,
                TelephonyManager.NETWORK_TYPE_LTE, 0, false, null,
                LTE_ATTACH_TYPE_COMBINED, LTE_ATTACH_EXTRA_INFO_NONE, true, true, false);
        return this;
    }

    public @NonNull ServiceStateBuilder addNetworkRegistrationInfoForEmergencyOnly() {
        addNetworkRegistrationInfo(DOMAIN_PS, TRANSPORT_TYPE_WWAN, REGISTRATION_STATE_EMERGENCY,
                TelephonyManager.NETWORK_TYPE_LTE, 0, true, null,
                LTE_ATTACH_TYPE_COMBINED, LTE_ATTACH_EXTRA_INFO_NONE, true, true, false);
        return this;
    }

    public @NonNull ServiceStateBuilder addNetworkRegistrationInfoForLteCs() {
        addNetworkRegistrationInfo(DOMAIN_CS, TRANSPORT_TYPE_WWAN, REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_LTE, 0, false, null,
                LTE_ATTACH_TYPE_COMBINED, LTE_ATTACH_EXTRA_INFO_NONE, true, true, false);
        return this;
    }

    public @NonNull ServiceStateBuilder addNetworkRegistrationInfoForLtePs() {
        addNetworkRegistrationInfo(DOMAIN_PS, TRANSPORT_TYPE_WWAN, REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_LTE, 0, false, null,
                LTE_ATTACH_TYPE_COMBINED, LTE_ATTACH_EXTRA_INFO_NONE, true, true, false);
        return this;
    }

    public @NonNull ServiceStateBuilder addNetworkRegistrationInfoForIwlan() {
        addNetworkRegistrationInfo(DOMAIN_PS, TRANSPORT_TYPE_WLAN, REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_IWLAN, 0, false, null,
                LTE_ATTACH_TYPE_COMBINED, LTE_ATTACH_EXTRA_INFO_NONE, true, true, false);
        return this;
    }

    public @NonNull ServiceStateBuilder addNetworkRegistrationInfoForNr() {
        addNetworkRegistrationInfo(DOMAIN_PS, TRANSPORT_TYPE_WWAN, REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_NR, 0, false, null,
                LTE_ATTACH_TYPE_UNKNOWN, LTE_ATTACH_EXTRA_INFO_NONE, true, true, false);
        return this;
    }

    public @NonNull ServiceStateBuilder addNetworkRegistrationInfoForNrWithEmf() {
        addNetworkRegistrationInfo(DOMAIN_PS, TRANSPORT_TYPE_WWAN, REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_NR, 0, false, null,
                LTE_ATTACH_TYPE_UNKNOWN, LTE_ATTACH_EXTRA_INFO_NONE, true, false, true);
        return this;
    }

    public @NonNull ServiceStateBuilder addNetworkRegistrationInfoForNrCs() {
        addNetworkRegistrationInfo(DOMAIN_CS, TRANSPORT_TYPE_WWAN, REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_NR, 0, false, null,
                LTE_ATTACH_TYPE_UNKNOWN, LTE_ATTACH_EXTRA_INFO_NONE, true, true, false);
        return this;
    }

    public @NonNull ServiceStateBuilder addNetworkRegistrationInfoForUmts() {
        addNetworkRegistrationInfo(DOMAIN_PS, TRANSPORT_TYPE_WWAN, REGISTRATION_STATE_HOME,
                TelephonyManager.NETWORK_TYPE_UMTS, 0, false, null,
                LTE_ATTACH_TYPE_UNKNOWN, LTE_ATTACH_EXTRA_INFO_NONE, false, false, false);
        return this;
    }

    public @NonNull ServiceStateBuilder addNetworkRegistrationInfo(
            @NonNull NetworkRegistrationInfo nri) {
        mNetworkRegistrationInfos.add(nri);
        return this;
    }

    public @NonNull ServiceStateBuilder addNetworkRegistrationInfo(int domain, int transportType,
            int registrationState, int networkType, int rejectCause, boolean emergencyOnly,
            CellIdentity cellId, int lteAttachResultType, int lteAttachExtraInfo,
            boolean vops, boolean emc, boolean emf) {
        VopsSupportInfo vopsInfo = null;

        if (networkType == TelephonyManager.NETWORK_TYPE_NR) {
            vopsInfo = new NrVopsSupportInfo(
                    vops ? NR_STATUS_VOPS_3GPP_SUPPORTED : NR_STATUS_VOPS_NOT_SUPPORTED,
                    emc ? NR_STATUS_EMC_5GCN_ONLY : NR_STATUS_EMC_NOT_SUPPORTED,
                    emf ? NR_STATUS_EMF_5GCN_ONLY : NR_STATUS_EMF_NOT_SUPPORTED);
        } else if (networkType == TelephonyManager.NETWORK_TYPE_LTE) {
            vopsInfo = new LteVopsSupportInfo(
                    vops ? LTE_STATUS_SUPPORTED : LTE_STATUS_NOT_SUPPORTED,
                    emc ? LTE_STATUS_SUPPORTED : LTE_STATUS_NOT_SUPPORTED);
        }

        DataSpecificRegistrationInfo dsri = null;

        if (vopsInfo != null) {
            if (vopsInfo instanceof LteVopsSupportInfo) {
                dsri = new DataSpecificRegistrationInfo.Builder(3)
                        .setVopsSupportInfo(vopsInfo)
                        .setLteAttachResultType(lteAttachResultType)
                        .setLteAttachExtraInfo(lteAttachExtraInfo)
                        .build();
            } else {
                dsri = new DataSpecificRegistrationInfo.Builder(3)
                        .setVopsSupportInfo(vopsInfo)
                        .build();
            }
        }

        if (cellId == null) {
            cellId = createCellIdentity(networkType);
        }

        if (domain == DOMAIN_CS) {
            if (registrationState == REGISTRATION_STATE_HOME) {
                setVoiceRegState(ServiceState.STATE_IN_SERVICE);
            } else {
                setVoiceRegState(ServiceState.STATE_OUT_OF_SERVICE);
            }
        } else if (domain == DOMAIN_PS) {
            if (emergencyOnly) {
                setVoiceRegState(ServiceState.STATE_EMERGENCY_ONLY);
                setDataRegState(ServiceState.STATE_EMERGENCY_ONLY);
            } else if (registrationState == REGISTRATION_STATE_HOME) {
                setDataRegState(ServiceState.STATE_IN_SERVICE);
            } else {
                setDataRegState(ServiceState.STATE_OUT_OF_SERVICE);
            }

            setEmergencyOnly(emergencyOnly);

            if (transportType == TRANSPORT_TYPE_WLAN) {
                setIWlanPreferred(true);
            }

            setOperatorAlphaLong(TestConstants.OPERATOR_ALPHA_LONG);
            setOperatorAlphaShort(TestConstants.OPERATOR_ALPHA_SHORT);
            setOperatorNumeric(TestConstants.MCC_MNC);

            if (networkType == TelephonyManager.NETWORK_TYPE_LTE) {
                setChannelNumber(1); // Band_1
            } else if (networkType == TelephonyManager.NETWORK_TYPE_NR) {
                setChannelNumber(422000 + 100); // Band_1
            }
        }

        NetworkRegistrationInfo nri = new NetworkRegistrationInfo.Builder()
                .setDomain(domain)
                .setTransportType(transportType)
                .setRegistrationState(registrationState)
                .setAccessNetworkTechnology(networkType)
                .setRejectCause(rejectCause)
                .setEmergencyOnly(emergencyOnly)
                .setCellIdentity(cellId)
                .setDataSpecificInfo(dsri)
                .build();
        addNetworkRegistrationInfo(nri);
        return this;
    }

    public @NonNull ServiceStateBuilder setVoiceRegState(int regState) {
        mVoiceRegState = regState;
        return this;
    }

    public @NonNull ServiceStateBuilder setDataRegState(int regState) {
        mDataRegState = regState;
        return this;
    }

    public @NonNull ServiceStateBuilder setOperatorAlphaLong(String operatorAlpha) {
        mOperatorAlphaLong = operatorAlpha;
        return this;
    }

    public @NonNull ServiceStateBuilder setOperatorAlphaShort(String operatorAlpha) {
        mOperatorAlphaShort = operatorAlpha;
        return this;
    }

    public @NonNull ServiceStateBuilder setOperatorNumeric(String operatorNumeric) {
        mOperatorNumeric = operatorNumeric;
        return this;
    }

    public @NonNull ServiceStateBuilder setEmergencyOnly(boolean emergencyOnly) {
        mIsEmergencyOnly = emergencyOnly;
        return this;
    }

    public @NonNull ServiceStateBuilder setChannelNumber(int channelNumber) {
        mChannelNumber = channelNumber;
        return this;
    }

    public @NonNull ServiceStateBuilder setIWlanPreferred(boolean iwlanPreferred) {
        mIsIwlanPreferred = iwlanPreferred;
        return this;
    }

    public ServiceState build() {
        Parcel p = null;

        try {
            p = Parcel.obtain();
            p.writeInt(mVoiceRegState);
            p.writeInt(mDataRegState);
            p.writeString(mOperatorAlphaLong);
            p.writeString(mOperatorAlphaShort);
            p.writeString(mOperatorNumeric);
            p.writeInt(0); // mIsManualNetworkSelection
            p.writeInt(0); // mCssIndicator
            p.writeInt(-1); // mNetworkId
            p.writeInt(-1); // mSystemId
            p.writeInt(-1); // mCdmaRoamingIndicator
            p.writeInt(-1); // mCdmaDefaultRoamingIndicator
            p.writeInt(-1); // mCdmaEriIconIndex
            p.writeInt(-1); // mCdmaEriIconMode
            p.writeInt(mIsEmergencyOnly ? 1 : 0);
            p.writeInt(0); // mArfcnRsrpBoost
            p.writeList(mNetworkRegistrationInfos);
            p.writeInt(mChannelNumber);
            p.writeIntArray(new int[0]); // mCellBandwidths
            p.writeInt(0); // mNrFrequencyRange
            p.writeString(mOperatorAlphaLong); // mOperatorAlphaLongRaw
            p.writeString(mOperatorAlphaShort); // mOperatorAlphaShortRaw
            p.writeBoolean(false); // mIsDataRoamingFromRegistration
            p.writeBoolean(mIsIwlanPreferred);
            return ServiceState.CREATOR.createFromParcel(p);
        } finally {
            if (p != null) {
                p.recycle();
            }
        }
    }

    private static CellIdentity createCellIdentity(int networkType) {
        switch (networkType) {
            case TelephonyManager.NETWORK_TYPE_LTE:
                return new CellIdentityLte(0x1111111, 13, 0x2222, 0, new int[] {}, 0,
                        TestConstants.MCC, TestConstants.MNC,
                        TestConstants.OPERATOR_ALPHA_LONG, TestConstants.OPERATOR_ALPHA_SHORT,
                        Collections.emptyList(), null);
            case TelephonyManager.NETWORK_TYPE_NR:
                return new CellIdentityNr(20, 0x333333, 633693, new int[] {1, 78},
                        TestConstants.MCC, TestConstants.MNC, 0x555555555L,
                        TestConstants.OPERATOR_ALPHA_LONG, TestConstants.OPERATOR_ALPHA_SHORT,
                        Collections.emptyList());
            case TelephonyManager.NETWORK_TYPE_UMTS: // fallthrough
            case TelephonyManager.NETWORK_TYPE_HSDPA: // fallthrough
            case TelephonyManager.NETWORK_TYPE_HSUPA: // fallthrough
            case TelephonyManager.NETWORK_TYPE_HSPA: // fallthrough
            case TelephonyManager.NETWORK_TYPE_HSPAP:
                return new CellIdentityWcdma(0x6666, 0x7777777, 3, 0,
                        TestConstants.MCC, TestConstants.MNC,
                        TestConstants.OPERATOR_ALPHA_LONG, TestConstants.OPERATOR_ALPHA_SHORT,
                        Collections.emptyList(), null);
            case TelephonyManager.NETWORK_TYPE_GPRS: // fallthrough
            case TelephonyManager.NETWORK_TYPE_EDGE:
                return new CellIdentityGsm(0x8888, 0x9999, 0, 1,
                        TestConstants.MCC, TestConstants.MNC,
                        TestConstants.OPERATOR_ALPHA_LONG, TestConstants.OPERATOR_ALPHA_SHORT,
                        Collections.emptyList());
            default:
                return null;
        }
    }
}
