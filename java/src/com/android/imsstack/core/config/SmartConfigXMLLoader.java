package com.android.imsstack.core.config;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.text.TextUtils;

import com.android.imsstack.util.DBUtils;
import com.android.imsstack.util.ImsConstants;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.Log;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NodeList;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

public class SmartConfigXMLLoader {
    private static final String TAG = "ImsStack_SmartConfigXMLLoader";

    private static final String SMART_CONFIG_TYPE = "SmartConfiguration";
    private static final String XML_ROOT_IN_ASSETS = "SmartConfiguration";

    private XMLFile mSmartXmlFile;

    private NodeList mNodes = null;
    private String[] mTableId = null;

    private Context mContext = null;

    public SmartConfigXMLLoader() {
    }

    public void init(Context context) {
        mContext = context;

        mSmartXmlFile = new XMLFile();
        mSmartXmlFile.setFilename(XML_ROOT_IN_ASSETS, SMART_CONFIG_TYPE, "", "", "", "");
    }

    public boolean isConfigChanged(SQLiteDatabase db) {
        if (!DBUtils.DB.hasTable(db,
                SmartConfigProviderInterface.SmartModifiedTime.TABLE_NAME)) {
            Log.i(TAG, SmartConfigProviderInterface.SmartModifiedTime.TABLE_NAME
                    + " table is not existed -> DB will be updated");
            return true;
        }

        if (DBUtils.DB.getRowCount(db,
                SmartConfigProviderInterface.SmartModifiedTime.TABLE_NAME) <= 0) {
            Log.i(TAG, SmartConfigProviderInterface.SmartModifiedTime.TABLE_NAME
                    + " table is empty -> DB will be updated");
            return true;
        }

        if (isSwVersionChanged(db)) {
            Log.w(TAG, "SW version changed -> DB will be re-created");
            return true;
        }

        int modifiedInDB = DBUtils.DB.getInt(db,
                SmartConfigProviderInterface.SmartModifiedTime.TABLE_NAME, null,
                SmartConfigProviderInterface.SmartModifiedTime.XML_MODIFIED_TIME, 0);
        int modifiedInXML = selectXmlResource(mSmartXmlFile);

        Log.i(TAG, "SmartConfigDB :: ModifiedTime(DB,XML) - " + modifiedInDB + ", " + modifiedInXML);

        if (modifiedInXML != modifiedInDB) {
            Log.w(TAG, "xml-file is the latest one in "
                + mSmartXmlFile.folder() + "/" + mSmartXmlFile.filename()
                + "-> DB will be updated");
            return true;
        }

        return false;
    }

    private boolean isSwVersionChanged(SQLiteDatabase db) {
        String swversionInDB = DBUtils.DB.getString(db ,
                 SmartConfigProviderInterface.SmartModifiedTime.TABLE_NAME , null ,
                 SmartConfigProviderInterface.SmartModifiedTime.SW_VERSION , "");

        if (TextUtils.isEmpty(swversionInDB)) {
            Log.w(TAG, "SmartConfigDB :: No SW_VERSION in DB");
            return true;
        }

        String swversion = ImsProperties.SW_VERSION_3CHARS;

        if (TextUtils.isEmpty(swversion)) {
            Log.w(TAG, "SmartConfigDB :: No SW_VERSION in device");
            return false;
        }

        if (!swversionInDB.equals(swversion)) {
            Log.w(TAG, "SmartConfigDB :: SW_VERSION - " + swversionInDB + " >> " + swversion);
            return true;
        }

        return false;
    }

