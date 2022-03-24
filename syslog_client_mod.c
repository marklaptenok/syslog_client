#include "syslog_client_mod.h"

//	printf()
#include <stdio.h>
//	strlen()
#include <string.h>
//	malloc(), free()
#include <stdlib.h>
#ifdef _DEBUG
//	intXX_t, uintXX_t
#include <inttypes.h>
#endif

//	close()
#include <unistd.h>
//	regular expression operations
#include <regex.h>        
//	inet_ntoa()
#include <arpa/inet.h>
//	struct sockaddr, socket(), sendto()
#include <sys/socket.h>
//	BSD socket errors
#include <errno.h>

	
int 
syslog_client_is_rfc5424_syslog_message(const char* const message)
{
	if (message == NULL || *message == '\0')
		return 0;

#ifdef _DEBUG
	printf("syslog_client_is_rfc5424_syslog_message()\n\tMessage: %s\n", message);
#endif

	//	Finds the space (%d32) between header and structured-data.
	char* ptr = (char*)message;
	for (uint8_t counter = 0; counter < 6; ++counter, ++ptr)
		if ((ptr = strchr(ptr, (int)' ')) == NULL)
			return 0;

	char* header = (char*)malloc(ptr - message);
	if (header == NULL)
		exit(MEMORY_ALLOCATION_FAILED_CODE);
	memcpy(header, message, ptr - message);
	header[ptr - message - 1] = '\0';

#ifdef _DEBUG
	printf("syslog_client_is_rfc5424_syslog_message()\n\tHeader: %s\n", header);
#endif

	regex_t regex_header;
	
	//	Compiles regular expression to check the header.
	int result = regcomp(
			&regex_header, 
			"^"
			"<([0-9]|[1-9][0-9]|1[1-8][0-9]|19[01])>"
			"[1-9][0-9]{0,2}"
			" "
			"("
					"-"
				"|"
					"("
						"[0-9]{4}"
						"-"
						"(0[1-9]|1[012])"
						"-"
						"([012][0-9]|3[01])"
						"T"
						"([01][0-9]|2[0-3])"
						":"
						"[0-5][0-9]"
						":"
						"[0-5][0-9]"
						"(\\.[0-9]{1,6})?"
						"(Z|([+-]([01][0-9]|2[0-3]):[0-5][0-9]))"
					")"
			")"
			" "
			"("
					"-"
				"|"
					"[[:print:]]{1,255}"
			")"
			" "
			"("
					"-"
				"|"
					"[[:print:]]{1,48}"
			")"
			" "
			"("
					"-"
				"|"
					"[[:print:]]{1,128}"
			")"
			" "
			"("
					"-"
				"|"
					"[[:print:]]{1,32}"
			")"
			"$",
		REG_EXTENDED | REG_NOSUB
	);
	
	if (result != 0) {
#ifdef _DEBUG
		char buffer[100] = {'\0'};
		
		regerror(result, &regex_header, buffer, sizeof(buffer));
    	printf("syslog_client_is_rfc5424_syslog_message()\n\tCompiling the regex_header failed: %s\n", buffer);
#endif
		free(header);
    	return 0;
	}

	//	Executes the regular expression on 'header'.
	result = regexec(&regex_header, header, (size_t)0, NULL, 0);
	
	//	Frees resources.
	regfree(&regex_header);
	free(header);

	//	Checks the result of matching the header.
	switch (result) {
		case 0:
    		break;
		case REG_NOMATCH:
    	default:
#ifdef _DEBUG
		{
			char buffer[100] = {'\0'};
    		
			regerror(result, &regex_header, buffer, sizeof(buffer));
    		printf("syslog_client_is_rfc5424_syslog_message()\n\tRegex_header failed: %s\n", buffer);
    	}
#endif
			return 0;
	}	//	-- end of switch ---

#ifdef _DEBUG
	printf("syslog_client_is_rfc5424_syslog_message()\n\tData: %s\n", ptr);
#endif

	regex_t regex_data;
	
	//	Compiles regular expression to check structured data and message (if presented).
	result = regcomp(
			&regex_data, 
			"^"
			"("
					"-"
				"|"
					"(\\[[^= \"]{1,32}( [^= \"]{1,32}=\".*\")*\\])+"
			")"
			"("
				" .*"
			"){0,1}"
			"$",
		REG_EXTENDED | REG_NOSUB
	);
	
	if (result != 0) {
#ifdef _DEBUG
		char buffer[100] = {'\0'};
		
		regerror(result, &regex_data, buffer, sizeof(buffer));
    	printf("syslog_client_is_rfc5424_syslog_message()\n\tCompiling the regex_data failed: %s\n", buffer);
#endif
    	return 0;
	}

	//	Executes the regular expression on 'header'.
	result = regexec(&regex_data, ptr, (size_t)0, NULL, 0);
	
	//	Frees resources.
	regfree(&regex_data);

	//	Checks the result of matching structured data and message (if presented).
	switch (result) {
		case 0:
    		break;
		case REG_NOMATCH:
    	default:
#ifdef _DEBUG
		{
			char buffer[100] = {'\0'};
    		
			regerror(result, &regex_data, buffer, sizeof(buffer));
    		printf("syslog_client_is_rfc5424_syslog_message()\n\tRegex_data failed: %s\n", buffer);
    	}
#endif
			return 0;
	}	//	-- end of switch ---
		//
	return 1;
}



