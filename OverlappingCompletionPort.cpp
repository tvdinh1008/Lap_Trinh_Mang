#include <stdio.h>
#include <WinSock2.h>
#define SEND 0
#define RECV 1

typedef struct
{
	WSABUF wsabuf;
	char buffer[1024];
	SOCKET c;
} PER_HANDLE_DATA;

typedef struct
{
	WSAOVERLAPPED ov;
	int oper;//=0 nếu là gửi =1 nếu là nhận để check khi quét port trong luồng
} PER_IO_DATA;

SOCKET clients[1024];
int count = 0;
HANDLE CompletionPort = INVALID_HANDLE_VALUE;

DWORD WINAPI CompletionThread(LPVOID arg)
{
	HANDLE port = (HANDLE)arg;
	PER_HANDLE_DATA* lp_per_handle_data = NULL;
	PER_IO_DATA* lp_per_io_data = NULL;
	while (0 == 0)
	{
		DWORD BytesTransferred = 0;
		//ktra trạng thái port
		BOOL r = GetQueuedCompletionStatus(port, &BytesTransferred, (PULONG_PTR)&lp_per_handle_data, (LPOVERLAPPED*)&lp_per_io_data, INFINITE);
		if (r)
		{
			if (BytesTransferred == 0)
			{
				//closesocket
				//free per_handle_data
				//free per_io_data
				printf("Giai phong bo nho o day\n");
			}
			else
			{
				if (lp_per_io_data->oper == RECV)
				{
					//gửi dữ liệu cho tất cả client khác
					for (int i = 0; i < count; i++)
					{
						if (clients[i] != lp_per_handle_data->c)
						{
							DWORD ByteSent = 0;
							//GlobalAlloc gửi xong ->cần free lần khác gọi nó là lp_per_io_data ở GetQueuedCompletionStatus
							PER_IO_DATA* lp_tmp_io_data = (PER_IO_DATA*)GlobalAlloc(GPTR, sizeof(PER_IO_DATA));
							memset(&(lp_tmp_io_data->ov), 0, sizeof(WSAOVERLAPPED));
							lp_tmp_io_data->oper = SEND;
							lp_per_handle_data->wsabuf.len = strlen(lp_per_handle_data->buffer);
							WSASend(clients[i], &(lp_per_handle_data->wsabuf), 1, &ByteSent, 0, (LPWSAOVERLAPPED)lp_tmp_io_data, NULL);
						}
					}
					//tiếp tục nhận dữ liệu qua WSARecv 
					//và nó cần cấu trúc overlapp mà cấu trúc này phải làm sạch(memset vì nó đã là cấu trúc đã hoành thành trc rồi)
					DWORD ByteRecv = 0, Flags = 0;
					memset(lp_per_io_data, 0, sizeof(WSAOVERLAPPED));
					lp_per_io_data->oper = RECV;
					lp_per_handle_data->wsabuf.len = sizeof(lp_per_handle_data->buffer);
					WSARecv(lp_per_handle_data->c, &(lp_per_handle_data->wsabuf), 1, &ByteRecv, &Flags, (LPWSAOVERLAPPED)lp_per_io_data, NULL);
				}
				if (lp_per_io_data->oper == SEND)
				{
					printf("Da gui du lieu den socket: %d\n", lp_per_handle_data->c);
					GlobalFree(lp_per_io_data);
					lp_per_io_data = NULL;
				}
			}
		}
	}
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
	//
	CompletionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, 0, 0, 0);
	
	//tạo 4 luồng quét port
	for (int i = 0; i < 4; i++)
	{
		CreateThread(NULL, 0, CompletionThread, CompletionPort, 0, NULL);
	}
	while (0 == 0)
	{
		SOCKADDR_IN caddr;
		int clen = sizeof(caddr);
		SOCKET c = WSAAccept(s, (sockaddr*)&caddr, &clen, NULL, NULL);
		clients[count++] = c;
		//GlobalAlloc cấp phát vùng nhớ hệ điều hành( dùng chung bộ nhớ). Calloc tạo ra 1 virtual memory
		PER_HANDLE_DATA* lp_per_handle_data = (PER_HANDLE_DATA*)GlobalAlloc(GPTR, sizeof(PER_HANDLE_DATA));
		lp_per_handle_data->wsabuf.buf = lp_per_handle_data->buffer;
		lp_per_handle_data->wsabuf.len = sizeof(lp_per_handle_data->buffer);
		lp_per_handle_data->c = c;

		//khai báo cấu trúc overlapping
		PER_IO_DATA* lp_per_io_data = (PER_IO_DATA*)GlobalAlloc(GPTR, sizeof(PER_IO_DATA));
		CreateIoCompletionPort((HANDLE)c, CompletionPort, (ULONG_PTR)lp_per_handle_data, 0);
		DWORD BytesRecv = 0, Flags = 0;
		memset(&(lp_per_io_data->ov), 0, sizeof(WSAOVERLAPPED));
		lp_per_io_data->oper = RECV;
		//khác với completionRoutine là truyền hàm =NULL vì ta sử dụng port để quét có dữ liệu đến
		WSARecv(c, &(lp_per_handle_data->wsabuf), 1, &BytesRecv, &Flags, &(lp_per_io_data->ov), NULL);
	}
}