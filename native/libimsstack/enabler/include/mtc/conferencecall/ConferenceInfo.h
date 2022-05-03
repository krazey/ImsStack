#ifndef CONFERENCE_INFO_H_
#define CONFERENCE_INFO_H_

#include "AString.h"

#include "conferencecall/ConferenceDef.h"

class INode;
class IElement;

class ConferenceInfo
{
public:
    class ConferenceDescription final
    {
    public:
        inline explicit ConferenceDescription() :
                nMaxUserCount(DEFAULT_MAX_USER_COUNT)
        {
        }
        inline ~ConferenceDescription() {}
        ConferenceDescription(IN const ConferenceDescription&) = delete;
        ConferenceDescription& operator=(IN const ConferenceDescription&) = delete;

    public:
        inline IMS_UINT32 GetMaxUserCount() const { return nMaxUserCount; }

    private:
        friend class ConferenceInfo;
        static const IMS_UINT32 DEFAULT_MAX_USER_COUNT = 6;

        // list conf-uris
        // list service-uris
        IMS_UINT32 nMaxUserCount;
        // available-media
    };

    class HostInfo final
    {
    public:
        inline explicit HostInfo() :
                strDisplayText(AString::ConstNull()),
                objUris(IMSList<AString>())
        {
        }
        inline ~HostInfo() {}
        HostInfo(IN const HostInfo&) = delete;
        HostInfo& operator=(IN const HostInfo&) = delete;

    public:
        inline const AString& GetDisplayText() const { return strDisplayText; }
        inline const IMSList<AString> GetUris() const { return objUris; }

    private:
        friend class ConferenceInfo;

        AString strDisplayText;
        // str web-page
        IMSList<AString> objUris;
    };

    class ConferenceState final
    {
    public:
        inline explicit ConferenceState() :
                nUserCount(0)
        {
        }
        inline ~ConferenceState() {}
        ConferenceState(IN const ConferenceState&) = delete;
        ConferenceState& operator=(IN const ConferenceState&) = delete;

    public:
        inline IMS_UINT32 GetUserCount() const { return nUserCount; }

    private:
        friend class ConferenceInfo;

        IMS_UINT32 nUserCount;
        // boolean active
        // boolean locaked
    };

    class User final
    {
    public:
        class EndPoint final
        {
        public:
            inline explicit EndPoint() :
                    strEntity(AString::ConstNull()),
                    nState(STATE_INVALID),
                    strDisplayText(AString::ConstNull()),
                    nStatus(STATUS_INVALID)
            {
            }
            inline ~EndPoint() {}
            EndPoint(IN const EndPoint&) = delete;
            EndPoint& operator=(IN const EndPoint&) = delete;

        public:
            inline const AString& GetEntity() const { return strEntity; }
            inline IMS_UINT32 GetState() const { return nState; }
            inline const AString& GetDisplayText() const { return strDisplayText; }
            inline IMS_UINT32 GetStatus() const { return nStatus; }

        private:
            friend class ConferenceInfo;

            AString strEntity;
            IMS_UINT32 nState;
            AString strDisplayText;
            // referred
            IMS_UINT32 nStatus;
            // joining-method
            // joining-info
            // disconnected-method
            // disconnection-info
        };

        class Media final
        {
        public:
            inline explicit Media() :
                    nId(0),
                    strDisplayText(AString::ConstNull()),
                    nType(CONF_MEDIA_TYPE_AUDIO),
                    strLabel(AString::ConstNull()),
                    nStatus(CONF_MEDIA_STATUS_SENDRECV)
            {
            }
            inline ~Media() {}

        private:
            Media(IN const Media&) = delete;
            Media& operator=(IN const Media&) = delete;

        public:
            inline IMS_UINT32 GetId() const { return nId; }
            inline const AString& GetDisplayText() const { return strDisplayText; }
            inline IMS_UINT32 GetType() const { return nType; }
            inline const AString& GetLabel() const { return strLabel; }
            inline IMS_UINT32 GetStatus() const { return nStatus; }

        private:
            friend class ConferenceInfo;

            IMS_UINT32 nId;
            AString strDisplayText;
            IMS_UINT32 nType;
            AString strLabel;
            // src-id
            IMS_UINT32 nStatus;
        };

