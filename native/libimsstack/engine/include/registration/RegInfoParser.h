/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20100720  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _REG_INFO_PARSER_H_
#define _REG_INFO_PARSER_H_

#ifdef __IMS_ASYNC_XML_PARSER__

#include "IXmlTransactionListener.h"
#include "IXmlTransaction.h"
#include "IXmlTransactionProvider.h"
#include "RegKey.h"

class IRegInfoParserListener;

class RegInfoParser : public IXmlTransactionListener
{
public:
    RegInfoParser(IN const RegKey& objRegKey_, IN IXmlTransactionProvider*& piXmlTxnProvider);
    virtual ~RegInfoParser();

public:
    const RegKey& GetRegKey() const;
    IMS_BOOL Parse(IN const AString& strRegInfo);
    void SetListener(IN IRegInfoParserListener* piListener);

private:
    // IXmlTransactionListener interface
    virtual IMS_RESULT XmlTransaction_NotifyParsingCompleted(IN IXmlTransaction* piTransaction);

private:
    RegKey objRegKey;
    IXmlTransactionProvider*& piXmlTxnProvider;

    IXmlTransaction* piXMLTxn;
    IRegInfoParserListener* piListener;
};

#else

#include "AString.h"
#include "RegKey.h"

class IRegInfoParserListener;

class RegInfoParser
{
public:
    explicit RegInfoParser(IN const RegKey& objRegKey_);
    virtual ~RegInfoParser();

public:
    const RegKey& GetRegKey() const;
    IMS_BOOL Parse(IN const AString& strRegInfo);
    void SetListener(IN IRegInfoParserListener* piListener);

private:
    RegKey objRegKey;

    IRegInfoParserListener* piListener;
};

#endif  // __IMS_ASYNC_XML_PARSER__

#endif  // _REG_INFO_PARSER_H_
