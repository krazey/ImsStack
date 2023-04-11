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
package com.android.imsstack.test;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Parcel;

import com.android.imsstack.enabler.aos.AosFactory;
import com.android.imsstack.enabler.aos.IAosRegistration;
import com.android.imsstack.enabler.aos.IAosRegistration.CapabilityPairs;
import com.android.imsstack.enabler.aos.IAosRegistrationListener;
import com.android.imsstack.enabler.media.MediaConstants;
import com.android.imsstack.enabler.mtc.IUMtcService;
import com.android.imsstack.enabler.mtc.MediaInfo;
import com.android.imsstack.enabler.mtc.MtcApp;
import com.android.imsstack.enabler.mtc.MtcCall;
import com.android.imsstack.enabler.mtc.MtcJniProxy;
import com.android.imsstack.enabler.mtc.SuppInfo;
import com.android.imsstack.imsservice.mmtel.ImsCallApp;
import com.android.imsstack.imsservice.mmtel.ImsCallSessionImpl;
import com.android.imsstack.imsservice.mmtel.ImsServiceManager;
import com.android.imsstack.internal.imsservice.ImsServiceRegistry;
import com.android.imsstack.internal.imsservice.MmTelFeatureRegistry;
import com.android.imsstack.system.ISystem;
import com.android.imsstack.system.ImsEventDef;
import com.android.imsstack.system.SystemInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

/**
 * IMS Test Helper
 */
public final class ImsTestHelper {
    private static ImsTestHelper sImsTestHelper = null;

    private static final String INTENT_AOS_TEST = "com.android.imsstack.action.INTENT_AOS_TEST";
    private static final String INTENT_SRVCC_TEST = "com.android.imsstack.action.INTENT_SRVCC_TEST";
    private static final String INTENT_MTC_TEST = "com.android.imsstack.action.INTENT_MTC_TEST";
    private static final String INTENT_QOS_TEST = "com.android.imsstack.action.INTENT_QOS_TEST";

    private static MtcCall sTempCall = null;

    private Context mContext;
    private ImsTestHelperReceiver mReceiver;

    public ImsTestHelper () {
        ImsLog.d("ImsTestHelper is loaded");

        mContext = AppContext.getInstance();
        if (mContext == null) {
            return;
        }
        mReceiver = new ImsTestHelperReceiver();
        mContext.registerReceiver(mReceiver, mReceiver.getFilter(), Context.RECEIVER_EXPORTED);
    }

    public static ImsTestHelper getInstance() {

        if ( sImsTestHelper == null ) {
            sImsTestHelper = new ImsTestHelper();
        }
        return sImsTestHelper;
    }

    public void cleanup() {
        ImsLog.d("cleanup()");
    }

    @VisibleForTesting
    protected static class ImsTestHelperReceiver extends BroadcastReceiver {
        IntentFilter mIntentFilter = new IntentFilter();

        public ImsTestHelperReceiver() {
            mIntentFilter.addAction(INTENT_AOS_TEST);
            mIntentFilter.addAction(INTENT_SRVCC_TEST);
            mIntentFilter.addAction(INTENT_MTC_TEST);
            mIntentFilter.addAction(INTENT_QOS_TEST);
        }

        public IntentFilter getFilter() {
            return mIntentFilter;
        }

        @Override
        public synchronized void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            ImsLog.d(ImsLog.lastSubString(action, "."));

            if (action.equals(INTENT_AOS_TEST)) {
                String strEvent = intent.getStringExtra("event");
                if (strEvent.equalsIgnoreCase("capa")) {
                    sendCapabilitiesChanged(intent.getStringExtra("network"),
                            intent.getStringExtra("voice"), intent.getStringExtra("video"),
                            intent.getStringExtra("call_composer"));
                } else if (strEvent.equalsIgnoreCase("vops")) {
                    sendVopsChanged(intent.getIntExtra("state", 0));
                }
            } else if (action.equals(INTENT_SRVCC_TEST)) {
                sendSrvccEvent(intent.getIntExtra("type", -1));
            } else if (action.equals(INTENT_MTC_TEST)) {
                sendMtcTestCommand(intent.getIntExtra("command", -1),
                        intent.getIntExtra("slotid", 0), intent.getIntArrayExtra("extras"));
            } else if (action.equals(INTENT_QOS_TEST)) {
                sendQosChanged(intent.getIntExtra("call", -1),
                        intent.getIntExtra("media", -1), intent.getStringExtra("ipaddress"),
                        intent.getIntExtra("port", -1), intent.getIntExtra("result", -1));
            }
        }

