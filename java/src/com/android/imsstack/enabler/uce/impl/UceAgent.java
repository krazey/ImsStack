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

package com.android.imsstack.enabler.uce.impl;

import android.net.Uri;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.os.Parcel;
import android.text.TextUtils;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.PreferenceInterface;
import com.android.imsstack.enabler.uce.impl.configuration.UceConfiguration;
import com.android.imsstack.enabler.uce.impl.define.UceConstant;
import com.android.imsstack.enabler.uce.impl.define.UceMessage;
import com.android.imsstack.enabler.uce.impl.define.UceOptionsReceivedData;
import com.android.imsstack.enabler.uce.impl.define.UceResponseData;
import com.android.imsstack.enabler.uce.impl.jni.IUceJNIListener;
import com.android.imsstack.enabler.uce.impl.jni.UceJNI;
import com.android.imsstack.enabler.uce.impl.options.UceOptionsResponseCallback;
import com.android.imsstack.enabler.uce.impl.publish.UcePublishRequestController;
import com.android.imsstack.enabler.uce.impl.subscribe.UceSubscribeRequestController;
import com.android.imsstack.enabler.uce.impl.utils.UceUtils;
import com.android.imsstack.enabler.uce.interf.OptionsResponse;
import com.android.imsstack.enabler.uce.interf.PublishResponse;
import com.android.imsstack.enabler.uce.interf.SubscribeResponse;
import com.android.imsstack.enabler.uce.interf.UceApiConstant;
import com.android.imsstack.enabler.uce.interf.UceEventListener;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import java.util.Collection;
import java.util.HashMap;
import java.util.Set;

interface UceAgentMessageHandler {
    public void onHandle(Message objMessage);
}

public class UceAgent extends Thread implements IUceJNIListener {
    private final int mSlotId;

    private final HashMap<Integer, UceAgentMessageHandler> mMessageHandler =
        new HashMap<Integer, UceAgentMessageHandler>();
    private UceAgentHandler mUceAgentHandler = null;

    private boolean mImsRegistered;
    private long mConnectedServices;

    private UceEventListener mUceEventListener;
    private UcePublishRequestController mUcePublishRequestController;
    private UceSubscribeRequestController mUceSubscribeRequestController;
    //private UceOptionsRequestController mUceOptionsRequestController;
    private UceConfiguration mUceConfiguration;

    private int mRegistrationTech = UceConstant.RADIO_TECHNOLOGY_TYPE_UNKNOWN;

    Looper mLoop = null;

    private UceJNI mUceJNI;
    private PreferenceInterface mPf;

    public UceAgent(String name, int nSimSlot) {
        this(name, nSimSlot, UceJNI.getInstance());
    }

    @VisibleForTesting
    public UceAgent(String name, int nSimSlot, UceJNI jni) {
        super(name);
        mSlotId = nSimSlot;
        mUceEventListener = null;
        mUceJNI = jni;
        mUceJNI.init(mSlotId);
    }

    public void run() {
        ImsLog.i(mSlotId, "run()");
        Looper.prepare();
        mLoop = Looper.myLooper();
        mUceAgentHandler = new UceAgentHandler(mLoop);
        initialize();
        Looper.loop();
        deInitialize();
    }

    /**
     * Sets the listener which can expect to change the publish state or to receive a options
     * request from after this point.
     * @param listener the {@link UceEventListener} to check for a publish state changed or to
     *                 notify of received options
     */
    public void setListener(UceEventListener listener) {
        mUceEventListener = listener;
        if (mImsRegistered && mUceEventListener != null) {
            int capaTriggerType = getCapabilityUpdateTriggerType(mRegistrationTech);
            try {
                ImsLog.d(mSlotId, "call onRequestPublishCapabilities() with " + capaTriggerType);
                mUceEventListener.onRequestPublishCapabilities(capaTriggerType);
            } catch (Exception e) {
                ImsLog.e(mSlotId, "Exception:" + e.toString());
            }
        }
    }

