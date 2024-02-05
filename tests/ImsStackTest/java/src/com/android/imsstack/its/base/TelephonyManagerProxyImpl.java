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

import android.annotation.CallbackExecutor;
import android.content.Context;
import android.net.Uri;
import android.telephony.Annotation.CallState;
import android.telephony.Annotation.NetworkType;
import android.telephony.Annotation.SrvccState;
import android.telephony.Annotation.UiccAppType;
import android.telephony.Annotation.UiccAppTypeExt;
import android.telephony.BarringInfo;
import android.telephony.CellInfo;
import android.telephony.PreciseCallState;
import android.telephony.PreciseDataConnectionState;
import android.telephony.ServiceState;
import android.telephony.SignalStrength;
import android.telephony.TelephonyCallback;
import android.telephony.TelephonyCallback.BarringInfoListener;
import android.telephony.TelephonyCallback.CallStateListener;
import android.telephony.TelephonyCallback.CellInfoListener;
import android.telephony.TelephonyCallback.PreciseCallStateListener;
import android.telephony.TelephonyCallback.PreciseDataConnectionStateListener;
import android.telephony.TelephonyCallback.ServiceStateListener;
import android.telephony.TelephonyCallback.SignalStrengthsListener;
import android.telephony.TelephonyCallback.SrvccStateListener;
import android.telephony.TelephonyCallback.UserMobileDataStateListener;
import android.telephony.TelephonyManager;
import android.telephony.TelephonyManager.AuthType;
import android.telephony.TelephonyManager.BootstrapAuthenticationCallback;
import android.telephony.TelephonyManager.CellInfoCallback;
import android.telephony.TelephonyManager.HalService;
import android.telephony.TelephonyManager.SimState;
import android.telephony.gba.UaSecurityProtocolIdentifier;
import android.util.ArraySet;
import android.util.Pair;
import android.util.SparseArray;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;

import com.android.imsstack.base.AppContext;
import com.android.imsstack.base.SystemServiceProxy.SubscriptionManagerProxy;
import com.android.imsstack.base.TelephonyManagerProxy;
import com.android.imsstack.util.Log;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.util.Set;
import java.util.concurrent.Executor;

/**
 * An implementation class to access the APIs of {@link TelephonyManager}.
 */
public class TelephonyManagerProxyImpl implements TelephonyManagerProxy {
    private static final String IMEI_POSTFIX = "35131133001560";
    private static SparseArray<TelephonyManagerProxyImpl> sTelephonyManagerProxies =
            new SparseArray<>();
    private final Context mContext;
    private final int mSubId;
    private final ArraySet<TelephonyCallbackRecord> mTelephonyCallbackRecords = new ArraySet<>();
    private final Set<String> mEmergencyNumbers = new ArraySet<>();
    private final DeviceInfoRecord mDeviceInfoRecord;
    private final SimInfoRecord mSimInfoRecord = new SimInfoRecord();
    private final NetworkInfoRecord mNetworkInfoRecord = new NetworkInfoRecord();
    private int mActiveModemCount = 1;
    private int mSupportedModemCount = 2;
    private Pair<Integer, Integer> mHalVersion = new Pair<>(2, 1);

    TelephonyManagerProxyImpl(@NonNull Context context) {
        this(context, TestConstants.SUB_ID_1);
    }

    TelephonyManagerProxyImpl(@NonNull Context context, int subId) {
        mContext = context;
        mSubId = subId;
        mDeviceInfoRecord = new DeviceInfoRecord(String.valueOf(mSubId) + IMEI_POSTFIX, "01");
        setDefaultValues();
        sTelephonyManagerProxies.put(mSubId, this);
    }

    @Override
    public @NonNull TelephonyManagerProxy createForSubscriptionId(int subId) {
        TelephonyManagerProxyImpl tmp = getTelephonyManagerProxy(subId);
        if (tmp == null) {
            tmp = new TelephonyManagerProxyImpl(mContext, subId);
        }
        return tmp;
    }

    @Override
    public void registerTelephonyCallback(@NonNull @CallbackExecutor Executor executor,
            @NonNull TelephonyCallback callback) {
        mTelephonyCallbackRecords.add(new TelephonyCallbackRecord(callback, executor));
    }

    @Override
    public void unregisterTelephonyCallback(@NonNull TelephonyCallback callback) {
        final ArraySet<TelephonyCallbackRecord> recordsToRemove = new ArraySet<>();
        mTelephonyCallbackRecords.forEach((r) -> {
            if (r.hasCallback(callback)) {
                recordsToRemove.add(r);
            }
        });

        recordsToRemove.forEach(mTelephonyCallbackRecords::remove);
    }

    @Override
    public int getActiveModemCount() {
        return mActiveModemCount;
    }

    @Override
    public int getSupportedModemCount() {
        return mSupportedModemCount;
    }

