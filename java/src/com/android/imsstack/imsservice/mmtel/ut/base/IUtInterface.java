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

package com.android.imsstack.imsservice.mmtel.ut.base;

import static android.telephony.ims.feature.CapabilityChangeRequest.CapabilityPair;

import android.annotation.IntDef;
import android.content.Context;

import com.android.imsstack.enabler.ssc.SscConstant;
import com.android.imsstack.enabler.ssc.SscServiceClassUtil;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.List;

/**
 * Provides the Ut interface interworking to get/set the supplementary service configuration.
 * {link @IImsUt}
 */
public interface IUtInterface {
    /**
     * Provide current Ut service availability
     */
    boolean isUtAvailable();

    /**
     * Changes capabilities and radio technologies
     */
    void changeCapabilities(List<CapabilityPair> enabledCaps, List<CapabilityPair> disabledCaps);

    /**
     * Initialize Ut service
     */
    void start(Context context);

    /**
     * Set listener for Ut operation result
     */
    void setListener(IUtListener listener);

    /**
     * Set listener to listen to Ut service state change
     */
    void setServiceStateListener(IUtServiceStateListener listener);

    /**
     * Inform service state change
     */
    void onServiceStateChanged();

    /**
     * Implementation of IImsUt.
     */
    void close();

    /**
     * Implementation of IImsUt.
     */
    void queryCallBarring(int tId, int condition);

    /**
     * Implementation of IImsUt.
     */
    void queryCallBarringForServiceClass(int tId, int condition, int serviceClass);

    /**
     * Implementation of IImsUt.
     */
    void queryCallForward(int tId, int condition, String number);

    /**
     * Implementation of IImsUt.
     */
    void queryCallWaiting(int tId);

    /**
     * Implementation of IImsUt.
     */
    void queryCLIR(int tId);

    /**
     * Implementation of IImsUt.
     */
    void queryCLIP(int tId);

    /**
     * Implementation of IImsUt.
     */
    void queryCOLR(int tId);

    /**
     * Implementation of IImsUt.
     */
    void queryCOLP(int tId);

    /**
     * Implementation of IImsUt.
     */
    void updateCallBarring(int tId, int condition, int action, String[] barringList);

    /**
     * Implementation of IImsUt.
     */
    void updateCallBarringForServiceClass(int tId, int cbType, int action, String[] barringList,
            int serviceClass);

    /**
     * Implementation of IImsUt.
     */
    void updateCallBarringWithPassword(int tId, int condition, int action, String[] barringList,
            int serviceClass, String password);

    /**
     * Implementation of IImsUt.
     */
    void updateCallForward(int tId, int action, int condition, String number, int serviceClass,
            int timeSeconds);

    /**
     * Implementation of IImsUt.
     */
    void updateCallWaiting(int tId, boolean enable, int serviceClass);

    /**
     * Implementation of IImsUt.
     */
    void updateCLIR(int tId, int clirMode);

    /**
     * Implementation of IImsUt.
     */
    void updateCLIP(int tId, boolean enable);

    /**
     * Implementation of IImsUt.
     */
    void updateCOLR(int tId, int presentation);

    /**
     * Implementation of IImsUt.
     */
    void updateCOLP(int tId, boolean enable);

    /**
     * Adds the listener to be notified when the terminal-based supplementary service configuration
     * is changed.
     *
     * @param listener The listener to be added.
     */
    void addTbSscChangeListener(
            TerminalBasedSupplementaryServiceConfigurationChangeListener listener);

    /**
     * Removes the listener not to be notified when the terminal-based supplementary service
     * configuration is changed.
     *
     * @param listener The listener to be removed.
     */
    void removeTbSscChangeListener(
            TerminalBasedSupplementaryServiceConfigurationChangeListener listener);

    abstract class SupplementaryServiceConfiguration {
        public static final int SS_TYPE_CB = 0;
        public static final int SS_TYPE_OIP = 1;
        public static final int SS_TYPE_OIR = 2;
        public static final int SS_TYPE_TIP = 3;
        public static final int SS_TYPE_TIR = 4;

