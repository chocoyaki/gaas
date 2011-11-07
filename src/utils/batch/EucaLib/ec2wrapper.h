/**
 * @file ec2wrapper.h
 *
 * @brief  Batch System implementation for Amazon EC2 compatible clouds
 *
 * @author Adrian Muresan (adrian.muresan@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENCE|
 */
/****************************************************************************/
/* Batch System implementation for Amazon EC2 compatible clouds             */
/* Contains a set of helper functions for communicating with Eucalyptus     */
/* (and Amazon EC2)                                                         */
/****************************************************************************/

#ifndef _EC2WRAPPER_H_
#define _EC2WRAPPER_H_

#include "soapH.h"
#include "plugin/smdevp.h"
#include "plugin/wsseapi.h"
#include "stdsoap2.h"

#ifdef __cplusplus
extern "C" {
#endif


// Allocates and initalizes a new soap structure with default settings for Eucalyptus i.e. with the SOAP_XML_CANONICAL | SOAP_XML_INDENT flags
extern struct soap *
soap_default_new();

// Reads a private key located at the specified path and returns it in the pointer parameter.
// Returns 0 - OK; !=0 - error.
extern int
PEM_read_PK(char *pem_path, EVP_PKEY **pk_out);

// Reads an X509 certificate located at the specified path and returns it in the pointer parameter.
// Returns 0 - OK; !=0 - error
extern int
PEM_read_X509_cert(char *pem_path, X509 **cert_out);

// Adds a security header to the soap message containing:
// 1. BinarySecurityToken — Contains the X.509 certificate in base64 encoded PEM format
// 2. Signature — Contains an XML digital signature created using the canonicalization, signature algorithm, and digest method (RSA-SHA1)
// 3. Timestamp — Requests to Amazon EC2 are valid within 5 minutes of this value to help prevent replay attacks
// Parameters:
// 1. the soap message
// 2. the private key
// 3. the X509 certificate
// Returns 0 - OK; !=0 - error
extern int
soap_wsse_add_security_header(struct soap *soap, EVP_PKEY *pk, X509 *cert);

#ifdef __cplusplus
}
#endif

#endif // _EC2WRAPPER_H_

/* end of ec2wrapper.h */
