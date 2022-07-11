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
package com.android.imsstack.core.carrier;

import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.ArrayMap;

import com.android.imsstack.core.agents.AgentFactory;
import com.android.imsstack.core.agents.Sim;
import com.android.imsstack.core.agents.SimInterface;
import com.android.imsstack.util.AppContext;
import com.android.imsstack.util.ImsPrivateProperties;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MSimUtils;
import com.android.imsstack.util.SODConfig;
import com.android.imsstack.util.SimCarrierId;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

/**
 * This class helps to load carrier codes from "config/carrier_code.xml".
 */
public final class CarrierCodeLoader {
    private static final String TAG = Log.TAG;
    private static final String CARRIER_CODE_FILE = "carrier_code.xml";
    private static final int NOT_MATCHED = -20;
    private static final int MATCH_GID = 4;
    private static final int MATCH_SPN = 3;
    private static final int MATCH_IMSI = 2;

    private static CarrierCodeLoader sCarrierCodeLoader = null;
    private static String sDefaultSimOperator = null;
    private XmlPullParserFactory mFactory;
    private final CarrierCode[] mCarrierCodes;
    private String mFileName = null;
    private boolean mIsAsset = false;

    /* package */ CarrierCodeLoader() {
        int phoneCount = MSimUtils.getPhoneCount();
        mCarrierCodes = new CarrierCode[phoneCount];

        for (int i = 0; i < phoneCount; i++) {
            mCarrierCodes[i] = null;
        }

        selectCarrierCodeFile();
    }

    public static CarrierCodeLoader getInstance() {
        if (sCarrierCodeLoader == null) {
            sCarrierCodeLoader = new CarrierCodeLoader();
        }

        return sCarrierCodeLoader;
    }

    public CarrierCode getCarrierCode(int slotId) {
        if (mCarrierCodes == null) {
            return null;
        }

        if (slotId < 0 || slotId >= mCarrierCodes.length) {
            return null;
        }

        return mCarrierCodes[slotId];
    }

    public CarrierCode fetchCarrierCode(SimCarrierId id, int slotId) {
        Log.d(TAG, "CarrierCode being fetched(" + slotId + ")");

        if (id == null) {
            return null;
        }

        // Check if the cached carrier code is the same with sim carrier id.
        CarrierCode carrierCode = getCarrierCode(slotId);

        if (carrierCode != null) {
            if (isSameCarrier(carrierCode, id)) {
                if (!id.isSimLocked() && TextUtils.isEmpty(carrierCode.getSimIccId())) {
                    carrierCode.setSimIccId(id.getIccId());
                }

                Log.i(TAG, "Cached CarrierCode: " + carrierCode);
                return carrierCode;
            }

            carrierCode = null;
        }

        if (Log.isDebuggable()) {
            Log.d(TAG, "CarrierCodeLoader :: start parsing with: " + id);
        }

        InputStream is = null;

        try {
            if (mIsAsset) {
                is = AppContext.getInstance().getAssets().open(mFileName);
            } else {
                is = new FileInputStream(mFileName);
            }

            synchronized (this) {
                if (mFactory == null) {
                    mFactory = XmlPullParserFactory.newInstance();
                }
            }

            XmlPullParser parser = mFactory.newPullParser();
            parser.setInput(is, "utf-8");

            carrierCode = getMatchedCarrierCodeFromXml(parser, id);

            if (carrierCode != null) {
                if (!id.isSimLocked()) {
                    carrierCode.setSimIccId(id.getIccId());
                }

                setCarrierCode(carrierCode, slotId);
            }
        } catch (IOException | XmlPullParserException e) {
            Log.e(TAG, "CarrierCodeLoader: " + e.toString());
        } finally {
            closeQuietly(is);
        }

        return carrierCode;
    }

    public static SimCarrierId getCarrierIdFromSim(int slotId) {
        boolean simLocked = false;

        SimInterface sim = AgentFactory.getInstance().getAgent(SimInterface.class, slotId);

        if (sim != null && sim.getSimState() == Sim.STATE_LOCKED) {
            simLocked = true;
        }

        return getCarrierIdFromSim(slotId, simLocked);
    }

