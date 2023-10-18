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
#ifndef INTERFACE_NODE_H_
#define INTERFACE_NODE_H_

#include "AString.h"

class IDocument;
class INamedNodeMap;
class INodeList;

/**
 * @brief This class is the primary datatype for the entire Document Object Model.
 *
 * This class represents a single node in the document tree.
 * While all objects implementing the Node interface expose methods for dealing with children,
 * not all objects implementing the Node interface may have children.
 *
 * @see IDocument, INamedNodeMap, INodeList
 */
class INode
{
protected:
    virtual ~INode() = default;

public:
    /**
     * @brief Adds a new child node to the end of the list of children of this node.
     *
     * If a new child node is already in the tree, it is first removed.
     *
     * @param piChild A new child node to add
     * @return The node to add.
     */
    virtual INode* AppendChild(IN INode* piChild) = 0;

    /**
     * @brief Returns a duplicate of this node.
     *
     * @param bDeep If IMS_TRUE, recursively clone the subtree under the specified node;
     *              if IMS_FALSE, clone only the node itself
     *              (and its attributes, if it is an Element).
     * @return The duplicate node.
     */
    virtual INode* CloneNode(IN IMS_BOOL bDeep) = 0;

    /**
     * @brief Returns an INamedNodeMap containing the attributes of this node
     *        (if it's an element) or null otherwise.
     *
     * @return A INamedNodeMap containing the attributes of this node, or null.
     */
    virtual INamedNodeMap* GetAttributes() const = 0;

    /**
     * @brief Returns an INodeList containing all children of this node.
     *
     * If there are no children, this is an INodeLIst containing no nodes.
     *
     * @return A INodeList that contains all children of this node.
     */
    virtual INodeList* GetChildNodes() const = 0;

    /**
     * @brief Returns the first child of this node.
     *
     * If there is no such node, this returns null.
     *
     * @return The first child of this node or null.
     */
    virtual INode* GetFirstChild() const = 0;

    /**
     * @brief Returns the last child of this node.
     *
     * If there is no such node, this returns null.
     *
     * @return The last child of this node or null.
     */
    virtual INode* GetLastChild() const = 0;

    /**
     * @brief Returns the local part of the qualified name of this node.
     *
     * For nodes of any type other than ELEMENT_NODE and ATTRIBUTE_NODE and nodes created
     * with a DOM Level 1 method, such as IDocument#CreateElement, this is always null.
     *
     * @return The local part of the qualified name of this node.
     */
    virtual const AString& GetLocalName() const = 0;

    /**
     * @brief Returns the namespace URI of this node, or null if it is unspecified.
     *
     * This is not a computed value that is the result of a namespace lookup based on
     * an examination of the namespace declarations.
     * It is merely the namespace URI given at creation time.
     * For nodes of any type other than ELEMENT_NODE and ATTRIBUTE_NODE and nodes created
     * with a DOM Level 1 method, such as IDocument#CreateElement, this is always null.
     *
     * @return The namespace URI of this node, or null.
     */
    virtual const AString& GetNameSpaceUri() const = 0;

    /**
     * @brief Returns the node immediately following this node.
     *
     * If there is no such node, this returns null.
     *
     * @return The node immediately following this node or null.
     */
    virtual INode* GetNextSibling() const = 0;

    /**
     * @brief Returns the name of this node, depending on its type.
     *
     * @return The name of this node.
     */
    virtual const AString& GetNodeName() const = 0;

    /**
     * @brief Returns a code representing the type of the underlying object.
     *
     * @return A code representing the type of the underlying object.\n
     *         #ATTRIBUTE_NODE\n
     *         #CDATA_SECTION_NODE\n
     *         #COMMENT_NODE\n
     *         #DOCUMENT_FRAGMENT_NODE\n
     *         #DOCUMENT_NODE\n
     *         #DOCUMENT_TYPE_NODE\n
     *         #ELEMENT_NODE\n
     *         #ENTITY_NODE\n
     *         #ENTITY_REFERENCE_NODE\n
     *         #NOTATION_NODE\n
     *         #PROCESSING_INSTRUCTION_NODE\n
     *         #TEXT_NODE\n
     */
    virtual IMS_SINT32 GetNodeType() const = 0;

    /**
     * @brief Returns the value of this node, depending on its type.
     *
     * @return A string containing the value of this node.
     */
    virtual const AString& GetNodeValue() const = 0;

    /**
     * @brief Returns the IDocument object associated with this node.
     *
     * This is also the IDocument object used to create new nodes.
     * When this node is a IDocument or a DocumentType which is not used with any IDocument yet,
     * this is null.
     *
     * @return The IDocument object associated with this node, or null.
     */
    virtual IDocument* GetOwnerDocument() const = 0;

    /**
     * @brief Returns the parent of this node.
     *
     * All nodes, except IAttr, IDocument may have a parent.
     * However, if a node has just been created and not yet added to the tree,
     * or if it has been removed from the tree, this is null.
     *
     * @return The parent of this node, or null.
     */
    virtual INode* GetParentNode() const = 0;

    /**
     * @brief Returns the namespace prefix of this node, or null if it is unspecified.
     *
     * @return The namespace prefix of this node, or null.
     */
    virtual const AString& GetPrefix() const = 0;

    /**
     * @brief Returns the node immediately preceding this node.
     *
     * If there is no such node, this returns null.
     *
     * @return The node immediately preceding this node or null.
     */
    virtual INode* GetPreviousSibling() const = 0;

