#ifndef _SIP_RT_CONFIG_H_
#define _SIP_RT_CONFIG_H_

#include "IPAddress.h"
#include "SipAddress.h"

/**
 * @brief This class defines the classes for the SIP runtime configurations.
 *
 * @see ISIPRTConfigHelper
 */
class SIPRTConfig
{
private:
    SIPRTConfig();

public:
    /// Configuration items which are configured on the runtime
    enum
    {
        CONFIG_I_BASE = 0,

        /// Parameter : Log class
        CONFIG_I_LOG_MASK,

        /// Parameter : SocketOption class
        CONFIG_I_REUSEADDR,
        CONFIG_I_LINGER,
        CONFIG_I_SHUTDOWN,
        CONFIG_I_KEEPALIVE,
        // TCP_OPTIONS {
        /// Default: 9 probes / 7200s / 75s\n
        /// Number of unacknowledged probes (count)
        CONFIG_I_TCP_KEEP_COUNT,
        /// Interval between the last data packet sent & the first keepalive probe (seconds)
        CONFIG_I_TCP_KEEP_IDLE,
        /// Interval between subsequential keepalive probes (seconds)
        CONFIG_I_TCP_KEEP_INTERVAL,
        // TCP_OPTIONS }

        /// Parameter : IPQoS class
        CONFIG_I_IP_QOS,

        /// Parameter : Header class
        CONFIG_I_SIP_HEADER,

        /// Parameter : IPSecSA class
        CONFIG_I_IPSEC_SA,

        /// Parameter : TCPPortRange class
        CONFIG_I_TCP_PORT_RANGE,

        /// Parameter : RegContactAddress class
        CONFIG_I_REG_CONTACT_ADDRESS,

        CONFIG_I_MAX
    };

    /// Feature configuration
    enum
    {
        FEATURE_NONE = 0x00000000,
        /// When outgoing SIP packet needs to be blocked
        FEATURE_SIP_TX_PACKET_BLOCKED = 0x00000001
    };

public:
    //// Mandatory (M), Optional (O)

    /// Base class for SIP runtime configuration
    class Base
    {
    public:
        inline Base()
        { }
        inline virtual ~Base()
        { }

    public:
        inline virtual IMS_BOOL Equals(IN CONST Base & /* objOther */) const
        { return IMS_TRUE; }
    };

    // LOG_EXCLUDING_SERVER_INFO
    /// LogMask class
    class LogMask
        : public Base
    {
    public:
        LogMask();
        virtual ~LogMask();

    public:
        enum
        {
            I_NONE = 0x0000,
            /// When SIP message should be hidden in the log
            I_MESSAGE_HIDDEN = 0x0001,
            /// When SIP routing related information should be hidden in the log
            I_ROUTING_INFO_HIDDEN = 0x0002,
        };

        /// M (SET/REMOVE)\n
        ///    CONFIG_I_LOG_MASK - I_xxx
        IMS_SINT32 nValue;
    };

    /// SocketOption class
    class SocketOption
        : public Base
    {
    public:
        SocketOption();
        SocketOption(IN CONST SocketOption &objRHS);
        virtual ~SocketOption();

    public:
        SocketOption& operator=(IN CONST SocketOption &objRHS);

    public:
        /**
         * @brief Checks if both SocketOption are the same or not.
         *
         * @return If both are the same, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
         */
        virtual IMS_BOOL Equals(IN CONST Base &objOther) const;
        /**
         * @brief Checks if this option is not dedicated to a specific IP or/and port.
         *
         * @return If it's not dedicated to a specific IP or/and port, returns IMS_TRUE.
         *         Otherwise, returns IMS_FALSE.
         */
        IMS_BOOL IsGlobalOption() const;

    public:
        /// M (SET/REMOVE)\n
        ///    CONFIG_I_REUSEADDR - 1 / 0\n
        ///    CONFIG_I_LINGER - integer ( >= 0 ) : seconds\n
        ///    CONFIG_I_SHUTDOWN - 0 (read) / 1 (write) / 2 (read/write) / > 2 (no shutdown)\n
        ///    CONFIG_I_KEEPALIVE - 1 / 0\n
        ///    CONFIG_I_TCP_KEEP_COUNT - N\n
        ///    CONFIG_I_TCP_KEEP_IDLE - N seconds\n
        ///    CONFIG_I_TCP_KEEP_INTERVAL - N seconds
        IMS_SINT32 nValue;
        /// O (SET/REMOVE) : Local IP address which the option will be set.
        IPAddress objIP;
        /// O (SET/REMOVE) : If the port number is not used, fill this value to zero (0).
        IMS_SINT32 nPort;
    };

    /// IPQoS class for IP-level QoS configuration
    class IPQoS
        : public Base
    {
    public:
        IPQoS();
        IPQoS(IN CONST IPQoS &objRHS);
        virtual ~IPQoS();

    public:
        IPQoS& operator=(IN CONST IPQoS &objRHS);

    public:
        /**
         * @brief Checks if both IPQoS are the same or not.
         *
         * @return If both are the same, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
         */
        virtual IMS_BOOL Equals(IN CONST Base &objOther) const;

    public:
        /// M (SET) : 1 byte value (IPv4)
        IMS_SINT32 nValue;
        /// M (SET/REMOVE) : Local IP address which the option will be set.
        IPAddress objIP;
        /// O (SET/REMOVE) : If the port number is not used, fill this value to zero (0).
        IMS_SINT32 nPort;
    };