    public:
        inline explicit User() :
                strEntity(AString::ConstNull()),
                nState(STATE_INVALID),
                strDisplayText(AString::ConstNull())
        {
        }
        ~User();
        User(IN const User&) = delete;
        User& operator=(IN const User&) = delete;

    public:
        inline const AString& GetEntity() const { return strEntity; }
        inline IMS_UINT32 GetState() const { return nState; }
        inline const AString& GetDisplayText() const { return strDisplayText; }
        inline const IMSList<EndPoint*>& GetEndPoints() const { return objEndPoints; }

    private:
        friend class ConferenceInfo;

        AString strEntity;
        IMS_UINT32 nState;
        AString strDisplayText;
        IMSList<EndPoint*> objEndPoints;
        // associated-aors
        // roles
        // languares
    };

public:
    explicit ConferenceInfo();
    explicit ConferenceInfo(IN const AString& strConferenceInfo);
    ~ConferenceInfo();
    ConferenceInfo(IN const ConferenceInfo&) = delete;
    ConferenceInfo& operator=(IN const ConferenceInfo&) = delete;

public:
    const ConferenceDescription& GetConferenceDescription() const;
    const HostInfo& GetHostInfo() const;
    const ConferenceState& GetConferenceState() const;
    const IMSList<User*>& GetUsers() const;

    inline IMS_UINT32 GetState() const { return m_nState; }
    inline IMS_SINT32 GetVersion() const { return m_nVersion; }

    IMS_BOOL Parse(IN const AString& strConferenceInfoPackage);

private:
    void CreateConferenceInfo(IN const IElement* piElement);
    void CreateConferenceDescription(IN const INode* piNode);
    void CreateHostInfo(IN const INode* piNode);
    void CreateConferenceState(IN const INode* piNode);
    void CreateUsers(IN const INode* piNode);
    // CreateSideBarsByRef()
    // CreateSideBarsByVal()

    void CreateEndPointEntity(IN const IElement* piElement, IN User* pUser);
    void CreateMedia(IN const IElement* piElement, IN User::EndPoint* pEndPoint);
    void CreateCallInfo(IN const IElement* piElement, IN User::EndPoint* pEndPoint);

    const IElement* GetSubElement(
            IN const IElement* piElement, IN const IMS_CHAR* pszSubElementName);
    const IMSList<IElement*>& GetSubElements(IN const IElement* piElement,
            IN const IMS_CHAR* pszSubElementName, OUT IMSList<IElement*>& objSubElements);
    const AString& GetSubElementValue(IN const IElement* piElement,
            IN const IMS_CHAR* pszSubElementName, OUT AString& strSubElementValue);

    IMS_UINT32 ConvertState(IN const AString& strState);
    IMS_UINT32 ConvertStatus(IN const AString& strStatus);

public:
    // no meaning?
    enum
    {
        TYPE_UNKNOWN = 0,
        TYPE_CONFERENCE_DESCRIPTION = 1,
        TYPE_HOST_INFO = 2,
        TYPE_CONFERENCE_STATE = 3,
        TYPE_USERS = 4,
        TYPE_SIDEBARS_BY_REF = 5,
        TYPE_SIDEBARS_BY_VAL = 6
    };

    enum
    {
        STATE_INVALID = 0,
        STATE_FULL = 1,
        STATE_PARTIAL = 2,
        STATE_DELETED = 3
    };

    static const IMS_CHAR ELEMENT_CONFERENCE_INFO[];
    static const IMS_CHAR ELEMENT_CONFERENCE_DESCRIPTION[];
    static const IMS_CHAR ELEMENT_HOST_INFO[];
    static const IMS_CHAR ELEMENT_CONFERENCE_STATE[];
    static const IMS_CHAR ELEMENT_USERS[];

    static const IMS_CHAR ELEMENT_DISPLAY_TEXT[];
    static const IMS_CHAR ELEMENT_ENTRY[];
    static const IMS_CHAR ELEMENT_URI[];
    static const IMS_CHAR ELEMENT_STATUS[];

    static const IMS_CHAR ATTR_VERSION[];
    static const IMS_CHAR ATTR_STATE[];
    static const IMS_CHAR ATTR_ENTITY[];

private:
    ConferenceDescription m_objConferenceDescription;
    HostInfo m_objHostInfo;
    ConferenceState m_objConferenceState;
    IMSList<User*> m_objUsers;

    IMS_UINT32 m_nState;
    IMS_SINT32 m_nVersion;
};

#endif
