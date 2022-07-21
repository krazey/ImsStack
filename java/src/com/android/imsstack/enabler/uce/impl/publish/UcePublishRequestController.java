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

package com.android.imsstack.enabler.uce.impl.publish;

import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;

import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.define.UceResponseData;
import com.android.imsstack.enabler.uce.impl.define.UceServiceIds;
import com.android.imsstack.enabler.uce.impl.jni.IUceJNIListener;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;
import com.android.imsstack.enabler.uce.impl.utils.UceUtils;
import com.android.imsstack.enabler.uce.interf.PublishResponse;
import com.android.imsstack.enabler.uce.interf.UceApiConstant;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.lang.ref.WeakReference;
import java.util.HashMap;

interface UcePublishMessageHandler {
    public void onHandle(Message objMessage, UcePublishRequestController controller);
}
 /**
  *  The UcePublishRequestController manages the request associated with
  * the UCE publish requests.
  * When the request receives from the caller, a UcePublishRequest instance is created
  * for each request.
  * And the request is complete, the created instance of the UcePublishRequest is deleted.
  */
public class UcePublishRequestController implements IUceJNIListener {
    private final int mSlotId;
    private final UcePublishControllerHandler mUcePublishControllerHandler;

    private boolean mIsImsRegistered;
    private UcePublishRequest mActiveRequest;
    private UcePublishRequest mPendingRequest;
    private long mActiveCapability;
    private long mPendingCapability;
    private UceJNI mUceJNI;

    @VisibleForTesting
    public boolean mIsUseExpiedEtag;