    /**
     * When the carrier config is changed, it will be call.
     */
    public void carrierConfigChanged() {
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
        if (mUcePublishRequestController == null) {
            ImsLog.d(mSlotId, "mUcePublishRequestController is null");
            try {
                cb.onCommandError(UceApiConstant.COMMAND_CODE_SERVICE_UNKNOWN);
            } catch(Exception e) {
                ImsLog.e(mSlotId, "Exception:" + e.toString());
            }
            return;
        }
        mUcePublishRequestController.publishCapabilities(pidfXml, cb);
    }

    /**
     * The user capabilities of one or multiple contacts have been requested by the framework.
     * <p>
     * The implementer must follow up this call with an
     * {@link SubscribeResponse#onCommandError(int)} call to indicate this operation has failed.
     * The response from the network to the SUBSCRIBE request must be sent back to the framework
     * using {@link SubscribeResponse#onNetworkResponse(int, String)}.
     * As NOTIFY requests come in from the network, the requested contact’s capabilities should be
     * sent back to the framework using
     * {@link SubscribeResponse#onNotifyCapabilitiesUpdate(List<String>)} and
     * {@link SubscribeResponse#onResourceTerminated(List<Pair<Uri, String>>)}
     * should be called with the presence information for the contacts specified.
     * <p>
     * Once the subscription is terminated,
     * {@link SubscribeResponse#onTerminated(String, long)} must be called for the
     * framework to finish listening for NOTIFY responses.
     *
     * @param uris A {@link Collection} of the {@link Uri}s that the framework is requesting the
     * UCE capabilities for.
     * @param cb The callback of the subscribe request.
     */
    public void subscribeCapabilities(Collection<Uri> uris, SubscribeResponse cb) {
        if (mUceSubscribeRequestController == null) {
            ImsLog.i(mSlotId, "mUceSubscribeRequestController is null");
            try {
                cb.onCommandError(UceApiConstant.COMMAND_CODE_SERVICE_UNKNOWN);
            } catch(Exception e) {
                ImsLog.e(mSlotId, "Exception:" + e.toString());
            }
            return;
        }
        mUceSubscribeRequestController.subscribeCapabilities(uris, cb);
    }

    /**
     * Push one's own capabilities to a remote user via the SIP OPTIONS presence exchange mechanism
     * in order to receive the capabilities of the remote user in response.
     * <p>
     * The implementer must use {@link OptionsResponse} to send the response of
     * this query from the network back to the framework.
     * @param contactUri The URI of the remote user that we wish to get the capabilities of.
     * @param myCapabilities The capabilities of this device to send to the remote user.
     * @param cb The callback of this request which is sent from the remote user.
     */
    public void sendOptionsCapabilityRequest(Uri contactUri, Set<String> myCapabilities,
        OptionsResponse cb) {
        /*
        if (mUceOptionsRequestController == null) {
            ImsLog.i(mSlotId, "mUceOptionsRequestController is null");
            try {
                cb.onCommandError(UceApiConstant.COMMAND_CODE_SERVICE_UNKNOWN);
            } catch(Exception e) {
                ImsLog.e(mSlotId, "Exception:" + e.toString());
            }
            return;
        }
        mUceOptionsRequestController.sendOptionsCapabilityRequest(contactUri, myCapabilities, cb);
         */
    }

    public void release() {
        if (mLoop != null) {
            mLoop.quit();
        }
    }

    @VisibleForTesting
    public void setImsRegistered(boolean imsRegistered) {
        mImsRegistered = imsRegistered;
    }

    @VisibleForTesting
    public void setPublishController(UcePublishRequestController controller) {
        mUcePublishRequestController = controller;
    }

    @VisibleForTesting
    public void setSubscribeController(UceSubscribeRequestController controller) {
        mUceSubscribeRequestController = controller;
    }

    @VisibleForTesting
    public Handler getHandler() {
        return mUceAgentHandler;
    }

