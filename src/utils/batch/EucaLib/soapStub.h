/**
 * @file soapStub.h
 * @brief  Auto-generated file by using gSOAP that provides aa SOAP client
 * @author Adrian Muresan (adrian.muresan@ens-lyon.fr)
 * @section Licence
 *   |LICENCE|
 */


#ifndef soapStub_H
#define soapStub_H
#define SOAP_NAMESPACE_OF_ec2 "http://ec2.amazonaws.com/doc/2008-12-01/"
#include "stdsoap2.h"
#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************\
*                                                                            *
* Enumerations                                                               *
*                                                                            *
\******************************************************************************/


#ifndef SOAP_TYPE_wsu__tTimestampFault
#define SOAP_TYPE_wsu__tTimestampFault (7)
/* wsu:tTimestampFault */
enum wsu__tTimestampFault {wsu__MessageExpired = 0};
#endif
typedef enum wsu__tTimestampFault wsu__tTimestampFault;

#ifndef SOAP_TYPE_wsse__FaultcodeEnum
#define SOAP_TYPE_wsse__FaultcodeEnum (9)
/* wsse:FaultcodeEnum */
enum wsse__FaultcodeEnum {wsse__UnsupportedSecurityToken = 0,
                          wsse__UnsupportedAlgorithm = 1,
                          wsse__InvalidSecurity = 2,
                          wsse__InvalidSecurityToken = 3,
                          wsse__FailedAuthentication = 4,
                          wsse__FailedCheck = 5,
                          wsse__SecurityTokenUnavailable = 6};
#endif
typedef enum wsse__FaultcodeEnum wsse__FaultcodeEnum;

#ifndef SOAP_TYPE_xsd__boolean_
#define SOAP_TYPE_xsd__boolean_ (62)
/* xsd:boolean */
enum xsd__boolean_ {_false = 0, _true = 1};
#endif

/******************************************************************************\
*                                                                            *
* Types with Custom Serializers                                              *
*                                                                            *
\******************************************************************************/


/******************************************************************************\
*                                                                            *
* Classes and Structs                                                        *
*                                                                            *
\******************************************************************************/


#if 0 /* volatile type: do not redeclare here */

#endif

#ifndef SOAP_TYPE__wsu__Timestamp
#define SOAP_TYPE__wsu__Timestamp (8)
/* wsu:Timestamp */
struct _wsu__Timestamp{
  char *wsu__Id;    /* optional attribute of type xsd:string */
  char *Created;    /* optional element of type xsd:string */
  char *Expires;    /* optional element of type xsd:string */
};
#endif
typedef struct _wsu__Timestamp _wsu__Timestamp;

#ifndef SOAP_TYPE__wsse__UsernameToken
#define SOAP_TYPE__wsse__UsernameToken (10)
/* wsse:UsernameToken */
struct _wsse__UsernameToken{
  char *Username;   /* optional element of type xsd:string */
  struct _wsse__Password *Password;         /* optional element of type wsse:Password */
  char *Nonce;      /* optional element of type xsd:string */
  char *wsu__Created;       /* optional element of type xsd:string */
  char *wsu__Id;    /* optional attribute of type xsd:string */
};
#endif // ifndef SOAP_TYPE__wsse__UsernameToken
typedef struct _wsse__UsernameToken _wsse__UsernameToken;

#ifndef SOAP_TYPE__wsse__BinarySecurityToken
#define SOAP_TYPE__wsse__BinarySecurityToken (13)
/* Primitive wsse:BinarySecurityToken schema type: */
struct _wsse__BinarySecurityToken{
  char *__item;
  char *wsu__Id;    /* optional attribute of type xsd:string */
  char *ValueType;          /* optional attribute of type xsd:string */
  char *EncodingType;       /* optional attribute of type xsd:string */
};
#endif
typedef struct _wsse__BinarySecurityToken _wsse__BinarySecurityToken;

#ifndef SOAP_TYPE__wsse__Reference
#define SOAP_TYPE__wsse__Reference (14)
/* wsse:Reference */
struct _wsse__Reference{
  char *URI;        /* optional attribute of type xsd:string */
  char *ValueType;          /* optional attribute of type xsd:string */
};
#endif
typedef struct _wsse__Reference _wsse__Reference;

#ifndef SOAP_TYPE__wsse__Embedded
#define SOAP_TYPE__wsse__Embedded (15)
/* wsse:Embedded */
struct _wsse__Embedded{
  char *wsu__Id;    /* optional attribute of type xsd:string */
  char *ValueType;          /* optional attribute of type xsd:string */
};
#endif
typedef struct _wsse__Embedded _wsse__Embedded;

#ifndef SOAP_TYPE__wsse__KeyIdentifier
#define SOAP_TYPE__wsse__KeyIdentifier (16)
/* Primitive wsse:KeyIdentifier schema type: */
struct _wsse__KeyIdentifier{
  char *__item;
  char *wsu__Id;    /* optional attribute of type xsd:string */
  char *ValueType;          /* optional attribute of type xsd:string */
  char *EncodingType;       /* optional attribute of type xsd:string */
};
#endif
typedef struct _wsse__KeyIdentifier _wsse__KeyIdentifier;

#ifndef SOAP_TYPE__wsse__SecurityTokenReference
#define SOAP_TYPE__wsse__SecurityTokenReference (17)
/* wsse:SecurityTokenReference */
struct _wsse__SecurityTokenReference{
  struct _wsse__Reference *Reference;       /* optional element of type wsse:Reference */
  struct _wsse__KeyIdentifier *KeyIdentifier;       /* optional element of type wsse:KeyIdentifier */
  struct _wsse__Embedded *Embedded;         /* optional element of type wsse:Embedded */
  char *wsu__Id;    /* optional attribute of type xsd:string */
  char *Usage;      /* optional attribute of type xsd:string */
};
#endif // ifndef SOAP_TYPE__wsse__SecurityTokenReference
typedef struct _wsse__SecurityTokenReference _wsse__SecurityTokenReference;

#ifndef SOAP_TYPE_ds__SignatureType
#define SOAP_TYPE_ds__SignatureType (21)
/* ds:SignatureType */
struct ds__SignatureType{
  struct ds__SignedInfoType *SignedInfo;    /* optional element of type ds:SignedInfoType */
  char *SignatureValue;     /* optional element of type xsd:string */
  struct ds__KeyInfoType *KeyInfo;          /* optional element of type ds:KeyInfoType */
  char *Id;         /* optional attribute of type xsd:string */
};
#endif
typedef struct ds__SignatureType ds__SignatureType;

#ifndef SOAP_TYPE__c14n__InclusiveNamespaces
#define SOAP_TYPE__c14n__InclusiveNamespaces (30)
/* c14n:InclusiveNamespaces */
struct _c14n__InclusiveNamespaces{
  char *PrefixList;         /* optional attribute of type xsd:string */
};
#endif
typedef struct _c14n__InclusiveNamespaces _c14n__InclusiveNamespaces;

#ifndef SOAP_TYPE_ds__TransformType
#define SOAP_TYPE_ds__TransformType (31)
/* ds:TransformType */
struct ds__TransformType{
  struct _c14n__InclusiveNamespaces *c14n__InclusiveNamespaces;     /* optional element of type c14n:InclusiveNamespaces */
  char *__any;
  char *Algorithm;          /* optional attribute of type xsd:string */
};
#endif
typedef struct ds__TransformType ds__TransformType;

#ifndef SOAP_TYPE_ds__KeyInfoType
#define SOAP_TYPE_ds__KeyInfoType (24)
/* ds:KeyInfoType */
struct ds__KeyInfoType{
  char *KeyName;    /* optional element of type xsd:string */
  struct ds__KeyValueType *KeyValue;        /* optional element of type ds:KeyValueType */
  struct ds__RetrievalMethodType *RetrievalMethod;          /* optional element of type ds:RetrievalMethodType */
  struct ds__X509DataType *X509Data;        /* optional element of type ds:X509DataType */
  struct _wsse__SecurityTokenReference *wsse__SecurityTokenReference;       /* optional element of type wsse:SecurityTokenReference */
  char *Id;         /* optional attribute of type xsd:string */
};
#endif // ifndef SOAP_TYPE_ds__KeyInfoType
typedef struct ds__KeyInfoType ds__KeyInfoType;

#ifndef SOAP_TYPE_ds__SignedInfoType
#define SOAP_TYPE_ds__SignedInfoType (22)
/* ds:SignedInfoType */
struct ds__SignedInfoType{
  struct ds__CanonicalizationMethodType *CanonicalizationMethod;    /* required element of type ds:CanonicalizationMethodType */
  struct ds__SignatureMethodType *SignatureMethod;          /* required element of type ds:SignatureMethodType */
  int __sizeReference;      /* sequence of elements <Reference> */
  struct ds__ReferenceType **Reference;     /* required element of type ds:ReferenceType */
  char *Id;         /* optional attribute of type xsd:string */
};
#endif // ifndef SOAP_TYPE_ds__SignedInfoType
typedef struct ds__SignedInfoType ds__SignedInfoType;

#ifndef SOAP_TYPE_ds__CanonicalizationMethodType
#define SOAP_TYPE_ds__CanonicalizationMethodType (26)
/* ds:CanonicalizationMethodType */
struct ds__CanonicalizationMethodType{
  char *Algorithm;          /* required attribute of type xsd:string */
  struct _c14n__InclusiveNamespaces *c14n__InclusiveNamespaces;     /* optional element of type c14n:InclusiveNamespaces */
};
#endif
typedef struct ds__CanonicalizationMethodType ds__CanonicalizationMethodType;

#ifndef SOAP_TYPE_ds__SignatureMethodType
#define SOAP_TYPE_ds__SignatureMethodType (27)
/* ds:SignatureMethodType */
struct ds__SignatureMethodType
{
  int *HMACOutputLength;    /* optional element of type xsd:int */
  char *Algorithm;          /* required attribute of type xsd:string */
};
#endif
typedef struct ds__SignatureMethodType ds__SignatureMethodType;

#ifndef SOAP_TYPE_ds__ReferenceType
#define SOAP_TYPE_ds__ReferenceType (28)
/* ds:ReferenceType */
struct ds__ReferenceType
{
  struct ds__TransformsType *Transforms;    /* optional element of type ds:TransformsType */
  struct ds__DigestMethodType *DigestMethod;        /* required element of type ds:DigestMethodType */
  char *DigestValue;        /* required element of type xsd:string */
  char *Id;         /* optional attribute of type xsd:string */
  char *URI;        /* optional attribute of type xsd:string */
  char *Type;       /* optional attribute of type xsd:string */
};
#endif // ifndef SOAP_TYPE_ds__ReferenceType
typedef struct ds__ReferenceType ds__ReferenceType;

#ifndef SOAP_TYPE_ds__TransformsType
#define SOAP_TYPE_ds__TransformsType (29)
/* ds:TransformsType */
struct ds__TransformsType
{
  int __sizeTransform;      /* sequence of elements <Transform> */
  struct ds__TransformType *Transform;      /* required element of type ds:TransformType */
};
#endif
typedef struct ds__TransformsType ds__TransformsType;

#ifndef SOAP_TYPE_ds__DigestMethodType
#define SOAP_TYPE_ds__DigestMethodType (33)
/* ds:DigestMethodType */
struct ds__DigestMethodType
{
  char *Algorithm;          /* required attribute of type xsd:string */
};
#endif
typedef struct ds__DigestMethodType ds__DigestMethodType;

#ifndef SOAP_TYPE_ds__KeyValueType
#define SOAP_TYPE_ds__KeyValueType (34)
/* ds:KeyValueType */
struct ds__KeyValueType
{
  struct ds__DSAKeyValueType *DSAKeyValue;          /* required element of type ds:DSAKeyValueType */
  struct ds__RSAKeyValueType *RSAKeyValue;          /* required element of type ds:RSAKeyValueType */
};
#endif
typedef struct ds__KeyValueType ds__KeyValueType;

#ifndef SOAP_TYPE_ds__RetrievalMethodType
#define SOAP_TYPE_ds__RetrievalMethodType (36)
/* ds:RetrievalMethodType */
struct ds__RetrievalMethodType
{
  struct ds__TransformsType *Transforms;    /* optional element of type ds:TransformsType */
  char *URI;        /* optional attribute of type xsd:string */
  char *Type;       /* optional attribute of type xsd:string */
};
#endif
typedef struct ds__RetrievalMethodType ds__RetrievalMethodType;