        // CAPABILITIES CHANGED TEST
        // extra parameter : network type, voice capa, video capa, call_composer capa
        // network = LTE / NR / IWLAN / UTRAN (multiple use with comma separated)
        // capa = 0 / 1 (multiple use with comma separated in network input order.)
        // ex) adb shell am broadcast -a com.android.imsstack.action.INTENT_AOS_TEST
        //     --es event capa --es network LTE,NR,IWLAN,UTRAN --es voice 1,1,0,0 --es video 0,0,1,0
        //     --es call_composer 1,1,1,0
        private void sendCapabilitiesChanged(
                String strNetwork, String strVoice, String strVideo, String strCallComposer) {
            ImsLog.d("sendCapabilitiesChanged :: network=" + strNetwork + ", voice=" + strVoice +
                    ", video=" + strVideo + ", call_composer=" + strCallComposer);

            AosFactory aosFactory = AosFactory.getInstance();
            IAosRegistration iAosRegistration = aosFactory.getAosRegistration(0);

            String[] strNetworks = strNetwork.split(",");
            String[] strVoices = strVoice.split(",");
            String[] strVideos = strVideo.split(",");
            String[] strCallComposers = strCallComposer.split(",");

            CapabilityPairs objCapabilityPairs = new CapabilityPairs();

            for (int i = 0; i < strNetworks.length; i++)
            {
                int nNetwork = -1;
                if (strNetworks[i].equalsIgnoreCase("LTE")) {
                    nNetwork = IAosRegistrationListener.NetworkType.LTE;
                } else if (strNetworks[i].equalsIgnoreCase("NR")) {
                    nNetwork = IAosRegistrationListener.NetworkType.NR;
                } else if (strNetworks[i].equalsIgnoreCase("UTRAN")) {
                    nNetwork = IAosRegistrationListener.NetworkType.UTRAN;
                } else if (strNetworks[i].equalsIgnoreCase("IWLAN")) {
                    nNetwork = IAosRegistrationListener.NetworkType.IWLAN;
                }

                int nCapabilities = 0;
                if (strVoices[i].equals("1")) {
                    nCapabilities |= IAosRegistrationListener.Capability.VOICE;
                }

                if (strVideos[i].equals("1")) {
                    nCapabilities |= IAosRegistrationListener.Capability.VIDEO;
                }

                if (strCallComposers[i].equals("1")) {
                    nCapabilities |= IAosRegistrationListener.Capability.CALL_COMPOSER;
                }

                objCapabilityPairs.addCapability(nNetwork, nCapabilities);
            }

            iAosRegistration.changeCapabilities(objCapabilityPairs);
        }

        // VOPS CHANGED TEST
        // extra parameter : vops state
        // state = 0 / 1
        // ex) adb shell am broadcast -a com.android.imsstack.action.INTENT_AOS_TEST
        //     --es event vops --ei state 1
        private void sendVopsChanged(int nState) {
            ISystem iSystem = SystemInterface.getInstance().getSystem(0);
            if (iSystem == null) {
                return;
            }

            ImsLog.d("sendVopsChanged :: nState=" + nState);

            iSystem.notifyEvent(ImsEventDef.IMS_EVENT_IMS_VOICE_OVER_PS_STATE, nState, 0);
        }

        // SRVCC STATE CHANGED TEST (Native MTC only)
        // extra parameter : srvcc state
        // state = 0(started) / 1(succeeded) / 2(failed) / 3(canceled)
        // ex)  adb shell am broadcast -a com.android.imsstack.action.INTENT_SRVCC_TEST --ei type 0
        private void sendSrvccEvent(int state) {
            ImsLog.d("sendSrvccEvent :: nState=" + state);

            ImsServiceManager sm = ImsServiceManager.getDefault();
            ImsCallApp callApp = sm.getCallApp(0);
            if (callApp == null) {
                return;
            }

            MmTelFeatureRegistry mmtelFeatureRegistry = ImsServiceRegistry.getInstance(
                    callApp.getPhoneId()).getMmTelFeatureRegistry();

            if (mmtelFeatureRegistry == null) {
                return;
            }
            mmtelFeatureRegistry.setSrvccState(state);
        }