    /**
     * Convert registration tech to the capability
     * @param currentRegiTech current registered tech.
     * @return the capability value that matches the tech value.
     */
    @VisibleForTesting
    public int getCapabilityUpdateTriggerType(int currentRegiTech) {
        int type = UceApiConstant.CAPABILITY_UPDATE_TRIGGER_UNKNOWN;
        switch (currentRegiTech) {
            case UceConstant.RADIO_TECHNOLOGY_TYPE_GERAN:
                type = UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_2G;
                break;
            case UceConstant.RADIO_TECHNOLOGY_TYPE_HRPD:
            case UceConstant.RADIO_TECHNOLOGY_TYPE_UTRAN:
                type = UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_3G;
                break;
            case UceConstant.RADIO_TECHNOLOGY_TYPE_EHRPD:
                type = UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_EHRPD;
                break;
            case UceConstant.RADIO_TECHNOLOGY_TYPE_LTE:
                type = UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_ENABLED;
                break;
            case UceConstant.RADIO_TECHNOLOGY_TYPE_LTE_NO_VOPS:
                type = UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_LTE_VOPS_DISABLED;
                break;
            case UceConstant.RADIO_TECHNOLOGY_TYPE_WIFI:
                type = UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_IWLAN;
                break;
            case UceConstant.RADIO_TECHNOLOGY_TYPE_NR:
                type = UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_NR5G_VOPS_ENABLED;
                break;
            case UceConstant.RADIO_TECHNOLOGY_TYPE_NR_NO_VOPS:
                type = UceApiConstant.CAPABILITY_UPDATE_TRIGGER_MOVE_TO_NR5G_VOPS_DISABLED;
                break;
            default:
                break;
        }
        return type;
    }