    public static SimCarrierId getCarrierIdFromSim(int slotId, boolean simLocked) {
        int carrierId = SimCarrierId.UNKNOWN_ID;
        int specificCarrierId = SimCarrierId.UNKNOWN_ID;
        String mcc = "";
        String mnc = "";
        String imsi = "";
        String gid1 = "";
        String gid2 = "";
        String spn = "";
        String iccId = "";

        TelephonyManager tm = getTelephonyManager(slotId);

        if (tm != null) {
            if (simLocked) {
                iccId = tm.getSimSerialNumber();
            } else {
                int testCarrierId = ImsPrivateProperties.Persistent.getInt(
                        ImsPrivateProperties.Persistent.KEY_TEST_CARRIER_ID, slotId);
                carrierId = (testCarrierId > 0) ? testCarrierId : tm.getSimCarrierId();
                specificCarrierId = tm.getSimSpecificCarrierId();
                // todo: read SIM carrier id from Phone (IppImsPhoneProxy)
                String simOperator = tm.getSimOperator();

                if (simOperator != null && simOperator.length() >= 3) {
                    mcc = simOperator.substring(0, 3);
                    mnc = simOperator.substring(3);
                }

                imsi = tm.getSubscriberId();
                gid1 = tm.getGroupIdLevel1();
                spn = tm.getSimOperatorName();
                iccId = tm.getSimSerialNumber();
            }
        }

        return new SimCarrierId(carrierId, specificCarrierId,
                mcc, mnc, imsi, gid1, spn, iccId, simLocked);
    }

    public static String getDefaultSimOperator() {
        if (sDefaultSimOperator == null) {
            sDefaultSimOperator = getOperatorFrom2ndNtCode();

            if (SODConfig.equalsOperator("CCT", sDefaultSimOperator)) {
                sDefaultSimOperator = "VZW";
            }
        }

        return sDefaultSimOperator;
    }

    public static void setDefaultSimOperatorCountry(int slotId) {
        String simOperator = getDefaultSimOperator();

        if (!TextUtils.isEmpty(simOperator)) {
            Log.i(TAG, "CarrierCode: set a default operator(from 2nd NT code)");

            String simCountry = ImsProperties.TARGET_COUNTRY;

            if (SODConfig.equalsOperator("TRF_CLR", simOperator)) {
                simCountry = "PR";
            }

            CarrierCodeLoader.setSimOperatorCountry(simOperator, "", simCountry, slotId);
        }
    }

    public static void setSimOperatorCountry(CarrierCode cc, int slotId) {
        String operator = "";
        String operatorSub = "";
        String country = "";

        if (cc != null) {
            operator = cc.getOperatorFast();
            operatorSub = cc.getOperatorSub();
            country = cc.getCountry();
        }

        setSimOperatorCountry(operator, operatorSub, country, slotId);
    }

    public static void setSimOperatorCountry(
            String operator, String operatorSub, String country, int slotId) {
        Log.i(TAG, "CarrierCode(SimOperatorCountry): " + slotId +
                " - op=" + operator + ", opSub=" + operatorSub + ", co=" + country);

        operator = emptyIfNull(operator);
        operatorSub = emptyIfNull(operatorSub);
        country = emptyIfNull(country);

        ImsPrivateProperties.Persistent.set(
                ImsPrivateProperties.Persistent.KEY_SIM_OPERATOR,
                operator, slotId);
        ImsPrivateProperties.Persistent.set(
                ImsPrivateProperties.Persistent.KEY_SIM_OPERATOR_SUB,
                operatorSub, slotId);
        ImsPrivateProperties.Persistent.set(
                ImsPrivateProperties.Persistent.KEY_SIM_COUNTRY,
                country, slotId);
    }

