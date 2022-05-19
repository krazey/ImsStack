package com.android.imsstack.core.config;

import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.text.TextUtils;

import com.android.imsstack.core.OperatorInfo;
import com.android.imsstack.util.DBUtils;
import com.android.imsstack.util.ImsConstants;
import com.android.imsstack.util.ImsProperties;
import com.android.imsstack.util.Log;
import com.android.imsstack.util.MSimUtils;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.Locale;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

//1. Example for External folder :
///ImsConstants.IMS_STORAGE_ROOT_DIR/xml/configuration.LGU.KR.xml
///ImsConstants.IMS_STORAGE_ROOT_DIR/xml/configuration.ATT.US.D725.xml
//2. Example for Assets folder :
//assets/Configuration/LGU.KR/configuration.LGU.KR.xml
//assets/Configuration/LGU.KR/configuration.LGU.KR.MOVED.xml
//3. Example for Model folder :
//assets/Configuration/ATT.US/D725/configuration.ATT.US.D725.xml
public class ConfigXMLLoader {
    private static final String TAG = "ImsStack_ConfigXMLLoader";
    private static final String XML_ROOT_IN_ASSETS = "Configuration";

    private static final String CONFIGURATION_TYPE = "configuration";
    private static final String CONFIGURATION_MEDIA_TYPE = "media";

    private static final String ELEMENT_PARAM = "param";
    private static final String ELEMENT_TABLE = "table";
    private static final String ATTR_TYPE = "type";
    private static final String ATTR_NAME = "name";
    private static final String ATTR_VALUE = "value";
    private static final int ATTR_I_TYPE = 0;
    private static final int ATTR_I_NAME = 1;
    private static final int ATTR_I_VALUE = 2;
    private static final int ATTR_MAX = 3;

    private XMLFile mConfigXmlFile = null;
    private XMLFile mMediaXmlFile = null;

    private NodeList mNodes = null;
    private String[] mTableId = null;

    private final int mSlotId;
    private final boolean mUseDefaultConfigOnly;
    private Context mContext = null;

    public ConfigXMLLoader() {
        mSlotId = MSimUtils.DEFAULT_SLOT_ID;
        mUseDefaultConfigOnly = true;
    }

    public ConfigXMLLoader(int slotId) {
        mSlotId = slotId;
        mUseDefaultConfigOnly = false;
    }

    public void init(Context context) {
        mContext = context;

        mConfigXmlFile = null;
        mMediaXmlFile = null;

        mNodes = null;
        mTableId = null;

        setXmlResource(CONFIGURATION_TYPE, ProviderInterface.DBInfo.TABLE_NAME,
                ProviderInterface.DBInfo.CONFIG_XML_MODIFIED_TIME);
        setXmlResource(CONFIGURATION_MEDIA_TYPE, ProviderInterface.MediaDBInfo.TABLE_NAME,
                ProviderInterface.MediaDBInfo.MEDIA_XML_MODIFIED_TIME);

        loadConfigs();
    }

    private boolean isAdminServicesUpdated(SQLiteDatabase db) {
        final int iSERVICE_VOLTE = ProviderInterface.Subscriber.AdminServices.VOLTE;
        final int iSERVICE_VILTE = ProviderInterface.Subscriber.AdminServices.VILTE;
        final int iSERVICE_VOWIFI = ProviderInterface.Subscriber.AdminServices.VOWIFI;
        final int iSERVICE_VIWIFI = ProviderInterface.Subscriber.AdminServices.VIWIFI;

        final boolean bVoLTEEnabled = OperatorInfo.isSupportVolte(mSlotId);
        final boolean bViLTEEnabled = OperatorInfo.isSupportVt(mSlotId);
        final boolean bVoWifiEnabled = OperatorInfo.isSupportVowifi(mSlotId);

        int adminServices = 0x00000000;

        if (bVoLTEEnabled) {
            adminServices |= iSERVICE_VOLTE;
        }

        if (bVoWifiEnabled) {
            adminServices |= iSERVICE_VOWIFI;
        }

        if (bViLTEEnabled) {
            adminServices |= iSERVICE_VILTE;
        }

        int oldAdminServices = DBUtils.DB.getHex(mSlotId, db,
                   ProviderInterface.Subscriber.TABLE_NAME,
                   ProviderInterface.Subscriber.SERVICES, 0x00000000);

        Log.w(TAG, "AdminServices :: " + oldAdminServices + " >> " + adminServices);

        if (adminServices != (oldAdminServices & (~iSERVICE_VIWIFI))) {
            return false;
        }

        return true;
    }