    public UcePublishRequestController(int slotId, Looper looper) {
        mSlotId = slotId;
        mUcePublishControllerHandler = new UcePublishControllerHandler(this, looper);

        mIsImsRegistered = false;
        mActiveRequest = null;
        mPendingRequest = null;
        mActiveCapability = 0L;
        mPendingCapability = 0L;

        mUceJNI = UceJNI.getInstance();
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_PUBLISH_RESPONSE_IND);
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_PUBLISH_CMD_ERROR_IND);

        mIsUseExpiedEtag = false;
    }

    @VisibleForTesting
    public UcePublishRequestController(int slotId, UceJNI jni, Looper looper) {
        mSlotId = slotId;
        mUcePublishControllerHandler = new UcePublishControllerHandler(this, looper);
        mIsImsRegistered = false;
        mActiveRequest = null;
        mPendingRequest = null;
        mActiveCapability = 0L;
        mPendingCapability = 0L;

        jni.addListener(mSlotId, this, UceMessage.UCE_PUBLISH_RESPONSE_IND);
        jni.addListener(mSlotId, this, UceMessage.UCE_PUBLISH_CMD_ERROR_IND);

        mIsUseExpiedEtag = false;
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
        if (!mIsImsRegistered) {
            mActiveCapability = 0;
            mPendingCapability = 0;
            mActiveRequest = null;
            mPendingRequest = null;
        }
    }

    /**
     * Set whether to use e-tag even if it has expired.
     * @param useExpiredEtag set to true if e-tag use even if it has expired, false otherwise.
     */
    public void setUseExpiredEtag(boolean useExpiredEtag) {
        mIsUseExpiedEtag = useExpiredEtag;
        ImsLog.i("mIsUseExpiedEtag:" + mIsUseExpiedEtag);
    }

    /**
     * Set the latest device's capabilities.
     * @param capability the latest device's capabilities
     */
    public void setCapability(long capability) {
        ImsLog.i("setCapability:" + capability);
        mActiveCapability = capability;
    }

    /**
     * Get the latest device's capabilities.
     * @return the latest device`s capabilities
     */
    public long getCapability() {
        ImsLog.i("getCapability:" + mActiveCapability);
        return mActiveCapability;
    }
    /**
     * The capabilities of this device have been updated and should be published to the network.
     * If this operation succeeds, network response updates should be sent to the framework using
     * {@link PublishResponse#onNetworkResponse(int, String)}.
     * @param pidfXml The XML PIDF document containing the capabilities of this device to be sent
     * to the carrier’s presence server.
     * @param cb The callback of the publish request
     */
    public void publishCapabilities(String pidfXml, PublishResponse cb) {
        if (!mIsImsRegistered)  {
            sendCommandError(cb, UceApiConstant.COMMAND_CODE_SERVICE_UNAVAILABLE);
            return;
        }
        long capability = getCapability(pidfXml);
        ImsLog.d("publishCapabilities:mActiveCapability=" + mActiveCapability + ", newCapability=" +
                capability);

        if (mActiveCapability == capability) {
            sendCommandError(cb, UceApiConstant.COMMAND_CODE_NO_CHANGE);
            return;
        }

        int key = UceUtils.generateKey();
        UcePublishRequest request = new UcePublishRequest(cb, mSlotId, key, mIsUseExpiedEtag);

        publishCapabilities(pidfXml, cb, request);
    }

    @VisibleForTesting
    public void publishCapabilities(String pidfXml, PublishResponse cb, UcePublishRequest active) {
        if (!mIsImsRegistered)  {
            sendCommandError(cb, UceApiConstant.COMMAND_CODE_SERVICE_UNAVAILABLE);
            active = null;
            return;
        }
        long capability = getCapability(pidfXml);
        ImsLog.d("publishCapabilities:mActiveCapability=" + mActiveCapability + ", newCapability=" +
                capability);

        if (mActiveCapability == capability) {
            sendCommandError(cb, UceApiConstant.COMMAND_CODE_NO_CHANGE);
            active = null;
            return;
        }

        active.setRequestInfo(pidfXml, isExtended(capability), capability);

        if (mActiveRequest == null) {
            mActiveRequest = active;
            if (mActiveRequest.sendRequest() == false) {
                mActiveRequest = null;
            } else {
                mActiveCapability = capability;
            }
        } else {
            ImsLog.d("publishCapabilities:set pending capability=" + capability);
            mPendingRequest = active;
            mPendingCapability = capability;
        }
    }

    /**
     * If there is a pending publish request, an error command is sent and it is deleted.
     */
    public void deletePendingRequest() {
        if (mPendingRequest == null) {
            return;
        }
        mPendingRequest.informCommandError(UceApiConstant.COMMAND_CODE_SERVICE_UNAVAILABLE);
        mPendingRequest = null;
        mPendingCapability = 0;
    }

    /**
     * If there is a pending publish request, send a request.
     */
    public void handlePendingRequest() {
        if (mPendingRequest == null) {
            return;
        }
        if (mPendingRequest.sendRequest() == true) {
            mActiveRequest = mPendingRequest;
            mActiveCapability = mPendingCapability;
        }
        mPendingRequest = null;
        mPendingCapability = 0;
    }

    /**
     * Converts input pidf xml to a long value.
     * @param pidfXml The XML PIDF document containing the capabilities of this device to be sent
     * to the carrier’s presence server.
     * @return a long value as a converted capabilities.
     */
    public long getCapability(String pidfXml) {
        long newCapability = UceServiceIds.SERVICE_ID_NONE;
        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_PRESENCE.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_PRESENCE;
            ImsLog.d("presence");
        }
        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_IPCALL.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_IPCALL_VOICE;

            String mmtel = pidfXml.substring(pidfXml.indexOf(
                    UceServiceIds.ServiceIds.SERVICE_ID_IPCALL.getId()));
            if (mmtel.contains("<caps:video>")) {
                String video = mmtel.substring(mmtel.indexOf("<caps:video>"));
                String videoValue = video.substring("<caps:video>".length(),
                        video.indexOf("</caps:video>"));
                if (videoValue.equals("true")) {
                    newCapability |= UceServiceIds.SERVICE_ID_IPCALL_VIDEO;
                    ImsLog.d("ip vioeo call");
                }
            }
        }
        newCapability |= getCapabilitiesAssociatedWithMessage(pidfXml);
        newCapability |= getCapabilitiesAssociatedWithFT(pidfXml);
        newCapability |= getCapabilitiesAssociatedWithChatbot(pidfXml);
        newCapability |= getCapabilitiesAssociatedWithCallComposer(pidfXml);

        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_GEOLOCATION_PUSH.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_GEOLOCATION_PUSH;
            ImsLog.d("geolocation push");
        }
        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_SHARED_MAP.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_SHARED_MAP;
            ImsLog.d("shared map");
        }
        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_SHARED_SKETCH.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_SHARED_SKETCH;
            ImsLog.d("shared sketch");
        }
        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_POST_CALL.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_POST_CALL;
            ImsLog.d("post call");
        }
        if (pidfXml.contains(
                UceServiceIds.ServiceIds.SERVICE_ID_GEOLOCATION_PUSH_VIA_SMS.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_GEOLOCATION_PUSH_VIA_SMS;
            ImsLog.d("geolocation push via SMS");
        }
        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_FILE_TRANSFER_VIA_SMS.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_FILE_TRANSFER_VIA_SMS;
            ImsLog.d("file transfer via SMS");
        }
        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_CANCEL_MESSAGE.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_CANCEL_MESSAGE;
            ImsLog.d("cancel message");
        }
        return newCapability;
    }

    @VisibleForTesting
    public void setActiveRequest(UcePublishRequest request) {
        mActiveRequest = request;
    }

    @VisibleForTesting
    public UcePublishRequest getActiveRequest() {
        return mActiveRequest;
    }

    @VisibleForTesting
    public void setPendingRequest(UcePublishRequest request) {
        mPendingRequest = request;
    }

    @VisibleForTesting
    public UcePublishRequest getPendingRequest() {
        return mPendingRequest;
    }

    @VisibleForTesting
    public long getPendingCapability() {
        return mPendingCapability;
    }

    @VisibleForTesting
    public Handler getHandler() {
        return mUcePublishControllerHandler;
    }

    private long getCapabilitiesAssociatedWithMessage(String pidfXml) {
        long newCapability = UceServiceIds.SERVICE_ID_NONE;

        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_STANDALONE_MESSAGING.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_STANDALONE_MESSAGING;
            ImsLog.d("standalone message");
        }
        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_CHAT_SESSION.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_CHAT_SESSION;
            ImsLog.d("chat session");
        }
        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_IM_SESSION.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_IM_SESSION;
            ImsLog.d("im session");
        }
        if (pidfXml.contains(
                UceServiceIds.ServiceIds.SERVICE_ID_FULL_STORE_FORWARD_GROUP_CHAT.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_FULL_STORE_FORWARD_GROUP_CHAT;
            ImsLog.d("full store forward group chat");
        }
        return newCapability;
    }

    private long getCapabilitiesAssociatedWithFT(String pidfXml) {
        long newCapability = UceServiceIds.SERVICE_ID_NONE;
        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_FILE_TRANSFER_THUMBNAIL.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_FILE_TRANSFER_THUMBNAIL;
        }
        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_FILE_TRANSFER_HTTP.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_FILE_TRANSFER_HTTP;
            ImsLog.d("file transfer HTTP");
        }
        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_FILE_TRANSFER.getId())) {
            String fileTransfer = pidfXml.substring(pidfXml.indexOf(
                    UceServiceIds.ServiceIds.SERVICE_ID_FILE_TRANSFER.getId()));
            String serviceId = fileTransfer.substring(0, fileTransfer.indexOf("</op:service-id>"));
            if (serviceId.equalsIgnoreCase(
                    UceServiceIds.ServiceIds.SERVICE_ID_FILE_TRANSFER.getId())) {
                String version = fileTransfer.substring(fileTransfer.indexOf("<op:version>"));
                String versionValue = version.substring("<op:version>".length(),
                        version.indexOf("</op:version>"));

                if (versionValue.equals(
                        UceServiceIds.ServiceIds.SERVICE_ID_FILE_TRANSFER.getVersion())) {
                    newCapability |= UceServiceIds.SERVICE_ID_FILE_TRANSFER;
                    ImsLog.d("file transfer");
                }
                String FileTransferStoreForwardVersion =
                        UceServiceIds.ServiceIds.SERVICE_ID_FT_STORE_FORWARD.getVersion();
                if (versionValue.equals(FileTransferStoreForwardVersion)) {
                    newCapability |= UceServiceIds.SERVICE_ID_FILE_TRANSFER_STORE_FORWARD;
                    ImsLog.d("file transfer store forward");
                }
            }
        }
        return newCapability;
    }

    private long getCapabilitiesAssociatedWithChatbot(String pidfXml) {
        long newCapability = UceServiceIds.SERVICE_ID_NONE;

        if (pidfXml.contains(
                UceServiceIds.ServiceIds.SERVICE_ID_CHATBOT_COMMUNICATION_SESSION.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_CHATBOT_COMMUNICATION_SESSION;
            ImsLog.d("chatbot communication session");
        }
        if (pidfXml.contains(
                UceServiceIds.ServiceIds.SERVICE_ID_CHATBOT_STADNALONE_MESSAGING.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_CHATBOT_STADNALONE_MESSAGING;
            ImsLog.d("chatbot standalone message");
        }
        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_CHATBOT_EXTEND_MESSAGE.getId())) {
            newCapability |= UceServiceIds.SERVICE_ID_CHATBOT_EXTEND_MESSAGE;
            ImsLog.d("chatbot extend message");
        }
        return newCapability;
    }

    private long getCapabilitiesAssociatedWithCallComposer(String pidfXml) {
        long newCapability = UceServiceIds.SERVICE_ID_NONE;

        if (pidfXml.contains(UceServiceIds.ServiceIds.SERVICE_ID_CALL_COMPOSER_V1.getId())) {
            String callcomposer = pidfXml.substring(pidfXml.indexOf(
                    UceServiceIds.ServiceIds.SERVICE_ID_CALL_COMPOSER_V1.getId()));

            String version = callcomposer.substring(callcomposer.indexOf("<op:version>"));
            String versionValue = version.substring("<op:version>".length(),
                    version.indexOf("</op:version>"));

            if (versionValue.equals(
                    UceServiceIds.ServiceIds.SERVICE_ID_CALL_COMPOSER_V1.getVersion())) {
                newCapability |= UceServiceIds.SERVICE_ID_CALL_COMPOSER_V1;
                ImsLog.d("call composer V1");
            }
            if (versionValue.equals(
                    UceServiceIds.ServiceIds.SERVICE_ID_CALL_COMPOSER_V2.getVersion())) {
                newCapability |= UceServiceIds.SERVICE_ID_CALL_COMPOSER_V2;
                ImsLog.d("call composer V2");
            }
        }
        return newCapability;
    }

    private void sendCommandError(PublishResponse cb, int code) {
        ImsLog.d("sendCommandError:" + code);
        try {
            cb.onCommandError(code);
        } catch (Exception e) {
            ImsLog.e("Exception:" + e.toString());
        }
    }

    private boolean isExtended(long capability) {
        if (((capability & UceServiceIds.SERVICE_ID_IPCALL_VOICE) ==
                UceServiceIds.SERVICE_ID_IPCALL_VOICE &&
                (capability & UceServiceIds.SERVICE_ID_IPCALL_VIDEO) ==
                        UceServiceIds.SERVICE_ID_IPCALL_VIDEO) ||
                (capability & UceServiceIds.SERVICE_ID_STANDALONE_MESSAGING) ==
                        UceServiceIds.SERVICE_ID_STANDALONE_MESSAGING ||
                (capability & UceServiceIds.SERVICE_ID_CHAT_SESSION) ==
                        UceServiceIds.SERVICE_ID_CHAT_SESSION ||
                (capability & UceServiceIds.SERVICE_ID_FILE_TRANSFER) ==
                        UceServiceIds.SERVICE_ID_FILE_TRANSFER) {
            return true;
        }
        return false;
    }

    private void deleteActiveRequest() {
        mActiveRequest = null;
        mActiveCapability = 0;
    }

    class UcePublishControllerHandler extends Handler {
        private final WeakReference<UcePublishRequestController> mUcePublishControllerRef;
        private HashMap<Integer, UcePublishMessageHandler> mMessageHandler =
                new HashMap<Integer, UcePublishMessageHandler>();

        public UcePublishControllerHandler(UcePublishRequestController controller, Looper looper) {
            super(looper);
            mMessageHandler.put(UceMessage.UCE_MSG_PUBLISH_RESPONSE, mResponseMsgHandler);
            mMessageHandler.put(UceMessage.UCE_MSG_PUBLISH_CMD_ERROR, mCommandMsgHandler);
            mUcePublishControllerRef = new WeakReference<>(controller);
        }
        @Override
        public void handleMessage(Message objMessage) {
            if (objMessage == null) {
                ImsLog.e("Message is null");
                return;
            }
            UcePublishRequestController controller = mUcePublishControllerRef.get();
            if (controller == null) {
                return;
            }
            UcePublishMessageHandler objMsgHandler = mMessageHandler.get(objMessage.what);
            if (objMsgHandler == null) {
                ImsLog.e("message can not be handled.");
                return;
            }
            objMsgHandler.onHandle(objMessage, controller);
        }

        private UcePublishMessageHandler mResponseMsgHandler = new UcePublishMessageHandler() {
            @Override
            public void onHandle(Message objMessage, UcePublishRequestController controller) {
                int requestKey = objMessage.arg1;
                UcePublishRequest request = controller.getActiveRequest();
                if (request == null || request.getKey() != requestKey) {
                    ImsLog.e("Do not find request for Key=" + requestKey);
                    return;
                }
                UceResponseData data = (UceResponseData) objMessage.obj;
                int responseCode = data.getResponseCode();
                String reason = data.getReason();
                int reasonHeaderCause = data.getReasonHeaderCause();
                String reasonHeaderText = data.getReasonHeaderText();
                String etag = data.getEtag();
                long capability = data.getCapability();
                int needToRetry = data.getNeedToRetry();
                request.informNetworkResponse(responseCode, reason, reasonHeaderCause,
                        reasonHeaderText, etag);
                controller.deleteActiveRequest();
                controller.setCapability(capability);
                controller.handlePendingRequest();
            }
        };
        private UcePublishMessageHandler mCommandMsgHandler = new UcePublishMessageHandler() {
            @Override
            public void onHandle(Message objMessage, UcePublishRequestController controller) {
                int requestKey = objMessage.arg1;
                UcePublishRequest request = controller.getActiveRequest();
                if (request == null || request.getKey() != requestKey) {
                    ImsLog.e("Do not find request for Key=" + requestKey);
                    return;
                }
                int commandErrorCode = objMessage.arg2;
                request.informCommandError(commandErrorCode);
                controller.deleteActiveRequest();
                controller.handlePendingRequest();
            }
        };
    }

    @Override
    public void onPublishResponseMessage(Parcel parcel) {
        ImsLog.d("");
        Message msg = Message.obtain();

        int messageType = parcel.readInt();
        if (messageType == UceMessage.UCE_PUBLISH_RESPONSE_IND) {
            msg.what = UceMessage.UCE_MSG_PUBLISH_RESPONSE;
            int requestKey = parcel.readInt();
            long capability = parcel.readLong();
            int responseCode = parcel.readInt();
            String reason = parcel.readString();
            int reasonHeaderCause = parcel.readInt();
            String reasonHeaderText = parcel.readString();
            String etag = parcel.readString();
            int needToRetry = parcel.readInt();

            ImsLog.d("response:" + responseCode + ", reason:" + reason + ", reasonHeaderCause:" +
                reasonHeaderCause + ", reasonHeaderText:" + reasonHeaderText + ", etag:" + etag +
                ", needToRetry:" + needToRetry);

            msg.arg1 = requestKey;
            UceResponseData data = new UceResponseData(responseCode, reason);
            data.setReasonHeaderCause(reasonHeaderCause);
            data.setReasonHeaderText(reasonHeaderText);
            data.setEtag(etag);
            data.setCapability(capability);
            data.setNeedToRetry(needToRetry);
            msg.obj = data;
        } else { // UceMessage.UCE_PUBLISH_CMD_ERROR_IND)
            msg.what = UceMessage.UCE_MSG_PUBLISH_CMD_ERROR;
            int requestKey = parcel.readInt();
            int commandErrorCode = parcel.readInt();
            msg.arg1 = requestKey;
            msg.arg2 = commandErrorCode;
        }
        mUcePublishControllerHandler.sendMessage(msg);
    }
}
