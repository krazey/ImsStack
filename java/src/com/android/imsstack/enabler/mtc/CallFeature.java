/*
    Author
    <table>
    date        author                  description
    --------    --------------          ----------
    20150428    hwangoo.park@           Created
    </table>

    Description
*/

package com.android.imsstack.enabler.mtc;

import android.telephony.CarrierConfigManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.ConfigInterface;
import com.android.imsstack.core.config.CarrierConfig;

import java.util.Arrays;

public final class CallFeature {

    private CallFeature() {
        // no-op
    }

    public static int getStatusCodeforCallTypeChangeReject(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig().getInt(
                CarrierConfig.Assets.KEY_SIP_STATUS_CODE_FOR_REJECTING_CALL_TYPE_CHANGE_INT);
    }

    public static boolean isAudioEvsSupported(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.ImsVoice.KEY_EVS_SUPPORT_BOOL);
    }

    /**
     * Checks if the carrier supports HEVC(H.264) video codec or not.
     *
     * @param slotId The slot-id to be checked.
     * @return true if the carrier supports HEVC, false otherwise.
     */
    public static boolean isVideoHevcSupported(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.ImsVt.KEY_HEVC_SUPPORT_BOOL);
    }

    public static boolean isCallHoldUsingInactive(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.Assets.KEY_AUDIO_HOLD_WITH_DIRECTION_INACTIVE_BOOL);
    }

    public static boolean isIncomingResumeEventSupported(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.Assets.KEY_INCOMING_RESUME_EVENT_SUPPORT_BOOL);
    }

    public static boolean isSrvccSupported(int slotId) {
        int[] srvccType = getConfigInterface(slotId).getCarrierConfig()
                .getIntArray(CarrierConfigManager.ImsVoice.KEY_SRVCC_TYPE_INT_ARRAY);
        if (srvccType != null) {
            return Arrays.stream(srvccType).anyMatch(i -> i
                    == CarrierConfigManager.ImsVoice.BASIC_SRVCC_SUPPORT);
        } else {
            return false;
        }
    }

    public static boolean isTtySupported(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfigManager.KEY_CARRIER_VOLTE_TTY_SUPPORTED_BOOL, false);
    }

    public static boolean isRttSupported(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfigManager.KEY_RTT_SUPPORTED_BOOL, false);
    }

    public static boolean isVideoDirectionInactiveOnVideoCallHold(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.Assets.KEY_VIDEO_HOLD_WITH_DIRECTION_INACTIVE_BOOL);
    }

    public static boolean isTextDirectionInactiveOnRttCallHold(int slotId) {
        return getConfigInterface(slotId).getCarrierConfig()
                .getBoolean(CarrierConfig.Assets.KEY_TEXT_HOLD_WITH_DIRECTION_INACTIVE_BOOL);
    }

    /**
     * Returns the configuration interface.
     *
     * @param slotId The slot-id to be retrieved.
     * @return A ConfigInterface instance.
     */
    private static ConfigInterface getConfigInterface(int slotId) {
        return AgentFactory.getInstance().getAgent(ConfigInterface.class, slotId);
    }
}