    public static void setSimOperatorCountryOnSimLoaded(CarrierCode cc, int slotId) {
        String operator = "";
        String operatorSub = "";
        String country = "";

        if (cc != null) {
            operator = cc.getOperatorFast();
            operatorSub = cc.getOperatorSub();
            country = cc.getCountry();

            if (!TextUtils.isEmpty(cc.getOperatorLate())
                    && !Objects.equals(cc.getOperatorFast(), cc.getOperatorLate())) {
                Log.i(TAG, "CarrierCode: late profile updated");
                operator = cc.getOperatorLate();
            }
        }

        setSimOperatorCountry(operator, operatorSub, country, slotId);
    }

    public static void testParseCarrierCodes() {
        Log.d(TAG, "testParseCarrierCodes - starts");

        InputStream is = null;
        CarrierCodeLoader ccl = CarrierCodeLoader.getInstance();

        try {
            if (ccl.mIsAsset) {
                is = AppContext.getInstance().getAssets().open(ccl.mFileName);
            } else {
                is = new FileInputStream(ccl.mFileName);
            }

            XmlPullParser parser = XmlPullParserFactory.newInstance().newPullParser();
            parser.setInput(is, "utf-8");

            testDisplayCarrierCodes(parser);
        } catch (IOException | XmlPullParserException e) {
            Log.e(TAG, "CarrierCodeLoader: " + e.toString());
        } finally {
            closeQuietly(is);
        }

        Log.d(TAG, "testParseCarrierCodes - ends");
    }

    public static void testFetchCarrierCodes() {
        Log.d(TAG, "testFetchCarrierCodes - starts");

        CarrierCodeLoader ccl = CarrierCodeLoader.getInstance();

        SimCarrierId[] ids = {
                new SimCarrierId("204", "04", "", "BAE0000000000000", "", "891480000000000"),
                new SimCarrierId("204", "04", "", "", "Appalachian", "891130000000000"),
                new SimCarrierId("310", "120", "", "", "", "890112010000000"),
                new SimCarrierId("310", "120", "", "000001", "abc", "890112090000000"),
                new SimCarrierId("310", "240", "", "6d38", "abc", "890124000000000"),
                new SimCarrierId("310", "240", "", "6d38", "abc", "380112090000000"),
                new SimCarrierId("312", "670", "", "", "abc", "891267000000000"),
                new SimCarrierId("312", "670", "", "", "abc", "891266000000000"),
                new SimCarrierId("330", "110", "", "ddff", "", "890111000000000")
            };
        String[] logTags = {
                "CarrierCode(1: VZW/US)=",
                "CarrierCode(2: ACG/US)=",
                "CarrierCode(3: SPR/US)=",
                "CarrierCode(4: SPR/US)=",
                "CarrierCode(5: TMO/US - MPCS)=",
                "CarrierCode(6: TMO/US - MPCS(wrong))=",
                "CarrierCode(7: ATT/US - FirstNet)=",
                "CarrierCode(8: ATT/US - FirstNet(wrong))=",
                "CarrierCode(9: TRF_CLR/US)="
            };
        CarrierCode cc;

        try {
            for (int i = 0; i < ids.length; i++) {
                cc = ccl.fetchCarrierCode(ids[i], 0);
                Log.d(TAG, "" + logTags[i] + ((cc != null) ? cc.toString() : "(null)"));
            }
        } catch (Throwable t) {
            Log.d(TAG, t.toString());
        }

        Log.d(TAG, "testFetchCarrierCodes - ends");
    }

    private void selectCarrierCodeFile() {
        if (!TextUtils.isEmpty(mFileName)) {
            return;
        }

        // assets/CarrierCode/[CA|KR|US]
        mFileName = "CarrierCode/" + ImsProperties.TARGET_COUNTRY + "/";

        if (SODConfig.equalsOperator(ImsProperties.TARGET_OPERATOR, "TRF")) {
            mFileName += "TRF/";
        }

        mFileName += CARRIER_CODE_FILE;
        mIsAsset = true;

        if (Log.isDebuggable()) {
            Log.d(TAG, "CarrierCodeLoader: " + mFileName);
        }
    }