    @Override
    public String getImei(int slotIndex) {
        TelephonyManagerProxyImpl tmp = getTelephonyManagerProxy(getSubId(slotIndex));
        return tmp != null ? tmp.mDeviceInfoRecord.getImei() : null;
    }

    @Override
    public String getDeviceSoftwareVersion(int slotIndex) {
        TelephonyManagerProxyImpl tmp = getTelephonyManagerProxy(getSubId(slotIndex));
        return tmp != null ? tmp.mDeviceInfoRecord.getSoftwareVersion() : null;
    }

    @Override
    public boolean isEmergencyNumber(@NonNull String number) {
        return mEmergencyNumbers.contains(number);
    }

    @Override
    public boolean hasIccCard() {
        return mSimInfoRecord.hasIccCard();
    }

    @Override
    public boolean isApplicationOnUicc(@UiccAppType int appType) {
        return mSimInfoRecord.isApplicationOnUicc(appType);
    }

    @Override
    public @SimState int getSimApplicationState() {
        return mSimInfoRecord.getApplicationState();
    }

    @Override
    public @SimState int getSimState(int slotIndex) {
        TelephonyManagerProxyImpl tmp = getTelephonyManagerProxy(getSubId(slotIndex));
        return tmp != null ? tmp.mSimInfoRecord.getState() : TelephonyManager.SIM_STATE_UNKNOWN;
    }

    @Override
    public @SimState int getSimCardState() {
        return mSimInfoRecord.getCardState();
    }

    @Override
    public int getSimCarrierId() {
        return mSimInfoRecord.getCarrierId();
    }

    @Override
    public @Nullable CharSequence getSimCarrierIdName() {
        return mSimInfoRecord.getCarrierIdName();
    }

    @Override
    public int getSimSpecificCarrierId() {
        return mSimInfoRecord.getSpecificCarrierId();
    }

    @Override
    public int getCarrierIdFromSimMccMnc() {
        return mSimInfoRecord.getCarrierIdFromSimMccMnc();
    }

    @Override
    public String getSubscriberId() {
        return mSimInfoRecord.getImsi();
    }

    @Override
    public String getSimOperator() {
        return mSimInfoRecord.getOperator();
    }

    @Override
    public String getSimCountryIso() {
        return mSimInfoRecord.getCountryIso();
    }

    @Override
    public String getSimSerialNumber() {
        return mSimInfoRecord.getSerialNumber();
    }

    @Override
    public String getGroupIdLevel1() {
        return mSimInfoRecord.getGroupIdLevel1();
    }

    @Override
    public String getSimOperatorName() {
        return mSimInfoRecord.getOperatorName();
    }

    @Override
    public String getSimServiceTable(@UiccAppType int appType) {
        return mSimInfoRecord.getServiceTable(appType);
    }

    @Override
    public String getIsimDomain() {
        return mSimInfoRecord.getIsimDomain();
    }

    @Override
    public String getImsPrivateUserIdentity() {
        return mSimInfoRecord.getIsimPrivateUserIdentity();
    }

    @Override
    public List<Uri> getImsPublicUserIdentities() {
        return mSimInfoRecord.getIsimPublicUserIdentities();
    }

    @Override
    public String getIccAuthentication(@UiccAppType int appType, @AuthType int authType,
            String data) {
        // This is not supported yet in the test environment.
        return null;
    }

    @Override
    public String sendEnvelopeWithStatus(String content) {
        // This is not supported yet in the test environment.
        return "";
    }

    @Override
    public boolean isDataEnabled() {
        return mNetworkInfoRecord.isDataEnabled();
    }

    @Override
    public boolean isDataRoamingEnabled() {
        return mNetworkInfoRecord.isDataRoamingEnabled();
    }

    @Override
    public @Nullable ServiceState getServiceState() {
        return mNetworkInfoRecord.getServiceState();
    }

    @Override
    public @NetworkType int getDataNetworkType() {
        return mNetworkInfoRecord.getDataNetworkType();
    }

    @Override
    public @NetworkType int getVoiceNetworkType() {
        return mNetworkInfoRecord.getVoiceNetworkType();
    }

    @Override
    public String getNetworkOperator() {
        return mNetworkInfoRecord.getOperator();
    }

    @Override
    public String getNetworkCountryIso() {
        return mNetworkInfoRecord.getCountryIso();
    }

    @Override
    public List<CellInfo> getAllCellInfo() {
        return mNetworkInfoRecord.getAllCellInfo();
    }

    @Override
    public void requestCellInfoUpdate(@NonNull @CallbackExecutor Executor executor,
            @NonNull CellInfoCallback callback) {
        executor.execute(() -> {
            List<CellInfo> allCellInfo = getAllCellInfo();
            callback.onCellInfo(allCellInfo != null ? allCellInfo : new ArrayList<CellInfo>());
        });
    }

