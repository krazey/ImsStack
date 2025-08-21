/*
 * Copyright (C) 2025 The Android Open Source Project
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

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.TelephonyInterface;
import com.android.imsstack.enabler.mtc.PermanentSuppInfo.PermanentSuppType;
import com.android.imsstack.enabler.mtc.SuppServiceUtils.SuppService;
import com.android.imsstack.imsservice.mmtel.ut.UtFactory;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.CbData;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.CbData.Condition;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.CbData.ServiceClass;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface.SupplementaryServiceConfiguration;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.internal.imsservice.MmTelFeatureRegistry;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import com.google.i18n.phonenumbers.NumberParseException;
import com.google.i18n.phonenumbers.PhoneNumberUtil;
import com.google.i18n.phonenumbers.Phonenumber;

import java.util.List;
import java.util.Locale;

/**
 * This class notifies the Native of information of the Terminal-based supplementary services.
 */
public class MtcTerminalBasedSupplementaryServiceNotifier {
    protected static final int MSG_ON_TBCW_CHANGED = 1;
    protected static final int MSG_ON_TBTIR_TBCB_CHANGED = 2;

    private final int mSlotId;
    private final MmtelFeatureListener mMmtelFeatureListener = new MmtelFeatureListener();
    private final SscConfigChangeListener mSscConfigChangeListener = new SscConfigChangeListener();
    private final TbssHandler mTbssHandler;
    private final PermanentSuppInfo mTbSuppInfo = new PermanentSuppInfo();
    private final PermanentSuppInfo mTbOutgoingCallBarringInfo = new PermanentSuppInfo();
    private MtcApp.MtcAppHandler mMtcAppHandler;

    public MtcTerminalBasedSupplementaryServiceNotifier(int slotId, Looper looper) {
        mSlotId = slotId;
        mTbssHandler = new TbssHandler(looper);
    }

    /**
     * Adds listeners.
     */
    public void init() {
        log("init");

        MmTelFeatureRegistry mmtelFr = ImsServiceRegistry.getInstance(mSlotId)
                .getMmTelFeatureRegistry();
        if (mmtelFr != null) {
            mmtelFr.addListener(mMmtelFeatureListener);
        }

        IUtInterface UtInterface = UtFactory.getInstance().getUtInterface(mSlotId);
        if (UtInterface == null) {
            return;
        }
        UtInterface.addTbSscChangeListener(mSscConfigChangeListener);
    }

    /**
     * Removes listeners.
     */
    public void deinit() {
        log("deinit");

        MmTelFeatureRegistry mmtelFr = ImsServiceRegistry.getInstance(mSlotId)
                .getMmTelFeatureRegistry();
        if (mmtelFr != null) {
            mmtelFr.removeListener(mMmtelFeatureListener);
        }

        IUtInterface UtInterface = UtFactory.getInstance().getUtInterface(mSlotId);
        if (UtInterface != null) {
            UtInterface.removeTbSscChangeListener(mSscConfigChangeListener);
        }
    }

    /**
     * Notifies the Native about terminal-based supplementary service if possible.
     */
    public void notifyInfo() {
        if (mMtcAppHandler == null || mTbSuppInfo.getServicesSize() == 0) {
            return;
        }

        Parcel parcel = Parcel.obtain();
        parcel.writeInt(IUMtcService.PERMANENT_SUPP_CHANGED);
        mTbSuppInfo.writeToParcel(parcel, 1);
        Message.obtain(mMtcAppHandler, MtcApp.MSG_SEND_NOTIFICATION, parcel).sendToTarget();

        mTbSuppInfo.getServices().clear();
    }

    /**
     * Sets the {@link MtcApp.MtcAppHandler} to send information.
     *
     * @param handler The handler to be set.
     */
    public void setHandler(MtcApp.MtcAppHandler handler) {
        mMtcAppHandler = handler;
    }

    /**
     * Checks if an outgoing call is barred based on the call type and the recipient's number.
     *
     * @param callType The type of the outgoing call.
     * @param callee The phone number of the recipient of the outgoing call to check
     *               if it is an international number.
     * @return {@code true} if the outgoing call is barred, {@code false} otherwise.
     */
    public boolean isOutgoingCallBarringActivated(int callType, String callee) {
        if (callType == IUMtcCall.CALLTYPE_VT || callType == IUMtcCall.CALLTYPE_VIDEO_RTT) {
            return isOutgoingBarringActivatedByCallType(
                    PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_ALL_VIDEO,
                    PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VIDEO, callee);
        } else {
            return isOutgoingBarringActivatedByCallType(
                    PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_ALL_VOICE,
                    PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VOICE, callee);
        }
    }

