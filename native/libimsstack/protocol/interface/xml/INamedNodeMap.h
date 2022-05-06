#ifndef INTERFACE_NAMED_NODE_MAP_H
#define INTERFACE_NAMED_NODE_MAP_H

#include "INode.h"

/**
 * @brief This class represents an entity, either parsed or unparsed, in an XML document.
 *
 * @see INode
 */
class INamedNodeMap
{
public:
    /**
     * @brief Returns the number of nodes in this map.
     *
     * The range of valid child node indices is 0 to (length-1) inclusive.
     *
     * @return The number of nodes in this map.
     */
    virtual IMS_SINT32 GetLength() const = 0;

    /**
     * @brief Retrieves a node specified by name.
     *
     * @param strName The node name to be retrieved
     * @return An instance of INode or null if not present.
     */
    virtual INode* GetNamedItem(IN const AString& strName) const = 0;

    /**
     * @brief Retrieves a node specified by local name and namespace URI.
     *
     * @param strNamespaceUri The namespace URI
     * @param strName The node name to be retrieved
     * @return An instance of INode or null if not present.
     */
    virtual INode* GetNamedItemNs(
            IN const AString& strNamespaceUri, IN const AString& strName) const = 0;

    /**
     * @brief Returns the indexth item in the map.
     *
     * If index is greater than or equal to the number of nodes in this map, this returns null.
     *
     * @param nIndex The index into this map
     * @return An instance of INode
     */
    virtual INode* Item(IN IMS_SINT32 nIndex) const = 0;

    /**
     * @brief Removes a node specified by local name.
     *
     * A removed attribute may be known to have a default value when this map contains
     * the attributes attached to an element, as returned by the attributes attribute of the Node
     * interface. If so, an attribute immediately appears containing the default value
     * as well as the corresponding namespace URI, local name, and prefix when applicable.
     *
     * @param strName The name of the node to remove
     * @return The node removed from this map or null if not present.
     */
    virtual INode* RemoveNamedItem(IN const AString& strName) = 0;

    /**
     * @brief Removes a node specified by local name and namespace URI.
     *
     * A removed attribute may be known to have a default value when this map contains
     * the attributes attached to an element, as returned by the attributes attribute of the Node
     * interface. If so, an attribute immediately appears containing the default value
     * as well as the corresponding namespace URI, local name, and prefix when applicable.
     *
     * @param strNamespaceUri The namespace URI of the node to remove
     * @param strLocalName The local name of the node to remove
     * @return The node removed from this map or null if not present.
     */
    virtual INode* RemoveNamedItemNs(
            IN const AString& strNamespaceUri, IN const AString& strLocalName) = 0;

    /**
     * @brief Sets a node using the specified node.
     *
     * If a node with that name is already present in this map, it is replaced by the new one.
     *
     * @param piNode A node to store in this map; The node will later be accessible using the value
     *               of its nodeName attirbute.
     * @return The replaced node if the new node replaces an existing node.
     *         Otherwise, null will be returned.
     */
    virtual INode* SetNamedItem(IN INode* piNode) = 0;

    /**
     * @brief Sets a node using the specified node.
     *
     * If a node with that namespace URI and that local name is already present in this map,
     * it is replaced by the new one.
     *
     * @param piNode A node to store in this map; The node will later be accessible using the value
     *               of its nodeName attirbute.
     * @return The replaced node if the new node replaces an existing node.
     *         Otherwise, null will be returned.
     */
    virtual INode* SetNamedItemNs(IN INode* piNode) = 0;
};

#endif