#ifndef SOAP_TYPE_ds__X509DataType
#define SOAP_TYPE_ds__X509DataType (38)
/* ds:X509DataType */
struct ds__X509DataType
{
  struct ds__X509IssuerSerialType *X509IssuerSerial;        /* required element of type ds:X509IssuerSerialType */
  char *X509SKI;    /* required element of type xsd:string */
  char *X509SubjectName;    /* required element of type xsd:string */
  char *X509Certificate;    /* required element of type xsd:string */
  char *X509CRL;    /* required element of type xsd:string */
};
#endif // ifndef SOAP_TYPE_ds__X509DataType
typedef struct ds__X509DataType ds__X509DataType;

#ifndef SOAP_TYPE_ds__X509IssuerSerialType
#define SOAP_TYPE_ds__X509IssuerSerialType (41)
/* ds:X509IssuerSerialType */
struct ds__X509IssuerSerialType
{
  char *X509IssuerName;     /* required element of type xsd:string */
  int X509SerialNumber;     /* required element of type xsd:int */
};
#endif
typedef struct ds__X509IssuerSerialType ds__X509IssuerSerialType;

#ifndef SOAP_TYPE_ds__DSAKeyValueType
#define SOAP_TYPE_ds__DSAKeyValueType (42)
/* ds:DSAKeyValueType */
struct ds__DSAKeyValueType
{
  char *G;          /* optional element of type xsd:string */
  char *Y;          /* required element of type xsd:string */
  char *J;          /* optional element of type xsd:string */
  char *P;          /* required element of type xsd:string */
  char *Q;          /* required element of type xsd:string */
  char *Seed;       /* required element of type xsd:string */
  char *PgenCounter;        /* required element of type xsd:string */
};
#endif // ifndef SOAP_TYPE_ds__DSAKeyValueType
typedef struct ds__DSAKeyValueType ds__DSAKeyValueType;

#ifndef SOAP_TYPE_ds__RSAKeyValueType
#define SOAP_TYPE_ds__RSAKeyValueType (43)
/* ds:RSAKeyValueType */
struct ds__RSAKeyValueType
{
  char *Modulus;    /* required element of type xsd:string */
  char *Exponent;   /* required element of type xsd:string */
};
#endif
typedef struct ds__RSAKeyValueType ds__RSAKeyValueType;

#ifndef SOAP_TYPE__wsse__Security
#define SOAP_TYPE__wsse__Security (55)
/* wsse:Security */
struct _wsse__Security
{
  struct _wsu__Timestamp *wsu__Timestamp;   /* optional element of type wsu:Timestamp */
  struct _wsse__UsernameToken *UsernameToken;       /* optional element of type wsse:UsernameToken */
  struct _wsse__BinarySecurityToken *BinarySecurityToken;   /* optional element of type wsse:BinarySecurityToken */
  struct ds__SignatureType *ds__Signature;          /* optional element of type ds:SignatureType */
  char *SOAP_ENV__actor;    /* optional attribute of type xsd:string */
  char *SOAP_ENV__role;     /* optional attribute of type xsd:string */
};
#endif // ifndef SOAP_TYPE__wsse__Security
typedef struct _wsse__Security _wsse__Security;

#ifndef SOAP_TYPE__wsse__Password
#define SOAP_TYPE__wsse__Password (11)
/* Primitive wsse:Password schema type: */
struct _wsse__Password
{
  char *__item;
  char *Type;       /* optional attribute of type xsd:string */
};
#endif
typedef struct _wsse__Password _wsse__Password;

#ifndef SOAP_TYPE_SOAP_ENV__Header
#define SOAP_TYPE_SOAP_ENV__Header (60)
/* SOAP Header: */
struct SOAP_ENV__Header
{
  struct _wsse__Security *wsse__Security;   /* mustUnderstand */
};
#endif

