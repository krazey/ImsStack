package com.android.imsstack.core.agents.dcm;

import android.content.Context;
import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDCSettings;
import com.android.imsstack.core.config.CarrierConfig;

import com.android.imsstack.util.ImsLog;

public class DCSettings implements IDCSettings {
    // Constants--------------------------------------------------

    // Variables--------------------------------------------------
    private Context mContext;
    private int mSlotId = 0;
    private CarrierConfig mCarrierConfig = null;

    // Static loading materials ----------------------------------
    // Public methods --------------------------------------------
    public DCSettings(int slotId) {
        mSlotId = slotId;
    }

    @Override
    public void init(Context context) {
        mContext = context;

        ConfigInterface config = AgentFactory.getInstance().getAgent(
                ConfigInterface.class, mSlotId);

        if (config != null) {
            mCarrierConfig = config.getCarrierConfig();

            if (mCarrierConfig == null) {
                ImsLog.w(mSlotId, "mCarrierConfig is null");
            }
        } else {
            ImsLog.w(mSlotId, "config is null");
        }
    }

    @Override
    public void cleanup() {
    }

    // Interface implementation methods --------------------------

    @Override
    public boolean isRoamingAllowed() {
        if (mCarrierConfig == null) {
            return true;
        }
        return mCarrierConfig.getBoolean(
                CarrierConfigManager.ImsVoice.KEY_CARRIER_VOLTE_ROAMING_AVAILABLE_BOOL, true);
    }

    @Override
    public boolean isVopsRequired() {
        if (mCarrierConfig == null) {
            return true;
        }

        boolean ignoreVops = mCarrierConfig.getBoolean(
                CarrierConfig.Assets.KEY_IGNORE_VOPS_FOR_VOLTE_ENABLE_BOOL, false);
        int[] noVopsRequire = mCarrierConfig.getIntArray(
                CarrierConfigManager.Ims.KEY_IMS_PDN_ENABLED_IN_NO_VOPS_SUPPORT_INT_ARRAY);
        IDCNetWatcher dcnw = (IDCNetWatcher)DCFactory.getDC(DCFactory.NETWORK_WATCHER, mSlotId);

        if (ignoreVops && (noVopsRequire != null) && (dcnw != null)) {
            for (int i = 0; i < noVopsRequire.length ; i++) {
                if (dcnw.isRoaming()) {
                    if(noVopsRequire[i] == CarrierConfigManager.Ims.NETWORK_TYPE_ROAMING) {
                        return false;
                    }
                } else {
                    if(noVopsRequire[i] == CarrierConfigManager.Ims.NETWORK_TYPE_HOME) {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    @Override
    public int[] getImsSupportedRats() {
        if (mCarrierConfig != null) {
            int[] supportedRats = mCarrierConfig.getIntArray(
                    CarrierConfigManager.Ims.KEY_SUPPORTED_RATS_INT_ARRAY);
            if (supportedRats != null) {
                return supportedRats;
            } else {
                ImsLog.w(mSlotId, "supportedRats is null");
            }
        } else {
            ImsLog.w(mSlotId, "mCarrierConfig is null");
        }
        return new int[]{};
    }

    @Override
    public int getPreferredIpVersion() {
        if (mCarrierConfig != null) {
            return mCarrierConfig.getInt(CarrierConfig.Ims.KEY_IMS_PREFERRED_IPTYPE_INT,
                    CarrierConfig.Ims.IPV6_PREFERRED);
        }
        return CarrierConfig.Ims.IPV6_PREFERRED;
    }

    @Override
    public int getEmergencyPreferredIpVersion() {
        if (mCarrierConfig != null) {
            return mCarrierConfig.getInt(CarrierConfig.Assets.KEY_EMERGENCY_PREFERRED_IPTYPE_INT,
                    CarrierConfig.Ims.IPV6_PREFERRED);
        }
        return CarrierConfig.Ims.IPV6_PREFERRED;
    }

    @Override
    public boolean isPermanentFailure(int causeCode) {
        if (mCarrierConfig != null) {
            int[] permanentFailure = mCarrierConfig.getIntArray(
                    CarrierConfig.Assets.KEY_PERMANENT_PDN_FAILURE_INT_ARRAY);
            if (permanentFailure != null) {
                for (int i = 0; i < permanentFailure.length ; i++) {
                    if (permanentFailure[i] == causeCode) {
                        ImsLog.w(mSlotId, "permanent failure cause " + causeCode);
                        return true;
                    }
                }
            }
        }
        return false;
    }
    // Private/Protected methods ---------------------------------
    //----------------------------------------------------------------------------------------------
}
