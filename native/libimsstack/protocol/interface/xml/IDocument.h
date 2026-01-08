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
#ifndef INTERFACE_DOCUMENT_H_
#define INTERFACE_DOCUMENT_H_

#include "INode.h"

class IAttr;
class IElement;
class IText;

/**
 * @brief This class represents the entire HTML or XML document.
 *
 * Conceptually, it is the root of the document tree, and provides the primary access
 * to the document's data.
 *
 * @see IAttr, IElement, INode, IText
 */
class IDocument : public INode
{
protected:
    ~IDocument() override = default;

public:
    /**
     * @brief Attempts to adopt a node from another document to this document.
     *
     * @param piNode The node to move into this document
     * @return The adopted node, or null if this operation fails.
     */
    virtual INode* AdoptNode(IN INode* piNode) = 0;

    /**
     * @brief Creates an Attr of the given name.
     *
     * Note that the Attr instance can then be set on an Element
     * using the #SetAttributeNode method.
     * To create an attribute with a qualified name and namespace URI,
     * use the #CreateAttributeNs method.
     *
     * @param strName The name of the attribute
     * @return A new IAttr object with the nodeName attribute set to name,
     *         and localName, prefix, and namespace URI set to null.\n
     *         The value of the attribute is the empty string.
     */
    virtual IAttr* CreateAttribute(IN const AString& strName) = 0;

    /**
     * @brief Creates an attribute of the given qualified name and namespace URI.
     *
     * Applications must use the value null as the namespaceURI parameter
     * for methods if they wish to have no namespace.
     *
     * @param strNamespaceUri The namespace URI of the attribute to create
     * @param strQualifiedName The qualified name of the attribute to instantiate
     * @return A new IAttr object.
     */
    virtual IAttr* CreateAttributeNs(
            IN const AString& strNamespaceUri, IN const AString& strQualifiedName) = 0;

    /**
     * @brief Creates an element of the type specified.
     *
     * Note that the instance returned implements the IElement interface,
     * so attributes can be specified directly on the returned object.
     * In addition, if there are known attributes with default values,
     * IAttr nodes representing them are automatically created and attached to the element.
     * To create an element with a qualified name and namespace URI,
     * use the CreateElementNs method.
     *
     * @param strTagName The name of the element type to instantiate.\n
     *                   For XML, this is case-sensitive. otherwise it depends on
     *                   the case-sensitivity of the markup language in use.\n
     *                   In that case, the name is mapped to the canonical form of
     *                   that markup by the DOM implementation.
     * @return A new IElement object with the nodeName attribute set to tagName,
     *         and localName, prefix, and namespaceURI set to null.
     */
    virtual IElement* CreateElement(IN const AString& strTagName) = 0;

    /**
     * @brief Creates an element of the given qualified name and namespace URI.
     *
     * Applications must use the value null as the namespaceURI parameter
     * for methods if they wish to have no namespace.
     *
     * @param strNamespaceUri The namespace URI of the element to create
     * @param strQualifiedName The qualified name of the element to instantiate
     * @return A new IElement object.
     */
    virtual IElement* CreateElementNs(
            IN const AString& strNamespaceUri, IN const AString& strQualifiedName) = 0;

    /**
     * @brief Creates a text node given the specified string.
     *
     * @param strData The data for the node
     * @return A new IText object.
     */
    virtual IText* CreateTextNode(IN const AString& strData) = 0;

    /**
     * @brief Returns a root element of the document.
     *
     * This is a convenience method that allows direct access to the child node
     * that is the root element of the document.
     *
     * @return A child node that is the root element of the document.
     */
    virtual IElement* GetDocumentElement() const = 0;

    /**
     * @brief Returns the element that has an ID attribute with the given value.
     *
     * If no such element exists, this returns null.
     * If more than one element has an ID attribute with that value,
     * what is returned is undefined.
     *
     * @param strElementId The unique id value for an element
     * @return The matching element or null if there is none.
     */
    virtual IElement* GetElementById(IN const AString& strElementId) const = 0;

    /**
     * @brief Returns an INodeList of all the elements with a given tag name in the order
     *        in which they are encountered in a preorder traversal of the document tree.
     *
     * @param strTagName The name of the tag to match on\n
     *                   The special value "*" matches all tags.\n
     *                   For XML, the tagname parameter is case-sensitive,
     *                   otherwise it depends on the case-sensitivity of
     *                   the markup language in use.
     * @return A new INodeList object containing all the matched elements.
     */
    virtual INodeList* GetElementsByTagName(IN const AString& strTagName) const = 0;

    /**
     * @brief Returns a NodeList of all the Elements with a given local name and namespace URI
     *        in document order.
     *
     * @param strNamespaceUri The namespace URI of the elements to match on\n
     *                        The special value "*" matches all namespaces.
     * @param strLocalName The local name of the elements to match on\n
     *                     The special value "*" matches all local names.
     * @return A new INodeList object containing all the matched elements.
     */
    virtual INodeList* GetElementsByTagNameNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) const = 0;

    /**
     * @brief Imports a node from another document to this document,
     *        without altering or removing the source node from the original document;
     *        this method creates a new copy of the source node.
     *        The returned node has no parent; (parentNode is null).
     *
     * @param piImportNode The node to import
     * @param bDeep If true, recursively import the subtree under the specified node;
     *              if false, import only the node itself, as explained above.\n
     *              This has no effect on nodes that cannot have any children,
     *              and on Attr, and EntityReference nodes.
     * @return The imported node that belongs to this Document.
     */
    virtual INode* ImportNode(IN INode* piNode, IN IMS_BOOL bDeep) = 0;

    /**
     * @brief Destroys the document.
     */
    virtual void DestroyDocument() = 0;

    /**
     * @brief Returns the encoding scheme of this document.
     *
     * @return An encoding string.
     */
    virtual const AString& GetEncodingScheme() const = 0;

    /**
     * @brief Returns the URL of this document.
     *
     * @return A URL string.
     */
    virtual const AString& GetUrl() const = 0;

    /**
     * @brief Returns the version of this document.
     *
     * @return A version string.
     */
    virtual const AString& GetVersion() const = 0;
};

#endif
