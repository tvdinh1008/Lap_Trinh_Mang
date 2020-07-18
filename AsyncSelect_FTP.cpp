// AsyncSelectExample.cpp : Defines the entry point for the application.
//
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "framework.h"
#include "AsyncSelectExample.h"
#include<WinSock2.h>

#define MAX_LOADSTRING 100
#define WM_ACCEPT WM_USER+1
#define WM_READ_CLOSE WM_USER+2

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

SOCKET s;
HWND hWnd;//
SOCKET clients[1024];
int status[1024]; //0 nếu đã disconnect, 1 nếu vẫn đang connect
int count = 0;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_ASYNCSELECTEXAMPLE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    WSADATA DATA;
    WSAStartup(MAKEWORD(2, 2), &DATA);

    s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN saddr;
    saddr.sin_family = AF_INET;
    saddr.sin_port = htons(21);
    saddr.sin_addr.S_un.S_addr = 0;//cho phép tất cả kết nối đến

    bind(s, (sockaddr*)&saddr, sizeof(saddr));
    listen(s, 10);

    //không gọi accept ở đây vì nó sẽ treo ở đây->sử dụng 
    //những sự kiện nào xẩy ra trên socket sẽ bị biến thành sư kiên trên cửa số hWnd
    //do trong này ta chỉ cần lấy sư kiện accept->moi kết noois đến cần accept sẽ phát sinh sự kiện vào cửa sổ với mã là WM_ACCEPT (ta tự định nghĩa)
    WSAAsyncSelect(s, hWnd, WM_ACCEPT, FD_ACCEPT);
    //chú ý 
    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_ASYNCSELECTEXAMPLE));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc; //dịa chỉ của hàm sẽ được gọi ra để xử lý sự kiện phát sinh trên cửa sổ
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ASYNCSELECTEXAMPLE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_ASYNCSELECTEXAMPLE);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance; // Store instance handle in our global variable

    hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu(xác định khi có nhiều cửa sổ - là cửa sổ nào)
//  WM_PAINT    - Paint the main window(sự kiên phát sinh -vẽ ấn menu, đóng)
//  WPARAM      ->tham số của sự kiện phát sinh trên vd chuột thì bấm ở đâu?
//  WM_DESTROY  - post a quit message and return
//
// các sự kiện phát sinh sẽ vào đây -> ta gán sự kiện của socket vào cửa sổ thì có sự kiện accept,recv,.. thì ta sử lý ở đây
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    SOCKET s, c;
    SOCKADDR_IN caddr;
    int clen = sizeof(caddr);
    char buffer[1024];
    switch (message)
    {
        //
    case WM_READ_CLOSE:
        c = (SOCKET)wParam;
        if (LOWORD(lParam) & FD_READ) //ktra xem là dữ liệu đến hay close
        {
            memset(buffer, 0, sizeof(buffer));
            recv(c, buffer, sizeof(buffer), 0);
            if (strstr(buffer, "USER"))
            {
                char* tmp = (char*)"331 Can password\r\n";
                send(c, tmp, strlen(tmp), 0);
            }
            
            if (strstr(buffer, "PASS"))
            {
                char* tmp = (char*)"230 Login Oki\r\n";
                send(c, tmp, strlen(tmp), 0);
            }
            if (strstr(buffer, "SYST"))
            {
                char* tmp = (char*)"215 FTP CUA DINH\r\n";
                send(c, tmp, strlen(tmp), 0);
            }
            
        }
        else if (LOWORD(lParam) & FD_CLOSE)
        {
            for (int i = 0; i < count; i++)
            {
                if (clients[i] == c)
                {
                    status[i] = 0;//kênh này đã disconnect
                    break;
                }
            }
            char* msg = (char*)"Da co 1 client roi zoom\n";
            for (int i = 0; i < count; i++)
            {
                if (clients[i] != c && status[i] == 1) //chỉ gửi cho client đang kết nối
                {
                    send(clients[i], msg, strlen(msg), 0);
                }
            }
        }
        break;
    case WM_ACCEPT:
        s = (SOCKET)wParam;
        //c là cái trao đổi dữ liệu vs client
        c = accept(s, (sockaddr*)&caddr, &clen);
        {
            char* tmp = (char*)"220 READY\r\n";
            send(c, tmp, strlen(tmp), 0);
            clients[count] = c;
            status[count] = 1;
            count++;
            //biến socket c thành nguồn sự kiện ánh xạ vào cửa sổ->sẽ ko bị treo khi trao đổi dữ liệu
            WSAAsyncSelect(c, hWnd, WM_READ_CLOSE, FD_READ | FD_CLOSE);//sau lệnh này nếu có dữ liệu trên socket c thì cửa sổ sẽ nhận đc thông điệp với mã là WM_READ
            //chú ý WSA AsyncSelect chỉ được gọi 1 lần -> ta cần | (or)  nếu viết 2 cái nó sẽ bị đè lên nhau(nó chỉ gọi cái cuối)
        }
        break;

        //
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam); //định danh của sự kiện
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
    }
    break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        // TODO: Add any drawing code that uses hdc here...
        EndPaint(hWnd, &ps);
    }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
