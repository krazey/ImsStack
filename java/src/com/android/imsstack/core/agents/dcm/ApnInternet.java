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

package com.android.imsstack.core.agents.dcm;

import android.content.Context;
import android.os.Message;
import android.telephony.TelephonyManager;

import com.android.imsstack.core.agents.agentif.MsgProcInterface;
import com.android.imsstack.core.agents.dcmif.EApnType;
import com.android.imsstack.core.agents.dcmif.EDataState;
import com.android.imsstack.util.ImsLog;

/**
 * this is data connection class for internet
 */
public class ApnInternet extends Apn {

    // Variables--------------------------------------------------

    // Public methods --------------------------------------------
    public ApnInternet(Context context, int slotId) {
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
        ImsLog.i("apn is connected");

        // Internet apn is handled by system. So it doesn't check employment state when connect
        // Using Internet pdn registration, send the data connected msg to native
        int dataState = getDataStateFromCM();

        if (mDataState != dataState) {
            ImsLog.w("data state :: " + mDataState + " >> " + dataState);

            setDataState(dataState);
            sendDataStateUpdateMessage(EApnType.INTERNET,
                    EDataState.convertIntTypeToEnum(EDataState.convertFromTMtoImsType(
                    (mDataState))));
        }

        return true;
    }

    @Override
    public void disconnect(int nTimeAfterRecover) {
        //Do nothing for Internet PDN
    }

    // Private/Protected methods ---------------------------------
    protected void initializeApn() {
        mType = EApnType.INTERNET;

        registerHandler(EVENT_NETWORK_AVAILABLE,
                new Handle_EVENT_NETWORK_AVAILABLE());
        registerHandler(EVENT_NETWORK_LOST,
                new Handle_EVENT_NETWORK_LOST());

        registerCallback(ImsNetworkCallback.EVENT_ALL);
    }

    private class Handle_EVENT_NETWORK_AVAILABLE implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            int curDataState = TelephonyManager.DATA_CONNECTED;

            if (mDataState != curDataState) {
                ImsLog.w("data state :: " + mDataState + " >> " + curDataState);

                setDataState(curDataState);

                sendDataStateUpdateMessage(EApnType.INTERNET, EDataState.DATA_STATE_CONNECTED);
            }
        }
    }

    private class Handle_EVENT_NETWORK_LOST implements MsgProcInterface {
        @Override
        public void procMsg(Message msg) {
            int curDataState = TelephonyManager.DATA_DISCONNECTED;

            if (mDataState != curDataState) {
                ImsLog.w("data state :: " + mDataState + " >> " + curDataState);

                setDataState(curDataState);

                sendDataStateUpdateMessage(EApnType.INTERNET, EDataState.DATA_STATE_DISCONNECTED);
            }
        }
    }
}
