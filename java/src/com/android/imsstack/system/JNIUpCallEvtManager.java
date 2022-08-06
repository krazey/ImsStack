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
package com.android.imsstack.system;

import android.os.Handler;
import android.os.Registrant;
import android.os.RegistrantList;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.IBatteryState;
import com.android.imsstack.util.ImsLog;
import com.android.imsstack.util.MSimUtils;

import java.util.HashMap;
import java.util.Hashtable;
import java.util.Map;

public class JNIUpCallEvtManager {
    // Constants--------------------------------------------------

    // Variables--------------------------------------------------
    private static JNIUpCallEvtManager sJNIUpCallEvtManager = new JNIUpCallEvtManager();

    private Map<Integer, IJNIUpCallEvt> mJniUpCallEvts =
        new HashMap<Integer, IJNIUpCallEvt>(MSimUtils.getMaxSimSlot());

    // Public methods --------------------------------------------
    public static JNIUpCallEvtManager getInstance() {
        return sJNIUpCallEvtManager;
    }

    public JNIUpCallEvtManager() {
    }

    public void init() {
        ImsLog.d("");
    }

    public synchronized void start(int slotId) {
        JNIUpCallEvt jniEvt = new JNIUpCallEvt(slotId);
        jniEvt.init();
        mJniUpCallEvts.put(slotId, jniEvt);
    }

    public synchronized void stop(int slotId) {
        JNIUpCallEvt jniEvt = (JNIUpCallEvt)mJniUpCallEvts.get(slotId);

        if (jniEvt == null) {
            return;
        }

        jniEvt.cleanup();
        mJniUpCallEvts.remove(slotId);
    }

    public synchronized IJNIUpCallEvt getJNIUpCallEvt(int slotId) {
        return mJniUpCallEvts.get(slotId);
    }

    //---------------------------------------------------------------------------------------------
    public interface EventProc {
        public void procEvt( int nEvent, int nWParam, int nLParam );
    }

    private class JNIUpCallEvt implements IJNIUpCallEvt, ISystemAPISendEvent {
        private Hashtable<Integer, EventProc> mapEventProc = new Hashtable <Integer, EventProc>();

        private RegistrantList mNativeBootCompleteRegistrants = new RegistrantList();

        private int mSlotId = 0;

        public JNIUpCallEvt(int slotId) {
            mSlotId = slotId;
        }

        public void init() {
            initEvtProcMap();

            ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

            if (system == null) {
                return;
            }

            system.setISystemAPISendEvent(this);
        }

        public void cleanup() {
            ISystem system = SystemInterface.getInstance().getSystem(mSlotId);

            if (system == null) {
                return;
            }

            system.setISystemAPISendEvent(null);
            mapEventProc.clear();
        }

        @Override
        public int getSlotId() {
            return mSlotId;
        }

        @Override
        public void registerForNativeBootComplete(Handler h, int what, Object obj) {
            mNativeBootCompleteRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForNativeBootComplete(Handler h) {
            mNativeBootCompleteRegistrants.remove(h);
        }

        @Override
        public int processEvent4Sys(int nEvent, int nWParam, int nLParam ) {
            ImsLog.i(mSlotId, "Event (" + nEvent + "), WP (" + nWParam + "), LP (" + nLParam + ")");

            EventProc EventProcess = mapEventProc.get(nEvent);

            if (EventProcess == null) {
                ImsLog.w("event is not supported");
                return 0;
            }

            EventProcess.procEvt(nEvent, nWParam, nLParam);

            return 1;

         }

        // Interface implementation methods --------------------------
        // Private/Protected methods ---------------------------------
        private void initEvtProcMap() {
            mapEventProc.put(ImsEventDef.IMS_EVENT_NATIVE_BOOT_COMPLETED,
                    new HandleIMSEventNativeBootCompleted()); // 0x00000012
            // handle as default without slod id
            //mapEventProc.put(ImsEventDef.IMS_EVENT_WAKE_LOCK, new handle_IMS_EVENT_WAKE_LOCK());
            // handle as default without slod id
            //mapEventProc.put(ImsEventDef.IMS_EVENT_WIFI_SERVICE,
            //        new handle_IMS_EVENT_WIFI_SERVICE()); // 0x10000000
        }

        private class HandleIMSEventNativeBootCompleted implements EventProc {
            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.i(mSlotId, "native is completed");
                mNativeBootCompleteRegistrants.notifyRegistrants();

                IBatteryState bs = (IBatteryState)AgentFactory.getAgent(
                        AgentFactory.BATTERY_STATE);

                if (bs != null) {
                    bs.notifyLowBatteryState(getSlotId());
                }
            }
        }
    }
}