    /// Header class for an additional SIP header control
    class Header
        : public Base
    {
    public:
        Header();
        Header(IN CONST Header &objRHS);
        virtual ~Header();

    public:
        Header& operator=(IN CONST Header &objRHS);

    public:
        /**
         * @brief Checks if both Header are the same or not.
         *
         * @return If both are the same, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
         */
        virtual IMS_BOOL Equals(IN CONST Base &objOther) const;

    public:
        /// M (SET/REMOVE) : header name
        AString strName;
        /// O (SET) : Extra parameter information to form SIP header\n
        /// If multiple information needs to be provisioned,
        /// each parameter will be split by semi-colon(;).
        AString strParameter;
    };

    /// IPSec SA (security association)
    class IPSecSA
        : public Base
    {
    public:
        IPSecSA();
        IPSecSA(IN CONST IPSecSA &objRHS);
        virtual ~IPSecSA();

    public:
        IPSecSA& operator=(IN CONST IPSecSA &objRHS);

    public:
        /**
         * @brief Checks if both IPSecSA are the same or not.
         *
         * @return If both are the same, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
         */
        virtual IMS_BOOL Equals(IN CONST Base &objOther) const;
        /**
         * @brief Gets the port-c for P-CSCF.
         *
         * @return The port number.
         */
        inline IMS_SINT32 GetPortPC() const
        { return nPortPC; }
        /**
         * @brief Gets the port-s for P-CSCF.
         *
         * @return The port number.
         */
        inline IMS_SINT32 GetPortPS() const
        { return nPortPS; }
        /**
         * @brief Gets the IP address for P-CSCF.
         *
         * @return The IP address.
         */
        inline const IPAddress& GetIPP() const
        { return objIPP; }
        /**
         * @brief Gets the port-c for UE.
         *
         * @return The port number.
         */
        inline IMS_SINT32 GetPortUC() const
        { return nPortUC; }
        /**
         * @brief Gets the port-s for UE.
         *
         * @return The port number.
         */
        inline IMS_SINT32 GetPortUS() const
        { return nPortUS; }
        /**
         * @brief Gets the IP address for UE.
         *
         * @return The IP address.
         */
        inline const IPAddress& GetIPU() const
        { return objIPU; }
        /**
         * @brief Checks if it's empty IPSec SA or not.
         *
         * @return If all the port numbers are zero, returns IMS_TRUE.
         *         Otherwise, returns IMS_FALSE.
         */
        IMS_BOOL IsEmpty() const;

    public:
        /// M (SET) : port_pc
        IMS_SINT32 nPortPC;
        /// M (SET) : port_ps
        IMS_SINT32 nPortPS;
        /// M (SET) : ip_p
        IPAddress objIPP;

        /// M (SET) : port_uc
        IMS_SINT32 nPortUC;
        /// M (SET) : port_us
        IMS_SINT32 nPortUS;
        /// M (SET) : ip_u
        IPAddress objIPU;
    };

    /// The configuration for port range of TCP client connection
    class TCPPortRange
        : public Base
    {
    public:
        TCPPortRange();
        TCPPortRange(IN CONST TCPPortRange &objRHS);
        virtual ~TCPPortRange();

    public:
        TCPPortRange& operator=(IN CONST TCPPortRange &objRHS);

    public:
        /**
         * @brief Checks if both TCPPortRange are the same or not.
         *
         * @return If both are the same, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
         */
        virtual IMS_BOOL Equals(IN CONST Base &objOther) const;
        /**
         * @brief Gets the starting port number of the provisioned port range.
         *
         * @return The starting port number.
         */
        inline IMS_SINT32 GetPortStart() const
        { return nPortStart; }
        /**
         * @brief Gets the ending port number of the provisioned port range.
         *
         * @return The endng port number.
         */
        inline IMS_SINT32 GetPortEnd() const
        { return nPortEnd; }

    public:
        /// M (SET) : start of port range (not-inclusive)
        IMS_SINT32 nPortStart;
        /// M (SET) : end of port range (not-inclusive)
        IMS_SINT32 nPortEnd;
    };

    /// Class for the verification of Contact address in IMS registration
    class RegContactAddress
        : public Base
    {
    public:
        RegContactAddress();
        RegContactAddress(IN CONST RegContactAddress &objRHS);
        virtual ~RegContactAddress();

    public:
        RegContactAddress& operator=(IN CONST RegContactAddress &objRHS);

    public:
        /**
         * @brief Checks if both RegContactAddress are the same or not.
         *
         * @return If both are the same, returns IMS_TRUE. Otherwise, returns IMS_FALSE.
         */
        virtual IMS_BOOL Equals(IN CONST Base &objOther) const;
        /**
         * @brief Gets the call-id string which is used for IMS registration.
         *
         * @return The call-id string.
         */
        inline const AString& GetCallId() const
        { return strCallId; }
        /**
         * @brief Gets the Contact address which is used for IMS registration.
         *
         * @return The Contact address.
         */
        inline const SIPAddress& GetUri() const
        { return objUri; }

    public:
        /// M (SET/REMOVE) : Call-ID to identify the current IMS registration
        AString strCallId;
        /// M (SET) : Contact address for the current IMS registration
        SIPAddress objUri;
    };
};

#endif // _SIP_RT_CONFIG_H_