    public boolean isConfigChanged(SQLiteDatabase db) {
        if (db == null) {
            return false;
        }

        if (!isConfigXmlFileReady()) {
            Log.e(TAG, "ConfigXML :: fatal error - config=" + mConfigXmlFile
                    + ", media=" + mMediaXmlFile);
            return false;
        }

        if (!DBUtils.DB.hasTable(db, ProviderInterface.DBInfo.TABLE_NAME)) {
            Log.w(TAG, ProviderInterface.DBInfo.TABLE_NAME + " table is not existed "
                    + "-> DB will be updated :: " + mConfigXmlFile.filename());
            return true;
        }

        int rowCnt = DBUtils.DB.getRowCount(mSlotId, db, ProviderInterface.DBInfo.TABLE_NAME);
        if (rowCnt <= 0) {
            Log.w(TAG, ProviderInterface.DBInfo.TABLE_NAME + " table is empty "
                    + "-> DB will be updated :: " + mConfigXmlFile.filename());
            return true;
        }

        String xmlFilenameInDB = DBUtils.DB.getString(mSlotId,
                db, ProviderInterface.DBInfo.TABLE_NAME,
                ProviderInterface.DBInfo.CONFIG_XML_FILE, "");
        Log.i(TAG, "ConfigXML :: db-info=" + xmlFilenameInDB);

        if (xmlFilenameInDB.length() > 0) {
            if (xmlFilenameInDB.equals(mConfigXmlFile.filename()) != true) {
                Log.w(TAG, "XML's filename in DB(" + xmlFilenameInDB + ") "
                        + "is not equal XML's filename(" + mConfigXmlFile.filename() + ") "
                        + "-> DB will be updated :: " + mConfigXmlFile.filename());
                return true;
            }
        }

        // SystemProperty VS volte_disabled
        String strSvcModifiedTime = DBUtils.DB.getString(mSlotId,
                db, ProviderInterface.DBInfo.TABLE_NAME,
                ProviderInterface.DBInfo.MAKE_DB_ON_NEXT_TIME, "");

        if (strSvcModifiedTime.length() > 0) {
            boolean notUpdatedDB = "0".equals(strSvcModifiedTime);
            boolean volteEnabled = OperatorInfo.isVoLTEServiceAvailable(mSlotId);
            if (!notUpdatedDB && volteEnabled) {
                Log.w(TAG, "NotUpdatedDB(" + notUpdatedDB + ") VS "
                        + "SystemProperty(" + OperatorInfo.isSupportVolte(mSlotId) + "/"
                        + OperatorInfo.isSupportVt(mSlotId) + "/"
                        + OperatorInfo.isSupportVowifi(mSlotId) + ") "
                        + "-> DB will be updated :: " + mConfigXmlFile.filename());
                return true;
            }
        }

        // ModifiedTime in XML VS ModifiedTime in DB
        int modifiedTimeInConfig = mConfigXmlFile.getModifiedTime();
        int modifiedTimeInMedia = mMediaXmlFile.getModifiedTime();

        int modifiedTimeInConfigInDB = DBUtils.DB.getInt(mSlotId,
                db, ProviderInterface.DBInfo.TABLE_NAME,
                ProviderInterface.DBInfo.CONFIG_XML_MODIFIED_TIME, 0);
        int modifiedTimeInMediaInDB = DBUtils.DB.getInt(mSlotId,
                db, ProviderInterface.MediaDBInfo.TABLE_NAME,
                ProviderInterface.MediaDBInfo.MEDIA_XML_MODIFIED_TIME, 0);

        if (modifiedTimeInConfig != modifiedTimeInConfigInDB
                || modifiedTimeInMedia != modifiedTimeInMediaInDB) {
            Log.w(TAG, "ConfigDB :: ModifiedTime - "
                + "Config(" + modifiedTimeInConfigInDB + "/" + modifiedTimeInConfig + "), "
                + "Media(" + modifiedTimeInMediaInDB + "/" + modifiedTimeInMedia + "), "
                + " -> DB will be updated :: "
                + mConfigXmlFile.filename() + "/"
                + mMediaXmlFile.filename());
            return true;
        }

        if (OperatorInfo.isEnablerTypeForNonOperator(mSlotId)
                && !isAdminServicesUpdated(db)) {
            Log.w(TAG, "IMS services changed -> DB will be updated");
            return true;
        }

        if (isSwVersionChanged(db)) {
            Log.w(TAG, "SW version changed -> DB will be re-created");
            return true;
        }

        Log.i(TAG, "DB will not be updated");

        return false;
    }

