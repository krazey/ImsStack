#ifndef TEXT_H_
#define TEXT_H_

#include "CharacterData.h"

class IText;

class Text : public CharacterData
{
public:
    Text();
    Text(IN xmlNodePtr pstNode);
    virtual ~Text();

    virtual IText* SplitText(IN IMS_SINT32 nOffset);
};

#endif
