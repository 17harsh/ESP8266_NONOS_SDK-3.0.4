/*
 * http.c
 *
 *  Created on: 23-May-2021
 *      Author: harsh
 */

#include "../../esp_proj_wifi/include/driver/http.h"

#include "stdlib.h"

#include "osapi.h"
#include "user_interface.h"
#include "mem.h"
#include "espconn.h"

//Set-Up Debugging Macros
#ifndef HTTP_DEBUG
	#define HTTP_LOG_DEBUG(message)					do {} while(0)
	#define HTTP_LOG_DEBUG_ARGS(message, args...)	do {} while(0)
#else
	#define HTTP_LOG_DEBUG(message)					do {os_printf("[HTTP-DEBUG] %s", message); os_printf("\r\n");} while(0)
	#define HTTP_LOG_DEBUG_ARGS(message, args...)	do {os_printf("[HTTP-DEBUG] "); os_printf(message, args); os_printf("\r\n");} while(0)
#endif

//** Private Methods **********************************************************************************//

HTTP_RESULT _httpRoutePath(char *iRecv, uint16 iLength, char **oRoutePath, uint16 *oPathLength);
HTTP_RESULT _httpRequestData(char *iRecv, uint16 iLength, char **oData, uint16 *oDataLength);

//****************************************************************************************************//

HTTP_RESULT isHttp(char *iRecv, uint16 iLength){
	HTTP_RESULT result = HTTP_FAIL;
	if(iLength > 0 && iRecv != NULL){
		char *http = NULL;
		http = os_strstr(iRecv, "HTTP");
		if(http != NULL) result = HTTP_SUCCESS;
	}
	return result;
}

HTTP_RESULT processHttpRequest (char *iRecv, uint16 iLength, HTTP_REQUEST *oHttpRequest){
	HTTP_RESULT result = HTTP_FAIL;
	if(iRecv != NULL && iLength > 0 && oHttpRequest != NULL){

		HTTP_REQUEST_TYPE httpRequest = HTTP_INVALID;
		if(os_strncmp(iRecv, "GET", 3) == 0){
			httpRequest =  HTTP_GET;
			HTTP_LOG_DEBUG("Request type : GET");
		}
		else if(os_strncmp(iRecv, "POST", 4) == 0){
			httpRequest =  HTTP_POST;
			HTTP_LOG_DEBUG("Request type : POST");
		}

		oHttpRequest->httpRequestType = httpRequest;

		if(httpRequest != HTTP_INVALID){
			char *routePath = NULL;
			uint16 pathLength = -1;
			if(_httpRoutePath(iRecv, iLength, &routePath, &pathLength) == HTTP_SUCCESS){
				if(pathLength == 0 && routePath != NULL){
					//that is, route path is root
					++pathLength;
					--routePath;
				}
				oHttpRequest->routeLength = pathLength;
				oHttpRequest->routePath = routePath;
				result = HTTP_SUCCESS;
			}

			char *data = NULL;
			uint16 dataLength = -1;
			if(_httpRequestData(iRecv, iLength, &data, &dataLength) == HTTP_SUCCESS){
				oHttpRequest->dataLength = dataLength;
				oHttpRequest->data = data;
				result = HTTP_SUCCESS;
			}
		}
	}
	return result;
}

HTTP_RESULT sendHttpResponse (struct espconn *espconn, HTTP_RESPONSE* iHttpResponse){
	HTTP_LOG_DEBUG("inside sendHttpResponse");
	HTTP_RESULT result = HTTP_FAIL;
	if(iHttpResponse != NULL){
		HTTP_LOG_DEBUG_ARGS("content : %s", iHttpResponse->content);

		char* responsePacket = (char*) os_zalloc(256 + iHttpResponse->contentLength);

		char *responseCode = NULL;
		if(iHttpResponse->responseCode == HTTP_OK) responseCode = "200 OK";
		else if(iHttpResponse->responseCode == HTTP_Bad_Request) responseCode = "400 Bad Request";
		else if(iHttpResponse->responseCode == HTTP_Not_Found) responseCode = "404 Not Found";

		char *contentType = NULL;
		if(iHttpResponse->contentType == text_html) contentType = "text/html";
		else if(iHttpResponse->contentType == text_css) contentType = "text/css";
		else if(iHttpResponse->contentType == application_javascript) contentType = "application/javascript";

		char *connection = NULL;
		if(iHttpResponse->connection == Closed) connection = "Closed";

		if(responsePacket != NULL && responseCode != NULL && contentType != NULL && connection !=NULL && iHttpResponse->content != NULL){
			os_sprintf(responsePacket, "HTTP/1.1 %s\r\n\
Server: ESP8266\r\n\
Content-Length: %d\r\n\
Content-Type: %s\r\n\
Connection: %s\r\n\r\n\
%s", responseCode, iHttpResponse->contentLength, contentType, connection, iHttpResponse->content);

			HTTP_LOG_DEBUG_ARGS("Content : %s",responsePacket);
			if(espconn != NULL){
				sint8 status = espconn_send(espconn, responsePacket, os_strlen(responsePacket));
				HTTP_LOG_DEBUG_ARGS("espconn send, status : %d",status);
				if(status == 0) result = HTTP_SUCCESS;
			}
		}
		os_free(responsePacket);
	}
	return result;
}

HTTP_RESULT _httpRoutePath(char *iRecv, uint16 iLength, char **oRoutePath, uint16 *oPathLength){
	HTTP_LOG_DEBUG("Inside httpRoutePath");
	HTTP_RESULT result = HTTP_FAIL;
	if(iRecv != NULL && iLength > 0){
		char* p1 = NULL;
		char* p2 = NULL;
		p1 = os_strstr(iRecv, "/");
		p2 = os_strstr(iRecv, "HTTP");
		if(p1 != NULL && p2 != NULL){
			++p1;--p2;
			*oRoutePath = p1;
			*oPathLength = p2 - p1;
			result = HTTP_SUCCESS;
		}
	}
	return result;
}

HTTP_RESULT _httpRequestData(char *iRecv, uint16 iLength, char **oData, uint16 *oDataLength){
	HTTP_LOG_DEBUG("Inside httpRequestData");
	HTTP_RESULT result = HTTP_FAIL;
	if(iRecv != NULL && iLength > 0){
		char* p1 = NULL;
		char* p2 = NULL;
		p1 = os_strstr(iRecv, "\r\n\r\n");
		p2 = os_strstr(iRecv, "Content-Length:");
		if(p1 != NULL && p2 != NULL){
			p1 += 4;
			*oData = p1;

			p2 += 16;
			char *p3 = os_strstr(p2, "\r\n");
			char p4[5] = {0};
			os_memcpy(p4, p2, p3-p2);
			uint16 datalength = atoi(p4);
			*oDataLength = datalength;
			result = HTTP_SUCCESS;
		}
	}
	return result;
}

