/**
 * @file soapClient.c
 *
 * @brief  Batch System implementation for Amazon EC2 compatible clouds
 *
 * @author  Adrian Muresan (adrian.muresan@ens-lyon.fr)
 *
 * @section Licence
 *   |LICENSE|
 */
/****************************************************************************/
/* Batch System implementation for Amazon EC2 compatible clouds             */
/* Auto-generated file by using gSOAP that provides an implementation       */
/* of a SOAP client for the EC2 interface.                                  */
/****************************************************************************/


#if defined(__BORLANDC__)
#pragma option push -w-8060
#pragma option push -w-8004
#endif
#include "soapH.h"
#ifdef __cplusplus
extern "C" {
#endif

SOAP_SOURCE_STAMP("@(#) soapClient.c ver 2.7.16 2010-04-20 15:04:34 GMT")


SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__RegisterImage(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__RegisterImageType *ec2__RegisterImage,
  struct ec2__RegisterImageResponseType *ec2__RegisterImageResponse) {
  struct __ec2__RegisterImage soap_tmp___ec2__RegisterImage;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "RegisterImage";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__RegisterImage.ec2__RegisterImage = ec2__RegisterImage;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__RegisterImage(soap, &soap_tmp___ec2__RegisterImage);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__RegisterImage(soap, &soap_tmp___ec2__RegisterImage,
                                         "-ec2:RegisterImage", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__RegisterImage(soap, &soap_tmp___ec2__RegisterImage,
                                       "-ec2:RegisterImage", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__RegisterImageResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__RegisterImageResponseType(soap, ec2__RegisterImageResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__RegisterImageResponseType(soap, ec2__RegisterImageResponse,
                                          "ec2:RegisterImageResponse",
                                          "ec2:RegisterImageResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__RegisterImage */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DeregisterImage(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DeregisterImageType *ec2__DeregisterImage,
  struct ec2__DeregisterImageResponseType *ec2__DeregisterImageResponse) {
  struct __ec2__DeregisterImage soap_tmp___ec2__DeregisterImage;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DeregisterImage";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DeregisterImage.ec2__DeregisterImage = ec2__DeregisterImage;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DeregisterImage(soap, &soap_tmp___ec2__DeregisterImage);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DeregisterImage(soap,
                                           &soap_tmp___ec2__DeregisterImage,
                                           "-ec2:DeregisterImage", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DeregisterImage(soap, &soap_tmp___ec2__DeregisterImage,
                                         "-ec2:DeregisterImage", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DeregisterImageResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DeregisterImageResponseType(soap,
                                                ec2__DeregisterImageResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DeregisterImageResponseType(soap, ec2__DeregisterImageResponse,
                                            "ec2:DeregisterImageResponse",
                                            "ec2:DeregisterImageResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DeregisterImage */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__CreateKeyPair(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__CreateKeyPairType *ec2__CreateKeyPair,
  struct ec2__CreateKeyPairResponseType *ec2__CreateKeyPairResponse) {
  struct __ec2__CreateKeyPair soap_tmp___ec2__CreateKeyPair;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "CreateKeyPair";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__CreateKeyPair.ec2__CreateKeyPair = ec2__CreateKeyPair;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__CreateKeyPair(soap, &soap_tmp___ec2__CreateKeyPair);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__CreateKeyPair(soap, &soap_tmp___ec2__CreateKeyPair,
                                         "-ec2:CreateKeyPair", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__CreateKeyPair(soap, &soap_tmp___ec2__CreateKeyPair,
                                       "-ec2:CreateKeyPair", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__CreateKeyPairResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__CreateKeyPairResponseType(soap, ec2__CreateKeyPairResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__CreateKeyPairResponseType(soap, ec2__CreateKeyPairResponse,
                                          "ec2:CreateKeyPairResponse",
                                          "ec2:CreateKeyPairResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__CreateKeyPair */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeKeyPairs(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeKeyPairsType *ec2__DescribeKeyPairs,
  struct ec2__DescribeKeyPairsResponseType *ec2__DescribeKeyPairsResponse) {
  struct __ec2__DescribeKeyPairs soap_tmp___ec2__DescribeKeyPairs;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DescribeKeyPairs";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DescribeKeyPairs.ec2__DescribeKeyPairs =
    ec2__DescribeKeyPairs;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DescribeKeyPairs(soap,
                                         &soap_tmp___ec2__DescribeKeyPairs);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DescribeKeyPairs(soap,
                                            &soap_tmp___ec2__DescribeKeyPairs,
                                            "-ec2:DescribeKeyPairs", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DescribeKeyPairs(soap,
                                          &soap_tmp___ec2__DescribeKeyPairs,
                                          "-ec2:DescribeKeyPairs", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DescribeKeyPairsResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DescribeKeyPairsResponseType(soap,
                                                 ec2__DescribeKeyPairsResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DescribeKeyPairsResponseType(soap,
                                             ec2__DescribeKeyPairsResponse,
                                             "ec2:DescribeKeyPairsResponse",
                                             "ec2:DescribeKeyPairsResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DescribeKeyPairs */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DeleteKeyPair(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DeleteKeyPairType *ec2__DeleteKeyPair,
  struct ec2__DeleteKeyPairResponseType *ec2__DeleteKeyPairResponse) {
  struct __ec2__DeleteKeyPair soap_tmp___ec2__DeleteKeyPair;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DeleteKeyPair";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DeleteKeyPair.ec2__DeleteKeyPair = ec2__DeleteKeyPair;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DeleteKeyPair(soap, &soap_tmp___ec2__DeleteKeyPair);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DeleteKeyPair(soap, &soap_tmp___ec2__DeleteKeyPair,
                                         "-ec2:DeleteKeyPair", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DeleteKeyPair(soap, &soap_tmp___ec2__DeleteKeyPair,
                                       "-ec2:DeleteKeyPair", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DeleteKeyPairResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DeleteKeyPairResponseType(soap, ec2__DeleteKeyPairResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DeleteKeyPairResponseType(soap, ec2__DeleteKeyPairResponse,
                                          "ec2:DeleteKeyPairResponse",
                                          "ec2:DeleteKeyPairResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DeleteKeyPair */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__RunInstances(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__RunInstancesType *ec2__RunInstances,
  struct ec2__RunInstancesResponseType *ec2__RunInstancesResponse) {
  struct __ec2__RunInstances soap_tmp___ec2__RunInstances;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "RunInstances";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__RunInstances.ec2__RunInstances = ec2__RunInstances;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__RunInstances(soap, &soap_tmp___ec2__RunInstances);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__RunInstances(soap, &soap_tmp___ec2__RunInstances,
                                        "-ec2:RunInstances", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__RunInstances(soap, &soap_tmp___ec2__RunInstances,
                                      "-ec2:RunInstances", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__RunInstancesResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__RunInstancesResponseType(soap, ec2__RunInstancesResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__RunInstancesResponseType(soap, ec2__RunInstancesResponse,
                                         "ec2:RunInstancesResponse",
                                         "ec2:RunInstancesResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__RunInstances */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__GetConsoleOutput(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__GetConsoleOutputType *ec2__GetConsoleOutput,
  struct ec2__GetConsoleOutputResponseType *ec2__GetConsoleOutputResponse) {
  struct __ec2__GetConsoleOutput soap_tmp___ec2__GetConsoleOutput;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "GetConsoleOutput";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__GetConsoleOutput.ec2__GetConsoleOutput =
    ec2__GetConsoleOutput;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__GetConsoleOutput(soap,
                                         &soap_tmp___ec2__GetConsoleOutput);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__GetConsoleOutput(soap,
                                            &soap_tmp___ec2__GetConsoleOutput,
                                            "-ec2:GetConsoleOutput", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__GetConsoleOutput(soap,
                                          &soap_tmp___ec2__GetConsoleOutput,
                                          "-ec2:GetConsoleOutput", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__GetConsoleOutputResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__GetConsoleOutputResponseType(soap,
                                                 ec2__GetConsoleOutputResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__GetConsoleOutputResponseType(soap,
                                             ec2__GetConsoleOutputResponse,
                                             "ec2:GetConsoleOutputResponse",
                                             "ec2:GetConsoleOutputResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__GetConsoleOutput */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__TerminateInstances(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__TerminateInstancesType *ec2__TerminateInstances,
  struct ec2__TerminateInstancesResponseType *ec2__TerminateInstancesResponse)
{
  struct __ec2__TerminateInstances soap_tmp___ec2__TerminateInstances;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "TerminateInstances";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__TerminateInstances.ec2__TerminateInstances =
    ec2__TerminateInstances;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__TerminateInstances(soap,
                                           &soap_tmp___ec2__TerminateInstances);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__TerminateInstances(soap,
                                              &
                                              soap_tmp___ec2__TerminateInstances,
                                              "-ec2:TerminateInstances", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__TerminateInstances(soap,
                                            &soap_tmp___ec2__TerminateInstances,
                                            "-ec2:TerminateInstances", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__TerminateInstancesResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__TerminateInstancesResponseType(
    soap, ec2__TerminateInstancesResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__TerminateInstancesResponseType(
    soap, ec2__TerminateInstancesResponse, "ec2:TerminateInstancesResponse",
    "ec2:TerminateInstancesResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__TerminateInstances */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__RebootInstances(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__RebootInstancesType *ec2__RebootInstances,
  struct ec2__RebootInstancesResponseType *ec2__RebootInstancesResponse) {
  struct __ec2__RebootInstances soap_tmp___ec2__RebootInstances;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "RebootInstances";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__RebootInstances.ec2__RebootInstances = ec2__RebootInstances;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__RebootInstances(soap, &soap_tmp___ec2__RebootInstances);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__RebootInstances(soap,
                                           &soap_tmp___ec2__RebootInstances,
                                           "-ec2:RebootInstances", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__RebootInstances(soap, &soap_tmp___ec2__RebootInstances,
                                         "-ec2:RebootInstances", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__RebootInstancesResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__RebootInstancesResponseType(soap,
                                                ec2__RebootInstancesResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__RebootInstancesResponseType(soap, ec2__RebootInstancesResponse,
                                            "ec2:RebootInstancesResponse",
                                            "ec2:RebootInstancesResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__RebootInstances */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeInstances(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeInstancesType *ec2__DescribeInstances,
  struct ec2__DescribeInstancesResponseType *ec2__DescribeInstancesResponse) {
  struct __ec2__DescribeInstances soap_tmp___ec2__DescribeInstances;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DescribeInstances";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DescribeInstances.ec2__DescribeInstances =
    ec2__DescribeInstances;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DescribeInstances(soap,
                                          &soap_tmp___ec2__DescribeInstances);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DescribeInstances(soap,
                                             &soap_tmp___ec2__DescribeInstances,
                                             "-ec2:DescribeInstances", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DescribeInstances(soap,
                                           &soap_tmp___ec2__DescribeInstances,
                                           "-ec2:DescribeInstances", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DescribeInstancesResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DescribeInstancesResponseType(
    soap, ec2__DescribeInstancesResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DescribeInstancesResponseType(
    soap, ec2__DescribeInstancesResponse, "ec2:DescribeInstancesResponse",
    "ec2:DescribeInstancesResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DescribeInstances */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeImages(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeImagesType *ec2__DescribeImages,
  struct ec2__DescribeImagesResponseType *ec2__DescribeImagesResponse) {
  struct __ec2__DescribeImages soap_tmp___ec2__DescribeImages;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DescribeImages";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DescribeImages.ec2__DescribeImages = ec2__DescribeImages;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DescribeImages(soap, &soap_tmp___ec2__DescribeImages);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DescribeImages(soap, &soap_tmp___ec2__DescribeImages,
                                          "-ec2:DescribeImages", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DescribeImages(soap, &soap_tmp___ec2__DescribeImages,
                                        "-ec2:DescribeImages", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DescribeImagesResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DescribeImagesResponseType(soap,
                                               ec2__DescribeImagesResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DescribeImagesResponseType(soap, ec2__DescribeImagesResponse,
                                           "ec2:DescribeImagesResponse",
                                           "ec2:DescribeImagesResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DescribeImages */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__CreateSecurityGroup(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__CreateSecurityGroupType *ec2__CreateSecurityGroup,
  struct ec2__CreateSecurityGroupResponseType *ec2__CreateSecurityGroupResponse)
{
  struct __ec2__CreateSecurityGroup soap_tmp___ec2__CreateSecurityGroup;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "CreateSecurityGroup";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__CreateSecurityGroup.ec2__CreateSecurityGroup =
    ec2__CreateSecurityGroup;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__CreateSecurityGroup(
    soap, &soap_tmp___ec2__CreateSecurityGroup);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__CreateSecurityGroup(soap,
                                               &
                                               soap_tmp___ec2__CreateSecurityGroup,
                                               "-ec2:CreateSecurityGroup", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__CreateSecurityGroup(soap,
                                             &
                                             soap_tmp___ec2__CreateSecurityGroup,
                                             "-ec2:CreateSecurityGroup", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__CreateSecurityGroupResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__CreateSecurityGroupResponseType(
    soap, ec2__CreateSecurityGroupResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__CreateSecurityGroupResponseType(
    soap, ec2__CreateSecurityGroupResponse, "ec2:CreateSecurityGroupResponse",
    "ec2:CreateSecurityGroupResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__CreateSecurityGroup */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DeleteSecurityGroup(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DeleteSecurityGroupType *ec2__DeleteSecurityGroup,
  struct ec2__DeleteSecurityGroupResponseType *ec2__DeleteSecurityGroupResponse)
{
  struct __ec2__DeleteSecurityGroup soap_tmp___ec2__DeleteSecurityGroup;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DeleteSecurityGroup";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DeleteSecurityGroup.ec2__DeleteSecurityGroup =
    ec2__DeleteSecurityGroup;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DeleteSecurityGroup(
    soap, &soap_tmp___ec2__DeleteSecurityGroup);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DeleteSecurityGroup(soap,
                                               &
                                               soap_tmp___ec2__DeleteSecurityGroup,
                                               "-ec2:DeleteSecurityGroup", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DeleteSecurityGroup(soap,
                                             &
                                             soap_tmp___ec2__DeleteSecurityGroup,
                                             "-ec2:DeleteSecurityGroup", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DeleteSecurityGroupResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DeleteSecurityGroupResponseType(
    soap, ec2__DeleteSecurityGroupResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DeleteSecurityGroupResponseType(
    soap, ec2__DeleteSecurityGroupResponse, "ec2:DeleteSecurityGroupResponse",
    "ec2:DeleteSecurityGroupResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DeleteSecurityGroup */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeSecurityGroups(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeSecurityGroupsType *ec2__DescribeSecurityGroups,
  struct ec2__DescribeSecurityGroupsResponseType *
  ec2__DescribeSecurityGroupsResponse) {
  struct __ec2__DescribeSecurityGroups soap_tmp___ec2__DescribeSecurityGroups;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DescribeSecurityGroups";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DescribeSecurityGroups.ec2__DescribeSecurityGroups =
    ec2__DescribeSecurityGroups;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DescribeSecurityGroups(
    soap, &soap_tmp___ec2__DescribeSecurityGroups);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DescribeSecurityGroups(soap,
                                                  &
                                                  soap_tmp___ec2__DescribeSecurityGroups,
                                                  "-ec2:DescribeSecurityGroups",
                                                  NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DescribeSecurityGroups(soap,
                                                &
                                                soap_tmp___ec2__DescribeSecurityGroups,
                                                "-ec2:DescribeSecurityGroups",
                                                NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DescribeSecurityGroupsResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DescribeSecurityGroupsResponseType(
    soap, ec2__DescribeSecurityGroupsResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DescribeSecurityGroupsResponseType(
    soap, ec2__DescribeSecurityGroupsResponse,
    "ec2:DescribeSecurityGroupsResponse",
    "ec2:DescribeSecurityGroupsResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DescribeSecurityGroups */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__AuthorizeSecurityGroupIngress(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__AuthorizeSecurityGroupIngressType *
  ec2__AuthorizeSecurityGroupIngress,
  struct ec2__AuthorizeSecurityGroupIngressResponseType *
  ec2__AuthorizeSecurityGroupIngressResponse) {
  struct __ec2__AuthorizeSecurityGroupIngress
    soap_tmp___ec2__AuthorizeSecurityGroupIngress;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "AuthorizeSecurityGroupIngress";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__AuthorizeSecurityGroupIngress.
  ec2__AuthorizeSecurityGroupIngress = ec2__AuthorizeSecurityGroupIngress;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__AuthorizeSecurityGroupIngress(
    soap, &soap_tmp___ec2__AuthorizeSecurityGroupIngress);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__AuthorizeSecurityGroupIngress(soap,
                                                         &
                                                         soap_tmp___ec2__AuthorizeSecurityGroupIngress,
                                                         "-ec2:AuthorizeSecurityGroupIngress",
                                                         NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__AuthorizeSecurityGroupIngress(soap,
                                                       &
                                                       soap_tmp___ec2__AuthorizeSecurityGroupIngress,
                                                       "-ec2:AuthorizeSecurityGroupIngress",
                                                       NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__AuthorizeSecurityGroupIngressResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__AuthorizeSecurityGroupIngressResponseType(
    soap, ec2__AuthorizeSecurityGroupIngressResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__AuthorizeSecurityGroupIngressResponseType(
    soap, ec2__AuthorizeSecurityGroupIngressResponse,
    "ec2:AuthorizeSecurityGroupIngressResponse",
    "ec2:AuthorizeSecurityGroupIngressResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__AuthorizeSecurityGroupIngress */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__RevokeSecurityGroupIngress(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__RevokeSecurityGroupIngressType *ec2__RevokeSecurityGroupIngress,
  struct ec2__RevokeSecurityGroupIngressResponseType *
  ec2__RevokeSecurityGroupIngressResponse) {
  struct __ec2__RevokeSecurityGroupIngress
    soap_tmp___ec2__RevokeSecurityGroupIngress;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "RevokeSecurityGroupIngress";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__RevokeSecurityGroupIngress.ec2__RevokeSecurityGroupIngress =
    ec2__RevokeSecurityGroupIngress;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__RevokeSecurityGroupIngress(
    soap, &soap_tmp___ec2__RevokeSecurityGroupIngress);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__RevokeSecurityGroupIngress(soap,
                                                      &
                                                      soap_tmp___ec2__RevokeSecurityGroupIngress,
                                                      "-ec2:RevokeSecurityGroupIngress",
                                                      NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__RevokeSecurityGroupIngress(soap,
                                                    &
                                                    soap_tmp___ec2__RevokeSecurityGroupIngress,
                                                    "-ec2:RevokeSecurityGroupIngress",
                                                    NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__RevokeSecurityGroupIngressResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__RevokeSecurityGroupIngressResponseType(
    soap, ec2__RevokeSecurityGroupIngressResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__RevokeSecurityGroupIngressResponseType(
    soap, ec2__RevokeSecurityGroupIngressResponse,
    "ec2:RevokeSecurityGroupIngressResponse",
    "ec2:RevokeSecurityGroupIngressResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__RevokeSecurityGroupIngress */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__ModifyImageAttribute(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__ModifyImageAttributeType *ec2__ModifyImageAttribute,
  struct ec2__ModifyImageAttributeResponseType *
  ec2__ModifyImageAttributeResponse) {
  struct __ec2__ModifyImageAttribute soap_tmp___ec2__ModifyImageAttribute;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "ModifyImageAttribute";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__ModifyImageAttribute.ec2__ModifyImageAttribute =
    ec2__ModifyImageAttribute;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__ModifyImageAttribute(
    soap, &soap_tmp___ec2__ModifyImageAttribute);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__ModifyImageAttribute(soap,
                                                &
                                                soap_tmp___ec2__ModifyImageAttribute,
                                                "-ec2:ModifyImageAttribute",
                                                NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__ModifyImageAttribute(soap,
                                              &
                                              soap_tmp___ec2__ModifyImageAttribute,
                                              "-ec2:ModifyImageAttribute", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__ModifyImageAttributeResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__ModifyImageAttributeResponseType(
    soap, ec2__ModifyImageAttributeResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__ModifyImageAttributeResponseType(
    soap, ec2__ModifyImageAttributeResponse, "ec2:ModifyImageAttributeResponse",
    "ec2:ModifyImageAttributeResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__ModifyImageAttribute */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__ResetImageAttribute(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__ResetImageAttributeType *ec2__ResetImageAttribute,
  struct ec2__ResetImageAttributeResponseType *ec2__ResetImageAttributeResponse)
{
  struct __ec2__ResetImageAttribute soap_tmp___ec2__ResetImageAttribute;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "ResetImageAttribute";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__ResetImageAttribute.ec2__ResetImageAttribute =
    ec2__ResetImageAttribute;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__ResetImageAttribute(
    soap, &soap_tmp___ec2__ResetImageAttribute);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__ResetImageAttribute(soap,
                                               &
                                               soap_tmp___ec2__ResetImageAttribute,
                                               "-ec2:ResetImageAttribute", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__ResetImageAttribute(soap,
                                             &
                                             soap_tmp___ec2__ResetImageAttribute,
                                             "-ec2:ResetImageAttribute", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__ResetImageAttributeResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__ResetImageAttributeResponseType(
    soap, ec2__ResetImageAttributeResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__ResetImageAttributeResponseType(
    soap, ec2__ResetImageAttributeResponse, "ec2:ResetImageAttributeResponse",
    "ec2:ResetImageAttributeResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__ResetImageAttribute */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeImageAttribute(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeImageAttributeType *ec2__DescribeImageAttribute,
  struct ec2__DescribeImageAttributeResponseType *
  ec2__DescribeImageAttributeResponse) {
  struct __ec2__DescribeImageAttribute soap_tmp___ec2__DescribeImageAttribute;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DescribeImageAttribute";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DescribeImageAttribute.ec2__DescribeImageAttribute =
    ec2__DescribeImageAttribute;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DescribeImageAttribute(
    soap, &soap_tmp___ec2__DescribeImageAttribute);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DescribeImageAttribute(soap,
                                                  &
                                                  soap_tmp___ec2__DescribeImageAttribute,
                                                  "-ec2:DescribeImageAttribute",
                                                  NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DescribeImageAttribute(soap,
                                                &
                                                soap_tmp___ec2__DescribeImageAttribute,
                                                "-ec2:DescribeImageAttribute",
                                                NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DescribeImageAttributeResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DescribeImageAttributeResponseType(
    soap, ec2__DescribeImageAttributeResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DescribeImageAttributeResponseType(
    soap, ec2__DescribeImageAttributeResponse,
    "ec2:DescribeImageAttributeResponse",
    "ec2:DescribeImageAttributeResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DescribeImageAttribute */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__ConfirmProductInstance(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__ConfirmProductInstanceType *ec2__ConfirmProductInstance,
  struct ec2__ConfirmProductInstanceResponseType *
  ec2__ConfirmProductInstanceResponse) {
  struct __ec2__ConfirmProductInstance soap_tmp___ec2__ConfirmProductInstance;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "ConfirmProductInstance";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__ConfirmProductInstance.ec2__ConfirmProductInstance =
    ec2__ConfirmProductInstance;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__ConfirmProductInstance(
    soap, &soap_tmp___ec2__ConfirmProductInstance);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__ConfirmProductInstance(soap,
                                                  &
                                                  soap_tmp___ec2__ConfirmProductInstance,
                                                  "-ec2:ConfirmProductInstance",
                                                  NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__ConfirmProductInstance(soap,
                                                &
                                                soap_tmp___ec2__ConfirmProductInstance,
                                                "-ec2:ConfirmProductInstance",
                                                NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__ConfirmProductInstanceResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__ConfirmProductInstanceResponseType(
    soap, ec2__ConfirmProductInstanceResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__ConfirmProductInstanceResponseType(
    soap, ec2__ConfirmProductInstanceResponse,
    "ec2:ConfirmProductInstanceResponse",
    "ec2:ConfirmProductInstanceResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__ConfirmProductInstance */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeAvailabilityZones(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeAvailabilityZonesType *ec2__DescribeAvailabilityZones,
  struct ec2__DescribeAvailabilityZonesResponseType *
  ec2__DescribeAvailabilityZonesResponse) {
  struct __ec2__DescribeAvailabilityZones
    soap_tmp___ec2__DescribeAvailabilityZones;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DescribeAvailabilityZones";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DescribeAvailabilityZones.ec2__DescribeAvailabilityZones =
    ec2__DescribeAvailabilityZones;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DescribeAvailabilityZones(
    soap, &soap_tmp___ec2__DescribeAvailabilityZones);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DescribeAvailabilityZones(soap,
                                                     &
                                                     soap_tmp___ec2__DescribeAvailabilityZones,
                                                     "-ec2:DescribeAvailabilityZones",
                                                     NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DescribeAvailabilityZones(soap,
                                                   &
                                                   soap_tmp___ec2__DescribeAvailabilityZones,
                                                   "-ec2:DescribeAvailabilityZones",
                                                   NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DescribeAvailabilityZonesResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DescribeAvailabilityZonesResponseType(
    soap, ec2__DescribeAvailabilityZonesResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DescribeAvailabilityZonesResponseType(
    soap, ec2__DescribeAvailabilityZonesResponse,
    "ec2:DescribeAvailabilityZonesResponse",
    "ec2:DescribeAvailabilityZonesResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DescribeAvailabilityZones */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__AllocateAddress(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__AllocateAddressType *ec2__AllocateAddress,
  struct ec2__AllocateAddressResponseType *ec2__AllocateAddressResponse) {
  struct __ec2__AllocateAddress soap_tmp___ec2__AllocateAddress;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "AllocateAddress";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__AllocateAddress.ec2__AllocateAddress = ec2__AllocateAddress;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__AllocateAddress(soap, &soap_tmp___ec2__AllocateAddress);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__AllocateAddress(soap,
                                           &soap_tmp___ec2__AllocateAddress,
                                           "-ec2:AllocateAddress", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__AllocateAddress(soap, &soap_tmp___ec2__AllocateAddress,
                                         "-ec2:AllocateAddress", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__AllocateAddressResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__AllocateAddressResponseType(soap,
                                                ec2__AllocateAddressResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__AllocateAddressResponseType(soap, ec2__AllocateAddressResponse,
                                            "ec2:AllocateAddressResponse",
                                            "ec2:AllocateAddressResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__AllocateAddress */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__ReleaseAddress(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__ReleaseAddressType *ec2__ReleaseAddress,
  struct ec2__ReleaseAddressResponseType *ec2__ReleaseAddressResponse) {
  struct __ec2__ReleaseAddress soap_tmp___ec2__ReleaseAddress;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "ReleaseAddress";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__ReleaseAddress.ec2__ReleaseAddress = ec2__ReleaseAddress;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__ReleaseAddress(soap, &soap_tmp___ec2__ReleaseAddress);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__ReleaseAddress(soap, &soap_tmp___ec2__ReleaseAddress,
                                          "-ec2:ReleaseAddress", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__ReleaseAddress(soap, &soap_tmp___ec2__ReleaseAddress,
                                        "-ec2:ReleaseAddress", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__ReleaseAddressResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__ReleaseAddressResponseType(soap,
                                               ec2__ReleaseAddressResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__ReleaseAddressResponseType(soap, ec2__ReleaseAddressResponse,
                                           "ec2:ReleaseAddressResponse",
                                           "ec2:ReleaseAddressResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__ReleaseAddress */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeAddresses(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeAddressesType *ec2__DescribeAddresses,
  struct ec2__DescribeAddressesResponseType *ec2__DescribeAddressesResponse) {
  struct __ec2__DescribeAddresses soap_tmp___ec2__DescribeAddresses;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DescribeAddresses";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DescribeAddresses.ec2__DescribeAddresses =
    ec2__DescribeAddresses;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DescribeAddresses(soap,
                                          &soap_tmp___ec2__DescribeAddresses);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DescribeAddresses(soap,
                                             &soap_tmp___ec2__DescribeAddresses,
                                             "-ec2:DescribeAddresses", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DescribeAddresses(soap,
                                           &soap_tmp___ec2__DescribeAddresses,
                                           "-ec2:DescribeAddresses", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DescribeAddressesResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DescribeAddressesResponseType(
    soap, ec2__DescribeAddressesResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DescribeAddressesResponseType(
    soap, ec2__DescribeAddressesResponse, "ec2:DescribeAddressesResponse",
    "ec2:DescribeAddressesResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DescribeAddresses */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__AssociateAddress(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__AssociateAddressType *ec2__AssociateAddress,
  struct ec2__AssociateAddressResponseType *ec2__AssociateAddressResponse) {
  struct __ec2__AssociateAddress soap_tmp___ec2__AssociateAddress;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "AssociateAddress";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__AssociateAddress.ec2__AssociateAddress =
    ec2__AssociateAddress;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__AssociateAddress(soap,
                                         &soap_tmp___ec2__AssociateAddress);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__AssociateAddress(soap,
                                            &soap_tmp___ec2__AssociateAddress,
                                            "-ec2:AssociateAddress", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__AssociateAddress(soap,
                                          &soap_tmp___ec2__AssociateAddress,
                                          "-ec2:AssociateAddress", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__AssociateAddressResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__AssociateAddressResponseType(soap,
                                                 ec2__AssociateAddressResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__AssociateAddressResponseType(soap,
                                             ec2__AssociateAddressResponse,
                                             "ec2:AssociateAddressResponse",
                                             "ec2:AssociateAddressResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__AssociateAddress */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DisassociateAddress(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DisassociateAddressType *ec2__DisassociateAddress,
  struct ec2__DisassociateAddressResponseType *ec2__DisassociateAddressResponse)
{
  struct __ec2__DisassociateAddress soap_tmp___ec2__DisassociateAddress;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DisassociateAddress";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DisassociateAddress.ec2__DisassociateAddress =
    ec2__DisassociateAddress;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DisassociateAddress(
    soap, &soap_tmp___ec2__DisassociateAddress);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DisassociateAddress(soap,
                                               &
                                               soap_tmp___ec2__DisassociateAddress,
                                               "-ec2:DisassociateAddress", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DisassociateAddress(soap,
                                             &
                                             soap_tmp___ec2__DisassociateAddress,
                                             "-ec2:DisassociateAddress", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DisassociateAddressResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DisassociateAddressResponseType(
    soap, ec2__DisassociateAddressResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DisassociateAddressResponseType(
    soap, ec2__DisassociateAddressResponse, "ec2:DisassociateAddressResponse",
    "ec2:DisassociateAddressResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DisassociateAddress */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__CreateVolume(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__CreateVolumeType *ec2__CreateVolume,
  struct ec2__CreateVolumeResponseType *ec2__CreateVolumeResponse) {
  struct __ec2__CreateVolume soap_tmp___ec2__CreateVolume;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "CreateVolume";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__CreateVolume.ec2__CreateVolume = ec2__CreateVolume;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__CreateVolume(soap, &soap_tmp___ec2__CreateVolume);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__CreateVolume(soap, &soap_tmp___ec2__CreateVolume,
                                        "-ec2:CreateVolume", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__CreateVolume(soap, &soap_tmp___ec2__CreateVolume,
                                      "-ec2:CreateVolume", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__CreateVolumeResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__CreateVolumeResponseType(soap, ec2__CreateVolumeResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__CreateVolumeResponseType(soap, ec2__CreateVolumeResponse,
                                         "ec2:CreateVolumeResponse",
                                         "ec2:CreateVolumeResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__CreateVolume */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DeleteVolume(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DeleteVolumeType *ec2__DeleteVolume,
  struct ec2__DeleteVolumeResponseType *ec2__DeleteVolumeResponse) {
  struct __ec2__DeleteVolume soap_tmp___ec2__DeleteVolume;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DeleteVolume";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DeleteVolume.ec2__DeleteVolume = ec2__DeleteVolume;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DeleteVolume(soap, &soap_tmp___ec2__DeleteVolume);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DeleteVolume(soap, &soap_tmp___ec2__DeleteVolume,
                                        "-ec2:DeleteVolume", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DeleteVolume(soap, &soap_tmp___ec2__DeleteVolume,
                                      "-ec2:DeleteVolume", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DeleteVolumeResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DeleteVolumeResponseType(soap, ec2__DeleteVolumeResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DeleteVolumeResponseType(soap, ec2__DeleteVolumeResponse,
                                         "ec2:DeleteVolumeResponse",
                                         "ec2:DeleteVolumeResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DeleteVolume */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeVolumes(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeVolumesType *ec2__DescribeVolumes,
  struct ec2__DescribeVolumesResponseType *ec2__DescribeVolumesResponse) {
  struct __ec2__DescribeVolumes soap_tmp___ec2__DescribeVolumes;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DescribeVolumes";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DescribeVolumes.ec2__DescribeVolumes = ec2__DescribeVolumes;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DescribeVolumes(soap, &soap_tmp___ec2__DescribeVolumes);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DescribeVolumes(soap,
                                           &soap_tmp___ec2__DescribeVolumes,
                                           "-ec2:DescribeVolumes", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DescribeVolumes(soap, &soap_tmp___ec2__DescribeVolumes,
                                         "-ec2:DescribeVolumes", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DescribeVolumesResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DescribeVolumesResponseType(soap,
                                                ec2__DescribeVolumesResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DescribeVolumesResponseType(soap, ec2__DescribeVolumesResponse,
                                            "ec2:DescribeVolumesResponse",
                                            "ec2:DescribeVolumesResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DescribeVolumes */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__AttachVolume(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__AttachVolumeType *ec2__AttachVolume,
  struct ec2__AttachVolumeResponseType *ec2__AttachVolumeResponse) {
  struct __ec2__AttachVolume soap_tmp___ec2__AttachVolume;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "AttachVolume";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__AttachVolume.ec2__AttachVolume = ec2__AttachVolume;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__AttachVolume(soap, &soap_tmp___ec2__AttachVolume);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__AttachVolume(soap, &soap_tmp___ec2__AttachVolume,
                                        "-ec2:AttachVolume", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__AttachVolume(soap, &soap_tmp___ec2__AttachVolume,
                                      "-ec2:AttachVolume", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__AttachVolumeResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__AttachVolumeResponseType(soap, ec2__AttachVolumeResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__AttachVolumeResponseType(soap, ec2__AttachVolumeResponse,
                                         "ec2:AttachVolumeResponse",
                                         "ec2:AttachVolumeResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__AttachVolume */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DetachVolume(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DetachVolumeType *ec2__DetachVolume,
  struct ec2__DetachVolumeResponseType *ec2__DetachVolumeResponse) {
  struct __ec2__DetachVolume soap_tmp___ec2__DetachVolume;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DetachVolume";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DetachVolume.ec2__DetachVolume = ec2__DetachVolume;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DetachVolume(soap, &soap_tmp___ec2__DetachVolume);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DetachVolume(soap, &soap_tmp___ec2__DetachVolume,
                                        "-ec2:DetachVolume", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DetachVolume(soap, &soap_tmp___ec2__DetachVolume,
                                      "-ec2:DetachVolume", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DetachVolumeResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DetachVolumeResponseType(soap, ec2__DetachVolumeResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DetachVolumeResponseType(soap, ec2__DetachVolumeResponse,
                                         "ec2:DetachVolumeResponse",
                                         "ec2:DetachVolumeResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DetachVolume */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__CreateSnapshot(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__CreateSnapshotType *ec2__CreateSnapshot,
  struct ec2__CreateSnapshotResponseType *ec2__CreateSnapshotResponse) {
  struct __ec2__CreateSnapshot soap_tmp___ec2__CreateSnapshot;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "CreateSnapshot";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__CreateSnapshot.ec2__CreateSnapshot = ec2__CreateSnapshot;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__CreateSnapshot(soap, &soap_tmp___ec2__CreateSnapshot);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__CreateSnapshot(soap, &soap_tmp___ec2__CreateSnapshot,
                                          "-ec2:CreateSnapshot", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__CreateSnapshot(soap, &soap_tmp___ec2__CreateSnapshot,
                                        "-ec2:CreateSnapshot", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__CreateSnapshotResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__CreateSnapshotResponseType(soap,
                                               ec2__CreateSnapshotResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__CreateSnapshotResponseType(soap, ec2__CreateSnapshotResponse,
                                           "ec2:CreateSnapshotResponse",
                                           "ec2:CreateSnapshotResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__CreateSnapshot */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DeleteSnapshot(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DeleteSnapshotType *ec2__DeleteSnapshot,
  struct ec2__DeleteSnapshotResponseType *ec2__DeleteSnapshotResponse) {
  struct __ec2__DeleteSnapshot soap_tmp___ec2__DeleteSnapshot;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DeleteSnapshot";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DeleteSnapshot.ec2__DeleteSnapshot = ec2__DeleteSnapshot;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DeleteSnapshot(soap, &soap_tmp___ec2__DeleteSnapshot);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DeleteSnapshot(soap, &soap_tmp___ec2__DeleteSnapshot,
                                          "-ec2:DeleteSnapshot", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DeleteSnapshot(soap, &soap_tmp___ec2__DeleteSnapshot,
                                        "-ec2:DeleteSnapshot", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DeleteSnapshotResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DeleteSnapshotResponseType(soap,
                                               ec2__DeleteSnapshotResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DeleteSnapshotResponseType(soap, ec2__DeleteSnapshotResponse,
                                           "ec2:DeleteSnapshotResponse",
                                           "ec2:DeleteSnapshotResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DeleteSnapshot */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeSnapshots(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeSnapshotsType *ec2__DescribeSnapshots,
  struct ec2__DescribeSnapshotsResponseType *ec2__DescribeSnapshotsResponse) {
  struct __ec2__DescribeSnapshots soap_tmp___ec2__DescribeSnapshots;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DescribeSnapshots";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DescribeSnapshots.ec2__DescribeSnapshots =
    ec2__DescribeSnapshots;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DescribeSnapshots(soap,
                                          &soap_tmp___ec2__DescribeSnapshots);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DescribeSnapshots(soap,
                                             &soap_tmp___ec2__DescribeSnapshots,
                                             "-ec2:DescribeSnapshots", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DescribeSnapshots(soap,
                                           &soap_tmp___ec2__DescribeSnapshots,
                                           "-ec2:DescribeSnapshots", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DescribeSnapshotsResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DescribeSnapshotsResponseType(
    soap, ec2__DescribeSnapshotsResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DescribeSnapshotsResponseType(
    soap, ec2__DescribeSnapshotsResponse, "ec2:DescribeSnapshotsResponse",
    "ec2:DescribeSnapshotsResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DescribeSnapshots */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__BundleInstance(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__BundleInstanceType *ec2__BundleInstance,
  struct ec2__BundleInstanceResponseType *ec2__BundleInstanceResponse) {
  struct __ec2__BundleInstance soap_tmp___ec2__BundleInstance;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "BundleInstance";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__BundleInstance.ec2__BundleInstance = ec2__BundleInstance;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__BundleInstance(soap, &soap_tmp___ec2__BundleInstance);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__BundleInstance(soap, &soap_tmp___ec2__BundleInstance,
                                          "-ec2:BundleInstance", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__BundleInstance(soap, &soap_tmp___ec2__BundleInstance,
                                        "-ec2:BundleInstance", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__BundleInstanceResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__BundleInstanceResponseType(soap,
                                               ec2__BundleInstanceResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__BundleInstanceResponseType(soap, ec2__BundleInstanceResponse,
                                           "ec2:BundleInstanceResponse",
                                           "ec2:BundleInstanceResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__BundleInstance */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeBundleTasks(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeBundleTasksType *ec2__DescribeBundleTasks,
  struct ec2__DescribeBundleTasksResponseType *ec2__DescribeBundleTasksResponse)
{
  struct __ec2__DescribeBundleTasks soap_tmp___ec2__DescribeBundleTasks;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DescribeBundleTasks";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DescribeBundleTasks.ec2__DescribeBundleTasks =
    ec2__DescribeBundleTasks;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DescribeBundleTasks(
    soap, &soap_tmp___ec2__DescribeBundleTasks);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DescribeBundleTasks(soap,
                                               &
                                               soap_tmp___ec2__DescribeBundleTasks,
                                               "-ec2:DescribeBundleTasks", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DescribeBundleTasks(soap,
                                             &
                                             soap_tmp___ec2__DescribeBundleTasks,
                                             "-ec2:DescribeBundleTasks", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DescribeBundleTasksResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DescribeBundleTasksResponseType(
    soap, ec2__DescribeBundleTasksResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DescribeBundleTasksResponseType(
    soap, ec2__DescribeBundleTasksResponse, "ec2:DescribeBundleTasksResponse",
    "ec2:DescribeBundleTasksResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DescribeBundleTasks */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__CancelBundleTask(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__CancelBundleTaskType *ec2__CancelBundleTask,
  struct ec2__CancelBundleTaskResponseType *ec2__CancelBundleTaskResponse) {
  struct __ec2__CancelBundleTask soap_tmp___ec2__CancelBundleTask;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "CancelBundleTask";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__CancelBundleTask.ec2__CancelBundleTask =
    ec2__CancelBundleTask;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__CancelBundleTask(soap,
                                         &soap_tmp___ec2__CancelBundleTask);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__CancelBundleTask(soap,
                                            &soap_tmp___ec2__CancelBundleTask,
                                            "-ec2:CancelBundleTask", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__CancelBundleTask(soap,
                                          &soap_tmp___ec2__CancelBundleTask,
                                          "-ec2:CancelBundleTask", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__CancelBundleTaskResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__CancelBundleTaskResponseType(soap,
                                                 ec2__CancelBundleTaskResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__CancelBundleTaskResponseType(soap,
                                             ec2__CancelBundleTaskResponse,
                                             "ec2:CancelBundleTaskResponse",
                                             "ec2:CancelBundleTaskResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__CancelBundleTask */

SOAP_FMAC5 int SOAP_FMAC6
soap_call___ec2__DescribeRegions(
  struct soap *soap, const char *soap_endpoint, const char *soap_action,
  struct ec2__DescribeRegionsType *ec2__DescribeRegions,
  struct ec2__DescribeRegionsResponseType *ec2__DescribeRegionsResponse) {
  struct __ec2__DescribeRegions soap_tmp___ec2__DescribeRegions;
  if (!soap_endpoint) {
    soap_endpoint = "https://ec2.amazonaws.com/";
  }
  if (!soap_action) {
    soap_action = "DescribeRegions";
  }
  soap->encodingStyle = NULL;
  soap_tmp___ec2__DescribeRegions.ec2__DescribeRegions = ec2__DescribeRegions;
  soap_begin(soap);
  soap_serializeheader(soap);
  soap_serialize___ec2__DescribeRegions(soap, &soap_tmp___ec2__DescribeRegions);
  if (soap_begin_count(soap)) {
    return soap->error;
  }
  if (soap->mode & SOAP_IO_LENGTH) {
    if (soap_envelope_begin_out(soap)
        || soap_putheader(soap)
        || soap_body_begin_out(soap)
        || soap_put___ec2__DescribeRegions(soap,
                                           &soap_tmp___ec2__DescribeRegions,
                                           "-ec2:DescribeRegions", NULL)
        || soap_body_end_out(soap)
        || soap_envelope_end_out(soap)) {
      return soap->error;
    }
  }
  if (soap_end_count(soap)) {
    return soap->error;
  }
  if (soap_connect(soap, soap_endpoint, soap_action)
      || soap_envelope_begin_out(soap)
      || soap_putheader(soap)
      || soap_body_begin_out(soap)
      || soap_put___ec2__DescribeRegions(soap, &soap_tmp___ec2__DescribeRegions,
                                         "-ec2:DescribeRegions", NULL)
      || soap_body_end_out(soap)
      || soap_envelope_end_out(soap)
      || soap_end_send(soap)) {
    return soap_closesock(soap);
  }
  if (!ec2__DescribeRegionsResponse) {
    return soap_closesock(soap);
  }
  soap_default_ec2__DescribeRegionsResponseType(soap,
                                                ec2__DescribeRegionsResponse);
  if (soap_begin_recv(soap)
      || soap_envelope_begin_in(soap)
      || soap_recv_header(soap)
      || soap_body_begin_in(soap)) {
    return soap_closesock(soap);
  }
  soap_get_ec2__DescribeRegionsResponseType(soap, ec2__DescribeRegionsResponse,
                                            "ec2:DescribeRegionsResponse",
                                            "ec2:DescribeRegionsResponseType");
  if (soap->error) {
    return soap_recv_fault(soap, 0);
  }
  if (soap_body_end_in(soap)
      || soap_envelope_end_in(soap)
      || soap_end_recv(soap)) {
    return soap_closesock(soap);
  }
  return soap_closesock(soap);
} /* soap_call___ec2__DescribeRegions */

#ifdef __cplusplus
}
#endif

#if defined(__BORLANDC__)
#pragma option pop
#pragma option pop
#endif

/* End of soapClient.c */
