// ScreenCapture.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "ScreenCapture.h"

#define MAX_LOADSTRING 100

// 全局变量: 
HINSTANCE hInst;								// 当前实例
TCHAR szTitle[MAX_LOADSTRING];					// 标题栏文本
TCHAR szWindowClass[MAX_LOADSTRING];			// 主窗口类名
int g_ScreenX, g_ScreenY;						//屏幕分辨率
HDC g_hMemDc;									//正常图像的内存兼容DC	
HDC g_hGrayMemDc;								//灰度图像的内存兼容DC

RECT g_rtMouse;									//鼠标选定的一个矩形
BOOL g_bMouseDown = FALSE;						//鼠标左键按下
BOOL g_bMouseUp = FALSE;						//鼠标左键抬起

BOOL g_bIsRect = FALSE;							//矩形区域已选定
HWND g_MainWnd;									//截图窗口句柄
// 此代码模块中包含的函数的前向声明: 
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//消息处理函数
LRESULT CALLBACK OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OnLButtonDBClick(HWND hWnd, WPARAM wParam, LPARAM lParam);

BOOL CALLBACK OnDlgCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
//功能函数
void ConvertToGrayBitmap(HDC hSrcDc, HBITMAP hSrcBitmap);
void WriteDatatoClipBoard();
void ScreenCapture();

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO:  在此放置代码。
	MSG msg;
	HACCEL hAccelTable;

	// 初始化全局字符串
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_SCREENCAPTURE, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化: 
	HWND hMainWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_MAIN_DLG), NULL, DialogProc);
	ShowWindow(hMainWnd, nCmdShow);
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SCREENCAPTURE));

	// 主消息循环: 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  函数:  MyRegisterClass()
//
//  目的:  注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SCREENCAPTURE));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   函数:  InitInstance(HINSTANCE, int)
//
//   目的:  保存实例句柄并创建主窗口
//
//   注释: 
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // 将实例句柄存储在全局变量中

   hWnd = CreateWindow(szWindowClass, szTitle, WS_POPUP,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, SW_MAXIMIZE);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  函数:  WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目的:    处理主窗口的消息。
