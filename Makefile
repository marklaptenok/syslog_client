all: release test

debug: syslog_client.c syslog_client_mod.c syslog_client_mod.h
	gcc -g -Wall -Wextra -Werror -pedantic -std=c11 -lc -D_DEBUG syslog_client.c syslog_client_mod.c -o syslog_client

release: syslog_client.c syslog_client_mod.c syslog_client_mod.h
	gcc -std=c11 -O3 -lc syslog_client.c syslog_client_mod.c -o syslog_client

test:
	@clear
	@./syslog_client -i 192.168.1.3 -m "<31>1 - - - - - [data] data"
	@printf "Positive test 1\t-\tPASSED\n\n"
	@./syslog_client -i 192.168.1.3 -m "<31>1 - - - - - [data data=\"some_data\"] data"
	@printf "Positive test 2\t-\tPASSED\n\n"
	@./syslog_client -i 192.168.1.3 -m "<25>1 2003-10-11T22:14:15.003Z mymachine.example.com evntslog - ID47 [exampleSDID@32473 iut=\"3\" eventSource=\"Application\" eventID=\"1011\"][examplePriority@32473 class=\"high\"]"	
	@printf "Positive test 3\t-\tPASSED\n\n"
	@./syslog_client -i 192.168.1.3 -m "<25>1 2004-10-11T22:14:15.003Z mymachine.example.com evntslog - ID47 [exampleSDID@32473 iut=\"3\" eventSource=\"Application\" eventID=\"1011\"][examplePriority@32473 class=\"high\"] some data v Češtině"	
	@printf "Positive test 4\t-\tPASSED\n\n"
	@./syslog_client -i 192.168.1.3 -m "<31>1 1999-01-23T23:00:03.056788+01:00 - - - - - data"
	@printf "Positive test 5\t-\tPASSED\n\n"
	@./syslog_client -i 192.168.1.3 -m "<31>1 1999-01-23T23:00:03.056788+01:00 test - - - - data"
	@printf "Positive test 6\t-\tPASSED\n\n"
	@./syslog_client -i 192.168.1.3 -m "<31>1 1999-01-23T23:00:03.056788+01:00 test_host test_app_name a123_procid a456_msgid - data"
	@printf "Positive test 7\t-\tPASSED\n\n"
	@./syslog_client -i 192.168.1.3 -m "<31>1 2023-01-23T23:00:03.056788+01:00 test_host test_app_name a123_procid a456_msgid [exampleSDID@32473 iut=\"3\" eventSource=\"Application\" eventID=\"1011\"][examplePriority@32473 class=\"high\"] some data v Češtině"
	@printf "Positive test 8\t-\tPASSED\n\n"
	@printf "All tests PASSED\n"

.PHONY: all debug release test