    @Override
    public void bootstrapAuthenticationRequest(@UiccAppTypeExt int appType, @NonNull Uri nafId,
            @NonNull UaSecurityProtocolIdentifier securityProtocol, boolean forceBootStrapping,
            @NonNull Executor executor, @NonNull BootstrapAuthenticationCallback callback) {
        // This is not supported yet in the test environment.
        executor.execute(() -> {
            callback.onAuthenticationFailure(
                    TelephonyManager.GBA_FAILURE_REASON_FEATURE_NOT_READY);
        });
    }

    @Override
    public @NonNull Pair<Integer, Integer> getHalVersion(@HalService int halService) {
        return mHalVersion;
    }

    /**
     * Notifies the application that the barring information is changed.
     *
     * @param barringInfo The barring information.
     */
    public void notifyBarringInfoChanged(@NonNull BarringInfo barringInfo) {
        mTelephonyCallbackRecords.forEach((r) -> r.dispatchBarringInfoChanged(barringInfo));
    }

    /**
     * Notifies the application that the call state is changed.
     *
     * @param state The call state.
     */
    public void notifyCallStateChanged(@CallState int state) {
        mTelephonyCallbackRecords.forEach((r) -> r.dispatchCallStateChanged(state));
    }

    /**
     * Notifies the application that the cell information is changed.
     *
     * @param cellInfo The list of {@link CellInfo}.
     */
    public void notifyCellInfoChanged(@NonNull List<CellInfo> cellInfo) {
        mTelephonyCallbackRecords.forEach((r) -> r.dispatchCellInfoChanged(cellInfo));
    }

    /**
     * Notifies the application that the precise call state is changed.
     *
     * @param callState The precise call state.
     */
    public void notifyPreciseCallStateChanged(@NonNull PreciseCallState callState) {
        mTelephonyCallbackRecords.forEach((r) -> r.dispatchPreciseCallStateChanged(callState));
    }

    /**
     * Notifies the application that the precise data connection state is changed.
     *
     * @param dataConnectionState The precise data connection state.
     */
    public void notifyPreciseDataConnectionStateChanged(
            @NonNull PreciseDataConnectionState dataConnectionState) {
        mTelephonyCallbackRecords.forEach(
                (r) -> r.dispatchPreciseDataConnectionStateChanged(dataConnectionState));
    }

    /**
     * Notifies the application that the service state is changed.
     *
     * @param serviceState The service state.
     */
    public void notifyServiceStateChanged(@NonNull ServiceState serviceState) {
        mTelephonyCallbackRecords.forEach((r) -> r.dispatchServiceStateChanged(serviceState));
    }

    /**
     * Notifies the application that the signal strength is changed.
     *
     * @param signalStrength The signal strength.
     */
    public void notifySignalStrengthsChanged(@NonNull SignalStrength signalStrength) {
        mTelephonyCallbackRecords.forEach((r) -> r.dispatchSignalStrengthsChanged(signalStrength));
    }

    /**
     * Notifies the application that the SRVCC state is changed.
     *
     * @param srvccState The SRVCC state.
     */
    public void notifySrvccStateChanged(@SrvccState int srvccState) {
        mTelephonyCallbackRecords.forEach((r) -> r.dispatchSrvccStateChanged(srvccState));
    }

    /**
     * Notifies the application that the user mobile data setting is changed.
     *
     * @param enabled The user mobile data setting.
     */
    public void notifyUserMobileDataStateChanged(boolean enabled) {
        mTelephonyCallbackRecords.forEach((r) -> r.dispatchUserMobileDataStateChanged(enabled));
    }

    /**
     * Sets the default values for this object.
     */
    public void setDefaultValues() {
        setActiveModemCount(1);
        setSupportedModemCount(2);
        setHalVersion(2, 1);
        clearEmergencyNumbers();
        initUsimApplication();
        initIsimApplication();
        initNetworkInfo();
    }

    /**
     * Initializes the network information of this object.
     */
    public void initNetworkInfo() {
        mNetworkInfoRecord.setDataEnabled(false);
        mNetworkInfoRecord.setDataRoamingEnabled(false);
        mNetworkInfoRecord.setServiceState(null);
        mNetworkInfoRecord.setDataNetworkType(TelephonyManager.NETWORK_TYPE_UNKNOWN);
        mNetworkInfoRecord.setVoiceNetworkType(TelephonyManager.NETWORK_TYPE_UNKNOWN);
        mNetworkInfoRecord.setOperator(TestConstants.MCC);
        mNetworkInfoRecord.setCountryIso("");
        mNetworkInfoRecord.setAllCellInfo(null);
    }

    /**
     * Sets the active modem count.
     *
     * @param modemCount The modem count.
     */
    public void setActiveModemCount(int modemCount) {
        mActiveModemCount = modemCount;
    }

    /**
     * Sets the supported modem count.
     *
     * @param modemCount The modem count.
     */
    public void setSupportedModemCount(int modemCount) {
        mSupportedModemCount = modemCount;
    }