        @IntDef(prefix = {"SS_TYPE_"}, value = {
                SS_TYPE_CB,
                SS_TYPE_OIP,
                SS_TYPE_OIR,
                SS_TYPE_TIP,
                SS_TYPE_TIR,
        })
        @Retention(RetentionPolicy.SOURCE)
        public @interface SupplementaryServiceType {}

        public static final int STATUS_ENABLED = SscConstant.STATUS_ENABLE;
        public static final int STATUS_DISABLED = SscConstant.STATUS_DISABLE;

        @IntDef(prefix = {"STATUS_"}, value = {
                STATUS_ENABLED,
                STATUS_DISABLED,
        })
        @Retention(RetentionPolicy.SOURCE)
        public @interface Status {}

        private final @SupplementaryServiceType int mType;
        private final @Status int mStatus;

        SupplementaryServiceConfiguration(int type, int status) {
            mType = type;
            mStatus = status;
        }

        public @SupplementaryServiceType int getType() {
            return mType;
        }

        public @Status int getStatus() {
            return mStatus;
        }
    }

    final class CbData extends SupplementaryServiceConfiguration {
        public static final int CONDITION_BAIC = SscConstant.CONDITION_BAIC;
        public static final int CONDITION_BAOC = SscConstant.CONDITION_BAOC;
        public static final int CONDITION_BOIC = SscConstant.CONDITION_BOIC;
        public static final int CONDITION_BOIC_EXHC = SscConstant.CONDITION_BOIC_EXHC;
        public static final int CONDITION_BIC_WR = SscConstant.CONDITION_BIC_WR;
        public static final int CONDITION_ACR = SscConstant.CONDITION_ACR;

        @IntDef(prefix = {"CONDITION_"}, value = {
                CONDITION_BAIC,
                CONDITION_BAOC,
                CONDITION_BOIC,
                CONDITION_BOIC_EXHC,
                CONDITION_BIC_WR,
                CONDITION_ACR,
        })
        @Retention(RetentionPolicy.SOURCE)
        public @interface Condition {}

        public static final int SERVICE_CLASS_VOICE = SscServiceClassUtil.SERVICE_CLASS_VOICE;
        public static final int SERVICE_CLASS_VIDEO = SscServiceClassUtil.SERVICE_CLASS_VIDEO;

        @IntDef(prefix = {"SERVICE_CLASS_"}, value = {
                SERVICE_CLASS_VOICE,
                SERVICE_CLASS_VIDEO,
        })
        @Retention(RetentionPolicy.SOURCE)
        public @interface ServiceClass {}

        private final @Condition int mCondition;
        private final @ServiceClass int mServiceClass;

        public CbData(int condition, int serviceClass, int status) {
            super(SS_TYPE_CB, status);
            mCondition = condition;
            mServiceClass = serviceClass;
        }

        public @Condition int getCondition() {
            return mCondition;
        }

        public @ServiceClass int getServiceClass() {
            return mServiceClass;
        }
    }

    final class OipData extends SupplementaryServiceConfiguration {
        public OipData(int status) {
            super(SS_TYPE_OIP, status);
        }
    }

    final class OirData extends SupplementaryServiceConfiguration {
        public OirData(int status) {
            super(SS_TYPE_OIR, status);
        }
    }

    final class TipData extends SupplementaryServiceConfiguration {
        public TipData(int status) {
            super(SS_TYPE_TIP, status);
        }
    }

    final class TirData extends SupplementaryServiceConfiguration {
        public TirData(int status) {
            super(SS_TYPE_TIR, status);
        }
    }

    interface TerminalBasedSupplementaryServiceConfigurationChangeListener {
        /**
         * Indicates that the terminal-based supplementary service configurations are updated by the
         * user.
         *
         * @param data A list of {@link SupplementaryServiceConfiguration}.
         */
        void onSupplementaryServiceConfigurationChanged(
                List<SupplementaryServiceConfiguration> data);
    }
}
