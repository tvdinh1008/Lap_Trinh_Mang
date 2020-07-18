#include<stdio.h>

#include<WinSock2.h>

HANDLE g_event[64];
SOCKET g_socket[64];
int g_count = 0;//số khóa nằm trong mảng



void main()
{
	char msg[256] = "hello client!\n";
	int k = strlen(msg);

	WSADATA DATA;
	WSAStartup(MAKEWORD(2, 2), &DATA);
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	SOCKADDR_IN saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.S_un.S_addr = 0;
	bind(s, (SOCKADDR*)&saddr, sizeof(saddr));
	listen(s, 10);

	HANDLE sevent = WSACreateEvent();
	//liên kết socket lễ tân với event
	WSAEventSelect(s, sevent, FD_ACCEPT | FD_READ | FD_CLOSE);
	g_event[g_count] = sevent;
	g_socket[g_count++] = s;

	while (1)
	{
		//đứng đợi mảng khóa đến khi có 1 khóa mở ra
		int index=WSAWaitForMultipleEvents(g_count, g_event, false, INFINITE, false);
		index -= WSA_WAIT_EVENT_0; //chỉ số đầu tiên của khóa bị mở

		for (int i = index; i < g_count; i++)
		{
			//nếu ko phải là trên g_event đó->nó sẽ bị timeout
			DWORD check= WSAWaitForMultipleEvents(1, &g_event[i], false, 10, false);
			if (check != WAIT_TIMEOUT && check != WSA_WAIT_FAILED)
			{
				WSANETWORKEVENTS NE;
				//khóa mở
				WSAEnumNetworkEvents(g_socket[i], g_event[i], &NE);
				if (NE.lNetworkEvents & FD_ACCEPT)
				{
					SOCKADDR_IN caddr;
					int clen = sizeof(caddr);

					SOCKET tmpSocket=accept(g_socket[i], (SOCKADDR*)&caddr, &clen);
					//liên kết hóa
					HANDLE tmpEvent = WSACreateEvent();
					WSAEventSelect(tmpSocket, tmpEvent, FD_ACCEPT | FD_READ | FD_CLOSE);
					g_socket[g_count] = tmpSocket;
					g_event[g_count++] = tmpEvent;

				}
				if (NE.lNetworkEvents & FD_READ)
				{
					char buffer[1024];
					memset(buffer, 0, sizeof(buffer));
					recv(g_socket[i], buffer, sizeof(buffer), 0);

					for (int j = 0; j < g_count; j++)
					{
						if (j != i)
						{
							send(g_socket[j], buffer, strlen(buffer), 0);
						}
					}

				}
				if (NE.lNetworkEvents & FD_CLOSE)
				{

				}
				if (NE.lNetworkEvents & FD_WRITE)
				{

				}
			}
		}


	}




}


