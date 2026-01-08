/*
 * Copyright (C) 2025 The Android Open Source Project
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

import android.telephony.AccessNetworkConstants.EutranBand;
import android.telephony.ServiceState;

/**
 * 3GPP TS 36.101 Table 5.7.3-1 E-UTRA channel numbers.
 */
public enum EutranBandArfcnFrequency {
        EUTRAN_BAND_1(EutranBand.BAND_1, 0, 599, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_2(EutranBand.BAND_2, 600, 1199, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_3(EutranBand.BAND_3, 1200, 1949, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_4(EutranBand.BAND_4, 1950, 2399, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_5(EutranBand.BAND_5, 2400, 2649, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_6(EutranBand.BAND_6, 2650, 2749, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_7(EutranBand.BAND_7, 2750, 3449, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_8(EutranBand.BAND_8, 3450, 3799, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_9(EutranBand.BAND_9, 3800, 4149, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_10(EutranBand.BAND_10, 4150, 4749, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_11(EutranBand.BAND_11, 4750, 4949, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_12(EutranBand.BAND_12, 5010, 5179, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_13(EutranBand.BAND_13, 5180, 5279, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_14(EutranBand.BAND_14, 5280, 5379, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_17(EutranBand.BAND_17, 5730, 5849, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_18(EutranBand.BAND_18, 5850, 5999, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_19(EutranBand.BAND_19, 6000, 6149, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_20(EutranBand.BAND_20, 6150, 6449, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_21(EutranBand.BAND_21, 6450, 6599, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_22(EutranBand.BAND_22, 6600, 7399, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_23(EutranBand.BAND_23, 7500, 7699, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_24(EutranBand.BAND_24, 7700, 8039, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_25(EutranBand.BAND_25, 8040, 8689, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_26(EutranBand.BAND_26, 8690, 9039, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_27(EutranBand.BAND_27, 9040, 9209, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_28(EutranBand.BAND_28, 9210, 9659, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_30(EutranBand.BAND_30, 9770, 9869, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_31(EutranBand.BAND_31, 9870, 9919, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_33(EutranBand.BAND_33, 36000, 36199, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_34(EutranBand.BAND_34, 36200, 36349, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_35(EutranBand.BAND_35, 36350, 36949, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_36(EutranBand.BAND_36, 36950, 37549, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_37(EutranBand.BAND_37, 37550, 37749, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_38(EutranBand.BAND_38, 37750, 38249, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_39(EutranBand.BAND_39, 38250, 38649, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_40(EutranBand.BAND_40, 38650, 39649, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_41(EutranBand.BAND_41, 39650, 41589, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_42(EutranBand.BAND_42, 41590, 43589, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_43(EutranBand.BAND_43, 43590, 45589, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_44(EutranBand.BAND_44, 45590, 46589, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_45(EutranBand.BAND_45, 46590, 46789, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_46(EutranBand.BAND_46, 46790, 54539, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_47(EutranBand.BAND_47, 54540, 55239, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_48(EutranBand.BAND_48, 55240, 56739, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_49(EutranBand.BAND_49, 56740, 58239, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_50(EutranBand.BAND_50, 58240, 59089, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_51(EutranBand.BAND_51, 59090, 59139, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_52(EutranBand.BAND_52, 59140, 60139, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_53(EutranBand.BAND_53, 60140, 60254, ServiceState.DUPLEX_MODE_TDD),
        EUTRAN_BAND_65(EutranBand.BAND_65, 65536, 66435, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_66(EutranBand.BAND_66, 66436, 67335, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_68(EutranBand.BAND_68, 67536, 67835, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_70(EutranBand.BAND_70, 68336, 68585, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_71(EutranBand.BAND_71, 68586, 68935, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_72(EutranBand.BAND_72, 68936, 68985, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_73(EutranBand.BAND_73, 68986, 69035, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_74(EutranBand.BAND_74, 69036, 69465, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_85(EutranBand.BAND_85, 70366, 70545, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_87(EutranBand.BAND_87, 70546, 70595, ServiceState.DUPLEX_MODE_FDD),
        EUTRAN_BAND_88(EutranBand.BAND_88, 70596, 70645, ServiceState.DUPLEX_MODE_FDD);

    private final int mBand;
    private final int mDownlinkOffset;
    private final int mDownlinkRange;
    private final int mDuplexMode;

    EutranBandArfcnFrequency(int band, int downlinkOffset, int downlinkRange, int duplexMode) {
        this.mBand = band;
        this.mDownlinkOffset = downlinkOffset;
        this.mDownlinkRange = downlinkRange;
        this.mDuplexMode = duplexMode;
    }

    public int getBand() {
        return mBand;
    }

    public int getDownlinkOffset() {
        return mDownlinkOffset;
    }

    public int getDownlinkRange() {
        return mDownlinkRange;
    }

    public int getDuplexMode() {
        return mDuplexMode;
    }

    /**
     * Check whether a specific EARFCN (E-UTRA Absolute Radio Frequency Channel Number) is
     * within the downlink frequency range of this band.
     *
     * @param earfcn The EARFCN value to be checked.
     * @return Returns {@code true} if the given {@code earfcn} is within the range,
     *         otherwise {@code false}.
     */
    public boolean isEarfcnInRange(int earfcn) {
        return earfcn >= this.getDownlinkOffset() && earfcn <= this.getDownlinkRange();
    }
}
