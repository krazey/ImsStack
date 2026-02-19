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

package com.android.imsstack.imsservice.mmtel.ut;

import android.telephony.TelephonyManager;
import android.telephony.ims.ImsReasonInfo;

import com.android.imsstack.core.agents.Usat;
import com.android.imsstack.core.agents.UsatInterface;
import com.android.imsstack.core.agents.dcm.DcFactory;
import com.android.imsstack.core.agents.dcmif.IDcNetWatcher;
import com.android.imsstack.enabler.IBaseContext;
import com.android.imsstack.enabler.ssc.SscConstant;
import com.android.imsstack.enabler.ssc.SscServiceClassUtil;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtInterface;
import com.android.imsstack.imsservice.mmtel.ut.base.IUtListener;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.ImsUtils;

/**
 * The class to process Ut transaction for each operation.
 */
public final class UtCommand {
    private static final boolean DBG = ImsLog.isDebuggable();

    public static final int CMD_CB = UtMmiConverter.CATEGORY_CB;
    public static final int CMD_CF = UtMmiConverter.CATEGORY_CF;
    public static final int CMD_CW = UtMmiConverter.CATEGORY_CW;
    public static final int CMD_OIR = UtMmiConverter.CATEGORY_OIR;
    public static final int CMD_OIP = UtMmiConverter.CATEGORY_OIP;
    public static final int CMD_TIR = UtMmiConverter.CATEGORY_TIR;
    public static final int CMD_TIP = UtMmiConverter.CATEGORY_TIP;

    private final IBaseContext mContext;
    private final int mTransactionId;
    private final int mCommand;
    private final int mAction;
    private int mCondition = SscConstant.CONDITION_INVALID; // for CB, CF
    private int mServiceClass = SscServiceClassUtil.SERVICE_CLASS_NONE;
    private int mNoReplyTimer = 0; // for CFNR
    private String mTargetNumber = null; // for CF
    private String mPassword = null; // for CB
    private String[] mBarringList = null; // for CB
    private boolean mEnable = false; // for CW, OIP, TIP
    private int mClirMode = SscConstant.OIR_DEFAULT; // for OIR
    private int mColrPresentation = SscConstant.TIR_UNKNOWN; // for TIR

    private final IUtListener mListener;

    private UtCommand(IBaseContext context, int transactionId, int command, int action,
            IUtListener listener) {
        mContext = context;
        mTransactionId = transactionId;
        mCommand = command;
        mAction = action;
        mListener = listener;
    }

    /**
     * triggering Ut operation for each command type. If USAT is supported, call control by UICC is
     * processed before Ut transaction.
     */
    public void startTransaction() {
        UsatInterface usat = mContext.getUsatInterface();
        if (usat != null && usat.isServiceAvailable(Usat.SERVICE_CALL_CONTROL)) {
            startCallControl(usat);
            return;
        }

        doUtTransaction();
    }

    private void startCallControl(UsatInterface usat) {
        int ccType = Usat.CALL_CONTROL_TYPE_SS;
        String dialedString = convertToDialString();
        int networkType = getNetworkTypeForUsat();
        int mediaType = Usat.MEDIA_TYPE_NONE;

        Usat.CallControlCommand usatCommand = usat.createCallControlCommand(ccType, dialedString,
                networkType, mediaType,
                new Usat.Listener() {
                    @Override
                    public void onCommandResponse(Usat.CommandResponse response) {
                        if (response.getResult() == Usat.RESULT_NOT_ALLOWED) {
                            handleUsatNotAllowed();
                        } else if (response.getResult() == Usat.RESULT_ALLOWED_WITH_MODIFICATION) {
                            handleUsatAllowedWithModification(response);
                        } else { // Usat.RESULT_ALLOWED
                            handleUsatAllowed();
                        }
                    }
                });

        usat.sendCommand(usatCommand);
    }