    /**
     * Adds the specified number to the emergency number list.
     *
     * @param number The number to be added.
     */
    public void addEmergencyNumber(@NonNull String number) {
        mEmergencyNumbers.add(number);
    }

    /**
     * Removes the specified number from the emergency number list.
     *
     * @param number The number to be removed.
     */
    public void removeEmergencyNumber(@NonNull String number) {
        mEmergencyNumbers.remove(number);
    }

    /**
     * Clears all the registered emergency numbers.
     */
    public void clearEmergencyNumbers() {
        mEmergencyNumbers.clear();
    }

    /**
     * Initializes the values for USIM application.
     */
    public void initUsimApplication() {
        mSimInfoRecord.setUsimAppType(TelephonyManager.APPTYPE_USIM);
        mSimInfoRecord.setApplicationState(TelephonyManager.SIM_STATE_NOT_READY);
        mSimInfoRecord.setCardState(TelephonyManager.SIM_STATE_PRESENT);
        mSimInfoRecord.setState(TelephonyManager.SIM_STATE_NOT_READY);
        mSimInfoRecord.setCarrierId(1911); // Test SIM
        mSimInfoRecord.setCarrierIdName("Test-SIM");
        mSimInfoRecord.setSpecificCarrierId(TelephonyManager.UNKNOWN_CARRIER_ID);
        if (mSubId == TestConstants.SUB_ID_1) {
            mSimInfoRecord.setSerialNumber(TestConstants.SIM_SERIAL_NUMBER_1);
            mSimInfoRecord.setImsi(TestConstants.MCC_MNC + TestConstants.PHONE_NUMBER_1);
        } else {
            mSimInfoRecord.setSerialNumber(TestConstants.SIM_SERIAL_NUMBER_2);
            mSimInfoRecord.setImsi(TestConstants.MCC_MNC + TestConstants.PHONE_NUMBER_2);
        }
        mSimInfoRecord.setOperator(TestConstants.MCC);
        mSimInfoRecord.setCountryIso("");
        mSimInfoRecord.setGroupIdLevel1(null);
        mSimInfoRecord.setOperatorName("Test-SIM");
        mSimInfoRecord.setServiceTable(TelephonyManager.APPTYPE_USIM, "0x00");
    }

    /**
     * Clears the values for USIM application.
     * All the values will be set as the default values.
     */
    public void clearUsimApplication() {
        mSimInfoRecord.setUsimAppType(TelephonyManager.APPTYPE_UNKNOWN);
        mSimInfoRecord.setApplicationState(TelephonyManager.SIM_STATE_UNKNOWN);
        mSimInfoRecord.setCardState(TelephonyManager.SIM_STATE_UNKNOWN);
        mSimInfoRecord.setState(TelephonyManager.SIM_STATE_UNKNOWN);
        mSimInfoRecord.setCarrierId(TelephonyManager.UNKNOWN_CARRIER_ID);
        mSimInfoRecord.setCarrierIdName("unknown");
        mSimInfoRecord.setSpecificCarrierId(TelephonyManager.UNKNOWN_CARRIER_ID);
        mSimInfoRecord.setSerialNumber(null);
        mSimInfoRecord.setImsi(null);
        mSimInfoRecord.setOperator(null);
        mSimInfoRecord.setCountryIso("");
        mSimInfoRecord.setGroupIdLevel1(null);
        mSimInfoRecord.setOperatorName(null);
        mSimInfoRecord.setServiceTable(TelephonyManager.APPTYPE_USIM, "0x00");
    }

    /**
     * Initializes the values for ISIM application.
     */
    public void initIsimApplication() {
        final String domain = String.format("ims.mnc0%s.mcc%s.3gppnetwork.org",
                TestConstants.MNC, TestConstants.MCC);
        final String phoneNumber = (mSubId == TestConstants.SUB_ID_1)
                ? TestConstants.PHONE_NUMBER_1
                : TestConstants.PHONE_NUMBER_2;
        final String impi = String.format("%s%s%s@%s",
                TestConstants.MCC, TestConstants.MNC, phoneNumber, domain);
        final String impuSip = String.format("sip:%s", impi);
        final String impuTel = String.format("tel:%s", phoneNumber);
        final List<Uri> impus = List.of(Uri.parse(impuSip), Uri.parse(impuTel));

        mSimInfoRecord.setIsimAppType(TelephonyManager.APPTYPE_ISIM);
        mSimInfoRecord.setIsimDomain(domain);
        mSimInfoRecord.setIsimPrivateUserIdentity(impi);
        mSimInfoRecord.setIsimPublicUserIdentities(impus);
        mSimInfoRecord.setServiceTable(TelephonyManager.APPTYPE_ISIM, "0x00");
    }

