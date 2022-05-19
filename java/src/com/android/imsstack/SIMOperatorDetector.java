package com.android.imsstack;

import android.content.Context;
import android.database.Cursor;
import android.net.Uri;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;

import com.android.imsstack.core.CommonStarter;
import com.android.imsstack.core.carrier.CarrierCode;
import com.android.imsstack.core.carrier.CarrierCodeLoader;
import com.android.imsstack.core.config.SmartConfigProviderInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsConstants;
import com.android.imsstack.util.ImsExtApi;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.ImsUtils;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.SODConfig;
import com.android.imsstack.util.SimCarrierId;
import com.android.internal.telephony.IccCardConstants;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import java.util.Locale;
import java.util.regex.Pattern;

public class SIMOperatorDetector {

    private static final String TAG = "ImsStack_SIMOperatorDetector";

    class Slot {
        // Flag to indicate that configuration needs to be updated into native logic
        boolean mDelivered = false;
        // Flag to indicate that IMS service is prohibited by any policies
        boolean mProhibited = false;
        // Flag to indicate which module is started or not
        boolean mInitialStart = true;
        boolean mCommonStart = false;
        boolean mVoLteStart = false;
        // Set to true when operator is changed
        boolean mChanged = false;
        // Set to true when supported services are changed
        boolean mUpdated = false;

        private final SODConfig.Operator mOperator;

        Slot(int slotId) {
            mOperator = new SODConfig.Operator(slotId);
        }

        SODConfig.Operator getOperatorInfo() {
            return mOperator;
        }

        @Override
        public String toString() {
            StringBuilder sb = new StringBuilder();

            sb.append("[ Slot: ch=");
            sb.append(mChanged ? 1 : 0);
            sb.append(", up=");
            sb.append(mUpdated ? 1 : 0);
            sb.append(", de=");
            sb.append(mDelivered ? 1 : 0);
            sb.append(", pr=");
            sb.append(mProhibited ? 1 : 0);
            sb.append(", ini_s=");
            sb.append(mInitialStart ? 1 : 0);
            sb.append(", com_s=");
            sb.append(mCommonStart ? 1 : 0);
            sb.append(", vol_s=");
            sb.append(mVoLteStart ? 1 : 0);
            sb.append(" ]");

            return sb.toString();
        }
    };

    class Sim {
        // Flag to indicate whether SIM is inserted or not
        boolean mEquiped = false;

        private final SODConfig.Sim mSim;

        Sim() {
            mSim = new SODConfig.Sim();
        }

        SODConfig.Sim getSimInfo() {
            return mSim;
        }
    };

    class ServiceProfile {
        private Slot mSlot;
        private Sim mSim;
        private SODConfig.SimProperties mSimProperties;

        ServiceProfile(int slotId) {
            mSlot = new Slot(slotId);
            mSim = new Sim();
            mSimProperties = new SODConfig.SimProperties();
        }

        Slot getSlot() {
            return mSlot;
        }

        Sim getSim() {
            return mSim;
        }

        SODConfig.SimProperties getSimProperties() {
            return mSimProperties;
        }
    };

    private final Context mContext;

    private final int mMaxSlot; // Single/DS/TS
    private final List<ServiceProfile> mServiceProfiles;
    private final SODConfig.Device mDevice;

    public SIMOperatorDetector(Context context) {
        mContext = context;
        mMaxSlot = MSimUtils.getMaxSimSlot();
        mServiceProfiles = new ArrayList<ServiceProfile>(mMaxSlot);

        for (int i = 0; i < mMaxSlot; i++) {
            mServiceProfiles.add(new ServiceProfile(i));
        }

        mDevice = new SODConfig.Device();
    }

    public synchronized void decideOperator(int slotId) {
        Log.i(TAG, "decideOperator(" + slotId + ")");

        if (!isSlotIdValid(slotId)) {
            return;
        }

        updateSysSimOperator(slotId);

        operatorDecision(slotId);

        if (getMaxSlot() > 1) {
            if (isMultiImsEnabled()) {
                setActive(slotId, true);
            }
            else {
                int ddsSlotId = getDefaultDataSlotId();
                setActive(slotId, (slotId == ddsSlotId));
            }
        }
        else {
            setActive(slotId, true);
        }

        checkProhibiting(slotId);

        updateServices(slotId);
    }

    public void deliverOperatorInfo(int slotId) {
        Log.i(TAG, "deliverOperatorInfo(" + slotId + ")");

        if (!isSlotIdValid(slotId)) {
            return;
        }

        Slot slot = getSlot(slotId);
        Sim sim = getSim(slotId);

        CommonStarter cs = CommonStarter.getInstance();
        cs.deliverOperatorInfo(slotId, slot.getOperatorInfo(), sim.getSimInfo(), mDevice,
                getSimProperties(slotId));
    }

    public void deliverUpdateServiceInfo(int slotId) {
        Log.i(TAG, "deliverUpdateServiceInfo(" + slotId + ")");

        if (!isSlotIdValid(slotId)) {
            return;
        }

        Slot slot = getSlot(slotId);
        Sim sim = getSim(slotId);

        CommonStarter cs = CommonStarter.getInstance();
        cs.deliverUpdateServiceInfo(slotId, slot.getOperatorInfo(), sim.getSimInfo(), mDevice);
    }

    public void deliverSimInfo(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        Sim sim = getSim(slotId);

        CommonStarter cs = CommonStarter.getInstance();
        cs.deliverSimInfo(slotId, sim.getSimInfo());
    }

    public int getDefaultSlot() {
        return MSimUtils.DEFAULT_SLOT_ID;
    }

    public int getMaxSlot() {
        return mMaxSlot;
    }

    public int getDefaultDataSlotId() {
        int subId = SubscriptionManager.getDefaultDataSubscriptionId();
        if (getMaxSlot() > 1) {
            if (subId != SubscriptionManager.INVALID_SUBSCRIPTION_ID) {
                int slotId = SubscriptionManager.getSlotIndex(subId);
                if (slotId != SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
                    Log.i(TAG, "DefaultDataSlotId - slotId=" + slotId + ", subId=" + subId);
                    return slotId;
                }
            }
        }

        Log.i(TAG, "DefaultDataSlotId - defaultSlot, subId=" + subId);

        return MSimUtils.DEFAULT_SLOT_ID;
    }

    public List<Integer> getOtherSlotIds(int slotId) {
        Log.i(TAG, "getOtherSlotIds(" + slotId + ")");

        if (getMaxSlot() < 2) {
            Log.i(TAG, "This device is only for Single SIM");
            return null;
        }

        List<Integer> slotIdList = new ArrayList<Integer>();
        for (int i = 0; i < getMaxSlot(); i++) {
            if (slotId != i) {
                slotIdList.add(i);
            }
        }

        Log.i(TAG, "Another SlotId List - " + slotIdList);
        if (slotIdList.isEmpty()) {
            return null;
        }

        return slotIdList;
    }

    public boolean isMultiImsEnabled() {
        return MSimUtils.isMultiImsEnabled() || MSimUtils.isMultiImsEnabledOnDssv();
    }

    public boolean isSlotIdValid(int slotId) {
        if (slotId < MSimUtils.DEFAULT_SLOT_ID || slotId >= getMaxSlot()) {
            StackTraceElement[] stes = (new Throwable()).getStackTrace();
            Log.d(TAG, "isSlotIdValid :: slotId=" + slotId + ", caller=" + stes[1].getMethodName()
                    + "(" + stes[1].getFileName() + ":" + stes[1].getLineNumber() + ")");
            return false;
        }

        return true;
    }

    // APIs for SLOT
    public int getSubId(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return MSimUtils.INVALID_SUB_ID;
        }