    private void doUtTransaction() {
        log("doUtTransaction()");

        IUtInterface ut = UtFactory.getInstance().getUtInterface(mContext.getSlotId());
        if (ut == null) {
            sendFailResponse(ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE, null);
            return;
        }

        if (!SscServiceClassUtil.isValid(mServiceClass)) {
            log("invalid service class : " + mServiceClass);
            sendFailResponse(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, null);
            return;
        }

        // remove service classes except voice and video
        mServiceClass = SscServiceClassUtil.removeInvalidServiceClass(mServiceClass);

        if (mAction == SscConstant.ACTION_INTERROGATION) {
            switch (mCommand) {
                case CMD_CB:
                    ut.queryCallBarringForServiceClass(mTransactionId, mCondition, mServiceClass);
                    break;
                case CMD_CF:
                    ut.queryCallForward(mTransactionId, mCondition, mTargetNumber);
                    break;
                case CMD_CW:
                    ut.queryCallWaiting(mTransactionId);
                    break;
                case CMD_OIR:
                    ut.queryCLIR(mTransactionId);
                    break;
                case CMD_OIP:
                    ut.queryCLIP(mTransactionId);
                    break;
                case CMD_TIR:
                    ut.queryCOLR(mTransactionId);
                    break;
                case CMD_TIP:
                    ut.queryCOLP(mTransactionId);
                    break;
                default:
                    // do nothing
                    break;
            }
        } else { // actions for update
            switch (mCommand) {
                case CMD_CB:
                    ut.updateCallBarringWithPassword(mTransactionId, mCondition, mAction,
                            mBarringList, mServiceClass, mPassword);
                    break;
                case CMD_CF:
                    ut.updateCallForward(mTransactionId, mAction, mCondition, mTargetNumber,
                            mServiceClass, mNoReplyTimer);
                    break;
                case CMD_CW:
                    ut.updateCallWaiting(mTransactionId, mEnable, mServiceClass);
                    break;
                case CMD_OIR:
                    ut.updateCLIR(mTransactionId, mClirMode);
                    break;
                case CMD_OIP:
                    ut.updateCLIP(mTransactionId, mEnable);
                    break;
                case CMD_TIR:
                    ut.updateCOLR(mTransactionId, mColrPresentation);
                    break;
                case CMD_TIP:
                    ut.updateCOLP(mTransactionId, mEnable);
                    break;
                default:
                    // do nothing
                    break;
            }
        }
    }

    private void handleUsatAllowed() {
        if (DBG) {
            log("Transaction ID : " + mTransactionId + " allowed by UICC");
        }

        doUtTransaction();
    }

    private void handleUsatNotAllowed() {
        if (DBG) {
            log("Transaction " + mTransactionId + " not allowed by UICC");
        }

        sendFailResponse(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, null);
    }

    private void handleUsatAllowedWithModification(Usat.CommandResponse response) {
        Usat.CallControlCommandResponse usatResponse = (Usat.CallControlCommandResponse) response;
        int modifiedCcType = usatResponse.getCcType();
        String dialedString = usatResponse.getDialedString();

        if (DBG) {
            log("Transaction " + mTransactionId + " modified to CC : " + modifiedCcType
                    + " and dial string : " + dialedString);
        }

        if (ImsUtils.hasWildValueForDialString(dialedString)) {
            /**
             * 3GPP 7.3.1.6
             * if the SS string data object or address data object is present and the ME receives
             * wild values according to TS 31.102[14], then the ME shall not process the command.
             */
            sendFailResponse(ImsReasonInfo.CODE_UT_OPERATION_NOT_ALLOWED, null);
            return;
        }

        switch (modifiedCcType) {
            case Usat.CALL_CONTROL_TYPE_MO_CALL:
                if (usatResponse.getMediaType() == Usat.MEDIA_TYPE_VIDEO) {
                    sendFailResponse(ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_DIAL_VIDEO, dialedString);
                } else {
                    sendFailResponse(ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_DIAL, dialedString);
                }
                break;
            case Usat.CALL_CONTROL_TYPE_USSD:
                sendFailResponse(ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_USSD, dialedString);
                break;
            case Usat.CALL_CONTROL_TYPE_SS:
                sendFailResponse(ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_SS, dialedString);
                break;
            case Usat.CALL_CONTROL_TYPE_NONE:
            default:
                // This is an exceptional case. modifiedCcType must have one of above types.
                doUtTransaction();
                break;
        }
    }

    private void sendFailResponse(int reasonCode, String extraText) {
        if (mListener == null) {
            return;
        }

        ImsReasonInfo ri = new ImsReasonInfo(reasonCode, ImsReasonInfo.CODE_UNSPECIFIED, extraText);
        if (mAction == SscConstant.ACTION_INTERROGATION) {
            mListener.utConfigurationQueryFailed(mTransactionId, ri);
        } else {
            mListener.utConfigurationUpdateFailed(mTransactionId, ri);
        }
    }