    /**
     * Clears the values for ISIM application.
     * All the values will be set as the default value.
     */
    public void clearIsimApplication() {
        mSimInfoRecord.setIsimAppType(TelephonyManager.APPTYPE_UNKNOWN);
        mSimInfoRecord.setIsimDomain(null);
        mSimInfoRecord.setIsimPrivateUserIdentity(null);
        mSimInfoRecord.setIsimPublicUserIdentities(null);
        mSimInfoRecord.setServiceTable(TelephonyManager.APPTYPE_ISIM, "0x00");
    }

    /**
     * Sets the SIM application state.
     *
     * @param state The SIM application state. Possible values are:
     *              {@link TelephonyManager#SIM_STATE_UNKNOWN},
     *              {@link TelephonyManager#SIM_STATE_PIN_REQUIRED},
     *              {@link TelephonyManager#SIM_STATE_PUK_REQUIRED},
     *              {@link TelephonyManager#SIM_STATE_NETWORK_LOCKED},
     *              {@link TelephonyManager#SIM_STATE_NOT_READY},
     *              {@link TelephonyManager#SIM_STATE_PERM_DISABLED},
     *              {@link TelephonyManager#SIM_STATE_LOADED}
     */
    public void setSimApplicationState(@SimState int state) {
        mSimInfoRecord.setApplicationState(state);
    }

    /**
     * Sets the SIM state.
     *
     * @param slotIndex The slot index.
     * @param state The SIM state. Possible values are:
     *              {@link TelephonyManager#SIM_STATE_UNKNOWN},
     *              {@link TelephonyManager#SIM_STATE_ABSENT},
     *              {@link TelephonyManager#SIM_STATE_PIN_REQUIRED},
     *              {@link TelephonyManager#SIM_STATE_PUK_REQUIRED},
     *              {@link TelephonyManager#SIM_STATE_NETWORK_LOCKED},
     *              {@link TelephonyManager#SIM_STATE_READY},
     *              {@link TelephonyManager#SIM_STATE_NOT_READY},
     *              {@link TelephonyManager#SIM_STATE_PERM_DISABLED},
     *              {@link TelephonyManager#SIM_STATE_CARD_IO_ERROR},
     *              {@link TelephonyManager#SIM_STATE_CARD_RESTRICTED}
     */
    public void setSimState(int slotIndex, @SimState int state) {
        TelephonyManagerProxyImpl tmp = getTelephonyManagerProxy(getSubId(slotIndex));
        if (tmp != null) {
            tmp.mSimInfoRecord.setState(state);
        }
    }

    /**
     * Sets the SIM card state.
     *
     * @param state The SIM card state. Possible values are:
     *              {@link TelephonyManager#SIM_STATE_UNKNOWN},
     *              {@link TelephonyManager#SIM_STATE_ABSENT},
     *              {@link TelephonyManager#SIM_STATE_CARD_IO_ERROR},
     *              {@link TelephonyManager#SIM_STATE_CARD_RESTRICTED},
     *              {@link TelephonyManager#SIM_STATE_PRESENT}
     */
    public void setSimCardState(@SimState int state) {
        mSimInfoRecord.setCardState(state);
    }

    /**
     * Sets the carrier id.
     *
     * @param carrierId The carrier id.
     */
    public void setSimCarrierId(int carrierId) {
        mSimInfoRecord.setCarrierId(carrierId);
    }

    /**
     * Sets the carrier id name.
     *
     * @param carrierIdName The carrier id name.
     */
    public void setSimCarrierIdName(CharSequence carrierIdName) {
        mSimInfoRecord.setCarrierIdName(carrierIdName);
    }

    /**
     * Sets the specific carrier id.
     *
     * @param specificCarrierId The specific carrier id.
     */
    public void setSimSpecificCarrierId(int specificCarrierId) {
        mSimInfoRecord.setSpecificCarrierId(specificCarrierId);
    }

    /**
     * Sets the IMSI string of SIM.
     *
     * @param imsi The IMSI string.
     */
    public void setSubscriberId(String imsi) {
        mSimInfoRecord.setImsi(imsi);
    }

    /**
     * Sets the operator(MNC) string of SIM.
     *
     * @param operator The operator string.
     */
    public void setSimOperator(String operator) {
        mSimInfoRecord.setOperator(operator);
    }

    /**
     * Sets the country ISO string of SIM.
     *
     * @param countryIso The country ISO string.
     */
    public void setSimCountryIso(String countryIso) {
        mSimInfoRecord.setCountryIso(countryIso);
    }

    /**
     * Sets the group id level1 string of SIM.
     *
     * @param gid1 The group id level1 string.
     */
    public void setGroupIdLevel1(String gid1) {
        mSimInfoRecord.setGroupIdLevel1(gid1);
    }

    /**
     * Sets the operator name of SIM.
     *
     * @param operatorName The operator name.
     */
    public void setSimOperatorName(String operatorName) {
        mSimInfoRecord.setOperatorName(operatorName);
    }

