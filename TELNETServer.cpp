#define _CRT_SECURE_NO_WARNINGS
#include <iostream> 
#include<WinSock2.h>
#include<Windows.h>
using namespace std;


char* welcome = (char*)"Welcome to Telnet server\n";
char* signIn = (char*)"Input <account> <password>:\n";
char* success = (char*)"Sign in success !\n";
char* loss = (char*)"Login false !\n";
CRITICAL_SECTION cs;//đồng bô các luồng

DWORD WINAPI ClientThread(LPVOID param)
{
    SOCKET c = (SOCKET)param;
    send(c, welcome, strlen(welcome), 0);
    send(c, signIn, strlen(signIn), 0);
    char buffer[1024];
    char tmp[1024];
    char cmd[1024];
    while (true)  
    {
        memset(buffer, 0, sizeof(buffer));
        recv(c, buffer, sizeof(buffer), 0);
        //loại bỏ ký tự xuống dòng
        while (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
        {
            buffer[strlen(buffer) - 1] = 0;
        }
        bool check = false;
        FILE* f = fopen("check.txt", "rt");
        while (!feof(f))
        {
            memset(tmp, 0, sizeof(tmp));
            fgets(tmp, sizeof(tmp), f);
            while (tmp[strlen(tmp) - 1] == '\n' || buffer[strlen(tmp) - 1] == '\r')
            {
                tmp[strlen(tmp) - 1] = 0;
            }
            if (strcmp(tmp, "")==0) continue;
            if (strcmp(tmp, buffer) == 0) {
                check = true;
                break;
            }
        }
        fclose(f);
        if (check)
        {
            send(c, success, strlen(success), 0);
            while (true)
            {
                memset(buffer, 0, sizeof(buffer));
                memset(cmd, 0, sizeof(cmd));
                recv(c, buffer, sizeof(buffer), 0);
                while (buffer[strlen(buffer) - 1] == '\n' || buffer[strlen(buffer) - 1] == '\r')
                {
                    buffer[strlen(buffer) - 1] = 0;
                }
                cout << "SOCKET: " << c << "  " << buffer << endl;
                

                //ghi vào file out.txt->để tránh xung đột->sử dụng blocking
                EnterCriticalSection(&cs);
                sprintf(cmd, "%s>out.txt", buffer);
                system(cmd);

                f = fopen("out.txt", "rt");
                while (!feof(f))
                {
                    memset(buffer, 0, sizeof(buffer));
                    fgets(buffer, sizeof(buffer), f);//đọc từng dòng 1
                    send(c, buffer, strlen(buffer), 0);
                }
                fclose(f);
                LeaveCriticalSection(&cs);
                send(c, "\n", 1, 0);
            }
        }
        send(c, loss, strlen(loss), 0);
        send(c, signIn, strlen(signIn), 0);
    }
    return 0;
}
int main(int argc,char*argv[])
{
    InitializeCriticalSection(&cs);
    WSADATA data;
    WSAStartup(MAKEWORD(2, 2), &data);

    //s là server: socket s là socket để tiếp nhận các kết nối đến
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //lưu địa chỉ server
    SOCKADDR_IN saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(atoi(argv[1]));
    saddr.sin_port = htons(_wtoi((wchar_t*)argv[1]));
    saddr.sin_addr.S_un.S_addr = htons(INADDR_ANY);//chấp nhận tất cả các kết nối đến
    //gắn socket lên card mạng
    bind(s, (sockaddr*)&saddr, sizeof(saddr));
    //lắng nghe các kết nối đến
    listen(s, 10);

    //tạo socket kết nối trao đổi dữ liệu với client
    SOCKADDR_IN caddr;
    int clen = sizeof(caddr);

    while (true) //lặp đợi kết nối liên tục->cần tạo luồng trao đổi với mỗi kết nối
    {
        DWORD id = 0;
        SOCKET c = accept(s, (sockaddr*)&caddr, &clen); //accept thành công -> tao luồng mới xong lại quay lại chờ kết nối tiếp
        CreateThread(NULL, 0, ClientThread, (LPVOID)c, 0, &id);
        //char* buff = new char[1024];
    
    }
    DeleteCriticalSection(&cs);
    closesocket(s);
    WSACleanup();
    return 0;
}