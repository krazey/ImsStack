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

package com.android.imsstack.imsservice.mmtel;

import android.telephony.ims.ImsSuppServiceNotification;

import com.android.internal.telephony.gsm.SuppServiceNotification;

/**
 * Helper class for ImsSuppServiceNotification.
 */
public class ImsSuppServiceUtils {
    public static final int NOTIFICATION_MO = 0;
    public static final int NOTIFICATION_MT = 1;

    public static ImsSuppServiceNotification createNotification(int type, int code) {
        return createNotification(type, code, -1);
    }

    public static ImsSuppServiceNotification createNotification(int type, int code,
            int index) {
        return new ImsSuppServiceNotification(type, code, index, -1, null, null);
    }

    public static ImsSuppServiceNotification createNotification(int type, int code,
            String number) {
        return createNotification(type, code, number, null);
    }

    public static ImsSuppServiceNotification createNotification(int type, int code,
            String number, String[] history) {
        return new ImsSuppServiceNotification(type, code, -1, -1, number, history);
    }

    public static class MO {
        /**
         * NOT_USED
         * Unconditional call forwarding is active.
         *
         * Notified when an outgoing call is made and unconditional forwarding is enabled.
         */
        public static ImsSuppServiceNotification getUnconditionalCfActive() {
            return createNotification(NOTIFICATION_MO,
                    SuppServiceNotification.CODE_1_UNCONDITIONAL_CF_ACTIVE);
        }

        /**
         * NOT_USED
         * Some of the conditional call forwardings are active.
         *
         * Notified when an outgoing call is made and conditional forwarding is enabled.
         */
        public static ImsSuppServiceNotification getSomeCfActive() {
            return createNotification(NOTIFICATION_MO,
                    SuppServiceNotification.CODE_1_SOME_CF_ACTIVE);
        }

        /**
         * USED
         * Call has been forwarded.
         *
         * Notified on A when the outgoing call actually gets forwarded to C.
         */
        public static ImsSuppServiceNotification getCallForwarded() {
            return createNotification(NOTIFICATION_MO,
                    SuppServiceNotification.CODE_1_CALL_FORWARDED);
        }

        /**
         * USED
         * Call is waiting.
         *
         * Notified on A when the B is busy on another call
         * and call waiting is enabled on B.
         */
        public static ImsSuppServiceNotification getCallIsWaiting() {
            return createNotification(NOTIFICATION_MO,
                    SuppServiceNotification.CODE_1_CALL_IS_WAITING);
        }

        /**
         * NOT_USED
         * This is a CUG call (also <index> present).
         *
         * Notified on A when A makes call to B, both A & B belong to a CUG group.
         *
         * @param index the index
         */
        public static ImsSuppServiceNotification getCUGCall(int index) {
            return createNotification(NOTIFICATION_MO,
                    SuppServiceNotification.CODE_1_CUG_CALL, index);
        }

        /**
         * USED
         * Outgoing calls are barred.
         *
         * Notified on A when outgoing is barred on A.
         */
        public static ImsSuppServiceNotification getOutgoingCallsBarred() {
            return createNotification(NOTIFICATION_MO,
                    SuppServiceNotification.CODE_1_OUTGOING_CALLS_BARRED);
        }

        /**
         * NOT_USED
         * Incoming calls are barred.
         *
         * Notified on A when A is calling B & incoming is barred on B.
         */
        public static ImsSuppServiceNotification getIncomingCallsBarred() {
            return createNotification(NOTIFICATION_MO,
                    SuppServiceNotification.CODE_1_INCOMING_CALLS_BARRED);
        }

        /**
         * NOT_USED
         * CLIR suppression rejected.
         *
         * Notified on A when CLIR suppression is rejected.
         */
        public static ImsSuppServiceNotification getCLIRSuppressionRejected() {
            return createNotification(NOTIFICATION_MO,
                    SuppServiceNotification.CODE_1_CLIR_SUPPRESSION_REJECTED);
        }

        /**
         * NOT_USED
         * Call has been deflected.
         *
         * Notified on A when the outgoing call gets deflected to C from B.
         */
        public static ImsSuppServiceNotification getCallDeflected() {
            return createNotification(NOTIFICATION_MO,
                    SuppServiceNotification.CODE_1_CALL_DEFLECTED);
        }
    }

