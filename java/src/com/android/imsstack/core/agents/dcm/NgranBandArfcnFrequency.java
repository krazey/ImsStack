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

import android.telephony.AccessNetworkConstants.NgranBands;
import android.telephony.ServiceState;

public enum NgranBandArfcnFrequency {

    /** 3GPP TS 38.101-1, Version 16.13.0, Table 5.2-1 and 5.4.2.3-1: FR1 bands */
    NR_BAND_1(NgranBands.BAND_1, 422000, 434000, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_2(NgranBands.BAND_2, 386000, 398000, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_3(NgranBands.BAND_3, 361000, 376000, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_5(NgranBands.BAND_5, 173800, 178800, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_7(NgranBands.BAND_7, 524000, 538000, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_8(NgranBands.BAND_8, 185000, 192000, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_12(NgranBands.BAND_12, 145800, 149200, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_14(NgranBands.BAND_14, 151600, 153600, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_18(NgranBands.BAND_18, 172000, 175000, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_20(NgranBands.BAND_20, 158200, 164200, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_25(NgranBands.BAND_25, 386000, 399000, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_26(NgranBands.BAND_26, 171800, 178800, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_28(NgranBands.BAND_28, 151600, 160600, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_29(NgranBands.BAND_29, 143400, 145600, ServiceState.DUPLEX_MODE_FDD), // SDL
    NR_BAND_30(NgranBands.BAND_30, 470000, 472000, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_34(NgranBands.BAND_34, 402000, 405000, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_38(NgranBands.BAND_38, 514000, 524000, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_39(NgranBands.BAND_39, 376000, 384000, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_40(NgranBands.BAND_40, 460000, 480000, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_41(NgranBands.BAND_41, 499200, 537999, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_46(NgranBands.BAND_46, 743334, 795000, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_47(47, 790334, 795000, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_48(NgranBands.BAND_48, 636667, 646666, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_50(NgranBands.BAND_50, 286400, 303400, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_51(NgranBands.BAND_51, 285400, 286400, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_53(NgranBands.BAND_53, 496700, 499000, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_65(NgranBands.BAND_65, 422000, 440000, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_66(NgranBands.BAND_66, 422000, 440000, ServiceState.DUPLEX_MODE_FDD), // same as 65
    NR_BAND_70(NgranBands.BAND_70, 399000, 404000, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_71(NgranBands.BAND_71, 123400, 130400, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_74(NgranBands.BAND_74, 295000, 303600, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_75(NgranBands.BAND_75, 286400, 303400, ServiceState.DUPLEX_MODE_FDD), // SDL
    NR_BAND_76(NgranBands.BAND_76, 285400, 286400, ServiceState.DUPLEX_MODE_FDD), // SDL
    NR_BAND_77(NgranBands.BAND_77, 620000, 680000, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_78(NgranBands.BAND_78, 620000, 653333, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_79(NgranBands.BAND_79, 693334, 733333, ServiceState.DUPLEX_MODE_FDD),
    // For SUL band 80, 81, 82, 83, 84, 86, and 89, they don't have downlink frequency
    NR_BAND_90(NgranBands.BAND_90, 499200, 538000, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_91(NgranBands.BAND_91, 285400, 286400, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_92(NgranBands.BAND_92, 286400, 303400, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_93(NgranBands.BAND_93, 285400, 286400, ServiceState.DUPLEX_MODE_FDD),
    NR_BAND_94(NgranBands.BAND_94, 286400, 286400, ServiceState.DUPLEX_MODE_FDD),
    // For SUL band 95, it doesn't have downlink frequency
    NR_BAND_96(NgranBands.BAND_96, 795000, 875000, ServiceState.DUPLEX_MODE_TDD),

    /** 3GPP TS 38.101-2, Version 16.13.0, Table 5.2-1 and 5.4.2.3-1: FR2 bands */
    NR_BAND_257(NgranBands.BAND_257, 2054166, 2104165, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_258(NgranBands.BAND_258, 2016667, 2070832, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_259(259, 2270833, 2337499, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_260(NgranBands.BAND_260, 2229166, 2279165, ServiceState.DUPLEX_MODE_TDD),
    NR_BAND_261(NgranBands.BAND_261, 2070833, 2084999, ServiceState.DUPLEX_MODE_TDD);

    private final int mBand;
    private final int mArfcnRangeFirst; // downlink
    private final int mArfcnRangeLast; // downlink
    private final int mDuplexMode;

    NgranBandArfcnFrequency(int band, int arfcnRangeFirst, int arfcnRangeLast, int duplexMode) {
        this.mBand = band;
        this.mArfcnRangeFirst = arfcnRangeFirst;
        this.mArfcnRangeLast = arfcnRangeLast;
        this.mDuplexMode = duplexMode;
    }

    public int getBand() {
        return mBand;
    }

    public int getArfcnRangeFirst() {
        return mArfcnRangeFirst;
    }

    public int getArfcnRangeLast() {
        return mArfcnRangeLast;
    }

    public int getDuplexMode() {
        return mDuplexMode;
    }
}
