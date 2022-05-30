package com.android.imsstack.core.config;

import android.telephony.ServiceState;

import com.android.imsstack.core.ImsGlobal;
import com.android.imsstack.core.OperatorInfo;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.DBUtils;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsProperties;

import java.util.concurrent.ConcurrentHashMap;

/**
 * Configurations(static or runtime) for an emergency calling.
 */
public class ECallConfigUtil {

    private static ECallConfigUtil sECallConfigUtil = null;
    private static ConcurrentHashMap<Integer, IECallConfig> ECallConfigs
                                            = new ConcurrentHashMap<Integer, IECallConfig>();

    private ECallConfigUtil() {
    }

    public static synchronized IECallConfig peekConfig(int slotID) {
        ImsLog.d("SlotID[" + slotID + "]");

        return ECallConfigs.get(slotID);
    }

    public static synchronized IECallConfig getInstance(int slotID) {
        ImsLog.d("SlotID[" + slotID + "]");

        if (sECallConfigUtil == null) {
            sECallConfigUtil = new ECallConfigUtil();
        }

        if(!ECallConfigs.containsKey(slotID)) {
            ImsLog.d("start");
            ECallConfigs.put(slotID, new ECallConfig(slotID));
        }

        return ECallConfigs.get(slotID);
    }

    public static synchronized void cleanup(int slotID) {
        ImsLog.d("SlotID[" + slotID + "]");

        if (sECallConfigUtil == null) {
            ImsLog.d("ECallConfigUtil is not started");
            return;
        }

        if (ECallConfigs.containsKey(slotID)) {
            IECallConfig eCallConfig = ECallConfigs.get(slotID);
            eCallConfig = null;
            ECallConfigs.remove(slotID);
        } else {
            ImsLog.d("already stoped");
        }
    }

}

class ECallConfig implements IECallConfig {
    private int mSlotID = -1;
    private boolean mSupportOverLTE = false;
    private boolean mSupportOverWiFi = false;
    private boolean mControlByVolteSetting = false;
    private boolean mControlByImsReg = false;
    private boolean mRequireNormalEnd = false;
    private boolean mBlockSupportOverLTE = false;

    ECallConfig(int slotID) {
        mSlotID = slotID;
        loadConfiguration();
        if (ImsProperties.TARGET_COUNTRY.equals("AU")) {
            mSupportOverLTE = true;
        }
    }

    // Interface implementation methods --------------------------
    @Override
    public int getEmergencyCallCapability() {
        int nECallCapa = ECALL_RAT_NONE;

        if (mSupportOverLTE && isImsEmergencyCallRequiredInLimitedServiceStatus()
                && !mBlockSupportOverLTE) {
            nECallCapa |= ECALL_RAT_LTE;
        }

        if (mSupportOverWiFi) {
            nECallCapa |= ECALL_RAT_WIFI;
        }

        return nECallCapa;
    }

    @Override
    public boolean isImsCallEndRequiredForImsEmergencyCall() {
        return mRequireNormalEnd;
    }


    @Override
    public boolean isImsEmergencyCallControlledByVoLteReg() {
        if (ImsGlobal.isCountry(mSlotID, "KR")) {
            ImsLog.d("KR E-Call check normal registration unconditionally: " + mControlByImsReg);
            return mControlByImsReg;
        }

        if (!isSimLoaded()) {
            ImsLog.d("Return false by force when UICC is not loaded yet");
            return false;
        }

        IDCNetWatcher dcnw = (IDCNetWatcher) DCFactory.getDC(DCFactory.NETWORK_WATCHER, mSlotID);
        if (dcnw != null) {
            // When result of RAT selection is "PS",
            // PS emergency call is preferred even though IMS normal registration is not done,
            // under following condition.
            // 1) no PS attached, or (usually, in this case, result of RAT selection will be "CS")
            // 2) no CS attached (in this case, CS E-Call could not be established)
            if (dcnw.getDataServiceState() != ServiceState.STATE_IN_SERVICE) {
                ImsLog.d("Return false by force when no service case for data");
                return false;
            }

            if (dcnw.getVoiceServiceState() != ServiceState.STATE_IN_SERVICE) {
                ImsLog.d("Return false by force when no service case for voice");
                return false;
            }
        }

        return mControlByImsReg;
    }

    @Override
    public boolean isImsEmergencyCallControlledByVoLteSetting() {
        return mControlByVolteSetting;
    }

    @Override
    public void setBlockSupportOverLTE(boolean bBlockSupportOverLTE) {
        //set it as TRUE if the VoLTE is disabled dynamically
        //ex) disabled by network via OMA DM or other reasons.
        mBlockSupportOverLTE = bBlockSupportOverLTE;
    }

    private void loadConfiguration() {
        mSupportOverLTE = readFromEmergencyConfiguration(
                            ProviderInterface.COM_EMERGENCY.SUPPORT_OVER_LTE);
        mSupportOverWiFi = readFromEmergencyConfiguration(
                            ProviderInterface.COM_EMERGENCY.SUPPORT_OVER_WIFI);
        mControlByVolteSetting = readFromEmergencyConfiguration(
                            ProviderInterface.COM_EMERGENCY.CONTROL_BY_VOLTE_SETTING);
        mControlByImsReg = readFromEmergencyConfiguration(
                            ProviderInterface.COM_EMERGENCY.CONTROL_BY_IMS_REG);
        mRequireNormalEnd = readFromEmergencyConfiguration(
                            ProviderInterface.COM_EMERGENCY.REQUIRE_NORMAL_CALL_END);
    }

    private boolean readFromEmergencyConfiguration(String column) {
        String strEnabled = DBUtils.CP.getString(AppContext.get().getContentResolver(),
                                ProviderInterface.COM_EMERGENCY.CONTENT_URI,
                                ImsDbController.selectForSlot(mSlotID),
                                column,
                                String.valueOf(false));

        if (strEnabled != null) {
            return strEnabled.equalsIgnoreCase("true");
        }

        return false;
    }

    private boolean isImsEmergencyCallRequiredInLimitedServiceStatus() {
        if (OperatorInfo.isEnablerTypeGlobal(mSlotID)) {
            if (ImsProperties.TARGET_COUNTRY.equals("AU")) {
                return true;
            }

            if (isSimLocked()) {
                ImsLog.i("Not required when SIM is locked");
                return false;
            }
        }

        return true;
    }

    private boolean isSimLoaded() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotID);
        return (sim != null) ? sim.isSimLoaded() : false;
    }

    private boolean isSimLocked() {
        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, mSlotID);

        if (sim != null) {
            return sim.getSimState() == Sim.STATE_LOCKED;
        }

        return false;
    }
}
