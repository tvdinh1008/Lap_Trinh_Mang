#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include<WinSock2.h>






//quét toàn bộ thư muc gốc của server giả sử là ở ổ c: của máy 

//char g_html[1024];//nếu có nhiều thư mục trong 1 thư mục -->tràn bộ đệm giải pháp là cấp phát động realoc thì truyền biến toàn cục
char* g_html=NULL;

void sendFile(SOCKET c,char * path)
{
	char header[1024];
	memset(header, 0, sizeof(header));
	//strcpy(header,"HTTP/1.1 200 OK\nServer: LTM\nContent-Type: application/octet-stream\n");
	strcpy(header, "HTTP/1.1 200 OK\nServer: LTM\n");
	char* pattern = strstr(path, "_###");
	pattern[0] = 0;
	char buffer[1024];
	

	if (stricmp(path + strlen(path) - 4, ".jpg") == 0)
	{
		sprintf(header+strlen(header),"Content-Type: image/jpeg\n");
	}
	else if (stricmp(path + strlen(path) - 4, ".mp4") == 0)
	{
		sprintf(header + strlen(header), "Content-Type: video/mp4\n");
	}
	else if (stricmp(path + strlen(path) - 4, ".mp3") == 0)
	{
		sprintf(header + strlen(header), "Content-Type: audio/mp3\n");
	}
	else
	{
		sprintf(header + strlen(header), "Content-Type: application/octet-stream\n");
	}

	send(c, header, strlen(header), 0);
	FILE* f = fopen(path, "rb");
	fseek(f, 0, SEEK_END);
	int flen = ftell(f);
	fseek(f, 0, SEEK_SET);
	sprintf(header + strlen(header), "Content-Lenght:%d\n\n",flen);

	while (!feof(f))
	{
		int r = fread(buffer, 1,sizeof(buffer), f);
		send(c, buffer, strlen(buffer), 0);
	}
}





//C:%s\\*.* nếu để ntn mà chuyền vào thẻ a nó sẽ mất đường link: vd 127.0.0.1/*.* nếu là thư mục gốc còn 127.0.0.1/temp/*.* nếu là thư mục temp
//nó hiểu là file:/C://....

//PATH chỉ có thể là "/" nếu nó là thư mục gốc (ta sẽ tìm C:/*.*)
//hoặc PATH là "/temp/x" tức là ta sẽ dùng biến truyền vào C:PATH/*.* hay C://temp/x/*.*
void ScanFolder(char* path)//đọc từ ổ nào??(path)
{
	char fullPath[1024];
	memset(fullPath, 0, sizeof(fullPath)); //*.* lấy tất cả các tên, lấy tất cả các đuôi
	if (path[strlen(path) - 1] == '/')
	{
		sprintf(fullPath, "C:%s*.*", path);//TH1 "/" -->kqua "C:/*.*" OKI
	}
	else
	{
		sprintf(fullPath, "C:%s/*.*", path);//TH1 "/temp" -->kqua "C:/temp/*.*" OKI
	}



	/*
	char fullPath[1024];
	memset(fullPath, 0, sizeof(fullPath));
	if (path[strlen(path) - 1] == '/')
	{
		sprintf(fullPath, "C:%s*.*", path);// C:\\*.* tìm tất cả
	}
	else
	{
		sprintf(fullPath, "C:%s/*.*", path);// C:\\*.* tìm tất cả
	}
	*/
	
	WIN32_FIND_DATAA DATA;// a là ascii
	HANDLE hFind=FindFirstFileA(fullPath,&DATA); // a là ascii. 

	char tmpHtml[1024];
	memset(tmpHtml, 0, sizeof(tmpHtml));

	if (DATA.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)//and bit để ktra nó là thư mục(không phải file)
	{
		if (path[strlen(path) - 1] == '/')
		{
			sprintf(tmpHtml, "<a href=\"%s%s\"><b>%s</b></a><br>", path, DATA.cFileName, DATA.cFileName);//thư mục chữ in đậm
		}
		else
		{
			sprintf(tmpHtml, "<a href=\"%s/%s\"><b>%s</b></a><br>", path, DATA.cFileName, DATA.cFileName);
		}
	}
	else //nó là file + _### để phân biệt
	{
		if (path[strlen(path) - 1] == '/')
		{

			sprintf(tmpHtml, "<a href=\"%s%s_###\">%s</a><br>", path, DATA.cFileName, DATA.cFileName);//file chữ thường ko đậm
		}
		else
		{
			sprintf(tmpHtml, "<a href=\"%s/%s_###\">%s</a><br>", path, DATA.cFileName, DATA.cFileName);
		}
	}
	int oldlen = g_html != NULL ? strlen(g_html): 0;
	g_html = (char*)realloc(g_html, oldlen + strlen(tmpHtml) + 1);
	sprintf(g_html + oldlen, "%s", tmpHtml);


	if (hFind != INVALID_HANDLE_VALUE)
	{
		while (FindNextFileA(hFind, &DATA) == true)
		{
			memset(tmpHtml, 0, sizeof(tmpHtml));

			//	<a href="/(tên file or thư mục)">file name</a>

			//duyệt tất cả các thư muc,file còn lại gắn vào xâu g_html
			if (DATA.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)//and bit để ktra nó là thư mục(không phải file)
			{
				if (path[strlen(path) - 1] == '/')
				{
					sprintf(tmpHtml, "<a href=\"%s%s\"><b>%s</b></a><br>", path, DATA.cFileName, DATA.cFileName);//tìm file đầu tiên trong thư muc path gắn vào xâu g_html
				}
				else
				{
					sprintf(tmpHtml, "<a href=\"%s/%s\"><b>%s</b></a><br>", path, DATA.cFileName, DATA.cFileName);
				}
			}
			else //nó là file + _### để phân biệt
			{
				if (path[strlen(path) - 1] == '/')
				{
					sprintf(tmpHtml, "<a href=\"%s%s_###\">%s</a><br>", path, DATA.cFileName, DATA.cFileName);
				}
				else
				{
					sprintf(tmpHtml, "<a href=\"%s/%s_###\">%s</a><br>", path, DATA.cFileName, DATA.cFileName);
				}
			}
			int oldlen = g_html != NULL ? strlen(g_html): 0;
			g_html = (char*)realloc(g_html, oldlen + strlen(tmpHtml) + 1);
			sprintf(g_html + oldlen, "%s", tmpHtml);
		}
	}

}


