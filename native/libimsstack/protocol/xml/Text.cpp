#include "Text.h"

PUBLIC
Text::Text() :
        CharacterData()
{
    m_nNodeType = INode::TEXT_NODE;
}

PUBLIC
Text::Text(IN xmlNodePtr pstNode) :
        CharacterData(pstNode)
{
    m_nNodeType = INode::TEXT_NODE;
}

PUBLIC VIRTUAL Text::~Text() {}

PUBLIC VIRTUAL IText* Text::SplitText(IN IMS_SINT32 nOffset)
{
    (void)nOffset;
    return IMS_NULL;
}