int 
syslog_client_send_message_to_syslog_server(const packet_t* const packet)
{
	if (packet == NULL)
		return PACKET_POINTER_IS_NULL_CODE;

	if (packet->message == NULL)
		return MESSAGE_POINTER_IS_NULL_CODE;

	if (*(packet->message) == '\0')
		return MESSAGE_EMPTY_CODE;

#ifdef _DEBUG
	printf("syslog_client_send_message_to_syslog_server()\n\tIP address: %s\n\tPort: %"PRIu16"\n\tMessage: %s\n",
			inet_ntoa(packet->connection_data.sin_addr), packet->connection_data.sin_port, packet->message);
#endif

	int server_socket = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (server_socket == -1)
		 return SOCKET_CREATION_FAILED_CODE;

	//	Sets to transmit the message without prior connection established
	//	and without blocking.
	//	TO-DO: use poll() to check whether the socket is ready to transmit.
	ssize_t result = sendto(
			server_socket, 
			packet->message, 
			strlen(packet->message), 
			MSG_DONTWAIT,
			(struct sockaddr*)&packet->connection_data,
			sizeof(packet->connection_data)
	);
	
	if (close(server_socket) == -1)
		return CLOSING_SOCKET_FAILED_CODE;

	if (result == (ssize_t)-1) {
		switch (errno) {
			case EAGAIN:
				break;
			case EACCES:
				return SYSLOG_SERVER_ADDRESS_IS_BROADCAST_ADDRESS_CODE;
			case ENOBUFS:
				return CLIENT_SIDE_NETWORK_ERROR_CODE;
			case ECONNREFUSED:
			case EHOSTUNREACH:
			case EHOSTDOWN:
			case ENETDOWN:
				return SYSLOG_SERVER_IS_NOT_AVAILABLE;
			case EMSGSIZE:
				return MESSAGE_IS_TO_LONG_CODE;
			default:
#ifdef _DEBUG
				printf("syslog_client_send_message_to_syslog_server()\n\terrno = %d\n", errno);
#endif
				return OTHER_REASON_FOR_SEND_TO_FAIL_CODE;
		}
	} else {
		if (result > 0 && (size_t)result != strlen(packet->message))
			return PARTIALLY_SENT_MESSAGE_CODE;
	}

#ifdef _DEBUG
	printf("syslog_client_send_message_to_syslog_server()\n\tsent data = %ld bytes\n", (long int)result);
#endif

	return SUCCESSFULLY_SENT_CODE;
}

