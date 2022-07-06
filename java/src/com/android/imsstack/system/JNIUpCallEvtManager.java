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

import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.agentif.IBatteryState;
import com.android.imsstack.util.AppContext;
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
        private RegistrantList mRegiStateChangedRegistrants = new RegistrantList();
        private RegistrantList mDebugServiceRegistrants = new RegistrantList();
        private RegistrantList mDebugAWTChangedRegistrants = new RegistrantList();
        private RegistrantList mVoLTEIndicatorChangedRegistrants = new RegistrantList();
        private RegistrantList mVoWIFIIndicatorChangedRegistrants = new RegistrantList();
        private RegistrantList mTraceMOCARegistrants = new RegistrantList();
        private RegistrantList mShowNotProvisionedNotiRegistrants = new RegistrantList();
        private RegistrantList mDataConnectionChangedRegistrants = new RegistrantList();
        private RegistrantList mISIMReadResultRegistrants = new RegistrantList();
        private RegistrantList mRegiReportToWFCRegistrants = new RegistrantList();
        private RegistrantList mRegServiceChangedRegistrants = new RegistrantList();
        private RegistrantList mRegFailureChangedRegistrants = new RegistrantList();
        private RegistrantList mDCNChangedRegistrants = new RegistrantList();
        private RegistrantList mObtainPhoneNumberRegistrants = new RegistrantList();
        private RegistrantList mMLTMessageRegistrants = new RegistrantList();
        private RegistrantList mSetPdnPreferenceToEpdg = new RegistrantList();
        private RegistrantList mSetServiceStatusToEpdg = new RegistrantList();
        private RegistrantList mReportBadNetworkToEpdg = new RegistrantList();
        private RegistrantList mRegDestroyedRegistrants = new RegistrantList();
        private RegistrantList mRegNotifyStateRegistrants = new RegistrantList();
        private RegistrantList mSendDataToModemRegistrants = new RegistrantList();
        private RegistrantList mSendSCMToModemRegistrants = new RegistrantList();
        private RegistrantList mRegistrationChangedRegistrants = new RegistrantList();
        private RegistrantList mCallInfoRegistrants = new RegistrantList();
        private RegistrantList mCallIMedianfoRegistrants = new RegistrantList();
        // IMS_CHANGE [SMS_Patch_0621][VZW], MSG_SMS_SMS_TO_911
        private RegistrantList mSendSCMStateRegistrants = new RegistrantList();

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
        public void registerForRegiStateChanged(Handler h, int what, Object obj) {
            mRegiStateChangedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForRegiStateChanged(Handler h) {
            mRegiStateChangedRegistrants.remove(h);
        }

        @Override
        public void registerForDebugService(Handler h, int what, Object obj) {
            mDebugServiceRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForDebugService(Handler h) {
            mDebugServiceRegistrants.remove(h);
        }

        @Override
        public void registerForDebugAWTChanged(Handler h, int what, Object obj) {
            mDebugAWTChangedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForDebugAWTChanged(Handler h) {
            mDebugAWTChangedRegistrants.remove(h);
        }

        @Override
        public void registerForVoLTEIndicatorChanged(Handler h, int what, Object obj) {
            mVoLTEIndicatorChangedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForVoLTEIndicatorChanged(Handler h) {
            mVoLTEIndicatorChangedRegistrants.remove(h);
        }

        @Override
        public void registerForVoWIFIIndicatorChanged(Handler h, int what, Object obj) {
            mVoWIFIIndicatorChangedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForVoWIFIIndicatorChanged(Handler h) {
            mVoWIFIIndicatorChangedRegistrants.remove(h);
        }

        @Override
        public void registerForTraceMOCA(Handler h, int what, Object obj) {
            mTraceMOCARegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForTraceMOCA(Handler h) {
            mTraceMOCARegistrants.remove(h);
        }

        @Override
        public void registerForShowNotProvisionedNoti(Handler h, int what, Object obj) {
            mShowNotProvisionedNotiRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForShowNotProvisionedNoti(Handler h) {
            mShowNotProvisionedNotiRegistrants.remove(h);
        }

        @Override
        public void registerForDataConnectionChanged(Handler h, int what, Object obj) {
            mDataConnectionChangedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForDataConnectionChanged(Handler h) {
            mDataConnectionChangedRegistrants.remove(h);
        }

        @Override
        public void registerForISIMReadResult(Handler h, int what, Object obj) {
            mISIMReadResultRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForISIMReadResult(Handler h) {
            mISIMReadResultRegistrants.remove(h);
        }

        @Override
        public void registerForRegiReportToWFC(Handler h, int what, Object obj) {
            mRegiReportToWFCRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForRegiReportToWFC(Handler h) {
            mRegiReportToWFCRegistrants.remove(h);
        }

        @Override
        public void registerForRegServiceChanged(Handler h, int what, Object obj) {
            mRegServiceChangedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForRegServiceChanged(Handler h) {
            mRegServiceChangedRegistrants.remove(h);
        }

        @Override
        public void registerForRegFailureChanged(Handler h, int what, Object obj) {
            mRegFailureChangedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForRegFailureChanged(Handler h) {
            mRegFailureChangedRegistrants.remove(h);
        }

        @Override
        public void registerForDCNChanged(Handler h, int what, Object obj) {
            mDCNChangedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForDCNChanged(Handler h) {
            mDCNChangedRegistrants.remove(h);
        }

        @Override
        public void registerForObtainPhoneNumber(Handler h, int what, Object obj) {
            mObtainPhoneNumberRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForObtainPhoneNumber(Handler h) {
            mObtainPhoneNumberRegistrants.remove(h);
        }

        @Override
        public void registerForMLTMessageChanged(Handler h, int what, Object obj) {
            mMLTMessageRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForMLTMessageChanged(Handler h) {
            mMLTMessageRegistrants.remove(h);
        }

        @Override
        public void registerForSetPdnPreferenceToEpdg(Handler h, int what, Object obj) {
            mSetPdnPreferenceToEpdg.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForSetPdnPreferenceToEpdg(Handler h) {
            mSetPdnPreferenceToEpdg.remove(h);
        }

        @Override
        public void registerForSetServiceStatusToEpdg(Handler h, int what, Object obj) {
            mSetServiceStatusToEpdg.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForSetServiceStatusToEpdg(Handler h) {
            mSetServiceStatusToEpdg.remove(h);
        }

        @Override
        public void registerForReportBadNetworkToEpdg(Handler h, int what, Object obj) {
            mReportBadNetworkToEpdg.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterReportBadNetworkToEpdg(Handler h) {
            mReportBadNetworkToEpdg.remove(h);
        }

        @Override
        public void registerForRegDestroyed(Handler h, int what, Object obj) {
            mRegDestroyedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForRegDestroyed(Handler h) {
            mRegDestroyedRegistrants.remove(h);
        }

        @Override
        public void registerForRegNotifyState(Handler h, int what, Object obj) {
            mRegNotifyStateRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForRegNotifyState(Handler h) {
            mRegNotifyStateRegistrants.remove(h);
        }

        @Override
        public void registerForSendDataToModem(Handler h, int what, Object obj) {
            mSendDataToModemRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForSendDataToModem(Handler h) {
            mSendDataToModemRegistrants.remove(h);
        }

        @Override
        public void registerForSendSCMToModem(Handler h, int what, Object obj) {
            mSendSCMToModemRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForSendSCMToModem(Handler h) {
            mSendSCMToModemRegistrants.remove(h);
        }

        @Override
        public void registerForRegistrationChanged(Handler h, int what, Object obj) {
            mRegistrationChangedRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForRegistrationChanged(Handler h) {
            mRegistrationChangedRegistrants.remove(h);
        }

        @Override
        public void registerForCallInfo(Handler h, int what, Object obj) {
            mCallInfoRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForCallInfo(Handler h) {
            mCallInfoRegistrants.remove(h);
        }

        @Override
        public void registerForCallMediaInfo(Handler h, int what, Object obj) {
            mCallIMedianfoRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForCallMediaInfo(Handler h) {
            mCallIMedianfoRegistrants.remove(h);
        }

        @Override
        public void registerForSCMState(Handler h, int what, Object obj) {
            mSendSCMStateRegistrants.add(new Registrant(h, what, obj));
        }

        @Override
        public void unregisterForSCMState(Handler h) {
            mSendSCMStateRegistrants.remove(h);
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
            mapEventProc.put(ImsEventDef.IMS_EVENT_REG_STATE,
                    new HandleIMSEventRegState()); // 0x00000002
            mapEventProc.put(ImsEventDef.IMS_EVENT_VOLTE_INDICATOR,
                    new handle_IMS_EVENT_VOLTE_INDICATOR()); // 0x00000006
            mapEventProc.put(ImsEventDef.IMS_EVENT_VOWIFI_INDICATOR,
                    new handle_IMS_EVENT_VOWIFI_INDICATOR()); // 0x00000007
            mapEventProc.put(ImsEventDef.IMS_EVENT_REG_SERVICE,
                    new Handle_IMS_EVENT_REG_SERVICE()); // 0x00000010
            mapEventProc.put(ImsEventDef.IMS_EVENT_REG_FAILURE,
                    new Handle_IMS_EVENT_REG_FAILURE()); // 0x00000011
            mapEventProc.put(ImsEventDef.IMS_EVENT_NATIVE_BOOT_COMPLETED,
                    new HandleIMSEventNativeBootCompleted()); // 0x00000012
            // handle as default without slod id
            //mapEventProc.put(ImsEventDef.IMS_EVENT_WAKE_LOCK, new handle_IMS_EVENT_WAKE_LOCK()); // 0x00000013
            mapEventProc.put(ImsEventDef.IMS_EVENT_DATA_CONNECTION,
                    new handle_IMS_EVENT_DATA_CONNECTION()); // 0x00000015
            mapEventProc.put(ImsEventDef.IMS_EVENT_OBTAIN_PHONE_NUMBER,
                    new Handle_IMS_EVENT_OBTAIN_PHONE_NUMBER());  //0x00000017
            mapEventProc.put(ImsEventDef.IMS_EVENT_DCN, new Handle_IMS_EVENT_DCN()); // 0x00000019
            mapEventProc.put(ImsEventDef.IMS_EVENT_MLT, new Handle_IMS_EVENT_MLT_MESSAGE()); // 0x00000021
            mapEventProc.put(ImsEventDef.IMS_EVENT_REG_DESTROYED,
                    new Handle_IMS_EVENT_REG_DESTROYED()); // 0x00000022
            mapEventProc.put(ImsEventDef.IMS_EVENT_NOTIFY_STATE,
                    new Handle_IMS_EVENT_NOTIFY_STATE()); // 0x00000023
            mapEventProc.put(ImsEventDef.IMS_EVENT_SHOW_MESSAGE,
                    new handle_IMS_EVENT_SHOW_MESSAGE()); // 0x00000080
            mapEventProc.put(ImsEventDef.IMS_EVENT_REGISTRATION,
                new Handle_IMS_EVENT_REGISTRATION()); // 0x00000100
            mapEventProc.put(ImsEventDef.IMS_EVENT_TRACE_MOCA,
                    new handle_IMS_EVENT_TRACE_MOCA()); // 0x00000500
            mapEventProc.put(ImsEventDef.IMS_EVENT_ISIM_STATE,
                    new handle_IMS_EVENT_ISIM_STATE()); // 0x00000700
            mapEventProc.put(ImsEventDef.IMS_EVENT_PDN_PRECONDITION_CHANGED,
                    new handle_IMS_EVENT_PDN_PRECONDITION_CHANGED()); // 0x00000701
            mapEventProc.put(ImsEventDef.IMS_EVENT_DEBUG,
                    new handle_IMS_EVENT_DEBUG()); // 0x00000800
            mapEventProc.put(ImsEventDef.IMS_EVENT_DEBUG_AWT_UPDATED,
                    new Handle_IMS_EVENT_DEBUG_AWT_UPDATED()); // 0x00000801
            mapEventProc.put(ImsEventDef.IMS_EVENT_REGI_REPORT_TO_WFC,
                    new Handle_IMS_EVENT_REGI_REPORT_TO_WFC()); // 0x04000000
            mapEventProc.put(ImsEventDef.IMS_EVENT_EPDG_PREFERENCE,
                    new Handle_IMS_EVENT_EPDG_PREFERENCE()); // 0x05000000
            mapEventProc.put(ImsEventDef.IMS_EVENT_SERVICE_STATUS,
                    new Handle_IMS_EVENT_SERVICE_STATUS()); // 0x05000001
            mapEventProc.put(ImsEventDef.IMS_EVENT_REPORT_BAD_NETWORK,
                    new Handle_IMS_EVENT_REPORT_BAD_NETWORK()); // 0x05000002
            // handle as default without slod id
            //mapEventProc.put(ImsEventDef.IMS_EVENT_WIFI_SERVICE,
            //        new handle_IMS_EVENT_WIFI_SERVICE()); // 0x10000000
            mapEventProc.put(ImsEventDef.IMS_EVENT_CALL_INFO_TO_WFC, //0x30000000
                    new Handle_IMS_EVENT_CALL_INFO_TO_WFC());
            mapEventProc.put(ImsEventDef.IMS_EVENT_SEND_DATA_TO_MODEM,
                    new Handle_IMS_EVENT_SEND_DATA_TO_MODEM()); // 0x80000001
            mapEventProc.put(ImsEventDef.IMS_EVENT_SEND_SCM_TO_MODEM,
                    new Handle_IMS_EVENT_SEND_SCM_TO_MODEM()); // 0x80000002
            mapEventProc.put(ImsEventDef.IMS_EVENT_CALL_INFO,
                    new Handle_IMS_EVENT_CALL_INFO()); // 0x90000000
            mapEventProc.put(ImsEventDef.IMS_EVENT_CALL_MEIDA_INFO,
                    new Handle_IMS_EVENT_CALL_MEDIA_INFO()); // 0x90000000
        }

        private void sendIntent(String action, String name1, int value1, String name2, int value2) {
            Intent intent = new Intent(action);

            //4 Remove the flag if the intent is not required this flag
            intent.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
            intent.putExtra(name1, value1);
            intent.putExtra(name2, value2);

            AppContext.get().sendBroadcast(intent);
        }

        private void sendIntent(String action, String[] name, int[] value) {
            if (name == null || value == null) {
                return;
            }

            if (name.length != value.length) {
                return;
            }

            Intent intent = new Intent(action);
            //4 Remove the flag if the intent is not required this flag
            intent.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);

            for (int i = 0; i < name.length; i++) {
                intent.putExtra(name[i], value[i]);
            }

            AppContext.get().sendBroadcast(intent);
        }

        private void sendIntent(String action, String name1, int value1, String name2, String value2) {
            Intent intent = new Intent(action);

            //4 Remove the flag if the intent is not required this flag
            intent.addFlags(Intent.FLAG_INCLUDE_STOPPED_PACKAGES);
            intent.putExtra(name1, value1);
            intent.putExtra(name2, value2);

            AppContext.get().sendBroadcast(intent);
        }

        //--------------------------------------------------------------------------------------------------------------
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

        private class HandleIMSEventRegState implements EventProc {
            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                Message msg = Message.obtain();
                msg.what = nEvent;
                // HIWORD Service Type
                msg.arg1 = nWParam & 0xFFFF;
                msg.arg2 = nLParam;

                ImsLog.i(mSlotId, "reg state :: type=" + msg.arg1 + ", state=" + msg.arg2);

                mRegiStateChangedRegistrants.notifyResult(msg);
            }
        }

        private class handle_IMS_EVENT_DEBUG implements EventProc {
            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mDebugServiceRegistrants.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_DEBUG_AWT_UPDATED implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                mDebugAWTChangedRegistrants.notifyRegistrants();
            }
        }

        private class handle_IMS_EVENT_VOLTE_INDICATOR implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mVoLTEIndicatorChangedRegistrants.notifyResult(msg);
            }
        }

        private class handle_IMS_EVENT_VOWIFI_INDICATOR implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mVoWIFIIndicatorChangedRegistrants.notifyResult(msg);
            }
        }

        private class handle_IMS_EVENT_SHOW_MESSAGE implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");

                if (nLParam == ImsEventDef.IMS_MESSAGE_SERVICE_NOT_PROVISIONED) {
                    mShowNotProvisionedNotiRegistrants.notifyRegistrants();
                }
            }
        }

        private class handle_IMS_EVENT_TRACE_MOCA implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");

                if (nWParam == ImsEventDef.IMS_TRACE_MOCA_STOP ) {
                    Message msg = Message.obtain();
                    msg.what = nEvent;
                    msg.arg1 = nWParam;
                    msg.arg2 = nLParam;

                    mTraceMOCARegistrants.notifyResult(msg);
                }
            }
        }

        private class handle_IMS_EVENT_DATA_CONNECTION implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");

                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mDataConnectionChangedRegistrants.notifyResult(msg);
            }
        }

        private class handle_IMS_EVENT_ISIM_STATE implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mISIMReadResultRegistrants.notifyResult(msg);
            }
        }

        private class handle_IMS_EVENT_PDN_PRECONDITION_CHANGED implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");

                String[] name = {"change", "param", MSimUtils.PHONE_KEY};
                int[] value = new int[3];
                value[0] = nWParam;
                value[1] = nLParam;
                value[2] = mSlotId;

                sendIntent(ImsEventDef.ACTION_IMS_PDN_PRECONDITION_CHANGED, name, value);
            }
        }

        private class Handle_IMS_EVENT_REGI_REPORT_TO_WFC implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mRegiReportToWFCRegistrants.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_EPDG_PREFERENCE implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mSetPdnPreferenceToEpdg.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_SERVICE_STATUS implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mSetServiceStatusToEpdg.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_REPORT_BAD_NETWORK implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mReportBadNetworkToEpdg.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_REG_SERVICE implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mRegServiceChangedRegistrants.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_REG_FAILURE implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mRegFailureChangedRegistrants.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_DCN implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mDCNChangedRegistrants.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_OBTAIN_PHONE_NUMBER implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mObtainPhoneNumberRegistrants.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_MLT_MESSAGE implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mMLTMessageRegistrants.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_REG_DESTROYED implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mRegDestroyedRegistrants.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_NOTIFY_STATE implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mRegNotifyStateRegistrants.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_CALL_INFO_TO_WFC implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.i(mSlotId, "nEvent : " + nEvent + ", nWParam : " + nWParam + ", nLParam : " + nLParam);
                int status;
                String strRat;

                // set WFC active state
                if ( nWParam == ImsEventDef.IMS_EVENT_CALL_STATE_ACTIVE ) {
                    status = 1;
                } else {
                    status = 0;
                }

                // set WFC RAT information (wifi/lte)
                if ( nLParam == ImsEventDef.IMS_EVENT_CALL_TYPE_LTE ) {
                    strRat = "lte";
                } else {
                    strRat = "wifi";
                }

                sendIntent("com.android.imsstack.service.uc.processingINVITEMsg", "state", status, "rat", strRat);
            }
        }

        private class Handle_IMS_EVENT_SEND_DATA_TO_MODEM implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mSendDataToModemRegistrants.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_SEND_SCM_TO_MODEM implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mSendSCMToModemRegistrants.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_REGISTRATION implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mRegistrationChangedRegistrants.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_CALL_INFO implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "not-handled: event=" + nEvent);
            }
        }

        private class Handle_IMS_EVENT_CALL_MEDIA_INFO implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mCallIMedianfoRegistrants.notifyResult(msg);
            }
        }

        private class Handle_IMS_EVENT_SCM_STATE implements EventProc {

            @Override
            public void procEvt(int nEvent, int nWParam, int nLParam) {
                ImsLog.d(mSlotId, "");
                Message msg = Message.obtain();
                msg.what = nEvent;
                msg.arg1 = nWParam;
                msg.arg2 = nLParam;

                mSendSCMStateRegistrants.notifyResult(msg);
            }
        }
    }
}