        Slot slot = getSlot(slotId);
        return slot.getOperatorInfo().getSubId();
    }

    public String getOperator(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        Slot slot = getSlot(slotId);
        return slot.getOperatorInfo().getOperator();
    }

    public String getCountry(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        Slot slot = getSlot(slotId);
        return slot.getOperatorInfo().getCountry();
    }

    public String getMccMnc(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        Slot slot = getSlot(slotId);
        return slot.getOperatorInfo().getMccMnc();
    }

    public String getEnablerType(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        Slot slot = getSlot(slotId);
        return slot.getOperatorInfo().getEnablerType();
    }

    public int getSupportServices(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return SODConfig.SUPPORT_NONE;
        }

        Slot slot = getSlot(slotId);
        return slot.getOperatorInfo().getServices();
    }

    public boolean isActive(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Slot slot = getSlot(slotId);
        return slot.getOperatorInfo().isActive();
    }

    public boolean isAvailable(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Slot slot = getSlot(slotId);
        return slot.getOperatorInfo().isAvailable();
    }

    public boolean isChanged(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Slot slot = getSlot(slotId);
        return slot.mChanged;
    }

    public boolean isDelivered(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Slot slot = getSlot(slotId);
        return slot.mDelivered;
    }

    public boolean isInitialStart(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Slot slot = getSlot(slotId);
        return slot.mInitialStart;
    }

    public boolean isCommonStart(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Slot slot = getSlot(slotId);
        return slot.mCommonStart;
    }

    public boolean isInboudRoaming(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Slot slot = getSlot(slotId);
        return slot.getOperatorInfo().isInboundRoaming();
    }

    public boolean isProhibited(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Slot slot = getSlot(slotId);
        return slot.mProhibited;
    }

    public boolean isUpdated(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Slot slot = getSlot(slotId);
        return slot.mUpdated;
    }

    public boolean isVoLTEStart(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Slot slot = getSlot(slotId);
        return slot.mVoLteStart;
    }

    // APIs related with OPERATRO_BASED_ON
    private boolean isOperatorBasedOn(int flag) {
        return mDevice.isOperatorBasedOn(flag);
    }

    public boolean isSIMBasedOn() {
        return isOperatorBasedOn(SODConfig.SIM_BASED);
    }

    public boolean isTargetBasedOn() {
        return isOperatorBasedOn(SODConfig.TARGET_BASED);
    }

    public boolean isNaOpen() {
        return (isSIMBasedOn() && isOperatorBasedOn(SODConfig.NA_OPEN));
    }

    public boolean isSmsOnly() {
        return isOperatorBasedOn(SODConfig.SMS_ONLY);
    }

    public boolean isSmsOnlyForNaOpen() {
        return (isNaOpen() && isSmsOnly());
    }

    public boolean isKrOpen() {
        return isOperatorBasedOn(SODConfig.KR_OPEN);
    }

    public boolean isSupportSIMMoved() {
        return isOperatorBasedOn(SODConfig.SUPPORT_SIM_MOVED);
    }

    public boolean isTargetOpenOnLaop() {
        return (isTargetBasedOn() && isOperatorBasedOn(SODConfig.TARGET_OPEN));
    }
    // APIs related with OPERATOR_BASED_ON

    public boolean isEnablerTypeForNonOperator(int slotId) {
        return SODConfig.isEnablerTypeForNonOperator(getEnablerType(slotId));
    }

    public boolean isVoLTEAvailable(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Slot slot = getSlot(slotId);
        SODConfig.Operator opInfo = slot.getOperatorInfo();

        return opInfo.isServiceOn(SODConfig.SUPPORT_VOLTE)
                || opInfo.isServiceOn(SODConfig.SUPPORT_VT)
                || opInfo.isServiceOn(SODConfig.SUPPORT_VOWIFI);
    }

    public boolean isVoLTESupported(int slotId) {
        return isServiceSupported(slotId, SODConfig.SUPPORT_VOLTE);
    }

    public boolean isVoLteEmergencySupported(int slotId) {
        return isServiceSupported(slotId, SODConfig.SUPPORT_VOLTE_EMERGENCY);
    }

    public boolean isVoWiFiSupported(int slotId) {
        return isServiceSupported(slotId, SODConfig.SUPPORT_VOWIFI);
    }

    public boolean isVTSupported(int slotId) {
        return isServiceSupported(slotId, SODConfig.SUPPORT_VT);
    }

    public void setActive(int slotId, boolean active) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        Slot slot = getSlot(slotId);
        slot.getOperatorInfo().setActive(active);
    }

    public void setInitialStart(int slotId, boolean started) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        Slot slot = getSlot(slotId);
        slot.mInitialStart = started;
    }

    public void setCommonStart(int slotId, boolean started) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        Slot slot = getSlot(slotId);
        slot.mCommonStart = started;
    }

    public void setChanged(int slotId, boolean changed) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        Slot slot = getSlot(slotId);
        slot.mChanged = changed;
    }

    public void setDelivered(int slotId, boolean delivered) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        Slot slot = getSlot(slotId);
        slot.mDelivered = delivered;
    }

    private void setInboundRoaming(int slotId, boolean inbounded) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        Slot slot = getSlot(slotId);
        slot.getOperatorInfo().setInboundRoaming(inbounded);
    }

    private void setProhibited(int slotId, boolean prohibited) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        Slot slot = getSlot(slotId);

        if (slot.mProhibited != prohibited) {
            Log.i(TAG, "ImsPolicy :: Service " + (prohibited ? "blocked" : "not blocked"));
        }

        slot.mProhibited = prohibited;
    }

    public void setUpdated(int slotId, boolean updated) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        Slot slot = getSlot(slotId);

        if (slot.mUpdated != updated) {
            if (updated) {
                Log.i(TAG, "ImsPolicy :: Service updated");
            }
        }

        slot.mUpdated = updated;
    }

    public void setVoLTEStart(int slotId, boolean started) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        Slot slot = getSlot(slotId);
        slot.mVoLteStart = started;
    }

    public boolean updateServices(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        boolean bVoLTE = false;
        boolean bVT = false;
        boolean bVoWiFi = false;
        boolean bVoLteEmergency = false;

        if (isProhibited(slotId)) {
            bVoLTE  = false;
            bVT     = false;
            bVoWiFi = false;

            ImsUtils.setServiceCapsToLocalStorage(slotId, null);
        }
        else {
            ImsUtils.ServiceCaps serviceCaps = ImsUtils.getServiceCapsByPlatform(mContext, slotId);

            Log.i(TAG, serviceCaps.toString());

            if (ImsConstants.USE_CARRIER_CONFIG
                    && !isEquipInSim(slotId) && isSIMRemoved(slotId)) {
                // Use old service capabilities (previous SIM's service capabilities)
                ImsUtils.ServiceCaps oldSC = ImsUtils.getServiceCapsFromLocalStorage(slotId);

                serviceCaps = (oldSC != null) ? oldSC : serviceCaps;

                Log.i(TAG, "LocalStorage: " + ((oldSC != null) ? oldSC.toString() : "(null)"));
            }

            bVoLTE = serviceCaps.isVoLteEnabled();
            bVT = serviceCaps.isVtEnabled();
            bVoWiFi = serviceCaps.isWfcEnabled();

            // VOLTE_EMERGENCY_CALLING
            if (!bVoLTE || isEmergencyCallAvailableOnSimAbsentForTracfone(slotId, serviceCaps)) {
                boolean isSimAbsentOrLocked = isSIMAbsent(slotId) || isSIMLocked(slotId);

                if (isSimAbsentOrLocked
                        && ImsUtils.isEmergencyCallEnabledOnServiceRestricted()) {
                    Log.i(TAG, "SimAbsentOrLocked: VoLTE is enabled for IMS e-call");
                    bVoLTE = true;
                    bVoLteEmergency = true;
                } else if (ImsConstants.USE_CARRIER_CONFIG
                        && ImsUtils.isDeviceEncryptionModeEnabledAsFDE()) {
                    Log.i(TAG, "EncryptionMode: VoLTE is enabled for IMS e-call");
                    bVoLTE = true;
                    bVoLteEmergency = true;
                }
            }

            // VOLTE_EMERGENCY_CALLING
            if (!bVoLTE && !bVT && !bVoWiFi) {
                if (ImsUtils.isEmergencyCallEnabledOnNonVoLteSim()) {
                    Log.d(TAG, "NonVoLteSim: VoLTE is enabled for IMS e-call");
                    bVoLTE = true;
                    bVoLteEmergency = true;
                }
            }

            ImsUtils.setServiceCapsToLocalStorage(slotId, serviceCaps);
        }

        if (bVoLTE == isVoLTESupported(slotId)
                && bVT == isVTSupported(slotId)
                && bVoWiFi == isVoWiFiSupported(slotId)
                && bVoLteEmergency == isVoLteEmergencySupported(slotId)) {
            Log.d(TAG, "ImsPolicy :: updateServices - Service not changed");
            return false;
        }

        setUpdated(slotId, true);
        setSupportServices(slotId, 0x00);

        if (bVoLTE) {
            enableSupportService(slotId, SODConfig.SUPPORT_VOLTE);
        }
        else {
            disableSupportService(slotId, SODConfig.SUPPORT_VOLTE);
        }
        if (bVT) {
            enableSupportService(slotId, SODConfig.SUPPORT_VT);
        }
        else {
            disableSupportService(slotId, SODConfig.SUPPORT_VT);
        }
        if (bVoWiFi) {
            enableSupportService(slotId, SODConfig.SUPPORT_VOWIFI);
        }
        else {
            disableSupportService(slotId, SODConfig.SUPPORT_VOWIFI);
        }

        if (bVoLteEmergency) {
            enableSupportService(slotId, SODConfig.SUPPORT_VOLTE_EMERGENCY);
        } else {
            disableSupportService(slotId, SODConfig.SUPPORT_VOLTE_EMERGENCY);
        }

        return true;
    }

    private void checkProhibiting(int slotId) {
        Cursor cSmartModifiedTime = getSmartModifiedTime();
        if (cSmartModifiedTime == null) {
            Log.e(TAG, "SmartConfig DB is not normal");
            setProhibited(slotId, true);
            return;
        }
        else {
            String version = getValueInCursor(cSmartModifiedTime,
                SmartConfigProviderInterface.SmartModifiedTime.VERSION, "1");
            cSmartModifiedTime.close();
            if ("2".equals(version)) {
                checkProhibiting2(slotId);
                return;
            }
        }

        String mccmnc = getMccMncInSim(slotId);
        if (TextUtils.isEmpty(mccmnc) || "45000".equals(mccmnc) || "00101".equals(mccmnc)) {
            Log.i(TAG, "This SIM(" + mccmnc + ") is ignored in CheckProhibiting");
            setProhibited(slotId, false);
            return;
        }

        // Checks whether operator is prohibited or not by operator_list
        Cursor cOperator = getOperatorFromOperatorList(getOperator(slotId), getCountry(slotId));
        if (cOperator == null) {
            if (ImsUtils.isEmergencyCallEnabledOnNonVoLteSim()) {
                // Do not prohibit the service even though operator list is not listed.
                setProhibited(slotId, isServiceBlockedByAcceptanceList(mccmnc));
            } else {
                setProhibited(slotId, true);
            }
            return;
        }

        cOperator.close();

        // Checking whether mccmnc is prohibited or not by service_acceptance_list
        setProhibited(slotId, isServiceBlockedByAcceptanceList(mccmnc));
    }

    private void checkProhibiting2(int slotId) {
        String mccmnc = getMccMncInSim(slotId);
        if (TextUtils.isEmpty(mccmnc) || isCommonTestSim(mccmnc)) {
            Log.i(TAG, "This SIM(" + mccmnc + ") is ignored in CheckProhibiting2");
            setProhibited(slotId, false);
            return;
        }

        String simOperator = getOperatorInSim(slotId);
        String simCountry = getCountryInSim(slotId);
        Cursor cSimOperator = getOperatorFromOperatorList(simOperator, simCountry);
        if (cSimOperator == null) {
            if (ImsUtils.isEmergencyCallEnabledOnNonVoLteSim()) {
                // Service acceptance list is prior to IMS emergency call on non-VoLTE SIM.
                setProhibited(slotId, isServiceBlockedByAcceptanceList(mccmnc));
            } else {
                setProhibited(slotId, true);
            }
            return;
        }

        boolean serviceAllowed = true;
        // Checks whether IMS service is prohibited or not by TOTC
        String totc = getSimOperator(slotId) + "/" + getSimCountry(slotId);
        Cursor cToTc = getServiceEnableListByToTc(totc);

        Log.d(TAG, "checkProhibiting2 :: TOTC - " + totc);

        if (cToTc != null) {
            serviceAllowed = isServiceEnabled(slotId, cSimOperator, cToTc);

            boolean inboundRoamingAllowed = false;

            if (!serviceAllowed && "KR".equalsIgnoreCase(simCountry)) {
                inboundRoamingAllowed = isInboundRoaming(slotId, cSimOperator, cToTc);
                if (inboundRoamingAllowed) {
                    Log.w(TAG, "SIM(" + simOperator + "/" + simCountry
                        + ") is for InboundRoaming");

                    serviceAllowed = true;
                }
            }

            setInboundRoaming(slotId, inboundRoamingAllowed);

            cToTc.close();

            Log.w(TAG, "SIM is " + (serviceAllowed ? "Not Prohibited" : "Prohibited")
                + " based on TOTC(" + totc + ")");
        }

        setProhibited(slotId, !serviceAllowed);

        cSimOperator.close();
    }

    private boolean isServiceBlockedByAcceptanceList(String mccmnc) {
        Log.w(TAG, "ImsPolicy :: service block check skipped - " + mccmnc);
        return false;
    }

    private boolean isServiceEnabled(int slotId, Cursor cSimOperator, Cursor cProhibit) {
        boolean bEnabled = true;
        boolean bExisted = false;

        String mccmnc = getMccMncInSim(slotId);
        String country = getCountryInSim(slotId);
        String opco = getOperatorInSim(slotId) + "/" + country;
        bExisted = isItExistedInList(cProhibit,
            SmartConfigProviderInterface.ServiceEnableListByNtCode.OPERATOR, opco);
        if (bExisted != true) {
            bExisted = isItExistedInList(cProhibit,
                SmartConfigProviderInterface.ServiceEnableListByNtCode.COUNTRY, country);
        }
        if (bExisted != true) {
            bExisted = isItExistedInList(cProhibit,
                SmartConfigProviderInterface.ServiceEnableListByNtCode.MCCMNC, mccmnc);
        }
        if (bExisted != true) {
            bExisted = isItExistedInList(cProhibit,
                SmartConfigProviderInterface.ServiceEnableListByNtCode.MCC,
                mccmnc.substring(0, 3));
        }
        if (bExisted != true) {
            String region = getValueInCursor(cSimOperator,
                SmartConfigProviderInterface.OperatorList.REGION, "");
            if (!TextUtils.isEmpty(region)) {
                bExisted = isItExistedInList(cProhibit,
                    SmartConfigProviderInterface.ServiceEnableListByNtCode.REGION, region);
            }
        }
        if (bExisted != true) {
            String group_id = getValueInCursor(cSimOperator,
                SmartConfigProviderInterface.OperatorList.GROUP_ID, "");
            if (!TextUtils.isEmpty(group_id)) {
                bExisted = isItExistedInList(cProhibit,
                    SmartConfigProviderInterface.ServiceEnableListByNtCode.GROUP_ID, group_id);
            }
        }

        if (bExisted) {
            boolean bExempt1 = isItExistedInList(cProhibit,
                SmartConfigProviderInterface.ServiceEnableListByNtCode.OPERATOR_EXEMPT,
                opco);
            boolean bExempt2 = isItExistedInList(cProhibit,
                SmartConfigProviderInterface.ServiceEnableListByNtCode.MCCMNC_EXEMPT,
                mccmnc);

            if (!bExempt2) {
                String mccWildcard = String.format(Locale.US, "%s*", mccmnc.substring(0, 3));

                bExempt2 = isItExistedInList(cProhibit,
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.MCCMNC_EXEMPT,
                        mccWildcard);
            }

            if (bExempt1 || bExempt2) {
                bEnabled = false;
            }
        }
        else {
            bEnabled = false;
        }

        return bEnabled;
    }

    private boolean isInboundRoaming(int slotId, Cursor cSimOperator, Cursor cProhibit) {
        boolean bInbound = false;

        Cursor cOperator = getOperatorFromOperatorList(getOperator(slotId), getCountry(slotId));
        if (cOperator == null) {
            Log.i(TAG, "Operator(" + getOperator(slotId) + "/" + getCountry(slotId)
                + ") is not existed in operator_list");
            return bInbound;
        }

        String basedOn = getValueInCursor(cOperator,
                SmartConfigProviderInterface.OperatorList.OPERATOR_BASED_ON, "");
        if (SODConfig.OPERATOR_BASED_ON_TARGET.equalsIgnoreCase(basedOn)) {
            String opco = getOperatorInSim(slotId) + "/" + getCountryInSim(slotId);
            bInbound = isItExistedInList(cProhibit,
                SmartConfigProviderInterface.ServiceEnableListByNtCode.INBOUNDING_ROAMING, opco);
        }

        cOperator.close();

        return bInbound;
    }

    private void disableSupportService(int slotId, int service) {
        Slot slot = getSlot(slotId);
        slot.getOperatorInfo().resetService(service);;
    }

    private void enableSupportService(int slotId, int service) {
        Slot slot = getSlot(slotId);
        slot.getOperatorInfo().setService(service);
    }

    private boolean isItExistedInList(String strList, String str, boolean checkWildcard) {
        ArrayList<String> list = new ArrayList<String>(Arrays.asList(strList.split(",")));
        if (list.contains(str)) {
            return true;
        }

        if (checkWildcard) {
            if (list.contains("*")) {
                return true;
            }
        }

        return false;
    }

    private boolean isItExistedInList(Cursor cursor, String field, String str) {
        if (cursor == null) {
            return true;
        }

        String strList = getValueInCursor(cursor, field, "");
        boolean bExisted = (!TextUtils.isEmpty(strList) && isItExistedInList(strList, str, true));
        Log.i(TAG, "Str(" + str + ") is " + (bExisted ? "existed" : " not existed")
            + " in " + field + " List(" + strList + ")");

        return bExisted;
    }

    private boolean isServiceSupported(int slotId, int service) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Slot slot = getSlot(slotId);
        return slot.getOperatorInfo().isServiceOn(service);
    }

    private void setEnablerType(int slotId, String enabler) {
        Slot slot = getSlot(slotId);
        slot.getOperatorInfo().setEnablerType(enabler);
    }

    private void setSupportServices(int slotId, int service) {
        Slot slot = getSlot(slotId);
        slot.getOperatorInfo().setServices(service);
    }
    // APIs for SLOT

    // APIs for SIM
    public String getOperatorInSim(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        Sim sim = getSim(slotId);
        return sim.getSimInfo().getOperator();
    }

    public String getCountryInSim(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        Sim sim = getSim(slotId);
        return sim.getSimInfo().getCountry();
    }

    public String getMccMncInSim(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        Sim sim = getSim(slotId);
        return sim.getSimInfo().getMccMnc();
    }

    public int getSimState(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return SODConfig.SIM_STATE_NONE;
        }

        Sim sim = getSim(slotId);
        return sim.getSimInfo().getState();
    }

    private static int getSimState(String iccState) {
        if (IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(iccState)) {
            return SODConfig.SIM_STATE_LOADED;
        }
        else if (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(iccState)) {
            return SODConfig.SIM_STATE_ABSENT;
        }
        else if (IccCardConstants.INTENT_VALUE_ICC_LOCKED.equals(iccState)) {
            return SODConfig.SIM_STATE_LOCKED;
        }
        else if (ImsExtApi.Uicc.SIM_REMOVED.equals(iccState)) {
            return SODConfig.SIM_STATE_REMOVED;
        }

        return SODConfig.SIM_STATE_NONE;
    }

    public boolean getTheLatestOperatorInfo(int slotId) {
        String sim_op = SODConfig.getValueForCachedOperatorInfo(
                slotId, SODConfig.KEY_SIM_OPERATOR);
        String sim_co = SODConfig.getValueForCachedOperatorInfo(
                slotId, SODConfig.KEY_SIM_COUNTRY);
        String sim_mccmnc = SODConfig.getValueForCachedOperatorInfo(
                slotId, SODConfig.KEY_MCCMNC);
        String sim_gid = SODConfig.getValueForCachedOperatorInfo(
                slotId, SODConfig.KEY_GID);
        String sim_spn = SODConfig.getValueForCachedOperatorInfo(
                slotId, SODConfig.KEY_SPN);
        String sim_imsi = SODConfig.getValueForCachedOperatorInfo(
                slotId, SODConfig.KEY_IMSI);

        setSIMInfo(slotId, sim_op, sim_co, sim_mccmnc, sim_gid, sim_spn, sim_imsi);

        if (TextUtils.isEmpty(sim_op)) {
            Log.w(TAG, "No cached operator-info");
            return false;
        }

        return true;
    }

    private boolean isMccMncValid(String mcc, String mnc) {
        if (mcc.length() != 3 || (mnc.length() != 2 && mnc.length() != 3)) {
            return false;
        }

        for (int i = 0; i < mcc.length(); i++) {
            if (mcc.charAt(i) < '0' || mcc.charAt(i) > '9') {
                return false;
            }
        }
        for (int i = 0; i < mnc.length(); i++) {
            if (mnc.charAt(i) < '0' || mnc.charAt(i) > '9') {
                return false;
            }
        }

        return true;
    }

    public boolean isTestSimForNaOpen(int slotId) {
        String mccmnc = getMccMncFromSim(slotId, "");
        return (isCommonTestSim(mccmnc) || "00202".equals(mccmnc));
    }

    public boolean isSIMAbsent(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Sim sim = getSim(slotId);
        return sim.getSimInfo().isStateOn(SODConfig.SIM_STATE_ABSENT);
    }

    public static boolean isSIMAbsent(String ss) {
        return (getSimState(ss) == SODConfig.SIM_STATE_ABSENT);
    }

    public boolean isSIMLoaded(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Sim sim = getSim(slotId);
        return sim.getSimInfo().isStateOn(SODConfig.SIM_STATE_LOADED);
    }

    public static boolean isSIMLoaded(String ss) {
        return (getSimState(ss) == SODConfig.SIM_STATE_LOADED);
    }

    public boolean isSIMLocked(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Sim sim = getSim(slotId);
        return sim.getSimInfo().isStateOn(SODConfig.SIM_STATE_LOCKED);
    }

    public static boolean isSIMLocked(String ss) {
        return (getSimState(ss) == SODConfig.SIM_STATE_LOCKED);
    }

    public boolean isSIMRemoved(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Sim sim = getSim(slotId);
        return sim.getSimInfo().isStateOn(SODConfig.SIM_STATE_REMOVED);
    }

    public static boolean isSIMRemoved(String ss) {
        return (getSimState(ss) == SODConfig.SIM_STATE_REMOVED);
    }

    public boolean isFinalSimState(int slotId) {
        return (isSIMAbsent(slotId) || isSIMRemoved(slotId) ||
                isSIMLoaded(slotId) || isSIMLocked(slotId));
    }

    public boolean isEquipInSim(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        Sim sim = getSim(slotId);
        return sim.mEquiped;
    }

    public void setEquipInSim(int slotId, boolean equip) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        Sim sim = getSim(slotId);
        sim.mEquiped = equip;
    }

    public void setSimState(int slotId, String iccState) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        int simState = getSimState(iccState);

        if (simState != SODConfig.SIM_STATE_NONE) {
            SODConfig.Sim simInfo = getSim(slotId).getSimInfo();
            simInfo.setState(simState);
        }
    }

    public void setSimState(int slotId, int state) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        SODConfig.Sim simInfo = getSim(slotId).getSimInfo();

        simInfo.clearState();
        simInfo.setState(state);
    }
    // APIs for SIM

    public void showAllInfo(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        Slot slot = getSlot(slotId);
        Sim sim = getSim(slotId);

        SODConfig.Operator opInfo = slot.getOperatorInfo();
        SODConfig.Sim simInfo = sim.getSimInfo();

        Log.w(TAG, "ImsPolicy: " + slot.toString() + ", " + opInfo.toString());
        Log.w(TAG, "ImsPolicy: [ SIM: equiped=" + sim.mEquiped + " ], " + simInfo.toString());
        Log.w(TAG, "ImsPolicy: [ DEV: operatorBasedOn=0x"
                + Integer.toHexString(mDevice.getOperatorBasedOn())
                + ", simMovedSupported=" + mDevice.isSimMovedSupported() + " ]");

        Log.d(TAG, "ImsPolicy: [ PROP: operator=" + getSimOperator(slotId)
                + ", country=" + getSimCountry(slotId)
                + ", sysSimOp=" + getSysSimOperator(slotId)
                + ", sysSimOpSub=" + getSysSimOperatorSub(slotId)
                + ", model=" + ImsProperties.MODEL + ", build=" + android.os.Build.TYPE
                + ", maxSlot=" + getMaxSlot() + ", multi-IMS=" + isMultiImsEnabled() + " ]");
    }

    // Private Method APIs
    private void operatorDecision(int slotId) {
        Log.i(TAG, "operatorDecision(" + slotId + ")");

        String targetOp = getSimOperator(slotId);
        String targetCo = getSimCountry(slotId);

        boolean bOperatorBasedOn = updateOperatorBasedOn(slotId, targetOp, targetCo);
        if (bOperatorBasedOn != true) {
            // If preferred operator/country is set for test purpose, use it for operator/country
            if (checkAndSetPreferredOperator(slotId)) {
                return;
            }

            setUnknownPolicy(slotId, targetOp, targetCo, SODConfig.ENABLER_TYPE_OPERATOR);
            return;
        }

        // If preferred operator/country is set for test purpose, use it for operator/country
        if (checkAndSetPreferredOperator(slotId)) {
            return;
        }

        if (isTargetBasedOn()) {
            if (isTargetOpenOnLaop()) {
                String defaultOp = CarrierCodeLoader.getDefaultSimOperator();

                if (TextUtils.isEmpty(defaultOp)) {
                    defaultOp = "ATT";
                } else {
                    defaultOp = ImsProperties.getImsOperator(defaultOp);
                }

                setTargetOpenPolicyOnLaop(slotId, getSysSimOperator(slotId), defaultOp, "US");
            }
            else if (isSupportSIMMoved()) {
                setTargetPolicyWithSimMoved(slotId);
            }
            else {
                setTargetPolicy(slotId);
            }
        }
        // SIM BasedOn, by default
        else {
            if (isNaOpen()) {
                setSIMPolicyForNaOpen(slotId, getSysSimOperator(slotId),
                        getDefaultOperatorForNaOpen(), "US");
            }
            else if ("CA".equalsIgnoreCase(targetCo)) {
                setSIMPolicyForCA(slotId);
            }
            else if (isKrOpen()) {
                setSIMPolicyForKROpen(slotId);
            }
            else {
                setSIMPolicy(slotId);
            }
        }
    }

    private void setPreferenceOperatorPolicy(int slotId, String operator, String country) {
        Log.i(TAG, "setPreferenceOperatorPolicy :: slotId=" + slotId
                + ", op=" + operator + ", co=" + country);

        String preferenceOperator = operator;
        String preferenceCountry = country;
        String strMccMnc  = "";
        String strGid     = "";
        String strSpn     = "";
        String strImsi    = "";
        String strEnabled = "";

        String[] aMncInfo = parseMccMnc(country, operator);
        if (aMncInfo != null) {
            strMccMnc = country + aMncInfo[0];

            Cursor cursor_list = getMccMncFromMccMncList(strMccMnc);
            if (cursor_list != null) {
                String gid = "";
                String spn = "";
                String imsi = "";
                if (aMncInfo.length > 1) {
                    gid = (TextUtils.isEmpty(aMncInfo[1]) ? "" : aMncInfo[1]);
                }
                if (aMncInfo.length > 2) {
                    spn = (TextUtils.isEmpty(aMncInfo[2]) ? "" : aMncInfo[2]);
                }
                if (aMncInfo.length > 3) {
                    imsi = (TextUtils.isEmpty(aMncInfo[3]) ? "" : aMncInfo[3]);
                }

                int iPosition = getBestMatchedOperator(cursor_list, gid, spn, imsi);
                if (iPosition != -1) {
                    if (cursor_list.moveToPosition(iPosition)) {
                        preferenceOperator = getValueInCursor(cursor_list,
                                SmartConfigProviderInterface.MccMncList.OPERATOR, "");
                        preferenceCountry = getValueInCursor(cursor_list,
                                SmartConfigProviderInterface.MccMncList.COUNTRY, "");
                        strGid = getValueInCursor(cursor_list,
                                SmartConfigProviderInterface.MccMncList.GID, "");
                        strSpn = getValueInCursor(cursor_list,
                                SmartConfigProviderInterface.MccMncList.SPN, "");
                        strImsi = getValueInCursor(cursor_list,
                                SmartConfigProviderInterface.MccMncList.IMSI, "");
                        strEnabled = getValueInCursor(cursor_list,
                                SmartConfigProviderInterface.MccMncList.ENABLED, "");
                    }
                }
                cursor_list.close();
            }

            Log.w(TAG, "Preference Information = "
                + strMccMnc + "/" + preferenceCountry + "/" + preferenceOperator + "/"
                + strGid + "/" + strSpn + "/" + Log.pii(strImsi) + "/"
                + strEnabled);
        }

        setSIMInfo(slotId, preferenceOperator, preferenceCountry,
                strMccMnc, strGid, strSpn, strImsi);

        String op = "";
        String co = "";
        Cursor cPrefInfo = getOperatorFromOperatorList(preferenceOperator, preferenceCountry);
        if (cPrefInfo != null) {
            op = getValueInCursor(cPrefInfo,
                    SmartConfigProviderInterface.OperatorList.OPERATOR, "");
            co = getValueInCursor(cPrefInfo,
                    SmartConfigProviderInterface.OperatorList.COUNTRY, "");
            cPrefInfo.close();
        }

        if (!TextUtils.isEmpty(op) && !TextUtils.isEmpty(co)) {
            setOperatorInfo(slotId, op, co, false);
        }
        else {
            setUnknownPolicy(slotId, preferenceOperator, preferenceCountry,
                    SODConfig.ENABLER_TYPE_OPERATOR);
        }
    }

    private int equalGid(Cursor cursor, String gid) {
        if (cursor == null) {
            return 0;
        }

        String cGid = getValueInCursor(cursor, SmartConfigProviderInterface.MccMncList.GID, "");
        if (TextUtils.isEmpty(cGid)) {
            return -1;
        }
        else {
            Log.w(TAG, "EqualGID = " + cGid.toUpperCase(Locale.ROOT) + "/" + gid);
            return (gid.startsWith(cGid.toUpperCase(Locale.ROOT)) ? 1 : 0);
        }
    }

    private int equalSpn(Cursor cursor, String spn) {
        if (cursor == null) {
            return 0;
        }

        String cSpn = getValueInCursor(cursor, SmartConfigProviderInterface.MccMncList.SPN, "");
        if (TextUtils.isEmpty(cSpn)) {
            return -1;
        }
        else {
            Log.w(TAG, "EqualSPN = " + cSpn + "/" + spn);
            return (cSpn.equalsIgnoreCase(spn) ? 1 : 0);
        }
    }

    private int matchedImsi(Cursor cursor, String imsi) {
        if (cursor == null) {
            return 0;
        }

        String cImsi = getValueInCursor(cursor, SmartConfigProviderInterface.MccMncList.IMSI, "");
        if (TextUtils.isEmpty(cImsi)) {
            return -1;
        }
        else {
            Log.w(TAG, "MatchedIMSI = " + cImsi + "/" + imsi);
            return (imsi.startsWith(cImsi) ? 1 : 0);
        }
    }

    private int getBestMatchedOperator(Cursor cursors, String gid, String spn, String imsi) {
        if (cursors == null) {
            Log.w(TAG, "Cursor List is null !!!");
            return -1;
        }

        int bestMatchedPriority = 0;
        int iBestMatchedCursor = -1;
        while (!cursors.isAfterLast()) {
            int currentMatchedPriority = 0;
            int iEqualGid = equalGid(cursors, gid);
            int iEqualSpn = equalSpn(cursors, spn);
            int iMatchedImsi = matchedImsi(cursors, imsi);

            if (iEqualGid == 1 && iEqualSpn == 1 && iMatchedImsi == 1) {
                iBestMatchedCursor = cursors.getPosition();
                bestMatchedPriority = 8;
                break;
            }
            if (iEqualGid == 1 && iEqualSpn == -1 && iMatchedImsi == 1) {
                currentMatchedPriority = 7;
            }
            else if (iEqualGid == -1 && iEqualSpn == 1 && iMatchedImsi == 1) {
                currentMatchedPriority = 6;
            }
            else if (iEqualGid == -1 && iEqualSpn == -1 && iMatchedImsi == 1) {
                currentMatchedPriority = 5;
            }
            else if (iEqualGid == 1 && iEqualSpn == 1 && iMatchedImsi == -1) {
                currentMatchedPriority = 4;
            }
            else if (iEqualGid == 1 && iEqualSpn == -1 && iMatchedImsi == -1) {
                currentMatchedPriority = 3;
            }
            else if (iEqualGid == -1 && iEqualSpn == 1 && iMatchedImsi == -1) {
                currentMatchedPriority = 2;
            }
            else if (iEqualGid == -1 && iEqualSpn == -1 && iMatchedImsi == -1) {
                currentMatchedPriority = 1;
            }

            if (currentMatchedPriority > bestMatchedPriority) {
                bestMatchedPriority = currentMatchedPriority;
                iBestMatchedCursor = cursors.getPosition();
            }

            cursors.moveToNext();
        }
        Log.d(TAG, "BestMatchedPriority = " + bestMatchedPriority
                + ", BestMatchedCursorPosition = " + iBestMatchedCursor);

        return iBestMatchedCursor;
    }

    private String[] parseMccMnc(String mcc, String mnc) {
        Log.w(TAG, "parseMccMnc(" + mcc + "/" + mnc);

        String trimedMnc = mnc.replaceAll("\\p{Z}",  "");
        String[] aStr = trimedMnc.split(",");

        if (isMccMncValid(mcc, aStr[0]) != true) {
            return null;
        }

        return aStr;
    }

    private void setSIMPolicyForNaOpen(int slotId, String carrier, String def_op, String def_co) {
        boolean valid = true;
        if (isSIMAbsent(slotId)) {
            valid = setDefaultSIMPolicyForNaOpen(slotId, carrier, def_op, def_co);
        }
        else {
            if (isTestSimForNaOpen(slotId)) {
                valid = setDefaultSIMPolicyForNaOpen(slotId, carrier, def_op, def_co);
            }
            else {
                String simOperator = def_op;
                String simCountry = def_co;

                if (ImsProperties.isUnknownOperator(carrier)) {
                    Log.w(TAG, "sim is not suppored, set default operator("
                        + def_op + "/" + def_co + ")");
                }
                else {
                    Cursor cCarrier = getCarrierFromNaoCarrierList(carrier);
                    if (cCarrier != null) {
                        simOperator = getValueInCursor(cCarrier,
                                SmartConfigProviderInterface.NaoCarrierList.OPERATOR, def_op);
                        simCountry = getValueInCursor(cCarrier,
                                SmartConfigProviderInterface.NaoCarrierList.COUNTRY, def_co);
                        cCarrier.close();
                    }
                    else {
                        Log.w(TAG, "carrier for " + carrier + " is not existed...");
                    }
                }
                setSIMInfo(slotId, simOperator, simCountry,
                        getMccMncFromSim(slotId, ""), "", "", "");
            }
        }

        Cursor cOperator = null;
        if (valid) {
            SODConfig.Sim simInfo = getSim(slotId).getSimInfo();
            cOperator = getOperatorFromOperatorList(simInfo.getOperator(), simInfo.getCountry());
        }

        String op = def_op;
        String co = def_co;
        if (cOperator != null) {
            op = getValueInCursor(
                    cOperator, SmartConfigProviderInterface.OperatorList.OPERATOR, def_op);
            co = getValueInCursor(
                    cOperator, SmartConfigProviderInterface.OperatorList.COUNTRY, def_co);
            setOperatorInfo(slotId, op, co, false);
            cOperator.close();
        }
        else {
            setUnknownPolicy(slotId, def_op, def_co, SODConfig.ENABLER_TYPE_OPERATOR);
        }

        Log.i(TAG, "slotId/carrier/operator/country/def_op/def_co="
            + slotId + "/" + carrier + "/" + op + "/" + co + "/" + def_op + "/" + def_co);
    }

    private boolean setDefaultSIMPolicyForNaOpen(int slotId,
            String carrier, String def_op, String def_co) {
        String sim_op = def_op;
        String sim_co = def_co;

        boolean bUnknownCarrier = ImsProperties.isUnknownOperator(carrier);

        if (bUnknownCarrier != true) {
            sim_op = carrier;
        }

        // Check whether SMS_ONLY is supported or not
        // This chages have just implemented only for NA OPEN
        Uri uri = Uri.parse("content://com.android.imsstack.provider.gims/gims_db_info");
        String selection = "id='" + slotId + "'";
        Cursor cursor = getCursor(uri, selection);
        String filename = null;

        if (cursor != null) {
            filename = getValueInCursor(cursor, "config_xml_file", "");
            cursor.close();
        }

        Cursor cCarrier = getCarrierFromNaoCarrierList(carrier);

        if (cCarrier != null) {
            sim_op = getValueInCursor(cCarrier,
                    SmartConfigProviderInterface.NaoCarrierList.OPERATOR, def_op);
            sim_co = getValueInCursor(cCarrier,
                    SmartConfigProviderInterface.NaoCarrierList.COUNTRY, def_co);
            cCarrier.close();
        }
        else {
            Log.w(TAG, "carrier for " + carrier + " is not existed...");
        }

        if (!TextUtils.isEmpty(filename)) {
            Log.w(TAG, "filename = " + filename);
            String[] tokens = filename.split(Pattern.quote("."));
            if (tokens.length >= 5) {
                //tokens[1]; // Operator
                //tokens[2]; // Country
                if (sim_op.equalsIgnoreCase(tokens[1])) {
                    if (tokens[3].equals("SMSONLY")) {
                        mDevice.setOperatorBasedOn(SODConfig.SMS_ONLY);
                    }
                }
            }
        }

        setSIMInfo(slotId, sim_op, sim_co, getMccMncFromSim(slotId, ""), "", "", "");

        return true;
    }

    private void setSIMPolicy(int slotId) {
        Log.i(TAG, "setSIMPolicy(" + slotId + ")");

        SODConfig.Sim simInfo = getSim(slotId).getSimInfo();

        if (updateSIMInfo(slotId)) {
            Log.i(TAG, "setSIMPolicy(" + slotId + ") - SIM Checking");

            String simOp = simInfo.getOperator();
            String simCo = simInfo.getCountry();

            Cursor cursor = getOperatorFromOperatorList(simOp, simCo);
            if (cursor == null) {
                setUnknownPolicy(slotId, simOp, simCo, SODConfig.ENABLER_TYPE_GLOBAL);
                return;
            }

            String op = getValueInCursor(
                    cursor, SmartConfigProviderInterface.OperatorList.OPERATOR, "");
            String co = getValueInCursor(
                    cursor, SmartConfigProviderInterface.OperatorList.COUNTRY, "");
            cursor.close();
            if (TextUtils.isEmpty(op) || TextUtils.isEmpty(co)) {
                setUnknownPolicy(slotId, simOp, simCo, SODConfig.ENABLER_TYPE_GLOBAL);
                return;
            }

            setOperatorInfo(slotId, op, co, false);
        }
        else {
            Log.i(TAG, "setSIMPolicy(" + slotId + ") - Abnormal Case");

            String simOp = simInfo.getOperator();
            String simCo = simInfo.getCountry();

            if (TextUtils.isEmpty(simOp)) {
                setUnknownPolicy(slotId, getSimOperator(slotId), getSimCountry(slotId),
                        SODConfig.ENABLER_TYPE_GLOBAL);
            } else {
                setUnknownPolicy(slotId, simOp, simCo, SODConfig.ENABLER_TYPE_GLOBAL);
            }
        }
    }

    private void setSIMPolicyForCA(int slotId) {
        Log.i(TAG, "setSIMPolicyForCA(" + slotId + ")");

        String sysSimOperator = getSysSimOperator(slotId);
        String sysSimOperatorSub = getSysSimOperatorSub(slotId);
        String operator = "";
        if (!TextUtils.isEmpty(sysSimOperatorSub)) {
            operator = sysSimOperatorSub;
        }
        else if (!TextUtils.isEmpty(sysSimOperator)) {
            operator = sysSimOperator;
        }
        else {
            operator = getSimOperator(slotId);
        }

        String country = getSimCountry(slotId);

        updateSIMInfo(slotId);
        if (isEquipInSim(slotId)) {
            SODConfig.Sim simInfo = getSim(slotId).getSimInfo();

            simInfo.setOperator(operator);
            simInfo.setCountry(country);
        }

        Cursor cursor = getOperatorFromOperatorList(operator, country);
        if (cursor == null) {
            setUnknownPolicy(slotId, operator, country, SODConfig.ENABLER_TYPE_CANADA);
        }
        else {
            setOperatorInfo(slotId, operator, country, false);
            cursor.close();
        }
    }

    private void setSIMPolicyForKROpen(int slotId) {
        Log.i(TAG, "setSIMPolicyForKROpen(" + slotId + ")");

        updateSIMInfo(slotId);

        String sysSimOperator = getSysSimOperator(slotId);

        if (ImsProperties.isUnknownOperator(sysSimOperator)) {
            // As default, KR enabler + LGU-SIM-MOVED-config
            setOperatorInfo(slotId, "LGU", "KR", false, true);
            /*
            {
                SODConfig.Sim simInfo = getSim(slotId).getSimInfo();

                if (TARGET_COUNTRY.equalsIgnoreCase(simInfo.getCountry())) {
                    setOperatorInfo(slotId,
                            simInfo.getOperator(), simInfo.getCountry(), false, false);
                } else {
                    setOperatorInfo(slotId, "LGU", "KR", false, true);
                }
            }
            */
        } else {
            setOperatorInfo(slotId, sysSimOperator, "KR", false, false);
        }
    }

    // Always final operator is set with target operator
    private void setTargetPolicy(int slotId) {
        Log.i(TAG, "setTargetPolicy(" + slotId + ")");

        // Just added sim info because some operators may use it
        updateSIMInfo(slotId);

        setOperatorInfo(slotId, getSimOperator(slotId), getSimCountry(slotId), false);
    }

    // If SIM is absent, and MccMnc is not existed in mccmnc_list table,
    // then final operator is set with target operator.
    // If Country of SIM is not equal to Country of Target,
    // then final operator is set with target operator by default.
    // If Country of SIM is equal to Country of Target,
    // then final operator is set with SIM operator and SIM_MOVED is set with true
    private void setTargetPolicyWithSimMoved(int slotId) {
        Log.i(TAG, "setTargetPolicyWithSimMoved(" + slotId + ")");

        String targetOp = getSimOperator(slotId);
        String targetCo = getSimCountry(slotId);
        if (updateSIMInfo(slotId)) {
            // Checking SIM_MOVED
            SODConfig.Sim simInfo = getSim(slotId).getSimInfo();
            if (targetCo.equalsIgnoreCase(simInfo.getCountry())) {
                setOperatorInfo(slotId, simInfo.getOperator(), simInfo.getCountry(), false);
            }
            else {
                setOperatorInfo(slotId, targetOp, targetCo, false);
            }
        }
        else {
            setOperatorInfo(slotId, targetOp, targetCo, false);
        }
    }

    private void setTargetOpenPolicyOnLaop(int slotId,
            String carrier, String def_op, String def_co) {
        setSIMPolicyForNaOpen(slotId, carrier, def_op, def_co);
    }

    private void setUnknownPolicy(int slotId, String op, String co, String enabler) {
        Log.i(TAG, "setUnknownPolicy(" + slotId + "/" + op + "/" + co + "/"
            + (isSIMBasedOn() ? "sim" : "target") + "/" + enabler + ")");

        String unknownOp = op;
        String targetCo = getSimCountry(slotId);

        // Checked SIM_MOVED only for KR in exception cases
        if ("KR".equalsIgnoreCase(targetCo)) {
            String mccmnc = getMccMncFromSim(slotId, "");
            if (!TextUtils.isEmpty(mccmnc)) {
                HashMap<String, String> mccmncList = new HashMap<String, String>();
                mccmncList.put("45002", "KT");
                mccmncList.put("45004", "KT");
                mccmncList.put("45008", "KT");
                mccmncList.put("45006", "LGU");
                mccmncList.put("45005", "SKT");
                mccmncList.put("45011", "SKT");

                String simOperator = mccmncList.get(mccmnc);
                if (TextUtils.isEmpty(simOperator)) {
                    setSIMInfo(slotId, mccmnc.substring(3),
                            mccmnc.substring(0, 3), mccmnc, "", "", "");
                }
                else {
                    setSIMInfo(slotId, simOperator, targetCo, mccmnc, "", "", "");
                    unknownOp = simOperator;
                }
            }

            Log.i(TAG, "Only For KR!!!, setUnknownPolicy(" + slotId + "/" + unknownOp + "/"
                    + co + "/" + (isSIMBasedOn() ? "sim" : "target") + "/" + enabler + ")");
        } else if ("AU".equalsIgnoreCase(targetCo)) {
            // AU-VOLTE-EMERGENCY-PATCH
            if (!"VDA".equals(unknownOp)
                    && !"OPT".equals(unknownOp)
                    && !"TEL".equals(unknownOp)
                    && !"OPEN".equals(unknownOp)) {
                Log.d(TAG, "ImsPolicy :: Unknown op-changed: " + unknownOp);
                unknownOp = "OPEN";
            }

            if (!"AU".equals(co)) {
                Log.d(TAG, "ImsPolicy :: Unknown co-changed: " + co);
                co = "AU";
            }
        }

        setOperatorInfo(slotId, unknownOp, co, true);
        setEnablerType(slotId, enabler);
    }

    private boolean updateOperatorBasedOn(int slotId, String op, String co) {
        mDevice.clearOperatorBasedOn();

        Cursor cOperator = getOperatorFromOperatorList(op, co);
        if (cOperator == null) {
            Log.w(TAG, "updateOperatorBasedOn :: operator for "
                + op + "/" + co + " is not existed...");
            mDevice.setOperatorBasedOn(SODConfig.TARGET_BASED);
            return false;
        }

        String strOperatorBasedOn = getValueInCursor(cOperator,
                SmartConfigProviderInterface.OperatorList.OPERATOR_BASED_ON, "");
        String strSupportSimMoved = getValueInCursor(cOperator,
                SmartConfigProviderInterface.OperatorList.SUPPORT_SIM_MOVED, "");
        mDevice.setSimMovedSupported("true".equalsIgnoreCase(strSupportSimMoved));
        cOperator.close();

        if (SODConfig.OPERATOR_BASED_ON_TARGET.equalsIgnoreCase(strOperatorBasedOn)) {
            mDevice.setOperatorBasedOn(SODConfig.TARGET_BASED);

            if ("true".equalsIgnoreCase(strSupportSimMoved)) {
                mDevice.setOperatorBasedOn(SODConfig.SUPPORT_SIM_MOVED);
            }
        }
        else if (SODConfig.OPERATOR_BASED_ON_SIM.equalsIgnoreCase(strOperatorBasedOn)) {
            mDevice.setOperatorBasedOn(SODConfig.SIM_BASED);

            if (mDevice.isSimMovedSupported()) {
                mDevice.setOperatorBasedOn(SODConfig.SUPPORT_SIM_MOVED);
            }

            String targetOp = getSimOperator(slotId);
            String targetCo = getSimCountry(slotId);

            if ("US".equalsIgnoreCase(targetCo)) {
                if ("NAO".equalsIgnoreCase(targetOp)) {
                    mDevice.setOperatorBasedOn(SODConfig.NA_OPEN);
                } else if ("OPEN".equalsIgnoreCase(targetOp)) {
                    // Android One/Go
                    mDevice.setOperatorBasedOn(SODConfig.NA_OPEN);
                }
            } else if ("KR".equalsIgnoreCase(targetCo)
                    && "OPEN".equalsIgnoreCase(targetOp)) {
                mDevice.setOperatorBasedOn(SODConfig.KR_OPEN);
            }
        }
        else {
            Log.w(TAG, "Unexpected OperatorBasedOn(" + strOperatorBasedOn + ")");
            mDevice.setOperatorBasedOn(SODConfig.TARGET_BASED);
            return false;
        }

        Log.i(TAG, "operatorBasedOn=0x" + Integer.toHexString(mDevice.getOperatorBasedOn())
            + ", simMovedSupported=" + mDevice.isSimMovedSupported());

        return true;
    }

    private boolean checkAndSetPreferredOperator(int slotId) {
        String preferredOperator = ImsPrivateProperties.Persistent.get(
                ImsPrivateProperties.Persistent.KEY_PREF_OPERATOR, "", slotId);
        String preferredCountry = ImsPrivateProperties.Persistent.get(
                ImsPrivateProperties.Persistent.KEY_PREF_COUNTRY, "", slotId);

        if (!TextUtils.isEmpty(preferredOperator) && !TextUtils.isEmpty(preferredCountry)) {
            setPreferenceOperatorPolicy(slotId, preferredOperator, preferredCountry);
            return true;
        }

        return false;
    }

    private boolean updateSIMInfo(int slotId) {
        Log.i(TAG, "updateSIMInfo(" + slotId + ")");
        boolean result = false;

        // If SIM_STATE is PIN/PUK LOCK, then set the latest operator information
        if (isSIMLocked(slotId)) {
            return getTheLatestOperatorInfo(slotId);
        }

        // Read MccMnc
        String mccmnc = getMccMncFromSim(slotId, "");
        if (TextUtils.isEmpty(mccmnc)) {
            setSIMInfo(slotId, "", "", "", "", "", "");
            return result;
        }

        // Search mccmnc in mccmnc_list
        String op = "";
        String co = "";
        Cursor cursors = getMccMncFromMccMncList(mccmnc);
        if (cursors == null) {
            co = mccmnc.substring(0, 3);
            op = mccmnc.substring(3);
            setSIMInfo(slotId, op, co, mccmnc, "", "", "");
            return result;
        }

        // Checking GID/SPN/IMSI & Setting Final Operator
        String gid = "";
        String spn = "";
        String imsi = "";

        String simGid = getGidFromSim(slotId, "");
        String simSpn = getSpnFromSim(slotId, "");
        String simImsi = getImsiFromSim(slotId, "");
        int iPosition = getBestMatchedOperator(cursors, simGid, simSpn, simImsi);
        if (iPosition != -1 && cursors.moveToPosition(iPosition)) {
            op = getValueInCursor(cursors, SmartConfigProviderInterface.MccMncList.OPERATOR, "");
            co = getValueInCursor(cursors, SmartConfigProviderInterface.MccMncList.COUNTRY, "");
            gid = getValueInCursor(cursors, SmartConfigProviderInterface.MccMncList.GID, "");
            spn = getValueInCursor(cursors, SmartConfigProviderInterface.MccMncList.SPN, "");
            imsi = getValueInCursor(cursors, SmartConfigProviderInterface.MccMncList.IMSI, "");
            result = true;
        }
        else {
            co = mccmnc.substring(0, 3);
            op = mccmnc.substring(3);
        }
        cursors.close();

        setSIMInfo(slotId, op, co, mccmnc, gid, spn, imsi);

        return result;
    }

    private String getSysSimOperator(int slotId) {
        SODConfig.SimProperties simProp = getSimProperties(slotId);
        return simProp.getOperator();
    }

    private String getSysSimOperatorSub(int slotId) {
        SODConfig.SimProperties simProp = getSimProperties(slotId);
        return simProp.getOperatorSub();
    }

    private String getSimOperator(int slotId) {
        SODConfig.SimProperties simProp = getSimProperties(slotId);
        return simProp.getOperator();
    }

    private String getSimCountry(int slotId) {
        SODConfig.SimProperties simProp = getSimProperties(slotId);
        return simProp.getCountry();
    }

    private void updateSysSimOperator(int slotId) {
        SODConfig.SimProperties simProp = getSimProperties(slotId);

        String oldOperator = simProp.getOperator();
        String oldOperatorSub = simProp.getOperatorSub();

        String sysSimOp = ImsProperties.getSysSimOperator(slotId);
        String sysSimOpSub = ImsProperties.getSysSimOperatorSub(slotId);
        String sysSimCo = ImsProperties.getSysSimCountry(slotId);

        simProp.setOperator(sysSimOp);
        simProp.setOperatorSub(sysSimOpSub);
        simProp.setCountry(sysSimCo);
    }

    // Internal API For Slot
    private Sim getSim(int slotId) {
        ServiceProfile profile = mServiceProfiles.get(slotId);
        return profile.getSim();
    }

    private Slot getSlot(int slotId) {
        ServiceProfile profile = mServiceProfiles.get(slotId);
        return profile.getSlot();
    }

    private SODConfig.SimProperties getSimProperties(int slotId) {
        ServiceProfile profile = mServiceProfiles.get(slotId);
        return profile.getSimProperties();
    }

    private void setTheLatestOperatorInfo(int slotId) {
        SODConfig.Operator opInfo = getSlot(slotId).getOperatorInfo();

        // Set the latest operator information
        SODConfig.setValueForCachedOperatorInfo(slotId,
                SODConfig.KEY_OPERATOR, opInfo.getOperator());
        SODConfig.setValueForCachedOperatorInfo(slotId,
                SODConfig.KEY_COUNTRY, opInfo.getCountry());

        String sim_operator = null;
        String sim_country = null;
        String sim_mccmnc = null;
        String sim_gid = null;
        String sim_spn = null;
        String sim_imsi = null;

        if (isEquipInSim(slotId)) {
            SODConfig.Sim simInfo = getSim(slotId).getSimInfo();
            sim_operator = simInfo.getOperator();
            sim_country = simInfo.getCountry();
            sim_mccmnc = simInfo.getMccMnc();
            sim_gid = simInfo.getGid();
            sim_spn = simInfo.getSpn();
            sim_imsi = simInfo.getImsi();
        }

        SODConfig.setValueForCachedOperatorInfo(slotId,
                SODConfig.KEY_SIM_OPERATOR, sim_operator);
        SODConfig.setValueForCachedOperatorInfo(slotId,
                SODConfig.KEY_SIM_COUNTRY, sim_country);
        SODConfig.setValueForCachedOperatorInfo(slotId,
                SODConfig.KEY_MCCMNC, sim_mccmnc);
        SODConfig.setValueForCachedOperatorInfo(slotId,
                SODConfig.KEY_GID, sim_gid);
        SODConfig.setValueForCachedOperatorInfo(slotId,
                SODConfig.KEY_SPN, sim_spn);
        SODConfig.setValueForCachedOperatorInfo(slotId,
                SODConfig.KEY_IMSI, sim_imsi);
    }

    private void setOperatorInfo(int slotId, String op, String co, boolean unknown) {
        setOperatorInfo(slotId, op, co, unknown, false);
    }

    private void setOperatorInfo(int slotId, String op, String co, boolean unknown, boolean simMoved) {
        Log.d(TAG, "setOperatorInfo(" + slotId + "/"
                + op + "/" + co + "/" + unknown + "/" + simMoved + ")");

        Slot slot = getSlot(slotId);
        SODConfig.Operator opInfo = slot.getOperatorInfo();

        if (op.equalsIgnoreCase(opInfo.getOperator()) && co.equalsIgnoreCase(opInfo.getCountry())) {
            Log.w(TAG, "No Changed Operator !!!");
            if (isEquipInSim(slotId)) {
                SODConfig.Sim simInfo = getSim(slotId).getSimInfo();

                opInfo.setSubId(simInfo.getSubId());
                opInfo.setMccMnc(simInfo.getMccMnc());
            }

            if (opInfo.isSimMoved() != simMoved) {
                opInfo.setSimMoved(simMoved);
                slot.mChanged = true;
                slot.mDelivered = false;
            }

            return;
        }

        opInfo.setAvailable(true);
        opInfo.setOperator(op);
        opInfo.setCountry(co);
        opInfo.setUnknownOperator(unknown);
        opInfo.setSimMoved(simMoved);

        slot.mChanged = true;
        slot.mDelivered = false;

        String enabler = "";
        String region = "";
        String groupId = "";
        String category = "";
        String configPerModel = "";

        Cursor cursor = getOperatorFromOperatorList(op, co);
        if (cursor != null) {
            enabler = getValueInCursor(
                    cursor, SmartConfigProviderInterface.OperatorList.ENABLER_TYPE, "");
            region = getValueInCursor(
                    cursor, SmartConfigProviderInterface.OperatorList.REGION, "");
            groupId = getValueInCursor(
                    cursor, SmartConfigProviderInterface.OperatorList.GROUP_ID, "");
            category = getValueInCursor(
                    cursor, SmartConfigProviderInterface.OperatorList.CATEGORY, "");
            configPerModel = getValueInCursor(
                    cursor, SmartConfigProviderInterface.OperatorList.CONFIG_PER_MODEL, "");
            cursor.close();
        }

        if (TextUtils.isEmpty(enabler)) {
            enabler = SODConfig.ENABLER_TYPE_GLOBAL;
        }

        opInfo.setEnablerType(enabler);
        opInfo.setRegion(region);
        opInfo.setGroupId(groupId);
        opInfo.setCategory(category);
        opInfo.setConfigPerModel("true".equalsIgnoreCase(configPerModel));

        if (isEquipInSim(slotId)) {
            SODConfig.Sim simInfo = getSim(slotId).getSimInfo();

            opInfo.setSubId(simInfo.getSubId());
            opInfo.setMccMnc(simInfo.getMccMnc());
        }

        if (isSIMLocked(slotId) != true) {
            setTheLatestOperatorInfo(slotId);
        }
    }

    private void setSIMInfo(int slotId, String op, String co,
            String mccmnc, String gid, String spn, String imsi) {
        Log.i(TAG, "setSIMInfo(" + slotId + ")");

        SODConfig.Sim simInfo = getSim(slotId).getSimInfo();

        simInfo.setSubId(MSimUtils.getSubId(slotId));
        simInfo.setOperator(op);
        simInfo.setCountry(co);
        simInfo.setMccMnc(mccmnc);
        simInfo.setGid(gid);
        simInfo.setSpn(spn);
        simInfo.setImsi(imsi);

        Log.d(TAG, "setSIMInfo :: slotId/operator/country/mccmnc/gid/spn/imsi = "
            + slotId + "/" + op + "/" + co + "/" + mccmnc + "/" + gid + "/" + spn + "/"
            + Log.pii(imsi));
    }

    private void setSimInfoFromCarrierCode(int slotId, boolean simLocked) {
        SimCarrierId id = CarrierCodeLoader.getCarrierIdFromSim(slotId, simLocked);
        CarrierCodeLoader ccl = CarrierCodeLoader.getInstance();
        CarrierCode cc = ccl.fetchCarrierCode(id, slotId);

        Log.d(TAG, "setSimInfoFromCarrierCode: " + ((cc != null) ? cc.toString() : "(null)"));

        if (cc == null) {
            return;
        }

        SODConfig.Sim simInfo = getSim(slotId).getSimInfo();

        simInfo.setMccMnc(cc.getMcc() + cc.getMnc());

        String mvnoData = cc.getMvnoMatchData();

        if (!TextUtils.isEmpty(mvnoData)) {
            switch (cc.getMvnoType()) {
            case CarrierCode.KEY_MVNO_TYPE_GID:
                simInfo.setGid(mvnoData);
                simInfo.setSpn("");
                simInfo.setImsi("");
                break;
            case CarrierCode.KEY_MVNO_TYPE_SPN:
                simInfo.setGid("");
                simInfo.setSpn(mvnoData);
                simInfo.setImsi("");
                break;
            case CarrierCode.KEY_MVNO_TYPE_IMSI:
                simInfo.setGid("");
                simInfo.setSpn("");
                simInfo.setImsi(mvnoData);
                break;
            default:
                // no-op
                break;
            }
        } else {
            simInfo.setGid("");
            simInfo.setSpn("");
            simInfo.setImsi("");
        }
    }

    // Internal APIs for reading SIM Info
    public String getMccMncFromSim(int slotId, String defaultValue) {
        if (!isSlotIdValid(slotId)) {
            return defaultValue;
        }

        TelephonyManager tm = null;

        if (MSimUtils.isMultiSimEnabled()) {
            int subId = MSimUtils.getSubId(slotId);
            if (MSimUtils.isValidSubId(subId)) {
                tm = AppContext.getTelephonyManager(subId);
            }
        } else {
            tm = AppContext.getTelephonyManager();
        }

        String mccmnc = (tm != null) ? tm.getSimOperator() : defaultValue;

        Log.i(TAG, "MccMnc/SlotId = " + mccmnc + "/" + slotId);

        return mccmnc;
    }

    private String getGidFromSim(int slotId, String defaultValue) {
        if (!isSlotIdValid(slotId)) {
            return defaultValue;
        }

        TelephonyManager tm = null;
        int subId = MSimUtils.getSubId(slotId);
        if (MSimUtils.isValidSubId(subId)) {
            tm = AppContext.getTelephonyManager(subId);
        }

        if (tm == null) {
            return defaultValue;
        }

        String gid = tm.getGroupIdLevel1();
        return (TextUtils.isEmpty(gid) ? defaultValue : gid.toUpperCase(Locale.ROOT));
    }

    private String getSpnFromSim(int slotId, String defaultValue) {
        if (!isSlotIdValid(slotId)) {
            return defaultValue;
        }

        TelephonyManager tm = null;

        if (MSimUtils.isMultiSimEnabled()) {
            int subId = MSimUtils.getSubId(slotId);
            if (MSimUtils.isValidSubId(subId)) {
                tm = AppContext.getTelephonyManager(subId);
            }
        } else {
            tm = AppContext.getTelephonyManager();
        }

        String spn = (tm != null) ? tm.getSimOperatorName() : null;
        return (TextUtils.isEmpty(spn) ? defaultValue : spn.toUpperCase(Locale.ROOT));
    }

    private String getImsiFromSim(int slotId, String defaultValue) {
        if (!isSlotIdValid(slotId)) {
            return defaultValue;
        }

        TelephonyManager tm = null;

        if (MSimUtils.isMultiSimEnabled()) {
            int subId = MSimUtils.getSubId(slotId);
            if (MSimUtils.isValidSubId(subId)) {
                tm = AppContext.getTelephonyManager(subId);
            }
        } else {
            tm = AppContext.getTelephonyManager();
        }

        return (tm != null) ? tm.getSubscriberId() : defaultValue;
    }

    // Internal APIs for DB operation
    private Cursor getSmartModifiedTime() {
        return getCursor(SmartConfigProviderInterface.SmartModifiedTime.CONTENT_URI, null);
    }

    private Cursor getOperatorFromOperatorList(String op, String co) {
        String selection = "operator='" + op + "' AND country='" + co + "'";
        return getCursor(SmartConfigProviderInterface.OperatorList.CONTENT_URI, selection);
    }

    private Cursor getMccMncFromMccMncList(String mccmnc) {
        String selection = "mccmnc='" + mccmnc + "'";
        return getCursor(SmartConfigProviderInterface.MccMncList.CONTENT_URI, selection);
    }

    private Cursor getServiceAcceptanceList(String op, String co) {
        String selection = SmartConfigProviderInterface.ServiceAcceptanceList.OPERATOR
                        + "='" + op + "' AND "
                        + SmartConfigProviderInterface.ServiceAcceptanceList.COUNTRY
                        + "='" + co + "'";
        return getCursor(SmartConfigProviderInterface.ServiceAcceptanceList.CONTENT_URI, selection);
    }

    private Cursor getServiceEnableListByToTc(String totc) {
        String selection = SmartConfigProviderInterface.ServiceEnableListByToTc.TOTC
                        + "='" + totc + "'";
        return getCursor(SmartConfigProviderInterface.ServiceEnableListByToTc.CONTENT_URI, selection);
    }

    private Cursor getCarrierFromNaoCarrierList(String carrier) {
        String selection = SmartConfigProviderInterface.NaoCarrierList.CARRIER
                        + "='" + carrier + "'";
        return getCursor(SmartConfigProviderInterface.NaoCarrierList.CONTENT_URI, selection);
    }

    private Cursor getCursor(Uri uri, String selection) {
        Cursor cursor = null;
        try {
            cursor = mContext.getContentResolver().query(uri, null, selection, null, null);
        }
        catch (Throwable t) {
            Log.e(TAG, t.toString());
            t.printStackTrace();
        }

        if (cursor != null) {
            if (cursor.moveToFirst()) {
                return cursor;
            }
            cursor.close();
        }

        return null;
    }

    private String getValueInCursor(Cursor cursor, String column, String defaultValue) {
        if (cursor != null) {
            try {
                int index = cursor.getColumnIndex(column);
                String value = (index < 0) ? defaultValue : cursor.getString(index);

                return value;
            }
            catch (Throwable t) {
                Log.e(TAG, t.toString());
                t.printStackTrace();
            }
        }

        return defaultValue;
    }

    private boolean isEmergencyCallAvailableOnSimAbsentForTracfone(int slotId,
            ImsUtils.ServiceCaps serviceCaps) {
        if (!SODConfig.equalsOperator("TRF", getSimOperator(slotId))) {
            return false;
        }

        if (!isSIMAbsent(slotId)) {
            return false;
        }

        // OOB & SIM ABSENT & VoLTE only enabled as default
        return serviceCaps.isVoLteEnabled()
                && !serviceCaps.isVtEnabled()
                && !serviceCaps.isWfcEnabled();
    }

    private static String getDefaultOperatorForNaOpen() {
        return "OPEN";
    }

    private static boolean isCommonTestSim(String mccmnc) {
        return "00101".equals(mccmnc) || "001001".equals(mccmnc)
                || "001010".equals(mccmnc) || "45000".equals(mccmnc);
    }

    private static boolean isCarrierCodeRequiredOnSimLocked() {
        return SODConfig.equalsCountry(ImsProperties.TARGET_COUNTRY, "CA");
    }

    private static boolean isSmsOverImsSupported(String op, String co) {
        if ("KR".equals(co)) {
            if ("SKT".equals(op)) {
                return false;
            }
        }

        return true;
    }
}
