// LogRecoveryFuzzer.cpp: 定義主控台應用程式的進入點。
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

FILE* fp = NULL;

void ProtocolTesting(char* target, int port, char* payload) {
	WSADATA firstsock;
	SOCKET s;
	char buffer[100000] = "";
	int ret = 0;

	//Initialise winsock
	if (WSAStartup(MAKEWORD(2, 0), &firstsock) == 0) {  //CHECKS FOR WINSOCK VERSION 2.0

		s = TCPConnect(target, port);

		//printf("send: \"%s\"\n", payload);

		//fwrite(payload, strlen(payload) + 1, 1, fp);

		//fflush(fp);

		send(s, payload, strlen(payload), 0);

		ret = recv(s, buffer, 100000, 0);
		if (ret > 0) {
			buffer[ret] = '\0';
		}

		printf("recv: \"%s\"\n", buffer);

		closesocket(s);

		WSACleanup();
	}
}

// \n
void TranslatePayload(char* org, char* converted) {
	int i;
	int now = 0;

	for (i = 0; org[i]; i++) {
		if (org[i] == '\n') {
			converted[now++] = '\r';
			converted[now++] = '\n';
		}
		else {
			converted[now++] = org[i];
		}
	}
	converted[i] = '\0';
}

void ReadLogFile(char* logfile) {
	unsigned long long pointer = 0;
	unsigned long long needlength = 0;
	unsigned long long totallength = 0;
	unsigned long long indexnow = 0;
	char* buffer = NULL;
	char* bufferpayload = NULL;
	int c;

	FILE* fp = fopen(logfile, "rb");
	while (!feof(fp)) {
		c = fgetc(fp);
		if (c == EOF) {
			break;
		}
		totallength++;
	}
	fclose(fp);

	while (pointer < totallength) {
		// get length
		needlength = 0;
		fp = fopen(logfile, "rb");
		fseek(fp, pointer, SEEK_SET);
		while (!feof(fp)) {
			c = fgetc(fp);
			if (c == EOF) {
				break;
			}
			if (c == '\0') {
				break;
			}
			needlength++;
		}
		fclose(fp);

		//printf("%llu %llu %llu\n", pointer, needlength, totallength);

		buffer = (char*)malloc(sizeof(char)*(needlength+1));
		bufferpayload = (char*)malloc(sizeof(char)*(needlength + 20));

		// get content
		indexnow = 0;
		fp = fopen(logfile, "rb");
		fseek(fp, pointer, SEEK_SET);
		while (!feof(fp)) {
			c = fgetc(fp);
			if (c == EOF) {
				break;
			}
			if (c == '\0') {
				break;
			}
			buffer[indexnow++] = c;
		}
		fclose(fp);
		buffer[indexnow] = '\0';

		pointer += needlength+1;

		//printf("%llu %llu %llu %llu %llu\n", pointer, needlength, totallength, indexnow, strlen(buffer));

		printf("content: '%s'\n", buffer);

		TranslatePayload(buffer, bufferpayload);
		ProtocolTesting((char*)"192.168.137.222", 80, bufferpayload);

		free(buffer);
		free(bufferpayload);
		//system("pause");
	}
}

int main(int argc, char* argv[])
{
	char* logfile = argv[1];

	/*srand(time(NULL));

	char filename[1024] = "";
	sprintf_s(filename, 1024, "payload%d.txt", rand());

	fopen_s(&fp, filename, "ab");*/

	printf("Filename: %s\n", logfile);

	ReadLogFile(logfile);

	//fclose(fp);

    return 0;
}