    private CarrierCode getMatchedCarrierCodeFromXml(XmlPullParser parser, SimCarrierId id)
            throws IOException, XmlPullParserException {
        CarrierCode carrierCode = null;
        int event;
        int score;

        while (((event = parser.next()) != XmlPullParser.END_DOCUMENT)) {
            if (event == XmlPullParser.START_TAG && "profile".equals(parser.getName())) {
                if ((score = checkFilters(parser, id)) <= 0) {
                    continue;
                }

                if (carrierCode == null) {
                    carrierCode = createCarrierCodeFromXml(parser, score);
                } else if (score > carrierCode.getScore()) {
                    CarrierCode oldCarrierCode = carrierCode;
                    carrierCode = createCarrierCodeFromXml(parser, score);

                    Log.i(TAG, "CarrierCode is changed: old=" + oldCarrierCode);
                    Log.i(TAG, "CarrierCode is changed: new=" + carrierCode);
                } else if (score == carrierCode.getScore()) {
                    CarrierCode newCarrierCode = createCarrierCodeFromXml(parser, score);
                    CarrierCode oldCarrierCode = carrierCode;

                    if (TextUtils.isEmpty(oldCarrierCode.getIccId())
                            && !TextUtils.isEmpty(newCarrierCode.getIccId())) {
                        // Do not replace the carrier code - iccId is not matched
                    } else {
                        carrierCode = newCarrierCode;

                        Log.i(TAG, "CarrierCode(ss) is changed: old=" + oldCarrierCode);
                        Log.i(TAG, "CarrierCode(ss) is changed: new=" + carrierCode);
                    }
                }
            }
        }

        return carrierCode;
    }

    private int checkFilters(XmlPullParser parser, SimCarrierId id) {
        int matches = 0;
        int mvnoType = CarrierCode.MVNO_TYPE_NONE;
        String iccId = "";

        for (int i = 0; i < parser.getAttributeCount(); i++) {
            String attribute = parser.getAttributeName(i);
            String value = parser.getAttributeValue(i);

            switch (attribute) {
            case CarrierCode.KEY_ICCID:
                iccId = value;
                break;
            case CarrierCode.KEY_MCC:
                matches += matchOnMcc(value, id);
                break;
            case CarrierCode.KEY_MNC:
                matches += matchOnMnc(value, id);
                break;
            case CarrierCode.KEY_MVNO_TYPE:
                if (CarrierCode.KEY_MVNO_TYPE_GID.equals(value)) {
                    mvnoType = CarrierCode.MVNO_TYPE_GID;
                } else if (CarrierCode.KEY_MVNO_TYPE_IMSI.equals(value)) {
                    mvnoType = CarrierCode.MVNO_TYPE_IMSI;
                } else if (CarrierCode.KEY_MVNO_TYPE_SPN.equals(value)) {
                    mvnoType = CarrierCode.MVNO_TYPE_SPN;
                }
                break;
            case CarrierCode.KEY_MVNO_MATCH_DATA:
                if (mvnoType == CarrierCode.MVNO_TYPE_GID) {
                    matches += matchOnGid1(value, id);
                } else if (mvnoType == CarrierCode.MVNO_TYPE_IMSI) {
                    matches += matchOnImsi(value, id);
                } else if (mvnoType == CarrierCode.MVNO_TYPE_SPN) {
                    matches += matchOnSpn(value, id);
                }
                break;
            case CarrierCode.KEY_CARRIER_CODE_FAST: // FALL-THROUGH
            case CarrierCode.KEY_CARRIER_CODE_LATE: // FALL-THROUGH
            case CarrierCode.KEY_CARRIER_CODE_SUB: // FALL-THROUGH
            case CarrierCode.KEY_COUNTRY: // FALL-THROUGH
            case "carrier": // FALL-THROUGH
            case "suffix":
                // no-op
                break;
            default:
                Log.d(TAG, "Unknown attribute(" + i + "): " + attribute + "=" + value);
                break;
            }
        }

        if (mvnoType == CarrierCode.MVNO_TYPE_NONE) {
            matches += matchOnIccId(iccId, id, 0);
        } else {
            matches += matchOnIccId(iccId, id, NOT_MATCHED);
        }

        return matches;
    }

