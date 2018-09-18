#include <stdio.h>
#include <stdint.h>
#include <windows.h>
#include "windowsx.h"
#include <math.h>
#include "winlcd.h"

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
static char szClassName[] = "LCD";
static int winx;
static int winy;
static uint32_t *frame_buffer = NULL;
static HWND hWnd;
static HANDLE hThread;
static DWORD dwThreadId;

int fKillThread = 0;

int lcd_init(proc_f fucn_addr, char *name, int size_x, int size_y) {
	MSG lpMsg;
	WNDCLASS wc;
	RECT Rect;
	HANDLE hInstance;
	DWORD dwWindowStyle = WS_OVERLAPPEDWINDOW & ~(WS_THICKFRAME);
	int iWindowSizeX, iWindowSizeY;
	char title[64];
#if (SCALE_2X == 1)
	winx = 2 * size_x;
	winy = 2 * size_y;
#else
	winx = size_x;
	winy = size_y;
#endif
	// Создаем кадровый буфер:
	frame_buffer = (uint32_t*)malloc(sizeof(DWORD) * winx * winy);
	// Инициализируем пток для обработчика окна:
	hInstance = GetModuleHandle(NULL);
	hThread = CreateThread( 
			NULL,                   // default security attributes
			0,                      // use default stack size  
			fucn_addr,              // thread function name
			NULL,                   // argument to thread function 
			0,                      // use default creation flags 
			&dwThreadId);           // returns the thread identifier 
	if (hThread == NULL) {
		MessageBox(NULL, "Не могу создать поток!", "Ошибка", MB_OK);
		return 1;
	}
	// Заполняем структуру класса окна
	wc.style         = (CS_HREDRAW | CS_VREDRAW);
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = NULL;
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = szClassName;
	// Форма окна:
	Rect.left  = 100;
	Rect.right = Rect.left + winx;
	Rect.top   = 100;
	Rect.bottom = Rect.top + winy;
	AdjustWindowRect(&Rect, dwWindowStyle, FALSE);
	iWindowSizeX = Rect.right - Rect.left;
	iWindowSizeY = Rect.bottom - Rect.top;
	// Регистрируем класс окна
	if (!RegisterClass(&wc)) {
		MessageBox(NULL, "Не могу зарегистрировать класс окна!", "Ошибка", MB_OK);
		return 2;
	}
#if (SCALE_2X == 1)
	sprintf(title, "BCOMP11 PC Emu %dx%d", winx/2, winy/2);
#else
	sprintf(title, "BCOMP11 PC Emu %dx%d", winx, winy);
#endif
	printf("Title: %s\r\n", title);
	// Создаем основное окно приложения
	hWnd = CreateWindow( 
			szClassName,                // Имя класса
			title,                      // Текст заголовка 
			dwWindowStyle,              // Стиль окна
			50,   50,                   // Позиция левого верхнего угла
			iWindowSizeX,  iWindowSizeY,// Ширина и высота окна
			(HWND) NULL,                // Указатель на родительское окно NULL
			(HMENU) NULL,               // Используется меню класса окна
			(HINSTANCE)hInstance,       // Указатель на текущее приложение
			NULL);                      // Передается в качестве lParam в событие WM_CREATE
	if (!hWnd) {
		MessageBox(NULL, "Не удается создать главное окно!", "Ошибка", MB_OK);
		return 3;
	}
	// Показываем наше окно:
	ShowWindow(hWnd, SW_SHOW); 
	UpdateWindow(hWnd);
	// Выполняем цикл обработки сообщений до закрытия приложения:
	while (GetMessage(&lpMsg, NULL, 0, 0)) {
		TranslateMessage(&lpMsg);
		DispatchMessage(&lpMsg);
	}
	// Освобождаем буфер кадра:
	free(frame_buffer);
	return 0;
}

static int CopyFrameBuffer(HDC hdc, RECT *r) {
	HBITMAP hbitmap;
	HDC hdct;
	BITMAPINFO bmi;
	VOID *pvbits;
	int h = r->bottom - r->top;
	int w = r->right - r->left;
	ZeroMemory(&bmi,sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biHeight = winy;
	bmi.bmiHeader.biWidth  = winx;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	hdct = CreateCompatibleDC(NULL);
	hbitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &pvbits, NULL,0x0);
	SelectObject(hdct, hbitmap);
	memcpy(pvbits, frame_buffer, h*w*4);
	BitBlt(hdc, r->left, r->top, r->right - r->left, r->bottom - r->top, hdct, 0, 0, SRCCOPY);
	DeleteObject(hbitmap);
	DeleteDC(hdct);
	return 1;
}

int xMPos, yMPos;
int iMKey = 0;
int iMButton = 0;

