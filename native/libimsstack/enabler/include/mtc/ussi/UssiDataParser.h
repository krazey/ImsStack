/*
 * author : aromi.kwak@
 * date : 201610
 * brief : Refactoring USSIHelper.
 *         Create USSIBodyParser
 */

#ifndef USSD_DATA_PARSER_H_
#define USSD_DATA_PARSER_H_

#include "ServiceTrace.h"
#include "INodeList.h"

class USSDDataParser
{
public:
    class AnyExtension
    {
    public:
        inline AnyExtension() :
                m_nUSSType(USS_TYPE_NONE),
                m_nAlertingPattern(-1)
        {
            IMS_TRACE_MEM("uc", "uc_M : AnyExtension[%" PFLS_u "][%" PFLS_x "]",
                    sizeof(AnyExtension), this, 0);
        }
        inline ~AnyExtension()
        {
            IMS_TRACE_MEM("uc", "uc_F : AnyExtension[%" PFLS_u "][%" PFLS_x "]",
                    sizeof(AnyExtension), this, 0);
        }

    private:
        AnyExtension(IN const AnyExtension& objRHS);
        AnyExtension& operator=(IN const AnyExtension& objRHS);

    public:
        inline IMS_UINT32 GetUSSType() const { return m_nUSSType; }
        inline IMS_SINT32 GetAlertingPattern() const { return m_nAlertingPattern; }

    public:
        // USS Type
        enum
        {
            USS_TYPE_NOTIFY = 0,   // UnstructuredSS-Notify
            USS_TYPE_REQUEST = 1,  // UnstructuredSS-Request
            USS_TYPE_NONE = 2      // no USS type specified
        };

    private:
        friend class USSDDataParser;

        IMS_UINT32 m_nUSSType;

        // unsignedByte. only in network initiated USSD request or USSD notification.
        IMS_SINT32 m_nAlertingPattern;
    };

public:
    USSDDataParser();
    explicit USSDDataParser(IN const AString& str3gppIms);
    ~USSDDataParser();

    inline const AString& GetLanguage() const { return m_strLanguage; }
    inline const AString& GetUSSDString() const { return m_strUSSDString; }
    inline IMS_UINT32 GetErrorCode() const { return m_nErrorCode; }

private:
    USSDDataParser(IN const USSDDataParser& objRHS);
    USSDDataParser& operator=(IN const USSDDataParser& objRHS);

public:
    const AnyExtension& GetAnyExtension() const;

    IMS_BOOL Parse(IN const AString& strUSSIBody);

private:
    void CreateAnyExtension(IN INode* piNode);

public:
    // error-code in ussd data
    enum
    {
        ERROR_CODE_NONE = 0,  // no error-code specified
        ERROR_CODE_1 = 1,     // unspecified. default error-code.
        ERROR_CODE_2 = 2,     // language/alphabet not supported
        ERROR_CODE_3 = 3,     // unexpected data value
        ERROR_CODE_4 = 4      // USSD-busy
    };

private:
    AnyExtension objAnyExtension;

    AString m_strLanguage;
    AString m_strUSSDString;
    IMS_UINT32 m_nErrorCode;
};

#endif  // USSD_DATA_PARSER_H_
