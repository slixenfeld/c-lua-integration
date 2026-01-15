#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"

#define BTN_RUN_ID	100
#define INPUT_LUA_ID	200
#define SCREEN_OUT_ID	300

#define LUA_OUT_SIZE 8192
static char lua_output[LUA_OUT_SIZE];
static size_t lua_out_len = 0;

char buffer[1024];

int lua_print_redirect(lua_State *L)
{
	int n = lua_gettop(L);

	for (int i = 1; i <= n; i++)
	{
		const char *s = lua_tostring(L, i);
		if (!s) s = "[non-string]";

		int written = snprintf(
				lua_output + lua_out_len,
				LUA_OUT_SIZE - lua_out_len,
				"%s%s",
				s,
				(i < n) ? "\t" : "\n"
				);

		if (written > 0)
			lua_out_len += written;
	}

	return 0; 
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)    
{
	switch (msg)
	{
		case WM_CREATE:

			CreateWindowA( "EDIT", "Lua",
					WS_VISIBLE | WS_CHILD,
					20, 20, 100, 20,
					hwnd,
					(HMENU)123,
					((LPCREATESTRUCT)lParam)->hInstance,
					NULL
				     );

			CreateWindowExA(
					WS_EX_CLIENTEDGE,
					"EDIT",	"",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP |
					ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN |
					WS_VSCROLL,
					20, 40, 200, 100,       
					hwnd,
					(HMENU)INPUT_LUA_ID,
					GetModuleHandle(NULL),
					NULL
				       );

			CreateWindowA( "EDIT", "Output",
					WS_VISIBLE | WS_CHILD,
					20, 150, 100, 20,
					hwnd,
					(HMENU)123,
					((LPCREATESTRUCT)lParam)->hInstance,
					NULL
				     );

			CreateWindowExA(
					WS_EX_CLIENTEDGE,
					"EDIT", "",
					WS_CHILD | WS_VISIBLE | WS_TABSTOP |
					ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN |
					WS_VSCROLL,
					20, 170, 200, 100,       
					hwnd,
					(HMENU)SCREEN_OUT_ID,
					GetModuleHandle(NULL),
					NULL
				       );			

			CreateWindowA( "BUTTON", "Run",
					WS_VISIBLE | WS_CHILD,
					20, 275, 100, 30,
					hwnd,
					(HMENU)BTN_RUN_ID,
					((LPCREATESTRUCT)lParam)->hInstance,
					NULL
				     );
			break;

		case WM_COMMAND:
			if (LOWORD(wParam) == BTN_RUN_ID)
			{
				HWND hEdit = GetDlgItem(hwnd, INPUT_LUA_ID);
				GetWindowTextA(hEdit, buffer, sizeof(buffer));

				lua_out_len = 0;
				lua_output[0] = '\0';

				lua_State *L = luaL_newstate();
				luaL_openlibs(L);

				lua_pushcfunction(L, lua_print_redirect);
				lua_setglobal(L, "print");

				if (luaL_dostring(L, buffer))
				{
					const char *err = lua_tostring(L, -1);
					snprintf(lua_output, LUA_OUT_SIZE, "Lua error:\n%s\n", err);
				}

				lua_close(L);

				SetWindowTextA(GetDlgItem(hwnd, SCREEN_OUT_ID), lua_output);
			}
			break;

		case WM_PAINT:
			{
				PAINTSTRUCT ps;
				HDC hdc = BeginPaint(hwnd, &ps);

				EndPaint(hwnd, &ps);
				break;
			}

		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR lpCmdLine, 
		int nCmdShow)
{
	WNDCLASSA wc;
	HWND hwnd;
	MSG msg;

	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "WinCLuaWindowClass";

	RegisterClassA(&wc);

	hwnd = CreateWindowA(
			"WinCLuaWindowClass",
			"Call Lua from C",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT,
			250, 350,
			NULL, NULL,
			hInstance,
			NULL
			);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}