    /**
     * Sets the service table of SIM.
     *
     * @param appType The targeted application.
     * @param serviceTable The service table value.
     */
    public void setSimServiceTable(@UiccAppType int appType, String serviceTable) {
        mSimInfoRecord.setServiceTable(appType, serviceTable);
    }

    /**
     * Sets the domain name of ISIM.
     *
     * @param domain The domain name.
     */
    public void setIsimDomain(String domain) {
        mSimInfoRecord.setIsimDomain(domain);
    }

    /**
     * Sets the IMS private user identity of ISIM.
     *
     * @param privateUserIdentity The private user identity.
     */
    public void getImsPrivateUserIdentity(String privateUserIdentity) {
        mSimInfoRecord.setIsimPrivateUserIdentity(privateUserIdentity);
    }

    /**
     * Sets the IMS public user identities of ISIM.
     *
     * @param publicUserIdentities The list of public user identity.
     */
    public void setImsPublicUserIdentities(List<Uri> publicUserIdentities) {
        mSimInfoRecord.setIsimPublicUserIdentities(publicUserIdentities);
    }

    /**
     * Sets the data enabled state.
     *
     * @param enabled The data enabled state.
     */
    public void setDataEnabled(boolean enabled) {
        mNetworkInfoRecord.setDataEnabled(enabled);
    }

    /**
     * Sets the data enabled state in the roaming area.
     *
     * @param enabled The data enabled state.
     */
    public void setDataRoamingEnabled(boolean enabled) {
        mNetworkInfoRecord.setDataRoamingEnabled(enabled);
    }

    /**
     * Sets the current service state.
     *
     * @param serviceState The service state.
     */
    public void setServiceState(@Nullable ServiceState serviceState) {
        mNetworkInfoRecord.setServiceState(serviceState);
    }

    /**
     * Sets the data network type.
     *
     * @param networkType The network type.
     */
    public void setDataNetworkType(@NetworkType int networkType) {
        mNetworkInfoRecord.setDataNetworkType(networkType);
    }

    /**
     * Sets the voice network type.
     *
     * @param networkType The network type.
     */
    public void setVoiceNetworkType(@NetworkType int networkType) {
        mNetworkInfoRecord.setVoiceNetworkType(networkType);
    }

    /**
     * Sets the operator(MNC) received from the network.
     *
     * @param operator The operator string.
     */
    public void setNetworkOperator(String operator) {
        mNetworkInfoRecord.setOperator(operator);
    }

    /**
     * Sets the country ISO string received from the network.
     *
     * @param countryIso The country ISO string.
     */
    public void setNetworkCountryIso(String countryIso) {
        mNetworkInfoRecord.setCountryIso(countryIso);
    }

    /**
     * Sets all cell information.
     *
     * @param cellInfos The list of cell information.
     */
    public void setAllCellInfo(List<CellInfo> cellInfos) {
        mNetworkInfoRecord.setAllCellInfo(cellInfos);
    }

    /**
     * Sets the HAL version.
     *
     * @param major The major version code.
     * @param minor The minor version code.
     */
    public void setHalVersion(int major, int minor) {
        mHalVersion = new Pair<>(major, minor);
    }

    private static int getSubId(int slotIndex) {
        SubscriptionManagerProxy smp = AppContext.getInstance().getSystemServiceProxy(
                SubscriptionManagerProxy.class);
        return smp.getSubscriptionId(slotIndex);
    }

    private static TelephonyManagerProxyImpl getTelephonyManagerProxy(int subId) {
        return sTelephonyManagerProxies.get(subId);
    }

    private static final class TelephonyCallbackRecord {
        private final TelephonyCallback mCallback;
        private final Executor mScheduler;

        TelephonyCallbackRecord(TelephonyCallback callback, Executor scheduler) {
            mCallback = callback;
            mScheduler = scheduler;
        }

        boolean hasCallback(TelephonyCallback callback) {
            return mCallback.equals(callback);
        }

        void dispatchBarringInfoChanged(@NonNull BarringInfo barringInfo) {
            final BarringInfoListener listener = getListener(BarringInfoListener.class);
            if (listener != null) {
                logi("dispatchBarringInfoChanged: " + barringInfo);
                mScheduler.execute(() -> listener.onBarringInfoChanged(barringInfo));
            }
        }

        void dispatchCallStateChanged(@CallState int state) {
            final CallStateListener listener = getListener(CallStateListener.class);
            if (listener != null) {
                logi("dispatchCallStateChanged: " + state);
                mScheduler.execute(() -> listener.onCallStateChanged(state));
            }
        }

        void dispatchCellInfoChanged(@NonNull List<CellInfo> cellInfo) {
            final CellInfoListener listener = getListener(CellInfoListener.class);
            if (listener != null) {
                logi("dispatchCellInfoChanged: " + cellInfo);
                mScheduler.execute(() -> listener.onCellInfoChanged(cellInfo));
            }
        }

