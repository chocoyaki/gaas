/**
 * @file ec2wrapper.c
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



#include "soapH.h"
#include "AmazonEC2Binding.nsmap"
#include "plugin/wsseapi.h"
#include "plugin/smdevp.h"
#include "stdsoap2.h"

// #include "ec2wrapper.h"

#define CONST_5MIN 5 * 60

#ifdef __cplusplus
extern "C" {
#endif

// Allocates and initalizes a new soap structure with default settings for Eucalyptus i.e. with the SOAP_XML_CANONICAL | SOAP_XML_INDENT flags
struct soap *
soap_default_new() {
  return soap_new1(SOAP_XML_CANONICAL | SOAP_XML_INDENT);
}

// Reads a private key located at the specified path and returns it in the pointer parameter.
// Returns 0 - OK; !=0 - error.
int
PEM_read_PK(char *pem_path, EVP_PKEY **pk_out) {
  FILE *fd;
  *pk_out = NULL;
  printf("path = %s\n", pem_path);
  fd = fopen(pem_path, "r");
  if (fd != NULL) {
    *pk_out = PEM_read_PrivateKey(fd, NULL, NULL, NULL);
    printf("read-pk\n");
    fclose(fd);
    return 0;     // OK
  } else {
    printf("FD error\n");
  }
  return 1;   // signal an error
} /* PEM_read_PK */

// Reads an X509 certificate located at the specified path and returns it in the pointer parameter.
// Returns 0 - OK; !=0 - error
int
PEM_read_X509_cert(char *pem_path, X509 **cert_out) {
  FILE *fd;
  *cert_out = NULL;
  fd = fopen(pem_path, "r");
  if (fd != NULL) {
    *cert_out = PEM_read_X509(fd, NULL, NULL, NULL);
    fclose(fd);
    return 0;     // OK
  }
  return 1;   // signal an error
} /* PEM_read_X509_cert */

// Adds a security header to the soap message containing:
// 1. BinarySecurityToken — Contains the X.509 certificate in base64 encoded PEM format
// 2. Signature — Contains an XML digital signature created using the canonicalization, signature algorithm, and digest method (RSA-SHA1)
// 3. Timestamp — Requests to Amazon EC2 are valid within 5 minutes of this value to help prevent replay attacks
// Parameters:
// 1. the soap message
// 2. the private key
// 3. the X509 certificate
// Returns 0 - OK; !=0 - error
int
soap_wsse_add_security_header(struct soap *soap, EVP_PKEY *pk, X509 *cert) {
  return
    soap_wsse_add_BinarySecurityTokenX509(soap, "X509Token", cert)            // Add the binary security token
    || soap_wsse_add_KeyInfo_SecurityTokenReferenceX509(soap, "#X509Token")   // Add the security token reference
    || soap_wsse_sign_body(soap, SOAP_SMD_SIGN_RSA_SHA1, pk, 0)       // Sign the message body
    || soap_wsse_add_Timestamp(soap, "Time", CONST_5MIN);                             // Amazon uses 5 minutes expiration for timestamps
}

#ifdef __cplusplus
}
#endif

/* end of ec2wrapper.c */