    private String convertToDialString() {
        switch(mCommand) {
            case CMD_CB:
                if (mAction == SscConstant.ACTION_INTERROGATION) {
                    return UtMmiConverter.getMmiCode(mCommand, mAction, mCondition);
                } else {
                    return UtMmiConverter.getMmiCodeSiaSib(mCommand, mAction, mCondition, mPassword,
                            mServiceClass);
                }
            case CMD_CF:
                if (mAction == SscConstant.ACTION_INTERROGATION) {
                    return UtMmiConverter.getMmiCode(mCommand, mAction, mCondition);
                } else {
                    return UtMmiConverter.getMmiCodeSiaSibSic(mCommand, mAction, mCondition,
                            mTargetNumber, mServiceClass, mNoReplyTimer);
                }
            case CMD_CW:
                if (mAction == SscConstant.ACTION_INTERROGATION) {
                    return UtMmiConverter.getMmiCode(mCommand, mAction);
                } else {
                    return UtMmiConverter.getMmiCodeSia(mCommand, mAction, mServiceClass);
                }
            case CMD_OIR:
            case CMD_OIP:
            case CMD_TIR:
            case CMD_TIP:
                return UtMmiConverter.getMmiCode(mCommand, mAction);
            default:
                // do nothing
                break;
        }

        return null;
    }

    private int getNetworkTypeForUsat() {
        IDcNetWatcher dnw = DcFactory.getDcAgent(IDcNetWatcher.class, mContext.getSlotId());
        int networkType = (dnw != null)
                ? dnw.getNetworkType() : TelephonyManager.NETWORK_TYPE_UNKNOWN;
        if (networkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
            networkType = TelephonyManager.NETWORK_TYPE_IWLAN;
        }

        return networkType;
    }

    /**
     * Builds {@link UtCommand} instances, which may include optional parameters.
     */
    public static final class Builder {
        private final UtCommand mUtCommand;

        public Builder(IBaseContext context, int transactionId, int command, int action,
                IUtListener listener) {
            mUtCommand = new UtCommand(context, transactionId, command, action, listener);
        }

        /**
         * Set reason for call forwarding or call barring.
         * @param condition The condition defined in {@link ImsUtInterface}
         */
        public Builder setCondition(int condition) {
            mUtCommand.mCondition = condition;
            return this;
        }

        /**
         * Set service class which is the telecommunication service defined in 3GPP 22.030 Annex C.
         */
        public Builder setServiceClass(int serviceClass) {
            mUtCommand.mServiceClass = serviceClass;
            return this;
        }

        /**
         * Set timer value for communication forwarding no answer.
         */
        public Builder setNoReplyTimer(int noReplyTimer) {
            mUtCommand.mNoReplyTimer = noReplyTimer;
            return this;
        }

        /**
         * Set the forward-to number for IMS call forwarding.
         * @param targetNumber The number in E.164 international format.
         */
        public Builder setTargetNumber(String targetNumber) {
            mUtCommand.mTargetNumber = targetNumber;
            return this;
        }

        /**
         * Set password which is used for updating call barring supplementary service.
         */
        public Builder setPassword(String password) {
            mUtCommand.mPassword = password;
            return this;
        }

        /**
         * Set barring lists.
         */
        public Builder setBarringList(String[] barringList) {
            mUtCommand.mBarringList = barringList;
            return this;
        }

        /**
         * Set enable
         */
        public Builder setEnable(boolean enable) {
            mUtCommand.mEnable = enable;
            return this;
        }

        /**
         * Set clir mode
         */
        public Builder setClirMode(int clirMode) {
            mUtCommand.mClirMode = clirMode;
            return this;
        }

        /**
         * Set colr presentation
         */
        public Builder setColrPresentation(int presentation) {
            mUtCommand.mColrPresentation = presentation;
            return this;
        }

        /**
         * @return a built {@link UtCommand} containing optional parameters that were set.
         */
        public UtCommand build() {
            return mUtCommand;
        }
    }

    private void log(String s) {
        ImsLog.d(mContext.getSlotId(), "[ISIL] " + s);
    }
}
