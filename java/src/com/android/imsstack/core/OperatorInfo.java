/**
 * Configuration
 * Role
 *         Provide software information and model information via static access.
 *         Operator & Country selection algorithm can be added in this package and class.
 */

package com.android.imsstack.core;

import android.text.TextUtils;

import com.android.imsstack.core.config.FeatureConfig;
import com.android.imsstack.core.config.FeatureTable;
import com.android.imsstack.system.SystemConfig;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.ImsUtils;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.SODConfig;

import java.util.ArrayList;
import java.util.List;

/**
 * This class provide software information and model information via static access.
 * And operator & country selection algorithm can be added in this package and class.
 */

public class OperatorInfo {
    private static final String TAG = "ImsStack_OperatorInfo";

    static class Profile {
        private final SODConfig.Operator mOperator;
        private final SODConfig.Sim mSim;
        private String mSysSimOperator = "";
        private String mConfigPostfix = "";

        Profile(int slotId) {
            mOperator = new SODConfig.Operator(slotId);
            mSim = new SODConfig.Sim();
        }

        SODConfig.Operator getOperator() {
            return mOperator;
        }

        SODConfig.Sim getSim() {
            return mSim;
        }

        String getConfigPostfix() {
            return mConfigPostfix;
        }

        String getSysSimOperator() {
            return mSysSimOperator;
        }

        void setConfigPostfix(String postfix) {
            mConfigPostfix = postfix;
        }

        void setSysSimOperator(String operator) {
            mSysSimOperator = operator;
        }
    };

    private static List<Profile> sProfiles;
    private static SODConfig.Device sDevice;

    public static boolean equalsOperator(String op1, String op2) {
        return SODConfig.equalsOperator(op1, op2);
    }

    public static boolean equalsCountry(String co1, String co2) {
        return SODConfig.equalsCountry(co1, co2);
    }

    public static boolean equalsOperatorCountry(String op1, String co1, String op2, String co2) {
        return SODConfig.equalsOperatorCountry(op1, co1, op2, co2);
    }

    private static boolean isOperatorBasedOn(int flag) {
        return sDevice.isOperatorBasedOn(flag);
    }

    private static boolean isSimBasedOn() {
        return isOperatorBasedOn(SODConfig.SIM_BASED);
    }

    private static boolean isTargetBasedOn() {
        return isOperatorBasedOn(SODConfig.TARGET_BASED);
    }

    public static boolean isNaOpen() {
        return (isSimBasedOn() && isOperatorBasedOn(SODConfig.NA_OPEN));
    }

    private static boolean isSmsOnly() {
        return isOperatorBasedOn(SODConfig.SMS_ONLY);
    }

    public static boolean isKrOpen() {
        return isOperatorBasedOn(SODConfig.KR_OPEN);
    }

    private static boolean isSupportSimMoved() {
        return isOperatorBasedOn(SODConfig.SUPPORT_SIM_MOVED);
    }

    public static boolean isTargetOpenOnLaop() {
        return (isTargetBasedOn() && isOperatorBasedOn(SODConfig.TARGET_OPEN));
    }

    public static void showDevice() {
        Log.w(TAG, "ImsPolicy: [ DEV: operatorBasedOn=0x"
                + Integer.toHexString(sDevice.getOperatorBasedOn())
                + ", simMovedSupported=" + sDevice.isSimMovedSupported() + " ]");
    }

    public static void showOperator(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        Profile profile = getProfile(slotId);
        SODConfig.Operator opInfo = profile.getOperator();
        SODConfig.Sim simInfo = profile.getSim();

    Log.w(TAG, "ImsPolicy: " + opInfo.toString() + ", configPostfix=" + profile.getConfigPostfix());
        Log.w(TAG, "ImsPolicy: " + simInfo.toString());
    }

    public static String getConfigPostfix(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        if (!isSlotAvailable(slotId)) {
            return "";
        }

        Profile profile = getProfile(slotId);
        return profile.getConfigPostfix();
    }

