/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100720  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _REG_INFO_MANAGER_H_
#define _REG_INFO_MANAGER_H_

#include "AString.h"
#include "IMSMap.h"
#include "IRegInfoParserListener.h"
#include "RegKey.h"

class IMutex;
class IDocument;
#ifdef __IMS_ASYNC_XML_PARSER__
class IXmlTransactionProvider;
#endif
class IRegInfoListener;
class RegInfo;
class RegInfoParser;

class RegInfoManager : public IRegInfoParserListener
{
private:
    RegInfoManager();
    virtual ~RegInfoManager();

    RegInfoManager(IN const RegInfoManager& objRHS);
    RegInfoManager& operator=(IN const RegInfoManager& objRHS);

public:
    IMS_BOOL CreateRegInfo(IN const RegKey& objRegKey);
    void DestroyRegInfo(IN const RegKey& objRegKey);
    RegInfo* GetRegInfo(IN const RegKey& objRegKey);
    const RegInfo* GetRegInfo(IN const RegKey& objRegKey) const;
    IMS_BOOL Initialize();
    IMS_BOOL Update(IN const RegKey& objRegKey, IN const AString& strRegInfo);

    // Debugging ...
    void DisplayRegInfo();

    static RegInfoManager* GetInstance();

private:
    // IRegInfoParserListener interface
    virtual void RegInfoParser_ParsingCompleted(
            IN RegInfoParser* pParser, IN IDocument* piDocument);
    virtual void RegInfoParser_ParsingFailed(IN RegInfoParser* pParser);

    IMS_BOOL AddRegInfoParser(IN RegInfoParser* pParser);
    void RemoveRegInfoParser(IN RegInfoParser*& pParser);

private:
    IMutex* piLock;

    // List of "reginfo" parser
    IMSList<RegInfoParser*> objParsers;
    // < RegKey , RegInfo* >
    IMSMap<RegKey, RegInfo*> objRegInfos;

#ifdef __IMS_ASYNC_XML_PARSER__
    IXmlTransactionProvider* piXmlTxnProvider;
#endif
};

#endif  // _REG_INFO_MANAGER_H_
