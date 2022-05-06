#ifndef INTERFACE_ATTR_H_
#define INTERFACE_ATTR_H_

#include "INode.h"

class IElement;

/**
 * @brief This class provides the APIs to control an XML attribute.
 *
 * @see IElement, INode
 */
class IAttr : public INode
{
public:
    /**
     * @brief Returns the name of this attribute.
     *
     * If Node#m_strLocalName is different from null, this attribute is a qualified name.
     *
     * @return The attribute name.
     */
    virtual const AString& GetName() const = 0;

    /**
     * @brief Returns the element node that this attribute is attached to.
     *
     * @return The element node or null
     */
    virtual IElement* GetOwnerElement() const = 0;

    /**
     * @brief Returns a flag specifying whether this attribute was explicitly given a value
     *        in the original document.
     *
     * @return If this attribute was explicitly given a value in the original document,
     *         returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL GetSpecified() const = 0;

    /**
     * @brief Returns the value of this attribute.
     *
     * @return A string containing the value of this attribute.
     */
    virtual const AString& GetValue() const = 0;

    /**
     * @brief Returns a flag specifying whether this attribute is known to be of type ID or not.
     *
     * In other words, whether this attribute contains an identifier for its owner element or not.
     *
     * @return If the attribute is of type ID, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
     */
    virtual IMS_BOOL IsId() const = 0;

    /**
     * @brief Sets the value of this attribute.
     *
     * On setting, this creates a Text node with the unparsed contents of the string.
     *
     * @param strValue A string containing the value of this attribute
     */
    virtual void SetValue(IN const AString& strValue) = 0;
};

#endif
