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
#ifndef INTERFACE_ELEMENT_H
#define INTERFACE_ELEMENT_H

#include "INode.h"

class IAttr;
class INodeList;

/**
 * @brief This class represents an element in an XML document.
 *
 * @see IAttr, INode, INodeList
 */
class IElement : public INode
{
protected:
    ~IElement() override = default;

public:
    /**
     * @brief Retrieves an attribute value by name.
     *
     * @param strName The name of the attribute to retrieve
     * @return The attribute value as a string, or the empty string
     *         if that attribute does not have a specified or default value.
     */
    virtual const AString& GetAttribute(IN const AString& strName) const = 0;

    /**
     * @brief Retrieves an attribute node by name.
     *
     * To retrieve an attribute node by qualified name and namespace URI,
     * use the GetAttributeNodeNs method.
     *
     * @param strName The name (nodeName) of the attribute to retrieve
     * @return The attribute node with the specified name(nodeName) or null
     *         if there is no such attribute.
     */
    virtual IAttr* GetAttributeNode(IN const AString& strName) const = 0;

    /**
     * @brief Retrieves an IAttr node by local name and namespace URI.
     *
     * @param strNamespaceUri The namespace URI of the attribute to retrieve
     * @param strLocalName The local name of the attribute to retrieve
     * @return The IAttr node with the specified attribute local name
     *         and namespace URI or null if there is no such attribute.
     */
    virtual IAttr* GetAttributeNodeNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) const = 0;

    /**
     * @brief Retrieves an attribute value by local name and namespace URI.
     *
     * @param strNamespaceUri The namespace URI of the attribute to retrieve
     * @param strLocalName The local name of the attribute to retrieve
     * @return The attribute value as a string, or the empty string
     *         if that attribute does not have a specified or default value.
     */
    virtual const AString& GetAttributeNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) const = 0;

    /**
     * @brief Returns an INodeList of all descendant elements with a given tag name,
     *        in the order in which they are encountered in a preorder traversal of
     *        this element tree.
     *
     * @param strName The name of the tag to match on\n
     *                The special value "*" matches all tags.
     * @return A list of matching element nodes.
     */
    virtual INodeList* GetElementsByTagName(IN const AString& strName) const = 0;

    /**
     * @brief Returns an INodeList of all the descendant elements with a given local name
     *        and namespace URI in the order in which they are encountered in a preorder traversal
     *        of this element tree.
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
     * @brief Returns the name of the element.
     *
     * @return A string containing the name of the element.
     */
    virtual const AString& GetTagName() const = 0;

    /**
     * @brief Returns true when an attribute with a given name is specified on this element
     *        or has a default value, false otherwise.
     *
     * @param strName The name of the attribute to look for
     * @return IMS_TRUE if an attribute with the given name is specified on this element
     *         or has a default value, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL HasAttributeB(IN const AString& strName) const = 0;

    /**
     * @brief Returns true when an attribute with a given local name and namespace URI is specified
     *        on this element or has a default value, false otherwise.
     *
     * @param strNamespaceUri The namespace URI of the attribute to look for
     * @param strLocalName The local name of the attribute to look for
     * @return IMS_TRUE if an attribute with the given local name and namespace URI is specified
     *         or has a default value on this element, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL HasAttributeNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) const = 0;

    /**
     * @brief Removes an attribute by name.
     *
     * If the removed attribute is known to have a default value,
     * an attribute immediately appears containing the default value as well as
     * the corresponding namespace URI, local name, and prefix when applicable.
     * To remove an attribute by local name and namespace URI, use the RemoveAttributeNs method.
     *
     * @param strName The name of the attribute to remove
     */
    virtual void RemoveAttribute(IN const AString& strName) = 0;

    /**
     * @brief Removes the specified attribute node.
     *
     * If the removed Attr has a default value it is immediately replaced.
     * The replacing attribute has the same namespace URI and local name,
     * as well as the original prefix, when applicable.
     *
     * @param piAttr The IAttr node to remove from the attribute list
     * @return The Attr node that was removed.
     */
    virtual IAttr* RemoveAttributeNode(IN IAttr* piAttr) = 0;

    /**
     * @brief Removes an attribute by local name and namespace URI.
     *
     * If the removed attribute has a default value it is immediately replaced.
     * The replacing attribute has the same namespace URI and local name,
     * as well as the original prefix.
     *
     * @param strNamespaceUri The namespace URI of the attribute to remove
     * @param strLocalName The local name of the attribute to remove
     */
    virtual void RemoveAttributeNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) = 0;

    /**
     * @brief Sets a new attribute.
     *
     * If an attribute with that name is already present in the element,
     * its value is changed to be that of the value parameter.
     *
     * @param strName The name of the attribute to create or alter
     * @param strValue The value to set in string form
     */
    virtual void SetAttribute(IN const AString& strName, IN const AString& strValue) = 0;

    /**
     * @brief Sets a new attribute node.
     *
     * If an attribute with that name(nodeName) is already present in the element,
     * it is replaced by the new one.
     * To add a new attribute node with a qualified name and namespace URI,
     * use the SetAttributeNodeNs method.
     *
     * @param piAttr The IAttr node to add to the attribute list
     * @return If the new attribute replaces an existing attribute,
     *         the replaced IAttr node is returned, otherwise null is returned.
     */
    virtual IAttr* SetAttributeNode(IN IAttr* piAttr) = 0;

    /**
     * @brief Sets a new attribute node with the namespace.
     *
     * If an attribute with that local name and that namespace URI
     * is already present in the element, it is replaced by the new one.
     *
     * @param piAttr The IAttr node to add to the attribute list
     * @return If the new attribute replaces an existing attribute with the same local name
     *         and namespace URI, the replaced IAttr node is returned, otherwise null is returned.
     */
    virtual IAttr* SetAttributeNodeNs(IN IAttr* piAttr) = 0;

    /**
     * @brief Sets a new attribute with the namespace.
     *
     * @param strNamespaceUri The namespace URI of the attribute to create or alter
     * @param strQualifiedName The qualified name of the attribute to create or alter
     * @param strValue The value to set in string form
     */
    virtual void SetAttributeNs(IN const AString& strNamespaceUri,
            IN const AString& strQualifiedName, IN const AString& strValue) = 0;

    /**
     * @brief Sets an ID attribute.
     *
     * If the parameter bIsId is IMS_TRUE, this method declares the specified attribute to be
     * a user-determined ID attribute .
     *
     * @param strName The name of the attribute
     * @param bIsId Flag specifying whether the attribute is a of type ID
     */
    virtual void SetIdAttribute(IN const AString& strName, IN IMS_BOOL bIsId) = 0;

    /**
     * @brief Sets an ID attribute node.
     *
     * If the parameter bIsId is IMS_TRUE, this method declares the specified attribute to be
     * a user-determined ID attribute .
     *
     * @param piIdAttr The attribute node
     * @param bIsId Flag specifying whether the attribute is a of type ID
     */
    virtual void SetIdAttributeNode(IN IAttr* piIdAttr, IN IMS_BOOL bIsId) = 0;

    /**
     * @brief Sets an ID attribute with the namespace.
     *
     * If the parameter bIsId is IMS_TRUE, this method declares the specified attribute to be
     * a user-determined ID attribute .
     *
     * @param strNamespaceUri The namespace URI of the attribute
     * @param strLocalName The local name of the attribute
     * @param bIsId Flag specifying whether the attribute is a of type ID
     */
    virtual void SetIdAttributeNodeNs(IN const AString& strNamespaceUri,
            IN const AString& strLocalName, IN IMS_BOOL bIsId) = 0;

    /**
     * @brief Retrieves an attribute value by name.
     *
     * @param pszName The name of the attribute to retrieve
     * @return The attribute value as a string, or the empty string
     *         if that attribute does not have a specified or default value.
     */
    virtual const AString& GetAttribute(IN const IMS_CHAR* pszName) const = 0;

    /**
     * @brief Retrieves an attribute node by name.
     *
     * @param pszName The name (nodeName) of the attribute to retrieve
     * @return The attribute node with the specified name(nodeName) or null
     *         if there is no such attribute.
     */
    virtual IAttr* GetAttributeNode(IN const IMS_CHAR* pszName) const = 0;

    /**
     * @brief Returns an INodeList of all descendant elements with a given tag name,
     *        in the order in which they are encountered in a preorder traversal of
     *        this element tree.
     *
     * @param pszName The name of the tag to match on\n
     *                The special value "*" matches all tags.
     * @return A list of matching element nodes.
     */
    virtual INodeList* GetElementsByTagName(IN const IMS_CHAR* pszName) const = 0;
};

#endif