    private void initialize() {
        ImsLog.d(mSlotId, "initialize");
        mUcePublishRequestController = new UcePublishRequestController(mSlotId, mLoop);
        mUceSubscribeRequestController = new UceSubscribeRequestController(mSlotId, mLoop);
        //mUceOptionsRequestController = new UceOptionsRequestController(mSlotId, mLoop);

        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_IMS_AGENT_CONNECTED_IND);
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_IMS_AGENT_DISCONNECTED_IND);
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_IMS_AGENT_REFRESHED_IND);
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_NETWORK_CHANGED);
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_UNPUBLISHED_IND);
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_PUBLISH_UPDATED_IND);
        mUceJNI.addListener(mSlotId, this, UceMessage.UCE_OPTIONS_RECEIVED_IND);

        mRegistrationTech = UceConstant.RADIO_TECHNOLOGY_TYPE_UNKNOWN;
        mImsRegistered = false;
        mConnectedServices = 0;

        mPf = AgentFactory.getInstance().getAgent(PreferenceInterface.class);
        mUceConfiguration = new UceConfiguration(mSlotId);
        mUceConfiguration.init();
        imsRegistrationStatusCheck();
    }

    private void deInitialize() {
        ImsLog.d(mSlotId, "deInitialize");
        mUceConfiguration = null;
        mUcePublishRequestController = null;
        mUceSubscribeRequestController = null;
        mImsRegistered = false;
        mConnectedServices = 0;
        mUceJNI.release(mSlotId);
    }

    class UceAgentHandler extends Handler {
        UceAgentHandler(Looper looper) {
            super(looper);
            /* speicial Message handler */
            // associated with Always on Service
            mMessageHandler.put(UceMessage.UCE_MSG_IMS_AGENT_CONNECTED, mUceAgentMsgHandler);
            mMessageHandler.put(UceMessage.UCE_MSG_IMS_AGENT_DISCONNECTED, mUceAgentMsgHandler);
            mMessageHandler.put(UceMessage.UCE_MSG_IMS_AGENT_REFRESHED, mUceAgentMsgHandler);
            // connectivity
            mMessageHandler.put(UceMessage.UCE_MSG_NETWORK_CHANGED, mUceConnetivityMsgHandler);
            // publish status
            mMessageHandler.put(UceMessage.UCE_MSG_UNPUBLISHED, mUcePublishChangedMsgHandler);
            mMessageHandler.put(UceMessage.UCE_MSG_PUBLISH_UPDATED, mUcePublishChangedMsgHandler);
            // options status
            mMessageHandler.put(UceMessage.UCE_MSG_OPTIONS_RECEIVED, mUceOptionsReceivedMsgHandler);
        }

        @Override
        public void handleMessage(Message objMessage) {
            if (objMessage == null) {
                ImsLog.e(mSlotId, "Message is null");
                return;
            }
            ImsLog.d(mSlotId, "slot id:" + mSlotId);
            UceAgentMessageHandler objMsgHandler = mMessageHandler.get(objMessage.what);
            if (objMsgHandler == null) {
                ImsLog.e(mSlotId, "message can not be handled.");
                return;
            }
            objMsgHandler.onHandle(objMessage);
        }

        private UceAgentMessageHandler mUceAgentMsgHandler = new UceAgentMessageHandler() {
            @Override
            public void onHandle(Message objMessage) {
                switch (objMessage.what) {
                    /* associated with Always on Service */
                    case UceMessage.UCE_MSG_IMS_AGENT_CONNECTED: {
                        ImsLog.d(mSlotId, "handle : UCE_MSG_IMS_AGENT_CONNECTED");
                        imsRegistered(objMessage.arg1, (long) objMessage.obj);
                    }
                    break;
                    case UceMessage.UCE_MSG_IMS_AGENT_DISCONNECTED: {
                        ImsLog.d(mSlotId, "handle : UCE_MSG_IMS_AGENT_DISCONNECTED");
                        imsDeregistered();
                    }
                    break;
                    /*
                    case UceMessage.UCE_MSG_IMS_AGENT_REFRESHED:
                    {
                        imsRegistered(objMessage.arg1, (long)objMessage.obj);
                    }
                    break;
                     */
                    default:
                        break;
                }
            }
        };
        private UceAgentMessageHandler mUceConnetivityMsgHandler = new UceAgentMessageHandler() {
            @Override
            public void onHandle(Message objMessage) {
                if (!mImsRegistered) {
                    ImsLog.i(mSlotId, "network changed but ims did not registered.");
                    return;
                }
                int networkType = objMessage.arg1;
                if (mRegistrationTech == networkType || mUceEventListener == null) {
                    ImsLog.i(mSlotId, "network type didn`t changed or listener is null");
                    return;
                }
                ImsLog.d(mSlotId, "handle : UCE_MSG_NETWORK_CHANGED");
                int capaTriggerType = getCapabilityUpdateTriggerType(networkType);
                try {
                    ImsLog.d(mSlotId, "call onRequestPublishCapabilities() with "
                            + capaTriggerType);
                    mUceEventListener.onRequestPublishCapabilities(capaTriggerType);
                } catch (Exception e) {
                    ImsLog.e(mSlotId, "Exception:" + e.toString());
                }
            }
        };
        private UceAgentMessageHandler mUcePublishChangedMsgHandler = new UceAgentMessageHandler() {
            @Override
            public void onHandle(Message objMessage) {
                if (mUceEventListener == null) {
                    ImsLog.i(mSlotId, "mUceEventListener is null");
                    return;
                }
                if (objMessage.what == UceMessage.UCE_MSG_PUBLISH_UPDATED) {
                    ImsLog.d(mSlotId, "handle : UCE_MSG_PUBLISH_UPDATED");
                    UceResponseData data = (UceResponseData)objMessage.obj;
                    int responseCode = data.getResponseCode();
                    String reason = data.getReason();
                    int reasonHdrCause = data.getReasonHeaderCause();
                    String reasonHdrText = data.getReasonHeaderText();
                    String eTag = data.getEtag();
                    int needToRetry = data.getNeedToRetry();
                    if (needToRetry == 0) {
                        mUcePublishRequestController.setCapability(data.getCapability());
                        mUcePublishRequestController.handlePendingRequest();
                    }
                    ImsLog.d(mSlotId, "responseCode:" + responseCode + ", reason:" + reason
                            + "reasonHeaderCause" + reasonHdrCause + "reasonHeaderText:"
                            + reasonHdrText);
                    try {
                        ImsLog.d(mSlotId, "call onPublishUpdated() with " + responseCode);
                        mUceEventListener.onPublishUpdated(responseCode, reason, reasonHdrCause,
                            reasonHdrText);
                    } catch (Exception e) {
                        ImsLog.e(mSlotId, "Exception:" + e.toString());
                    }
                    if (!TextUtils.isEmpty(eTag)) {
                        if (mPf != null) {
                            mPf.putString(UceConstant.PREFERENCE_ETAG, eTag, mSlotId);
                        }
                    }
                } else { // UceMessage.UCE_UNPUBLISHED_IND
                    ImsLog.d(mSlotId, "handle : UCE_UNPUBLISHED_IND");
                    mUcePublishRequestController.deletePendingRequest();
                    try {
                        ImsLog.d(mSlotId, "call onUnPublish()");
                        mUceEventListener.onUnPublish();
                    } catch (Exception e) {
                        ImsLog.e(mSlotId, "Exception:" + e.toString());
                    }
                    if (mPf != null) {
                        mPf.putString(UceConstant.PREFERENCE_ETAG, "", mSlotId);
                    }
                }
            }
        };
        private UceAgentMessageHandler mUceOptionsReceivedMsgHandler =
                new UceAgentMessageHandler() {
            @Override
            public void onHandle(Message objMessage) {
                if (mUceEventListener == null) {
                    ImsLog.i(mSlotId, "mUceEventListener is null");
                    return;
                }
                ImsLog.d(mSlotId, "handle : UCE_MSG_OPTIONS_RECEIVED");
                int requestKey = objMessage.arg1;
                UceOptionsReceivedData data = (UceOptionsReceivedData)objMessage.obj;
                Uri contactUri = Uri.parse(data.getRemoteUri());
                Set<String> remoteCapabilities = UceUtils.getFeatureTags(data.getRemoteCaps());
                UceOptionsResponseCallback callback = new UceOptionsResponseCallback(requestKey,
                        mSlotId);
                try {
                    ImsLog.d(mSlotId, "call onRemoteCapabilityRequest()");
                    mUceEventListener.onRemoteCapabilityRequest(contactUri, remoteCapabilities,
                            callback);
                } catch (Exception e) {
                    ImsLog.e(mSlotId, "Exception:" + e.toString());
                }
            }
        };
    }

    private void imsRegistered(int registrationTech, long connectedServices) {
        mUcePublishRequestController.setImsRegistrationStatus(true);
        mUcePublishRequestController.setUseExpiredEtag(mUceConfiguration.isUseExpiredEtag());

        mUceSubscribeRequestController.setImsRegistrationStatus(true);
        if ((registrationTech == mRegistrationTech) && (mConnectedServices == connectedServices)) {
            return;
        }
        int capaTriggerType = getCapabilityUpdateTriggerType(registrationTech);
        mRegistrationTech = registrationTech;
        mConnectedServices = connectedServices;
        if (mUceEventListener != null) {
            try {
                ImsLog.d(mSlotId, "call onRequestPublishCapabilities() with " + capaTriggerType);
                mUceEventListener.onRequestPublishCapabilities(capaTriggerType);
            } catch (Exception e) {
                ImsLog.e(mSlotId, "Exception:" + e.toString());
            }
        }
    }

    private void imsDeregistered() {
        mRegistrationTech = UceConstant.RADIO_TECHNOLOGY_TYPE_UNKNOWN;
        mConnectedServices = 0;
        mUcePublishRequestController.setImsRegistrationStatus(false);
        mUceSubscribeRequestController.setImsRegistrationStatus(false);
    }

    private void imsRegistrationStatusCheck() {
        ImsLog.d(mSlotId, "imsRegistrationStatusCheck");
        if (mImsRegistered) {
            return;
        }
        Parcel parcel = Parcel.obtain();

        parcel.writeInt(UceMessage.UCE_GET_IMS_REGISTRATION_CMD);
        mUceJNI.sendMessage(mSlotId, parcel);
    }

    @Override
    public void onServiceStatusMessage(Parcel parcel) {
        Message msg = Message.obtain();
        int messageType = parcel.readInt();
        ImsLog.d(mSlotId, "messageType:" + messageType);

        if (messageType == UceMessage.UCE_IMS_AGENT_CONNECTED_IND) {
            mImsRegistered = true;
            msg.what = UceMessage.UCE_MSG_IMS_AGENT_CONNECTED;
        } else if (messageType == UceMessage.UCE_IMS_AGENT_DISCONNECTED_IND) {
            mImsRegistered = false;
            msg.what = UceMessage.UCE_MSG_IMS_AGENT_DISCONNECTED;
        }/*
        else if (messageType == UceMessage.UCE_IMS_AGENT_REFRESHED_IND) {
            mImsRegistered = true;
            msg.what = UceMessage.UCE_MSG_IMS_AGENT_REFRESHED;
            return;
        }
        */
        ImsLog.i(mSlotId, "Registration Status : " + mImsRegistered);

        if (mImsRegistered) {
            int mRadioTechType = parcel.readInt();
            ImsLog.d(mSlotId, "IMS registered");
            msg.arg1 = mRadioTechType;
            msg.obj = Long.valueOf(parcel.readLong());
        } else {
            ImsLog.d(mSlotId, "IMS de-registered");
        }
        mUceAgentHandler.sendMessage(msg);
    }

    @Override
    public void onNetworkStatusMessage(Parcel parcel) {
        ImsLog.d(mSlotId, "");
        int messageType = parcel.readInt();
        int networkType = parcel.readInt();
        if (messageType == UceMessage.UCE_NETWORK_CHANGED) {
            ImsLog.i(mSlotId, "network type:" + networkType);
            Message msg = Message.obtain();
            msg.what = UceMessage.UCE_MSG_NETWORK_CHANGED;
            msg.arg1 = networkType;
            mUceAgentHandler.sendMessage(msg);
        }
    }

    @Override
    public void onReceivedRemoteOptionsMessage(Parcel parcel) {
        int messageType = parcel.readInt();
        if (messageType == UceMessage.UCE_OPTIONS_RECEIVED_IND) {
            Message msg = Message.obtain();
            msg.what = UceMessage.UCE_MSG_OPTIONS_RECEIVED;
            int requestKey = parcel.readInt();
            String remote = parcel.readString();
            long capabilities = parcel.readLong();
            UceOptionsReceivedData data = new UceOptionsReceivedData(remote, capabilities);
            msg.arg1 = requestKey;
            msg.obj = data;
            mUceAgentHandler.sendMessage(msg);
        }
    }

    @Override
    public void onPublishStatusMessage(Parcel parcel) {
        Message msg = Message.obtain();

        int messageType = parcel.readInt();
        if (messageType == UceMessage.UCE_PUBLISH_UPDATED_IND) {
            ImsLog.d(mSlotId, "Publish updated");
            msg.what = UceMessage.UCE_MSG_PUBLISH_UPDATED;
            long capability = parcel.readLong();
            int responseCode = parcel.readInt();
            String reason = parcel.readString();
            int reasonHeaderCause = parcel.readInt();
            String reasonHeaderText = parcel.readString();
            String etag = parcel.readString();
            int needToRetry = parcel.readInt();

            ImsLog.d(mSlotId, "responseCode:" + responseCode + ", reason:" + reason
                    + ", reasonHeaderCause" + reasonHeaderCause + ", reasonHeaderText:"
                    + reasonHeaderText + ", capability:" + capability + ", needToRetry:"
                    + needToRetry);
            UceResponseData data = new UceResponseData(responseCode, reason);
            data.setReasonHeaderCause(reasonHeaderCause);
            data.setReasonHeaderText(reasonHeaderText);
            data.setEtag(etag);
            data.setCapability(capability);
            data.setNeedToRetry(needToRetry);
            msg.obj = data;
        } else { // UceMessage.UCE_UNPUBLISH_IND
            ImsLog.d(mSlotId, "un-published");
            msg.what = UceMessage.UCE_MSG_UNPUBLISHED;
        }
        mUceAgentHandler.sendMessage(msg);
    }
}
