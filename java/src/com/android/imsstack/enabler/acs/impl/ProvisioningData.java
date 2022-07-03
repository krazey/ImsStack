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

package com.android.imsstack.enabler.acs.impl;

import android.text.TextUtils;
import android.util.Xml;

import com.android.imsstack.util.ImsLog;
import com.android.internal.annotations.VisibleForTesting;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.ArrayList;
import java.util.LinkedHashMap;
import java.util.Map;

/**
 * This class parse provisioning xml data and generate xml from parsed data
 */
public class ProvisioningData {
    public static final String TAG_PROVISIONINGDOC = "wap-provisioningdoc";
    public static final String TAG_CHARACTERISTIC = "characteristic";
    public static final String ELEM_PARM = "parm";
    public static final String ATTR_VERSION = "version";
    public static final String ATTR_TYPE = "type";
    public static final String ATTR_NAME = "name";
    public static final String ATTR_VALUE = "value";

    /**
     * This class handles each characteristic block in RCS provisioning XML data
     */
    private static class Characteristic {
        private final LinkedHashMap<String, String> mParms = new LinkedHashMap<>();
        private final ArrayList<Characteristic> mChilds = new ArrayList<>();
        private final Characteristic mParent;

        private String mType;

        private Characteristic(Characteristic parent, String type) {
            mParent = parent;
            mType = type;
        }

        private void updateTypeValue(String typeValue) {
            mType = typeValue;
        }

        private void putParm(String key, String value) {
            mParms.put(key, value);
        }

        private void putChild(Characteristic child) {
            mChilds.add(child);
        }

        private String getValue(String key) {
            String value = mParms.get(key);
            ImsLog.i("read : " + value);
            return value;
        }

        private boolean containsKey(String key) {
            return mParms.containsKey(key);
        }
    }

    private final Characteristic mRoot = new Characteristic(null, "root");
    private byte[] mRawData;
    private boolean mIsComplete;

    /**
     * Creator to parse xml data
     * @param data provisioning xml data
     */
    public ProvisioningData(byte[] data) {
        // copy data
        mRawData = data.clone();
        mIsComplete = false;

        // parse data
        try {
            parse(new ByteArrayInputStream(mRawData), mRoot);
        } catch (XmlPullParserException | IOException e) {
            ImsLog.i(e.getMessage());
        }
    }

    /**
     * Creator to parse xml file
     * @param parent The parent abstract pathname
     * @param child The child pathname string
     */
    public ProvisioningData(File parent, String child) {
        mIsComplete = false;

        try {
            // read data from file
            File file = new File(parent, child);
            FileInputStream inputStream = new FileInputStream(file);

            // parse data
            parse(inputStream, mRoot);
        } catch (XmlPullParserException | IOException e) {
            ImsLog.i(e.getMessage());
        }
    }

    /**
     * Check XML parsing is completed
     * @return  true if xml parsing is completed, otherwise is false
     */
    public boolean isComplete() {
        return mIsComplete;
    }

    /**
     * Update element ant attribute
     * @param provisioningData xml parser has updated data
     * @return true if the operation is success, otherwise return false
     */
    public boolean updateData(ProvisioningData provisioningData) {
        return true;
    }

    /**
     * Store xml parsed data into xml file
     * @param parent file descriptor include path information
     * @param child file name
     * @return true if the operation is success, otherwise return false
     */
    public boolean saveXmlFile(File parent, String child) {
        if (!mIsComplete) {
            ImsLog.i("not available parsed data");
            return false;
        }

        ByteArrayOutputStream out = encodeXmlData(mRoot);
        if (out != null) {
            try {
                ImsLog.i(out.toString());
                File file = new File(parent, child);
                FileOutputStream outputStream = new FileOutputStream(file);
                out.writeTo(outputStream);
                outputStream.close();
            } catch (IOException e) {
                ImsLog.i(e.getMessage());
                return false;
            }
        }

        return true;
    }

