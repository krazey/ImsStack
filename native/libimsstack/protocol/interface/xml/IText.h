#ifndef INTERFACE_TEXT_H
#define INTERFACE_TEXT_H

#include "ICharacterData.h"

/**
 * @brief This class represents the textual content (termed character data in XML)
 *        of an element or attribute.
 *
 * @see ICharacterData
 */
class IText : public ICharacterData
{
public:
    /**
     * @brief Breaks this node into two nodes at the specified offset,
     *        keeping both in the tree as siblings.
     *
     * After being split, this node will contain all the content up to the offset point.
     *
     * @param nOffset The 16-bit unit offset at which to split, starting from 0
     * @return The new node, of the same type as this node.
     */
    virtual IText* SplitText(IN IMS_SINT32 nOffset) = 0;
};

#endif
