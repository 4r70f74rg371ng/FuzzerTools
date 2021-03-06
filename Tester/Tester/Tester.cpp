// Tester.cpp: 定義主控台應用程式的進入點。
//

#include "stdafx.h"

#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#pragma comment(lib, "ws2_32")

int allisipv4(char* str) {
	int i;
	int countdot = 0;
	int countnum = 0;

	if (str) {
		for (i = 0; str[i]; i++) {
			if (str[i] == '.') {
				if (countnum == 0) {
					return 0; // only dot
				}
				countdot++;
				countnum = 0;
			}
			else if ((str[i] < '0') || (str[i] > '9')) {
				return 0;
			}
			else {
				countnum++;
			}
		}
		if (countnum == 0) {
			return 0;
		}
		if (countdot != 3) {
			return 0;
		}
		return 1;
	}

	return 0;
}

SOCKET TCPConnect(char* hostname, int port) {

	SOCKET s = INVALID_SOCKET;
	struct hostent *host;
	int err;
	struct sockaddr_in sa; //this stores the destination address
	int resolved = 0;
	int success = 0;

	strcpy_s((char *)&sa, sizeof sa, "");
	sa.sin_family = AF_INET; //this line must be like this coz internet

	if (allisipv4(hostname)) {
		sa.sin_addr.s_addr = inet_addr(hostname); //get ip into s_addr
		resolved = 1;
	}
	else if ((host = gethostbyname(hostname)) != 0) {
		strcpy_s((char *)&sa.sin_addr, sizeof sa.sin_addr, (char *)host->h_addr_list[0]);
		resolved = 1;
	}
	else {
		printf("[-] Error resolving hostname\n");
	}

	if (resolved == 1) {
		s = socket(AF_INET, SOCK_STREAM, 0); //make net a valid socket handle
		if (s >= 0) {
			sa.sin_port = htons(port);
			//connect to the server with that socket
			err = connect(s, (struct sockaddr *)&sa, sizeof sa);
		}
	}

	return s;
}

void HTTPTesting(char* target, int port, char* payload) {
	
	SOCKET s;
	char buffer[100000] = "";
	int ret = 0;

	

	s = TCPConnect(target, port);

	//printf("send: \"%s\"\n", payload);

	send(s, payload, strlen(payload), 0);

	ret = recv(s, buffer, 100000, 0);
	if (ret > 0) {
		buffer[ret] = '\0';
	}

	//printf("recv: \"%s\"\n", buffer);

	closesocket(s);

		
}

#define fuzzsize 100000

int GetFuzzChar() {
	//char fuzz[37] = "><,.'\":;/?]}[{\\|=+-_)(*&^%$#@!~`\r\n\t\f";
	//int count = 36;

	//char fuzz[] = "\\%'\"?&:A\r\n";
	//int count = 8;

	char fuzz[] = "/%'\"?&:A";
	int count = 6;

	int i;

	return fuzz[rand() % count];
}

void FuzzMutation(char* str) {
	int len = 0;
	int i;
	int count;

	len = strlen(str);

	count = rand() % len;

	str[count] = GetFuzzChar();

	len = strlen(str);

	str[len] = '\r';
	str[len + 1] = '\n';
	str[len + 2] = '\r';
	str[len + 3] = '\n';
	str[len + 4] = '\0';

	//printf("tolen: %d\n", len + 4);
}

void FuzzGeneration(char* str) {
	int len = 0;
	int i;
	int count;

	len = strlen(str);

	count = rand() % len;

	// count 
	for (i = len - 1; i > count; i--) {
		str[i + fuzzsize] = str[i];
	}

	str[count + 1] = '/';

	for (i = 2; i <= fuzzsize; i++) {
		str[count + i] = GetFuzzChar();
	}

	str[len + fuzzsize] = '\0';

	len = strlen(str);

	str[len] = '\r';
	str[len + 1] = '\n';
	str[len + 2] = '\r';
	str[len + 3] = '\n';
	str[len + 4] = '\0';

	//printf("tolen: %d\n", len + 4);
}

