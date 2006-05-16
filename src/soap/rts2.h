/** @mainpage rts2 Definitions

@section rts2 Service Binding "rts2"

@subsection rts2 "rts2" Documentation

gSOAP 2.7.6c generated service definition

@subsection rts2_operations Operations

  - @ref ns1__getEqu

@subsection rts2_ports Endpoint Ports

  - http://lascaux.asu.cas.cz/rts2.cgi

*/

//  Note: modify this file to customize the generated data type declarations

//gsoapopt w
//#import "stl.h"

/*
To customize the names of the namespace prefixes generated by wsdl2h, modify
the prefix names below and add the modified lines to typemap.dat to run wsdl2h:

ns1 = urn:rts2
*/

//gsoap ns1   schema namespace: urn:rts2
//gsoap ns1   schema form:      unqualified

// Forward declarations

// End of forward declarations


//gsoap ns1  service name:      rts2 
//gsoap ns1  service type:      equ 
//gsoap ns1  service port:      http://lascaux.asu.cas.cz/rts2.cgi 
//gsoap ns1  service namespace: urn:rts2 

/// Operation response struct "ns1__getEquResponse" of service binding "rts2" operation "ns1__getEqu"
struct ns1__getEquResponse
{
  double ra;
  double dec;
};

/// Operation "ns1__getEqu" of service binding "rts2"

/**

Operation details:

Get current coordinates of the telescope.

  - SOAP RPC encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"

C stub function (defined in soapClient.c[pp]):
@code
  int soap_call_ns1__getEqu(struct soap *soap,
    NULL, // char *endpoint = NULL selects default endpoint for this operation
    NULL, // char *action = NULL selects default action for this operation
    struct ns1__getEquResponse&
  );
@endcode

C++ proxy class (defined in soaprts2Proxy.h):
  class rts2;

*/

//gsoap ns1  service method-style:      getEqu rpc
//gsoap ns1  service method-encoding:   getEqu http://schemas.xmlsoap.org/soap/encoding/
//gsoap ns1  service method-action:     getEqu ""
int ns1__getEqu (struct ns1__getEquResponse &);

/*  End of rts2 Definitions */