    public static class MT {
        /**
         * USED
         * This is a forwarded call (MT call setup).
         *
         * Notified on C when the incoming call is forwarded from B.
         *
         * @param cause the reason for call forwarded
         * @param number the originating number
         * @param history the forwarded number list if any
         */
        public static ImsSuppServiceNotification getForwardedCall(int cause,
                String number, String[] history) {
            // "type" is not used
            return createNotification(NOTIFICATION_MT,
                    SuppServiceNotification.CODE_2_FORWARDED_CALL, number, history);
        }

        /**
         * NOT_USED
         * This is a CUG call (also <index> present) (MT call setup).
         *
         * Notified on B when A makes call to B, both A & B belong to a CUG group.
         *
         * @param index the index
         */
        public static ImsSuppServiceNotification getCUGCall(int index) {
            return createNotification(NOTIFICATION_MT,
                    SuppServiceNotification.CODE_2_CUG_CALL, index);
        }

        /**
         * USED
         * Call has been put on hold (during a voice call).
         *
         * Notified on B when A makes call to B & puts it on hold.
         */
        public static ImsSuppServiceNotification getCallOnHold() {
            return createNotification(NOTIFICATION_MT,
                    SuppServiceNotification.CODE_2_CALL_ON_HOLD);
        }

        /**
         * USED
         * Call has been retrieved (during a voice call).
         *
         * Notified on B when A makes call to B, puts it on hold & retrieves it back.
         */
        public static ImsSuppServiceNotification getCallRetrieved() {
            return createNotification(NOTIFICATION_MT,
                    SuppServiceNotification.CODE_2_CALL_RETRIEVED);
        }

        /**
         * USED
         * Multiparty call entered (during a voice call).
         * (Conference call)
         *
         * Notified on B when the call is changed as multiparty (conference).
         */
        public static ImsSuppServiceNotification getMultipartyCall() {
            return createNotification(NOTIFICATION_MT,
                    SuppServiceNotification.CODE_2_MULTI_PARTY_CALL);
        }

        /**
         * NOT_USED
         * Call on hold has been released (this is not a SS notification) (during a voice call).
         *
         * Notified on B when A makes call to B, puts it on hold & then releases it.
         */
        public static ImsSuppServiceNotification getOnHoldCallReleased() {
            return createNotification(NOTIFICATION_MT,
                    SuppServiceNotification.CODE_2_ON_HOLD_CALL_RELEASED);
        }

        /**
         * NOT_USED
         * Forward check SS message received (can be received whenever).
         *
         * Notified on C when incoming call is forwarded from B.
         */
        public static ImsSuppServiceNotification getForwardCheckReceived() {
            return createNotification(NOTIFICATION_MT,
                    SuppServiceNotification.CODE_2_FORWARD_CHECK_RECEIVED);
        }

        /**
         * NOT_USED
         * Call is being connected (alerting) with the remote party in alerting state
         * in explicit call transfer operation (during a voice call).
         *
         * Notified on B when call is connecting through Explicit Cold Transfer.
         */
        public static ImsSuppServiceNotification getCallConnectingECT() {
            return createNotification(NOTIFICATION_MT,
                    SuppServiceNotification.CODE_2_CALL_CONNECTING_ECT);
        }

        /**
         * NOT_USED
         * Call has been connected with the other remote party in explicit call transfer operation
         * (also number and subaddress parameters may be present)
         * (during a voice call or MT call setup).
         *
         * Notified on B when call is connected through Explicit Cold Transfer.
         *
         * @param number the connected number
         */
        public static ImsSuppServiceNotification getCallConnectedECT(String number) {
            return createNotification(NOTIFICATION_MT,
                    SuppServiceNotification.CODE_2_CALL_CONNECTED_ECT, number);
        }

        /**
         * NOT_USED
         * This is a deflected call (MT call setup).
         *
         * Notified on B when the incoming call is deflected call.
         */
        public static ImsSuppServiceNotification getDeflectedCall() {
            return createNotification(NOTIFICATION_MT,
                    SuppServiceNotification.CODE_2_DEFLECTED_CALL);
        }

        /**
         * NOT_USED
         * Additional incoming call forwarded.
         *
         * Notified on B when it is busy and the incoming call gets forwarded to C.
         */
        public static ImsSuppServiceNotification getAdditionalCallForwarded() {
            return createNotification(NOTIFICATION_MT,
                    SuppServiceNotification.CODE_2_ADDITIONAL_CALL_FORWARDED);
        }
    }
}
