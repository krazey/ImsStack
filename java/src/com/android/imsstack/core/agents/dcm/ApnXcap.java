package com.android.imsstack.core.agents.dcm;

import android.content.Context;
import android.os.AsyncResult;
import android.os.Message;
import android.telephony.TelephonyManager;
import android.telephony.data.ApnSetting;

import com.android.imsstack.core.agents.agentif.MsgProcInterface;
import com.android.imsstack.core.agents.dcmif.EApnReqState;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.core.agents.dcmif.EIpVersion;
import com.android.imsstack.core.agents.dcmif.IDCNetWatcher;
import com.android.imsstack.core.agents.dcmif.IDCSettings;
import com.android.imsstack.util.ImsLog;

public class ApnXcap extends Apn {

    // Constants--------------------------------------------------
    protected static final int OBTAIN_IPV6_ADDRESS_DELAY_INTERVAL = 2000;

    // Variables--------------------------------------------------

    // Public methods --------------------------------------------
    public ApnXcap(Context context, int slotId) {
        super(context, slotId);

        initializeApn();
    }

    // Interface implementation methods --------------------------
    @Override
    public void cleanup() {
        if (mDcNetWatcher != null) {
            mDcNetWatcher.unregisterForAirplaneModeChanged(this);
        }
        super.cleanup();
    }

    @Override
    public boolean connect() {
        if (isApnEmployed() == false) {
            ImsLog.w(nSlotId, "apn is not employed");
            return false;
        }

        if (mAPNState == EApnReqState.APN_REQUEST_DONE) {
            ImsLog.i(nSlotId, "apn request is already done");
            return true;
        }

        if (mESMCausePermanentFailure) {
            ImsLog.w(nSlotId, "permanent apn block because of esm until next bootup.");
            return false;
        }

        requestNetwork();
        setAPNReqState(EApnReqState.APN_REQUEST_DONE);
        setDataState(TelephonyManager.DATA_CONNECTING);

        return true;
    }

    @Override
    public void disconnect(int nTimeAfterRecover) {
        if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
            ImsLog.w(nSlotId, "apn request is not done");
            return;
        }

        if (hasMessages(EVENT_WAITING_LOCAL_ADDRESS_IPV6)) {
            removeMessages(EVENT_WAITING_LOCAL_ADDRESS_IPV6);
        }

        releaseNetwork();
        setAPNReqState(EApnReqState.APN_REQUEST_IDLE);

        int dataState = TelephonyManager.DATA_DISCONNECTED;
        if (mDataState != dataState) {
            setDataState(dataState);
        }

        sendDataStateUpdateMessage(EApnType.XCAP, EDataState.DATA_STATE_DISCONNECTED);
    }

    // Private/Protected methods ---------------------------------
    protected void initializeApn() {
        eType = EApnType.XCAP;

        registerHandler(EVENT_NETWORK_AVAILABLE,
                new Handle_EVENT_NETWORK_AVAILABLE());
        registerHandler(EVENT_NETWORK_LOST,
                new Handle_EVENT_NETWORK_LOST());
        registerHandler(EVENT_IP_CHANGED,
                new Handle_EVENT_IP_CHANGED());
        registerHandler(EVENT_WAITING_LOCAL_ADDRESS_IPV6,
                new Handle_EVENT_WAITING_LOCAL_ADDRESS_IPV6());
        registerHandler(EVENT_AIRPLANE_MODE_CHANGED,
                new Handle_EVENT_AIRPLANE_MODE_CHANGED());
        registerHandler(EVENT_DATA_CONNECTION_FAILED,
                new Handle_EVENT_DATA_CONNECTION_FAILED());

        //register message handler to DCNetWatcher
        if (mDcNetWatcher != null) {
            mDcNetWatcher.registerForAirplaneModeChanged(this, EVENT_AIRPLANE_MODE_CHANGED, null);
        }
    }

    private boolean procWaitingLocalAddressForIpv6() {
        if (!hasLocalAddress(EIpVersion.IPV6.getInt())) {
            if (hasMessages(EVENT_WAITING_LOCAL_ADDRESS_IPV6)) {
                ImsLog.i("ipv6 is waiting");
                return true;
            }

            sendEmptyMessageDelayed(EVENT_WAITING_LOCAL_ADDRESS_IPV6,
                    OBTAIN_IPV6_ADDRESS_DELAY_INTERVAL);
            return true;
        }

        return false;
    }

    private class Handle_EVENT_NETWORK_AVAILABLE implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            int curDataState = TelephonyManager.DATA_CONNECTED;

            if (mAPNState != EApnReqState.APN_REQUEST_DONE) {
                ImsLog.w(nSlotId, "apn is not requested, ignore event");
                return;
            }

            // Check to get Ipv6
            if (mApnProtocol != ApnSetting.PROTOCOL_IP) {
                if (procWaitingLocalAddressForIpv6() == true) {
                    ImsLog.i("can't obtain IPv6, wait " +
                            OBTAIN_IPV6_ADDRESS_DELAY_INTERVAL + "ms");
                    return;
                }
            }

            if (mDataState != curDataState) {
                ImsLog.i(nSlotId, "data state :: " + mDataState + " >> " + curDataState);

                setDataState(curDataState);
                sendDataStateUpdateMessage(EApnType.XCAP, EDataState.DATA_STATE_CONNECTED);
            }
        }
    }

    private class Handle_EVENT_NETWORK_LOST implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            int curDataState = TelephonyManager.DATA_DISCONNECTED;

            if (hasMessages(EVENT_WAITING_LOCAL_ADDRESS_IPV6)) {
                removeMessages(EVENT_WAITING_LOCAL_ADDRESS_IPV6);
            }

            if (mDataState != curDataState) {
                ImsLog.i(nSlotId, "data state :: " + mDataState + " >> " + curDataState);

                setDataState(curDataState);

                sendDataStateUpdateMessage(EApnType.XCAP, EDataState.DATA_STATE_DISCONNECTED);

                disconnect(0);
            }
        }
    }

    private class Handle_EVENT_IP_CHANGED implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.i(nSlotId, "ip is changed");

            if (getDataState() == TelephonyManager.DATA_CONNECTED) {

                if (!isIPChanged()) {
                    ImsLog.i(nSlotId, "ip is changed but ip address is same");
                    return;
                }

                sendDataStateUpdateMessage(EApnType.XCAP, EDataState.DATA_STATE_IP_CHANGED);
                return;
            }

            // handle the waiting Ipv6
            if (!hasMessages(EVENT_WAITING_LOCAL_ADDRESS_IPV6)) {
                return;
            }

            removeMessages(EVENT_WAITING_LOCAL_ADDRESS_IPV6);

            updateDataState();
        }
    }

    private class Handle_EVENT_WAITING_LOCAL_ADDRESS_IPV6 implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            ImsLog.i(nSlotId, "apn is delayed, data is updated");
            updateDataState();
        }
    }

    private class Handle_EVENT_AIRPLANE_MODE_CHANGED implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            AsyncResult ar = (AsyncResult)msg.obj;

            if (ar == null) {
                ImsLog.d(nSlotId, "ar is null");
                return;
            }

            Boolean radiooff = (Boolean)ar.result;

            if (radiooff == null) {
                ImsLog.d(nSlotId, "radiooff is null");
                return;
            }

            ImsLog.i(nSlotId, "airplane mode = " + radiooff.booleanValue());

            if (radiooff.booleanValue() == true) {
                disconnect(0);
            }
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

            if (isESMCausePermanentFailure()) {
                notifyPdnConnectionFailed(EApnType.XCAP);
            }
        }
    }
}
