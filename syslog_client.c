//	printf()
#include <stdio.h>
//	strtol()
#include <stdlib.h>
//	intXX_t, uintXX_t
#include <inttypes.h>

//	getopt()
#include <unistd.h>
//	inet_addr()
#include <arpa/inet.h>
//	struct sockaddr_in, struct in_addr, htons()
#include <netinet/in.h>
//	AF_INET
#include <sys/socket.h>

//	Syslog Client module
#include "syslog_client_mod.h"

//	exit codes
typedef enum exit_codes {
	MESSAGE_SUCCESSFULLY_SENT_CODE,
	SERVER_ADDRESS_IS_NOT_SPECIFIED_CODE,
	INVALID_SERVER_ADDRESS_CODE,
	SERVER_PORT_IS_NOT_SPECIFIED_CODE,
	INVALID_SERVER_PORT_CODE,
	EMPTY_MESSAGE_CODE,
	INVALID_SYSLOG_MESSAGE_CODE,
	INVALID_OPTION_CODE,
	SENDING_TO_SYSLOG_SERVER_FAILED_CODE
} exit_codes_t;

#define USAGE 																																\
			"Simple SYSLOG UDP-client.\nCopyright @ Mark Laptenok <marklaptenok@seznam.cz>\n\n"												\
			"syslog_client [-i <ip_address>] [-p <port_number>] -m <message>\n\n"															\
			"\t<ip_address>\tIPv4 address of a SYSLOG server(e.g. 192.168.1.3).\n\t\t\tDefault value is 192.168.1.1.\n\n"					\
			"\t<port_number>\tport number where the SYSLOG server listens\n\t\t\tfor incomming UDP-connections.\n\t\t\tDefault is 514.\n\n"	\
			"\t<message>\ta string that is formatted according to RFC5424.\n"

//	main() user messages
#define INVALID_SERVER_ADDRESS "The SYSLOG server address specified is not a valid IPv4 address\n"
#define INVALID_SERVER_PORT "The SYSLOG server port specified is not a valid number (it has to be in range 1-65535)\n"
#define INVALID_SYSLOG_MESSAGE "The specified message is not compliant with RFC5424\n"
#define SENDING_TO_SYSLOG_SERVER_FAILED "Sending message failed\n"
#define MESSAGE_SUCCESSFULLY_SENT "The specified message was successfully sent\nWARNING: It's not guaranteed that it's received!\n"


//	Prints the application usage rules to the standard output.
void 
print_usage();

int main(int argc, char** args)
{	
	//	Sets the defaults.
	packet_t packet = {
		.connection_data.sin_family = AF_INET,
		.connection_data.sin_port = htons(514),
		.connection_data.sin_addr.s_addr = (uint32_t)inet_addr("192.168.1.1"),
		.message = NULL
	};

	//	Parses the CLI arguments.
	int option = -1;
	while ((option = getopt(argc, args, "i:p:m:h")) != -1) {

		switch ((char)option) {
			//	Reads the IP address of a SYSLOG server to which we will send a message.
			case 'i':
				if (optarg == NULL) {
					print_usage();
					return SERVER_ADDRESS_IS_NOT_SPECIFIED_CODE;
				}
				
				in_addr_t address = (in_addr_t)-1;
				if ((address = inet_addr(optarg)) == (in_addr_t)-1) {
					printf(INVALID_SERVER_ADDRESS);
					return INVALID_SERVER_ADDRESS_CODE;
				}

				packet.connection_data.sin_addr.s_addr = address;
				
				break;
			//	Reads the port at which the SYSLOG server listens for incomming connections.
			case 'p':
				if (optarg == NULL) {
					print_usage();
					return SERVER_PORT_IS_NOT_SPECIFIED_CODE;
				}

				char* end = NULL;
				long int convertion_result = strtol(optarg, &end, 10);
			   	if (*end != '\0' || convertion_result < (long int)1 || convertion_result > (long int)65535) {
					printf(INVALID_SERVER_PORT);
					return INVALID_SERVER_PORT_CODE;
				}

				packet.connection_data.sin_port = htons((uint16_t)convertion_result);

				break;
			//	Reads user's SYSLOG message to send.
			case 'm':
				//	Sets 'message' to the address of the message in the 'args'.
				packet.message = args[optind-1];

				break;
			//	If an unknown option specified or '-h' specified, shows the usage rules.	
			case 'h':
				print_usage();
				return 0;
			case '?':
			default:
				print_usage();
				return INVALID_OPTION_CODE;

		}	//	--- end of switch() ---

	}	//	--- end of while(getopt()) ---

	//	Checks that a message is presented.
	if (packet.message == NULL || *packet.message == '\0') {
		print_usage();
		return EMPTY_MESSAGE_CODE;
	}

	//	Verifies that we deal with a proper message. Otherwise, notifies the user and terminates. 
	if (syslog_client_is_rfc5424_syslog_message(packet.message) == 0) {
		printf(INVALID_SYSLOG_MESSAGE);
		return INVALID_SYSLOG_MESSAGE_CODE;
	}
	
	//	Starts connection and sending the message.
	syslog_client_send_message_error_codes_t error = SUCCESSFULLY_SENT_CODE;
	if ((error = syslog_client_send_message_to_syslog_server(&packet)) != 0) {
		printf("(%d) %s", error, SENDING_TO_SYSLOG_SERVER_FAILED);

		return SENDING_TO_SYSLOG_SERVER_FAILED_CODE;
	}	

	printf(MESSAGE_SUCCESSFULLY_SENT);
	return MESSAGE_SUCCESSFULLY_SENT_CODE;
}

void 
print_usage()
{
	printf(USAGE);
}

