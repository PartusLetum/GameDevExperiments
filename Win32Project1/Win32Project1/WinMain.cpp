#include <stdio.h>
#include <windows.h>

HDC bufDC;
HBITMAP bufBMP;
RECT rect;
bool isRunning;

void DrawLine(HDC hdc, COLORREF color, int x1, int y1, int x2, int y2)
{
	HPEN hPen = CreatePen(PS_SOLID, 3, color);
	SelectObject(hdc, hPen);
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
	DeleteObject(hPen);
}

void Render(HDC hdc)
{
	int x1, y1, x2, y2;
	x1 = rand() % 200;
	y1 = rand() % 200;
	x2 = rand() % 200;
	y2 = rand() % 200;
	COLORREF color = RGB(x1 % 255, x1 % 255, x1 % 255);
	//color = RGB(100, 0, 0);
	SetPixel(bufDC, x1, y1, color);  // random
	//DrawLine(bufDC, color, x1, y1, x2, y2);
	BitBlt(hdc, 0, 0, rect.right, rect.bottom, bufDC, 0, 0, SRCCOPY);
}

void PaintWindow(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hwnd, &ps);
	Render(hdc);
	EndPaint(hwnd, &ps);
}

DWORD RenderThread(HWND hwnd)
{
	HDC hdc = GetDC(hwnd);

	while (isRunning)
	{
		Render(hdc);
	}

	return 0;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN: // same as pressing the X button:
	case WM_CLOSE:   DestroyWindow(hwnd); isRunning = false; PostQuitMessage(0); return 0;
	case WM_DESTROY: PostQuitMessage(0);  return 0;
	case WM_PAINT:   PaintWindow(hwnd);   return 0;
	case WM_ERASEBKGND: return (LRESULT)1;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
	// Register window class
	WNDCLASSA wc =
	{
		0, WndProc, 0, 0, 0,
		LoadIcon(NULL, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW),
		(HBRUSH)GetStockObject(BLACK_BRUSH), // background color == black
		NULL, // no menu
		"ExampleWindowClass"
	};

	ATOM wClass = RegisterClassA(&wc);
	if (!wClass)
	{
		fprintf(stderr, "%s\n", "Couldn’t create Window Class");
		return 1;
	}

	// Create the window
	HWND hwnd = CreateWindowA(
		MAKEINTATOM(wClass),
		"Window sample",     // window title
		WS_OVERLAPPEDWINDOW, // title bar, thick borders, etc.
		CW_USEDEFAULT, CW_USEDEFAULT, 320, 240,
		NULL, // no parent window
		NULL, // no menu
		GetModuleHandle(NULL),  // EXE's HINSTANCE
		NULL  // no magic user data
	);
	if (!hwnd)
	{
		fprintf(stderr, "%ld\n", GetLastError());
		fprintf(stderr, "%s\n", "Failed to create Window");
		return 1;
	}

	// Make window visible
	ShowWindow(hwnd, SW_SHOWNORMAL);

	GetClientRect(hwnd, &rect);

	bufDC = CreateCompatibleDC(GetDC(hwnd));
	bufBMP = CreateCompatibleBitmap(GetDC(hwnd), rect.right, rect.bottom);
	SelectObject(bufDC, bufBMP);

	isRunning = false;
	HANDLE hRenderThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RenderThread, hwnd, 0, NULL);
	isRunning = true;

	// Event loop
	MSG msg;
	while (isRunning)
	{
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			// Translate and dispatch message
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (msg.message == WM_QUIT)
		{
			isRunning = false;
			break;
		}

		// Do update, rendering and all the real game loop stuff
		Sleep(1);
	}

	return msg.wParam;
}