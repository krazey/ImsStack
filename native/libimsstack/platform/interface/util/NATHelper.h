/*
    Author
    <table>
    date      author                    description
    --------  --------------            ----------
    20150210  hwangoo.park@             Created
    </table>

    Description

*/

#ifndef _NAT_HELPER_H_
#define _NAT_HELPER_H_

#include "IPAddress.h"

class NATHelper
{
private:
    NATHelper();
    ~NATHelper();

    NATHelper(IN const NATHelper& objRHS);
    NATHelper& operator=(IN const NATHelper& objRHS);

public:
    void Clear(IN IMS_SINT32 nSlotId);
    // Argument: device's public IP address
    IPAddress GetPrivateAddress(IN IMS_SINT32 nSlotId, IN const IPAddress& objPublicIP) const;
    // Argument: device's IP address
    IPAddress GetPublicAddress(IN IMS_SINT32 nSlotId, IN const IPAddress& objPrivateIP) const;
    IMS_BOOL IsBehindNAT(
            IN IMS_SINT32 nSlotId, IN const IPAddress& objPrivateIP = IPAddress::NONE) const;
    void RemovePublicAddress(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nId);
    // Argument: device's IP address
    void RemovePublicAddress(IN IMS_SINT32 nSlotId, IN const IPAddress& objPrivateIP);
    void SetPublicAddress(IN IMS_SINT32 nSlotId, IN IMS_SINT32 nId,
            IN const IPAddress& objPrivateIP, IN const IPAddress& objPublicIP);

    static NATHelper* GetInstance();
    static IMS_BOOL IsNATResolverRequired();

private:
    void RemoveIPBinding(
            IN IMS_SINT32 nSlotId, IN IMS_SINT32 nId, IN const IPAddress& objPrivateIP);

private:
    class IPBinding
    {
    public:
        inline IPBinding() :
                nId(0),
                objIP(IPAddress::NONE),
                objPublicIP(IPAddress::NONE)
        {
        }

        inline IPBinding(
                IN IMS_SINT32 nId_, IN const IPAddress& objIP_, IN const IPAddress& objPublicIP_) :
                nId(nId_),
                objIP(objIP_),
                objPublicIP(objPublicIP_)
        {
        }

        inline IPBinding(IN const IPBinding& objRHS) :
                nId(objRHS.nId),
                objIP(objRHS.objIP),
                objPublicIP(objRHS.objPublicIP)
        {
        }

        inline ~IPBinding() {}

    public:
        inline IPBinding& operator=(IN const IPBinding& objRHS)
        {
            if (this != &objRHS)
            {
                nId = objRHS.nId;
                objIP = objRHS.objIP;
                objPublicIP = objRHS.objPublicIP;
            }

            return (*this);
        }

    public:
        inline IMS_SINT32 GetId() const { return nId; }
        inline const IPAddress& GetPrivateIP() const { return objIP; }
        inline const IPAddress& GetPublicIP() const { return objPublicIP; }
        inline void SetPublicIP(IN const IPAddress& objIP) { objPublicIP = objIP; }

    private:
        IMS_SINT32 nId;
        IPAddress objIP;
        IPAddress objPublicIP;
    };

    IMSList<IPBinding>* GetIPBindings(IN IMS_SINT32 nSlotId) const;

private:
    IMSList<IPBinding>** ppBindings;
};

#endif  // _NAT_HELPER_H_