    private boolean isOutgoingBarringActivatedByCallType(@PermanentSuppType int barringTypeAll,
            @PermanentSuppType int barringTypeInternational, String callee) {
        return isOutgoingBarringActivated(barringTypeAll)
                || (isInternationalCall(callee)
                && isOutgoingBarringActivated(barringTypeInternational));
    }

    private boolean isOutgoingBarringActivated(@PermanentSuppType int type) {
        SuppService targetService = mTbOutgoingCallBarringInfo.getService(type);
        return targetService != null && targetService.boolValue;
    }

    /**
     * Determines if a given phone number is an international number relative to the
     * network's current country.
     *
     * @param callee The phone number to check (e.g., "+821012345678", "01012345678").
     * @return true if the number is international, false otherwise.
     */
    private boolean isInternationalCall(String callee) {
        if (!callee.startsWith("+")) {
            return false;
        }

        String networkCountryIso = getNetworkCountryIso();
        if (networkCountryIso.isEmpty()) {
            log("Network country is unknown, can't determine if call is international.");
            return false;
        }

        try {
            PhoneNumberUtil phoneUtil = PhoneNumberUtil.getInstance();
            Phonenumber.PhoneNumber parsedNumber = phoneUtil.parse(callee, networkCountryIso);

            String numberRegionCode = phoneUtil.getRegionCodeForNumber(parsedNumber);
            if (!phoneUtil.isValidNumberForRegion(parsedNumber, numberRegionCode)) {
                log("Invalid number: " + callee);
                return false;
            }


            log("Checking number: " + callee
                    + ", network country: " + networkCountryIso
                    + ", number region: " + numberRegionCode);

            return !networkCountryIso.equals(numberRegionCode);

        } catch (NumberParseException e) {
            ImsLog.e(this, mSlotId, "Error parsing phone number : callee " + e.toString());
            return false;
        }
    }

    /**
     * Gets the device's current network country ISO code (e.g., "KR" for South Korea).
     */
    private String getNetworkCountryIso() {
        TelephonyInterface telephony = AgentFactory.getInstance().getAgent(
                TelephonyInterface.class, mSlotId);

        if (telephony != null) {
            String networkCountryIso = telephony.getNetworkCountryIso();
            if (networkCountryIso != null && !networkCountryIso.isEmpty()) {
                log("Network country : " + networkCountryIso.toUpperCase(Locale.ROOT));
                return networkCountryIso.toUpperCase(Locale.ROOT);
            }
        }

        log("Could not determine device network country.");
        return "";
    }

    private void updateTbssInfo(@PermanentSuppInfo.PermanentSuppType int type, Object value) {
        PermanentSuppInfo targetInfo;

        if (type == PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_ALL_VOICE
                || type == PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VOICE
                || type == PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_ALL_VIDEO
                || type == PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VIDEO) {
            targetInfo = mTbOutgoingCallBarringInfo;
        } else {
            targetInfo = mTbSuppInfo;
        }

        if (value instanceof Integer) {
            int intValue = (Integer) value;
            targetInfo.addServiceInt(type, intValue);
        } else if (value instanceof Boolean) {
            boolean boolValue = (Boolean) value;
            targetInfo.addServiceBool(type, boolValue);
        } else if (value instanceof String) {
            targetInfo.addServiceStr(type, String.valueOf(value));
        }
    }

