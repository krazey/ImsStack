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

package com.android.imsstack.enabler.uce.impl.options;

import com.android.imsstack.util.ImsLog;
import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.define.UceResponseData;
import com.android.imsstack.enabler.uce.impl.jni.IUceJNIListener;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;
import com.android.imsstack.enabler.uce.impl.utils.UceUtils;
import com.android.imsstack.enabler.uce.interf.OptionsResponse;
import com.android.imsstack.enabler.uce.interf.UceApiConstant;
import com.android.internal.annotations.VisibleForTesting;

import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;

import java.util.HashMap;
import java.util.Map;
import java.util.Set;

interface UceOptionsMessageHandler {
    public void onHandle(Message objMessage);
}

public class UceOptionsRequestController implements IUceJNIListener {
    private final int mSlotId;
    private final Map<Integer, UceOptionsRequest> mUceOptionsRequestMap;
    private final UceOptionsControllerHandler mUceOptionsControllerHandler;
    private boolean mIsImsRegistered;

    private UceJNI mUceJNI;

    private HashMap<Integer, UceOptionsMessageHandler> mMessageHandler =
        new HashMap<Integer, UceOptionsMessageHandler>();

    public UceOptionsRequestController(int slotId, Looper looper) {
        mSlotId = slotId;
        mIsImsRegistered = false;
        mUceOptionsRequestMap = new HashMap<Integer, UceOptionsRequest>();
        mUceOptionsControllerHandler = new UceOptionsControllerHandler(looper);

        mUceJNI = UceJNI.getInstance();

        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_OPTIONS_RESPONSE_IND);
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_OPTIONS_CMD_ERROR_IND);
    }

    @VisibleForTesting
    public UceOptionsRequestController(int slotId, UceJNI jni, Looper looper) {
        mSlotId = slotId;
        mIsImsRegistered = false;
        mUceOptionsRequestMap = new HashMap<Integer, UceOptionsRequest>();
        mUceOptionsControllerHandler = new UceOptionsControllerHandler(looper);

        mUceJNI = jni;

        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_OPTIONS_RESPONSE_IND);
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_OPTIONS_CMD_ERROR_IND);
    }

    /**
     * Set the current ims registration status.
     * @param imsRegistered set to true if the ims registration is successful, false otherwise.
     */
    public void setImsRegistrationStatus(boolean imsRegistered) {
        if (mIsImsRegistered != imsRegistered) {
            mIsImsRegistered = imsRegistered;
            ImsLog.i("IsImsRegistered:" + mIsImsRegistered);
        }
    }

    /**
     * Push one's own capabilities to a remote user via the SIP OPTIONS presence exchange mechanism
     * in order to receive the capabilities of the remote user in response.
     */
    public void sendOptionsCapabilityRequest(Uri contactUri, Set<String> myCapabilities,
            OptionsResponse cb) {
        if (!mIsImsRegistered) {
            sendCommandError(cb, UceApiConstant.COMMAND_CODE_SERVICE_UNAVAILABLE);
            return;
        }

        int key = UceUtils.generateKey();
        UceOptionsRequest request = new UceOptionsRequest(cb, mSlotId, key);
        String remoteUri = contactUri.toString();
        if (request.sendRequest(remoteUri, myCapabilities)) {
            mUceOptionsRequestMap.put(key, request);
        }
    }

    @VisibleForTesting
    public void sendOptionsCapabilityRequest(Uri contactUri, Set<String> myCapabilities,
            OptionsResponse cb, UceOptionsRequest request) {
        if (!mIsImsRegistered) {
            sendCommandError(cb, UceApiConstant.COMMAND_CODE_SERVICE_UNAVAILABLE);
            return;
        }
        int key = UceUtils.generateKey();
        String remoteUri = contactUri.toString();
        if (request.sendRequest(remoteUri, myCapabilities)) {
            mUceOptionsRequestMap.put(key, request);
        }
    }

    @VisibleForTesting
    public Handler getHandler() {
        return mUceOptionsControllerHandler;
    }

    @VisibleForTesting
    public void setRequestWithKey(int key, UceOptionsRequest request) {
        mUceOptionsRequestMap.put(key, request);
    }

    class UceOptionsControllerHandler extends Handler {
        public UceOptionsControllerHandler(Looper looper) {
            super(looper);
            mMessageHandler.put(UceMessage.UCE_MSG_OPTIONS_RESPONSE, mResponseMsgHandler);
            mMessageHandler.put(UceMessage.UCE_MSG_OPTIONS_CMD_ERROR, mCommandMsgHandler);
        }

        @Override
        public void handleMessage(Message objMessage) {
            if (objMessage == null) {
                ImsLog.e("Message is null");
                return;
            }
            UceOptionsMessageHandler objMsgHandler = mMessageHandler.get(objMessage.what);
            if (objMsgHandler == null) {
                ImsLog.e("message can not be handled.");
                return;
            }
            objMsgHandler.onHandle(objMessage);
        }

        private UceOptionsMessageHandler mResponseMsgHandler = new UceOptionsMessageHandler() {
            @Override
            public void onHandle(Message objMessage) {
                int requestKey = objMessage.arg1;
                UceOptionsRequest request = getOptionsRequest(requestKey);
                if (request == null) {
                    ImsLog.e("Do not find request for Key=" + requestKey);
                    return;
                }
                UceResponseData data = (UceResponseData)objMessage.obj;
                int responseCode = data.getResponseCode();
                String reason = data.getReason();
                long capabilities = data.getCapability();

                request.informNetworkResponse(responseCode, reason, capabilities);
                removeOptionsRequest(requestKey);
            }
        };

        private UceOptionsMessageHandler mCommandMsgHandler = new UceOptionsMessageHandler() {
            @Override
            public void onHandle(Message objMessage) {
                int requestKey = objMessage.arg1;
                UceOptionsRequest request = getOptionsRequest(requestKey);
                if (request == null) {
                    ImsLog.e("Do not find request for Key=" + requestKey);
                    return;
                }
                int commandErrorCode = objMessage.arg2;
                request.informCommandError(commandErrorCode);
                removeOptionsRequest(requestKey);
            }
        };
    }

    private void sendCommandError(OptionsResponse cb, int code) {
        ImsLog.d("sendCommandError:" + code);
        try {
            cb.onCommandError(code);
        } catch (Exception e) {
            ImsLog.e("Exception:" + e.toString());
        }
    }

    private UceOptionsRequest getOptionsRequest(int key) {
        UceOptionsRequest request = null;
        try {
            request = mUceOptionsRequestMap.get(key);
        } catch (ClassCastException e) {
            ImsLog.e("ClassCastException:" + e.toString());
        } catch (NullPointerException e) {
            ImsLog.e("NullPointerException:" + e.toString());
        } finally {
            return request;
        }
    }

    private void removeOptionsRequest(int key) {
        try {
            mUceOptionsRequestMap.remove(key);
            ImsLog.d("remove key:" + key);
        } catch (ClassCastException e) {
            ImsLog.e("ClassCastException:" + e.toString());
        } catch (NullPointerException e) {
            ImsLog.e("NullPointerException:" + e.toString());
        } catch (UnsupportedOperationException e) {
            ImsLog.e("UnsupportedOperationException:" + e.toString());
        } finally {
            return;
        }
    }

    @Override
    public void onOptionsResponseMessage(Parcel parcel) {
        ImsLog.d("");
        Message msg = Message.obtain();
        int messageType = parcel.readInt();
        switch (messageType) {
            case UceMessage.UCE_OPTIONS_RESPONSE_IND: {
                msg.what = UceMessage.UCE_MSG_OPTIONS_RESPONSE;
                int requestKey = parcel.readInt();
                int responseCode = parcel.readInt();
                String reason = parcel.readString();
                long capabilities = parcel.readLong();

                ImsLog.d("responseCode:" + responseCode + ", reason:" + reason);

                msg.arg1 = requestKey;
                UceResponseData data = new UceResponseData(responseCode, reason);
                data.setCapability(capabilities);
                msg.obj = data;
                break;
            }
            case UceMessage.UCE_OPTIONS_CMD_ERROR_IND: {
                msg.what = UceMessage.UCE_MSG_OPTIONS_CMD_ERROR;
                int requestKey = parcel.readInt();
                int commandErrorCode = parcel.readInt();
                msg.arg1 = requestKey;
                msg.arg2 = commandErrorCode;
                break;
            }
            default:
                ImsLog.d("message not handle");
                return;
        }
        mUceOptionsControllerHandler.sendMessage(msg);
    }
}