void FuzzMutationGeneration(char* str) {
	int len = 0;
	int i;
	int count;

	len = strlen(str);

	count = rand() % len;

	// count 
	for (i = len - 1; i > count; i--) {
		str[i + fuzzsize] = str[i];
	}

	str[count + 1] = '/';

	for (i = 1; i <= fuzzsize; i++) {
		str[count + i] = GetFuzzChar();
	}

	str[count] = str[count] + 1;

	str[len + fuzzsize] = '\0';

	len = strlen(str);

	str[len] = '\r';
	str[len + 1] = '\n';
	str[len + 2] = '\r';
	str[len + 3] = '\n';
	str[len + 4] = '\0';

	//printf("tolen: %d\n", len + 4);
}

typedef void(*DoFuzzFunc)(char*);

DoFuzzFunc fuzzfunc[10] = {
	FuzzMutation,
	FuzzGeneration,
	FuzzMutationGeneration
};

void HTTPFuzzing(char* target, int port, char* raw_request) {
	char* buffer = NULL;
	int len;
	//SOCKET s;

	len = strlen(raw_request);

	buffer = (char*)malloc(sizeof(char)*(len + fuzzsize + 1000));

	while (1) {

		strcpy_s(buffer, len + 1, raw_request);

		//buffer[len] = '\0';
		//printf("From len: %d ", len);

		fuzzfunc[rand() % 3](buffer);

		//printf("to len: %d/%d\n", strlen(buffer), (len + fuzzsize + 100));

		HTTPTesting(target, port, buffer);

		//Sleep(1000);
	}

}

char* FuzzGenParam(char* str, int offset, int length) {
	int len = 0;
	int i;
	int count;

	char* buffer = NULL;

	if (str) {

		len = strlen(str);

		buffer = (char*)malloc(sizeof(char)*(len + length + 1000));

		if (buffer) {

			strcpy_s(buffer, len + 1, str);

			count = rand() % len;

			// count 
			for (i = len - 1; i > count; i--) {
				buffer[i + length] = buffer[i];
			}

			buffer[count + 1] = '/';

			for (i = 2; i <= length; i++) {
				buffer[count + i] = GetFuzzChar();
			}

			buffer[len + length] = '\0';

			len = strlen(buffer);

			buffer[len] = '\r';
			buffer[len + 1] = '\n';
			buffer[len + 2] = '\r';
			buffer[len + 3] = '\n';
			buffer[len + 4] = '\0';

		}
	}

	return buffer;
}

void HTTPFuzzing2(char* target, int port, int offset, int length, char* raw_request) {
	char* buffer = NULL;
	printf("%d %d\n", offset, length);
	buffer = FuzzGenParam(raw_request, offset, length);
	//printf("to len: %d/%d\n", strlen(buffer), (len + fuzzsize + 100));
	HTTPTesting(target, port, buffer);
	free(buffer);
	//Sleep(1000);
}


int main() {
	char raw_request[] = "GET / HTTP/1.0\r\nHost: 192.168.137.111\r\nUser-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:59.0) Gecko/20100101 Firefox/59.0\r\nAccept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\nAccept-Language: zh-TW,zh;q=0.8,en-US;q=0.5,en;q=0.3\r\nAccept-Encoding: gzip, deflate\r\nConnection: keep-alive\r\nUpgrade-Insecure-Requests: 1\r\n\r\n";
	char target[] = "192.168.137.222";
	int port = 80;

	WSADATA firstsock;

	//HTTPFuzzing(target, port, raw_request);

	/*int i;

	for (i = 0; i < strlen(raw_request); i++) {
		printf("%d: %s\n", i, raw_request + i);
	}*/
	int i,j;

	//Initialise winsock
	if (WSAStartup(MAKEWORD(2, 0), &firstsock) == 0) {  //CHECKS FOR WINSOCK VERSION 2.0
		while (1) {
			for (i = 1; i <= 1000; i++) {
				//HTTPFuzzing2(target, port, 21, i * 1000, raw_request);
				//HTTPFuzzing2(target, port, 22, i * 1000, raw_request);

				//HTTPFuzzing2(target, port, 220, i * 1000, raw_request);
				//HTTPFuzzing2(target, port, 221, i * 1000, raw_request);
				for (j = 0; j < strlen(raw_request); j++) {
					HTTPFuzzing2(target, port, j, i * 1000, raw_request);
				}
			}
		}
		//
		WSACleanup();
	}

	system("pause");
    return 0;
}

