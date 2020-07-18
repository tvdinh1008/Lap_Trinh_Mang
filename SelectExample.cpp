//#define FD_SETSIZE 128 //ok tăng từ 64 lên 128
#include<stdio.h>
#include<WinSock2.h>
//#define FD_SETSIZE 128 không dc do trình dịch ta phải đặt bên trên
#define MAX_SOCKET_PER_THREAD 2

//mảng chứa socket client kết nối vs server
SOCKET g_client[1024];
int g_count=0;

//luồng này sẽ đc tạo ra nếu quá MAX_SOCKET_PER_THREAD
//Tách luồng và xử lý
DWORD WINAPI ClientThread(LPVOID param)
{
	fd_set read;
	int startIndex = (int)param;//chuyền chỉ số

	int threadID = GetCurrentThreadId();
	while (true)
	{
		FD_ZERO(&read);
		int number = 0;
		for (int i = startIndex; i < g_count && number < MAX_SOCKET_PER_THREAD; i++)
		{
			printf("Thread %d: add socket %d (%d) to read set\n", threadID,i, g_client[i]);
			FD_SET(g_client[i], &read);
			number += 1;
		}
		//cần timeout để khi luồng A có thêm 1 thằng khác kết nối đến->nó cần đc bổ sung vào luồng đang chạy này
		timeval t;
		t.tv_sec = 1;
		t.tv_usec = 0;
		select(0, &read, NULL, NULL,&t);//kết thúc thăm dò sau 1 s

		for (int i = startIndex; i < g_count&& number<MAX_SOCKET_PER_THREAD; i++,number++)
		{
			if (FD_ISSET(g_client[i], &read))
			{
				char buffer[1024];
				memset(buffer, 0, sizeof(buffer));
				//ta chỉ goi nhận khi đã chắc chắn có dữ liệu đến -->không bị treo
				recv(g_client[i], buffer, sizeof(buffer), 0);
				printf("socket %d:  %s\n", g_client[i], buffer);

				for (int j = 0; j < g_count; j++)
				{
					if (i != j)
					{
						send(g_client[j], buffer, strlen(buffer), 0);
					}
				}

			}
		}
	}
	return 0;
}


int main()
{
	WSADATA DATA;
	WSAStartup(MAKEWORD(2, 2), &DATA);
	SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct sockaddr_in saddr;
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8888);
	saddr.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(s, (sockaddr*)&saddr, sizeof(saddr));
	listen(s, 10);

	fd_set read;//là struck gồm: mảng sock tối đa 64  và fd_count(max 64)

	/*
		c = accept(s, (sockaddr*)&caddr, &clen);
		//chú ý accept, recv,send đều treo cho đến khi nhận sự kiện
		*/
	while (true)
	{
		FD_ZERO(&read);//đặt fd_count=0 tức không có sock nào cả, xóa sạch tập thăm dò đi
		FD_SET(s, &read);//thêm socket s vào read và tăng fd_count++
		
		for (int i = 0; i < g_count && i< MAX_SOCKET_PER_THREAD; i++)
		{
			printf("Thread %d: add socket %d (%d) to read set\n", GetCurrentThreadId(),i, g_client[i]);
			FD_SET(g_client[i], &read);
		}
		//treo ở đây đến khi nào có socket trong mảng read có sự kiên mới qua
		select(0, &read, NULL, NULL, NULL);//thăm dò

		//--> accept sẽ không bị treo nữa vì đã có sự kiện
		if (FD_ISSET(s, &read))//chỉ có sự kiện kết nối đến với s thôi vì s luôn nhận kết nối
		{
			SOCKET c;
			struct sockaddr_in caddr;
			int clen = sizeof(caddr);
			c = accept(s, (sockaddr*)&caddr, &clen);
			//ta chỉ gọi accept khi sự kiện đã có -->sẽ không bị treo nữa
			printf("sokect %d connected\n",c);
			g_client[g_count] = c;
			g_count++;
			if (g_count>1 &&(g_count-1) % MAX_SOCKET_PER_THREAD == 0)//tạo luồng vì tràn->tạo 1 thread mới
			{
				DWORD ID=0;
				CreateThread(NULL, 0, ClientThread, (LPVOID)(g_count - 1), 0, &ID);//chỉ số tính từ 0
			}
		}
		//TH khác có dữ liệu đến mà cũng có thể là sự kiện kết nối
		for (int i = 0; i < g_count; i++)
		{
			//nhận dữ liệu gửi đến g_client[i] và gửi lại cho tất cả g_client khác
			if (FD_ISSET(g_client[i], &read))
			{
				char buffer[1024];
				memset(buffer, 0, sizeof(buffer));
				//ta chỉ goi nhận khi đã chắc chắn có dữ liệu đến -->không bị treo
				recv(g_client[i], buffer, sizeof(buffer), 0);
				printf("socket %d:  %s\n", g_client[i], buffer);

				for (int j = 0; j < g_count; j++)
				{
					if (i != j)
					{
						send(g_client[j], buffer, strlen(buffer), 0);
					}
				}

			}
		}
	}

	return 0;
}