        // MTC TEST COMMAND (Native MTC only)
        // extra parameter : test command, WParam, LParam
        // ex)  adb shell am broadcast -a com.android.imsstack.action.INTENT_MTC_TEST
        // .    --ei command 0 --ei slotid 0 --eia extras 0,0,1...
        // no exception check for the array as it's only for test
        private void sendMtcTestCommand(int command, int slotId, int[] extras) {
            ImsLog.d("sendMtcTestCommand :: command=" + command);

            ImsServiceManager sm = ImsServiceManager.getDefault();
            ImsCallApp callApp = sm.getCallApp(slotId);
            if (callApp == null) {
                return;
            }

            MtcApp mtcApp = callApp.getCallManager().getMtcApp();
            if (mtcApp == null) {
                return;
            }

            if (command == 100) {
                // 0 : callAttributes
                // 1 : emergencyRouting type
                ImsLog.d("sendMtcTestCommand :: open emergency service");
                sTempCall = mtcApp.createCall(extras[0]);
                mtcApp.openEmergencyService(sTempCall, extras[1]);
                return;
            } else if (command == 101) {
                // 0 : callAttributes
                // 1 : serviceType, 2 : emergency, 3 : offline, 4 : ussi
                ImsLog.d("sendMtcTestCommand :: open call");
                sTempCall = mtcApp.createCall(extras[0]);
                sTempCall.open(extras[1], extras[2] != 0, extras[3] != 0, extras[4] != 0);
                return;
            } else if (command == 102) {
                // 0 : callType
                // 1 : callee, 2 : actualCallee, 3 ~ 5 : audio/video/text direction
                ImsLog.d("sendMtcTestCommand :: start call");
                if (sTempCall == null) {
                    return;
                }
                MediaInfo mediaInfo = new MediaInfo();
                mediaInfo.ADir = extras[3];
                mediaInfo.VDir = extras[4];
                mediaInfo.TDir = extras[5];
                SuppInfo suppInfo = new SuppInfo();
                sTempCall.start(extras[0], extras[1] + "", extras[2] + "", mediaInfo, suppInfo);
                return;
            } else if (command == 103) {
                // 0 : reason
                ImsLog.d("sendMtcTestCommand :: terminate call");
                if (sTempCall == null) {
                    return;
                }
                sTempCall.terminate(extras[0], true);
                return;
            } else if (command == 104) {
                ImsLog.d("sendMtcTestCommand :: close call");
                if (sTempCall == null) {
                    return;
                }
                sTempCall.close();
                return;
            }

            Parcel parcel = Parcel.obtain();
            parcel.writeInt(IUMtcService.TEST_COMMAND);
            parcel.writeInt(command);
            parcel.writeInt(extras[0]);
            parcel.writeInt(extras[1]);

            MtcJniProxy.getInstance().sendDataToNative(mtcApp.getJNIService(), parcel);
        }

        // QOS CHANGED Test
        // extra parameter : call, media, IP address, port, result
        // call : foreground call(0) / background call(1)
        // media : audio(0) / video(1) / text(2)
        // result : inactive(0) / active(1)
        // ex)  adb shell am broadcast -a com.android.imsstack.action.INTENT_QOS_TEST
        // .    --ei call 0 --ei media 0 --es ipaddress 192.168.0.1 --ei port 50010 --ei result 1
        private void sendQosChanged(int call, int media, String ipAddress, int port, int result) {
            ImsLog.d("sendQosChanged :: call=" + call + ", media=" + media);

            ImsServiceManager sm = ImsServiceManager.getDefault();
            ImsCallApp callApp = sm.getCallApp(0);
            if (callApp == null) {
                return;
            }

            ImsCallSessionImpl callSession = null;
            if (call == 0) {
                callSession = callApp.getCallManager().getConnectingSession();
                if (callSession == null) {
                    callSession = callApp.getCallManager().getActiveSession();
                }
            } else {
                callSession = callApp.getCallManager().getHoldSession();
            }

            if (callSession == null) {
                return;
            }

            MtcCall mtcCall = callSession.getMtcCall();
            if (mtcCall == null) {
                return;
            }

            ImsLog.d("sendQosChanged :: call=" + call + ", media=" + media);

            Parcel parcel = Parcel.obtain();
            parcel.writeInt(MediaConstants.NOTIFY_QOS_INFO);
            parcel.writeInt(media);
            parcel.writeString(ipAddress);
            parcel.writeInt(port);
            parcel.writeBoolean(result == 1);

            MtcJniProxy.getInstance().sendDataToNative(mtcCall.getNativeCallId(), parcel);
        }
    }
}
