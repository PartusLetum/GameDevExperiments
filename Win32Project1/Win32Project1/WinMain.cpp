//
// http://real3d.pk/opengl_tuto_3.html
//

#include <stdio.h>
#include <windows.h>
#define _USE_MATH_DEFINES
#include <math.h>

HDC bufDC;
HBITMAP bufBMP;
RECT rect;
bool isRunning;

struct Point3d { float x, y, z; };
struct Face3d { unsigned int v1, v2, v3; };

const int vertexCount = 8;		// Number of vertices in model / mesh
Point3d vertices[vertexCount];	// Vertices of the model

const int faceCount = 12;		// Number of faces in model
Face3d faces[faceCount];		// Faces that make up the model (indicies into vertices)
Point3d faceNormals[faceCount];	// Normals to the faces that make up the model
COLORREF faceColors[faceCount]; // Color of the faces

Point3d viewLocation;
float angle = 0.0f;

void NormalizeVector( Point3d &v )
{
	float length = sqrt( v.x * v.x + v.y * v.y + v.z * v.z );
	if( length != 0 )
	{
		v.x /= length;
		v.y /= length;
		v.z /= length;
	}
}

float DotProduct( Point3d u, Point3d v )
{
	return ( u.x * v.x + u.y * v.y + u.z * v.z );
}

Point3d SubtractVector( Point3d u, Point3d v )
{
	u.x -= v.x; u.y -= v.y; u.z -= v.z;
	return u;
}

Point3d CrossJoin(Point3d u, Point3d v)
{
	Point3d uv;

	uv.x = (u.y * v.z) - (u.z * v.y);
	uv.y = (u.z * v.x) - (u.x * v.z);
	uv.z = (u.x * v.y) - (u.y * v.x);

	return uv;
}

Point3d CalculateSurfaceNormal( Point3d v1, Point3d v2, Point3d v3 )
{
	Point3d U = SubtractVector(v2, v1);
	Point3d V = SubtractVector(v3, v1);
	Point3d normal = CrossJoin(V, U);

	NormalizeVector( normal );

	return normal;
}

void LoadModel()
{
	// Load up a cube

	// Front Square
	vertices[0].x = -50; vertices[0].y =  50; vertices[0].z =  50;
	vertices[1].x =  50; vertices[1].y =  50; vertices[1].z =  50;
	vertices[2].x =  50; vertices[2].y = -50; vertices[2].z =  50;
	vertices[3].x = -50; vertices[3].y = -50; vertices[3].z =  50;
	// Back Square
	vertices[4].x = -50; vertices[4].y =  50; vertices[4].z = -50;
	vertices[5].x =  50; vertices[5].y =  50; vertices[5].z = -50;
	vertices[6].x =  50; vertices[6].y = -50; vertices[6].z = -50;
	vertices[7].x = -50; vertices[7].y = -50; vertices[7].z = -50;

	// Face 1
	faces[0].v1 = 0; faces[0].v2 = 1; faces[0].v3 = 3; faceColors[0] = RGB( 255, 0, 0 );
	faces[1].v1 = 2; faces[1].v2 = 3; faces[1].v3 = 1; faceColors[1] = RGB( 0, 255, 0 );
	// Face 2
	faces[2].v1 = 1; faces[2].v2 = 5; faces[2].v3 = 2; faceColors[2] = RGB( 0,   0, 255 );
	faces[3].v1 = 6; faces[3].v2 = 2; faces[3].v3 = 5; faceColors[3] = RGB( 0, 255, 255 );
	// Face 3
	faces[4].v1 = 5; faces[4].v2 = 1; faces[4].v3 = 6; faceColors[4] = RGB( 255, 0, 0 );
	faces[5].v1 = 6; faces[5].v2 = 1; faces[5].v3 = 2; faceColors[5] = RGB( 0, 255, 0 );
	// Face 4
	faces[6].v1 = 7; faces[6].v2 = 0; faces[6].v3 = 4; faceColors[6] = RGB( 255, 0, 0 );
	faces[7].v1 = 3; faces[7].v2 = 0; faces[7].v3 = 7; faceColors[7] = RGB( 0, 255, 0 );
	// Face 5
	faces[8].v1 = 1; faces[8].v2 = 0; faces[8].v3 = 2; faceColors[8] = RGB( 255, 0, 0 );
	faces[9].v1 = 0; faces[9].v2 = 2; faces[9].v3 = 3; faceColors[9] = RGB( 0, 255, 0 );
	// Face 6
	faces[10].v1 = 1; faces[10].v2 = 0; faces[10].v3 = 2; faceColors[10] = RGB( 255, 0, 0 );
	faces[11].v1 = 0; faces[11].v2 = 2; faces[11].v3 = 3; faceColors[11] = RGB( 0, 255, 0 );

	Point3d v = SubtractVector( vertices[faces[0].v1], vertices[faces[0].v2] );
	float length = sqrt( v.x * v.x + v.y * v.y + v.z * v.z );

	// Calculate surface normals ...
	for( int i = 0; i < faceCount; i++ )
	{
		int v1 = faces[i].v1; int v2 = faces[i].v2; int v3 = faces[i].v3;
		faceNormals[i] = CalculateSurfaceNormal( vertices[v1], vertices[v2], vertices[v3] );
	}
}

