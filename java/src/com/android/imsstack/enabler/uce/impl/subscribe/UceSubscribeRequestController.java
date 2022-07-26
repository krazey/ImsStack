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

package com.android.imsstack.enabler.uce.impl.subscribe;

import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.text.TextUtils;

import com.android.imsstack.enabler.uce.impl.define.UceConstant;
import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.define.UceResponseData;
import com.android.imsstack.enabler.uce.impl.jni.IUceJNIListener;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;
import com.android.imsstack.enabler.uce.impl.utils.UceUtils;
import com.android.imsstack.enabler.uce.interf.SubscribeResponse;
import com.android.imsstack.enabler.uce.interf.UceApiConstant;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

interface UceSubscribeMessageHandler {
    public void onHandle(Message objMessage);
}

 /**
 * The UceSubscribeRequestController manages the request associated with
 * the UCE subscription requests.
 * When the request receives from the caller, a UceSubscribeRequest instance is created
 * for each request.
 * And the request is complete, the created instance of the UceSubscribeRequest is deleted.
 */
public class UceSubscribeRequestController implements IUceJNIListener {
    private final int mSlotId;
    private final Map<Integer, UceSubscribeRequest> mUceSubscribeRequestMap =
            new HashMap<Integer, UceSubscribeRequest>();
    private final UceSubscribeControllerHandler mUceSubscribeControllerHandler;
    private boolean mIsImsRegistered;
    private UceJNI mUceJNI;

    private HashMap<Integer, UceSubscribeMessageHandler> mMessageHandler =
        new HashMap<Integer, UceSubscribeMessageHandler>();

    public UceSubscribeRequestController(int slotId, Looper looper) {
        this (slotId, UceJNI.getInstance(), looper);
    }

