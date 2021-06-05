/*
 * http.h
 *
 *  Created on: 23-May-2021
 *      Author: harsh
 */

#ifndef INCLUDE_DRIVER_HTTP_H_
#define INCLUDE_DRIVER_HTTP_H_

#include "c_types.h"

//forward declaration
struct espconn;

//un-comment this for debugging messages
//#define HTTP_DEBUG

typedef enum httpResult {
	HTTP_FAIL,
	HTTP_SUCCESS
}HTTP_RESULT;

typedef enum httpResponseCode {
	HTTP_OK, //200,
	HTTP_Bad_Request, //400,
	HTTP_Not_Found  //404
}HTTP_RESPONSE_CODE;

typedef enum connectionState{
	Closed,
	Keep_Alive
}CONNECTION;

typedef enum contentType{
	text_html,
	text_css,
	application_javascript
}CONTENT_TYPE;

typedef struct httpResponse{
	HTTP_RESPONSE_CODE responseCode;
	CONNECTION connection;
	char *content;
	uint16 contentLength;
	CONTENT_TYPE contentType;
} HTTP_RESPONSE;

typedef enum httpRequestType {
	HTTP_GET,
	HTTP_POST,
	HTTP_INVALID
}HTTP_REQUEST_TYPE ;

typedef struct httpRequest{
	HTTP_REQUEST_TYPE httpRequestType;
	char *routePath;
	uint16 routeLength;
	char *data;
	uint16 dataLength;
} HTTP_REQUEST;



/******************************************************************************
 * FunctionName : is_http
 * Description  : check if received data is of http type
 * Parameters   : precv	-- data received
 *                length -- length of received data
 * Returns      : bool	-- true if data is http type
 * 						-- false if data is not http type
*******************************************************************************/
HTTP_RESULT isHttp(char *iRecv, uint16 iLength);

HTTP_RESULT processHttpRequest (char *iRecv, uint16 iLength, HTTP_REQUEST *oHttpRequest);

HTTP_RESULT sendHttpResponse (struct espconn *espconn, HTTP_RESPONSE* iHttpResponse);

#endif /* INCLUDE_DRIVER_HTTP_H_ */
