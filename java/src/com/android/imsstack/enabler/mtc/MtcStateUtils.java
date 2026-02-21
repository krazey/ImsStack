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
package com.android.imsstack.enabler.mtc;

import android.content.Context;

import com.android.imsstack.base.DeviceConfig;
import com.android.imsstack.base.MSimUtils;
import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;
import com.android.imsstack.core.config.ServiceCaps;
import com.android.imsstack.internal.enabler.ImsStateStore;
import com.android.imsstack.util.ImsLog;

public class MtcStateUtils {
    public static final int INIT_REG_STATE = 0x01;
    public static final int INIT_CALL_STATE = 0x02;
    public static final int INIT_ALL = INIT_REG_STATE | INIT_CALL_STATE;

    public static final int STATE_INACTIVE = ImsStateStore.STATE_INACTIVE;
    public static final int STATE_ACTIVE = ImsStateStore.STATE_ACTIVE;

    public static final String SERVICE_VOIP = "VOIP";
    public static final String SERVICE_VT = "VT";
    public static final String SERVICE_UC = "UC";

    public static final String EXTRA_SUB_ID = "subId";
    public static final String EXTRA_STATE = "state";
    public static final String EXTRA_SVC_TYPE = "svcType";

    private MtcStateUtils() {
    }

    // When "ims" service is opened or operator is changed
    public static void initializeState(Context context, int slotId) {
        ImsStateStore.init(slotId);

        boolean isVoLteEnabled = ServiceCaps.isVoLteEnabledByPlatform(slotId);
        boolean isWfcEnabled = ServiceCaps.isWfcEnabledByPlatform(slotId);
        boolean isVtEnabled = ServiceCaps.isVtEnabledByPlatform(slotId);
        ConfigInterface config = AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
        CarrierConfig cc = (config != null) ? config.getCarrierConfig() : null;
        boolean isVoLteProvisioningRequired =
                (cc != null) ? cc.isVoLteProvisioningRequired() : false;
        boolean isVtProvisioningRequired = isVoLteProvisioningRequired;
        boolean isWfcProvisioningRequired = false;
        boolean isImsStateSyncRequired = isVoLteProvisioningRequired || isVtProvisioningRequired;

        logi("initializeState :: slotId=" + slotId
                + ", voLteEnabled=" + isVoLteEnabled
                + ", wfcEnabled=" + isWfcEnabled
                + ", vtEnabled=" + isVtEnabled
                + ", voLteProvisioningRequired=" + isVoLteProvisioningRequired
                + ", vtProvisioningRequired=" + isVtProvisioningRequired);

        int voLteProvisioned = STATE_INACTIVE;
        int vtProvisioned = STATE_INACTIVE;
        int wfcProvisioned = STATE_INACTIVE;
        ImsStateStore.MmTelState mmTelState = ImsStateStore.getMmTelState(slotId);

        if (isVoLteProvisioningRequired) {
            // VoLTE on/off will be determined by the other parameter (ex. DM)
            voLteProvisioned = mmTelState.isVoLteProvisioned() ? STATE_ACTIVE : STATE_INACTIVE;
        } else if (isVoLteEnabled) {
            voLteProvisioned = STATE_ACTIVE;
        } else {
            voLteProvisioned = STATE_INACTIVE;
        }

        if (isVtProvisioningRequired) {
            // VT on/off will be determined by the other parameter (ex. DM)
            vtProvisioned = mmTelState.isVtProvisioned() ? STATE_ACTIVE : STATE_INACTIVE;
        } else if (isVtEnabled) {
            vtProvisioned = STATE_ACTIVE;
        } else {
            vtProvisioned = STATE_INACTIVE;
        }

        if (isWfcProvisioningRequired) {
            // WFC on/off will be determined by the other parameter (ex. DM)
            wfcProvisioned = mmTelState.isWfcProvisioned() ? STATE_ACTIVE : STATE_INACTIVE;
        } else if (isWfcEnabled) {
            wfcProvisioned = STATE_ACTIVE;
        } else {
            wfcProvisioned = STATE_INACTIVE;
        }

        mmTelState.setProvisioned(voLteProvisioned, vtProvisioned, wfcProvisioned);

        if (isImsStateSyncRequired) {
            logi("initializeState :: ImsStateSyncRequired");
        }
    }

    public static void initializeImsState(Context context, int phoneId, int initFlags) {
        if (phoneId >= MSimUtils.DEFAULT_PHONE_ID) {
            initializeImsStateInternal(phoneId, initFlags);
        } else {
            if (DeviceConfig.isMultiSimEnabled()) {
                int activeSimCount = DeviceConfig.getActiveSimCount();
                for (int i = 0; i < activeSimCount; ++i) {
                    initializeImsStateInternal(i, initFlags);
                }
            } else {
                initializeImsStateInternal(MSimUtils.DEFAULT_PHONE_ID, initFlags);
            }
        }
    }

    public static int getRegisteredServiceType(Context context, int phoneId) {
        return ImsStateStore.getMmTelState(phoneId).getRegisteredServiceType();
    }

    public static boolean isVoLteProvisioned(Context context, int phoneId) {
        return ImsStateStore.getMmTelState(phoneId).isVoLteProvisioned();
    }

    public static boolean isVtProvisioned(Context context, int phoneId) {
        return ImsStateStore.getMmTelState(phoneId).isVtProvisioned();
    }

    public static boolean isWfcProvisioned(Context context, int phoneId) {
        return ImsStateStore.getMmTelState(phoneId).isWfcProvisioned();
    }

    public static void updateCallState(Context context, int slotId, int state) {
        logi("updateCallState :: state=" + state + ", slotId=" + slotId);

        ImsStateStore.getCallState(slotId).setState(state);
    }

    public static void updateRegState(Context context, int slotId, int state) {
        logi("updateRegState :: state=" + state + ", slotId=" + slotId);

        ImsStateStore.getMmTelState(slotId).setRegisteredServiceType(state);
    }

    public static void updateVoLteProvisioned(Context context, int slotId, int provisioned) {
        logi("updateVoLteProvisioned :: provisioned=" + provisioned + ", slotId=" + slotId);

        ImsStateStore.getMmTelState(slotId).setVoLteProvisioned(provisioned);
    }

    public static void updateVtProvisioned(Context context, int slotId, int provisioned) {
        logi("updateVtProvisioned :: provisioned=" + provisioned + ", slotId=" + slotId);

        ImsStateStore.getMmTelState(slotId).setVtProvisioned(provisioned);
    }

    public static void updateWfcProvisioned(Context context, int slotId, int provisioned) {
        logi("updateWfcProvisioned :: provisioned=" + provisioned + ", slotId=" + slotId);

        ImsStateStore.getMmTelState(slotId).setWfcProvisioned(provisioned);
    }

    private static void initializeImsStateInternal(int phoneId, int initFlags) {
        if ((initFlags & INIT_CALL_STATE) != 0) {
            ImsStateStore.getCallState(phoneId).init();
        }

        if ((initFlags & INIT_REG_STATE) != 0) {
            ImsStateStore.getRegState(phoneId).init();
            ImsStateStore.getMmTelState(phoneId).setRegisteredServiceType(STATE_INACTIVE);
        }
    }

    private static void log(String s) {
        ImsLog.d("[ISIL] " + s);
    }

    private static void logi(String s) {
        ImsLog.i("[ISIL] " + s);
    }
}
