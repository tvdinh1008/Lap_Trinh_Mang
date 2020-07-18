#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <WinSock2.h>

HANDLE* g_handleThread = NULL;
SOCKET* g_socket = NULL;
int* g_threadStatus = NULL;
int g_clientCount = 0;

DWORD WINAPI ClientThread(LPVOID param)
{
	char* welcome = (char*)"Welcome to Telnet Server\n Please login by sending [user password]\n";
	SOCKET c = (SOCKET)param;
	send(c, welcome, strlen(welcome), 0);
	char buffer[1024];
	char cmd[1024];
	char line[1024];
	char name[1024];
	char psw[1024];
	char cname[1024];
	char cpsw[1024];
	while (0 == 0) //Login
	{
		memset(buffer, 0, sizeof(buffer));
		recv(c, buffer, sizeof(buffer), 0);
		memset(cname, 0, sizeof(cname));
		memset(cpsw, 0, sizeof(cpsw));
		sscanf(buffer, "%s%s", cname, cpsw);

		FILE* f = fopen("C:\\Temp\\Users.txt", "rt");
		int found = 0;
		while (!feof(f))
		{
			memset(line, 0, sizeof(line));
			fgets(line, sizeof(line), f);
			memset(name, 0, sizeof(name));
			memset(psw, 0, sizeof(psw));
			sscanf(line, "%s%s", name, psw);
			if (strcmp(name, cname) == 0 && strcmp(psw, cpsw) == 0)
			{
				found = 1;
				break;
			}
		}
		fclose(f);
		if (found)
			break;
		else
		{
			char* invalid = (char*)"Invalid name or password. Please try again!\n";
			send(c, invalid, strlen(invalid), 0);
		}
	}

	char* login = (char*)"Now please send a command to be executed\n";
	send(c, login, strlen(login), 0);

	while (0 == 0)
	{
		memset(buffer, 0, sizeof(buffer));
		recv(c, buffer, sizeof(buffer), 0);
		while (buffer[strlen(buffer) - 1] == '\n' ||
			buffer[strlen(buffer) - 1] == '\r')
		{
			buffer[strlen(buffer) - 1] = 0;
		}
		printf("SOCKET: %d - %s\n", c, buffer);
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd, "%s > out.txt", buffer);
		if (strcmp(buffer, "quit") == 0)
		{
			break;
		}

		system(cmd);
		FILE* f = fopen("out.txt", "rt");
		while (!feof(f))
		{
			memset(buffer, 0, sizeof(buffer));
			fgets(buffer, sizeof(buffer), f);
			send(c, buffer, strlen(buffer), 0);
		}
		fclose(f);
		send(c, "\n", 1, 0);
	}

	closesocket(c);
	for (int i = 0; i < g_clientCount; i++)
	{
		if (c == g_socket[i])
		{
			g_threadStatus[i] = 1; //Stop
			break;
		}
	}
	return 0;
}

void main()
{
	WSADATA DATA;
	WSAStartup(MAKEWORD(2, 2), &DATA);
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN saddr, caddr;
	int clen = sizeof(caddr); //BAT BUOC
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.S_un.S_addr = 0; //Any address
	bind(s, (sockaddr*)&saddr, sizeof(saddr));
	listen(s, 10);
	while (0 == 0)
	{
		DWORD ID = 0;
		SOCKET c = accept(s, (sockaddr*)&caddr, &clen);
		g_clientCount += 1;
		g_handleThread = (HANDLE*)realloc(g_handleThread, g_clientCount * sizeof(HANDLE));
		g_socket = (SOCKET*)realloc(g_socket, g_clientCount * sizeof(SOCKET));
		g_threadStatus = (int*)realloc(g_threadStatus, g_clientCount * sizeof(int));
		g_socket[g_clientCount - 1] = c;
		g_threadStatus[g_clientCount - 1] = 0; //Running
		g_handleThread[g_clientCount - 1] = CreateThread(NULL, 0, ClientThread, (LPVOID)c, 0, &ID);
		//CreatThread moi de chay vong for duoi day
		for (int i = 0; i < g_clientCount; i++)
		{
			if (g_threadStatus[i] == 1)
			{
				CloseHandle(g_handleThread[i]);
				g_handleThread[i] = INVALID_HANDLE_VALUE;
				g_socket[i] = INVALID_SOCKET;
			}
		}
	}

	for (int i = 0; i < g_clientCount; i++)
	{
		if (g_handleThread[i] != INVALID_HANDLE_VALUE)
		{
			CloseHandle(g_handleThread[i]);
		}
		if (g_socket[i] != INVALID_SOCKET)
		{
			closesocket(g_socket[i]);
		}
	}

	free(g_handleThread);
	free(g_socket);
	free(g_threadStatus);
}