DWORD WINAPI ClientThread(LPVOID param)
{
	SOCKET c = (SOCKET)param;
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	recv(c, buffer, sizeof(buffer), 0);
	
	if (g_html != NULL)
	{
		free(g_html);
		g_html = NULL;
	}

	//memset(g_html, 0, sizeof(g_html));
	g_html = (char*)calloc(1024, 1);//cấp phát động


	sprintf(g_html, "<html><h1>SIMPLE HTTP FILE SERVER</h1><br>");
	char ACTION[1024];
	char PATH[1024];
	char VERSION[1024];
	memset(ACTION, 0,sizeof(ACTION));//GET
	memset(PATH, 0, sizeof(PATH));//Thư mục gốc vd: / , temp/ ,...
	memset(VERSION, 0, sizeof(VERSION));// HTTP/1.1
	//tách 3 phần do:request từ trang web(buffer) là: GET / HTTP/1.1 sử dụng sscanf (đọc bỏ qua các dấu cách
	sscanf(buffer, "%s%s%s", ACTION, PATH, VERSION);

	/*
	char realPath[1024];
	memset(realPath, 0, sizeof(realPath));
	sprintf(realPath, "C:%s\\*.*",PATH);
	*/
	while (strstr(PATH, "%20") != NULL)
	{
		//	PATH     found
		//    v      v
		//vd: virtual%20machine (%20 có 3 ký tự thay bằng " " do trình duyệt mã hóa dấu cách thành %20)
		char* found = strstr(PATH, "%20");
		found[0] = ' ';
		strcpy(found + 1, found + 3);
	}
	//nó là file
	if (strstr(PATH, "_###")!=NULL)
	{
		sendFile(c,PATH);
	}
	else {
		//char* http = (char*)"<html><h1>SIMPLE HTTP FILE SERVER</h1></html>";
		char* welcome = (char*)"HTTP/1.1 200 OK\nServer: LTM\nContent-Type: text/html\n\n";
		send(c, welcome, strlen(welcome), 0);

		char* input = (char*)"<form method=\"POST\"> <input type=\"file\"><input type=\"submit\"></form><br>";
		send(c, input, strlen(input), 0);

		ScanFolder(PATH);

		g_html = (char*)realloc(g_html, strlen(g_html) + 8);
		sprintf(g_html + strlen(g_html), "</html>");

		send(c, g_html, strlen(g_html), 0);
	}
	closesocket(c);
	return 0;
}

int main()
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

	while (true)
	{
		DWORD id = 0;
		SOCKADDR_IN caddr;
		int clen = sizeof(caddr);
		SOCKET c=accept(s, (sockaddr*)&caddr, &clen);
		//muốn truyền nhiều tham số hơn thì dùng struct
		CreateThread(, 0, ClNULLientThread, (LPVOID)c, 0, &id);

	}
}