static LRESULT CALLBACK WndProc(HWND hWnd, UINT messg, WPARAM wParam, LPARAM lParam) {
	RECT Rect;
	HDC hdc;
	switch (messg) {
	case WM_CREATE:
		GetClientRect(hWnd, &Rect);
		winx = Rect.right - Rect.left;
		winy = Rect.bottom - Rect.top;
		break;
	case WM_PAINT:
		hdc = GetDC(hWnd);
		GetClientRect(hWnd, &Rect);
		CopyFrameBuffer(hdc, &Rect);
		ValidateRect(hWnd, &Rect);
		break;
	case WM_COMMAND: 
		switch (LOWORD(wParam)) {
		default: break;
		} 
		break;
	case WM_MOUSEMOVE:
#if (SCALE_2X == 1)
		xMPos = GET_X_LPARAM(lParam)/2;
		yMPos = GET_Y_LPARAM(lParam)/2;
#else
		xMPos = GET_X_LPARAM(lParam);
		yMPos = GET_Y_LPARAM(lParam);
#endif
		break;
#if 1
	case WM_LBUTTONDOWN:
		iMKey = 1;
		iMButton |= 0x01;
		break;
	case WM_LBUTTONUP:
		iMKey = 0;
		iMButton &= ~0x01;
		break;
	case WM_RBUTTONDOWN:
		iMKey = 1;
		iMButton |= 0x02;
		break;
	case WM_RBUTTONUP:
		iMKey = 0;
		iMButton &= ~0x02;
		break;
#endif
	case WM_KEYDOWN:
		printf("WM_KEYDOWN: %02x\r\n", wParam);
		switch (wParam) {
		case VK_LEFT: break;
		case VK_RIGHT: break;
		case VK_UP: break;
		case VK_DOWN: break;
		case VK_HOME: break;
		case VK_END: break;
		case VK_INSERT: break;
		case VK_DELETE: break;
		case VK_F2: break;
		case VK_ESCAPE: goto win_exit;
		case 0x41: iMButton |= 0x01; break; // key A
		case 0x53: iMButton |= 0x02; break; // key S
		default: break;
		}
		break;
	case WM_KEYUP:
		printf("WM_KEYUP: %02x\r\n", wParam);
		switch (wParam) {
		case VK_LEFT: break;
		case VK_RIGHT: break;
		case VK_UP: break;
		case VK_DOWN: break;
		case VK_HOME: break;
		case VK_END: break;
		case VK_INSERT: break;
		case VK_DELETE: break;
		case VK_F2: break;
		case VK_ESCAPE: goto win_exit;
		case 0x41: iMButton &= ~0x01; break; // key A
		case 0x53: iMButton &= ~0x02; break; // key S
		default: break;
		}
		break;
	case WM_DESTROY:
win_exit:
		printf("WndProc(): WM_DESTROY!\r\n");
		fKillThread = 1;
		WaitForSingleObject(hThread, 500);
		CloseHandle(hThread);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, messg, wParam, lParam);
	}
	return 0;
}

uint32_t palette_oled[256] = { 0x00000000, 0x0F0F0F, 0x1F1F1F, 0x2F2F2F, 0x3F3F3F, 0x4F4F4F, 0x5F5F5F, 0x6F6F6F, 0x7F7F7F, 0x8F8F8F, 0x9F9F9F, 0xAFAFAF, 0xBFBFBF, 0xCFCFCF, 0xDFDFDF, 0xEFEFEF, 0xFFFFFF };
uint32_t palette[256] = { 0x00000000, 0xFFFFFF, 0xFF0000, 0x00FF00, 0x0000FF };

DWORD WINAPI lcd_cpframe( uint8_t *data ) {
	int i, j;
	if (fKillThread) {
		return 0;
	}
	for (i = 0; i < winy/2; i++) {
		for (j = 0; j < winx/2; j++) {
			frame_buffer[j*2+0+(winy-i*2-2)*winx] = data[j+(i/8)*winx/2]&(1<<(i&0x07))?0xFF:0;
			frame_buffer[j*2+1+(winy-i*2-1)*winx] = data[j+(i/8)*winx/2]&(1<<(i&0x07))?0xFF:0;
			frame_buffer[j*2+0+(winy-i*2-1)*winx] = data[j+(i/8)*winx/2]&(1<<(i&0x07))?0xFF:0;
			frame_buffer[j*2+1+(winy-i*2-2)*winx] = data[j+(i/8)*winx/2]&(1<<(i&0x07))?0xFF:0;
		}
	}
	// Команда на обновление окна:
	InvalidateRect(hWnd, 0, TRUE);
	UpdateWindow(hWnd);
	return 0;
}