    private boolean isSwVersionChanged(SQLiteDatabase db) {
        if (!OperatorInfo.getSimOperator(mSlotId).equals("RJIL")
                && !OperatorInfo.isEnablerTypeForNonOperator(mSlotId)) {
            return false;
        }

        String swversionInDB = DBUtils.DB.getString(mSlotId,
                db, ProviderInterface.DBInfo.TABLE_NAME, ProviderInterface.DBInfo.SW_VERSION, "");

        if (TextUtils.isEmpty(swversionInDB)) {
            Log.w(TAG, "ConfigDB :: No SW_VERSION in DB");
            return true;
        }

        String swversion = ImsProperties.SW_VERSION_3CHARS;

        if (TextUtils.isEmpty(swversion)) {
            Log.w(TAG, "ConfigDB :: No SW_VERSION in device");
            return false;
        }

        if (!swversionInDB.equals(swversion)) {
            Log.w(TAG, "ConfigDB :: SW_VERSION - " + swversionInDB + " >> " + swversion);
            return true;
        }

        return false;
    }

    private InputStream openXmlFileInExternal(String ext_folder, String filename) {
        InputStream in = null;
        String strFullName = ((ext_folder.length() > 0) ? (ext_folder + "/") : "") + filename;

        Log.i(TAG, "ConfigXML(Ext) :: " + strFullName);

        try {
            File xml_file = new File(strFullName);
            in = new FileInputStream(xml_file);
        } catch (FileNotFoundException e) {
            Log.i(TAG, "ConfigXML(Ext) :: " + e.toString());
        }

        return in;
    }

