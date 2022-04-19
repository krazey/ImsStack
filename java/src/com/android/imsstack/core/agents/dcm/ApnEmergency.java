package com.android.imsstack.core.agents.dcm;

import android.content.Context;
import android.os.Message;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.agentif.MsgProcInterface;
import com.android.imsstack.core.agents.dcm.DCFactory;
import com.android.imsstack.core.agents.dcmif.ApnStateListener;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.IDCApn;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDCSettings;
import com.android.imsstack.core.agents.dcmif.IDCUtil;
import com.android.imsstack.util.ImsLog;

public class ApnEmergency extends Apn {

    // Public methods --------------------------------------------
    public ApnEmergency(Context context, int slotId) {
        super(context, slotId);

        initializeApn();
    }

    // Interface implementation methods --------------------------
    @Override
    public void cleanup() {
        super.cleanup();
    }

    @Override
    public boolean connect() {
        if (isApnEmployed() == false) {
            ImsLog.w(nSlotId, "apn is not employed");
            return false;
        }

        if (mAPNState == EApnReqState.APN_REQUEST_DONE) {
            ImsLog.w(nSlotId, "request is already done");
            return true;
        }

        setAPNReqState(EApnReqState.APN_REQUEST_DONE);

        requestNetwork();

        setDataState(TelephonyManager.DATA_CONNECTING);

        return true;
    }

    @Override
    public void disconnect(int nTimeAfterRecover) {
        if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
            ImsLog.w(nSlotId, "request is not done");
            return;
        }

        boolean isPdnConnected = isConnected();
        releaseNetwork();

        setDataState(TelephonyManager.DATA_DISCONNECTED);
        setAPNReqState(EApnReqState.APN_REQUEST_IDLE);

        if (isPdnConnected) {
            sendDataStateUpdateMessage(EApnType.EMERGENCY,
                    EDataState.DATA_STATE_DISCONNECTED);
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
        eType = EApnType.EMERGENCY;

        registerHandler(EVENT_NETWORK_AVAILABLE,
                new Handle_EVENT_NETWORK_AVAILABLE());
        registerHandler(EVENT_NETWORK_LOST,
                new Handle_EVENT_NETWORK_LOST());
        registerHandler(EVENT_IP_CHANGED,
                new Handle_EVENT_IP_CHANGED());
        registerHandler(EVENT_DATA_CONNECTION_FAILED,
                new Handle_EVENT_DATA_CONNECTION_FAILED());
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
                IDCUtil dcutil = (IDCUtil)DCFactory.getDC(DCFactory.UTIL, getSlotId());

                if (dcutil != null) {
                    dcutil.updateAllCellInfoForcinglyOnLimitedServiceState();
                }
            }

            if (mDataState != curDataState) {
                ImsLog.i(nSlotId, "data state :: " + mDataState + " >> " + curDataState
                        + ", apn string = " + mApnString + ", network type = " + mNetworkType);

                setDataState(curDataState);

                if (mNetworkType == TelephonyManager.NETWORK_TYPE_UNKNOWN) {
                    updateNetworkType();
                }
                handleIpcanCategory(mNetworkType);

                sendDataStateUpdateMessage(EApnType.EMERGENCY, EDataState.DATA_STATE_CONNECTED);
            }
        }
    }

    private class Handle_EVENT_NETWORK_LOST implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            int curDataState = TelephonyManager.DATA_DISCONNECTED;

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w(nSlotId, "apn is not requested");
                return;
            }

            if (mDataState != curDataState) {
                ImsLog.i(nSlotId, "data state :: " + mDataState + " >> " + curDataState);

                setDataState(curDataState);
                sendDataStateUpdateMessage(EApnType.EMERGENCY, EDataState.DATA_STATE_DISCONNECTED);
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

            sendDataStateUpdateMessage(EApnType.EMERGENCY, EDataState.DATA_STATE_IP_CHANGED);
        }
    }

    private class Handle_EVENT_DATA_CONNECTION_FAILED implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            int curDataState = TelephonyManager.DATA_DISCONNECTED;

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w(nSlotId, "apn is not requested");
                return;
            }

            if (mDataState != curDataState) {
                ImsLog.i(nSlotId, "data state :: " + mDataState + " >> " + curDataState);

                setDataState(curDataState);
                sendDataStateUpdateMessage(EApnType.EMERGENCY, EDataState.DATA_STATE_DISCONNECTED);
            }
        }
    }
}