    /**
     * get int value associated attribute in provisioning data
     * @param attributeName attribute name
     * @param defaultValue this will be return when failed to get internal operation
     * @return value associated attribute
     */
    public int getIntValue(String attributeName, int defaultValue) {
        int ret = defaultValue;
        String value = getValue(mRoot, attributeName);

        try {
            ret = Integer.parseInt(value);
        } catch (Exception e) {
            ImsLog.i(e.getMessage());
        }

        return ret;
    }

    /**
     * get long value associated attribute in provisioning data
     * @param attributeName attribute name
     * @param defaultValue this will be return when failed to get internal operation
     * @return value associated attribute
     */
    public long getLongValue(String attributeName, long defaultValue) {
        long ret = defaultValue;
        String value = getValue(mRoot, attributeName);

        try {
            ret = Long.parseLong(value);
        } catch (Exception e) {
            ImsLog.i(e.getMessage());
        }

        return ret;
    }

    /**
     * get String value associated attribute in provisioning data
     * @param attributeName attribute name
     * @param defaultValue this will be return when failed to get internal operation
     * @return value associated attribute
     */
    public String getStringValue(String attributeName, String defaultValue) {
        String value = getValue(mRoot, attributeName);

        if (TextUtils.isEmpty(value)) {
            return defaultValue;
        }

        return value;
    }

    private String getValue(Characteristic root, String key) {
        for (Characteristic c : root.mChilds) {
            if (c == null) {
                continue;
            }
            if (c.containsKey(key)) {
                return c.getValue(key);
            }
        }

        for (Characteristic c : root.mChilds) {
            String value = getValue(c, key);
            if (!TextUtils.isEmpty(value)) {
                return value;
            }
        }

        return null;
    }

    private void parse(InputStream in, Characteristic root)
            throws XmlPullParserException, IOException {
        try {
            XmlPullParser parser = Xml.newPullParser();
            parser.setFeature(XmlPullParser.FEATURE_PROCESS_NAMESPACES, false);
            parser.setInput(in, null);
            parser.nextTag();
            readProvisioningDoc(parser, root);
            mIsComplete = true;
        } finally {
            in.close();
        }
    }

    private void readProvisioningDoc(XmlPullParser parser, Characteristic root)
            throws XmlPullParserException, IOException {
        parser.require(XmlPullParser.START_TAG, null, TAG_PROVISIONINGDOC);
        int count = parser.getAttributeCount();
        if (count > 0) {
            for (int i = 0; i < count; i++) {
                String name = parser.getAttributeName(i).trim().toLowerCase();
                if (ATTR_VERSION.equals(name)) {
                    String value = parser.getAttributeValue(i);
                    root.updateTypeValue(value);
                }
            }
        }

        while (parser.next() != XmlPullParser.END_DOCUMENT) {
            if (parser.getEventType() != XmlPullParser.START_TAG) {
                continue;
            }
            String tagName = parser.getName();
            if (TAG_CHARACTERISTIC.equals(tagName)) {
                Characteristic child = readCharacteristic(root, parser);
                root.putChild(child);
            } else {
                skip(parser);
            }
        }
    }