    private boolean isSameCarrier(CarrierCode carrierCode, SimCarrierId id) {
        String simIccId = carrierCode.getSimIccId();

        if (!TextUtils.isEmpty(simIccId)
                && simIccId.equals(id.getIccId())) {
            return true;
        }

        int matches = 0;

        matches += matchOnIccId(carrierCode.getIccId(), id, NOT_MATCHED);
        matches += matchOnMcc(carrierCode.getMcc(), id);
        matches += matchOnMnc(carrierCode.getMnc(), id);

        switch (carrierCode.getMvnoType()) {
        case CarrierCode.KEY_MVNO_TYPE_GID:
            matches += matchOnGid1(carrierCode.getMvnoMatchData(), id);
            break;
        case CarrierCode.KEY_MVNO_TYPE_IMSI:
            matches += matchOnImsi(carrierCode.getMvnoMatchData(), id);

            if (!TextUtils.isEmpty(id.getGid1())
                    || !TextUtils.isEmpty(id.getSpn())) {
                // Re-match with SimCarrierId
                matches += NOT_MATCHED;
            }
            break;
        case CarrierCode.KEY_MVNO_TYPE_SPN:
            matches += matchOnSpn(carrierCode.getMvnoMatchData(), id);

            if (!TextUtils.isEmpty(id.getGid1())) {
                // Re-match with SimCarrierId
                matches += NOT_MATCHED;
            }
            break;
        default:
            if (!TextUtils.isEmpty(id.getGid1())
                    || !TextUtils.isEmpty(id.getSpn())) {
                // Re-match with SimCarrierId
                matches += NOT_MATCHED;
            }
            break;
        }

        return matches > 0;
    }

    private void setCarrierCode(CarrierCode carrierCode, int slotId) {
        if (mCarrierCodes == null) {
            return;
        }

        if (slotId < 0 || slotId >= mCarrierCodes.length) {
            return;
        }

        Log.i(TAG, "CarrierCode: old=" + mCarrierCodes[slotId] + ", new=" + carrierCode);

        mCarrierCodes[slotId] = carrierCode;
    }

    private static CarrierCode createCarrierCodeFromXml(XmlPullParser parser, int score) {
        String operatorFast = "";
        String operatorLate = "";
        String operatorSub = "";
        String country = "";
        String iccId = "";
        String mcc = "";
        String mnc = "";
        String mvnoType = "";
        String mvnoMatchData = "";

        for (int i = 0; i < parser.getAttributeCount(); i++) {
            String attribute = parser.getAttributeName(i);
            String value = parser.getAttributeValue(i);

            switch (attribute) {
                case CarrierCode.KEY_ICCID:
                    iccId = emptyIfNull(value);
                    break;
                case CarrierCode.KEY_MCC:
                    mcc = emptyIfNull(value);
                    break;
                case CarrierCode.KEY_MNC:
                    mnc = emptyIfNull(value);
                    break;
                case CarrierCode.KEY_MVNO_TYPE:
                    mvnoType = emptyIfNull(value);
                    break;
                case CarrierCode.KEY_MVNO_MATCH_DATA:
                    mvnoMatchData = emptyIfNull(value);
                    break;
                case CarrierCode.KEY_CARRIER_CODE_FAST:
                    operatorFast = emptyIfNull(value);
                    break;
                case CarrierCode.KEY_CARRIER_CODE_LATE:
                    operatorLate = emptyIfNull(value);
                    break;
                case CarrierCode.KEY_CARRIER_CODE_SUB:
                    operatorSub = emptyIfNull(value);
                    break;
                case CarrierCode.KEY_COUNTRY:
                    country = emptyIfNull(value);
                    break;
                case "carrier": // FALL-THROUGH
                case "suffix":
                    // no-op
                    break;
                default:
                    Log.d(TAG, "Unknown attribute(" + i + "): " + attribute + "=" + value);
                    break;
            }
        }

        return new CarrierCode(operatorFast, operatorLate, operatorSub,
                country, iccId, mcc, mnc, mvnoType, mvnoMatchData, score);
    }