    private int selectXmlResource(XMLFile xml_file) {
        boolean bExternal = xml_file.existInExternal(ImsConstants.PATH_XML);
        boolean bAsset = xml_file.exist();

        if (!bExternal && !bAsset) {
            Log.i(TAG, "File(" + xml_file.filename() + ") is not found");
            return -1;
        }

        int modifiedInExternal = -1;
        int modifiedInAsset = -1;

        if (bExternal) {
            InputStream isExternal = openXmlFileInExternal(
                    ImsConstants.PATH_XML, xml_file.filename());
            if (isExternal != null) {
                modifiedInExternal = getModifiedTime(isExternal);
                closeXmlFile(isExternal);
            }
        }

        if (bAsset) {
            InputStream isAsset = openXmlFileInAssets(xml_file.folder(), xml_file.filename());
            if (isAsset != null) {
                modifiedInAsset = getModifiedTime(isAsset);
                closeXmlFile(isAsset);
            }
        }

        Log.i(TAG, "SmartConfigXML(" + xml_file.filename() + ") :: modifiedTime(asset|ext)=["
            + modifiedInAsset + "|" + modifiedInExternal + "]");

        if (modifiedInExternal > modifiedInAsset) {
            xml_file.setFolder(ImsConstants.PATH_XML);
            xml_file.setExternal(true);
            return modifiedInExternal;
        }

        xml_file.setFolder(XML_ROOT_IN_ASSETS);
        xml_file.setExternal(false);

        return modifiedInAsset;
    }

    // New APIs
    private int getModifiedTime(InputStream is) {
        NodeList nodes = getNode(is);
        if (nodes == null) {
            Log.w(TAG, "getModifiedTime :: NodeList is null");
            return -1;
        }

        String[] tables = getTableNames(nodes);
        if (tables == null) {
            Log.w(TAG, "getModifiedTime :: No tables");
            return -1;
        }

        String strModifiedTime = "";

        for (int i = 0; i < tables.length; i++) {
            if (!tables[i].equals(SmartConfigProviderInterface.SmartModifiedTime.TABLE_NAME)) {
                continue;
            }

            Element element = (Element)nodes.item(i);
            NodeList param = (element != null) ? element.getElementsByTagName("param") : null;
            Element line = (param != null) ? (Element)param.item(0) : null;

            if (line == null) {
                Log.e(TAG, "No element :: element="
                        + element + ", param=" + param + ", line=" + line);
                break;
            }

            strModifiedTime = line.getAttribute(
                    SmartConfigProviderInterface.SmartModifiedTime.XML_MODIFIED_TIME);
            break;
        }

        int modifiedTime = -1;

        try {
            modifiedTime = Integer.parseInt(strModifiedTime);
        } catch (Exception e) {
            Log.w(TAG, "getModifiedTime :: " + e.toString());
        }

        if (Log.isDebuggable()) {
            Log.d(TAG, "modifiedTime=" + modifiedTime);
        }

        return modifiedTime;
    }

    private static NodeList getNode(InputStream is) {
        try {
            if (is == null) {
                Log.e(TAG, "InputStream is null");
                return null;
            }

            Document doc = getDocument(is);
            if (doc == null) {
                Log.e(TAG, "Document is null");
                return null;
            }

            Element order = doc.getDocumentElement();
            NodeList nodes = order.getElementsByTagName("table");

            return nodes;
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "getNode :: " + e.toString());
        }

