//
// http://real3d.pk/opengl_tuto_3.html
//

#include <stdio.h>
#include <windows.h>

HDC bufDC;
HBITMAP bufBMP;
RECT rect;
bool isRunning;

struct Point3d { float x, y, z; };
struct Face3d { unsigned int v1, v2, v3; };

const int vertexCount = 3;		// Number of vertices in model
Point3d vertices[3];			// Vertices of the model

const int faceCount = 1;		// Number of faces in model
Face3d faces[faceCount];		// Faces that make up the model (indicies into vertices)
Point3d faceNormals[faceCount];	// Normals to the faces that make up the model

void LoadModel()
{
	vertices[0].x = -50; vertices[0].y = -50; vertices[0].z = 0;
	vertices[1].x =  50; vertices[1].y =  50; vertices[1].z = 0;
	vertices[2].x =  50; vertices[2].y = -50; vertices[2].z = 0;

	faces[0].v1 = 0; faces[0].v2 = 1; faces[0].v3 = 2;
}

void DrawLine(HDC hdc, COLORREF color, int x1, int y1, int x2, int y2)
{
	HPEN hPen = CreatePen(PS_SOLID, 1, color);
	SelectObject(hdc, hPen);
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
	DeleteObject(hPen);
}

void DrawModel(HDC hdc)
{
	for (int i = 0; i < faceCount; i++)
	{
		COLORREF color = RGB(255, 255, 255);
		//color = RGB(100, 0, 0);
		DrawLine(bufDC, color, (int)vertices[faces[i].v1].x + 100, (int)vertices[faces[i].v1].y + 100, (int)vertices[faces[i].v2].x + 100, (int)vertices[faces[i].v2].y + 100);
		DrawLine(bufDC, color, (int)vertices[faces[i].v2].x + 100, (int)vertices[faces[i].v2].y + 100, (int)vertices[faces[i].v3].x + 100, (int)vertices[faces[i].v3].y + 100);
		DrawLine(bufDC, color, (int)vertices[faces[i].v3].x + 100, (int)vertices[faces[i].v3].y + 100, (int)vertices[faces[i].v1].x + 100, (int)vertices[faces[i].v1].y + 100);
	}
}

void Render(HDC hdc)
{
	DrawModel(hdc);
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

void SetClientSize(HWND hwnd, int clientWidth, int clientHeight)
{
	if (IsWindow(hwnd))
	{
		RECT winRC, clientRC;
		GetWindowRect(hwnd, &winRC); //get the current window rect
		GetClientRect(hwnd, &clientRC); //get the current client rect
		int dx = (clientRC.right - clientRC.left) - clientWidth; //calculate difference between current client width and desired client width
		int dy = (clientRC.bottom - clientRC.top) - clientHeight; //same for height
																  //adjust the size
		SetWindowPos(hwnd, 0, 0, 0, winRC.right - winRC.left - dx, winRC.bottom - winRC.top - dy, SWP_NOZORDER | SWP_NOMOVE);
	}
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_KEYDOWN: // same as pressing the X button:
	case WM_CLOSE:   DestroyWindow(hwnd); isRunning = false; PostQuitMessage(0); return 0;
	case WM_DESTROY: PostQuitMessage(0);  return 0;
	case WM_PAINT:   PaintWindow(hwnd);   return 0;
	//case WM_SIZING:
	//case WM_SIZE:	 PaintWindow(hwnd);   return 0;
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
	rect.top = 0; rect.left = 0; rect.right = 320; rect.bottom = 240;
	SetClientSize(hwnd, 320, 240);

	bufDC = CreateCompatibleDC(GetDC(hwnd));
	bufBMP = CreateCompatibleBitmap(GetDC(hwnd), rect.right, rect.bottom);
	SelectObject(bufDC, bufBMP);

	isRunning = false;
	HANDLE hRenderThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RenderThread, hwnd, 0, NULL);
	LoadModel();
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