    private @PermanentSuppInfo.PermanentSuppType int getTbcbType(
            @Condition int condition, @ServiceClass int serviceClass) {
        if (serviceClass == CbData.SERVICE_CLASS_VOICE) {
            return switch (condition) {
                case CbData.CONDITION_BAOC
                        -> PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_ALL_VOICE;
                case CbData.CONDITION_BOIC
                        -> PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VOICE;
                case CbData.CONDITION_BAIC
                        -> PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ALL_VOICE;
                case CbData.CONDITION_BIC_WR
                        -> PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ROAMING_VOICE;
                case CbData.CONDITION_ACR
                        -> PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ANONYMOUS_VOICE;
                case CbData.CONDITION_BOIC_EXHC
                        -> PermanentSuppInfo
                            .SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_EXCEPT_HOME_VOICE;
                default -> throw new IllegalArgumentException(
                        "Unsupported condition: " + condition);
            };
        } else if (serviceClass == CbData.SERVICE_CLASS_VIDEO) {
            return switch (condition) {
                case CbData.CONDITION_BAOC
                        -> PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_ALL_VIDEO;
                case CbData.CONDITION_BOIC
                        -> PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VIDEO;
                case CbData.CONDITION_BAIC
                        -> PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ALL_VIDEO;
                case CbData.CONDITION_BIC_WR
                        -> PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ROAMING_VIDEO;
                case CbData.CONDITION_ACR
                        -> PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ANONYMOUS_VIDEO;
                case CbData.CONDITION_BOIC_EXHC
                        -> PermanentSuppInfo
                            .SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_EXCEPT_HOME_VIDEO;
                default -> throw new IllegalArgumentException(
                        "Unsupported condition: " + condition);
            };
        }

        throw new IllegalArgumentException("Unsupported service class: " + serviceClass);
    }

    private static void log(String s) {
        ImsLog.d("[GII-MTC] " + s);
    }

    @VisibleForTesting
    protected class MmtelFeatureListener implements MmTelFeatureRegistry.Listener {
        @Override
        public void onTerminalBasedCallWaitingStatusChanged() {
            MmTelFeatureRegistry mmtelFr = ImsServiceRegistry.getInstance(mSlotId)
                    .getMmTelFeatureRegistry();
            if (mmtelFr != null) {
                Message.obtain(mTbssHandler, MSG_ON_TBCW_CHANGED,
                        mmtelFr.isTerminalBasedCallWaitingEnabled()).sendToTarget();
            }
        }
    }

    private class SscConfigChangeListener implements
            IUtInterface.TerminalBasedSupplementaryServiceConfigurationChangeListener {
        @Override
        public void onSupplementaryServiceConfigurationChanged(
                List<SupplementaryServiceConfiguration> data) {
            Message.obtain(mTbssHandler, MSG_ON_TBTIR_TBCB_CHANGED, data).sendToTarget();
        }
    }

    private class TbssHandler extends Handler {
        TbssHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case MSG_ON_TBCW_CHANGED: {
                    log("MSG_ON_TBCW_CHANGED");
                    updateTbssInfo(PermanentSuppInfo.SUPP_TYPE_TB_CW, msg.obj);
                    break;
                }

                case MSG_ON_TBTIR_TBCB_CHANGED: {
                    log("MSG_ON_TBTIR_TBCB_CHANGED");
                    if (!(msg.obj instanceof List)) {
                        ImsLog.e(this, mSlotId, "Invalid object type for "
                                + "MSG_ON_TBTIR_TBCB_CHANGED: " + msg.obj);
                        break;
                    }

                    for (Object item : (List<?>) msg.obj) {
                        if (!(item instanceof SupplementaryServiceConfiguration)) {
                            continue;
                        }
                        SupplementaryServiceConfiguration sscConfig =
                                (SupplementaryServiceConfiguration) item;
                        if (sscConfig.getType() == SupplementaryServiceConfiguration.SS_TYPE_TIR) {
                            updateTbssInfo(PermanentSuppInfo.SUPP_TYPE_TB_TIR, sscConfig.getStatus()
                                    == SupplementaryServiceConfiguration.STATUS_ENABLED);
                        } else if (sscConfig.getType()
                                == SupplementaryServiceConfiguration.SS_TYPE_CB) {
                            try {
                                updateTbssInfo(getTbcbType(((CbData) sscConfig).getCondition(),
                                        ((CbData) sscConfig).getServiceClass()),
                                        sscConfig.getStatus()
                                        == SupplementaryServiceConfiguration.STATUS_ENABLED);
                            } catch (IllegalArgumentException e) {
                                log("Invalid TBCB condition or service class: " + e.getMessage());
                            }
                        }
                    }
                    break;
                }

                default:
                    // no-op
                    break;
            }

            notifyInfo();
        }
    }
}