    private Characteristic readCharacteristic(Characteristic parent, XmlPullParser parser)
            throws XmlPullParserException, IOException {
        // TODO : check this, do we need to call this?
        parser.require(XmlPullParser.START_TAG, null, TAG_CHARACTERISTIC);

        int count = parser.getAttributeCount();
        Characteristic current = null;

        if (count > 0) {
            for (int i = 0; i < count; i++) {
                String name = parser.getAttributeName(i).trim().toLowerCase();
                if (ATTR_TYPE.equals(name)) {
                    String typeValue = parser.getAttributeValue(i);
                    current = new Characteristic(parent, typeValue);
                }
            }
        }
        int endTag = parser.next();
        String endName = parser.getName();
        while (!(endTag == XmlPullParser.END_TAG
                && TAG_CHARACTERISTIC.equals(endName))) {
            if (parser.getEventType() == XmlPullParser.START_TAG) {
                String tag = parser.getName().trim().toLowerCase();
                if (ELEM_PARM.equals(tag)) {
                    count = parser.getAttributeCount();
                    String key = null;
                    String value = null;
                    if (count > 1) {
                        for (int i = 0; i < count; i++) {
                            String name = parser.getAttributeName(i).trim().toLowerCase();
                            if (ATTR_NAME.equals(name)) {
                                key = parser.getAttributeValue(i);
                            } else if (ATTR_VALUE.equals(name)) {
                                value = parser.getAttributeValue(i);
                            }
                        }
                    }
                    if (current != null && key != null && value != null) {
                        current.putParm(key, value);
                    }
                } else if (TAG_CHARACTERISTIC.equals(tag)) {
                    current.putChild(readCharacteristic(current, parser));
                }
            }

            endTag = parser.next();
            endName = parser.getName().trim().toLowerCase();
        }
        parser.require(XmlPullParser.END_TAG, null, "characteristic");

        return current;
    }

    private void skip(XmlPullParser parser) throws XmlPullParserException, IOException {
        if (parser.getEventType() != XmlPullParser.START_TAG) {
            throw new IllegalStateException();
        }
        int depth = 1;
        while (depth != 0) {
            switch (parser.next()) {
                case XmlPullParser.END_TAG:
                    depth--;
                    break;
                case XmlPullParser.START_TAG:
                    depth++;
                    break;
            }
        }
    }

    private ByteArrayOutputStream encodeXmlData(Characteristic root) {
        ByteArrayOutputStream out = new ByteArrayOutputStream();

        String line = "";
        try {
            out.write("<?xml version=\"1.0\"?>".getBytes());
            line = "<" + TAG_PROVISIONINGDOC + " version=\"" + root.mType + "\">";
            out.write(line.getBytes());

            for (Characteristic child : root.mChilds) {
                writeCharacteristic(out, child);
            }

            line = "</wap-provisioningdoc>";
            out.write(line.getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }

        return out;
    }

    private void writeCharacteristic(ByteArrayOutputStream out, Characteristic data)
            throws IOException {
        String line = "";
        // write characteristic start tag and type attribute
        // <characteristic type="value">

        // if empty tag : no parm element and child
        if (data.mParms.size() == 0 && data.mChilds.size() == 0) {
            line = "<" + TAG_CHARACTERISTIC + " type=\"" + data.mType + "\"/>";
            out.write(line.getBytes());
            return;
        }

        line = "<" + TAG_CHARACTERISTIC + " type=\"" + data.mType + "\">";
        out.write(line.getBytes());

        // write parm element and name, value attributes
        // <parm name="" value=""/>
        for (Map.Entry<String, String> entry : data.mParms.entrySet()) {
            line = "<parm name=\"" + entry.getKey() + "\" value=\"" + entry.getValue() + "\"/>";
            out.write(line.getBytes());
        }

        // write child characteristic tag
        for (Characteristic child : data.mChilds) {
            writeCharacteristic(out, child);
        }

        // write characteristic end tag
        line = "</characteristic>";
        out.write(line.getBytes());
    }

    /**
     * print whole parsed data
     */
    @VisibleForTesting
    public void dumpLog() {
        dumpLog(mRoot, "-");
    }

    private void dumpLog(Characteristic characteristic, String prefix) {
        ImsLog.i(prefix + ": type : " + characteristic.mType);
        for (Map.Entry<String, String> entry : characteristic.mParms.entrySet()) {
            ImsLog.i(prefix + ": parm : " + entry.getKey() + "=" + entry.getValue());
        }
        for (Characteristic c : characteristic.mChilds) {
            dumpLog(c, prefix + "  -");
        }
    }
}
