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

import com.android.imsstack.enabler.ssc.SscConstant;
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

import java.util.List;

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

    public void setHandler(MtcApp.MtcAppHandler handler) {
        mMtcAppHandler = handler;
    }

    private void updateTbssInfo(@PermanentSuppInfo.PermanentSuppType int type, Object value) {
        if (value instanceof Integer) {
            int intValue = (Integer) value;
            mTbSuppInfo.addServiceInt(type, intValue);
        } else if (value instanceof Boolean) {
            boolean boolValue = (Boolean) value;
            mTbSuppInfo.addServiceBool(type, boolValue);
        } else if (value instanceof String) {
            mTbSuppInfo.addServiceStr(type, String.valueOf(value));
        }
    }

    private @PermanentSuppInfo.PermanentSuppType int getTbcbType(
            @Condition int condition, @ServiceClass int serviceClass) {
        if (serviceClass == CbData.SERVICE_CLASS_VOICE) {
            switch (condition) {
                case SscConstant.CONDITION_BAOC:
                    return PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_ALL_VOICE;
                case SscConstant.CONDITION_BOIC:
                    return PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VOICE;
                case SscConstant.CONDITION_BAIC:
                    return PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ALL_VOICE;
                case SscConstant.CONDITION_BIC_WR:
                    return PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ROAMING_VOICE;
                case SscConstant.CONDITION_ACR:
                    return PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ANONYMOUS_VOICE;
                default:
                    // No specific TB_CB_ mapping for this VOICE condition
                    return PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_ALL_VOICE;
            }
        } else {
            switch (condition) {
                case SscConstant.CONDITION_BAOC:
                    return PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_ALL_VIDEO;
                case SscConstant.CONDITION_BOIC:
                    return PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_INTERNATIONAL_VIDEO;
                case SscConstant.CONDITION_BAIC:
                    return PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ALL_VIDEO;
                case SscConstant.CONDITION_BIC_WR:
                    return PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ROAMING_VIDEO;
                case SscConstant.CONDITION_ACR:
                    return PermanentSuppInfo.SUPP_TYPE_TB_CB_INCOMING_ANONYMOUS_VIDEO;
                default:
                    return PermanentSuppInfo.SUPP_TYPE_TB_CB_OUTGOING_ALL_VIDEO;
            }
        }
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
                    for (SupplementaryServiceConfiguration sscConfig :
                            (List<SupplementaryServiceConfiguration>) msg.obj) {
                        if (sscConfig.getType() == SupplementaryServiceConfiguration.SS_TYPE_TIR) {
                            updateTbssInfo(PermanentSuppInfo.SUPP_TYPE_TB_TIR, sscConfig.getStatus()
                                    == SupplementaryServiceConfiguration.STATUS_ENABLED);
                        } else if (sscConfig.getType()
                                == SupplementaryServiceConfiguration.SS_TYPE_CB) {
                            updateTbssInfo(getTbcbType(((CbData) sscConfig).getCondition(),
                                    ((CbData) sscConfig).getServiceClass()), sscConfig.getStatus()
                                    == SupplementaryServiceConfiguration.STATUS_ENABLED);
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
