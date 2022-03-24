#ifndef SYSLOG_CLIENT_MOD_H
#define SYSLOG_CLIENT_MOD_H

//	struct sockaddr_in, struct in_addr, htons()
#include <netinet/in.h>


//	syslog_client_send_message_to_syslog_server() return codes
typedef enum syslog_client_send_message_error_codes {
	SUCCESSFULLY_SENT_CODE,
	PACKET_POINTER_IS_NULL_CODE,
	MESSAGE_POINTER_IS_NULL_CODE,
	MESSAGE_EMPTY_CODE,
	SOCKET_CREATION_FAILED_CODE,
	SYSLOG_SERVER_ADDRESS_IS_BROADCAST_ADDRESS_CODE,
	CLIENT_SIDE_NETWORK_ERROR_CODE,
	SYSLOG_SERVER_IS_NOT_AVAILABLE,
	MESSAGE_IS_TO_LONG_CODE,
	OTHER_REASON_FOR_SEND_TO_FAIL_CODE,
	CLOSING_SOCKET_FAILED_CODE,
	PARTIALLY_SENT_MESSAGE_CODE,
	MEMORY_ALLOCATION_FAILED_CODE
} syslog_client_send_message_error_codes_t;

typedef struct packet {
	struct sockaddr_in connection_data;
	char* message;
} packet_t;


//	Returns 1, if the 'message' has passed the validation.
//	Returns 0, otherwise.
int 
syslog_client_is_rfc5424_syslog_message(const char* const message);

//	Returns 0, if the 'packet->message' is successfully sent.
//	Returns >0, otherwise.
int 
syslog_client_send_message_to_syslog_server(const packet_t* const packet);

#endif
