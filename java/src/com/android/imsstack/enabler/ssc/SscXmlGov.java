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

package com.android.imsstack.enabler.ssc;

import com.android.imsstack.enabler.ssc.data.SscServiceData;
import com.android.imsstack.enabler.ssc.data.SscServiceQueryData;
import com.android.imsstack.util.ImsLog;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.xml.sax.InputSource;

import java.io.StringReader;
import java.io.StringWriter;
import java.util.HashMap;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.TransformerException;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;

public class SscXmlGov {
    private static HashMap<Integer, SscXmlGov> sUtXmlGovs = new HashMap<>();
    private SscXmlParser mSscXmlParser = null;
    private SscXmlCreator mSscXmlCreator = null;
    private Document mSimservDoc = null;
    private Document mSimservDocForUpdate = null;

    private SscXmlGov() {
        ImsLog.d("");
        mSscXmlCreator = new SscXmlCreator();
        mSscXmlParser = new SscXmlParser();
    }

    protected static SscXmlGov getInstance(int slotId) {
        if (!sUtXmlGovs.containsKey(slotId)) {
            sUtXmlGovs.put(slotId, new SscXmlGov());
        }
        return sUtXmlGovs.get(slotId);
    }

    protected void clear() {
        setXmlData(null);
    }

    protected void updateXmlData(int responseCode) {
        if (responseCode >= 200 || responseCode < 300) {
            mSimservDoc = mSimservDocForUpdate;
        }
        mSimservDocForUpdate = null;
    }

    protected boolean isXmlDataPresent() {
        return (mSimservDoc != null) ? true : false;
    }

    protected SscServiceData parseXmlStream(SscServiceData updateData, Document document) {
        SscServiceQueryData convertedData = new SscServiceQueryData(updateData.getSlotId(),
                updateData.getSsType(), updateData.getEventNumber(), updateData.getTransactionId(),
                updateData.getServiceClass());
        convertedData.setResponseCode(updateData.getResponseCode());
        return parseXmlStream(convertedData, document);
    }

    protected SscServiceData parseXmlStream(SscServiceQueryData queryData, Document document) {
        if (document == null) {
            return null;
        }

        Element rootElement = document.getDocumentElement();
        if (rootElement == null) {
            return null;
        }

        String strDocument = getStringFromDoc(rootElement);

        int slotId = queryData.getSlotId();
        if (SscConfig.isOmitNamespaceSs(slotId)) {
            strDocument = removeNamespace(strDocument, SscXmlFormat.NS_SS_PREFIX);
            document = getDocFromString(strDocument);
        }

        if (SscConfig.isOmitNamespaceCp(slotId)) {
            strDocument = removeNamespace(strDocument, SscXmlFormat.NS_CP_PREFIX);
            document = getDocFromString(strDocument);
        }

        if (queryData.getSsType() == ESsType.NONE
                && queryData.getResponseCode() == SscConstant.HTTP_OK) {
            if (SscConfig.isOmitNamespaceOfDocumentElement(slotId)) {
                String docElementTag = SscXmlFormat.NS_SS_PREFIX + SscXmlFormat.SIMSERVS;
                strDocument = strDocument.replace(docElementTag, SscXmlFormat.SIMSERVS);
                document = getDocFromString(strDocument);
            }
            setXmlData(document);
        }

        ImsLog.d(slotId, "\n" + strDocument);

        SscServiceData data = mSscXmlParser.getSSCServiceFromDoc(queryData, document, mSimservDoc);
        if (data == null) {
            ImsLog.e("SscServiceData is null");
            return null;
        }

        return data;
    }

    protected String createXmlStream(SscServiceData data) {
        if (mSimservDoc == null) {
            ImsLog.e(data.getSlotId(), "mSimservDoc is null");
            return null;
        }

        mSimservDocForUpdate = getNewDoc();
        if (mSimservDocForUpdate == null) {
            return null;
        }

        Element rootElement =
                (Element) mSimservDocForUpdate.importNode(mSimservDoc.getDocumentElement(), true);
        mSimservDocForUpdate.appendChild(rootElement);

        Element resultXml = mSscXmlCreator.createXML(mSimservDocForUpdate, data);
        return getStringFromDoc(resultXml);
    }

    private void setXmlData(Document doc) {
        mSimservDoc = doc;
    }

    private String removeNamespace(String document, String namespace) {
        return (document != null) ? document.replace(namespace, "") : null;
    }

    private String getStringFromDoc(Element xmlElement) {
        if (xmlElement == null) {
            return null;
        }

        String output = null;
        TransformerFactory tf = TransformerFactory.newInstance();
        Transformer transformer;
        try {
            transformer = tf.newTransformer();
            transformer.setOutputProperty(OutputKeys.OMIT_XML_DECLARATION, "yes");
            transformer.setOutputProperty(OutputKeys.INDENT, "yes");
            transformer.setOutputProperty("{http://xml.apache.org/xslt}indent-amount", "4");
            StringWriter writer = new StringWriter();
            transformer.transform(new DOMSource(xmlElement), new StreamResult(writer));
            output = writer.getBuffer().toString();
        } catch (TransformerConfigurationException e) {
            ImsLog.e(e.toString());
            e.printStackTrace();
            return null;
        } catch (TransformerException e) {
            ImsLog.e(e.toString());
            e.printStackTrace();
            return null;
        }

        return output;
    }

    private Document getDocFromString(String xml) {
        if (xml == null) {
            return null;
        }

        DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
        Document document = null;
        try {
            docFactory.setNamespaceAware(false);
            DocumentBuilder builder = docFactory.newDocumentBuilder();
            InputSource is = new InputSource(new StringReader(xml));
            document = builder.parse(is);
        } catch (Exception e) {
            ImsLog.e(e.toString());
            e.printStackTrace();
            return null;
        }

        return document;
    }

    private Document getNewDoc() {
        Document doc = null;
        try {
            DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder docBuilder = docFactory.newDocumentBuilder();
            doc = docBuilder.newDocument();
            doc.setXmlStandalone(true);
        } catch (Exception e) {
            ImsLog.e(e.toString());
            e.printStackTrace();
            return null;
        }

        return doc;
    }
}