//
//  WM_COMMAND	- 处理应用程序菜单
//  WM_PAINT	- 绘制主窗口
//  WM_DESTROY	- 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// 分析菜单选择: 
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
		break;
	case WM_CREATE:
		{
		g_MainWnd = hWnd;
			ScreenCapture();
		}
		break;
	case WM_PAINT:
		return OnPaint(hWnd, wParam, lParam);
	case WM_CLOSE:
		CloseWindow(hWnd);
		break;
	case WM_MOUSEMOVE:
		return OnMouseMove(hWnd, wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(hWnd, wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(hWnd, wParam, lParam);
	case WM_LBUTTONDBLCLK:
		return OnLButtonDBClick(hWnd, wParam, lParam);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
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
//WM_PAINT消息处理函数
LRESULT CALLBACK OnPaint(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	POINT ptPenWidth = { 2, 2 };
	LOGPEN logpen = { 0 };
	HDC hdc;

	//初始化画笔
	logpen.lopnColor = RGB(0, 0, 255);
	logpen.lopnStyle = PS_SOLID;
	logpen.lopnWidth = ptPenWidth;
	HPEN hPen = CreatePenIndirect(&logpen);
	
	//初始化画刷
	LOGBRUSH logbrush = { 0 };
	logbrush.lbStyle = BS_NULL;
	HBRUSH hBrush = CreateBrushIndirect(&logbrush);
	hdc = BeginPaint(hWnd, &ps);
	SelectObject(hdc, hPen);
	SelectObject(hdc, hBrush);
	
	//将灰度图像放到内存兼容dc中，这个是兼容该应用程序的dc
	HDC memDc = CreateCompatibleDC(hdc);
	HBITMAP bmp = CreateCompatibleBitmap(hdc, g_ScreenX, g_ScreenY);
	SelectObject(memDc, bmp);

	//将灰度图像绘制到这个兼容DC中
	BitBlt(memDc, 0, 0, g_ScreenX, g_ScreenX, g_hGrayMemDc, 0, 0, SRCCOPY);
	SelectObject(memDc, hBrush);
	SelectObject(memDc, hPen);

	if (g_bMouseDown || g_bIsRect)
	{
		//将正常图像的部分绘制到内存dc中，相当于在之前的画布上贴上一个正常的图像
		BitBlt(memDc, g_rtMouse.left, g_rtMouse.top, g_rtMouse.right - g_rtMouse.left, g_rtMouse.bottom - g_rtMouse.top, g_hMemDc, g_rtMouse.left, g_rtMouse.top, SRCCOPY);
		Rectangle(memDc, g_rtMouse.left, g_rtMouse.top, g_rtMouse.right, g_rtMouse.bottom);
	}
	//将整个图像贴到程序上作为背景
	BitBlt(hdc, 0, 0, g_ScreenX, g_ScreenY, memDc, 0, 0, SRCCOPY);

	DeleteObject(bmp);
	DeleteObject(memDc);
	EndPaint(hWnd, &ps);
	return 0;
}
//截屏函数, 并将其绘制到程序的窗口上
void ScreenCapture()
{
	HDC hDc = CreateDC(_T("DISPLAY"), NULL, NULL, NULL);  //创建屏幕截图的DC
	g_ScreenX = GetDeviceCaps(hDc, HORZRES);   //获取屏幕的水平分辨率
	g_ScreenY = GetDeviceCaps(hDc, VERTRES);   //获取屏幕的垂直分辨率
	
	g_hMemDc = CreateCompatibleDC(hDc);        //创建兼容的内存DC
	HBITMAP hBitMap = CreateCompatibleBitmap(hDc, g_ScreenX, g_ScreenY); //创建兼容的bitmap
	SelectObject(g_hMemDc, hBitMap);
	BitBlt(g_hMemDc, 0, 0, g_ScreenX, g_ScreenY, hDc, 0, 0, SRCCOPY); //将屏幕截图保存到内存DC中
	
	g_hGrayMemDc = CreateCompatibleDC(hDc);
	HBITMAP hGrayBitmap = CreateCompatibleBitmap(hDc, g_ScreenX, g_ScreenY);
	SelectObject(g_hGrayMemDc, hGrayBitmap);
	BitBlt(g_hGrayMemDc, 0, 0, g_ScreenX, g_ScreenY, hDc, 0, 0, SRCCOPY);

	ConvertToGrayBitmap(g_hGrayMemDc, hGrayBitmap);

	DeleteObject(hDc);
	DeleteObject(hBitMap);
	DeleteObject(hGrayBitmap);
}

void ConvertToGrayBitmap(HDC hSrcDc, HBITMAP hSrcBitmap)
{
	HBITMAP retBmp = hSrcBitmap;
	BITMAPINFO bmpInfo;
	ZeroMemory(&bmpInfo, sizeof(BITMAPINFO));
	bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	GetDIBits(hSrcDc, retBmp, 0, 0, NULL, &bmpInfo, DIB_RGB_COLORS);

	BYTE* bits = new BYTE[bmpInfo.bmiHeader.biSizeImage];
	GetBitmapBits(retBmp, bmpInfo.bmiHeader.biSizeImage, bits);

	int bytePerPixel = 4;//默认32位
	if (bmpInfo.bmiHeader.biBitCount == 24)
	{
		bytePerPixel = 3;
	}
	for (DWORD i = 0; i<bmpInfo.bmiHeader.biSizeImage; i += bytePerPixel)
	{
		BYTE r = *(bits + i);
		BYTE g = *(bits + i + 1);
		BYTE b = *(bits + i + 2);
		*(bits + i) = *(bits + i + 1) = *(bits + i + 2) = (r + b + g) / 3;
	}
	SetBitmapBits(hSrcBitmap, bmpInfo.bmiHeader.biSizeImage, bits);
	delete[] bits;
}

LRESULT CALLBACK OnLButtonDown(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (!g_bMouseDown && !g_bIsRect)
	{
		g_bMouseDown = TRUE;
		POINT pt = { 0 };
		GetCursorPos(&pt);
		g_rtMouse.left = pt.x;
		g_rtMouse.top = pt.y;
		g_rtMouse.right = pt.x;
		g_rtMouse.bottom = pt.y;
		InvalidateRgn(hWnd, 0, FALSE);
	}

	return 0;
}

LRESULT CALLBACK OnLButtonUp(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (g_bMouseDown)
	{
		g_bMouseUp = TRUE;
		g_bMouseDown = FALSE;
		g_bIsRect = TRUE;
	}
	return 0;
}

LRESULT CALLBACK OnMouseMove(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (g_bMouseDown)
	{
		POINT pt = { 0 };
		GetCursorPos(&pt);
		g_rtMouse.right = pt.x;
		g_rtMouse.bottom = pt.y;
		InvalidateRgn(hWnd, 0, FALSE);
	}
	return 0;
}

LRESULT CALLBACK OnLButtonDBClick(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (g_bIsRect)
	{
		WriteDatatoClipBoard();
		ZeroMemory(&g_rtMouse, sizeof(POINT));
		g_bIsRect = FALSE;
		g_bMouseDown = FALSE;
		g_bMouseUp = FALSE;
		InvalidateRgn(hWnd, 0, FALSE);
	}
	return 0;
}

void WriteDatatoClipBoard()
{
	HDC hMemDc, hScrDc;
	HBITMAP hBmp, hOldBmp;
	int width, height;
	width = g_rtMouse.right - g_rtMouse.left;
	height = g_rtMouse.bottom - g_rtMouse.top;

	hScrDc = CreateDC(L"DISPLAY", NULL, NULL, NULL);
	hMemDc = CreateCompatibleDC(hScrDc);
	hBmp = CreateCompatibleBitmap(hScrDc, width, height);

	//这几步利用SelectObject会返回原始GDI对象的特性，从DC中获取对应位图的信息
	hOldBmp = (HBITMAP)SelectObject(hMemDc, hBmp);
	//将截取部分的图片放入到画布中
	BitBlt(hMemDc, 0, 0, width, height, g_hMemDc, g_rtMouse.left, g_rtMouse.top, SRCCOPY);
	hBmp = (HBITMAP)SelectObject(hMemDc, hOldBmp);

	DeleteDC(hMemDc);
	DeleteDC(hScrDc);
	//复制到剪贴板
	if (OpenClipboard(0))
	{
		EmptyClipboard();
		SetClipboardData(CF_BITMAP, hBmp);
		CloseClipboard();
	}

	DeleteObject(hBmp);
	DeleteObject(hMemDc);
	DeleteObject(hScrDc);
}

BOOL CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	case WM_COMMAND:
		return OnDlgCommand(hWnd, wParam, lParam);
	case WM_CLOSE:
		CloseWindow(hWnd);
		SendMessage(hWnd, WM_DESTROY, 0, 0);
		return FALSE;
	default:
		return FALSE;
	}
}

BOOL CALLBACK OnDlgCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int wmEvent = HIWORD(wParam);

	if (wmEvent == BN_CLICKED)
	{
		if (wmId == IDC_NEW)
		{
			if (!IsWindow(g_MainWnd))
			{
				//截图窗口不存在，则显示这个窗口
				if (!InitInstance(hInst, SW_NORMAL))
				{
					return FALSE;
				}
			}
		}

		if (wmId == IDCANCEL)
		{
			if (IsWindow(g_MainWnd))
			{
				//截图窗口存在则关闭截图窗口
				SendMessage(g_MainWnd, WM_CLOSE, 0, 0);
				return TRUE;
			}
		}
		return TRUE;
	}
	return FALSE;
}