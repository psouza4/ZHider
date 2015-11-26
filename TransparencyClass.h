#define WS_EX_LAYERED    0x00080000
#define LWA_ALPHA        0x00000002

extern HMODULE			hUserDll;

class trWindow
{
public:
	HWND		hWnd;
	INT			curTrans;
	COLORREF	color;

	trWindow(HWND);
	~trWindow(VOID);
	BOOL trans(COLORREF, BYTE, DWORD);
	BOOL trans(int);
};

class HRT
{
public:
	LARGE_INTEGER res_timer;
	LARGE_INTEGER start_timer;
	LARGE_INTEGER end_timer;

	void start(void);
	__int64 end(void);
};