        void dispatchPreciseCallStateChanged(@NonNull PreciseCallState callState) {
            final PreciseCallStateListener listener = getListener(PreciseCallStateListener.class);
            if (listener != null) {
                logi("dispatchPreciseCallStateChanged: " + callState);
                mScheduler.execute(() -> listener.onPreciseCallStateChanged(callState));
            }
        }

        void dispatchPreciseDataConnectionStateChanged(
                @NonNull PreciseDataConnectionState dataConnectionState) {
            final PreciseDataConnectionStateListener listener =
                    getListener(PreciseDataConnectionStateListener.class);
            if (listener != null) {
                logi("dispatchPreciseDataConnectionStateChanged: " + dataConnectionState);
                mScheduler.execute(() ->
                        listener.onPreciseDataConnectionStateChanged(dataConnectionState));
            }
        }

        void dispatchServiceStateChanged(@NonNull ServiceState serviceState) {
            final ServiceStateListener listener = getListener(ServiceStateListener.class);
            if (listener != null) {
                logi("dispatchServiceStateChanged: " + serviceState);
                mScheduler.execute(() -> listener.onServiceStateChanged(serviceState));
            }
        }

        void dispatchSignalStrengthsChanged(@NonNull SignalStrength signalStrength) {
            final SignalStrengthsListener listener = getListener(SignalStrengthsListener.class);
            if (listener != null) {
                logi("dispatchSignalStrengthsChanged: " + signalStrength);
                mScheduler.execute(() -> listener.onSignalStrengthsChanged(signalStrength));
            }
        }

        void dispatchSrvccStateChanged(@SrvccState int srvccState) {
            final SrvccStateListener listener = getListener(SrvccStateListener.class);
            if (listener != null) {
                logi("dispatchSrvccStateChanged: " + srvccState);
                mScheduler.execute(() -> listener.onSrvccStateChanged(srvccState));
            }
        }

        void dispatchUserMobileDataStateChanged(boolean enabled) {
            final UserMobileDataStateListener listener =
                    getListener(UserMobileDataStateListener.class);
            if (listener != null) {
                logi("dispatchUserMobileDataStateChanged: " + enabled);
                mScheduler.execute(() -> listener.onUserMobileDataStateChanged(enabled));
            }
        }

        @SuppressWarnings("unchecked")
        <T> T getListener(Class<T> clazz) {
            if (mCallback != null && clazz.isInstance(mCallback)) {
                return (T) mCallback;
            }
            return null;
        }
    }

    private static void logi(String s) {
        Log.i(Log.TAG, "TelephonyManagerProxy: " + s);
    }

    private static final class DeviceInfoRecord {
        private final String mImei;
        private final String mSoftwareVersion;

        DeviceInfoRecord(String imei, String softwareVersion) {
            mImei = imei;
            mSoftwareVersion = softwareVersion;
        }

        String getImei() {
            return mImei;
        }

        String getSoftwareVersion() {
            return mSoftwareVersion;
        }
    }

    private static final class SimInfoRecord {
        /** USIM records */
        private @UiccAppType int mUsimAppType = TelephonyManager.APPTYPE_UNKNOWN;
        private int mApplicationState = TelephonyManager.SIM_STATE_UNKNOWN;
        private int mState = TelephonyManager.SIM_STATE_UNKNOWN;
        private int mCardState = TelephonyManager.SIM_STATE_UNKNOWN;
        private int mCarrierId = TelephonyManager.UNKNOWN_CARRIER_ID;
        private int mSpecificCarrierId = TelephonyManager.UNKNOWN_CARRIER_ID;
        private CharSequence mCarrierIdName = "";
        private String mImsi;
        private String mOperator;
        private String mOperatorName;
        private String mCountryIso = "";
        private String mSerialNumber;
        private String mGroupIdLevel1;
        private String mServiceTable = "0x00";

        /** ISIM records */
        private @UiccAppType int mIsimAppType = TelephonyManager.APPTYPE_UNKNOWN;
        private String mIsimServiceTable = "0x00";
        private String mIsimDomain;
        private String mIsimPrivateUserIdentity;
        private List<Uri> mIsimPublicUserIdentities = Collections.emptyList();

        boolean hasIccCard() {
            return mState != TelephonyManager.SIM_STATE_UNKNOWN
                    && mState != TelephonyManager.SIM_STATE_ABSENT;
        }

        boolean isApplicationOnUicc(@UiccAppType int appType) {
            return mUsimAppType == appType || mIsimAppType == appType;
        }

        @SimState int getApplicationState() {
            return mApplicationState;
        }

        @SimState int getState() {
            return mState;
        }

        @SimState int getCardState() {
            return mCardState;
        }

        int getCarrierId() {
            return mCarrierId;
        }

        int getSpecificCarrierId() {
            return mSpecificCarrierId;
        }

        CharSequence getCarrierIdName() {
            return mCarrierIdName;
        }

        int getCarrierIdFromSimMccMnc() {
            return mCarrierId;
        }