    @VisibleForTesting
    public UceSubscribeRequestController(int slotId, UceJNI jni, Looper looper) {
        mSlotId = slotId;
        mIsImsRegistered = false;
        mUceSubscribeControllerHandler = new UceSubscribeControllerHandler(looper);

        mUceJNI = jni;
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_SUBSCRIBE_RESPONSE_IND);
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_PRESENCE_NOTIFY_IND);
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_SUBSCRIBE_CMD_ERROR_IND);
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_SUBSCRIBE_RESOURCE_TERMINATED_IND);
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_SUBSCRIBE_TERMINATED_IND);
    }
    /**
     * Set the current ims registration status.
     * @param imsRegistered set to true if the ims registration is successful, false otherwise.
     */
    public void setImsRegistrationStatus(boolean imsRegistered) {
        if (mIsImsRegistered != imsRegistered) {
            mIsImsRegistered = imsRegistered;
            ImsLog.i(mSlotId, "IsImsRegistered:" + mIsImsRegistered);
        }
    }

    /**
     * The user capabilities of one or multiple contacts have been requested by the framework.
     * @param uris A list of numbers that the capabilities are being requested for.
     * @param cb A callback for when the request for capabilities completes.
     */
    public void subscribeCapabilities(Collection<Uri> uris, SubscribeResponse cb) {
        if (!mIsImsRegistered) {
            sendCommandError(cb, UceApiConstant.COMMAND_CODE_SERVICE_UNAVAILABLE);
            return;
        }
        if (uris.isEmpty()) {
            sendCommandError(cb, UceApiConstant.COMMAND_CODE_INVALID_PARAM);
            return;
        }

        int key = UceUtils.generateKey();
        UceSubscribeRequest request = new UceSubscribeRequest(cb, mSlotId, key);

        subscribeCapabilities(key, uris, cb, request);
    }

    @VisibleForTesting
    public void subscribeCapabilities(int key, Collection<Uri> uris, SubscribeResponse cb,
            UceSubscribeRequest request) {
        if (!mIsImsRegistered) {
            sendCommandError(cb, UceApiConstant.COMMAND_CODE_SERVICE_UNAVAILABLE);
            request = null;
            return;
        }
        if (uris.isEmpty()) {
            sendCommandError(cb, UceApiConstant.COMMAND_CODE_INVALID_PARAM);
            request = null;
            return;
        }

        ArrayList<String> queryingUri = new ArrayList<>();
        uris.forEach(uri -> {
            queryingUri.add(uri.toString());
        });
        if (request.sendRequest(queryingUri)) {
            mUceSubscribeRequestMap.put(key, request);
            ImsLog.d(mSlotId, "add key:" + key);
        }
    }

    @VisibleForTesting
    public Handler getHandler() {
        return mUceSubscribeControllerHandler;
    }

    @VisibleForTesting
    public void setRequestWithKey(int key, UceSubscribeRequest request) {
        mUceSubscribeRequestMap.put(key, request);
    }

    /**
     * Get the UceSubscribeRequest associate with the input key
     * @param key The key to get the request.
     * @return Request stored as key in the MAP.
     */
    @VisibleForTesting
    public UceSubscribeRequest getRequestWithKey(int key) {
        return mUceSubscribeRequestMap.get(key);
    }

    class UceSubscribeControllerHandler extends Handler {
        public UceSubscribeControllerHandler(Looper looper) {
            super(looper);
            mMessageHandler.put(UceMessage.UCE_MSG_SUBSCRIBE_RESPONSE, mResponseMsgHandler);
            mMessageHandler.put(UceMessage.UCE_MSG_PRESENCE_NOTIFY, mNotifyMsgHandler);
            mMessageHandler.put(UceMessage.UCE_MSG_SUBSCRIBE_CMD_ERROR, mCommandMsgHandler);
            mMessageHandler.put(UceMessage.UCE_MSG_SUBSCRIBE_TERMINATED, mTerminatedMsgHandler);
            mMessageHandler.put(UceMessage.UCE_MSG_SUBSCRIBE_RESOURCE_TERMINATED,
                mResourceTerminatedMsgHandler);
        }

        @Override
        public void handleMessage(Message objMessage) {
            if (objMessage == null) {
                ImsLog.e(mSlotId, "Message is null");
                return;
            }
            UceSubscribeMessageHandler objMsgHandler = mMessageHandler.get(objMessage.what);
            if (objMsgHandler == null) {
                ImsLog.e(mSlotId, "message can not be handled.");
                return;
            }
            objMsgHandler.onHandle(objMessage);
        }

        private UceSubscribeMessageHandler mResponseMsgHandler = new UceSubscribeMessageHandler() {
            @Override
            public void onHandle(Message objMessage) {
                int requestKey = objMessage.arg1;
                UceSubscribeRequest request = getSubscribeRequest(requestKey);
                if (request == null) {
                    ImsLog.e(mSlotId, "Do not find request for Key=" + requestKey);
                    return;
                }
                UceResponseData data = (UceResponseData)objMessage.obj;
                int responseCode = data.getResponseCode();
                String reason = data.getReason();
                int reasonHeaderCause = data.getReasonHeaderCause();
                String reasonHeaderText = data.getReasonHeaderText();

                request.informNetworkResponse(responseCode, reason, reasonHeaderCause,
                    reasonHeaderText);
                if (responseCode != 200 && responseCode != 202) {
                    removeSubscribeRequest(requestKey);
                }
            }
        };

        private UceSubscribeMessageHandler mNotifyMsgHandler = new UceSubscribeMessageHandler() {
            @Override
            public void onHandle(Message objMessage) {
                int requestKey = objMessage.arg1;
                UceSubscribeRequest request = getSubscribeRequest(requestKey);
                if (request == null) {
                    ImsLog.e(mSlotId, "Do not find request for Key=" + requestKey);
                    return;
                }
                List<String> pidfXmls = (List)objMessage.obj;
                request.informCapabilitiesUpdate(pidfXmls);
            }
        };

        private UceSubscribeMessageHandler mCommandMsgHandler = new UceSubscribeMessageHandler() {
            @Override
            public void onHandle(Message objMessage) {
                int requestKey = objMessage.arg1;
                UceSubscribeRequest request = getSubscribeRequest(requestKey);
                if (request == null) {
                    ImsLog.e(mSlotId, "Do not find request for Key=" + requestKey);
                    return;
                }
                int commandErrorCode = objMessage.arg2;
                request.informCommandError(commandErrorCode);
                removeSubscribeRequest(requestKey);
            }
        };

        private UceSubscribeMessageHandler mTerminatedMsgHandler =
            new UceSubscribeMessageHandler() {
            @Override
            public void onHandle(Message objMessage) {
                int requestKey = objMessage.arg1;
                UceSubscribeRequest request = getSubscribeRequest(requestKey);
                if (request == null) {
                    ImsLog.e(mSlotId, "Do not find request for Key=" + requestKey);
                    return;
                }
                int retryAfterMillsecond = objMessage.arg2;
                Bundle bundle = objMessage.getData();
                String reason = bundle.getString(UceConstant.BUNDLE_GET_REASON);
                request.informTerminate(reason, retryAfterMillsecond);
                removeSubscribeRequest(requestKey);
            }
        };

        private UceSubscribeMessageHandler mResourceTerminatedMsgHandler =
            new UceSubscribeMessageHandler() {
            @Override
            public void onHandle(Message objMessage) {
                int requestKey = objMessage.arg1;
                UceSubscribeRequest request = getSubscribeRequest(requestKey);
                if (request == null) {
                    ImsLog.e(mSlotId, "Do not find request for Key=" + requestKey);
                    return;
                }
                ArrayList<UceResourceInfo> list = (ArrayList)objMessage.obj;
                request.informResourceTerminate(list);
            }
        };
    }

    private void sendCommandError(SubscribeResponse cb, int code) {
        ImsLog.d(mSlotId, "sendCommandError:" + code);
        try {
            cb.onCommandError(code);
        } catch (Exception e) {
            ImsLog.e(mSlotId, "Exception:" + e.toString());
        }
    }

    private UceSubscribeRequest getSubscribeRequest(int key) {
        UceSubscribeRequest request = null;
        try {
            request = mUceSubscribeRequestMap.get(key);
        } catch (ClassCastException e) {
            ImsLog.e(mSlotId, "ClassCastException:" + e.toString());
        } catch (NullPointerException e) {
            ImsLog.e(mSlotId, "NullPointerException:" + e.toString());
        } finally {
            return request;
        }
    }

    private void removeSubscribeRequest(int key) {
        try {
            mUceSubscribeRequestMap.remove(key);
            ImsLog.d(mSlotId, "remove key:" + key);
        } catch (ClassCastException e) {
            ImsLog.e(mSlotId, "ClassCastException:" + e.toString());
        } catch (NullPointerException e) {
            ImsLog.e(mSlotId, "NullPointerException:" + e.toString());
        } catch (UnsupportedOperationException e) {
            ImsLog.e(mSlotId, "UnsupportedOperationException:" + e.toString());
        } finally {
            return;
        }
    }

    @Override
    public void onSubscribeResponseMessage(Parcel parcel) {
        Message msg = Message.obtain();
        int messageType = parcel.readInt();
        switch (messageType) {
            case UceMessage.UCE_SUBSCRIBE_RESPONSE_IND: {
                ImsLog.d(mSlotId, "UCE_SUBSCRIBE_RESPONSE_IND");
                msg.what = UceMessage.UCE_MSG_SUBSCRIBE_RESPONSE;
                int requestKey = parcel.readInt();
                int responseCode = parcel.readInt();
                String reason = parcel.readString();
                int reasonHeaderCause = parcel.readInt();
                String reasonHeaderText = parcel.readString();

                ImsLog.d(mSlotId, "responseCode:" + responseCode + ", reason:" + reason
                        + "reasonHeaderCause" + reasonHeaderCause
                        + "reasonHeaderText:" + reasonHeaderText);

                msg.arg1 = requestKey;
                UceResponseData data = new UceResponseData(responseCode, reason);
                data.setReasonHeaderCause(reasonHeaderCause);
                data.setReasonHeaderText(reasonHeaderText);
                msg.obj = data;
                break;
            }
            case UceMessage.UCE_PRESENCE_NOTIFY_IND: {
                ImsLog.d(mSlotId, "UCE_PRESENCE_NOTIFY_IND");
                msg.what = UceMessage.UCE_MSG_PRESENCE_NOTIFY;
                int requestKey = parcel.readInt();
                int count = parcel.readInt();
                List<String> pidfXmls = new ArrayList<>();
                for (int i = 0; i < count; i++) {
                    String pidfXml = parcel.readString();
                    pidfXmls.add(pidfXml);
                }
                msg.arg1 = requestKey;
                msg.obj = pidfXmls;
                break;
            }
            case UceMessage.UCE_SUBSCRIBE_CMD_ERROR_IND: {
                ImsLog.d(mSlotId, "UCE_SUBSCRIBE_CMD_ERROR_IND");
                msg.what = UceMessage.UCE_MSG_SUBSCRIBE_CMD_ERROR;
                int requestKey = parcel.readInt();
                int commandErrorCode = parcel.readInt();
                msg.arg1 = requestKey;
                msg.arg2 = commandErrorCode;
                break;
            }
            case UceMessage.UCE_SUBSCRIBE_TERMINATED_IND: {
                ImsLog.d(mSlotId, "UCE_SUBSCRIBE_TERMINATED_IND");
                msg.what = UceMessage.UCE_MSG_SUBSCRIBE_TERMINATED;
                int requestKey = parcel.readInt();
                String reason = parcel.readString();
                int retryAfterMillsecond = parcel.readInt();

                msg.arg1 = requestKey;
                msg.arg2 = retryAfterMillsecond;

                Bundle bundle = new Bundle();
                bundle.putString(UceConstant.BUNDLE_GET_REASON, reason);
                msg.setData(bundle);
                break;
            }
            case UceMessage.UCE_SUBSCRIBE_RESOURCE_TERMINATED_IND: {
                ImsLog.d(mSlotId, "UCE_SUBSCRIBE_RESOURCE_TERMINATED_IND");
                msg.what = UceMessage.UCE_MSG_SUBSCRIBE_RESOURCE_TERMINATED;
                int requestKey = parcel.readInt();
                int count = parcel.readInt();
                ArrayList<UceResourceInfo> list = new ArrayList<>();
                for (int i = 0; i < count; i++) {
                    String id = parcel.readString();
                    String reason = parcel.readString();
                    if (!TextUtils.isEmpty(id)) {
                        UceResourceInfo resource = new UceResourceInfo(id, reason);
                        list.add(resource);
                    }
                }
                msg.arg1 = requestKey;
                msg.obj = list;
                break;
            }
            default:
                return;
        }
        mUceSubscribeControllerHandler.sendMessage(msg);
    }
}