    public static String getEnablerType(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        if (!isSlotAvailable(slotId)) {
            return "";
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.getEnablerType();
    }

    public static String getCountry(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        if (!isSlotAvailable(slotId)) {
            return "";
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.getCountry();
    }

    public static String getOperator(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        if (!isSlotAvailable(slotId)) {
            return "";
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.getOperator();
    }

    public static String getRegion(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        if (!isSlotAvailable(slotId)) {
            return "";
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.getRegion();
    }

    public static String getGroupId(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        if (!isSlotAvailable(slotId)) {
            return "";
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.getGroupId();
    }

    public static String getSimCountry(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        if (!isSlotAvailable(slotId)) {
            return "";
        }

        SODConfig.Sim simInfo = getSimInfo(slotId);
        return simInfo.getCountry();
    }

    public static String getSimGid(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        if (!isSlotAvailable(slotId)) {
            return "";
        }

        SODConfig.Sim simInfo = getSimInfo(slotId);
        return simInfo.getGid();
    }

    public static String getSimImsi(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        if (!isSlotAvailable(slotId)) {
            return "";
        }

        SODConfig.Sim simInfo = getSimInfo(slotId);
        return simInfo.getImsi();
    }

    public static String getSimMccMnc(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        if (!isSlotAvailable(slotId)) {
            return "";
        }

        SODConfig.Sim simInfo = getSimInfo(slotId);
        return simInfo.getMccMnc();
    }

    public static String getSimSpn(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        if (!isSlotAvailable(slotId)) {
            return "";
        }

        SODConfig.Sim simInfo = getSimInfo(slotId);
        return simInfo.getSpn();
    }

    public static String getSimOperator(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return "";
        }

        if (!isSlotAvailable(slotId)) {
            return "";
        }

        SODConfig.Sim simInfo = getSimInfo(slotId);
        return simInfo.getOperator();
    }

    public static int getSimState(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return SODConfig.SIM_STATE_NONE;
        }

        if (!isSlotAvailable(slotId)) {
            return SODConfig.SIM_STATE_NONE;
        }

        SODConfig.Sim simInfo = getSimInfo(slotId);
        return simInfo.getState();
    }

    public static int getSubId(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return MSimUtils.INVALID_SUB_ID;
        }

        if (!isSlotAvailable(slotId)) {
            return MSimUtils.INVALID_SUB_ID;
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.getSubId();
    }

    public static int getSupportServices(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return SODConfig.SUPPORT_NONE;
        }

        if (!isSlotAvailable(slotId)) {
            return SODConfig.SUPPORT_NONE;
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.getServices();
    }

    public static boolean isConfigPerModel(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        if (!isSlotAvailable(slotId)) {
            return false;
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.isConfigPerModel();
    }

    public static boolean isSimMoved(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return false;
        }

        if (!isSlotAvailable(slotId)) {
            return false;
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.isSimMoved();
    }

    private static boolean isSlotAvailable(int slotId) {
        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.isAvailable();
    }

    public static boolean isEnablerTypeCanada(int slotId) {
        return SODConfig.isEnablerTypeCanada(getEnablerType(slotId));
    }

    public static boolean isEnablerTypeGlobal(int slotId) {
        return SODConfig.isEnablerTypeGlobal(getEnablerType(slotId));
    }

    public static boolean isEnablerTypeForNonOperator(int slotId) {
        return SODConfig.isEnablerTypeForNonOperator(getEnablerType(slotId));
    }

    public static boolean isGroupTMUS(int slotId, String operator, String country) {
        String groupId = getGroupId(slotId);

        if ("TMUS".equals(groupId)) {
            return true;
        }

        if (TextUtils.isEmpty(operator)) {
            operator = getOperator(slotId);
        }

        if (TextUtils.isEmpty(country)) {
            country = getCountry(slotId);
        }

        return equalsOperatorCountry("TMO", "US", operator, country)
                || equalsOperator("MPCS", operator)
                || equalsOperatorCountry("TRF", "US", operator, country);
    }

    public static boolean isUnknownOperator(int slotId) {
        if (!isSlotIdValid(slotId)) {
            return true;
        }

        if (!isSlotAvailable(slotId)) {
            return true;
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.isUnknownOperator();
    }

    public static boolean isSupportVolte(int slotId) {
        Log.i(TAG, "isSupportVolte");

        if (!isSlotIdValid(slotId)) {
            return false;
        }

        if (!isSlotAvailable(slotId)) {
            return false;
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.isServiceOn(SODConfig.SUPPORT_VOLTE);
    }

    public static boolean isSupportVoLteEmergency(int slotId) {
        Log.i(TAG, "isSupportVoLteEmergency");

        if (!isSlotIdValid(slotId)) {
            return false;
        }

        if (!isSlotAvailable(slotId)) {
            return false;
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.isServiceOn(SODConfig.SUPPORT_VOLTE_EMERGENCY);
    }

    public static boolean isSupportVowifi(int slotId) {
        Log.i(TAG, "isSupportVowifi");

        if (!isSlotIdValid(slotId)) {
            return false;
        }

        if (!isSlotAvailable(slotId)) {
            return false;
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.isServiceOn(SODConfig.SUPPORT_VOWIFI);
    }

    public static boolean isSupportVt(int slotId) {
        Log.i(TAG, "isSupportVt");

        if (!isSlotIdValid(slotId)) {
            return false;
        }

        if (!isSlotAvailable(slotId)) {
            return false;
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.isServiceOn(SODConfig.SUPPORT_VT);
    }

    public static boolean isSlotIdValid(int slotId) {
        if (slotId < MSimUtils.DEFAULT_SLOT_ID || slotId >= MSimUtils.getMaxSimSlot()) {
            StackTraceElement[] stes = (new Throwable()).getStackTrace();
            Log.d(TAG, "isSlotIdValid :: slotId=" + slotId + ", caller=" + stes[1].getMethodName()
                    + "(" + stes[1].getFileName() + ":" + stes[1].getLineNumber() + ")");
            return false;
        }

        return true;
    }

    public static boolean isVoLTEServiceAvailable(int slotId) {
        Log.i(TAG, "isVoLTEServiceAvailable");

        if (!isSlotIdValid(slotId)) {
            return false;
        }

        if (!isSlotAvailable(slotId)) {
            return false;
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        return opInfo.isServiceOn(SODConfig.SUPPORT_VOLTE)
                || opInfo.isServiceOn(SODConfig.SUPPORT_VT)
                || opInfo.isServiceOn(SODConfig.SUPPORT_VOWIFI);
    }

    public static void updateOperatorInfo(int slotId,
            SODConfig.Operator operator, SODConfig.Sim sim, SODConfig.Device device,
            SODConfig.SimProperties simProp) {
        Log.w(TAG, "updateOperatorInfo :: slotId=" + slotId);

        if (!isSlotIdValid(slotId)) {
            return;
        }

        Profile profile = getProfile(slotId);
        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        SODConfig.Sim simInfo = getSimInfo(slotId);

        opInfo.copyFrom(operator);
        simInfo.copyFrom(sim);

        sDevice.copyFrom(device);

        profile.setSysSimOperator(simProp.getOperator());

        // Additional Info
        if (isSupportSimMoved()) {
            if (isKrOpen()) {
                if (ImsProperties.isUnknownOperator(getSysSimOperator(slotId))) {
                    // As default, KR enabler + LGU-SIM-MOVED-config
                    opInfo.setSimMoved(true);
                } else {
                    opInfo.setSimMoved(false);
                }
            } else {
                if (!ImsProperties.IMS_TARGET_OPERATOR.equalsIgnoreCase(opInfo.getOperator())
                        || !ImsProperties.TARGET_COUNTRY.equalsIgnoreCase(opInfo.getCountry())) {
                    opInfo.setSimMoved(true);
                } else {
                    opInfo.setSimMoved(false);
                }
            }
        }
        else if (opInfo.isInboundRoaming()) {
            opInfo.setSimMoved(true);
        }

        if (isNaOpen()) {
            if ("TRF_VZW".equalsIgnoreCase(getSysSimOperator(slotId))) {
                profile.setConfigPostfix("VOLTEONLY");
            } else if ("ATT".equals(simInfo.getOperator())
                    && !ImsUtils.isWfcEnabledByPlatform(AppContext.get(), slotId)) {
                profile.setConfigPostfix("NAO_VOLTEONLY");
            } else {
                profile.setConfigPostfix("NAO");
            }
        }

        if (ImsProperties.TARGET_OPERATOR.equals("CRK")) {
            profile.setConfigPostfix("CRK");
        }

        if (ImsProperties.TARGET_OPERATOR.equals("DISH")) {
            profile.setConfigPostfix("DISH");
        }

        if (isTargetOpenOnLaop()) {
            if (ImsProperties.TARGET_OPERATOR.equals("TRF")
                    && ImsProperties.TARGET_COUNTRY.equals("US")) {
                if ("ATT".equals(opInfo.getOperator()) || "TMO".equals(opInfo.getOperator())) {
                    profile.setConfigPostfix("TRF");
                }
            }
        }

        // Prepaid device
        if ("BELL".equals(opInfo.getOperator())) {
            if (!ImsUtils.isVtEnabledByPlatform(AppContext.get(), slotId)
                    && !ImsUtils.isWfcEnabledByPlatform(AppContext.get(), slotId)) {
                profile.setConfigPostfix("VOLTEONLY");
            }
        }
    }

    public static void updateServiceInfo(int slotId,
            SODConfig.Operator operator, SODConfig.Sim sim, SODConfig.Device device) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        int supportedServices = operator.getServices();

    Log.w(TAG, "updateSimInfo :: slotId=" + slotId + ", supportedServices=" + supportedServices);

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        SODConfig.Sim simInfo = getSimInfo(slotId);

        // slot information
        opInfo.setSubId(sim.getSubId());
        opInfo.setMccMnc(sim.getMccMnc());
        opInfo.setServices(supportedServices);

        // sim information
        simInfo.copyFrom(sim);
    }

    public static void updateSimInfo(int slotId, SODConfig.Sim sim) {
        if (!isSlotIdValid(slotId)) {
            return;
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        SODConfig.Sim simInfo = getSimInfo(slotId);

        // slot information
        opInfo.setSubId(sim.getSubId());
        opInfo.setMccMnc(sim.getMccMnc());

        // sim information
        simInfo.copyFrom(sim);

        Log.w(TAG, "ImsPolicy: updateSimInfo=" + slotId + ", " + simInfo.toString());
    }

    public static void setDDSChanged(int slotId, boolean ddsChanged) {
        Log.i(TAG, "setDDSChanged(" + slotId + "/" + ddsChanged + ")");

        if (!isSlotIdValid(slotId)) {
            return;
        }

        SODConfig.Operator opInfo = getOperatorInfo(slotId);
        opInfo.setActive(ddsChanged);
    }

    public static String getSysSimOperator(int slotId) {
        Profile profile = getProfile(slotId);
        return profile.getSysSimOperator();
    }

    /** Interworking with SystemConfig -- starts */
    public static void setSystemConfigForBootup() {
        setSystemConfig(-1, SystemConfig.EVENT_ON_BOOT, false, false);
    }

    public static void setSystemConfigForAllConfigurationChanged(
            int slotId, boolean simRemoved) {
        setSystemConfig(slotId, SystemConfig.EVENT_ALL_CONFIGURATION_CHANGED, simRemoved, false);
    }

    public static void setSystemConfigForServiceFeature(int slotId) {
        setSystemConfig(slotId, SystemConfig.EVENT_FEATURE_CHANGED, false, false);
    }

    public static void setSystemConfigForDDSChanged(int slotId, boolean ddsChanged) {
        setSystemConfig(slotId, SystemConfig.EVENT_DDS_CHANGED, false, ddsChanged);
    }

    private static void setSystemConfig(int slotId, int event, boolean simRemoved, boolean dds)
    {
        int simCount = (slotId < 0) ? MSimUtils.getMaxSimSlot() : 1;
        SystemConfig[] sc = new SystemConfig[simCount];

        for (int i = 0; i < simCount; ++i) {
            int currentSlotId = (slotId < 0) ? i : slotId;

            if (!isSlotIdValid(currentSlotId)) {
                continue;
            }

            SODConfig.Operator opInfo = getOperatorInfo(currentSlotId);
            int extraInfo = 0;

            if (opInfo.isSimMoved()) {
                extraInfo |= SystemConfig.EXTRA_INFO_SIM_MOBILITY;
                if ("KR".equalsIgnoreCase(opInfo.getCountry())) {
                    extraInfo |= SystemConfig.EXTRA_INFO_KR_ENABLER;
                }
            } else {
                // KR enabler for test purpose
                if ("KR".equalsIgnoreCase(opInfo.getCountry())) {
                    boolean krEnablerEnabled = ImsPrivateProperties.Persistent.getBoolean(
                            ImsPrivateProperties.Persistent.KEY_PREF_KR_ENABLER, currentSlotId);

                    if (krEnablerEnabled) {
                        extraInfo |= SystemConfig.EXTRA_INFO_KR_ENABLER;
                    }
                }
            }

            extraInfo |= (simRemoved) ? SystemConfig.EXTRA_INFO_NO_UICC :
                    (!MSimUtils.hasIccCard(currentSlotId) ? SystemConfig.EXTRA_INFO_NO_UICC : 0);

            if (MSimUtils.isMultiSimEnabled()) {
                if (event == SystemConfig.EVENT_DDS_CHANGED) {
                    extraInfo |= (dds) ? SystemConfig.EXTRA_INFO_DDS : 0;
                }
                else {
                    extraInfo |= (opInfo.isActive()) ? SystemConfig.EXTRA_INFO_DDS : 0;
                }
            }

            if (CapabilityConfigs.isVoNrEnabled(currentSlotId)) {
                int nrUeCapability = ImsUtils.getNrUeCapability(currentSlotId);

                if (ImsUtils.has(nrUeCapability, ImsUtils.NR_UE_CAPABILITY_I_SA)) {
                    if (ImsUtils.has(nrUeCapability, ImsUtils.NR_UE_CAPABILITY_I_VONR)) {
                        extraInfo |= SystemConfig.EXTRA_INFO_NR_UE_CAPABILITY_VONR;
                    }
                } else {
                    extraInfo |= SystemConfig.EXTRA_INFO_NR_NSA_MODE;
                }
            } else {
                extraInfo |= SystemConfig.EXTRA_INFO_NR_NSA_MODE;
            }

            int features = 0;
            List<FeatureTable.Feature> featureList = FeatureTable.getFeatures();

            for (FeatureTable.Feature feature : featureList) {
                features = setOrClearFeature(currentSlotId,
                        feature.getFeature(), feature.getFeatureMask(), features);
            }

            int serviceFeatures = 0;
            if (getSupportServices(currentSlotId) > 0) {
                List<FeatureTable.Feature> serviceFeatureList = FeatureTable.getServiceFeatures();

                for (FeatureTable.Feature feature : serviceFeatureList) {
                    serviceFeatures = setOrClearFeature(currentSlotId,
                            feature.getFeature(), feature.getFeatureMask(), serviceFeatures);
                }

                // VOLTE_EMERGENCY_CALLING
                serviceFeatures = adjustServiceFeatures(currentSlotId, serviceFeatures);
            }

            sc[i] = new SystemConfig(currentSlotId,
                    opInfo.getOperator(), opInfo.getCountry(), opInfo.getEnablerType(),
                    extraInfo, features, serviceFeatures);
        }

        SystemConfig.setConfiguration(event, sc);
    }

    private static int setOrClearFeature(int slotId, String feature, int featureMask, int features) {
        if (FeatureConfig.isEnabled(slotId, feature)) {
            features |= featureMask;
        } else {
            features &= (~featureMask);
        }

        return features;
    }

    private static int adjustServiceFeatures(int slotId, int serviceFeatures) {
        if (isSupportVoLteEmergency(slotId)) {
            Log.i(TAG, "SystemConfig :: VoLTE enabled for e-call");
            serviceFeatures |= SystemConfig.FEATURE_S_VOLTE;

            if (!"KR".equals(ImsProperties.TARGET_COUNTRY)) {
                serviceFeatures |= SystemConfig.FEATURE_S_VOLTE_EMERGENCY;
            }

            serviceFeatures &= ~(SystemConfig.FEATURE_S_VT);
            serviceFeatures &= ~(SystemConfig.FEATURE_S_VOWIFI);
        }
        /* FIXME: check if the below logic is required or not
        else if (ImsUtils.isEmergencyCallEnabledOnServiceRestricted()
                && (getSupportServices(slotId) == SODConfig.SUPPORT_VOLTE)) {
            if (ImsUtils.isDeviceEncryptionModeEnabledAsFDE()) {
                Log.i(TAG, "SystemConfig :: VoLTE only for e-call");
                serviceFeatures |= SystemConfig.FEATURE_S_VOLTE;
                serviceFeatures &= ~(SystemConfig.FEATURE_S_VT);
                serviceFeatures &= ~(SystemConfig.FEATURE_S_VOWIFI);
            }
        }*/

        return serviceFeatures;
    }
    /** Interworking with SystemConfig -- ends */

    // Internal API
    private static Profile getProfile(int slotId) {
        return sProfiles.get(slotId);
    }

    private static SODConfig.Operator getOperatorInfo(int slotId) {
        Profile profile = getProfile(slotId);
        return profile.getOperator();
    }

    private static SODConfig.Sim getSimInfo(int slotId) {
        Profile profile = getProfile(slotId);
        return profile.getSim();
    }

    static {
        sProfiles = new ArrayList<Profile>(MSimUtils.getMaxSimSlot());

        for (int i = 0; i < MSimUtils.getMaxSimSlot(); i++) {
            sProfiles.add(new Profile(i));
        }

        sDevice = new SODConfig.Device();
    }
}