    private static boolean existFile(String path, String name) {
        try {
            File file = new File(path, name);
            return file.exists();
        } catch (Exception e) {
            e.printStackTrace();
        }

        return false;
    }

    private static TelephonyManager getTelephonyManager(int slotId) {
        if (MSimUtils.isMultiSimEnabled()) {
            int subId = MSimUtils.getSubId(slotId);
            if (MSimUtils.isValidSubId(subId)) {
                return AppContext.getTelephonyManager(subId);
            }

            return null;
        }

        return AppContext.getTelephonyManager();
    }

    private static int matchOnGid1(String xmlGid, SimCarrierId id) {
        int result = 0;
        String gid = id.getGid1();

        if (TextUtils.isEmpty(xmlGid)) {
            if (!TextUtils.isEmpty(gid)) {
                result = NOT_MATCHED;
            }
        } else {
            if (TextUtils.isEmpty(gid)) {
                if (id.isSimLocked()) {
                    // Ignore: sim-locked
                } else {
                    result = NOT_MATCHED;
                }
            } else {
                result = gid.equalsIgnoreCase(xmlGid) ? MATCH_GID : NOT_MATCHED;
            }
        }

        return result;
    }

    private static int matchOnIccId(String xmlIccId, SimCarrierId id, int defaultNotMatched) {
        int result = 0;

        if (TextUtils.isEmpty(xmlIccId)) {
            // Ignore: no iccid in carrier code xml
        } else {
            String iccId = id.getIccId();

            if (TextUtils.isEmpty(iccId)) {
                // Ignore: sim-absent or error
            } else {
                result = iccId.startsWith(xmlIccId) ? 1 : defaultNotMatched;
            }
        }

        return result;
    }

    private static int matchOnImsi(String xmlImsi, SimCarrierId id) {
        int result = 0;

        if (TextUtils.isEmpty(xmlImsi)) {
            // Ignore: no imsi in carrier code xml
        } else {
            String imsi = id.getImsi();

            if (TextUtils.isEmpty(imsi)) {
                // Ignore: sim-absent or sim-locked or error
            } else {
                String regexXmlImsi = xmlImsi.replace('x', '.');
                Pattern imsiPattern = Pattern.compile(regexXmlImsi, Pattern.CASE_INSENSITIVE);
                Matcher matcher = imsiPattern.matcher(imsi);

                result = matcher.matches() ? MATCH_IMSI : NOT_MATCHED;
            }
        }

        return result;
    }

    private static int matchOnMcc(String xmlMcc, SimCarrierId id) {
        int result = 0;

        if (TextUtils.isEmpty(xmlMcc)) {
            // Ignore: no mcc in carrier code xml
        } else {
            String mcc = id.getMcc();

            if (TextUtils.isEmpty(mcc)) {
                // Ignore: sim-absent or sim-locked or error
            } else {
                result = mcc.equals(xmlMcc) ? 1 : NOT_MATCHED;
            }
        }

        return result;
    }

    private static int matchOnMnc(String xmlMnc, SimCarrierId id) {
        int result = 0;

        if (TextUtils.isEmpty(xmlMnc)) {
            // Ignore: no mnc in carrier code xml
        } else {
            String mnc = id.getMnc();

            if (TextUtils.isEmpty(mnc)) {
                // Ignore: sim-absent or sim-locked or error
            } else {
                result = mnc.equals(xmlMnc) ? 1 : NOT_MATCHED;
            }
        }

        return result;
    }