    /**
     * @brief Returns the text content of this node and its descendants.
     *
     * @return A string containing the text content of this node and its descendants.
     */
    virtual const AString& GetTextContent() const = 0;

    /**
     * @brief Checks whether this node (if it is an element) has any attributes.
     *
     * @return IMS_TRUE if this node has any attributes, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL HasAttribute() const = 0;

    /**
     * @brief Checks whether this node has any children.
     *
     * @return IMS_TRUE if this node has any children, IMS_FALSE otherwise.
     */
    virtual IMS_BOOL HasChildNode() const = 0;

    /**
     * @brief Inserts the new child node before the existing child node - reference child node.
     *
     * If piRefChild is null, insert piNewChild at the end of the list of children.
     * If the piNewChild is already in the tree, it is first removed.
     *
     * @param piNewChild The node to insert
     * @param piRefChild The reference node, i.e.,
     *                   the node before which the new node must be inserted
     * @return The node being inserted.
     */
    virtual INode* InsertBefore(IN INode* piNewChild, IN INode* piRefChild) = 0;

    /**
     * @brief Tests whether the DOM implementation implements a specific feature and
     *        that feature is supported by this node.
     *
     * @param strFeature The name of the feature to test
     * @param strVersion This is the version number of the feature to test. In Level 2,
     *                   version 1, this is the string "2.0".\n
     *                   If the version is not specified, supporting any version of the feature
     *                   will cause the method to return true.
     * @return Returns IMS_TRUE if the specified feature is supported on this node,
     *         IMS_FALSE otherwise.
     */
    virtual IMS_BOOL IsSupported(IN const AString& strFeature, IN const AString& strVersion) = 0;

    /**
     * @brief Puts all IText nodes in the full depth of the sub-tree underneath this INode,
     *        including attribute nodes, into a "normal" form where only structure
     *        (e.g., elements, comments) separates IText nodes.
     *
     * @return IMS_SUCCESS if this operation succeeds, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT Normalize() = 0;

    /**
     * @brief Removes the child node indicated by piOldChild from the list of children,
     *        and returns it.
     *
     * @param piChild The node being removed
     * @return The node removed.
     */
    virtual INode* RemoveChild(IN INode* piChild) = 0;

    /**
     * @brief Replaces the child node piOldChild with piNewChild in the list of children,
     *        and returns the piOldChild node.
     *
     * If the piNewChild is already in the tree, it is first removed.
     *
     * @param piNewChild The new node to put in the child list
     * @param piOldChild The node being replaced in the list
     * @return The node replaced.
     */
    virtual INode* ReplaceChild(IN INode* piNewChild, IN INode* piOldChild) = 0;

    /**
     * @brief Sets the value of this node, depending on its type.
     *
     * @param strNodeValue The value of the node
     */
    virtual void SetNodeValue(IN const AString& strNodeValue) = 0;

    /**
     * @brief Sets the namespace prefix of this node, or null if it is unspecified.
     *
     * @param strPrefix The namespace prefix to set
     */
    virtual void SetPrefix(IN const AString& strPrefix) = 0;

    /**
     * @brief Sets the text content of this node and its descendants.
     *
     * @param strTextContext A string containing the new text content for this node
     */
    virtual void SetTextContent(IN const AString& strTextContext) = 0;

    /**
     * @brief Destroys an INodeList.
     *
     * @param piNodeList The INodeList to destroy
     */
    virtual void DestroyNodeList(IN INodeList*& piNodeList) = 0;

    /**
     * @brief Destroys an INamedNodeMap.
     *
     * @param piNodeList The INamedNodeMap to destroy
     */
    virtual void DestroyNamedNodeMap(IN INamedNodeMap*& piNamedNodeMap) = 0;

    /**
     * @brief Sets the next sibling to this node.
     *
     * @param piNode The INode to set
     */
    virtual void SetNextSibling(INode* piNode) = 0;

    /**
     * @brief Sets the previous sibling to this node.
     *
     * @param piNode The INode to set
     */
    virtual void SetPreviousSibling(INode* piNode) = 0;

    /**
     * @brief Sets the parent node of this node.
     *
     * @param piNode The INode to set
     */
    virtual void SetParent(INode* piNode) = 0;

    /**
     * @brief Sets the child node of this node.
     *
     * @param piNode The INode to set
     * @return IMS_SUCCESS if this operation succeeds, IMS_FAILURE otherwise.
     */
    virtual IMS_RESULT SetChildren(INode* piNode) = 0;

    /**
     * @brief Sets the owner document of this node.
     *
     * @param piDocument The IDocument to set
     */
    virtual void SetOwnerDocument(IN IDocument* piDocument) = 0;

public:
    enum NODE_TYPE_ENTYPE
    {
        INVALID_NODE = -1,

        ATTRIBUTE_NODE = 0,
        CDATA_SECTION_NODE,
        COMMENT_NODE,
        DOCUMENT_FRAGMENT_NODE,
        DOCUMENT_NODE,
        DOCUMENT_TYPE_NODE,
        ELEMENT_NODE,
        ENTITY_NODE,
        ENTITY_REFERENCE_NODE,
        NOTATION_NODE,
        PROCESSING_INSTRUCTION_NODE,
        TEXT_NODE,

        MAX_NODE,
    };
};

#endif