#ifndef SOAP_TYPE_ec2__RegisterImageType
#define SOAP_TYPE_ec2__RegisterImageType (63)
/* ec2:RegisterImageType */
struct ec2__RegisterImageType
{
  char *imageLocation;      /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__RegisterImageResponseType
#define SOAP_TYPE_ec2__RegisterImageResponseType (64)
/* ec2:RegisterImageResponseType */
struct ec2__RegisterImageResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  char *imageId;    /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DeregisterImageType
#define SOAP_TYPE_ec2__DeregisterImageType (65)
/* ec2:DeregisterImageType */
struct ec2__DeregisterImageType
{
  char *imageId;    /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DeregisterImageResponseType
#define SOAP_TYPE_ec2__DeregisterImageResponseType (66)
/* ec2:DeregisterImageResponseType */
struct ec2__DeregisterImageResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE_ec2__CreateKeyPairType
#define SOAP_TYPE_ec2__CreateKeyPairType (67)
/* ec2:CreateKeyPairType */
struct ec2__CreateKeyPairType
{
  char *keyName;    /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__CreateKeyPairResponseType
#define SOAP_TYPE_ec2__CreateKeyPairResponseType (68)
/* ec2:CreateKeyPairResponseType */
struct ec2__CreateKeyPairResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  char *keyName;    /* required element of type xsd:string */
  char *keyFingerprint;     /* required element of type xsd:string */
  char *keyMaterial;        /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DeleteKeyPairType
#define SOAP_TYPE_ec2__DeleteKeyPairType (69)
/* ec2:DeleteKeyPairType */
struct ec2__DeleteKeyPairType
{
  char *keyName;    /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DeleteKeyPairResponseType
#define SOAP_TYPE_ec2__DeleteKeyPairResponseType (70)
/* ec2:DeleteKeyPairResponseType */
struct ec2__DeleteKeyPairResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeKeyPairsType
#define SOAP_TYPE_ec2__DescribeKeyPairsType (71)
/* ec2:DescribeKeyPairsType */
struct ec2__DescribeKeyPairsType
{
  struct ec2__DescribeKeyPairsInfoType *keySet;     /* required element of type ec2:DescribeKeyPairsInfoType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeKeyPairsInfoType
#define SOAP_TYPE_ec2__DescribeKeyPairsInfoType (72)
/* ec2:DescribeKeyPairsInfoType */
struct ec2__DescribeKeyPairsInfoType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeKeyPairsItemType *item;       /* optional element of type ec2:DescribeKeyPairsItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeKeyPairsItemType
#define SOAP_TYPE_ec2__DescribeKeyPairsItemType (74)
/* ec2:DescribeKeyPairsItemType */
struct ec2__DescribeKeyPairsItemType
{
  char *keyName;    /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeKeyPairsResponseType
#define SOAP_TYPE_ec2__DescribeKeyPairsResponseType (76)
/* ec2:DescribeKeyPairsResponseType */
struct ec2__DescribeKeyPairsResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  struct ec2__DescribeKeyPairsResponseInfoType *keySet;     /* required element of type ec2:DescribeKeyPairsResponseInfoType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeKeyPairsResponseInfoType
#define SOAP_TYPE_ec2__DescribeKeyPairsResponseInfoType (77)
/* ec2:DescribeKeyPairsResponseInfoType */
struct ec2__DescribeKeyPairsResponseInfoType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeKeyPairsResponseItemType *item;       /* optional element of type ec2:DescribeKeyPairsResponseItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeKeyPairsResponseItemType
#define SOAP_TYPE_ec2__DescribeKeyPairsResponseItemType (79)
/* ec2:DescribeKeyPairsResponseItemType */
struct ec2__DescribeKeyPairsResponseItemType
{
  char *keyName;    /* required element of type xsd:string */
  char *keyFingerprint;     /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__RunInstancesType
#define SOAP_TYPE_ec2__RunInstancesType (81)
/* ec2:RunInstancesType */
struct ec2__RunInstancesType
{
  char *imageId;    /* required element of type xsd:string */
  int minCount;     /* required element of type xsd:int */
  int maxCount;     /* required element of type xsd:int */
  char *keyName;    /* optional element of type xsd:string */
  struct ec2__GroupSetType *groupSet;       /* required element of type ec2:GroupSetType */
  char *additionalInfo;     /* optional element of type xsd:string */
  struct ec2__UserDataType *userData;       /* optional element of type ec2:UserDataType */
  char *addressingType;     /* optional element of type xsd:string */
  char *instanceType;       /* required element of type xsd:string */
  struct ec2__PlacementRequestType *placement;      /* optional element of type ec2:PlacementRequestType */
  char *kernelId;   /* optional element of type xsd:string */
  char *ramdiskId;          /* optional element of type xsd:string */
  struct ec2__BlockDeviceMappingType *blockDeviceMapping;   /* optional element of type ec2:BlockDeviceMappingType */
};
#endif // ifndef SOAP_TYPE_ec2__RunInstancesType

#ifndef SOAP_TYPE_ec2__GroupSetType
#define SOAP_TYPE_ec2__GroupSetType (82)
/* ec2:GroupSetType */
struct ec2__GroupSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__GroupItemType *item;          /* optional element of type ec2:GroupItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__GroupItemType
#define SOAP_TYPE_ec2__GroupItemType (90)
/* ec2:GroupItemType */
struct ec2__GroupItemType
{
  char *groupId;    /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__UserDataType
#define SOAP_TYPE_ec2__UserDataType (84)
/* ec2:UserDataType */
struct ec2__UserDataType
{
  char *data;       /* required element of type xsd:string */
  char *version;    /* required attribute of type xsd:string */
  char *encoding;   /* required attribute of type xsd:string */
  char *__mixed;
};
#endif

#ifndef SOAP_TYPE_ec2__BlockDeviceMappingType
#define SOAP_TYPE_ec2__BlockDeviceMappingType (88)
/* ec2:BlockDeviceMappingType */
struct ec2__BlockDeviceMappingType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__BlockDeviceMappingItemType *item;     /* optional element of type ec2:BlockDeviceMappingItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__BlockDeviceMappingItemType
#define SOAP_TYPE_ec2__BlockDeviceMappingItemType (92)
/* ec2:BlockDeviceMappingItemType */
struct ec2__BlockDeviceMappingItemType
{
  char *virtualName;        /* required element of type xsd:string */
  char *deviceName;         /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__PlacementRequestType
#define SOAP_TYPE_ec2__PlacementRequestType (86)
/* ec2:PlacementRequestType */
struct ec2__PlacementRequestType
{
  char *availabilityZone;   /* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__RunInstancesResponseType
#define SOAP_TYPE_ec2__RunInstancesResponseType (94)
/* ec2:RunInstancesResponseType */
struct ec2__RunInstancesResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  char *reservationId;      /* required element of type xsd:string */
  char *ownerId;    /* required element of type xsd:string */
  struct ec2__GroupSetType *groupSet;       /* required element of type ec2:GroupSetType */
  struct ec2__RunningInstancesSetType *instancesSet;        /* required element of type ec2:RunningInstancesSetType */
  char *requesterId;        /* optional element of type xsd:string */
};
#endif // ifndef SOAP_TYPE_ec2__RunInstancesResponseType

#ifndef SOAP_TYPE_ec2__ReservationInfoType
#define SOAP_TYPE_ec2__ReservationInfoType (97)
/* ec2:ReservationInfoType */
struct ec2__ReservationInfoType
{
  char *reservationId;      /* required element of type xsd:string */
  char *ownerId;    /* required element of type xsd:string */
  struct ec2__GroupSetType *groupSet;       /* required element of type ec2:GroupSetType */
  struct ec2__RunningInstancesSetType *instancesSet;        /* required element of type ec2:RunningInstancesSetType */
  char *requesterId;        /* optional element of type xsd:string */
};
#endif // ifndef SOAP_TYPE_ec2__ReservationInfoType

#ifndef SOAP_TYPE_ec2__RunningInstancesSetType
#define SOAP_TYPE_ec2__RunningInstancesSetType (95)
/* ec2:RunningInstancesSetType */
struct ec2__RunningInstancesSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__RunningInstancesItemType *item;       /* required element of type ec2:RunningInstancesItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__RunningInstancesItemType
#define SOAP_TYPE_ec2__RunningInstancesItemType (98)
/* ec2:RunningInstancesItemType */
struct ec2__RunningInstancesItemType
{
  char *instanceId;         /* required element of type xsd:string */
  char *imageId;    /* required element of type xsd:string */
  struct ec2__InstanceStateType *instanceState;     /* required element of type ec2:InstanceStateType */
  char *privateDnsName;     /* required element of type xsd:string */
  char *dnsName;    /* required element of type xsd:string */
  char *reason;     /* optional element of type xsd:string */
  char *keyName;    /* optional element of type xsd:string */
  char *amiLaunchIndex;     /* optional element of type xsd:string */
  struct ec2__ProductCodesSetType *productCodes;    /* optional element of type ec2:ProductCodesSetType */
  char *instanceType;       /* required element of type xsd:string */
  time_t launchTime;        /* required element of type xsd:dateTime */
  struct ec2__PlacementResponseType *placement;     /* optional element of type ec2:PlacementResponseType */
  char *kernelId;   /* optional element of type xsd:string */
  char *ramdiskId;          /* optional element of type xsd:string */
  char *platform;   /* optional element of type xsd:string */
};
#endif // ifndef SOAP_TYPE_ec2__RunningInstancesItemType

#ifndef SOAP_TYPE_ec2__PlacementResponseType
#define SOAP_TYPE_ec2__PlacementResponseType (105)
/* ec2:PlacementResponseType */
struct ec2__PlacementResponseType
{
  char *availabilityZone;   /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__GetConsoleOutputType
#define SOAP_TYPE_ec2__GetConsoleOutputType (107)
/* ec2:GetConsoleOutputType */
struct ec2__GetConsoleOutputType
{
  char *instanceId;         /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__GetConsoleOutputResponseType
#define SOAP_TYPE_ec2__GetConsoleOutputResponseType (108)
/* ec2:GetConsoleOutputResponseType */
struct ec2__GetConsoleOutputResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  char *instanceId;         /* required element of type xsd:string */
  time_t timestamp;         /* required element of type xsd:dateTime */
  char *output;     /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__TerminateInstancesType
#define SOAP_TYPE_ec2__TerminateInstancesType (109)
/* ec2:TerminateInstancesType */
struct ec2__TerminateInstancesType
{
  struct ec2__TerminateInstancesInfoType *instancesSet;     /* required element of type ec2:TerminateInstancesInfoType */
};
#endif

#ifndef SOAP_TYPE_ec2__TerminateInstancesInfoType
#define SOAP_TYPE_ec2__TerminateInstancesInfoType (110)
/* ec2:TerminateInstancesInfoType */
struct ec2__TerminateInstancesInfoType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__TerminateInstancesItemType *item;     /* required element of type ec2:TerminateInstancesItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__TerminateInstancesItemType
#define SOAP_TYPE_ec2__TerminateInstancesItemType (112)
/* ec2:TerminateInstancesItemType */
struct ec2__TerminateInstancesItemType
{
  char *instanceId;         /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__TerminateInstancesResponseType
#define SOAP_TYPE_ec2__TerminateInstancesResponseType (114)
/* ec2:TerminateInstancesResponseType */
struct ec2__TerminateInstancesResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  struct ec2__TerminateInstancesResponseInfoType *instancesSet;     /* required element of type ec2:TerminateInstancesResponseInfoType */
};
#endif

#ifndef SOAP_TYPE_ec2__TerminateInstancesResponseInfoType
#define SOAP_TYPE_ec2__TerminateInstancesResponseInfoType (115)
/* ec2:TerminateInstancesResponseInfoType */
struct ec2__TerminateInstancesResponseInfoType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__TerminateInstancesResponseItemType *item;     /* optional element of type ec2:TerminateInstancesResponseItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__TerminateInstancesResponseItemType
#define SOAP_TYPE_ec2__TerminateInstancesResponseItemType (117)
/* ec2:TerminateInstancesResponseItemType */
struct ec2__TerminateInstancesResponseItemType
{
  char *instanceId;         /* required element of type xsd:string */
  struct ec2__InstanceStateType *shutdownState;     /* required element of type ec2:InstanceStateType */
  struct ec2__InstanceStateType *previousState;     /* required element of type ec2:InstanceStateType */
};
#endif

#ifndef SOAP_TYPE_ec2__RebootInstancesType
#define SOAP_TYPE_ec2__RebootInstancesType (119)
/* ec2:RebootInstancesType */
struct ec2__RebootInstancesType
{
  struct ec2__RebootInstancesInfoType *instancesSet;        /* required element of type ec2:RebootInstancesInfoType */
};
#endif

#ifndef SOAP_TYPE_ec2__RebootInstancesInfoType
#define SOAP_TYPE_ec2__RebootInstancesInfoType (120)
/* ec2:RebootInstancesInfoType */
struct ec2__RebootInstancesInfoType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__RebootInstancesItemType *item;        /* required element of type ec2:RebootInstancesItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__RebootInstancesItemType
#define SOAP_TYPE_ec2__RebootInstancesItemType (122)
/* ec2:RebootInstancesItemType */
struct ec2__RebootInstancesItemType
{
  char *instanceId;         /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__RebootInstancesResponseType
#define SOAP_TYPE_ec2__RebootInstancesResponseType (124)
/* ec2:RebootInstancesResponseType */
struct ec2__RebootInstancesResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeInstancesType
#define SOAP_TYPE_ec2__DescribeInstancesType (125)
/* ec2:DescribeInstancesType */
struct ec2__DescribeInstancesType
{
  struct ec2__DescribeInstancesInfoType *instancesSet;      /* required element of type ec2:DescribeInstancesInfoType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeInstancesInfoType
#define SOAP_TYPE_ec2__DescribeInstancesInfoType (126)
/* ec2:DescribeInstancesInfoType */
struct ec2__DescribeInstancesInfoType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeInstancesItemType *item;      /* optional element of type ec2:DescribeInstancesItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeInstancesItemType
#define SOAP_TYPE_ec2__DescribeInstancesItemType (128)
/* ec2:DescribeInstancesItemType */
struct ec2__DescribeInstancesItemType
{
  char *instanceId;         /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeInstancesResponseType
#define SOAP_TYPE_ec2__DescribeInstancesResponseType (130)
/* ec2:DescribeInstancesResponseType */
struct ec2__DescribeInstancesResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  struct ec2__ReservationSetType *reservationSet;   /* required element of type ec2:ReservationSetType */
};
#endif

#ifndef SOAP_TYPE_ec2__ReservationSetType
#define SOAP_TYPE_ec2__ReservationSetType (131)
/* ec2:ReservationSetType */
struct ec2__ReservationSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__ReservationInfoType *item;    /* optional element of type ec2:ReservationInfoType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeImagesType
#define SOAP_TYPE_ec2__DescribeImagesType (134)
/* ec2:DescribeImagesType */
struct ec2__DescribeImagesType
{
  struct ec2__DescribeImagesExecutableBySetType *executableBySet;   /* optional element of type ec2:DescribeImagesExecutableBySetType */
  struct ec2__DescribeImagesInfoType *imagesSet;    /* required element of type ec2:DescribeImagesInfoType */
  struct ec2__DescribeImagesOwnersType *ownersSet;          /* optional element of type ec2:DescribeImagesOwnersType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeImagesInfoType
#define SOAP_TYPE_ec2__DescribeImagesInfoType (137)
/* ec2:DescribeImagesInfoType */
struct ec2__DescribeImagesInfoType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeImagesItemType *item;         /* optional element of type ec2:DescribeImagesItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeImagesItemType
#define SOAP_TYPE_ec2__DescribeImagesItemType (141)
/* ec2:DescribeImagesItemType */
struct ec2__DescribeImagesItemType
{
  char *imageId;    /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeImagesOwnersType
#define SOAP_TYPE_ec2__DescribeImagesOwnersType (139)
/* ec2:DescribeImagesOwnersType */
struct ec2__DescribeImagesOwnersType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeImagesOwnerType *item;        /* optional element of type ec2:DescribeImagesOwnerType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeImagesOwnerType
#define SOAP_TYPE_ec2__DescribeImagesOwnerType (143)
/* ec2:DescribeImagesOwnerType */
struct ec2__DescribeImagesOwnerType
{
  char *owner;      /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeImagesExecutableBySetType
#define SOAP_TYPE_ec2__DescribeImagesExecutableBySetType (135)
/* ec2:DescribeImagesExecutableBySetType */
struct ec2__DescribeImagesExecutableBySetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeImagesExecutableByType *item;         /* optional element of type ec2:DescribeImagesExecutableByType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeImagesExecutableByType
#define SOAP_TYPE_ec2__DescribeImagesExecutableByType (145)
/* ec2:DescribeImagesExecutableByType */
struct ec2__DescribeImagesExecutableByType
{
  char *user;       /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeImagesResponseType
#define SOAP_TYPE_ec2__DescribeImagesResponseType (147)
/* ec2:DescribeImagesResponseType */
struct ec2__DescribeImagesResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  struct ec2__DescribeImagesResponseInfoType *imagesSet;    /* required element of type ec2:DescribeImagesResponseInfoType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeImagesResponseInfoType
#define SOAP_TYPE_ec2__DescribeImagesResponseInfoType (148)
/* ec2:DescribeImagesResponseInfoType */
struct ec2__DescribeImagesResponseInfoType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeImagesResponseItemType *item;         /* optional element of type ec2:DescribeImagesResponseItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeImagesResponseItemType
#define SOAP_TYPE_ec2__DescribeImagesResponseItemType (150)
/* ec2:DescribeImagesResponseItemType */
struct ec2__DescribeImagesResponseItemType
{
  char *imageId;    /* required element of type xsd:string */
  char *imageLocation;      /* required element of type xsd:string */
  char *imageState;         /* required element of type xsd:string */
  char *imageOwnerId;       /* required element of type xsd:string */
  enum xsd__boolean_ isPublic;      /* required element of type xsd:boolean */
  struct ec2__ProductCodesSetType *productCodes;    /* optional element of type ec2:ProductCodesSetType */
  char *architecture;       /* optional element of type xsd:string */
  char *imageType;          /* optional element of type xsd:string */
  char *kernelId;   /* optional element of type xsd:string */
  char *ramdiskId;          /* optional element of type xsd:string */
  char *platform;   /* optional element of type xsd:string */
};
#endif // ifndef SOAP_TYPE_ec2__DescribeImagesResponseItemType

#ifndef SOAP_TYPE_ec2__CreateSecurityGroupType
#define SOAP_TYPE_ec2__CreateSecurityGroupType (152)
/* ec2:CreateSecurityGroupType */
struct ec2__CreateSecurityGroupType
{
  char *groupName;          /* required element of type xsd:string */
  char *groupDescription;   /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__CreateSecurityGroupResponseType
#define SOAP_TYPE_ec2__CreateSecurityGroupResponseType (153)
/* ec2:CreateSecurityGroupResponseType */
struct ec2__CreateSecurityGroupResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE_ec2__DeleteSecurityGroupType
#define SOAP_TYPE_ec2__DeleteSecurityGroupType (154)
/* ec2:DeleteSecurityGroupType */
struct ec2__DeleteSecurityGroupType
{
  char *groupName;          /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DeleteSecurityGroupResponseType
#define SOAP_TYPE_ec2__DeleteSecurityGroupResponseType (155)
/* ec2:DeleteSecurityGroupResponseType */
struct ec2__DeleteSecurityGroupResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeSecurityGroupsType
#define SOAP_TYPE_ec2__DescribeSecurityGroupsType (156)
/* ec2:DescribeSecurityGroupsType */
struct ec2__DescribeSecurityGroupsType
{
  struct ec2__DescribeSecurityGroupsSetType *securityGroupSet;      /* required element of type ec2:DescribeSecurityGroupsSetType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeSecurityGroupsSetType
#define SOAP_TYPE_ec2__DescribeSecurityGroupsSetType (157)
/* ec2:DescribeSecurityGroupsSetType */
struct ec2__DescribeSecurityGroupsSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeSecurityGroupsSetItemType *item;      /* optional element of type ec2:DescribeSecurityGroupsSetItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeSecurityGroupsSetItemType
#define SOAP_TYPE_ec2__DescribeSecurityGroupsSetItemType (159)
/* ec2:DescribeSecurityGroupsSetItemType */
struct ec2__DescribeSecurityGroupsSetItemType
{
  char *groupName;          /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeSecurityGroupsResponseType
#define SOAP_TYPE_ec2__DescribeSecurityGroupsResponseType (161)
/* ec2:DescribeSecurityGroupsResponseType */
struct ec2__DescribeSecurityGroupsResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  struct ec2__SecurityGroupSetType *securityGroupInfo;      /* required element of type ec2:SecurityGroupSetType */
};
#endif

#ifndef SOAP_TYPE_ec2__IpPermissionSetType
#define SOAP_TYPE_ec2__IpPermissionSetType (164)
/* ec2:IpPermissionSetType */
struct ec2__IpPermissionSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__IpPermissionType *item;       /* optional element of type ec2:IpPermissionType */
};
#endif

#ifndef SOAP_TYPE_ec2__IpPermissionType
#define SOAP_TYPE_ec2__IpPermissionType (165)
/* ec2:IpPermissionType */
struct ec2__IpPermissionType
{
  char *ipProtocol;         /* required element of type xsd:string */
  int fromPort;     /* required element of type xsd:int */
  int toPort;       /* required element of type xsd:int */
  struct ec2__UserIdGroupPairSetType *groups;       /* required element of type ec2:UserIdGroupPairSetType */
  struct ec2__IpRangeSetType *ipRanges;     /* required element of type ec2:IpRangeSetType */
};
#endif // ifndef SOAP_TYPE_ec2__IpPermissionType

#ifndef SOAP_TYPE_ec2__IpRangeSetType
#define SOAP_TYPE_ec2__IpRangeSetType (169)
/* ec2:IpRangeSetType */
struct ec2__IpRangeSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__IpRangeItemType *item;        /* optional element of type ec2:IpRangeItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__IpRangeItemType
#define SOAP_TYPE_ec2__IpRangeItemType (171)
/* ec2:IpRangeItemType */
struct ec2__IpRangeItemType
{
  char *cidrIp;     /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__UserIdGroupPairSetType
#define SOAP_TYPE_ec2__UserIdGroupPairSetType (167)
/* ec2:UserIdGroupPairSetType */
struct ec2__UserIdGroupPairSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__UserIdGroupPairType *item;    /* optional element of type ec2:UserIdGroupPairType */
};
#endif

#ifndef SOAP_TYPE_ec2__UserIdGroupPairType
#define SOAP_TYPE_ec2__UserIdGroupPairType (173)
/* ec2:UserIdGroupPairType */
struct ec2__UserIdGroupPairType
{
  char *userId;     /* required element of type xsd:string */
  char *groupName;          /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__SecurityGroupSetType
#define SOAP_TYPE_ec2__SecurityGroupSetType (162)
/* ec2:SecurityGroupSetType */
struct ec2__SecurityGroupSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__SecurityGroupItemType *item;          /* optional element of type ec2:SecurityGroupItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__SecurityGroupItemType
#define SOAP_TYPE_ec2__SecurityGroupItemType (175)
/* ec2:SecurityGroupItemType */
struct ec2__SecurityGroupItemType
{
  char *ownerId;    /* required element of type xsd:string */
  char *groupName;          /* required element of type xsd:string */
  char *groupDescription;   /* required element of type xsd:string */
  struct ec2__IpPermissionSetType *ipPermissions;   /* required element of type ec2:IpPermissionSetType */
};
#endif

#ifndef SOAP_TYPE_ec2__AuthorizeSecurityGroupIngressType
#define SOAP_TYPE_ec2__AuthorizeSecurityGroupIngressType (178)
/* ec2:AuthorizeSecurityGroupIngressType */
struct ec2__AuthorizeSecurityGroupIngressType
{
  char *userId;     /* required element of type xsd:string */
  char *groupName;          /* required element of type xsd:string */
  struct ec2__IpPermissionSetType *ipPermissions;   /* required element of type ec2:IpPermissionSetType */
};
#endif

#ifndef SOAP_TYPE_ec2__AuthorizeSecurityGroupIngressResponseType
#define SOAP_TYPE_ec2__AuthorizeSecurityGroupIngressResponseType (179)
/* ec2:AuthorizeSecurityGroupIngressResponseType */
struct ec2__AuthorizeSecurityGroupIngressResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE_ec2__RevokeSecurityGroupIngressType
#define SOAP_TYPE_ec2__RevokeSecurityGroupIngressType (180)
/* ec2:RevokeSecurityGroupIngressType */
struct ec2__RevokeSecurityGroupIngressType
{
  char *userId;     /* required element of type xsd:string */
  char *groupName;          /* required element of type xsd:string */
  struct ec2__IpPermissionSetType *ipPermissions;   /* required element of type ec2:IpPermissionSetType */
};
#endif

#ifndef SOAP_TYPE_ec2__RevokeSecurityGroupIngressResponseType
#define SOAP_TYPE_ec2__RevokeSecurityGroupIngressResponseType (181)
/* ec2:RevokeSecurityGroupIngressResponseType */
struct ec2__RevokeSecurityGroupIngressResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE_ec2__InstanceStateType
#define SOAP_TYPE_ec2__InstanceStateType (100)
/* ec2:InstanceStateType */
struct ec2__InstanceStateType
{
  int code;         /* required element of type xsd:int */
  char *name;       /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE__ec2__union_ModifyImageAttributeType
#define SOAP_TYPE__ec2__union_ModifyImageAttributeType (187)
/* xsd:choice */
union _ec2__union_ModifyImageAttributeType
{
#define SOAP_UNION__ec2__union_ModifyImageAttributeType_launchPermission (1)
  struct ec2__LaunchPermissionOperationType *launchPermission;
#define SOAP_UNION__ec2__union_ModifyImageAttributeType_productCodes (2)
  struct ec2__ProductCodeListType *productCodes;
};
#endif

#ifndef SOAP_TYPE_ec2__ModifyImageAttributeType
#define SOAP_TYPE_ec2__ModifyImageAttributeType (182)
/* ec2:ModifyImageAttributeType */
struct ec2__ModifyImageAttributeType
{
  char *imageId;    /* required element of type xsd:string */
  int __union_ModifyImageAttributeType;     /* union discriminant (of union defined below) */
  union _ec2__union_ModifyImageAttributeType union_ModifyImageAttributeType;        /* required element of type xsd:choice */
};
#endif

#ifndef SOAP_TYPE__ec2__union_LaunchPermissionOperationType
#define SOAP_TYPE__ec2__union_LaunchPermissionOperationType (190)
/* xsd:choice */
union _ec2__union_LaunchPermissionOperationType
{
#define SOAP_UNION__ec2__union_LaunchPermissionOperationType_add (1)
  struct ec2__LaunchPermissionListType *add;
#define SOAP_UNION__ec2__union_LaunchPermissionOperationType_remove (2)
  struct ec2__LaunchPermissionListType *remove;
};
#endif

#ifndef SOAP_TYPE_ec2__LaunchPermissionOperationType
#define SOAP_TYPE_ec2__LaunchPermissionOperationType (183)
/* Choice: */
struct ec2__LaunchPermissionOperationType
{
  int __union_LaunchPermissionOperationType;        /* union discriminant (of union defined below) */
  union _ec2__union_LaunchPermissionOperationType
    union_LaunchPermissionOperationType;                                                    /* required element of type xsd:choice */
};
#endif

#ifndef SOAP_TYPE_ec2__LaunchPermissionListType
#define SOAP_TYPE_ec2__LaunchPermissionListType (188)
/* ec2:LaunchPermissionListType */
struct ec2__LaunchPermissionListType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__LaunchPermissionItemType *item;       /* optional element of type ec2:LaunchPermissionItemType */
};
#endif

#ifndef SOAP_TYPE__ec2__union_LaunchPermissionItemType
#define SOAP_TYPE__ec2__union_LaunchPermissionItemType (193)
/* xsd:choice */
union _ec2__union_LaunchPermissionItemType
{
#define SOAP_UNION__ec2__union_LaunchPermissionItemType_userId (1)
  char *userId;
#define SOAP_UNION__ec2__union_LaunchPermissionItemType_group (2)
  char *group;
};
#endif

#ifndef SOAP_TYPE_ec2__LaunchPermissionItemType
#define SOAP_TYPE_ec2__LaunchPermissionItemType (191)
/* Choice: */
struct ec2__LaunchPermissionItemType
{
  int __union_LaunchPermissionItemType;     /* union discriminant (of union defined below) */
  union _ec2__union_LaunchPermissionItemType union_LaunchPermissionItemType;        /* required element of type xsd:choice */
};
#endif

#ifndef SOAP_TYPE_ec2__ProductCodeListType
#define SOAP_TYPE_ec2__ProductCodeListType (185)
/* ec2:ProductCodeListType */
struct ec2__ProductCodeListType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__ProductCodeItemType *item;    /* optional element of type ec2:ProductCodeItemType */
};
#endif

#ifndef SOAP_TYPE__ec2__union_ProductCodeItemType
#define SOAP_TYPE__ec2__union_ProductCodeItemType (196)
/* xsd:choice */
union _ec2__union_ProductCodeItemType
{
#define SOAP_UNION__ec2__union_ProductCodeItemType_productCode (1)
  char *productCode;
};
#endif

#ifndef SOAP_TYPE_ec2__ProductCodeItemType
#define SOAP_TYPE_ec2__ProductCodeItemType (194)
/* Choice: */
struct ec2__ProductCodeItemType
{
  int __union_ProductCodeItemType;          /* union discriminant (of union defined below) */
  union _ec2__union_ProductCodeItemType union_ProductCodeItemType;          /* required element of type xsd:choice */
};
#endif

#ifndef SOAP_TYPE_ec2__ModifyImageAttributeResponseType
#define SOAP_TYPE_ec2__ModifyImageAttributeResponseType (197)
/* ec2:ModifyImageAttributeResponseType */
struct ec2__ModifyImageAttributeResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE__ec2__union_ResetImageAttributeType
#define SOAP_TYPE__ec2__union_ResetImageAttributeType (201)
/* xsd:choice */
union _ec2__union_ResetImageAttributeType
{
#define SOAP_UNION__ec2__union_ResetImageAttributeType_launchPermission (1)
  struct ec2__EmptyElementType *launchPermission;
};
#endif

#ifndef SOAP_TYPE_ec2__ResetImageAttributeType
#define SOAP_TYPE_ec2__ResetImageAttributeType (198)
/* ec2:ResetImageAttributeType */
struct ec2__ResetImageAttributeType
{
  int __union_ResetImageAttributeType;      /* union discriminant (of union defined below) */
  union _ec2__union_ResetImageAttributeType union_ResetImageAttributeType;          /* required element of type xsd:choice */
  char *imageId;    /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__EmptyElementType
#define SOAP_TYPE_ec2__EmptyElementType (199)
/* ec2:EmptyElementType */
struct ec2__EmptyElementType
{
#ifdef WITH_NOEMPTYSTRUCT
  char dummy;       /* dummy member to enable compilation */
#endif
};
#endif

#ifndef SOAP_TYPE_ec2__ResetImageAttributeResponseType
#define SOAP_TYPE_ec2__ResetImageAttributeResponseType (202)
/* ec2:ResetImageAttributeResponseType */
struct ec2__ResetImageAttributeResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE__ec2__union_DescribeImageAttributeType
#define SOAP_TYPE__ec2__union_DescribeImageAttributeType (204)
/* xsd:choice */
union _ec2__union_DescribeImageAttributeType
{
#define SOAP_UNION__ec2__union_DescribeImageAttributeType_launchPermission (1)
  struct ec2__EmptyElementType *launchPermission;
#define SOAP_UNION__ec2__union_DescribeImageAttributeType_productCodes (2)
  struct ec2__EmptyElementType *productCodes;
#define SOAP_UNION__ec2__union_DescribeImageAttributeType_kernel (3)
  struct ec2__EmptyElementType *kernel;
#define SOAP_UNION__ec2__union_DescribeImageAttributeType_ramdisk (4)
  struct ec2__EmptyElementType *ramdisk;
#define SOAP_UNION__ec2__union_DescribeImageAttributeType_blockDeviceMapping (5)
  struct ec2__EmptyElementType *blockDeviceMapping;
};
#endif // ifndef SOAP_TYPE__ec2__union_DescribeImageAttributeType

#ifndef SOAP_TYPE_ec2__DescribeImageAttributeType
#define SOAP_TYPE_ec2__DescribeImageAttributeType (203)
/* ec2:DescribeImageAttributeType */
struct ec2__DescribeImageAttributeType
{
  int __union_DescribeImageAttributeType;   /* union discriminant (of union defined below) */
  union _ec2__union_DescribeImageAttributeType union_DescribeImageAttributeType;    /* required element of type xsd:choice */
  char *imageId;    /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE__ec2__union_DescribeImageAttributeResponseType
#define SOAP_TYPE__ec2__union_DescribeImageAttributeResponseType (208)
/* xsd:choice */
union _ec2__union_DescribeImageAttributeResponseType
{
#define \
  SOAP_UNION__ec2__union_DescribeImageAttributeResponseType_launchPermission (1)
  struct ec2__LaunchPermissionListType *launchPermission;
#define SOAP_UNION__ec2__union_DescribeImageAttributeResponseType_productCodes ( \
    2)
  struct ec2__ProductCodeListType *productCodes;
#define SOAP_UNION__ec2__union_DescribeImageAttributeResponseType_kernel (3)
  struct ec2__NullableAttributeValueType *kernel;
#define SOAP_UNION__ec2__union_DescribeImageAttributeResponseType_ramdisk (4)
  struct ec2__NullableAttributeValueType *ramdisk;
#define \
  SOAP_UNION__ec2__union_DescribeImageAttributeResponseType_blockDeviceMapping ( \
    5)
  struct ec2__BlockDeviceMappingType *blockDeviceMapping;
};
#endif // ifndef SOAP_TYPE__ec2__union_DescribeImageAttributeResponseType

#ifndef SOAP_TYPE_ec2__DescribeImageAttributeResponseType
#define SOAP_TYPE_ec2__DescribeImageAttributeResponseType (205)
/* ec2:DescribeImageAttributeResponseType */
struct ec2__DescribeImageAttributeResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  char *imageId;    /* required element of type xsd:string */
  int __union_DescribeImageAttributeResponseType;   /* union discriminant (of union defined below) */
  union _ec2__union_DescribeImageAttributeResponseType
    union_DescribeImageAttributeResponseType;                                                       /* required element of type xsd:choice */
};
#endif // ifndef SOAP_TYPE_ec2__DescribeImageAttributeResponseType

#ifndef SOAP_TYPE_ec2__NullableAttributeValueType
#define SOAP_TYPE_ec2__NullableAttributeValueType (206)
/* ec2:NullableAttributeValueType */
struct ec2__NullableAttributeValueType
{
  char *value;      /* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__AttributeValueType
#define SOAP_TYPE_ec2__AttributeValueType (209)
/* ec2:AttributeValueType */
struct ec2__AttributeValueType
{
  char *value;      /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__ConfirmProductInstanceType
#define SOAP_TYPE_ec2__ConfirmProductInstanceType (210)
/* ec2:ConfirmProductInstanceType */
struct ec2__ConfirmProductInstanceType
{
  char *productCode;        /* required element of type xsd:string */
  char *instanceId;         /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__ProductCodesSetType
#define SOAP_TYPE_ec2__ProductCodesSetType (102)
/* ec2:ProductCodesSetType */
struct ec2__ProductCodesSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__ProductCodesSetItemType *item;        /* optional element of type ec2:ProductCodesSetItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__ProductCodesSetItemType
#define SOAP_TYPE_ec2__ProductCodesSetItemType (211)
/* ec2:ProductCodesSetItemType */
struct ec2__ProductCodesSetItemType
{
  char *productCode;        /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__ConfirmProductInstanceResponseType
#define SOAP_TYPE_ec2__ConfirmProductInstanceResponseType (213)
/* ec2:ConfirmProductInstanceResponseType */
struct ec2__ConfirmProductInstanceResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
  char *ownerId;    /* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeAvailabilityZonesType
#define SOAP_TYPE_ec2__DescribeAvailabilityZonesType (214)
/* ec2:DescribeAvailabilityZonesType */
struct ec2__DescribeAvailabilityZonesType
{
  struct ec2__DescribeAvailabilityZonesSetType *availabilityZoneSet;        /* required element of type ec2:DescribeAvailabilityZonesSetType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeAvailabilityZonesSetType
#define SOAP_TYPE_ec2__DescribeAvailabilityZonesSetType (215)
/* ec2:DescribeAvailabilityZonesSetType */
struct ec2__DescribeAvailabilityZonesSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeAvailabilityZonesSetItemType *item;   /* optional element of type ec2:DescribeAvailabilityZonesSetItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeAvailabilityZonesSetItemType
#define SOAP_TYPE_ec2__DescribeAvailabilityZonesSetItemType (217)
/* ec2:DescribeAvailabilityZonesSetItemType */
struct ec2__DescribeAvailabilityZonesSetItemType
{
  char *zoneName;   /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeAvailabilityZonesResponseType
#define SOAP_TYPE_ec2__DescribeAvailabilityZonesResponseType (219)
/* ec2:DescribeAvailabilityZonesResponseType */
struct ec2__DescribeAvailabilityZonesResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  struct ec2__AvailabilityZoneSetType *availabilityZoneInfo;        /* required element of type ec2:AvailabilityZoneSetType */
};
#endif

#ifndef SOAP_TYPE_ec2__AvailabilityZoneSetType
#define SOAP_TYPE_ec2__AvailabilityZoneSetType (220)
/* ec2:AvailabilityZoneSetType */
struct ec2__AvailabilityZoneSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__AvailabilityZoneItemType *item;       /* optional element of type ec2:AvailabilityZoneItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__AvailabilityZoneItemType
#define SOAP_TYPE_ec2__AvailabilityZoneItemType (222)
/* ec2:AvailabilityZoneItemType */
struct ec2__AvailabilityZoneItemType
{
  char *zoneName;   /* required element of type xsd:string */
  char *zoneState;          /* required element of type xsd:string */
  char *regionName;         /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__AllocateAddressType
#define SOAP_TYPE_ec2__AllocateAddressType (224)
/* ec2:AllocateAddressType */
struct ec2__AllocateAddressType
{
#ifdef WITH_NOEMPTYSTRUCT
  char dummy;       /* dummy member to enable compilation */
#endif
};
#endif

#ifndef SOAP_TYPE_ec2__AllocateAddressResponseType
#define SOAP_TYPE_ec2__AllocateAddressResponseType (225)
/* ec2:AllocateAddressResponseType */
struct ec2__AllocateAddressResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  char *publicIp;   /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__ReleaseAddressType
#define SOAP_TYPE_ec2__ReleaseAddressType (226)
/* ec2:ReleaseAddressType */
struct ec2__ReleaseAddressType
{
  char *publicIp;   /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__ReleaseAddressResponseType
#define SOAP_TYPE_ec2__ReleaseAddressResponseType (227)
/* ec2:ReleaseAddressResponseType */
struct ec2__ReleaseAddressResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeAddressesType
#define SOAP_TYPE_ec2__DescribeAddressesType (228)
/* ec2:DescribeAddressesType */
struct ec2__DescribeAddressesType
{
  struct ec2__DescribeAddressesInfoType *publicIpsSet;      /* required element of type ec2:DescribeAddressesInfoType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeAddressesInfoType
#define SOAP_TYPE_ec2__DescribeAddressesInfoType (229)
/* ec2:DescribeAddressesInfoType */
struct ec2__DescribeAddressesInfoType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeAddressesItemType *item;      /* optional element of type ec2:DescribeAddressesItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeAddressesItemType
#define SOAP_TYPE_ec2__DescribeAddressesItemType (231)
/* ec2:DescribeAddressesItemType */
struct ec2__DescribeAddressesItemType
{
  char *publicIp;   /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeAddressesResponseType
#define SOAP_TYPE_ec2__DescribeAddressesResponseType (233)
/* ec2:DescribeAddressesResponseType */
struct ec2__DescribeAddressesResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  struct ec2__DescribeAddressesResponseInfoType *addressesSet;      /* required element of type ec2:DescribeAddressesResponseInfoType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeAddressesResponseInfoType
#define SOAP_TYPE_ec2__DescribeAddressesResponseInfoType (234)
/* ec2:DescribeAddressesResponseInfoType */
struct ec2__DescribeAddressesResponseInfoType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeAddressesResponseItemType *item;      /* optional element of type ec2:DescribeAddressesResponseItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeAddressesResponseItemType
#define SOAP_TYPE_ec2__DescribeAddressesResponseItemType (236)
/* ec2:DescribeAddressesResponseItemType */
struct ec2__DescribeAddressesResponseItemType
{
  char *publicIp;   /* required element of type xsd:string */
  char *instanceId;         /* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__AssociateAddressType
#define SOAP_TYPE_ec2__AssociateAddressType (238)
/* ec2:AssociateAddressType */
struct ec2__AssociateAddressType
{
  char *publicIp;   /* required element of type xsd:string */
  char *instanceId;         /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__AssociateAddressResponseType
#define SOAP_TYPE_ec2__AssociateAddressResponseType (239)
/* ec2:AssociateAddressResponseType */
struct ec2__AssociateAddressResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE_ec2__DisassociateAddressType
#define SOAP_TYPE_ec2__DisassociateAddressType (240)
/* ec2:DisassociateAddressType */
struct ec2__DisassociateAddressType
{
  char *publicIp;   /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DisassociateAddressResponseType
#define SOAP_TYPE_ec2__DisassociateAddressResponseType (241)
/* ec2:DisassociateAddressResponseType */
struct ec2__DisassociateAddressResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE_ec2__CreateVolumeType
#define SOAP_TYPE_ec2__CreateVolumeType (242)
/* ec2:CreateVolumeType */
struct ec2__CreateVolumeType
{
  char *size;       /* optional element of type xsd:string */
  char *snapshotId;         /* optional element of type xsd:string */
  char *availabilityZone;   /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__CreateVolumeResponseType
#define SOAP_TYPE_ec2__CreateVolumeResponseType (243)
/* ec2:CreateVolumeResponseType */
struct ec2__CreateVolumeResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  char *volumeId;   /* required element of type xsd:string */
  char *size;       /* required element of type xsd:string */
  char *snapshotId;         /* required element of type xsd:string */
  char *availabilityZone;   /* required element of type xsd:string */
  char *status;     /* required element of type xsd:string */
  time_t createTime;        /* required element of type xsd:dateTime */
};
#endif // ifndef SOAP_TYPE_ec2__CreateVolumeResponseType

#ifndef SOAP_TYPE_ec2__DeleteVolumeType
#define SOAP_TYPE_ec2__DeleteVolumeType (244)
/* ec2:DeleteVolumeType */
struct ec2__DeleteVolumeType
{
  char *volumeId;   /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DeleteVolumeResponseType
#define SOAP_TYPE_ec2__DeleteVolumeResponseType (245)
/* ec2:DeleteVolumeResponseType */
struct ec2__DeleteVolumeResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeVolumesType
#define SOAP_TYPE_ec2__DescribeVolumesType (246)
/* ec2:DescribeVolumesType */
struct ec2__DescribeVolumesType
{
  struct ec2__DescribeVolumesSetType *volumeSet;    /* required element of type ec2:DescribeVolumesSetType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeVolumesSetType
#define SOAP_TYPE_ec2__DescribeVolumesSetType (247)
/* ec2:DescribeVolumesSetType */
struct ec2__DescribeVolumesSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeVolumesSetItemType *item;     /* optional element of type ec2:DescribeVolumesSetItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeVolumesSetItemType
#define SOAP_TYPE_ec2__DescribeVolumesSetItemType (249)
/* ec2:DescribeVolumesSetItemType */
struct ec2__DescribeVolumesSetItemType
{
  char *volumeId;   /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeVolumesResponseType
#define SOAP_TYPE_ec2__DescribeVolumesResponseType (251)
/* ec2:DescribeVolumesResponseType */
struct ec2__DescribeVolumesResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  struct ec2__DescribeVolumesSetResponseType *volumeSet;    /* required element of type ec2:DescribeVolumesSetResponseType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeVolumesSetResponseType
#define SOAP_TYPE_ec2__DescribeVolumesSetResponseType (252)
/* ec2:DescribeVolumesSetResponseType */
struct ec2__DescribeVolumesSetResponseType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeVolumesSetItemResponseType *item;     /* optional element of type ec2:DescribeVolumesSetItemResponseType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeVolumesSetItemResponseType
#define SOAP_TYPE_ec2__DescribeVolumesSetItemResponseType (254)
/* ec2:DescribeVolumesSetItemResponseType */
struct ec2__DescribeVolumesSetItemResponseType
{
  char *volumeId;   /* required element of type xsd:string */
  char *size;       /* required element of type xsd:string */
  char *snapshotId;         /* required element of type xsd:string */
  char *availabilityZone;   /* required element of type xsd:string */
  char *status;     /* required element of type xsd:string */
  time_t createTime;        /* required element of type xsd:dateTime */
  struct ec2__AttachmentSetResponseType *attachmentSet;     /* required element of type ec2:AttachmentSetResponseType */
};
#endif // ifndef SOAP_TYPE_ec2__DescribeVolumesSetItemResponseType

#ifndef SOAP_TYPE_ec2__AttachmentSetResponseType
#define SOAP_TYPE_ec2__AttachmentSetResponseType (256)
/* ec2:AttachmentSetResponseType */
struct ec2__AttachmentSetResponseType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__AttachmentSetItemResponseType *item;          /* optional element of type ec2:AttachmentSetItemResponseType */
};
#endif

#ifndef SOAP_TYPE_ec2__AttachmentSetItemResponseType
#define SOAP_TYPE_ec2__AttachmentSetItemResponseType (258)
/* ec2:AttachmentSetItemResponseType */
struct ec2__AttachmentSetItemResponseType
{
  char *volumeId;   /* required element of type xsd:string */
  char *instanceId;         /* required element of type xsd:string */
  char *device;     /* required element of type xsd:string */
  char *status;     /* required element of type xsd:string */
  time_t attachTime;        /* required element of type xsd:dateTime */
};
#endif // ifndef SOAP_TYPE_ec2__AttachmentSetItemResponseType

#ifndef SOAP_TYPE_ec2__AttachVolumeType
#define SOAP_TYPE_ec2__AttachVolumeType (260)
/* ec2:AttachVolumeType */
struct ec2__AttachVolumeType
{
  char *volumeId;   /* required element of type xsd:string */
  char *instanceId;         /* required element of type xsd:string */
  char *device;     /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__AttachVolumeResponseType
#define SOAP_TYPE_ec2__AttachVolumeResponseType (261)
/* ec2:AttachVolumeResponseType */
struct ec2__AttachVolumeResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  char *volumeId;   /* required element of type xsd:string */
  char *instanceId;         /* required element of type xsd:string */
  char *device;     /* required element of type xsd:string */
  char *status;     /* required element of type xsd:string */
  time_t attachTime;        /* required element of type xsd:dateTime */
};
#endif // ifndef SOAP_TYPE_ec2__AttachVolumeResponseType

#ifndef SOAP_TYPE_ec2__DetachVolumeType
#define SOAP_TYPE_ec2__DetachVolumeType (262)
/* ec2:DetachVolumeType */
struct ec2__DetachVolumeType
{
  char *volumeId;   /* required element of type xsd:string */
  char *instanceId;         /* optional element of type xsd:string */
  char *device;     /* optional element of type xsd:string */
  enum xsd__boolean_ *force;        /* optional element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE_ec2__DetachVolumeResponseType
#define SOAP_TYPE_ec2__DetachVolumeResponseType (264)
/* ec2:DetachVolumeResponseType */
struct ec2__DetachVolumeResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  char *volumeId;   /* required element of type xsd:string */
  char *instanceId;         /* required element of type xsd:string */
  char *device;     /* required element of type xsd:string */
  char *status;     /* required element of type xsd:string */
  time_t attachTime;        /* required element of type xsd:dateTime */
};
#endif // ifndef SOAP_TYPE_ec2__DetachVolumeResponseType

#ifndef SOAP_TYPE_ec2__CreateSnapshotType
#define SOAP_TYPE_ec2__CreateSnapshotType (265)
/* ec2:CreateSnapshotType */
struct ec2__CreateSnapshotType
{
  char *volumeId;   /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__CreateSnapshotResponseType
#define SOAP_TYPE_ec2__CreateSnapshotResponseType (266)
/* ec2:CreateSnapshotResponseType */
struct ec2__CreateSnapshotResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  char *snapshotId;         /* required element of type xsd:string */
  char *volumeId;   /* required element of type xsd:string */
  char *status;     /* required element of type xsd:string */
  time_t startTime;         /* required element of type xsd:dateTime */
  char *progress;   /* required element of type xsd:string */
};
#endif // ifndef SOAP_TYPE_ec2__CreateSnapshotResponseType

#ifndef SOAP_TYPE_ec2__DeleteSnapshotType
#define SOAP_TYPE_ec2__DeleteSnapshotType (267)
/* ec2:DeleteSnapshotType */
struct ec2__DeleteSnapshotType
{
  char *snapshotId;         /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DeleteSnapshotResponseType
#define SOAP_TYPE_ec2__DeleteSnapshotResponseType (268)
/* ec2:DeleteSnapshotResponseType */
struct ec2__DeleteSnapshotResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  enum xsd__boolean_ return_;       /* required element of type xsd:boolean */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeSnapshotsType
#define SOAP_TYPE_ec2__DescribeSnapshotsType (269)
/* ec2:DescribeSnapshotsType */
struct ec2__DescribeSnapshotsType
{
  struct ec2__DescribeSnapshotsSetType *snapshotSet;        /* required element of type ec2:DescribeSnapshotsSetType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeSnapshotsSetType
#define SOAP_TYPE_ec2__DescribeSnapshotsSetType (270)
/* ec2:DescribeSnapshotsSetType */
struct ec2__DescribeSnapshotsSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeSnapshotsSetItemType *item;   /* optional element of type ec2:DescribeSnapshotsSetItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeSnapshotsSetItemType
#define SOAP_TYPE_ec2__DescribeSnapshotsSetItemType (272)
/* ec2:DescribeSnapshotsSetItemType */
struct ec2__DescribeSnapshotsSetItemType
{
  char *snapshotId;         /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeSnapshotsResponseType
#define SOAP_TYPE_ec2__DescribeSnapshotsResponseType (274)
/* ec2:DescribeSnapshotsResponseType */
struct ec2__DescribeSnapshotsResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  struct ec2__DescribeSnapshotsSetResponseType *snapshotSet;        /* required element of type ec2:DescribeSnapshotsSetResponseType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeSnapshotsSetResponseType
#define SOAP_TYPE_ec2__DescribeSnapshotsSetResponseType (275)
/* ec2:DescribeSnapshotsSetResponseType */
struct ec2__DescribeSnapshotsSetResponseType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeSnapshotsSetItemResponseType *item;   /* optional element of type ec2:DescribeSnapshotsSetItemResponseType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeSnapshotsSetItemResponseType
#define SOAP_TYPE_ec2__DescribeSnapshotsSetItemResponseType (277)
/* ec2:DescribeSnapshotsSetItemResponseType */
struct ec2__DescribeSnapshotsSetItemResponseType
{
  char *snapshotId;         /* required element of type xsd:string */
  char *volumeId;   /* required element of type xsd:string */
  char *status;     /* required element of type xsd:string */
  time_t startTime;         /* required element of type xsd:dateTime */
  char *progress;   /* required element of type xsd:string */
};
#endif // ifndef SOAP_TYPE_ec2__DescribeSnapshotsSetItemResponseType

#ifndef SOAP_TYPE_ec2__BundleInstanceType
#define SOAP_TYPE_ec2__BundleInstanceType (279)
/* ec2:BundleInstanceType */
struct ec2__BundleInstanceType
{
  char *instanceId;         /* required element of type xsd:string */
  struct ec2__BundleInstanceTaskStorageType *storage;       /* required element of type ec2:BundleInstanceTaskStorageType */
};
#endif

#ifndef SOAP_TYPE_ec2__BundleInstanceTaskStorageType
#define SOAP_TYPE_ec2__BundleInstanceTaskStorageType (280)
/* ec2:BundleInstanceTaskStorageType */
struct ec2__BundleInstanceTaskStorageType
{
  struct ec2__BundleInstanceS3StorageType *S3;      /* required element of type ec2:BundleInstanceS3StorageType */
};
#endif

#ifndef SOAP_TYPE_ec2__BundleInstanceS3StorageType
#define SOAP_TYPE_ec2__BundleInstanceS3StorageType (282)
/* ec2:BundleInstanceS3StorageType */
struct ec2__BundleInstanceS3StorageType
{
  char *bucket;     /* required element of type xsd:string */
  char *prefix;     /* required element of type xsd:string */
  char *awsAccessKeyId;     /* optional element of type xsd:string */
  char *uploadPolicy;       /* optional element of type xsd:string */
  char *uploadPolicySignature;      /* optional element of type xsd:string */
};
#endif // ifndef SOAP_TYPE_ec2__BundleInstanceS3StorageType

#ifndef SOAP_TYPE_ec2__BundleInstanceResponseType
#define SOAP_TYPE_ec2__BundleInstanceResponseType (284)
/* ec2:BundleInstanceResponseType */
struct ec2__BundleInstanceResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  struct ec2__BundleInstanceTaskType *bundleInstanceTask;   /* required element of type ec2:BundleInstanceTaskType */
};
#endif

#ifndef SOAP_TYPE_ec2__BundleInstanceTaskType
#define SOAP_TYPE_ec2__BundleInstanceTaskType (285)
/* ec2:BundleInstanceTaskType */
struct ec2__BundleInstanceTaskType
{
  char *instanceId;         /* required element of type xsd:string */
  char *bundleId;   /* required element of type xsd:string */
  char *state;      /* required element of type xsd:string */
  time_t startTime;         /* required element of type xsd:dateTime */
  time_t updateTime;        /* required element of type xsd:dateTime */
  struct ec2__BundleInstanceTaskStorageType *storage;       /* required element of type ec2:BundleInstanceTaskStorageType */
  char *progress;   /* optional element of type xsd:string */
  struct ec2__BundleInstanceTaskErrorType *error;   /* optional element of type ec2:BundleInstanceTaskErrorType */
};
#endif // ifndef SOAP_TYPE_ec2__BundleInstanceTaskType

#ifndef SOAP_TYPE_ec2__BundleInstanceTaskErrorType
#define SOAP_TYPE_ec2__BundleInstanceTaskErrorType (287)
/* ec2:BundleInstanceTaskErrorType */
struct ec2__BundleInstanceTaskErrorType
{
  char *code;       /* required element of type xsd:string */
  char *message;    /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeBundleTasksType
#define SOAP_TYPE_ec2__DescribeBundleTasksType (289)
/* ec2:DescribeBundleTasksType */
struct ec2__DescribeBundleTasksType
{
  struct ec2__DescribeBundleTasksInfoType *bundlesSet;      /* required element of type ec2:DescribeBundleTasksInfoType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeBundleTasksInfoType
#define SOAP_TYPE_ec2__DescribeBundleTasksInfoType (290)
/* ec2:DescribeBundleTasksInfoType */
struct ec2__DescribeBundleTasksInfoType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeBundleTasksItemType *item;    /* optional element of type ec2:DescribeBundleTasksItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeBundleTasksItemType
#define SOAP_TYPE_ec2__DescribeBundleTasksItemType (292)
/* ec2:DescribeBundleTasksItemType */
struct ec2__DescribeBundleTasksItemType
{
  char *bundleId;   /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeBundleTasksResponseType
#define SOAP_TYPE_ec2__DescribeBundleTasksResponseType (294)
/* ec2:DescribeBundleTasksResponseType */
struct ec2__DescribeBundleTasksResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  struct ec2__BundleInstanceTasksSetType *bundleInstanceTasksSet;   /* required element of type ec2:BundleInstanceTasksSetType */
};
#endif

#ifndef SOAP_TYPE_ec2__BundleInstanceTasksSetType
#define SOAP_TYPE_ec2__BundleInstanceTasksSetType (295)
/* ec2:BundleInstanceTasksSetType */
struct ec2__BundleInstanceTasksSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__BundleInstanceTaskType *item;         /* optional element of type ec2:BundleInstanceTaskType */
};
#endif

#ifndef SOAP_TYPE_ec2__CancelBundleTaskType
#define SOAP_TYPE_ec2__CancelBundleTaskType (297)
/* ec2:CancelBundleTaskType */
struct ec2__CancelBundleTaskType
{
  char *bundleId;   /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__CancelBundleTaskResponseType
#define SOAP_TYPE_ec2__CancelBundleTaskResponseType (298)
/* ec2:CancelBundleTaskResponseType */
struct ec2__CancelBundleTaskResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  struct ec2__BundleInstanceTaskType *bundleInstanceTask;   /* required element of type ec2:BundleInstanceTaskType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeRegionsType
#define SOAP_TYPE_ec2__DescribeRegionsType (299)
/* ec2:DescribeRegionsType */
struct ec2__DescribeRegionsType
{
  struct ec2__DescribeRegionsSetType *regionSet;    /* required element of type ec2:DescribeRegionsSetType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeRegionsSetType
#define SOAP_TYPE_ec2__DescribeRegionsSetType (300)
/* ec2:DescribeRegionsSetType */
struct ec2__DescribeRegionsSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__DescribeRegionsSetItemType *item;     /* optional element of type ec2:DescribeRegionsSetItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeRegionsSetItemType
#define SOAP_TYPE_ec2__DescribeRegionsSetItemType (302)
/* ec2:DescribeRegionsSetItemType */
struct ec2__DescribeRegionsSetItemType
{
  char *regionName;         /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ec2__DescribeRegionsResponseType
#define SOAP_TYPE_ec2__DescribeRegionsResponseType (304)
/* ec2:DescribeRegionsResponseType */
struct ec2__DescribeRegionsResponseType
{
  char *requestId;          /* SOAP 1.2 RPC return element (when namespace qualified) */    /* required element of type xsd:string */
  struct ec2__RegionSetType *regionInfo;    /* required element of type ec2:RegionSetType */
};
#endif

#ifndef SOAP_TYPE_ec2__RegionSetType
#define SOAP_TYPE_ec2__RegionSetType (305)
/* ec2:RegionSetType */
struct ec2__RegionSetType
{
  int __sizeitem;   /* sequence of elements <item> */
  struct ec2__RegionItemType *item;         /* optional element of type ec2:RegionItemType */
};
#endif

#ifndef SOAP_TYPE_ec2__RegionItemType
#define SOAP_TYPE_ec2__RegionItemType (307)
/* ec2:RegionItemType */
struct ec2__RegionItemType
{
  char *regionName;         /* required element of type xsd:string */
  char *regionEndpoint;     /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE___ec2__RegisterImage
#define SOAP_TYPE___ec2__RegisterImage (312)
/* Operation wrapper: */
struct __ec2__RegisterImage
{
  struct ec2__RegisterImageType *ec2__RegisterImage;        /* optional element of type ec2:RegisterImageType */
};
#endif

#ifndef SOAP_TYPE___ec2__DeregisterImage
#define SOAP_TYPE___ec2__DeregisterImage (316)
/* Operation wrapper: */
struct __ec2__DeregisterImage
{
  struct ec2__DeregisterImageType *ec2__DeregisterImage;    /* optional element of type ec2:DeregisterImageType */
};
#endif

#ifndef SOAP_TYPE___ec2__CreateKeyPair
#define SOAP_TYPE___ec2__CreateKeyPair (320)
/* Operation wrapper: */
struct __ec2__CreateKeyPair
{
  struct ec2__CreateKeyPairType *ec2__CreateKeyPair;        /* optional element of type ec2:CreateKeyPairType */
};
#endif

#ifndef SOAP_TYPE___ec2__DescribeKeyPairs
#define SOAP_TYPE___ec2__DescribeKeyPairs (324)
/* Operation wrapper: */
struct __ec2__DescribeKeyPairs
{
  struct ec2__DescribeKeyPairsType *ec2__DescribeKeyPairs;          /* optional element of type ec2:DescribeKeyPairsType */
};
#endif

#ifndef SOAP_TYPE___ec2__DeleteKeyPair
#define SOAP_TYPE___ec2__DeleteKeyPair (328)
/* Operation wrapper: */
struct __ec2__DeleteKeyPair
{
  struct ec2__DeleteKeyPairType *ec2__DeleteKeyPair;        /* optional element of type ec2:DeleteKeyPairType */
};
#endif

#ifndef SOAP_TYPE___ec2__RunInstances
#define SOAP_TYPE___ec2__RunInstances (332)
/* Operation wrapper: */
struct __ec2__RunInstances
{
  struct ec2__RunInstancesType *ec2__RunInstances;          /* optional element of type ec2:RunInstancesType */
};
#endif

#ifndef SOAP_TYPE___ec2__GetConsoleOutput
#define SOAP_TYPE___ec2__GetConsoleOutput (336)
/* Operation wrapper: */
struct __ec2__GetConsoleOutput
{
  struct ec2__GetConsoleOutputType *ec2__GetConsoleOutput;          /* optional element of type ec2:GetConsoleOutputType */
};
#endif

#ifndef SOAP_TYPE___ec2__TerminateInstances
#define SOAP_TYPE___ec2__TerminateInstances (340)
/* Operation wrapper: */
struct __ec2__TerminateInstances
{
  struct ec2__TerminateInstancesType *ec2__TerminateInstances;      /* optional element of type ec2:TerminateInstancesType */
};
#endif

#ifndef SOAP_TYPE___ec2__RebootInstances
#define SOAP_TYPE___ec2__RebootInstances (344)
/* Operation wrapper: */
struct __ec2__RebootInstances
{
  struct ec2__RebootInstancesType *ec2__RebootInstances;    /* optional element of type ec2:RebootInstancesType */
};
#endif

#ifndef SOAP_TYPE___ec2__DescribeInstances
#define SOAP_TYPE___ec2__DescribeInstances (348)
/* Operation wrapper: */
struct __ec2__DescribeInstances
{
  struct ec2__DescribeInstancesType *ec2__DescribeInstances;        /* optional element of type ec2:DescribeInstancesType */
};
#endif

#ifndef SOAP_TYPE___ec2__DescribeImages
#define SOAP_TYPE___ec2__DescribeImages (352)
/* Operation wrapper: */
struct __ec2__DescribeImages
{
  struct ec2__DescribeImagesType *ec2__DescribeImages;      /* optional element of type ec2:DescribeImagesType */
};
#endif

#ifndef SOAP_TYPE___ec2__CreateSecurityGroup
#define SOAP_TYPE___ec2__CreateSecurityGroup (356)
/* Operation wrapper: */
struct __ec2__CreateSecurityGroup
{
  struct ec2__CreateSecurityGroupType *ec2__CreateSecurityGroup;    /* optional element of type ec2:CreateSecurityGroupType */
};
#endif

#ifndef SOAP_TYPE___ec2__DeleteSecurityGroup
#define SOAP_TYPE___ec2__DeleteSecurityGroup (360)
/* Operation wrapper: */
struct __ec2__DeleteSecurityGroup
{
  struct ec2__DeleteSecurityGroupType *ec2__DeleteSecurityGroup;    /* optional element of type ec2:DeleteSecurityGroupType */
};
#endif

#ifndef SOAP_TYPE___ec2__DescribeSecurityGroups
#define SOAP_TYPE___ec2__DescribeSecurityGroups (364)
/* Operation wrapper: */
struct __ec2__DescribeSecurityGroups
{
  struct ec2__DescribeSecurityGroupsType *ec2__DescribeSecurityGroups;      /* optional element of type ec2:DescribeSecurityGroupsType */
};
#endif

#ifndef SOAP_TYPE___ec2__AuthorizeSecurityGroupIngress
#define SOAP_TYPE___ec2__AuthorizeSecurityGroupIngress (368)
/* Operation wrapper: */
struct __ec2__AuthorizeSecurityGroupIngress
{
  struct ec2__AuthorizeSecurityGroupIngressType *
  ec2__AuthorizeSecurityGroupIngress;                                                       /* optional element of type ec2:AuthorizeSecurityGroupIngressType */
};
#endif

#ifndef SOAP_TYPE___ec2__RevokeSecurityGroupIngress
#define SOAP_TYPE___ec2__RevokeSecurityGroupIngress (372)
/* Operation wrapper: */
struct __ec2__RevokeSecurityGroupIngress
{
  struct ec2__RevokeSecurityGroupIngressType *ec2__RevokeSecurityGroupIngress;      /* optional element of type ec2:RevokeSecurityGroupIngressType */
};
#endif

#ifndef SOAP_TYPE___ec2__ModifyImageAttribute
#define SOAP_TYPE___ec2__ModifyImageAttribute (376)
/* Operation wrapper: */
struct __ec2__ModifyImageAttribute
{
  struct ec2__ModifyImageAttributeType *ec2__ModifyImageAttribute;          /* optional element of type ec2:ModifyImageAttributeType */
};
#endif

#ifndef SOAP_TYPE___ec2__ResetImageAttribute
#define SOAP_TYPE___ec2__ResetImageAttribute (380)
/* Operation wrapper: */
struct __ec2__ResetImageAttribute
{
  struct ec2__ResetImageAttributeType *ec2__ResetImageAttribute;    /* optional element of type ec2:ResetImageAttributeType */
};
#endif

#ifndef SOAP_TYPE___ec2__DescribeImageAttribute
#define SOAP_TYPE___ec2__DescribeImageAttribute (384)
/* Operation wrapper: */
struct __ec2__DescribeImageAttribute
{
  struct ec2__DescribeImageAttributeType *ec2__DescribeImageAttribute;      /* optional element of type ec2:DescribeImageAttributeType */
};
#endif

#ifndef SOAP_TYPE___ec2__ConfirmProductInstance
#define SOAP_TYPE___ec2__ConfirmProductInstance (388)
/* Operation wrapper: */
struct __ec2__ConfirmProductInstance
{
  struct ec2__ConfirmProductInstanceType *ec2__ConfirmProductInstance;      /* optional element of type ec2:ConfirmProductInstanceType */
};
#endif

#ifndef SOAP_TYPE___ec2__DescribeAvailabilityZones
#define SOAP_TYPE___ec2__DescribeAvailabilityZones (392)
/* Operation wrapper: */
struct __ec2__DescribeAvailabilityZones
{
  struct ec2__DescribeAvailabilityZonesType *ec2__DescribeAvailabilityZones;        /* optional element of type ec2:DescribeAvailabilityZonesType */
};
#endif

#ifndef SOAP_TYPE___ec2__AllocateAddress
#define SOAP_TYPE___ec2__AllocateAddress (396)
/* Operation wrapper: */
struct __ec2__AllocateAddress
{
  struct ec2__AllocateAddressType *ec2__AllocateAddress;    /* optional element of type ec2:AllocateAddressType */
};
#endif

#ifndef SOAP_TYPE___ec2__ReleaseAddress
#define SOAP_TYPE___ec2__ReleaseAddress (400)
/* Operation wrapper: */
struct __ec2__ReleaseAddress
{
  struct ec2__ReleaseAddressType *ec2__ReleaseAddress;      /* optional element of type ec2:ReleaseAddressType */
};
#endif

#ifndef SOAP_TYPE___ec2__DescribeAddresses
#define SOAP_TYPE___ec2__DescribeAddresses (404)
/* Operation wrapper: */
struct __ec2__DescribeAddresses
{
  struct ec2__DescribeAddressesType *ec2__DescribeAddresses;        /* optional element of type ec2:DescribeAddressesType */
};
#endif

#ifndef SOAP_TYPE___ec2__AssociateAddress
#define SOAP_TYPE___ec2__AssociateAddress (408)
/* Operation wrapper: */
struct __ec2__AssociateAddress
{
  struct ec2__AssociateAddressType *ec2__AssociateAddress;          /* optional element of type ec2:AssociateAddressType */
};
#endif

#ifndef SOAP_TYPE___ec2__DisassociateAddress
#define SOAP_TYPE___ec2__DisassociateAddress (412)
/* Operation wrapper: */
struct __ec2__DisassociateAddress
{
  struct ec2__DisassociateAddressType *ec2__DisassociateAddress;    /* optional element of type ec2:DisassociateAddressType */
};
#endif

#ifndef SOAP_TYPE___ec2__CreateVolume
#define SOAP_TYPE___ec2__CreateVolume (416)
/* Operation wrapper: */
struct __ec2__CreateVolume
{
  struct ec2__CreateVolumeType *ec2__CreateVolume;          /* optional element of type ec2:CreateVolumeType */
};
#endif

#ifndef SOAP_TYPE___ec2__DeleteVolume
#define SOAP_TYPE___ec2__DeleteVolume (420)
/* Operation wrapper: */
struct __ec2__DeleteVolume
{
  struct ec2__DeleteVolumeType *ec2__DeleteVolume;          /* optional element of type ec2:DeleteVolumeType */
};
#endif

#ifndef SOAP_TYPE___ec2__DescribeVolumes
#define SOAP_TYPE___ec2__DescribeVolumes (424)
/* Operation wrapper: */
struct __ec2__DescribeVolumes
{
  struct ec2__DescribeVolumesType *ec2__DescribeVolumes;    /* optional element of type ec2:DescribeVolumesType */
};
#endif

#ifndef SOAP_TYPE___ec2__AttachVolume
#define SOAP_TYPE___ec2__AttachVolume (428)
/* Operation wrapper: */
struct __ec2__AttachVolume
{
  struct ec2__AttachVolumeType *ec2__AttachVolume;          /* optional element of type ec2:AttachVolumeType */
};
#endif

#ifndef SOAP_TYPE___ec2__DetachVolume
#define SOAP_TYPE___ec2__DetachVolume (432)
/* Operation wrapper: */
struct __ec2__DetachVolume
{
  struct ec2__DetachVolumeType *ec2__DetachVolume;          /* optional element of type ec2:DetachVolumeType */
};
#endif

#ifndef SOAP_TYPE___ec2__CreateSnapshot
#define SOAP_TYPE___ec2__CreateSnapshot (436)
/* Operation wrapper: */
struct __ec2__CreateSnapshot
{
  struct ec2__CreateSnapshotType *ec2__CreateSnapshot;      /* optional element of type ec2:CreateSnapshotType */
};
#endif

#ifndef SOAP_TYPE___ec2__DeleteSnapshot
#define SOAP_TYPE___ec2__DeleteSnapshot (440)
/* Operation wrapper: */
struct __ec2__DeleteSnapshot
{
  struct ec2__DeleteSnapshotType *ec2__DeleteSnapshot;      /* optional element of type ec2:DeleteSnapshotType */
};
#endif

#ifndef SOAP_TYPE___ec2__DescribeSnapshots
#define SOAP_TYPE___ec2__DescribeSnapshots (444)
/* Operation wrapper: */
struct __ec2__DescribeSnapshots
{
  struct ec2__DescribeSnapshotsType *ec2__DescribeSnapshots;        /* optional element of type ec2:DescribeSnapshotsType */
};
#endif

#ifndef SOAP_TYPE___ec2__BundleInstance
#define SOAP_TYPE___ec2__BundleInstance (448)
/* Operation wrapper: */
struct __ec2__BundleInstance
{
  struct ec2__BundleInstanceType *ec2__BundleInstance;      /* optional element of type ec2:BundleInstanceType */
};
#endif

#ifndef SOAP_TYPE___ec2__DescribeBundleTasks
#define SOAP_TYPE___ec2__DescribeBundleTasks (452)
/* Operation wrapper: */
struct __ec2__DescribeBundleTasks
{
  struct ec2__DescribeBundleTasksType *ec2__DescribeBundleTasks;    /* optional element of type ec2:DescribeBundleTasksType */
};
#endif

#ifndef SOAP_TYPE___ec2__CancelBundleTask
#define SOAP_TYPE___ec2__CancelBundleTask (456)
/* Operation wrapper: */
struct __ec2__CancelBundleTask
{
  struct ec2__CancelBundleTaskType *ec2__CancelBundleTask;          /* optional element of type ec2:CancelBundleTaskType */
};
#endif

#ifndef SOAP_TYPE___ec2__DescribeRegions
#define SOAP_TYPE___ec2__DescribeRegions (460)
/* Operation wrapper: */
struct __ec2__DescribeRegions
{
  struct ec2__DescribeRegionsType *ec2__DescribeRegions;    /* optional element of type ec2:DescribeRegionsType */
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Code
#define SOAP_TYPE_SOAP_ENV__Code (461)
/* SOAP Fault Code: */
struct SOAP_ENV__Code
{
  char *SOAP_ENV__Value;    /* optional element of type xsd:QName */
  struct SOAP_ENV__Code *SOAP_ENV__Subcode;         /* optional element of type SOAP-ENV:Code */
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Detail
#define SOAP_TYPE_SOAP_ENV__Detail (463)
/* SOAP-ENV:Detail */
struct SOAP_ENV__Detail
{
  int __type;       /* any type of element <fault> (defined below) */
  void *fault;      /* transient */
  char *__any;
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Reason
#define SOAP_TYPE_SOAP_ENV__Reason (466)
/* SOAP-ENV:Reason */
struct SOAP_ENV__Reason
{
  char *SOAP_ENV__Text;     /* optional element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_SOAP_ENV__Fault
#define SOAP_TYPE_SOAP_ENV__Fault (467)
/* SOAP Fault: */
struct SOAP_ENV__Fault
{
  char *faultcode;          /* optional element of type xsd:QName */
  char *faultstring;        /* optional element of type xsd:string */
  char *faultactor;         /* optional element of type xsd:string */
  struct SOAP_ENV__Detail *detail;          /* optional element of type SOAP-ENV:Detail */
  struct SOAP_ENV__Code *SOAP_ENV__Code;    /* optional element of type SOAP-ENV:Code */
  struct SOAP_ENV__Reason *SOAP_ENV__Reason;        /* optional element of type SOAP-ENV:Reason */
  char *SOAP_ENV__Node;     /* optional element of type xsd:string */
  char *SOAP_ENV__Role;     /* optional element of type xsd:string */
  struct SOAP_ENV__Detail *SOAP_ENV__Detail;        /* optional element of type SOAP-ENV:Detail */
};
#endif // ifndef SOAP_TYPE_SOAP_ENV__Fault

/******************************************************************************\
*                                                                            *
* Typedefs                                                                   *
*                                                                            *
\******************************************************************************/

#ifndef SOAP_TYPE__QName
#define SOAP_TYPE__QName (5)
typedef char *_QName;
#endif

#ifndef SOAP_TYPE__XML
#define SOAP_TYPE__XML (6)
typedef char *_XML;
#endif


/******************************************************************************\
*                                                                            *
* Externals                                                                  *
*                                                                            *
\******************************************************************************/


/******************************************************************************\
*                                                                            *
* Client-Side Call Stubs                                                     *
*                                                                            *
\******************************************************************************/


SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__RegisterImage(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__RegisterImageType *ec2__RegisterImage,
  struct ec2__RegisterImageResponseType *ec2__RegisterImageResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DeregisterImage(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DeregisterImageType *ec2__DeregisterImage,
  struct ec2__DeregisterImageResponseType *ec2__DeregisterImageResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__CreateKeyPair(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__CreateKeyPairType *ec2__CreateKeyPair,
  struct ec2__CreateKeyPairResponseType *ec2__CreateKeyPairResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeKeyPairs(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeKeyPairsType *ec2__DescribeKeyPairs,
  struct ec2__DescribeKeyPairsResponseType *ec2__DescribeKeyPairsResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DeleteKeyPair(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DeleteKeyPairType *ec2__DeleteKeyPair,
  struct ec2__DeleteKeyPairResponseType *ec2__DeleteKeyPairResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__RunInstances(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__RunInstancesType *ec2__RunInstances,
  struct ec2__RunInstancesResponseType *ec2__RunInstancesResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__GetConsoleOutput(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__GetConsoleOutputType *ec2__GetConsoleOutput,
  struct ec2__GetConsoleOutputResponseType *ec2__GetConsoleOutputResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__TerminateInstances(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__TerminateInstancesType *ec2__TerminateInstances,
  struct ec2__TerminateInstancesResponseType *ec2__TerminateInstancesResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__RebootInstances(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__RebootInstancesType *ec2__RebootInstances,
  struct ec2__RebootInstancesResponseType *ec2__RebootInstancesResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeInstances(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeInstancesType *ec2__DescribeInstances,
  struct ec2__DescribeInstancesResponseType *ec2__DescribeInstancesResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeImages(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeImagesType *ec2__DescribeImages,
  struct ec2__DescribeImagesResponseType *ec2__DescribeImagesResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__CreateSecurityGroup(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__CreateSecurityGroupType *ec2__CreateSecurityGroup,
  struct ec2__CreateSecurityGroupResponseType *ec2__CreateSecurityGroupResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DeleteSecurityGroup(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DeleteSecurityGroupType *ec2__DeleteSecurityGroup,
  struct ec2__DeleteSecurityGroupResponseType *ec2__DeleteSecurityGroupResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeSecurityGroups(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeSecurityGroupsType *ec2__DescribeSecurityGroups,
  struct ec2__DescribeSecurityGroupsResponseType *
  ec2__DescribeSecurityGroupsResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__AuthorizeSecurityGroupIngress(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__AuthorizeSecurityGroupIngressType *
  ec2__AuthorizeSecurityGroupIngress,
  struct ec2__AuthorizeSecurityGroupIngressResponseType *
  ec2__AuthorizeSecurityGroupIngressResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__RevokeSecurityGroupIngress(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__RevokeSecurityGroupIngressType *ec2__RevokeSecurityGroupIngress,
  struct ec2__RevokeSecurityGroupIngressResponseType *
  ec2__RevokeSecurityGroupIngressResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__ModifyImageAttribute(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__ModifyImageAttributeType *ec2__ModifyImageAttribute,
  struct ec2__ModifyImageAttributeResponseType *
  ec2__ModifyImageAttributeResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__ResetImageAttribute(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__ResetImageAttributeType *ec2__ResetImageAttribute,
  struct ec2__ResetImageAttributeResponseType *ec2__ResetImageAttributeResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeImageAttribute(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeImageAttributeType *ec2__DescribeImageAttribute,
  struct ec2__DescribeImageAttributeResponseType *
  ec2__DescribeImageAttributeResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__ConfirmProductInstance(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__ConfirmProductInstanceType *ec2__ConfirmProductInstance,
  struct ec2__ConfirmProductInstanceResponseType *
  ec2__ConfirmProductInstanceResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeAvailabilityZones(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeAvailabilityZonesType *ec2__DescribeAvailabilityZones,
  struct ec2__DescribeAvailabilityZonesResponseType *
  ec2__DescribeAvailabilityZonesResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__AllocateAddress(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__AllocateAddressType *ec2__AllocateAddress,
  struct ec2__AllocateAddressResponseType *ec2__AllocateAddressResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__ReleaseAddress(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__ReleaseAddressType *ec2__ReleaseAddress,
  struct ec2__ReleaseAddressResponseType *ec2__ReleaseAddressResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeAddresses(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeAddressesType *ec2__DescribeAddresses,
  struct ec2__DescribeAddressesResponseType *ec2__DescribeAddressesResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__AssociateAddress(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__AssociateAddressType *ec2__AssociateAddress,
  struct ec2__AssociateAddressResponseType *ec2__AssociateAddressResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DisassociateAddress(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DisassociateAddressType *ec2__DisassociateAddress,
  struct ec2__DisassociateAddressResponseType *ec2__DisassociateAddressResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__CreateVolume(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__CreateVolumeType *ec2__CreateVolume,
  struct ec2__CreateVolumeResponseType *ec2__CreateVolumeResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DeleteVolume(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DeleteVolumeType *ec2__DeleteVolume,
  struct ec2__DeleteVolumeResponseType *ec2__DeleteVolumeResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeVolumes(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeVolumesType *ec2__DescribeVolumes,
  struct ec2__DescribeVolumesResponseType *ec2__DescribeVolumesResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__AttachVolume(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__AttachVolumeType *ec2__AttachVolume,
  struct ec2__AttachVolumeResponseType *ec2__AttachVolumeResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DetachVolume(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DetachVolumeType *ec2__DetachVolume,
  struct ec2__DetachVolumeResponseType *ec2__DetachVolumeResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__CreateSnapshot(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__CreateSnapshotType *ec2__CreateSnapshot,
  struct ec2__CreateSnapshotResponseType *ec2__CreateSnapshotResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DeleteSnapshot(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DeleteSnapshotType *ec2__DeleteSnapshot,
  struct ec2__DeleteSnapshotResponseType *ec2__DeleteSnapshotResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeSnapshots(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeSnapshotsType *ec2__DescribeSnapshots,
  struct ec2__DescribeSnapshotsResponseType *ec2__DescribeSnapshotsResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__BundleInstance(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__BundleInstanceType *ec2__BundleInstance,
  struct ec2__BundleInstanceResponseType *ec2__BundleInstanceResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeBundleTasks(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeBundleTasksType *ec2__DescribeBundleTasks,
  struct ec2__DescribeBundleTasksResponseType *ec2__DescribeBundleTasksResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__CancelBundleTask(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__CancelBundleTaskType *ec2__CancelBundleTask,
  struct ec2__CancelBundleTaskResponseType *ec2__CancelBundleTaskResponse);

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeRegions(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeRegionsType *ec2__DescribeRegions,
  struct ec2__DescribeRegionsResponseType *ec2__DescribeRegionsResponse);

#ifdef __cplusplus
}
#endif

#endif // ifndef soapStub_H

/* End of soapStub.h */
