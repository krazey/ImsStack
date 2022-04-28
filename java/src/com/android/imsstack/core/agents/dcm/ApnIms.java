package com.android.imsstack.core.agents.dcm;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Message;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.SubsInfoInterface;
import com.android.imsstack.core.agents.agentif.IIMSPhoneAgent;
import com.android.imsstack.core.agents.agentif.MsgProcInterface;
import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcmif.ApnStateListener;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IApn;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDCSettings;
import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosInfo;
import com.android.imsstack.util.FeatureUtils;
import com.android.imsstack.util.ImsLog;

public class ApnIms extends Apn {

    // Public methods --------------------------------------------
    public ApnIms(Context context, int slotId) {
        super(context, slotId);

        initializeApn();
    }

    // Interface implementation methods --------------------------
    @Override
    public void cleanup() {
        ImsLog.d(nSlotId, "clean up");

        super.cleanup();
    }

    @Override
    public boolean connect() {
        synchronized (this) {
            SubsInfoInterface subsInfo = AgentFactory.getInstance().getAgent(
                    SubsInfoInterface.class, nSlotId);
            if (subsInfo != null) {
                if (!subsInfo.isImsEnabled()) {
                    ImsLog.w(nSlotId, "ims is off");
                    return false;
                }
            }

            if (isApnEmployed() == false) {
                ImsLog.w(nSlotId, "apn is not employed");
                return false;
            }

            if (mAPNState == EApnReqState.APN_REQUEST_DONE) {
                ImsLog.i(nSlotId, "apn request is already done");
                return true;
            }

            setAPNReqState(EApnReqState.APN_REQUEST_DONE);
            requestNetwork();

            setDataState(TelephonyManager.DATA_CONNECTING);

            return true;
        }
    }

    @Override
    public void disconnect(int nTimeAfterRecover) {
        synchronized (this) {
            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w(nSlotId, "apn request is not done");
                return;
            }

            releaseNetwork();
            setAPNReqState(EApnReqState.APN_REQUEST_IDLE);

            int dataState = TelephonyManager.DATA_DISCONNECTED;

            if (mDataState != dataState) {
                ImsLog.w(nSlotId, "data state :: " + mDataState + " >> " + dataState);

                setDataState(dataState);
            }

            sendDataStateUpdateMessage(EApnType.IMS, EDataState.DATA_STATE_DISCONNECTED);
        }
    }

    @Override
    public String getApn() {
        if (mApnString != null) {
            return mApnString;
        }

        return super.getApn();
    }

    // Private/Protected methods ---------------------------------
    protected void initializeApn() {
        eType = EApnType.IMS;

        registerHandler(EVENT_NETWORK_AVAILABLE,
                new Handle_EVENT_NETWORK_AVAILABLE());
        registerHandler(EVENT_NETWORK_LOST,
                new Handle_EVENT_NETWORK_LOST());
        registerHandler(EVENT_IP_CHANGED,
                new Handle_EVENT_IP_CHANGED());
        registerHandler(EVENT_PCSCF_CHANGED,
                new Handle_EVENT_PCSCF_CHANGED());
        registerHandler(EVENT_DATA_CONNECTION_FAILED,
                new Handle_EVENT_DATA_CONNECTION_FAILED());
    }

    /**
     * Notifies the application that data handover information is changed.
     */
    @Override
    protected void notifyHandoverInfoChanged(int handoverState, int networkType, int failCause) {
        super.notifyHandoverInfoChanged(handoverState, networkType, failCause);

        if (handoverState == IApn.HANDOVER_FAILURE) {
            ImsLog.d(nSlotId, "notifyIpcanHandoverFailure :: networkType=" + networkType +
                    ", failCause=" + failCause);

            IAosInfo aosInfo = AosFactory.getInstance().getAosInfo(nSlotId);
            if (aosInfo != null) {
                int targetNetwork = (networkType == TelephonyManager.NETWORK_TYPE_IWLAN) ?
                        IApn.IPCAN_CATEGORY_MOBILE : IApn.IPCAN_CATEGORY_WLAN;
                aosInfo.notifyIpcanHandoverFailure(targetNetwork, failCause);
            }
        }
    }

    private class Handle_EVENT_NETWORK_AVAILABLE implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            int curDataState = TelephonyManager.DATA_CONNECTED;

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w(nSlotId, "apn is not requested, ignore event");
                return;
            }

            if (mDataState != curDataState) {
                ImsLog.w(nSlotId, "data state :: " + mDataState + " >> " + curDataState
                        + ", apn string = " + mApnString + ", network type = " + mNetworkType);

                setDataState(curDataState);

                if (mNetworkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
                    updateNetworkType();
                }
                handleIpcanCategory(mNetworkType);

                sendDataStateUpdateMessage(EApnType.IMS, EDataState.DATA_STATE_CONNECTED);
            }
        }
    }

    private class Handle_EVENT_NETWORK_LOST implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            int curDataState = TelephonyManager.DATA_DISCONNECTED;

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w(nSlotId, "apn is not requested");

                if (mDataState != TelephonyManager.DATA_CONNECTED) {
                    ImsLog.w(nSlotId, "data is not connected");
                    return;
                }
            }

            if (mDataState != curDataState) {
                ImsLog.i(nSlotId, "data state :: " + mDataState + " >> " + curDataState);

                setDataState(curDataState);

                sendDataStateUpdateMessage(EApnType.IMS, EDataState.DATA_STATE_DISCONNECTED);
            }
        }
    }

    private class Handle_EVENT_IP_CHANGED implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.i(nSlotId, "ip is changed");

            if (getDataState() != TelephonyManager.DATA_CONNECTED) {
                return;
            }

            if (!isIPChanged()) {
                ImsLog.i(nSlotId, "ip is changed but ip address is same");
                return;
            }

            sendDataStateUpdateMessage(EApnType.IMS, EDataState.DATA_STATE_IP_CHANGED);
        }
    }

    private class Handle_EVENT_PCSCF_CHANGED implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.i(nSlotId, "PCSCF address is changed");

            sendDataStateUpdateMessage(EApnType.IMS, EDataState.DATA_STATE_PCSCF_CHANGED);
        }
    }

    private class Handle_EVENT_DATA_CONNECTION_FAILED implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.d(nSlotId, "");

            if (msg == null) {
                return;
            }

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w("apn is not requested, ignore event");
                return;
            }

            if (mESMCausePermanentFailure) {
                sendDataStateUpdateMessage(EApnType.IMS, EDataState.DATA_STATE_CONNECT_FAILED);
            }
        }
    }
}