    private InputStream openXmlFileInAssets(String folder, String filename) {
        InputStream in = null;
        String strFullName = ((folder.length() > 0) ? (folder + "/") : "") + filename;

        Log.i(TAG, "ConfigXML(Asset) :: " + strFullName);

        try {
            in = mContext.getAssets().open(strFullName);
        } catch (IOException e) {
            Log.i(TAG, "ConfigXML(Asset) :: " + e.toString());
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
        if (!isConfigLoaded()) {
            loadConfigs();
        }

        return mTableId;
    }

    public String[] getTableNamesFromCache() {
        return mTableId;
    }

    public String[][] getTableElements(int id) {
        if (!isConfigLoaded()) {
            loadConfigs();
        }

        if (mNodes == null || mTableId == null) {
            return null;
        }

        if (id < 0 || id > mNodes.getLength()) {
            Log.e(TAG, "Invalid table id: " + id);
            return null;
        }

        Element element = (Element)mNodes.item(id);
        NodeList param = (element != null) ? element.getElementsByTagName(ELEMENT_PARAM) : null;

        if (param == null) {
            Log.e(TAG, "No element: e=" + element + ", el=" + param);
            return null;
        }

        int paramCount = param.getLength();
        String paramType = "-1";
        String paramName = "-1";
        String paramValue = "-1";

        String array[][] = new String[paramCount][ATTR_MAX];
        boolean isDBInfoTable = ProviderInterface.DBInfo.TABLE_NAME.equals(mTableId[id]);

        for (int i = 0; i < paramCount; i++) {
            Element line = (Element)param.item(i);
            paramType = line.getAttribute(ATTR_TYPE);
            paramName = line.getAttribute(ATTR_NAME);
            paramValue = line.getAttribute(ATTR_VALUE);

            array[i][ATTR_I_TYPE] = paramType;
            array[i][ATTR_I_NAME] = paramName;

            // INVALID_INTEGER value
            if (MSimUtils.isMultiSimEnabled()
                    && TextUtils.isEmpty(paramValue) && "INTEGER".equals(paramType)) {
                paramValue = String.valueOf(-1);
            }

            if (ProviderInterface.ID.equalsIgnoreCase(paramName)) {
                array[i][ATTR_I_VALUE] = String.format(Locale.US, "%d", mSlotId);
            } else {
                array[i][ATTR_I_VALUE] = paramValue;
            }

            if (isDBInfoTable) {
                if (ProviderInterface.DBInfo.SW_VERSION.equals(paramName)) {
                    array[i][ATTR_I_VALUE] = ImsProperties.SW_VERSION_3CHARS;
                }
            }
        }

        return array;
    }

    private NodeList getNode() {
        try {
            Document doc = getDocument();

            if (doc == null) {
                return null;
            }

            Element order = doc.getDocumentElement();
            NodeList nodes = order.getElementsByTagName(ELEMENT_TABLE);

            return nodes;
        } catch (Exception e) {
            Log.e(TAG, "getNode :: " + e.toString());
        }

        return null;
    }

    public boolean checkDBIntegrityFile(String dbIntegrityFile) {
        String strFullName = ImsConstants.PATH_XML + "/" + dbIntegrityFile + "_" + mSlotId;
        File integrity_file = new File(strFullName);

        if (integrity_file.exists()) {
            Log.i(TAG, "checkDBIntegrityFile :: file exists - " + strFullName);
            return true;
        }

        return false;
    }

    public void createDBIntegrityFile(String dbIntegrityFile) {
        if (checkDBIntegrityFile(dbIntegrityFile)) {
            return;
        }

        String strFullName = ImsConstants.PATH_XML + "/" + dbIntegrityFile + "_" + mSlotId;
        File integrity_dir = new File(ImsConstants.PATH_XML);
        File integrity_file = new File(strFullName);

        try {
            integrity_dir.mkdir();
            integrity_file.createNewFile();
        } catch (IOException e) {
            Log.e(TAG, "createDBIntegrityFile :: " + e.toString());
        }
    }

    public void deleteDBIntegrityFile(String dbIntegrityFile) {
        String strFullName = ImsConstants.PATH_XML + "/" + dbIntegrityFile + "_" + mSlotId;
        File integrity_file = new File(strFullName);

        if (integrity_file.exists()) {
            integrity_file.delete();
        }
    }

    // FIXME : Added new APIs for checking Increament ModifiedTime
    private InputStream openXmlFile(XMLFile xmlFile) {
        if (xmlFile.external()) {
            return openXmlFileInExternal(xmlFile.folder(), xmlFile.filename());
        }

        return openXmlFileInAssets(xmlFile.folder(), xmlFile.filename());
    }

    private boolean setXmlResource(String configType, String table, String key) {
        if (TextUtils.isEmpty(configType)) {
            return false;
        }

        // Setting xml file - default, operator+country, model
        ArrayList<XMLFile> xml_file_list = new ArrayList<XMLFile>();

        XMLFile xml_file;
        xml_file = new XMLFile();
        xml_file.setFilename(XML_ROOT_IN_ASSETS, configType, "", "", "", "");
        xml_file_list.add(xml_file);

        if (!mUseDefaultConfigOnly) {
            String operator = OperatorInfo.getOperator(mSlotId);
            String country = OperatorInfo.getCountry(mSlotId);

            xml_file = new XMLFile();
            xml_file.setFilename(XML_ROOT_IN_ASSETS, configType, operator, "", "", "");
            xml_file_list.add(xml_file);

            // OPEN-CA/OPEN-US : OOB(Out-Of-Box) scenario or PTCRB
            if ("CA".equals(country) || OperatorInfo.isNaOpen()) {
                if ("OPEN".equals(operator) || "GCF".equals(operator)) {
                    xml_file = new XMLFile();
                    xml_file.setFilename(XML_ROOT_IN_ASSETS, configType, "OPEN", "NA", "", "");
                    xml_file_list.add(xml_file);
                }
            }

            xml_file = new XMLFile();
            xml_file.setFilename(XML_ROOT_IN_ASSETS, configType, operator, country, "", "");
            xml_file_list.add(xml_file);

            if (OperatorInfo.isSimMoved(mSlotId)) {
                xml_file = new XMLFile();
                xml_file.setFilename(XML_ROOT_IN_ASSETS, configType, operator, country,
                        "MOVED", "");
                xml_file_list.add(xml_file);
            }

            if (OperatorInfo.isConfigPerModel(mSlotId)) {
                xml_file = new XMLFile();
                String postName = OperatorInfo.getConfigPostfix(mSlotId);
                if (TextUtils.isEmpty(postName)) {
                    postName = ImsProperties.MODEL;
                }
                xml_file.setFilename(XML_ROOT_IN_ASSETS, configType, operator, country,
                        OperatorInfo.isSimMoved(mSlotId) ? "MOVED" : "", postName);
                xml_file_list.add(xml_file);
            }
        }

        if (xml_file_list.size() <= 0) {
            return false;
        }

        for (int i = (xml_file_list.size() - 1); i >= 0; i--) {
            XMLFile xmlFile = openXmlResource(xml_file_list.get(i), table, key, null);

            if (xmlFile != null) {
                if (CONFIGURATION_TYPE.equals(configType)) {
                    mConfigXmlFile = xmlFile;
                } else if (CONFIGURATION_MEDIA_TYPE.equals(configType)) {
                    mMediaXmlFile= xmlFile;
                } else {
                    Log.w(TAG, "setXmlResource :: Invalid ConfigType(" + configType + ")");
                    return false;
                }

                return true;
            }
        }

        return false;
    }

    private XMLFile openXmlResource(XMLFile xml_file, String table, String key, String dump) {
        boolean bExternal = xml_file.existInExternal(ImsConstants.PATH_XML);
        boolean bAsset = xml_file.exist();

        if (!bExternal && !bAsset) {
            Log.d(TAG, "File(" + xml_file.filename() + ") is not found");
            return null;
        }

        int modifiedInExternal = -1;
        int modifiedInAsset = -1;

        if (bExternal) {
            InputStream isExternal = openXmlFileInExternal(
                    ImsConstants.PATH_XML, xml_file.filename());
            modifiedInExternal = getModifiedTime(isExternal, table, key);
            closeXmlFile(isExternal);
        }

        if (bAsset) {
            InputStream isAsset = openXmlFileInAssets(xml_file.folder(), xml_file.filename());
            modifiedInAsset = getModifiedTime(isAsset, table, key);
            closeXmlFile(isAsset);
        }

        Log.i(TAG, "ConfigXML(" + xml_file.filename() + ") :: modifiedTime(asset|ext)=["
                + modifiedInAsset + "|" + modifiedInExternal + "]");

        if (modifiedInExternal > modifiedInAsset) {
            xml_file.setFolder(ImsConstants.PATH_XML);
            xml_file.setExternal(true);
            xml_file.setModifiedTime(modifiedInExternal);
            return xml_file;
        }

        xml_file.setModifiedTime(modifiedInAsset);

        return xml_file;
    }

    private int getModifiedTime(InputStream is, String table, String key) {
        if (is == null) {
            return -1;
        }

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
            if (tables[i].equals(table) != true) {
                continue;
            }

            String[][] array = getTableElements(mSlotId, i, nodes);
            if (array == null) {
                Log.w(TAG, "getModifiedTime :: No elements in " + table);
                return -1;
            }

            strModifiedTime = getContentsValue(array, key);
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

    private static String getContentsValue(String[][] contents, String key) {
        if (contents == null || contents.length == 0) {
            return "";
        }

        String value = "";
        for (int i = 0; i < contents.length; ++i) {
            // '1' - column name, '2' - column default value
            if (contents[i][1].equals(key)) {
                value = contents[i][2];
                break;
            }
        }

        return value;
    }

    private static NodeList getNode(InputStream is) {
        try {
            Document doc = getDocument(is);

            if (doc == null) {
                return null;
            }

            Element order = doc.getDocumentElement();
            NodeList nodes = order.getElementsByTagName(ELEMENT_TABLE);
            return nodes;
        } catch (Exception e) {
            Log.e(TAG, "getNode(InputStream) :: " + e.toString());
        }

        return null;
    }

    private static Document getDocument(InputStream is) {
        if (is == null) {
            throw new IllegalArgumentException("InputStream is null");
        }

        Document doc = null;

        try {
            DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
            DocumentBuilder builder = factory.newDocumentBuilder();

            doc = builder.parse(is);
        } catch (Exception e) {
            Log.e(TAG, "getDocument(InputStream) :: " + e.toString());
        }

        return doc;
    }

    private Document getDocument() {
        Log.i(TAG, "ConfigXML(ParseNMerge) :: modifiedTime(config|media)=["
                + mConfigXmlFile.getModifiedTime() + "|"
                + mMediaXmlFile.getModifiedTime() + "]");

        InputStream in = null;
        InputStream in_media = null;

        if (mConfigXmlFile.external()) {
            in = openXmlFileInExternal(mConfigXmlFile.folder(), mConfigXmlFile.filename());
        } else {
            in = openXmlFileInAssets(mConfigXmlFile.folder(), mConfigXmlFile.filename());
        }

        if (mMediaXmlFile.external()) {
            in_media = openXmlFileInExternal(mMediaXmlFile.folder(), mMediaXmlFile.filename());
        } else {
            in_media = openXmlFileInAssets(mMediaXmlFile.folder(), mMediaXmlFile.filename());
        }

        Document doc = null;

        if (in != null && in_media != null) {
            try {
                DocumentBuilderFactory factory = DocumentBuilderFactory.newInstance();
                DocumentBuilder builder = factory.newDocumentBuilder();

                doc = builder.parse(in);

                Document doc_media = builder.parse(in_media);

                NodeList list = doc_media.getElementsByTagName(ELEMENT_TABLE);
                for (int i = 0; i < list.getLength(); i++) {
                    Element element = (Element) list.item(i);
                    // Imports a node from another document to this document, without altering
                    // or removing the source node from the original document
                    Node copiedNode = doc.importNode(element, true);

                    // Adds the node to the end of the list of children of this node
                    doc.getDocumentElement().appendChild(copiedNode);
                }
            }
            catch (Exception e) {
                Log.e(TAG, "getDocument :: " + e.toString());
            }
        }

        closeXmlFile(in);
        closeXmlFile(in_media);

        return doc;
    }

    private static String[] getTableNames(NodeList nodes) {
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

    private static String[][] getTableElements(int slotId, int tableId, NodeList nodes) {
        if (tableId < 0 || tableId > nodes.getLength()) {
            return null;
        }

        Element element = (Element)nodes.item(tableId);
        NodeList param = (element != null) ? element.getElementsByTagName(ELEMENT_PARAM) : null;

        if (param == null) {
            Log.e(TAG, "No element: e=" + element + ", el=" + param);
            return null;
        }

        int paramCount = param.getLength();
        String paramType = "-1";
        String paramName = "-1";
        String paramValue = "-1";

        String array[][] = new String[paramCount][ATTR_MAX];

        for (int i = 0; i < paramCount; i++) {
            Element line = (Element)param.item(i);
            paramType = line.getAttribute(ATTR_TYPE);
            paramName = line.getAttribute(ATTR_NAME);
            paramValue = line.getAttribute(ATTR_VALUE);

            array[i][ATTR_I_TYPE] = paramType;
            array[i][ATTR_I_NAME] = paramName;

            if (ProviderInterface.ID.equalsIgnoreCase(paramName)) {
                array[i][ATTR_I_VALUE] = String.format(Locale.US, "%d", slotId);
            } else {
                array[i][ATTR_I_VALUE] = paramValue;
            }
        }

        return array;
    }

    private void loadConfigs() {
        if (!isConfigXmlFileReady()) {
            Log.e(TAG, "ConfigXML :: fatal error - config=" + mConfigXmlFile
                    + ", media=" + mMediaXmlFile);
            return;
        }

        mTableId = null;
        mNodes = getNode();

        if (mNodes != null) {
            mTableId = getTableNames(mNodes);
        }
    }

    private boolean isConfigLoaded() {
        return (mNodes != null) && (mTableId != null);
    }

    private boolean isConfigXmlFileReady() {
        return (mConfigXmlFile != null && mMediaXmlFile != null);
    }

    // Class for XML File
    private class XMLFile {
        private String mFolder;
        private String mFilename;
        private boolean mExist = false;
        private boolean mExternal = false;
        private int mModifiedTime = -1;

        public XMLFile() {
            mFolder = "";
            mFilename = "";
            mExist = false;
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

        public void setFilename(String root_folder, String type,
                                String op, String co, String sim_moved, String model) {

            String filename = type;
            String folder = root_folder;

            if (op != null && op.length() > 0) {
                filename += "." + op;
                folder += "/" + op;

                if (co != null && co.length() > 0) {
                    filename += "." + co;
                    folder += "/" + co;
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

            mExist = existInAssets(folder);
        }

        public void setExternal(boolean external) {
            mExternal = external;
        }

        public void setFolder(String folder) {
            mFolder = folder;
        }

        public int getModifiedTime() {
            return mModifiedTime;
        }

        public void setModifiedTime(int modifiedTime) {
            mModifiedTime = modifiedTime;
        }

        public boolean existInExternal(String ext_folder) {
            String strFullName
                    = ((ext_folder.length() > 0) ? (ext_folder + "/") : "") + mFilename;
            File file = new File(strFullName);

            return file.exists();
        }

        private boolean existInAssets(String folder) {
            boolean bIsXmlFile = false;
            String strFullName = ((folder.length() > 0) ? (folder + "/") : "") + mFilename;

            try {
                InputStream is = mContext.getAssets().open(strFullName);
                if (is != null) {
                    bIsXmlFile = true;
                    is.close();
                }
            } catch (IOException e) {
                bIsXmlFile = false;

                if (Log.isDebuggable()) {
                    Log.i(TAG, "existInAssets :: " + e.toString());
                }
            }

            return bIsXmlFile;
        }
    }
}
