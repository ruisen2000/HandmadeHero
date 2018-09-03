/*
	Author: Greyson Wang
	Date: June 22, 2018
*/

#include <Windows.h>

#define internal static 
#define local_persist static 
#define global_variable static 

global_variable bool Running;  // static is init to 0 by default

global_variable BITMAPINFO BitmapInfo;	// contains dimensions and colour info of the bitmap.  The colour info describes how pixels correspond to RGB 
global_variable void* BitmapMemory; 
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;

internal void Win32ResizeDIBSection(int Width, int Height)
{
	// Delete old one before allocating the new backbuffer bitmap
	if(BitmapHandle)
	{
		DeleteObject(BitmapHandle);
	}
	if (BitmapDeviceContext)
	{
		BitmapDeviceContext = CreateCompatibleDC(0);
	}

	// allocate memory for backbuffer bitmap
	BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader); // number of bits in header to skip to get to colour table
	BitmapInfo.bmiHeader.biWidth = Width;
	BitmapInfo.bmiHeader.biHeight = Height;
	BitmapInfo.bmiHeader.biPlanes = 1;
	BitmapInfo.bmiHeader.biBitCount = 32;  // 32 bit colour
	BitmapInfo.bmiHeader.biCompression = BI_RGB; // no compression
		
	BitmapHandle = CreateDIBSection(
		BitmapDeviceContext, &BitmapInfo,
		DIB_RGB_COLORS,
		&BitmapMemory,
		0,0);		
}

// Redraw contents of the window
internal void Win32UpdateWindow(HDC DeviceContext, int X, int Y, int Width, int Height)
{
	// Copy from backbuffer that contains rendered image to the window.
	// Backbuffer has the same dimensions as the window (for now)
	StretchDIBits(DeviceContext,
		X, Y, Width, Height,
		X, Y, Width, Height,
		BitmapMemory,
		&BitmapInfo,
		DIB_RGB_COLORS, SRCCOPY); // copy directly from back buffer
}

LRESULT CALLBACK Win32MainWindowsCallback(
   HWND   Window,
   UINT   Message,
   WPARAM WParam,
   LPARAM LParam
)
{
	LRESULT Result = 0;
	switch(Message)
	{
		case WM_SIZE:
			{
				RECT ClientRect;
				GetClientRect(Window, &ClientRect);
				int Width = ClientRect.right - ClientRect.left;
				int Height = ClientRect.bottom - ClientRect.top;
				Win32ResizeDIBSection(Width, Height);				
				break;
			}
		case WM_DESTROY:
			{
				Running = false;
				OutputDebugStringA("WM_DESTROY\n");
				break;
			}
		case WM_CLOSE:
			{
				Running = false;
				OutputDebugStringA("WM_CLOSE\n");
				break;
			}
		case WM_ACTIVATEAPP:
			{
				OutputDebugStringA("WM_ACTIVATEAPP\n");
				break;
			}
		case WM_PAINT:
			{
				PAINTSTRUCT Paint;
				HDC DeviceContext = BeginPaint(Window, &Paint);
				int X = Paint.rcPaint.left;
				int Y = Paint.rcPaint.top;
				int Width = Paint.rcPaint.right - Paint.rcPaint.left;
				int Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
				Win32UpdateWindow(DeviceContext, X, Y, Width, Height);				
				EndPaint(Window, &Paint);
				break;
			}
		default:
			Result = DefWindowProcA(Window, Message, WParam, LParam);
	}

	return Result;
}


int CALLBACK WinMain(
   HINSTANCE Instance,
   HINSTANCE PrevInstance,
   LPSTR     CommandLine,
   int       ShowCode
)
{
	WNDCLASSA WindowClass = {};
	WindowClass.style = CS_OWNDC|CS_HREDRAW|CS_VREDRAW;
	WindowClass.lpfnWndProc = Win32MainWindowsCallback;
	WindowClass.hInstance = Instance;
	WindowClass.lpszClassName = "HandmadeHero";

	if(RegisterClassA(&WindowClass))
	{
		// Register the window.
		HWND WindowHandle = 
			CreateWindowExA(
				0,
				WindowClass.lpszClassName,
				"handmade Hero",
				WS_OVERLAPPEDWINDOW|WS_VISIBLE,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				CW_USEDEFAULT,
				0,
				0,
				Instance,
				0);
		if(WindowHandle)
		{
			Running = true;
			// Pull messages sent by windows or anyone else from message queue.
			while(Running)
			{
				MSG Message;
				BOOL MessageResult = GetMessageA(&Message, 0, 0, 0);
				if (MessageResult > 0)
				{					 
					TranslateMessage(&Message);  // windows processing to get it ready for dispatch
					DispatchMessageA(&Message);	// send the message
				}
				else
				{
					break;
				}
			}			
		}
		else
		{
			// TODO
		}
	}
	else
	{
		// TODO
	}
	return 0;
}