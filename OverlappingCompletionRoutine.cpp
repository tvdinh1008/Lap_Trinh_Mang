#include <stdio.h>
#include <WinSock2.h>
typedef struct
{
	WSAOVERLAPPED ov;
	WSABUF wsabuf;
	char buffer[1024];
	SOCKET c;
} MYOVERLAPPED;

SOCKET clients[1024];
int count = 0;

void CALLBACK RecvCB(DWORD dwError, DWORD cbTransfered, LPWSAOVERLAPPED ov, DWORD dwFlags)
{
	MYOVERLAPPED* lpov = (MYOVERLAPPED*)ov; 
	SOCKET c = lpov->c;
	for (int i = 0; i < count; i++)
	{
		if (clients[i] != c)
		{
			DWORD BytesSent;
			WSAOVERLAPPED sendOV;
			memset(&sendOV, 0, sizeof(WSAOVERLAPPED));
			lpov->wsabuf.len = strlen(lpov->wsabuf.buf);//khi gửi đi thì cần gửi kích thước buffer
			WSASend(clients[i], &(lpov->wsabuf), 1, &BytesSent, 0, &sendOV, NULL);
		}
	}
	DWORD BytesRecv = 0, Flags = 0;

	//memset overlap trước khi tái sử dụng nó do ko treo lên lần nhận dữ liệu tiếp theo lpov lại có giá trị là nằm ở đây
	//Tức là gọi lần 1 thì lpov là giá trị đc gắn vào của WSARecv lần trước
	memset(ov, 0, sizeof(WSAOVERLAPPED));
	lpov->wsabuf.len = sizeof(lpov->buffer);//khi muốn nhận về cần kích thước chứa tối đa của buffer
	WSARecv(c, &(lpov->wsabuf), 1, &BytesRecv, &Flags, (WSAOVERLAPPED*)lpov, RecvCB);
}

void main()
{
	WSADATA data;
	WSAStartup(MAKEWORD(2, 2), &data);
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.S_un.S_addr = 0;  //htons(INADDR_ANY)
	bind(s, (sockaddr*)&saddr, sizeof(saddr));
	listen(s, 10);

	while (0 == 0)
	{
		DWORD ID = 0;
		SOCKADDR_IN caddr;
		int clen = sizeof(caddr);
		SOCKET c = accept(s, (sockaddr*)&caddr, &clen);
		MYOVERLAPPED* lpov = (MYOVERLAPPED*)calloc(1, sizeof(MYOVERLAPPED));
		lpov->wsabuf.buf = lpov->buffer;
		lpov->wsabuf.len = sizeof(lpov->buffer);
		lpov->c = c;

		clients[count++] = c;
		DWORD r, flag = 0;
		//khác với completionPort là phải truyền hàm gọi khi có dữ liệu đến RecvCB 
		WSARecv(c, &lpov->wsabuf, 1, &r, &flag, (WSAOVERLAPPED*)lpov, RecvCB);
	}
}