void DrawLine(HDC hdc, COLORREF color, int x1, int y1, int x2, int y2)
{
	HPEN hPen = CreatePen(PS_SOLID, 1, color);
	SelectObject(hdc, hPen);
	MoveToEx(hdc, x1, y1, NULL);
	LineTo(hdc, x2, y2);
	DeleteObject(hPen);
}

void FillFlatBottomTriangle( Point3d v1, Point3d v2, Point3d v3, COLORREF color, bool wireframe)
{
	float invslope1 = ( v2.x - v1.x ) / ( v2.y - v1.y );
	float invslope2 = ( v3.x - v1.x ) / ( v3.y - v1.y );

	float curx1 = v1.x;
	float curx2 = v1.x;

	HPEN hPen = CreatePen( PS_SOLID, 1, color );
	SelectObject( bufDC, hPen );
	for( int scanlineY = v1.y; scanlineY <= v2.y; scanlineY++ )
	{
		MoveToEx( bufDC, ( int )curx1, scanlineY, NULL );
		LineTo( bufDC, ( int )curx2, scanlineY );
		curx1 += invslope1;
		curx2 += invslope2;
	}

	if( wireframe )
	{
		color = RGB( 255, 255, 255 );
		DrawLine( bufDC, color, ( int )v1.x, ( int )v1.y, ( int )v2.x, ( int )v2.y );
		DrawLine( bufDC, color, ( int )v2.x, ( int )v2.y, ( int )v3.x, ( int )v3.y );
		DrawLine( bufDC, color, ( int )v3.x, ( int )v3.y, ( int )v1.x, ( int )v1.y );
	}

	DeleteObject( hPen );
}

void FillFlatTopTriangle( Point3d v1, Point3d v2, Point3d v3, COLORREF color, bool wireframe )
{
	float invslope1 = ( v3.x - v1.x ) / ( v3.y - v1.y );
	float invslope2 = ( v3.x - v2.x ) / ( v3.y - v2.y );

	float curx1 = v3.x;
	float curx2 = v3.x;

	HPEN hPen = CreatePen( PS_SOLID, 1, color );
	SelectObject( bufDC, hPen );
	for( int scanlineY = (int)v3.y; scanlineY > v1.y; scanlineY-- )
	{
		MoveToEx( bufDC, (int)curx1, scanlineY, NULL );
		LineTo( bufDC, (int)curx2, scanlineY );
		curx1 -= invslope1;
		curx2 -= invslope2;
	}

	if( wireframe )
	{
		color = RGB( 255, 255, 255 );
		DrawLine( bufDC, color, ( int )v1.x, ( int )v1.y, ( int )v2.x, ( int )v2.y );
		DrawLine( bufDC, color, ( int )v2.x, ( int )v2.y, ( int )v3.x, ( int )v3.y );
		DrawLine( bufDC, color, ( int )v3.x, ( int )v3.y, ( int )v1.x, ( int )v1.y );
	}

	DeleteObject( hPen );
}

void FillTriangle( Point3d v1, Point3d v2, Point3d v3, COLORREF color, bool wireframe )
{
	// First sort the three vertices by y-coordinate ascending so v1 is the topmost vertice
	// TODO: SortVertices !!!
	// We should probably do this before this so it is only done once
	Point3d temp;
	if( v1.y > v2.y ) { temp = v2; v2 = v1; v1 = temp; }
	if( v2.y > v3.y ) { temp = v3; v3 = v2; v2 = temp; }
	if( v1.y > v2.y ) { temp = v2; v2 = v1; v1 = temp; }

	// Here we know that v1.y <= v2.y <= v3.y
	// Check for trivial case of bottom flat triangle
	if( v2.y == v3.y )
	{
		FillFlatBottomTriangle( v1, v2, v3, color, wireframe );
	}
	// Check for trivial case of top-flat triangle
	else if( v1.y == v2.y )
	{
		FillFlatTopTriangle( v1, v2, v3, color, wireframe );
	}
	else
	{
		// General case - split the triangle into a topflat and bottom-flat one
		Point3d v4;
		v4.x = ( int )( v1.x + ( ( float )( v2.y - v1.y ) / ( float )( v3.y - v1.y ) ) * ( v3.x - v1.x ) );
		v4.y = v2.y;
		FillFlatBottomTriangle( v1, v2, v4, color, false );
		FillFlatTopTriangle( v2, v4, v3, color, false );

		if( wireframe )
		{
			HPEN hPen = CreatePen( PS_SOLID, 1, color );
			SelectObject( bufDC, hPen );
			color = RGB( 255, 255, 255 );
			DrawLine( bufDC, color, ( int )v1.x, ( int )v1.y, ( int )v2.x, ( int )v2.y );
			DrawLine( bufDC, color, ( int )v2.x, ( int )v2.y, ( int )v3.x, ( int )v3.y );
			DrawLine( bufDC, color, ( int )v3.x, ( int )v3.y, ( int )v1.x, ( int )v1.y );
			DeleteObject( hPen );
		}
	}
}