        String getImsi() {
            return mImsi;
        }

        String getOperator() {
            return mOperator;
        }

        String getCountryIso() {
            return mCountryIso;
        }

        String getSerialNumber() {
            return mSerialNumber;
        }

        String getGroupIdLevel1() {
            return mGroupIdLevel1;
        }

        String getOperatorName() {
            return mOperatorName;
        }

        String getServiceTable(@UiccAppType int appType) {
            if (appType == TelephonyManager.APPTYPE_ISIM) {
                return mIsimServiceTable;
            }
            return mServiceTable;
        }

        String getIsimDomain() {
            return mIsimDomain;
        }

        String getIsimPrivateUserIdentity() {
            return mIsimPrivateUserIdentity;
        }

        List<Uri> getIsimPublicUserIdentities() {
            return mIsimPublicUserIdentities;
        }

        void setUsimAppType(@UiccAppType int appType) {
            mUsimAppType = appType;
        }

        void setApplicationState(@SimState int applicationState) {
            mApplicationState = applicationState;
        }

        void setState(@SimState int state) {
            mState = state;
        }

        void setCardState(@SimState int cardState) {
            mCardState = cardState;
        }

        void setCarrierId(int carrierId) {
            mCarrierId = carrierId;
        }

        void setSpecificCarrierId(int specificCarrierId) {
            mSpecificCarrierId = specificCarrierId;
        }

        void setCarrierIdName(CharSequence carrierIdName) {
            mCarrierIdName = carrierIdName;
        }

        void setImsi(String imsi) {
            mImsi = imsi;
        }

        void setOperator(String operator) {
            mOperator = operator;
        }

        void setCountryIso(String countryIso) {
            mCountryIso = countryIso;
        }

        void setSerialNumber(String serialNumber) {
            mSerialNumber = serialNumber;
        }

        void setGroupIdLevel1(String groupIdLevel1) {
            mGroupIdLevel1 = groupIdLevel1;
        }

        void setOperatorName(String operatorName) {
            mOperatorName = operatorName;
        }

        void setServiceTable(@UiccAppType int appType, String serviceTable) {
            if (appType == TelephonyManager.APPTYPE_ISIM) {
                mIsimServiceTable = serviceTable;
            } else if (appType == TelephonyManager.APPTYPE_USIM) {
                mServiceTable = serviceTable;
            }
        }

        void setIsimAppType(@UiccAppType int appType) {
            mIsimAppType = appType;
        }

        void setIsimDomain(String domain) {
            mIsimDomain = domain;
        }

        void setIsimPrivateUserIdentity(String privateUserIdentity) {
            mIsimPrivateUserIdentity = privateUserIdentity;
        }

        void setIsimPublicUserIdentities(List<Uri> publicUserIdentities) {
            mIsimPublicUserIdentities = publicUserIdentities != null
                    ? Collections.unmodifiableList(publicUserIdentities)
                    : Collections.emptyList();
        }
    }

    private static final class NetworkInfoRecord {
        private boolean mDataEnabled;
        private boolean mDataRoamingEnabled;
        private ServiceState mServiceState;
        private int mDataNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        private int mVoiceNetworkType = TelephonyManager.NETWORK_TYPE_UNKNOWN;
        private String mOperator;
        private String mCountryIso = "";
        private List<CellInfo> mCellInfos = Collections.emptyList();

        boolean isDataEnabled() {
            return mDataEnabled;
        }

        boolean isDataRoamingEnabled() {
            return mDataRoamingEnabled;
        }

        ServiceState getServiceState() {
            return mServiceState;
        }

        int getDataNetworkType() {
            return mDataNetworkType;
        }

        int getVoiceNetworkType() {
            return mVoiceNetworkType;
        }

        String getOperator() {
            return mOperator;
        }

        String getCountryIso() {
            return mCountryIso;
        }

        List<CellInfo> getAllCellInfo() {
            return mCellInfos;
        }

        void setDataEnabled(boolean dataEnabled) {
            mDataEnabled = dataEnabled;
        }

        void setDataRoamingEnabled(boolean dataRoamingEnabled) {
            mDataRoamingEnabled = dataRoamingEnabled;
        }

        void setServiceState(ServiceState serviceState) {
            mServiceState = serviceState;
        }

        void setDataNetworkType(int dataNetworkType) {
            mDataNetworkType = dataNetworkType;
        }

        void setVoiceNetworkType(int voiceNetworkType) {
            mVoiceNetworkType = voiceNetworkType;
        }

        void setOperator(String operator) {
            mOperator = operator;
        }

        void setCountryIso(String countryIso) {
            mCountryIso = countryIso;
        }

        void setAllCellInfo(List<CellInfo> cellInfos) {
            mCellInfos = cellInfos != null
                    ? Collections.unmodifiableList(cellInfos)
                    : Collections.emptyList();
        }
    }
}
