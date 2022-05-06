#ifndef CHARACTER_DATA_H_
#define CHARACTER_DATA_H_

#include "Node.h"

class CharacterData : public Node
{
public:
    CharacterData();
    CharacterData(IN xmlNodePtr pstNode);
    virtual ~CharacterData();

    // CharacterData
    virtual void AppendData(IN const AString& strData);
    virtual void DeleteData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount);
    virtual const AString& GetData() const;
    virtual IMS_SINT32 GetLength() const;
    virtual void InsertData(IN IMS_SINT32 nOffset, IN const AString& strData);
    virtual void ReplaceData(
            IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount, IN const AString& strData);
    virtual void SetData(IN const AString& strData);
    virtual AString SubstringData(IN IMS_SINT32 nOffset, IN IMS_SINT32 nCount);

protected:
    AString m_strCharacterData;
};

#endif