        return null;
    }

    private String[] getTableNames(NodeList nodes) {
        String[] tables = new String[nodes.getLength()];

        for (int i = 0; i < nodes.getLength(); i++) {

            Element element = (Element)nodes.item(i);

            if (element == null) {
                Log.e(TAG, "element is null");
                return null;
            }

            tables[i] = element.getAttribute("id");
        }

        return tables;
    }
    // End of New APIs

    private InputStream openXmlFileInExternal(String ext_folder, String filename) {
        InputStream in = null;
        String strFullName = ((ext_folder.length() > 0) ? (ext_folder + "/") : "") + filename;

        try {
            File xml_file = new File(strFullName);
            Log.i(TAG, "SmartConfigXML(Ext) :: " + xml_file.getAbsolutePath());
            in = new FileInputStream(xml_file);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        }

        return in;
    }

    private InputStream openXmlFileInAssets(String folder, String filename) {
        InputStream in = null;
        String strFullName = ((folder.length() > 0) ? (folder + "/") : "") + filename;

        try {
            Log.i(TAG, "SmartConfigXML(Asset) :: " + strFullName);
            in = mContext.getAssets().open(strFullName);
        } catch (IOException e) {
            e.printStackTrace();
        }

        return in;
    }

    private void closeXmlFile(InputStream in) {
        try {
            if (in != null) {
                in.close();
            }
        } catch (IOException ioe) {
            ioe.printStackTrace();
        }
    }


    public String[] getTableNames() {
        if (mNodes == null) {
            mNodes = getNode();
        }

        if (mNodes == null) {
            Log.e(TAG, "nodes is null");
            return null;
        }

        mTableId = new String[mNodes.getLength()];

        for (int i = 0; i < mNodes.getLength(); i++) {

            Element element = (Element)mNodes.item(i);

            if (element == null) {
                Log.e(TAG, "element is null");
                return null;
            }

            mTableId[i] = element.getAttribute("id");
        }

        return mTableId;
    }

    public String[][][] getTableElements(int i) {
        if (mNodes == null) {
            mNodes = getNode();
        }

        if (mNodes == null) {
            Log.e(TAG, "nodes is null");
            return null;
        }

        Element element = (Element)mNodes.item(i);

        if (element == null) {
            Log.e(TAG, "element is null");
            return null;
        }

        String strAttribute = element.getAttribute("id");
        if (SmartConfigProviderInterface.SmartModifiedTime.TABLE_NAME.equals(strAttribute)) {
            return getTableElementsFromSmartModifiedTime(element);
        } else if (SmartConfigProviderInterface.OperatorList.TABLE_NAME.equals(strAttribute)) {
            return getTableElementsFromOperatorList(element);
        } else if (SmartConfigProviderInterface.MccMncList.TABLE_NAME.equals(strAttribute)) {
            return getTableElementsFromMccMncList(element);
        } else if (SmartConfigProviderInterface.NtCodeList.TABLE_NAME.equals(strAttribute)) {
            return getTableElementsFromNtCodeList(element);
        } else if (SmartConfigProviderInterface.ServiceAcceptanceList.TABLE_NAME.equals(strAttribute)) {
            return getTableElementsFromServiceRestrictionList(element);
        } else if (SmartConfigProviderInterface.ServiceEnableListByToTc.TABLE_NAME.equals(strAttribute)) {
            return getTableElementsFromServiceEnableListByToTc(element);
        } else if (SmartConfigProviderInterface.ServiceEnableListByNtCode.TABLE_NAME.equals(strAttribute)) {
            return getTableElementsFromServiceEnableListByNtCode(element);
        } else if (SmartConfigProviderInterface.NaoCarrierList.TABLE_NAME.equals(strAttribute)) {
            return getTableElementsFromNaoCarrierList(element);
        }

        return null;
    }

    private String[][][] getTableElementsFromSmartModifiedTime(Element element) {
        NodeList param = element.getElementsByTagName("param");

        if (param == null) {
            Log.e(TAG, "param is null");
            return null;
        }

        int param_length = param.getLength();
        int attribute_length = SmartConfigProviderInterface.SmartModifiedTime.ATTR_LENGTH;
        String paramId = "-1";
        String paramProperty = "-1";
        String paramXmlFile = "-1";
        String paramModifiedTime = "-1";
        String paramSwVersion = "-1";
        String paramVersion = "-1";

        String array[][][] = new String[param_length][attribute_length][2];

        for (int i = 0; i < param_length; i++) {
            Element line = (Element)param.item(i);
            paramId = line.getAttribute(SmartConfigProviderInterface.SmartModifiedTime.ID);
            paramProperty = line.getAttribute(
                        SmartConfigProviderInterface.SmartModifiedTime.PROPERTY);
            paramXmlFile = line.getAttribute(
                        SmartConfigProviderInterface.SmartModifiedTime.XML_FILE);
            paramModifiedTime = line.getAttribute(
                    SmartConfigProviderInterface.SmartModifiedTime.XML_MODIFIED_TIME);
            paramSwVersion = ImsProperties.SW_VERSION_3CHARS;
            paramVersion = line.getAttribute(
                    SmartConfigProviderInterface.SmartModifiedTime.VERSION);

            array[i][0][0] = SmartConfigProviderInterface.SmartModifiedTime.ID;
            array[i][0][1] = paramId;
            array[i][1][0] = SmartConfigProviderInterface.SmartModifiedTime.PROPERTY;
            array[i][1][1] = paramProperty;
            array[i][2][0] = SmartConfigProviderInterface.SmartModifiedTime.XML_FILE;
            array[i][2][1] = paramXmlFile;
            array[i][3][0] = SmartConfigProviderInterface.SmartModifiedTime.XML_MODIFIED_TIME;
            array[i][3][1] = paramModifiedTime;
            array[i][4][0] = SmartConfigProviderInterface.SmartModifiedTime.SW_VERSION;
            array[i][4][1] = String.valueOf(paramSwVersion);
            array[i][5][0] = SmartConfigProviderInterface.SmartModifiedTime.VERSION;
            array[i][5][1] = paramVersion;
        }

        return array;
    }

    private String[][][] getTableElementsFromOperatorList(Element element) {
        NodeList param = element.getElementsByTagName("param");

        if (param == null) {
            Log.e(TAG, "param is null");
            return null;
        }

        int param_length = param.getLength();
        int attribute_length = 10;
        String paramOperator = "-1";
        String paramCountry = "-1";
        String paramBrand = "-1";
        String paramRegion = "-1";
        String paramGroupId = "-1";
        String paramCategory = "-1";
        String paramOperatorBasedOn = "-1";
        String paramEnablerType = "-1";
        String paramSupportSimMoved = "-1";
        String paramConfigPerModel = "-1";

        String array[][][] = new String[param_length][attribute_length][2];

        for (int i = 0; i < param_length; i++) {
            Element line = (Element)param.item(i);
            paramOperator = line.getAttribute(
                        SmartConfigProviderInterface.OperatorList.OPERATOR);
            paramCountry = line.getAttribute(
                        SmartConfigProviderInterface.OperatorList.COUNTRY);
            paramRegion = line.getAttribute(
                        SmartConfigProviderInterface.OperatorList.REGION);
            paramCategory = line.getAttribute(
                        SmartConfigProviderInterface.OperatorList.CATEGORY);
            paramGroupId = line.getAttribute(
                        SmartConfigProviderInterface.OperatorList.GROUP_ID);
            paramBrand = line.getAttribute(SmartConfigProviderInterface.OperatorList.BRAND);
            paramOperatorBasedOn = line.getAttribute(
                        SmartConfigProviderInterface.OperatorList.OPERATOR_BASED_ON);
            paramEnablerType = line.getAttribute(
                        SmartConfigProviderInterface.OperatorList.ENABLER_TYPE);
            paramSupportSimMoved = line.getAttribute(
                        SmartConfigProviderInterface.OperatorList.SUPPORT_SIM_MOVED);
            paramConfigPerModel = line.getAttribute(
                        SmartConfigProviderInterface.OperatorList.CONFIG_PER_MODEL);

            array[i][0][0] = SmartConfigProviderInterface.OperatorList.OPERATOR;
            array[i][0][1] = paramOperator;
            array[i][1][0] = SmartConfigProviderInterface.OperatorList.COUNTRY;
            array[i][1][1] = paramCountry;
            array[i][2][0] = SmartConfigProviderInterface.OperatorList.REGION;
            array[i][2][1] = paramRegion;
            array[i][3][0] = SmartConfigProviderInterface.OperatorList.CATEGORY;
            array[i][3][1] = paramCategory;
            array[i][4][0] = SmartConfigProviderInterface.OperatorList.GROUP_ID;
            array[i][4][1] = paramGroupId;
            array[i][5][0] = SmartConfigProviderInterface.OperatorList.BRAND;
            array[i][5][1] = paramBrand;
            array[i][6][0] = SmartConfigProviderInterface.OperatorList.OPERATOR_BASED_ON;
            array[i][6][1] = paramOperatorBasedOn;
            array[i][7][0] = SmartConfigProviderInterface.OperatorList.ENABLER_TYPE;
            array[i][7][1] = paramEnablerType;
            array[i][8][0] = SmartConfigProviderInterface.OperatorList.SUPPORT_SIM_MOVED;
            array[i][8][1] = paramSupportSimMoved;
            array[i][9][0] = SmartConfigProviderInterface.OperatorList.CONFIG_PER_MODEL;
            array[i][9][1] = paramConfigPerModel;
        }

        return array;
    }

    private String[][][] getTableElementsFromMccMncList(Element element) {
        NodeList param = element.getElementsByTagName("param");

        if (param == null) {
            Log.e(TAG, "param is null");
            return null;
        }

        int param_length = param.getLength();
        int attribute_length = 7;
        String paramMccMnc = "-1";
        String paramOperator = "-1";
        String paramCountry = "-1";
        String paramGid = "";
        String paramSpn = "";
        String paramImsi = "";
        String paramEnabled = "";

        String array[][][] = new String[param_length][attribute_length][2];

        for (int i = 0; i < param_length; i++) {
            Element line = (Element)param.item(i);
            paramMccMnc = line.getAttribute(SmartConfigProviderInterface.MccMncList.MCCMNC);
            paramOperator = line.getAttribute(
                        SmartConfigProviderInterface.MccMncList.OPERATOR);
            paramCountry = line.getAttribute(
                        SmartConfigProviderInterface.MccMncList.COUNTRY);
            paramGid = line.getAttribute(
                    SmartConfigProviderInterface.MccMncList.GID);
            paramSpn = line.getAttribute(
                    SmartConfigProviderInterface.MccMncList.SPN);
            paramImsi = line.getAttribute(
                    SmartConfigProviderInterface.MccMncList.IMSI);
            paramEnabled = line.getAttribute(
                    SmartConfigProviderInterface.MccMncList.ENABLED);

            array[i][0][0] = SmartConfigProviderInterface.MccMncList.MCCMNC;
            array[i][0][1] = paramMccMnc;
            array[i][1][0] = SmartConfigProviderInterface.MccMncList.OPERATOR;
            array[i][1][1] = paramOperator;
            array[i][2][0] = SmartConfigProviderInterface.MccMncList.COUNTRY;
            array[i][2][1] = paramCountry;
            array[i][3][0] = SmartConfigProviderInterface.MccMncList.GID;
            array[i][3][1] = paramGid;
            array[i][4][0] = SmartConfigProviderInterface.MccMncList.SPN;
            array[i][4][1] = paramSpn;
            array[i][5][0] = SmartConfigProviderInterface.MccMncList.IMSI;
            array[i][5][1] = paramImsi;
            array[i][6][0] = SmartConfigProviderInterface.MccMncList.ENABLED;
            array[i][6][1] = paramEnabled;
        }

        return array;
    }

    private String[][][] getTableElementsFromNtCodeList(Element element) {
        NodeList param = element.getElementsByTagName("param");

        if (param == null) {
            Log.e(TAG, "param is null");
            return null;
        }

        int param_length = param.getLength();
        int attribute_length = 6;
        String paramNtCode = "-1";
        String paramOperator = "-1";
        String paramCountry = "-1";
        String paramRegion = "-1";
        String paramCategory = "-1";
        String paramRestriction = "-1";

        String array[][][] = new String[param_length][attribute_length][2];

        for (int i = 0; i < param_length; i++) {
            Element line = (Element)param.item(i);
            paramNtCode = line.getAttribute(SmartConfigProviderInterface.NtCodeList.NTCODE);
            paramOperator = line.getAttribute(
                        SmartConfigProviderInterface.NtCodeList.OPERATOR);
            paramCountry = line.getAttribute(
                        SmartConfigProviderInterface.NtCodeList.COUNTRY);
            paramRegion = line.getAttribute(
                        SmartConfigProviderInterface.NtCodeList.REGION);
            paramCategory = line.getAttribute(
                    SmartConfigProviderInterface.NtCodeList.CATEGORY);
            paramRestriction = line.getAttribute(
                    SmartConfigProviderInterface.NtCodeList.RESTRICTION);

            array[i][0][0] = SmartConfigProviderInterface.NtCodeList.NTCODE;
            array[i][0][1] = paramNtCode;
            array[i][1][0] = SmartConfigProviderInterface.NtCodeList.OPERATOR;
            array[i][1][1] = paramOperator;
            array[i][2][0] = SmartConfigProviderInterface.NtCodeList.COUNTRY;
            array[i][2][1] = paramCountry;
            array[i][3][0] = SmartConfigProviderInterface.NtCodeList.REGION;
            array[i][3][1] = paramRegion;
            array[i][4][0] = SmartConfigProviderInterface.NtCodeList.CATEGORY;
            array[i][4][1] = paramCategory;
            array[i][5][0] = SmartConfigProviderInterface.NtCodeList.RESTRICTION;
            array[i][5][1] = paramRestriction;
        }

        return array;
    }

    private String[][][] getTableElementsFromServiceRestrictionList(Element element) {
        NodeList param = element.getElementsByTagName("param");

        if (param == null) {
            Log.e(TAG, "param is null");
            return null;
        }

        int param_length = param.getLength();
        int attribute_length = 4;
        String paramOperator = "-1";
        String paramCountry = "-1";
        String paramMcc = "-1";
        String paramMccmnc = "-1";

        String array[][][] = new String[param_length][attribute_length][2];

        for (int i = 0; i < param_length; i++) {
            Element line = (Element)param.item(i);
            paramOperator = line.getAttribute(
                        SmartConfigProviderInterface.ServiceAcceptanceList.OPERATOR);
            paramCountry = line.getAttribute(
                        SmartConfigProviderInterface.ServiceAcceptanceList.COUNTRY);
            paramMcc = line.getAttribute(
                        SmartConfigProviderInterface.ServiceAcceptanceList.MCC);
            paramMccmnc = line.getAttribute(SmartConfigProviderInterface.ServiceAcceptanceList.MCCMNC);

            array[i][0][0] = SmartConfigProviderInterface.ServiceAcceptanceList.OPERATOR;
            array[i][0][1] = paramOperator;
            array[i][1][0] = SmartConfigProviderInterface.ServiceAcceptanceList.COUNTRY;
            array[i][1][1] = paramCountry;
            array[i][2][0] = SmartConfigProviderInterface.ServiceAcceptanceList.MCC;
            array[i][2][1] = paramMcc;
            array[i][3][0] = SmartConfigProviderInterface.ServiceAcceptanceList.MCCMNC;
            array[i][3][1] = paramMccmnc;

        }

        return array;
    }

    private String[][][] getTableElementsFromServiceEnableListByToTc(Element element) {
        NodeList param = element.getElementsByTagName("param");

        if (param == null) {
            Log.e(TAG, "param is null");
            return null;
        }

        int param_length = param.getLength();
        int attribute_length = 10;
        String paramToTc = "";
        String paramOperator = "";
        String paramCountry = "";
        String paramMccMnc = "";
        String paramMcc = "";
        String paramRegion = "";
        String paramGroupId = "";
        String paramOperatorExempt = "";
        String paramMccMncExempt = "";
        String paramInboundingRoaming = "";

        String array[][][] = new String[param_length][attribute_length][2];

        for (int i = 0; i < param_length; i++) {
            Element line = (Element)param.item(i);
            paramToTc = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByToTc.TOTC);
            paramOperator = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.OPERATOR);
            paramCountry = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.COUNTRY);
            paramMccMnc = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.MCCMNC);
            paramMcc = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.MCC);
            paramRegion = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.REGION);
            paramGroupId = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.GROUP_ID);
            paramOperatorExempt = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.OPERATOR_EXEMPT);
            paramMccMncExempt = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.MCCMNC_EXEMPT);
            paramInboundingRoaming = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.INBOUNDING_ROAMING);

            array[i][0][0] = SmartConfigProviderInterface.ServiceEnableListByToTc.TOTC;
            array[i][0][1] = paramToTc;
            array[i][1][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.OPERATOR;
            array[i][1][1] = paramOperator;
            array[i][2][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.COUNTRY;
            array[i][2][1] = paramCountry;
            array[i][3][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.MCCMNC;
            array[i][3][1] = paramMccMnc;
            array[i][4][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.MCC;
            array[i][4][1] = paramMcc;
            array[i][5][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.REGION;
            array[i][5][1] = paramRegion;
            array[i][6][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.GROUP_ID;
            array[i][6][1] = paramGroupId;
            array[i][7][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.OPERATOR_EXEMPT;
            array[i][7][1] = paramOperatorExempt;
            array[i][8][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.MCCMNC_EXEMPT;
            array[i][8][1] = paramMccMncExempt;
            array[i][9][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.INBOUNDING_ROAMING;
            array[i][9][1] = paramInboundingRoaming;
        }

        return array;
    }

    private String[][][] getTableElementsFromServiceEnableListByNtCode(Element element) {
        NodeList param = element.getElementsByTagName("param");

        if (param == null) {
            Log.e(TAG, "param is null");
            return null;
        }

        int param_length = param.getLength();
        int attribute_length = 11;
        String paramNtCode = "-1";
        String paramOperator = "";
        String paramCountry = "";
        String paramMccMnc = "";
        String paramMcc = "";
        String paramRegion = "";
        String paramGroupId = "";
        String paramOperatorExempt = "";
        String paramMccMncExempt = "";
        String paramInboundingRoaming = "";
        String paramDesc = "";

        String array[][][] = new String[param_length][attribute_length][2];

        for (int i = 0; i < param_length; i++) {
            Element line = (Element)param.item(i);
            paramNtCode = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.NTCODE);
            paramOperator = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.OPERATOR);
            paramCountry = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.COUNTRY);
            paramMccMnc = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.MCCMNC);
            paramMcc = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.MCC);
            paramRegion = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.REGION);
            paramGroupId = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.GROUP_ID);
            paramOperatorExempt = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.OPERATOR_EXEMPT);
            paramMccMncExempt = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.MCCMNC_EXEMPT);
            paramInboundingRoaming = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.INBOUNDING_ROAMING);
            paramDesc = line.getAttribute(
                        SmartConfigProviderInterface.ServiceEnableListByNtCode.DESC);

            array[i][0][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.NTCODE;
            array[i][0][1] = paramNtCode;
            array[i][1][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.OPERATOR;
            array[i][1][1] = paramOperator;
            array[i][2][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.COUNTRY;
            array[i][2][1] = paramCountry;
            array[i][3][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.MCCMNC;
            array[i][3][1] = paramMccMnc;
            array[i][4][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.MCC;
            array[i][4][1] = paramMcc;
            array[i][5][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.REGION;
            array[i][5][1] = paramRegion;
            array[i][6][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.GROUP_ID;
            array[i][6][1] = paramGroupId;
            array[i][7][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.OPERATOR_EXEMPT;
            array[i][7][1] = paramOperatorExempt;
            array[i][8][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.MCCMNC_EXEMPT;
            array[i][8][1] = paramMccMncExempt;
            array[i][9][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.INBOUNDING_ROAMING;
            array[i][9][1] = paramInboundingRoaming;
            array[i][10][0] = SmartConfigProviderInterface.ServiceEnableListByNtCode.DESC;
            array[i][10][1] = paramDesc;
        }

        return array;
    }

    private String[][][] getTableElementsFromNaoCarrierList(Element element) {
        NodeList param = element.getElementsByTagName("param");

        if (param == null) {
            Log.e(TAG, "param is null");
            return null;
        }

        int param_length = param.getLength();
        int attribute_length = 3;
        String paramCarrier = "-1";
        String paramOperator = "-1";
        String paramCountry = "-1";

        String array[][][] = new String[param_length][attribute_length][2];

        for (int i = 0; i < param_length; i++) {
            Element line = (Element)param.item(i);
            paramCarrier = line.getAttribute(SmartConfigProviderInterface.NaoCarrierList.CARRIER);
            paramOperator = line.getAttribute(
                        SmartConfigProviderInterface.NaoCarrierList.OPERATOR);
            paramCountry = line.getAttribute(
                        SmartConfigProviderInterface.NaoCarrierList.COUNTRY);

            array[i][0][0] = SmartConfigProviderInterface.NaoCarrierList.CARRIER;
            array[i][0][1] = paramCarrier;
            array[i][1][0] = SmartConfigProviderInterface.NaoCarrierList.OPERATOR;
            array[i][1][1] = paramOperator;
            array[i][2][0] = SmartConfigProviderInterface.NaoCarrierList.COUNTRY;
            array[i][2][1] = paramCountry;
        }

        return array;
    }

    private NodeList getNode() {
        InputStream isSmartConfig;

        if (mSmartXmlFile.external()) {
            isSmartConfig = openXmlFileInExternal(mSmartXmlFile.folder(), mSmartXmlFile.filename());
        } else {
            isSmartConfig = openXmlFileInAssets(mSmartXmlFile.folder(), mSmartXmlFile.filename());
        }

        try {
            Document doc = getDocument(isSmartConfig);
            if (doc == null) {
                Log.e(TAG, "Document is null");
                return null;
            }

            Element order = doc.getDocumentElement();
            NodeList nodes = order.getElementsByTagName("table");

            return nodes;
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "getNode :: " + e.toString());
        } finally {
            closeXmlFile(isSmartConfig);
        }

        return null;
    }

    private static Document getDocument(InputStream in) {
        Document doc = null;

        if (in != null) {
            try {
                DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
                DocumentBuilder builder = factory.newDocumentBuilder();

                doc = builder.parse(in);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }

        return doc;
    }

    private class XMLFile {
        private String mFolder;
        private String mFilename;
        private boolean mExist = false;
        private boolean mExternal = false;

        public XMLFile() {
            mFolder = "";
            mFilename = "";
            mExist = false;
            mExternal = false;
        }

        public String filename() {
            return mFilename;
        }

        public String folder() {
            return mFolder;
        }

        public boolean exist() {
            return mExist;
        }

        public boolean external() {
            return mExternal;
        }

        public void setFilename(String filename) {
            mFilename = filename;
        }

        public void setFilename(String root_folder, String type
                            , String op, String co, String sim_moved, String model) {

            String filename = type;
            String folder = root_folder;

            if (op != null && op.length() > 0) {
                filename += "." + op;
                folder += "/" + op;

                if (co != null && co.length() > 0) {
                    filename += "." + co;
                    folder += "." + co;
                }
                if (sim_moved != null && sim_moved.length() > 0) {
                    filename += "." + "MOVED";
                }
                if (model != null && model.length() > 0) {
                    filename += "." + model;
                    folder += "/" + model;
                }
            }

            filename += "." + "xml";

            mFilename = filename;
            mFolder = folder;

            Log.d(TAG, "Folder = " + folder + ", filename = " + filename);

            mExist = existInAssets(folder);
        }

        public void setExternal(boolean external) {
            mExternal = external;
        }

        public void setFolder(String folder) {
            mFolder = folder;
        }

        public boolean existInExternal(String folder) {
            String filePath = ((folder.length() > 0) ? (folder + "/") : "") + mFilename;
            File file = new File(filePath);
            return file.exists();
        }

        private boolean existInAssets(String folder) {
            boolean hasFile = false;
            String filePath = ((folder.length() > 0) ? (folder + "/") : "") + mFilename;

            try {
                InputStream is = mContext.getAssets().open(filePath);
                if (is != null) {
                    hasFile = true;
                    closeXmlFile(is);
                }
            } catch (IOException e) {
                hasFile = false;

                if (Log.isDebuggable()) {
                    Log.i(TAG, "existInAssets :: " + e.toString());
                }
            }

            return hasFile;
        }
    }
}
