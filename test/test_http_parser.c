#include "../src/http_parser.h"

typedef void (*MyFuncPtr)(void);

char relative_request[] = "GET /index.html HTTP/1.1\r\n"
			  "Host: www.example.com:1209\r\n"
			  "User-Agent: Mozilla/5.0\r\n"
			  "Accept-Language: en\r\n"
			  "\r\n";

char absolute_request[] = "GET http://www.example.com:1209/index.html HTTP/1.1\r\n"
			  "Host: www.example.com:1209\r\n"
			  "User-Agent: Mozilla/5.0\r\n"
			  "Accept-Language: en\r\n"
			  "\r\n";

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_CYAN "\x1b[36m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define print_test_name(name) \
	printf(ANSI_COLOR_CYAN "TEST: %s" ANSI_COLOR_RESET "\n", name);

#define pass() printf(ANSI_COLOR_GREEN "PASSED TEST" ANSI_COLOR_RESET "\n");

#define fail() printf(ANSI_COLOR_RED "FAILED TEST" ANSI_COLOR_RESET "\n");

/* TESTS */
void parse_all_relative(void)
{
	print_test_name("parse_all_relative");
	HTTPRequest parsed = parse_http_request(relative_request);
	printf("Parsed Method: %s\n", parsed.method);
	printf("Parsed FullURL: %s\n", parsed.full_url);
	printf("Parsed Host: %s\n", parsed.host);
	printf("Parsed Path: %s\n", parsed.path);
	printf("Parsed Port: %d\n", parsed.port);
	printf("Parsed Raw Request:\n%s", parsed.raw_request);
	pass();
}

void parse_all_absolute(void)
{
	print_test_name("parse_all_absolute");
	HTTPRequest parsed = parse_http_request(absolute_request);
	printf("Parsed Method: %s\n", parsed.method);
	printf("Parsed FullURL: %s\n", parsed.full_url);
	printf("Parsed Host: %s\n", parsed.host);
	printf("Parsed Path: %s\n", parsed.path);
	printf("Parsed Port: %d\n", parsed.port);
	printf("Parsed Raw Request:\n%s", parsed.raw_request);
	pass();
}

/* TEST ARRAY */
MyFuncPtr a[] = { parse_all_relative, parse_all_absolute };

/* RUN TESTS */
int main(void)
{
	for (int i = 0; i < (int)sizeof(a) / (int)sizeof(a[0]); i++) {
		a[i]();
	}
	printf(ANSI_COLOR_GREEN "ALL TESTS COMPLETE" ANSI_COLOR_RESET "\n");
	return 0;
}