void DrawTriangle( HDC hdc, COLORREF color, Face3d &face, Point3d* vertices )
{
	DrawLine( hdc, color, vertices[face.v1].x, vertices[face.v1].y, vertices[face.v2].x, vertices[face.v2].y );
	DrawLine( hdc, color, vertices[face.v2].x, vertices[face.v2].y, vertices[face.v3].x, vertices[face.v3].y );
	DrawLine( hdc, color, vertices[face.v3].x, vertices[face.v3].y, vertices[face.v1].x, vertices[face.v1].y );
}

// Mesh!?!
void DrawModel(HDC hdc)
{
	// Perform Transformations and stuff ...
	int xOffset = ( int )viewLocation.x; int yOffset = ( int )viewLocation.y; int zOffset = ( int )viewLocation.z;
	Point3d* vt = (Point3d*)malloc( sizeof( Point3d ) * vertexCount );
	Point3d* vs = (Point3d*)malloc( sizeof( Point3d ) * vertexCount );
	for( int i = 0; i < vertexCount; i ++ ) // ?? Vertex or Verticie ??
	{
		// Todo: Change transformations to use Matrices like a grown up
		// World / view Transformations
		vt[i].x = ( vertices[i].z * cos( angle ) ) + ( vertices[i].x * sin( angle ) );
		vt[i].y = vertices[i].y;
		vt[i].z = ( vertices[i].z * sin( angle ) ) - ( vertices[i].x * cos( angle ) );

		vt[i].x += xOffset;
		vt[i].y += yOffset;
		vt[i].z += zOffset;
		
		// Meh Projection
		vs[i].x = ( vt[i].x * 100.0f / vt[i].z );
		vs[i].y = ( vt[i].y * 100.0f / vt[i].z );

		// Translate to screen co-ordinates
		vs[i].x += 160;
		vs[i].y = 100 - vs[i].y;
		//vt[i].z += zOffset;
	}
	Point3d* nt = ( Point3d* )malloc( sizeof( Point3d ) * faceCount );
	for( int i = 0; i < faceCount; i++ )
	{
		float x = faceNormals[i].x;
		float y = faceNormals[i].y;
		float z = faceNormals[i].z;

		nt[i].x = ( z * cos( angle ) ) + ( x * sin( angle ) );
		nt[i].y = y;
		nt[i].z = ( z * sin( angle ) ) - ( x * cos( angle ) );
	}

	// Render Faces
	Point3d cameraLocation;
	cameraLocation.x = 0; cameraLocation.y = 0; cameraLocation.z = 0;
	for (int i = 0; i < 4; i++)
	{
		// Back face culling
		if( DotProduct( SubtractVector( cameraLocation, vt[faces[i].v1] ), nt[i] ) < 0 )
			continue;

		COLORREF color = faceColors[i];
		//FillTriangle( vs[faces[i].v1], vs[faces[i].v2], vs[faces[i].v3], color, true );
		DrawTriangle( bufDC, color, faces[i], vs );
		
		// Debug: Render surface normals
		float x1, y1, z1, x2, y2, z2;
		x1 = vs[faces[i].v1].x; y1 = vs[faces[i].v1].y; z1 = vs[faces[i].v1].z;
		x2 = (nt[i].x * 10.0f) + x1; y2 = (nt[i].y * 10.0f) + y1; z2 = (nt[i].z * 10.0f) + z1;

		//x1 += 160; y1 = 100 - y1;
		//x2 += 160; y2 = 100 - y2;

		DrawLine( bufDC, color, x1, y1, x2, y2 );
	}
	
	delete vt;
	delete vs;
	delete nt;
}

void Render(HDC hdc)
{
	BitBlt( bufDC, 0, 0, rect.right, rect.bottom, NULL, 0, 0, BLACKNESS );
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
	bool movingLeft = true;

	while (isRunning)
	{
		// Do update, rendering and all the real game loop stuff
		//viewLocation.x += ( movingLeft ? 0.1f : -0.1f );
		if( viewLocation.x > 320 ) movingLeft = false;
		if( viewLocation.x < -320 ) movingLeft = true;

		angle += 0.0001f;

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
	viewLocation.x = 0; viewLocation.y = 0; viewLocation.z = 150;
	angle = 0.0f;
	//viewLocation.x = 0; viewLocation.y = 0; viewLocation.z = 100;
	bool movingLeft = true;
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
		// Actually, do this is in the game loop thread ...
		Sleep(1);
	}

	return msg.wParam;
}