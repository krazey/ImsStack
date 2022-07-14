/*
    Author
    <table>
    date         author                 description
    --------     --------------         ----------
    20141106     hyunho.shin@           Created
    </table>

    Description
*/

package com.android.imsstack.enabler.uce.impl.configuration;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.util.ImsLog;

public final class UceConfiguration {
    private int mSlotId;
    private CarrierConfig mCarrierConfig;

    public UceConfiguration(int slotId) {
        mSlotId = slotId;
    }

    public void init() {
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

    public boolean isUseExpiredEtag() {
        if (mCarrierConfig == null) {
            return false;
        }
        return mCarrierConfig.getBoolean(
                CarrierConfig.ImsUce.KEY_USE_EXPIRED_ETAG_BOOL, false);
    }
}