    private static int matchOnSpn(String xmlSpn, SimCarrierId id) {
        int result = 0;
        String spn = id.getSpn();

        if (TextUtils.isEmpty(xmlSpn)) {
            if (!TextUtils.isEmpty(spn)) {
                result = NOT_MATCHED;
            }
        } else {
            if (TextUtils.isEmpty(spn)) {
                if (id.isSimLocked()) {
                    // Ignore: sim-locked
                } else {
                    result = NOT_MATCHED;
                }
            } else {
                Pattern spnPattern = Pattern.compile(xmlSpn, Pattern.CASE_INSENSITIVE);
                Matcher matcher = spnPattern.matcher(spn);
                result = matcher.matches() ? MATCH_SPN : NOT_MATCHED;
            }
        }

        return result;
    }

    private static void closeQuietly(AutoCloseable closeable) {
        if (closeable != null) {
            try {
                closeable.close();
            } catch (RuntimeException rethrown) {
               throw rethrown;
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }

    private static void testDisplayCarrierCodes(XmlPullParser parser)
            throws IOException, XmlPullParserException {
        List<CarrierCode> carrierCodes = new ArrayList<>();
        int event;

        while (((event = parser.next()) != XmlPullParser.END_DOCUMENT)) {
            if (event == XmlPullParser.START_TAG && "profile".equals(parser.getName())) {
               carrierCodes.add(createCarrierCodeFromXml(parser, 0));
            }
        }

        for (int i = 0; i < carrierCodes.size(); i++) {
            CarrierCode cc = carrierCodes.get(i);
            Log.d(TAG, "CarrierCode(" + i + ")=" + cc.toString());
        }
    }

    // NT_CODE {
    private static String getNtCodes() {
        Path ntCodeListFile = Paths.get("/data/local/ntcode_list");
        String ntCodes = "";

        try {
            List<String> lines = Files.readAllLines(ntCodeListFile, StandardCharsets.UTF_8);
            for (String line : lines) {
                ntCodes += line;
            }
        } catch (Throwable t) {
            Log.w(TAG, "Reading NT code failed");
        }

        return ntCodes;
    }

    private static String getOperatorFrom2ndNtCode() {
        String ntCodes = getNtCodes();

        if (TextUtils.isEmpty(ntCodes)) {
            return "";
        }
        int index = ntCodes.indexOf("\"100,10F,");

        if (index < 0) {
            return "";
        }

        // Length of ("100,10F,FFFFFFFF,FFFFFFFF,)
        final int testNtCodePrefixLen = 27;
        String ntCodeSubset = null;

        try {
            ntCodeSubset = ntCodes.substring(index + testNtCodePrefixLen,
                    index + testNtCodePrefixLen + 2);
        } catch (IndexOutOfBoundsException e) {
            e.printStackTrace();
        }

        if (TextUtils.isEmpty(ntCodeSubset)) {
            return "";
        }

        ArrayMap<String, String> operatorMap = new ArrayMap<>();
        operatorMap.put("01", "VZW");
        operatorMap.put("02", "ATT");
        operatorMap.put("03", "TMO");
        operatorMap.put("04", "SPR");
        operatorMap.put("05", "USC");
        operatorMap.put("06", "LRA");
        operatorMap.put("07", "ACG");
        operatorMap.put("08", "TRF_ATT");
        operatorMap.put("09", "TRF_TMO");
        operatorMap.put("10", "TRF_VZW");
        operatorMap.put("11", "TRF_SM");
        operatorMap.put("12", "TRF_CLR");
        operatorMap.put("13", "TRF_WFM");
        operatorMap.put("14", "CRK");
        operatorMap.put("15", "CCT");
        operatorMap.put("21", "RGS");
        operatorMap.put("22", "BELL");
        operatorMap.put("23", "TLS");
        operatorMap.put("24", "VTR");

        String operator = operatorMap.getOrDefault(ntCodeSubset, "");

        Log.d(TAG, "CarrierCode: operator(NT-code)=" + operator);

        return operator;
    }
    // }

    private static String emptyIfNull(String s) {
        return s == null ? "" : s;
    }
}
