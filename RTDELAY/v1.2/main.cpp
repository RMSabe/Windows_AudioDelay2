/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "globldef.h"
#include "cstrdef.h"
#include "thread.h"
#include "strdef.hpp"

#include "shared.hpp"

#include <combaseapi.h>

#include "AudioRTDelay.hpp"
#include "AudioPB.hpp"
#include "AudioPB_i16.hpp"
#include "AudioPB_i24.hpp"

#define CUSTOM_GENERIC_WNDCLASS_NAME TEXT("__CUSTOMGENERICWNDCLASS__")

#define CUSTOM_WM_PLAYBACK_FINISHED (WM_USER | 1U)

#define RUNTIME_STATUS_INIT 0
#define RUNTIME_STATUS_IDLE 1
#define RUNTIME_STATUS_CHOOSEFILE 2
#define RUNTIME_STATUS_CHOOSEAUDIODEV 3
#define RUNTIME_STATUS_PLAYBACK_RUNNING 4
#define RUNTIME_STATUS_PLAYBACK_FINISHED 5

/*Delay Buffer Size*/

#define __RTDELAY_BUFFER_SIZE_FRAMES 65536U

/*Channel count for parallel feedforward/feedback delays.*/

#define __RTDELAY_N_FFCH 4U
#define __RTDELAY_N_FBCH 4U

#define PB_I16 1
#define PB_I24 2

#define CUSTOMCOLOR_BLACK 0x00000000
#define CUSTOMCOLOR_WHITE 0x00ffffff
#define CUSTOMCOLOR_LTGRAY 0x00c0c0c0

#define BRUSHINDEX_TRANSPARENT 0U
#define BRUSHINDEX_CUSTOM_SOLID_BLACK 1U
#define BRUSHINDEX_CUSTOM_SOLID_WHITE 2U
#define BRUSHINDEX_CUSTOM_SOLID_LTGRAY 3U

#define PP_BRUSH_LENGTH 4U
#define PP_BRUSH_SIZE (PP_BRUSH_LENGTH*sizeof(HBRUSH))

#define CUSTOMFONT_SMALL_CHARSET DEFAULT_CHARSET
#define CUSTOMFONT_SMALL_WIDTH 8U
#define CUSTOMFONT_SMALL_HEIGHT 16U
#define CUSTOMFONT_SMALL_WEIGHT FW_NORMAL

#define CUSTOMFONT_MEDIUM_CHARSET DEFAULT_CHARSET
#define CUSTOMFONT_MEDIUM_WIDTH 16U
#define CUSTOMFONT_MEDIUM_HEIGHT 24U
#define CUSTOMFONT_MEDIUM_WEIGHT FW_NORMAL

#define CUSTOMFONT_LARGE_CHARSET DEFAULT_CHARSET
#define CUSTOMFONT_LARGE_WIDTH 20U
#define CUSTOMFONT_LARGE_HEIGHT 35U
#define CUSTOMFONT_LARGE_WEIGHT FW_NORMAL

#define FONTINDEX_CUSTOM_SMALL 0U
#define FONTINDEX_CUSTOM_MEDIUM 1U
#define FONTINDEX_CUSTOM_LARGE 2U

#define PP_FONT_LENGTH 3U
#define PP_FONT_SIZE (PP_FONT_LENGTH*sizeof(HFONT))

#define CHILDWNDINDEX_TEXT1 0U
#define CHILDWNDINDEX_TEXT2 1U
#define CHILDWNDINDEX_TEXT3 2U
#define CHILDWNDINDEX_TEXT4 3U
#define CHILDWNDINDEX_BUTTON1 4U
#define CHILDWNDINDEX_BUTTON2 5U
#define CHILDWNDINDEX_BUTTON3 6U
#define CHILDWNDINDEX_BUTTON4 7U
#define CHILDWNDINDEX_BUTTON5 8U
#define CHILDWNDINDEX_BUTTON6 9U
#define CHILDWNDINDEX_BUTTON7 10U
#define CHILDWNDINDEX_BUTTON8 11U
#define CHILDWNDINDEX_BUTTON9 12U
#define CHILDWNDINDEX_TEXTBOX1 13U
#define CHILDWNDINDEX_TEXTBOX2 14U
#define CHILDWNDINDEX_TEXTBOX3 15U
#define CHILDWNDINDEX_TEXTBOX4 16U
#define CHILDWNDINDEX_TEXTBOX5 17U
#define CHILDWNDINDEX_TEXTBOX6 18U
#define CHILDWNDINDEX_RADIOBUTTON1 19U
#define CHILDWNDINDEX_RADIOBUTTON2 20U
#define CHILDWNDINDEX_RADIOBUTTON3 21U
#define CHILDWNDINDEX_RADIOBUTTON4 22U
#define CHILDWNDINDEX_RADIOBUTTON5 23U
#define CHILDWNDINDEX_RADIOBUTTON6 24U
#define CHILDWNDINDEX_RADIOBUTTON7 25U
#define CHILDWNDINDEX_RADIOBUTTON8 26U
#define CHILDWNDINDEX_LISTBOX1 27U
#define CHILDWNDINDEX_BTNGROUPBOX1 28U
#define CHILDWNDINDEX_BTNGROUPBOX2 29U
#define CHILDWNDINDEX_BTNGROUPBOX3 30U
#define CHILDWNDINDEX_BTNGROUPBOX4 31U
#define CHILDWNDINDEX_BTNGROUPBOX5 32U
#define CHILDWNDINDEX_BTNGROUPBOX6 33U
#define CHILDWNDINDEX_BTNGROUPBOX7 34U
#define CHILDWNDINDEX_BTNGROUPBOX8 35U
#define CHILDWNDINDEX_BTNGROUPBOX9 36U
#define CHILDWNDINDEX_BTNGROUPBOX10 37U
#define CHILDWNDINDEX_BTNGROUPBOX11 38U
#define CHILDWNDINDEX_CONTAINER1 39U
#define CHILDWNDINDEX_CONTAINER2 40U
#define CHILDWNDINDEX_CONTAINER3 41U
#define CHILDWNDINDEX_CONTAINER4 42U
#define CHILDWNDINDEX_CONTAINER5 43U
#define CHILDWNDINDEX_CONTAINER6 44U
#define CHILDWNDINDEX_CONTAINER7 45U
#define CHILDWNDINDEX_CONTAINER8 46U 
#define CHILDWNDINDEX_CONTAINER9 47U
#define CHILDWNDINDEX_CONTAINER10 48U
#define CHILDWNDINDEX_CONTAINER11 49U

#define PP_CHILDWND_LENGTH 50U
#define PP_CHILDWND_SIZE (PP_CHILDWND_LENGTH*sizeof(HWND))

#define MAINWND_CAPTION TEXT("Audio Real-Time Delay 2")

#define MAINWND_BKCOLOR CUSTOMCOLOR_LTGRAY
#define MAINWND_BRUSHINDEX BRUSHINDEX_CUSTOM_SOLID_LTGRAY

#define CONTAINERWND_BKCOLOR MAINWND_BKCOLOR
#define CONTAINERWND_BRUSHINDEX BRUSHINDEX_TRANSPARENT

#define TEXTWND_TEXTCOLOR CUSTOMCOLOR_BLACK
#define TEXTWND_BKCOLOR MAINWND_BKCOLOR
#define TEXTWND_BRUSHINDEX BRUSHINDEX_TRANSPARENT

/*============================================================================*/

/*
	CHILDWND mapping for when playback is running:

	Below are defined macros to aid with referencing each child window to its purpose.

	Header: text1
	Subheader: text2

	Container1: Parameters
	{
		btngroupbox1 (params)

		text3 (params1)
		text4 (params2)
	}

	Container2: Set Dry Input Amplitude
	{
		btngroupbox2 (dry amp)

		textbox1 (update dry amp)
		button2 (update dry amp)
	}

	Container3: Set Output Amplitude
	{
		btngroupbox3 (out amp)

		textbox2 (update dry amp)
		button3 (update out amp)
	}

	Container4: FF Delay
	{
		btngroupbox4 (ff delay)

		Container6: choose ff channel
		{
			btngroupbox6 (ff channel)

			radiobutton 1-4 (ff channel 1-4)
		}

		Container7: set ff delay time
		{
			btngroupbox7 (ff delay time)

			textbox3 (update ff delay time)
			button6 (update ff delay time)
		}

		Container8: set ff amp
		{
			btngroupbox8 (ff amp)

			textbox4 (update ff amp)
			button7 (update ff amp)
		}

		button4 (reset ff params)
	}

	Container5: FB Delay
	{
		btngroupbox5 (fb delay)

		Container9: choose fb channel
		{
			btngroupbox9 (fb channel)

			radiobutton 5-8 (fb channel 1-4)
		}

		Container10: set fb delay time
		{
			btngroupbox10 (fb delay time)

			textbox5 (update fb delay time)
			button8 (update fb delay time)
		}

		Container11: set fb amp
		{
			btngroupbox11 (fb amp)

			textbox6 (update fb amp)
			button9 (update fb amp)
		}

		button5 (reset fb params)
	}

	Footer: button1 (stop playback)
*/

#define PBRUN_CHILDWNDINDEX_TEXT_HEADER CHILDWNDINDEX_TEXT1
#define PBRUN_CHILDWNDINDEX_TEXT_SUBHEADER CHILDWNDINDEX_TEXT2
#define PBRUN_CHILDWNDINDEX_TEXT_PARAMS1 CHILDWNDINDEX_TEXT3
#define PBRUN_CHILDWNDINDEX_TEXT_PARAMS2 CHILDWNDINDEX_TEXT4
#define PBRUN_CHILDWNDINDEX_BUTTON_STOPPB CHILDWNDINDEX_BUTTON1
#define PBRUN_CHILDWNDINDEX_BUTTON_UPDATEDRYAMP CHILDWNDINDEX_BUTTON2
#define PBRUN_CHILDWNDINDEX_BUTTON_UPDATEOUTAMP CHILDWNDINDEX_BUTTON3
#define PBRUN_CHILDWNDINDEX_BUTTON_RESETFFPARAMS CHILDWNDINDEX_BUTTON4
#define PBRUN_CHILDWNDINDEX_BUTTON_RESETFBPARAMS CHILDWNDINDEX_BUTTON5
#define PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFFDELAY CHILDWNDINDEX_BUTTON6
#define PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFFAMP CHILDWNDINDEX_BUTTON7
#define PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFBDELAY CHILDWNDINDEX_BUTTON8
#define PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFBAMP CHILDWNDINDEX_BUTTON9
#define PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEDRYAMP CHILDWNDINDEX_TEXTBOX1
#define PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEOUTAMP CHILDWNDINDEX_TEXTBOX2
#define PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFDELAY CHILDWNDINDEX_TEXTBOX3
#define PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFAMP CHILDWNDINDEX_TEXTBOX4
#define PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBDELAY CHILDWNDINDEX_TEXTBOX5
#define PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBAMP CHILDWNDINDEX_TEXTBOX6
#define PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL1 CHILDWNDINDEX_RADIOBUTTON1
#define PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL2 CHILDWNDINDEX_RADIOBUTTON2
#define PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL3 CHILDWNDINDEX_RADIOBUTTON3
#define PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL4 CHILDWNDINDEX_RADIOBUTTON4
#define PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL1 CHILDWNDINDEX_RADIOBUTTON5
#define PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL2 CHILDWNDINDEX_RADIOBUTTON6
#define PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL3 CHILDWNDINDEX_RADIOBUTTON7
#define PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL4 CHILDWNDINDEX_RADIOBUTTON8
#define PBRUN_CHILDWNDINDEX_BTNGROUPBOX_PARAMS CHILDWNDINDEX_BTNGROUPBOX1
#define PBRUN_CHILDWNDINDEX_BTNGROUPBOX_DRYAMP CHILDWNDINDEX_BTNGROUPBOX2
#define PBRUN_CHILDWNDINDEX_BTNGROUPBOX_OUTAMP CHILDWNDINDEX_BTNGROUPBOX3
#define PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FF CHILDWNDINDEX_BTNGROUPBOX4
#define PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FB CHILDWNDINDEX_BTNGROUPBOX5
#define PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FFCHSEL CHILDWNDINDEX_BTNGROUPBOX6
#define PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FFDELAY CHILDWNDINDEX_BTNGROUPBOX7
#define PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FFAMP CHILDWNDINDEX_BTNGROUPBOX8
#define PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FBCHSEL CHILDWNDINDEX_BTNGROUPBOX9
#define PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FBDELAY CHILDWNDINDEX_BTNGROUPBOX10
#define PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FBAMP CHILDWNDINDEX_BTNGROUPBOX11
#define PBRUN_CHILDWNDINDEX_CONTAINER_PARAMS CHILDWNDINDEX_CONTAINER1
#define PBRUN_CHILDWNDINDEX_CONTAINER_DRYAMP CHILDWNDINDEX_CONTAINER2
#define PBRUN_CHILDWNDINDEX_CONTAINER_OUTAMP CHILDWNDINDEX_CONTAINER3
#define PBRUN_CHILDWNDINDEX_CONTAINER_FF CHILDWNDINDEX_CONTAINER4
#define PBRUN_CHILDWNDINDEX_CONTAINER_FB CHILDWNDINDEX_CONTAINER5
#define PBRUN_CHILDWNDINDEX_CONTAINER_FFCHSEL CHILDWNDINDEX_CONTAINER6
#define PBRUN_CHILDWNDINDEX_CONTAINER_FFDELAY CHILDWNDINDEX_CONTAINER7
#define PBRUN_CHILDWNDINDEX_CONTAINER_FFAMP CHILDWNDINDEX_CONTAINER8
#define PBRUN_CHILDWNDINDEX_CONTAINER_FBCHSEL CHILDWNDINDEX_CONTAINER9
#define PBRUN_CHILDWNDINDEX_CONTAINER_FBDELAY CHILDWNDINDEX_CONTAINER10
#define PBRUN_CHILDWNDINDEX_CONTAINER_FBAMP CHILDWNDINDEX_CONTAINER11

/*============================================================================*/

#define ATTEMPTUPDATEPARAM_DRYAMP 1
#define ATTEMPTUPDATEPARAM_OUTAMP 2
#define ATTEMPTUPDATEPARAM_FFDELAY 3
#define ATTEMPTUPDATEPARAM_FFAMP 4
#define ATTEMPTUPDATEPARAM_FBDELAY 5
#define ATTEMPTUPDATEPARAM_FBAMP 6

__declspec(align(PTR_SIZE_BYTES)) HANDLE p_audiothread = NULL;
__declspec(align(PTR_SIZE_BYTES)) HANDLE h_filein = INVALID_HANDLE_VALUE;

__declspec(align(PTR_SIZE_BYTES)) HBRUSH pp_brush[PP_BRUSH_LENGTH] = {NULL};
__declspec(align(PTR_SIZE_BYTES)) HFONT pp_font[PP_FONT_LENGTH] = {NULL};
__declspec(align(PTR_SIZE_BYTES)) HWND pp_childwnd[PP_CHILDWND_LENGTH] = {NULL};

__declspec(align(PTR_SIZE_BYTES)) HWND p_mainwnd = NULL;

__declspec(align(PTR_SIZE_BYTES)) AudioPB *p_audio = NULL;
__declspec(align(PTR_SIZE_BYTES)) audiopb_params_t pb_params;

__declspec(align(PTR_SIZE_BYTES)) __string tstr = TEXT("");

__declspec(align(4)) INT runtime_status = -1;
__declspec(align(4)) INT prev_status = -1;

__declspec(align(2)) WORD custom_generic_wndclass_id = 0u;

extern BOOL WINAPI app_init(VOID);
extern VOID WINAPI app_deinit(VOID);

extern BOOL WINAPI gdiobj_init(VOID);
extern VOID WINAPI gdiobj_deinit(VOID);

extern BOOL WINAPI register_wndclass(VOID);
extern BOOL WINAPI create_mainwnd(VOID);
extern BOOL WINAPI create_childwnd(VOID);

extern INT WINAPI app_get_ref_status(VOID);

extern VOID WINAPI runtime_loop(VOID);

extern VOID WINAPI paintscreen_choosefile(VOID);
extern VOID WINAPI paintscreen_chooseaudiodev(VOID);
extern VOID WINAPI paintscreen_playback_running(VOID);
extern VOID WINAPI paintscreen_playback_finished(VOID);

extern VOID WINAPI text_choose_font(VOID);
extern VOID WINAPI mainwndchildren_align(VOID);
extern VOID WINAPI containerchildren_align(VOID);
extern VOID WINAPI childwnd_setup(BOOL redraw_mainwnd);

extern BOOL WINAPI window_get_dimensions(HWND p_wnd, INT *p_xpos, INT *p_ypos, INT *p_width, INT *p_height, INT *p_centerx, INT *p_centery);
extern BOOL WINAPI window_set_parent(HWND p_wnd, HWND p_parent);

extern BOOL WINAPI mainwnd_redraw(VOID);
extern VOID WINAPI childwnd_hide_all(VOID);

extern BOOL WINAPI catch_messages(VOID);

extern LRESULT CALLBACK mainwnd_wndproc(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam);

extern LRESULT CALLBACK mainwnd_event_wmcommand(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmdestroy(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmsize(HWND p_wnd, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK mainwnd_event_wmpaint(HWND p_wnd, WPARAM wparam, LPARAM lparam);

extern LRESULT CALLBACK container_wndproc(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam);
extern LRESULT CALLBACK container_wndproc_fwrdtoparent(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam);

extern LRESULT CALLBACK container_event_wmpaint(HWND p_wnd, WPARAM wparam, LPARAM lparam);

extern LRESULT CALLBACK window_event_wmctlcolorstatic(HWND p_wnd, WPARAM wparam, LPARAM lparam);

extern VOID WINAPI fxparams_text_update(VOID);

extern BOOL WINAPI attempt_update_param(HWND textbox, INT param);

extern LONG_PTR WINAPI ffdelay_get_current_chsel(VOID);
extern LONG_PTR WINAPI fbdelay_get_current_chsel(VOID);

extern BOOL WINAPI choosefile_proc(VOID);
extern BOOL WINAPI chooseaudiodev_proc(ULONG_PTR index_sel, BOOL dev_default);
extern BOOL WINAPI initaudioobj_proc(VOID);

extern BOOL WINAPI filein_open(const TCHAR *filein_dir);
extern VOID WINAPI filein_close(VOID);

extern INT WINAPI filein_get_params(VOID);
extern BOOL WINAPI compare_signature(const CHAR *auth, const UINT8 *buf);

extern DWORD WINAPI audiothread_proc(VOID *p_args);

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
	p_instance = hInstance;

	if(!app_init()) return -1;

	runtime_loop();

	app_deinit();
	return 0;
}

BOOL WINAPI app_init(VOID)
{
	HRESULT n_ret = 0;

	if(p_instance == NULL)
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Invalid Instance."));
		goto _l_app_init_error;
	}

	p_processheap = GetProcessHeap();
	if(p_processheap == NULL)
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Invalid Process Heap."));
		goto _l_app_init_error;
	}

	if(!gdiobj_init())
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: GDIOBJ Init Failed."));
		goto _l_app_init_error;
	}

	if(!register_wndclass())
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Register WNDCLASS Failed."));
		goto _l_app_init_error;
	}

	if(!create_mainwnd())
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Create MAINWND Failed."));
		goto _l_app_init_error;
	}

	if(!create_childwnd())
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: Create CHILDWND Failed."));
		goto _l_app_init_error;
	}

	n_ret = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if(n_ret != S_OK)
	{
		__SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Error: COMBASEAPI Init Failed."));
		goto _l_app_init_error;
	}

	runtime_status = RUNTIME_STATUS_INIT;
	return TRUE;

_l_app_init_error:

	MessageBox(NULL, textbuf, TEXT("INIT ERROR"), (MB_ICONSTOP | MB_OK));
	app_deinit();
	return FALSE;
}

VOID WINAPI app_deinit(VOID)
{
	if(p_audiothread != NULL) thread_stop(&p_audiothread, 0u);

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	filein_close();

	if(p_mainwnd != NULL) DestroyWindow(p_mainwnd);

	if(custom_generic_wndclass_id)
	{
		UnregisterClass(CUSTOM_GENERIC_WNDCLASS_NAME, p_instance);
		custom_generic_wndclass_id = 0u;
	}

	gdiobj_deinit();
	CoUninitialize();

	return;
}

__declspec(noreturn) VOID WINAPI app_exit(UINT exit_code, const TCHAR *exit_msg)
{
	app_deinit();

	if(exit_msg != NULL) MessageBox(NULL, exit_msg, TEXT("PROCESS EXIT CALLED"), (MB_ICONSTOP | MB_OK));

	ExitProcess(exit_code);

	while(TRUE) Sleep(16u);
}

BOOL WINAPI gdiobj_init(VOID)
{
	ULONG_PTR n_obj = 0u;
	LOGFONT logfont;

	pp_brush[BRUSHINDEX_TRANSPARENT] = (HBRUSH) GetStockObject(HOLLOW_BRUSH);
	pp_brush[BRUSHINDEX_CUSTOM_SOLID_BLACK] = CreateSolidBrush(CUSTOMCOLOR_BLACK);
	pp_brush[BRUSHINDEX_CUSTOM_SOLID_WHITE] = CreateSolidBrush(CUSTOMCOLOR_WHITE);
	pp_brush[BRUSHINDEX_CUSTOM_SOLID_LTGRAY] = CreateSolidBrush(CUSTOMCOLOR_LTGRAY);

	for(n_obj = 0u; n_obj < PP_BRUSH_LENGTH; n_obj++) if(pp_brush[n_obj] == NULL) return FALSE;

	ZeroMemory(&logfont, sizeof(LOGFONT));

	logfont.lfCharSet = CUSTOMFONT_SMALL_CHARSET;
	logfont.lfWidth = CUSTOMFONT_SMALL_WIDTH;
	logfont.lfHeight = CUSTOMFONT_SMALL_HEIGHT;
	logfont.lfWeight = CUSTOMFONT_SMALL_WEIGHT;

	pp_font[FONTINDEX_CUSTOM_SMALL] = CreateFontIndirect(&logfont);

	logfont.lfCharSet = CUSTOMFONT_MEDIUM_CHARSET;
	logfont.lfWidth = CUSTOMFONT_MEDIUM_WIDTH;
	logfont.lfHeight = CUSTOMFONT_MEDIUM_HEIGHT;
	logfont.lfWeight = CUSTOMFONT_MEDIUM_WEIGHT;

	pp_font[FONTINDEX_CUSTOM_MEDIUM] = CreateFontIndirect(&logfont);

	logfont.lfCharSet = CUSTOMFONT_LARGE_CHARSET;
	logfont.lfWidth = CUSTOMFONT_LARGE_WIDTH;
	logfont.lfHeight = CUSTOMFONT_LARGE_HEIGHT;
	logfont.lfWeight = CUSTOMFONT_LARGE_WEIGHT;

	pp_font[FONTINDEX_CUSTOM_LARGE] = CreateFontIndirect(&logfont);

	for(n_obj = 0u; n_obj < PP_FONT_LENGTH; n_obj++) if(pp_font[n_obj] == NULL) return FALSE;

	return TRUE;
}

VOID WINAPI gdiobj_deinit(VOID)
{
	ULONG_PTR n_obj = 0u;

	for(n_obj = 0u; n_obj < PP_BRUSH_LENGTH; n_obj++)
	{
		if(pp_brush[n_obj] != NULL)
		{
			DeleteObject(pp_brush[n_obj]);
			pp_brush[n_obj] = NULL;
		}
	}

	for(n_obj = 0u; n_obj < PP_FONT_LENGTH; n_obj++)
	{
		if(pp_font[n_obj] != NULL)
		{
			DeleteObject(pp_font[n_obj]);
			pp_font[n_obj] = NULL;
		}
	}

	return;
}

BOOL WINAPI register_wndclass(VOID)
{
	WNDCLASS wndclass;

	ZeroMemory(&wndclass, sizeof(WNDCLASS));

	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = &DefWindowProc;
	wndclass.hInstance = p_instance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = pp_brush[BRUSHINDEX_TRANSPARENT];
	wndclass.lpszClassName = CUSTOM_GENERIC_WNDCLASS_NAME;

	custom_generic_wndclass_id = RegisterClass(&wndclass);

	return (BOOL) custom_generic_wndclass_id;
}

BOOL WINAPI create_mainwnd(VOID)
{
	DWORD style = (WS_CAPTION | WS_VISIBLE | WS_SYSMENU | WS_OVERLAPPED | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SIZEBOX | WS_MAXIMIZE);

	p_mainwnd = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, MAINWND_CAPTION, style, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, p_instance, NULL);
	if(p_mainwnd == NULL) return FALSE;

	if(SetWindowLongPtr(p_mainwnd, GWLP_WNDPROC, (LONG_PTR) &mainwnd_wndproc)) return TRUE;

	return FALSE;

	/*
		Avoid using "return (BOOL) SetWindowLongPtr(...);"

		SetWindowLongPtr() return type is LONG_PTR, casting it to BOOL could cause runtime errors on 64bit Windows.
	*/
}

BOOL WINAPI create_childwnd(VOID)
{
	ULONG_PTR n_wnd = 0u;
	DWORD style = 0u;

	/*=== CONTAINERS ===*/

	style = WS_CHILD;

	pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_PARAMS] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_DRYAMP] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_OUTAMP] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FF] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FB] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	if(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_PARAMS] == NULL) return FALSE;
	if(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_DRYAMP] == NULL) return FALSE;
	if(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_OUTAMP] == NULL) return FALSE;
	if(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FF] == NULL) return FALSE;
	if(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FB] == NULL) return FALSE;

	if(!SetWindowLongPtr(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_PARAMS], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;
	if(!SetWindowLongPtr(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_DRYAMP], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;
	if(!SetWindowLongPtr(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_OUTAMP], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;
	if(!SetWindowLongPtr(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FF], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;
	if(!SetWindowLongPtr(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FB], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;

	pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFCHSEL] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FF], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFDELAY] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FF], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFAMP] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FF], NULL, p_instance, NULL);

	pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBCHSEL] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FB], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBDELAY] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FB], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBAMP] = CreateWindow(CUSTOM_GENERIC_WNDCLASS_NAME, NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FB], NULL, p_instance, NULL);

	if(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFCHSEL] == NULL) return FALSE;
	if(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFDELAY] == NULL) return FALSE;
	if(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFAMP] == NULL) return FALSE;

	if(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBCHSEL] == NULL) return FALSE;
	if(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBDELAY] == NULL) return FALSE;
	if(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBAMP] == NULL) return FALSE;

	if(!SetWindowLongPtr(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFCHSEL], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;
	if(!SetWindowLongPtr(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFDELAY], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;
	if(!SetWindowLongPtr(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFAMP], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;

	if(!SetWindowLongPtr(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBCHSEL], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;
	if(!SetWindowLongPtr(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBDELAY], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;
	if(!SetWindowLongPtr(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBAMP], GWLP_WNDPROC, (LONG_PTR) &container_wndproc)) return FALSE;

	/*=== BUTTON GROUP BOXES ===*/

	style = (WS_CHILD | BS_GROUPBOX);

	pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_PARAMS] = CreateWindow(TEXT("BUTTON"), TEXT("Current Parameters"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_PARAMS], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_DRYAMP] = CreateWindow(TEXT("BUTTON"), TEXT("Dry Input Amplitude"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_DRYAMP], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_OUTAMP] = CreateWindow(TEXT("BUTTON"), TEXT("Output Amplitude"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_OUTAMP], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FF] = CreateWindow(TEXT("BUTTON"), TEXT("Feedforward Delay"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FF], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FB] = CreateWindow(TEXT("BUTTON"), TEXT("Feedback Delay"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FB], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FFCHSEL] = CreateWindow(TEXT("BUTTON"), TEXT("Delay Channel Select"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFCHSEL], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FFDELAY] = CreateWindow(TEXT("BUTTON"), TEXT("Delay Time (# Samples)"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFDELAY], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FFAMP] = CreateWindow(TEXT("BUTTON"), TEXT("Delay Amplitude"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFAMP], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FBCHSEL] = CreateWindow(TEXT("BUTTON"), TEXT("Delay Channel Select"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBCHSEL], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FBDELAY] = CreateWindow(TEXT("BUTTON"), TEXT("Delay Time (# Samples)"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBDELAY], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FBAMP] = CreateWindow(TEXT("BUTTON"), TEXT("Delay Amplitude"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBAMP], NULL, p_instance, NULL);

	/*=== TEXT ===*/

	style = (WS_CHILD | SS_CENTER);

	pp_childwnd[CHILDWNDINDEX_TEXT1] = CreateWindow(TEXT("STATIC"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_SUBHEADER] = CreateWindow(TEXT("STATIC"), TEXT("WARNING: feedback delays are IIR (Infinite Impulse Response) circuits. Improper settings may cause it to clip endlessly, creating a very loud and unpleasant noise."), style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	style = (WS_CHILD | SS_LEFT);

	pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_PARAMS1] = CreateWindow(TEXT("STATIC"), NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_PARAMS], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_PARAMS2] = CreateWindow(TEXT("STATIC"), NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_PARAMS], NULL, p_instance, NULL);

	/*=== TEXT BOXES ===*/

	style = (WS_CHILD | WS_TABSTOP | ES_CENTER);

	pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEDRYAMP] = CreateWindow(TEXT("EDIT"), NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_DRYAMP], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEOUTAMP] = CreateWindow(TEXT("EDIT"), NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_OUTAMP], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFDELAY] = CreateWindow(TEXT("EDIT"), NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFDELAY], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFAMP] = CreateWindow(TEXT("EDIT"), NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFAMP], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBDELAY] = CreateWindow(TEXT("EDIT"), NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBDELAY], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBAMP] = CreateWindow(TEXT("EDIT"), NULL, style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBAMP], NULL, p_instance, NULL);

	/*=== PUSH BUTTONS ===*/

	style = (WS_CHILD | WS_TABSTOP | BS_PUSHBUTTON | BS_TEXT | BS_CENTER | BS_VCENTER);

	pp_childwnd[CHILDWNDINDEX_BUTTON1] = CreateWindow(TEXT("BUTTON"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_BUTTON2] = CreateWindow(TEXT("BUTTON"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);
	pp_childwnd[CHILDWNDINDEX_BUTTON3] = CreateWindow(TEXT("BUTTON"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_RESETFFPARAMS] = CreateWindow(TEXT("BUTTON"), TEXT("Reset Parameters"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FF], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_RESETFBPARAMS] = CreateWindow(TEXT("BUTTON"), TEXT("Reset Parameters"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FB], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFFDELAY] = CreateWindow(TEXT("BUTTON"), TEXT("Update Delay Time"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFDELAY], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFFAMP] = CreateWindow(TEXT("BUTTON"), TEXT("Update Delay Amplitude"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFAMP], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFBDELAY] = CreateWindow(TEXT("BUTTON"), TEXT("Update Delay Time"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBDELAY], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFBAMP] = CreateWindow(TEXT("BUTTON"), TEXT("Update Delay Amplitude"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBAMP], NULL, p_instance, NULL);

	/*=== RADIO BUTTONS ===*/

	style = (WS_CHILD | WS_TABSTOP | BS_LEFT | BS_AUTORADIOBUTTON);

	pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL1] = CreateWindow(TEXT("BUTTON"), TEXT("FF Channel 1"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFCHSEL], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL2] = CreateWindow(TEXT("BUTTON"), TEXT("FF Channel 2"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFCHSEL], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL3] = CreateWindow(TEXT("BUTTON"), TEXT("FF Channel 3"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFCHSEL], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL4] = CreateWindow(TEXT("BUTTON"), TEXT("FF Channel 4"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFCHSEL], NULL, p_instance, NULL);

	pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL1] = CreateWindow(TEXT("BUTTON"), TEXT("FB Channel 1"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBCHSEL], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL2] = CreateWindow(TEXT("BUTTON"), TEXT("FB Channel 2"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBCHSEL], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL3] = CreateWindow(TEXT("BUTTON"), TEXT("FB Channel 3"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBCHSEL], NULL, p_instance, NULL);
	pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL4] = CreateWindow(TEXT("BUTTON"), TEXT("FB Channel 4"), style, 0, 0, 0, 0, pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBCHSEL], NULL, p_instance, NULL);

	/*=== LIST BOX ===*/

	style = (WS_CHILD | WS_TABSTOP | WS_VSCROLL | LBS_HASSTRINGS);

	pp_childwnd[CHILDWNDINDEX_LISTBOX1] = CreateWindow(TEXT("LISTBOX"), NULL, style, 0, 0, 0, 0, p_mainwnd, NULL, p_instance, NULL);

	for(n_wnd = 0u; n_wnd < PP_CHILDWND_LENGTH; n_wnd++) if(pp_childwnd[n_wnd] == NULL) return FALSE;

	return TRUE;
}

INT WINAPI app_get_ref_status(VOID)
{
	if(runtime_status == RUNTIME_STATUS_IDLE) return prev_status;

	return runtime_status;
}

VOID WINAPI runtime_loop(VOID)
{
	while(catch_messages())
	{
		switch(runtime_status)
		{
			case RUNTIME_STATUS_IDLE:
				Sleep(10u);
				break;

			case RUNTIME_STATUS_INIT:
				childwnd_setup(TRUE);
				runtime_status = RUNTIME_STATUS_CHOOSEFILE;

			case RUNTIME_STATUS_CHOOSEFILE:
				paintscreen_choosefile();
				goto _l_runtime_loop_runtimestatus_notidle;

			case RUNTIME_STATUS_CHOOSEAUDIODEV:
				paintscreen_chooseaudiodev();
				goto _l_runtime_loop_runtimestatus_notidle;

			case RUNTIME_STATUS_PLAYBACK_RUNNING:
				paintscreen_playback_running();
				goto _l_runtime_loop_runtimestatus_notidle;

			case RUNTIME_STATUS_PLAYBACK_FINISHED:
				paintscreen_playback_finished();
				goto _l_runtime_loop_runtimestatus_notidle;
		}

_l_runtime_loop_runtimestatus_idle:

		continue;

_l_runtime_loop_runtimestatus_notidle:

		prev_status = runtime_status;
		runtime_status = RUNTIME_STATUS_IDLE;
	}

	return;
}

VOID WINAPI paintscreen_choosefile(VOID)
{
	childwnd_hide_all();

	mainwndchildren_align();

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETTEXT, 0, (LPARAM) TEXT("Choose Audio File"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON1], WM_SETTEXT, 0, (LPARAM) TEXT("Browse"));

	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON1], SW_SHOW);
	return;
}

VOID WINAPI paintscreen_chooseaudiodev(VOID)
{
	childwnd_hide_all();

	window_set_parent(pp_childwnd[CHILDWNDINDEX_BUTTON2], p_mainwnd);
	window_set_parent(pp_childwnd[CHILDWNDINDEX_BUTTON3], p_mainwnd);

	mainwndchildren_align();

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETTEXT, 0, (LPARAM) TEXT("Choose Playback Device"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON1], WM_SETTEXT, 0, (LPARAM) TEXT("Return"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON2], WM_SETTEXT, 0, (LPARAM) TEXT("Choose Selected Device"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON3], WM_SETTEXT, 0, (LPARAM) TEXT("Choose Default Device"));

	if(p_audio->loadAudioDeviceList(pp_childwnd[CHILDWNDINDEX_LISTBOX1]))
	{
		ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON2], SW_SHOW);
		ShowWindow(pp_childwnd[CHILDWNDINDEX_LISTBOX1], SW_SHOW);
	}
	else
	{
		tstr = TEXT("Error: failed to load audio device list.\r\nExtended Error Message: ");
		tstr += p_audio->getLastErrorMessage();
		MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	}

	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON3], SW_SHOW);

	return;
}

VOID WINAPI paintscreen_playback_running(VOID)
{
	childwnd_hide_all();

	window_set_parent(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEDRYAMP], pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_DRYAMP]);
	window_set_parent(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEOUTAMP], pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_OUTAMP]);

	mainwndchildren_align();
	containerchildren_align();

	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_HEADER], WM_SETTEXT, 0, (LPARAM) TEXT("Playback Running"));
	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_STOPPB], WM_SETTEXT, 0, (LPARAM) TEXT("Stop Playback"));
	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEDRYAMP], WM_SETTEXT, 0, (LPARAM) TEXT("Update Dry Input Amplitude"));
	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEOUTAMP], WM_SETTEXT, 0, (LPARAM) TEXT("Update Output Amplitude"));

	fxparams_text_update();

	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEDRYAMP], WM_SETTEXT, 0, (LPARAM) TEXT(""));
	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEOUTAMP], WM_SETTEXT, 0, (LPARAM) TEXT(""));
	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFDELAY], WM_SETTEXT, 0, (LPARAM) TEXT(""));
	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFAMP], WM_SETTEXT, 0, (LPARAM) TEXT(""));
	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBDELAY], WM_SETTEXT, 0, (LPARAM) TEXT(""));
	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBAMP], WM_SETTEXT, 0, (LPARAM) TEXT(""));

	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_HEADER], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_SUBHEADER], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_PARAMS1], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_PARAMS2], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_STOPPB], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEDRYAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEOUTAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_RESETFFPARAMS], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_RESETFBPARAMS], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFFDELAY], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFFAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFBDELAY], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFBAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEDRYAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEOUTAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFDELAY], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBDELAY], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL1], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL2], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL3], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL4], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL1], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL2], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL3], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL4], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_PARAMS], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_DRYAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_OUTAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FF], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FB], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FFCHSEL], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FFDELAY], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FFAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FBCHSEL], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FBDELAY], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FBAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_PARAMS], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_DRYAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_OUTAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FF], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FB], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFCHSEL], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFDELAY], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFAMP], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBCHSEL], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBDELAY], SW_SHOW);
	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBAMP], SW_SHOW);

	return;
}

VOID WINAPI paintscreen_playback_finished(VOID)
{
	childwnd_hide_all();

	mainwndchildren_align();

	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETTEXT, 0, (LPARAM) TEXT("Playback Finished"));
	SendMessage(pp_childwnd[CHILDWNDINDEX_BUTTON1], WM_SETTEXT, 0, (LPARAM) TEXT("Start Over"));

	ShowWindow(pp_childwnd[CHILDWNDINDEX_TEXT1], SW_SHOW);
	ShowWindow(pp_childwnd[CHILDWNDINDEX_BUTTON1], SW_SHOW);

	return;
}

VOID WINAPI text_choose_font(VOID)
{
	SendMessage(pp_childwnd[CHILDWNDINDEX_TEXT1], WM_SETFONT, (WPARAM) pp_font[FONTINDEX_CUSTOM_LARGE], (LPARAM) TRUE);

	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_SUBHEADER], WM_SETFONT, (WPARAM) pp_font[FONTINDEX_CUSTOM_MEDIUM], (LPARAM) TRUE);

	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_PARAMS1], WM_SETFONT, (WPARAM) pp_font[FONTINDEX_CUSTOM_SMALL], (LPARAM) TRUE);
	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_PARAMS2], WM_SETFONT, (WPARAM) pp_font[FONTINDEX_CUSTOM_SMALL], (LPARAM) TRUE);

	return;
}

VOID WINAPI mainwndchildren_align(VOID)
{
	const INT PBRUN_MARGIN = 10;
	const INT BUTTON_HEIGHT = 20;
	const INT TEXT_PARAMS_NLINES_MIN = 8u;
	const INT TEXT_PARAMS_HEIGHT_MIN = TEXT_PARAMS_NLINES_MIN*CUSTOMFONT_SMALL_HEIGHT;

	INT mainwnd_width = 0;
	INT mainwnd_height = 0;
	INT mainwnd_centerx = 0;

	INT text1_dim[4] = {0};

	INT textsubheader_dim[4] = {0};

	INT button1_dim[4] = {0};
	INT button2_dim[4] = {0};
	INT button3_dim[4] = {0};

	INT listbox1_dim[4] = {0};

	INT containerparams_dim[4] = {0};
	INT containerdryamp_dim[4] = {0};
	INT containeroutamp_dim[4] = {0};
	INT containerff_dim[4] = {0};
	INT containerfb_dim[4] = {0};

	INT ref_status = -1;

	window_get_dimensions(p_mainwnd, NULL, NULL, &mainwnd_width, &mainwnd_height, &mainwnd_centerx, NULL);
	ref_status = app_get_ref_status();

	text1_dim[0] = 20;
	text1_dim[1] = 20;
	text1_dim[2] = mainwnd_width - 2*text1_dim[0];
	text1_dim[3] = CUSTOMFONT_LARGE_HEIGHT;

	button1_dim[2] = 200;
	button1_dim[3] = BUTTON_HEIGHT;

	button1_dim[1] = mainwnd_height - BUTTON_HEIGHT - 60;

	if(ref_status == RUNTIME_STATUS_PLAYBACK_RUNNING) goto _l_mainwndchildren_align_playback_running;

	listbox1_dim[2] = text1_dim[2];
	listbox1_dim[0] = text1_dim[0];
	listbox1_dim[1] = text1_dim[1] + text1_dim[3] + 10;
	listbox1_dim[3] = 100;

	switch(ref_status)
	{
		case RUNTIME_STATUS_CHOOSEAUDIODEV:
			button2_dim[1] = button1_dim[1];
			button2_dim[2] = button1_dim[2];
			button2_dim[3] = button1_dim[3];

			button3_dim[1] = button1_dim[1];
			button3_dim[2] = button1_dim[2];
			button3_dim[3] = button1_dim[3];

			button2_dim[0] = mainwnd_centerx - button2_dim[2]/2;
			button3_dim[0] = button2_dim[0] + button2_dim[2] + 10;
			button1_dim[0] = button2_dim[0] - button1_dim[2] - 10;

			SetWindowPos(pp_childwnd[CHILDWNDINDEX_BUTTON2], NULL, button2_dim[0], button2_dim[1], button2_dim[2], button2_dim[3], 0u);
			SetWindowPos(pp_childwnd[CHILDWNDINDEX_BUTTON3], NULL, button3_dim[0], button3_dim[1], button3_dim[2], button3_dim[3], 0u);

			break;

		case RUNTIME_STATUS_CHOOSEFILE:
		case RUNTIME_STATUS_PLAYBACK_FINISHED:

			button1_dim[0] = mainwnd_centerx - button1_dim[2]/2;
			break;
	}

	SetWindowPos(pp_childwnd[CHILDWNDINDEX_TEXT1], NULL, text1_dim[0], text1_dim[1], text1_dim[2], text1_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_BUTTON1], NULL, button1_dim[0], button1_dim[1], button1_dim[2], button1_dim[3], 0u);
	SetWindowPos(pp_childwnd[CHILDWNDINDEX_LISTBOX1], NULL, listbox1_dim[0], listbox1_dim[1], listbox1_dim[2], listbox1_dim[3], 0u);

	return;

_l_mainwndchildren_align_playback_running:

	textsubheader_dim[0] = text1_dim[0];
	textsubheader_dim[2] = text1_dim[2];
	textsubheader_dim[1] = text1_dim[1] + text1_dim[3] + PBRUN_MARGIN;
	textsubheader_dim[3] = 2*CUSTOMFONT_MEDIUM_HEIGHT;

	button1_dim[0] = mainwnd_centerx - button1_dim[2]/2;

	containerfb_dim[0] = text1_dim[0];
	containerfb_dim[2] = text1_dim[2];
	containerfb_dim[3] = 180;
	containerfb_dim[1] = button1_dim[1] - containerfb_dim[3] - PBRUN_MARGIN;

	containerff_dim[0] = containerfb_dim[0];
	containerff_dim[2] = containerfb_dim[2];
	containerff_dim[3] = containerfb_dim[3];
	containerff_dim[1] = containerfb_dim[1] - containerff_dim[3] - PBRUN_MARGIN;

	containerdryamp_dim[0] = text1_dim[0];
	containerdryamp_dim[2] = mainwnd_centerx - containerdryamp_dim[0] - PBRUN_MARGIN/2;
	containerdryamp_dim[3] = 70;
	containerdryamp_dim[1] = containerff_dim[1] - containerdryamp_dim[3] - PBRUN_MARGIN;

	containeroutamp_dim[0] = mainwnd_centerx + PBRUN_MARGIN/2;
	containeroutamp_dim[1] = containerdryamp_dim[1];
	containeroutamp_dim[2] = containerdryamp_dim[2];
	containeroutamp_dim[3] = containerdryamp_dim[3];

	containerparams_dim[0] = text1_dim[0];
	containerparams_dim[2] = text1_dim[2];
	containerparams_dim[3] = TEXT_PARAMS_HEIGHT_MIN;
	containerparams_dim[1] = textsubheader_dim[1] + textsubheader_dim[3] + PBRUN_MARGIN;

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_HEADER], NULL, text1_dim[0], text1_dim[1], text1_dim[2], text1_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_SUBHEADER], NULL, textsubheader_dim[0], textsubheader_dim[1], textsubheader_dim[2], textsubheader_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_STOPPB], NULL, button1_dim[0], button1_dim[1], button1_dim[2], button1_dim[3], 0u);

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_PARAMS], NULL, containerparams_dim[0], containerparams_dim[1], containerparams_dim[2], containerparams_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_DRYAMP], NULL, containerdryamp_dim[0], containerdryamp_dim[1], containerdryamp_dim[2], containerdryamp_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_OUTAMP], NULL, containeroutamp_dim[0], containeroutamp_dim[1], containeroutamp_dim[2], containeroutamp_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FF], NULL, containerff_dim[0], containerff_dim[1], containerff_dim[2], containerff_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FB], NULL, containerfb_dim[0], containerfb_dim[1], containerfb_dim[2], containerfb_dim[3], 0u);

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_PARAMS], NULL, 0, 0, containerparams_dim[2], containerparams_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_DRYAMP], NULL, 0, 0, containerdryamp_dim[2], containerdryamp_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_OUTAMP], NULL, 0, 0, containeroutamp_dim[2], containeroutamp_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FF], NULL, 0, 0, containerff_dim[2], containerff_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FB], NULL, 0, 0, containerfb_dim[2], containerfb_dim[3], 0u);

	return;
}

VOID WINAPI containerchildren_align(VOID)
{
	const INT BUTTON_HEIGHT = 20;
	const INT TEXTBOX_HEIGHT = 20;
	const INT CHILDWND_MARGIN_SMALL = 4;
	const INT CHILDWND_MARGIN_LARGE = 20;

	INT parent_width = 0;
	INT parent_height = 0;
	INT parent_centerx = 0;

	INT text_params1_dim[4] = {0};
	INT text_params2_dim[4] = {0};

	INT textbox_dryoutamp_dim[4] = {0};
	INT button_dryoutamp_dim[4] = {0};

	INT container_chsel_dim[4] = {0};
	INT container_chdelay_dim[4] = {0};
	INT container_champ_dim[4] = {0};

	INT textbox_updatechdelayamp_dim[4] = {0};

	INT button_updatechdelayamp_dim[4] = {0};

	INT button_resetparams_dim[4] = {0};

	INT radiobutton_chsel1_dim[4] = {0};
	INT radiobutton_chsel2_dim[4] = {0};
	INT radiobutton_chsel3_dim[4] = {0};
	INT radiobutton_chsel4_dim[4] = {0};

	/*Container Params Children*/

	window_get_dimensions(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_PARAMS], NULL, NULL, &parent_width, &parent_height, &parent_centerx, NULL);

	text_params1_dim[0] = CHILDWND_MARGIN_SMALL;
	text_params1_dim[1] = CHILDWND_MARGIN_LARGE;
	text_params1_dim[2] = parent_centerx - 2*CHILDWND_MARGIN_SMALL;
	text_params1_dim[3] = parent_height - CHILDWND_MARGIN_LARGE - CHILDWND_MARGIN_SMALL;

	text_params2_dim[0] = parent_centerx + CHILDWND_MARGIN_SMALL;
	text_params2_dim[1] = text_params1_dim[1];
	text_params2_dim[2] = text_params1_dim[2];
	text_params2_dim[3] = text_params1_dim[3];

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_PARAMS1], NULL, text_params1_dim[0], text_params1_dim[1], text_params1_dim[2], text_params1_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_PARAMS2], NULL, text_params2_dim[0], text_params2_dim[1], text_params2_dim[2], text_params2_dim[3], 0u);

	/*Container DryAmp / OutAmp Children*/

	window_get_dimensions(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_DRYAMP], NULL, NULL, &parent_width, &parent_height, &parent_centerx, NULL);

	button_dryoutamp_dim[0] = CHILDWND_MARGIN_SMALL;
	button_dryoutamp_dim[2] = parent_width - 2*CHILDWND_MARGIN_SMALL;
	button_dryoutamp_dim[3] = BUTTON_HEIGHT;
	button_dryoutamp_dim[1] = parent_height - button_dryoutamp_dim[3] - CHILDWND_MARGIN_SMALL;

	textbox_dryoutamp_dim[0] = button_dryoutamp_dim[0];
	textbox_dryoutamp_dim[2] = button_dryoutamp_dim[2];
	textbox_dryoutamp_dim[3] = TEXTBOX_HEIGHT;
	textbox_dryoutamp_dim[1] = button_dryoutamp_dim[1] - textbox_dryoutamp_dim[3] - CHILDWND_MARGIN_SMALL;

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEDRYAMP], NULL, button_dryoutamp_dim[0], button_dryoutamp_dim[1], button_dryoutamp_dim[2], button_dryoutamp_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEOUTAMP], NULL, button_dryoutamp_dim[0], button_dryoutamp_dim[1], button_dryoutamp_dim[2], button_dryoutamp_dim[3], 0u);

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEDRYAMP], NULL, textbox_dryoutamp_dim[0], textbox_dryoutamp_dim[1], textbox_dryoutamp_dim[2], textbox_dryoutamp_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEOUTAMP], NULL, textbox_dryoutamp_dim[0], textbox_dryoutamp_dim[1], textbox_dryoutamp_dim[2], textbox_dryoutamp_dim[3], 0u);

	/*Container FF / FB Children*/

	window_get_dimensions(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FF], NULL, NULL, &parent_width, &parent_height, &parent_centerx, NULL);

	button_resetparams_dim[2] = 240;
	button_resetparams_dim[3] = BUTTON_HEIGHT;
	button_resetparams_dim[0] = parent_centerx - button_resetparams_dim[2]/2;
	button_resetparams_dim[1] = parent_height - BUTTON_HEIGHT - CHILDWND_MARGIN_SMALL;

	container_champ_dim[0] = parent_centerx + CHILDWND_MARGIN_SMALL;
	container_champ_dim[2] = parent_centerx - 2*CHILDWND_MARGIN_SMALL;
	container_champ_dim[3] = TEXTBOX_HEIGHT + BUTTON_HEIGHT + 2*CHILDWND_MARGIN_SMALL + CHILDWND_MARGIN_LARGE;
	container_champ_dim[1] = button_resetparams_dim[1] - container_champ_dim[3] - CHILDWND_MARGIN_SMALL;

	container_chdelay_dim[0] = container_champ_dim[0];
	container_chdelay_dim[2] = container_champ_dim[2];
	container_chdelay_dim[3] = container_champ_dim[3];
	container_chdelay_dim[1] = container_champ_dim[1] - container_chdelay_dim[3] - CHILDWND_MARGIN_SMALL;

	container_chsel_dim[0] = CHILDWND_MARGIN_SMALL;
	container_chsel_dim[2] = parent_centerx - 2*CHILDWND_MARGIN_SMALL;
	container_chsel_dim[3] = 4*(BUTTON_HEIGHT + CHILDWND_MARGIN_SMALL) + CHILDWND_MARGIN_LARGE;
	container_chsel_dim[1] = button_resetparams_dim[1] - container_chsel_dim[3] - CHILDWND_MARGIN_SMALL;

	button_updatechdelayamp_dim[0] = CHILDWND_MARGIN_SMALL;
	button_updatechdelayamp_dim[2] = container_chdelay_dim[2] - 2*CHILDWND_MARGIN_SMALL;
	button_updatechdelayamp_dim[3] = BUTTON_HEIGHT;
	button_updatechdelayamp_dim[1] = container_chdelay_dim[3] - button_updatechdelayamp_dim[3] - CHILDWND_MARGIN_SMALL;

	textbox_updatechdelayamp_dim[0] = button_updatechdelayamp_dim[0];
	textbox_updatechdelayamp_dim[2] = button_updatechdelayamp_dim[2];
	textbox_updatechdelayamp_dim[3] = TEXTBOX_HEIGHT;
	textbox_updatechdelayamp_dim[1] = button_updatechdelayamp_dim[1] - textbox_updatechdelayamp_dim[3] - CHILDWND_MARGIN_SMALL;

	radiobutton_chsel4_dim[0] = CHILDWND_MARGIN_SMALL;
	radiobutton_chsel4_dim[2] = container_chsel_dim[2] - 2*CHILDWND_MARGIN_SMALL;
	radiobutton_chsel4_dim[3] = BUTTON_HEIGHT;
	radiobutton_chsel4_dim[1] = container_chsel_dim[3] - radiobutton_chsel4_dim[3] - CHILDWND_MARGIN_SMALL;

	radiobutton_chsel3_dim[0] = radiobutton_chsel4_dim[0];
	radiobutton_chsel3_dim[2] = radiobutton_chsel4_dim[2];
	radiobutton_chsel3_dim[3] = radiobutton_chsel4_dim[3];
	radiobutton_chsel3_dim[1] = radiobutton_chsel4_dim[1] - radiobutton_chsel3_dim[3] - CHILDWND_MARGIN_SMALL;

	radiobutton_chsel2_dim[0] = radiobutton_chsel4_dim[0];
	radiobutton_chsel2_dim[2] = radiobutton_chsel4_dim[2];
	radiobutton_chsel2_dim[3] = radiobutton_chsel4_dim[3];
	radiobutton_chsel2_dim[1] = radiobutton_chsel3_dim[1] - radiobutton_chsel2_dim[3] - CHILDWND_MARGIN_SMALL;

	radiobutton_chsel1_dim[0] = radiobutton_chsel4_dim[0];
	radiobutton_chsel1_dim[2] = radiobutton_chsel4_dim[2];
	radiobutton_chsel1_dim[3] = radiobutton_chsel4_dim[3];
	radiobutton_chsel1_dim[1] = radiobutton_chsel2_dim[1] - radiobutton_chsel1_dim[3] - CHILDWND_MARGIN_SMALL;

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFCHSEL], NULL, container_chsel_dim[0], container_chsel_dim[1], container_chsel_dim[2], container_chsel_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFDELAY], NULL, container_chdelay_dim[0], container_chdelay_dim[1], container_chdelay_dim[2], container_chdelay_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FFAMP], NULL, container_champ_dim[0], container_champ_dim[1], container_champ_dim[2], container_champ_dim[3], 0u);

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBCHSEL], NULL, container_chsel_dim[0], container_chsel_dim[1], container_chsel_dim[2], container_chsel_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBDELAY], NULL, container_chdelay_dim[0], container_chdelay_dim[1], container_chdelay_dim[2], container_chdelay_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_FBAMP], NULL, container_champ_dim[0], container_champ_dim[1], container_champ_dim[2], container_champ_dim[3], 0u);

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FFCHSEL], NULL, 0, 0, container_chsel_dim[2], container_chsel_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FFDELAY], NULL, 0, 0, container_chdelay_dim[2], container_chdelay_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FFAMP], NULL, 0, 0, container_champ_dim[2], container_champ_dim[3], 0u);

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FBCHSEL], NULL, 0, 0, container_chsel_dim[2], container_chsel_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FBDELAY], NULL, 0, 0, container_chdelay_dim[2], container_chdelay_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BTNGROUPBOX_FBAMP], NULL, 0, 0, container_champ_dim[2], container_champ_dim[3], 0u);

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_RESETFFPARAMS], NULL, button_resetparams_dim[0], button_resetparams_dim[1], button_resetparams_dim[2], button_resetparams_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_RESETFBPARAMS], NULL, button_resetparams_dim[0], button_resetparams_dim[1], button_resetparams_dim[2], button_resetparams_dim[3], 0u);

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFFDELAY], NULL, button_updatechdelayamp_dim[0], button_updatechdelayamp_dim[1], button_updatechdelayamp_dim[2], button_updatechdelayamp_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFFAMP], NULL, button_updatechdelayamp_dim[0], button_updatechdelayamp_dim[1], button_updatechdelayamp_dim[2], button_updatechdelayamp_dim[3], 0u);

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFBDELAY], NULL, button_updatechdelayamp_dim[0], button_updatechdelayamp_dim[1], button_updatechdelayamp_dim[2], button_updatechdelayamp_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFBAMP], NULL, button_updatechdelayamp_dim[0], button_updatechdelayamp_dim[1], button_updatechdelayamp_dim[2], button_updatechdelayamp_dim[3], 0u);

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFDELAY], NULL, textbox_updatechdelayamp_dim[0], textbox_updatechdelayamp_dim[1], textbox_updatechdelayamp_dim[2], textbox_updatechdelayamp_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFAMP], NULL, textbox_updatechdelayamp_dim[0], textbox_updatechdelayamp_dim[1], textbox_updatechdelayamp_dim[2], textbox_updatechdelayamp_dim[3], 0u);

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBDELAY], NULL, textbox_updatechdelayamp_dim[0], textbox_updatechdelayamp_dim[1], textbox_updatechdelayamp_dim[2], textbox_updatechdelayamp_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBAMP], NULL, textbox_updatechdelayamp_dim[0], textbox_updatechdelayamp_dim[1], textbox_updatechdelayamp_dim[2], textbox_updatechdelayamp_dim[3], 0u);

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL1], NULL, radiobutton_chsel1_dim[0], radiobutton_chsel1_dim[1], radiobutton_chsel1_dim[2], radiobutton_chsel1_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL2], NULL, radiobutton_chsel2_dim[0], radiobutton_chsel2_dim[1], radiobutton_chsel2_dim[2], radiobutton_chsel2_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL3], NULL, radiobutton_chsel3_dim[0], radiobutton_chsel3_dim[1], radiobutton_chsel3_dim[2], radiobutton_chsel3_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL4], NULL, radiobutton_chsel4_dim[0], radiobutton_chsel4_dim[1], radiobutton_chsel4_dim[2], radiobutton_chsel4_dim[3], 0u);

	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL1], NULL, radiobutton_chsel1_dim[0], radiobutton_chsel1_dim[1], radiobutton_chsel1_dim[2], radiobutton_chsel1_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL2], NULL, radiobutton_chsel2_dim[0], radiobutton_chsel2_dim[1], radiobutton_chsel2_dim[2], radiobutton_chsel2_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL3], NULL, radiobutton_chsel3_dim[0], radiobutton_chsel3_dim[1], radiobutton_chsel3_dim[2], radiobutton_chsel3_dim[3], 0u);
	SetWindowPos(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL4], NULL, radiobutton_chsel4_dim[0], radiobutton_chsel4_dim[1], radiobutton_chsel4_dim[2], radiobutton_chsel4_dim[3], 0u);

	return;
}

VOID WINAPI childwnd_setup(BOOL redraw_mainwnd)
{
	text_choose_font();

	mainwndchildren_align();
	containerchildren_align();

	if(redraw_mainwnd) mainwnd_redraw();
	return;
}

BOOL WINAPI window_get_dimensions(HWND p_wnd, INT *p_xpos, INT *p_ypos, INT *p_width, INT *p_height, INT *p_centerx, INT *p_centery)
{
	RECT rect;

	if(p_wnd == NULL) return FALSE;
	if(!GetWindowRect(p_wnd, &rect)) return FALSE;

	if(p_xpos != NULL) *p_xpos = rect.left;
	if(p_ypos != NULL) *p_ypos = rect.top;
	if(p_width != NULL) *p_width = rect.right - rect.left;
	if(p_height != NULL) *p_height = rect.bottom - rect.top;
	if(p_centerx != NULL) *p_centerx = (rect.right - rect.left)/2;
	if(p_centery != NULL) *p_centery = (rect.bottom - rect.top)/2;

	return TRUE;
}

BOOL WINAPI window_set_parent(HWND p_wnd, HWND p_parent)
{
	if(p_wnd == NULL) return FALSE;

	if(SetWindowLongPtr(p_wnd, GWLP_HWNDPARENT, (LONG_PTR) p_parent)) return TRUE;

	return FALSE;

	/*
		Avoid using "return (BOOL) SetWindowLongPtr(...);"

		SetWindowLongPtr() return type is LONG_PTR, casting it to BOOL could cause runtime errors on 64bit Windows.
	*/
}

BOOL WINAPI mainwnd_redraw(VOID)
{
	if(p_mainwnd == NULL) return FALSE;

	return RedrawWindow(p_mainwnd, NULL, NULL, (RDW_ERASE | RDW_FRAME | RDW_INTERNALPAINT | RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN));
}

VOID WINAPI childwnd_hide_all(VOID)
{
	SIZE_T n_wnd = 0u;

	for(n_wnd = 0u; n_wnd < PP_CHILDWND_LENGTH; n_wnd++) ShowWindow(pp_childwnd[n_wnd], SW_HIDE);
	return;
}

BOOL WINAPI catch_messages(VOID)
{
	MSG msg;

	while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if(msg.message == WM_QUIT) return FALSE;

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return TRUE;
}

LRESULT CALLBACK mainwnd_wndproc(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_COMMAND:
			return mainwnd_event_wmcommand(p_wnd, wparam, lparam);

		case WM_DESTROY:
			return mainwnd_event_wmdestroy(p_wnd, wparam, lparam);

		case WM_SIZE:
			return mainwnd_event_wmsize(p_wnd, wparam, lparam);

		case WM_PAINT:
			return mainwnd_event_wmpaint(p_wnd, wparam, lparam);

		case WM_CTLCOLORSTATIC:
			return window_event_wmctlcolorstatic(p_wnd, wparam, lparam);

		case CUSTOM_WM_PLAYBACK_FINISHED:
			thread_stop(&p_audiothread, 0u);
			if(p_audio != NULL)
			{
				delete p_audio;
				p_audio = NULL;
			}

			runtime_status = RUNTIME_STATUS_PLAYBACK_FINISHED;
			return 0;
	}

	return DefWindowProc(p_wnd, msg, wparam, lparam);
}

LRESULT CALLBACK mainwnd_event_wmcommand(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	LONG_PTR _numptr = 0;

	if(p_wnd == NULL) return 0;
	if(!lparam) return 0;

	if(prev_status == RUNTIME_STATUS_PLAYBACK_RUNNING) goto _l_mainwnd_event_wmcommand_playback_running;

	if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[CHILDWNDINDEX_BUTTON1]))
	{
		switch(prev_status)
		{
			case RUNTIME_STATUS_CHOOSEFILE:
				if(choosefile_proc()) runtime_status = RUNTIME_STATUS_CHOOSEAUDIODEV;
				break;

			case RUNTIME_STATUS_CHOOSEAUDIODEV:
				if(p_audio != NULL)
				{
					delete p_audio;
					p_audio = NULL;
				}

			case RUNTIME_STATUS_PLAYBACK_FINISHED:

				runtime_status = RUNTIME_STATUS_CHOOSEFILE;
				break;
		}
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[CHILDWNDINDEX_BUTTON2]))
	{
		if(prev_status == RUNTIME_STATUS_CHOOSEAUDIODEV)
		{
			_numptr = listbox_get_sel_index(pp_childwnd[CHILDWNDINDEX_LISTBOX1]);
			if(_numptr < 0)
			{
				MessageBox(NULL, TEXT("Error: no item selected"), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
				return 0;
			}

			if(!chooseaudiodev_proc((ULONG_PTR) _numptr, FALSE)) return 0;
			if(!initaudioobj_proc()) return 0;

			p_audiothread = thread_create_default(&audiothread_proc, NULL, NULL);
			runtime_status = RUNTIME_STATUS_PLAYBACK_RUNNING;
		}
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[CHILDWNDINDEX_BUTTON3]))
	{
		if(prev_status == RUNTIME_STATUS_CHOOSEAUDIODEV)
		{
			if(!chooseaudiodev_proc(0u, TRUE)) return 0;
			if(!initaudioobj_proc()) return 0;

			p_audiothread = thread_create_default(&audiothread_proc, NULL, NULL);
			runtime_status = RUNTIME_STATUS_PLAYBACK_RUNNING;
		}
	}

	return 0;

_l_mainwnd_event_wmcommand_playback_running:

	if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_STOPPB]))
	{
		p_audio->stopPlayback();
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEDRYAMP]))
	{
		attempt_update_param(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEDRYAMP], ATTEMPTUPDATEPARAM_DRYAMP);
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEOUTAMP]))
	{
		attempt_update_param(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEOUTAMP], ATTEMPTUPDATEPARAM_OUTAMP);
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_RESETFFPARAMS]))
	{
		p_audio->rtdelayResetFFParams();
		fxparams_text_update();

		SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFDELAY], WM_SETTEXT, 0, (LPARAM) TEXT(""));
		SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFAMP], WM_SETTEXT, 0, (LPARAM) TEXT(""));
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_RESETFBPARAMS]))
	{
		p_audio->rtdelayResetFBParams();
		fxparams_text_update();

		SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBDELAY], WM_SETTEXT, 0, (LPARAM) TEXT(""));
		SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBAMP], WM_SETTEXT, 0, (LPARAM) TEXT(""));
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFFDELAY]))
	{
		attempt_update_param(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFDELAY], ATTEMPTUPDATEPARAM_FFDELAY);
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFFAMP]))
	{
		attempt_update_param(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFFAMP], ATTEMPTUPDATEPARAM_FFAMP);
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFBDELAY]))
	{
		attempt_update_param(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBDELAY], ATTEMPTUPDATEPARAM_FBDELAY);
	}
	else if(((ULONG_PTR) lparam) == ((ULONG_PTR) pp_childwnd[PBRUN_CHILDWNDINDEX_BUTTON_UPDATEFBAMP]))
	{
		attempt_update_param(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXTBOX_UPDATEFBAMP], ATTEMPTUPDATEPARAM_FBAMP);
	}

	return 0;
}

LRESULT CALLBACK mainwnd_event_wmdestroy(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	ULONG_PTR n_wnd = 0u;

	listbox_clear(pp_childwnd[CHILDWNDINDEX_LISTBOX1]);

	p_mainwnd = NULL;
	for(n_wnd = 0u; n_wnd < PP_CHILDWND_LENGTH; n_wnd++) pp_childwnd[n_wnd] = NULL;

	PostQuitMessage(0);
	return 0;
}

LRESULT CALLBACK mainwnd_event_wmsize(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	if(p_wnd != NULL) childwnd_setup(TRUE);

	return 0;
}

LRESULT CALLBACK mainwnd_event_wmpaint(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	HDC p_wnddc = NULL;
	PAINTSTRUCT ps;

	if(p_wnd == NULL) return 0;

	p_wnddc = BeginPaint(p_wnd, &ps);
	if(p_wnddc == NULL) return 0;

	FillRect(ps.hdc, &ps.rcPaint, pp_brush[MAINWND_BRUSHINDEX]);
	EndPaint(p_wnd, &ps);

	return 0;
}

LRESULT CALLBACK container_wndproc(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch(msg)
	{
		case WM_COMMAND:
			return container_wndproc_fwrdtoparent(p_wnd, msg, wparam, lparam);

		case WM_PAINT:
			return container_event_wmpaint(p_wnd, wparam, lparam);

		case WM_CTLCOLORSTATIC:
			return window_event_wmctlcolorstatic(p_wnd, wparam, lparam);
	}

	return DefWindowProc(p_wnd, msg, wparam, lparam);
}

LRESULT CALLBACK container_wndproc_fwrdtoparent(HWND p_wnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	HWND p_parentwnd = NULL;

	if(p_wnd == NULL) return 0;

	p_parentwnd = (HWND) GetWindowLongPtr(p_wnd, GWLP_HWNDPARENT);
	if(p_parentwnd == NULL) return 0;

	PostMessage(p_parentwnd, msg, wparam, lparam);
	return 0;
}

LRESULT CALLBACK container_event_wmpaint(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	HDC p_wnddc = NULL;
	PAINTSTRUCT ps;

	if(p_wnd == NULL) return 0;

	p_wnddc = BeginPaint(p_wnd, &ps);
	if(p_wnddc == NULL) return 0;

	FillRect(ps.hdc, &ps.rcPaint, pp_brush[CONTAINERWND_BRUSHINDEX]);
	EndPaint(p_wnd, &ps);

	return 0;
}

LRESULT CALLBACK window_event_wmctlcolorstatic(HWND p_wnd, WPARAM wparam, LPARAM lparam)
{
	if(p_wnd == NULL) return 0;
	if(!wparam) return 0;
	if(!lparam) return 0;

	SetTextColor((HDC) wparam, TEXTWND_TEXTCOLOR);
	SetBkColor((HDC) wparam, TEXTWND_BKCOLOR);

	return (LRESULT) pp_brush[TEXTWND_BRUSHINDEX];
}

BOOL WINAPI listbox_clear(HWND p_listbox)
{
	ULONG_PTR n_items = 0u;
	LONG_PTR _numptr = 0;

	if(p_listbox == NULL) return FALSE;

	_numptr = listbox_get_item_count(p_listbox);
	if(_numptr < 0) return FALSE;

	n_items = (ULONG_PTR) _numptr;

	while(n_items > 0u)
	{
		_numptr = listbox_remove_item(p_listbox, (n_items - 1u));
		if(_numptr < 0) return FALSE;

		n_items--;
	}

	return TRUE;
}

LONG_PTR WINAPI listbox_add_item(HWND p_listbox, const TCHAR *text)
{
	if(p_listbox == NULL) return -1;
	if(text == NULL) return -1;

	return (LONG_PTR) SendMessage(p_listbox, LB_ADDSTRING, 0, (LPARAM) text);
}

LONG_PTR WINAPI listbox_remove_item(HWND p_listbox, ULONG_PTR index)
{
	if(p_listbox == NULL) return -1;

	return (LONG_PTR) SendMessage(p_listbox, LB_DELETESTRING, (WPARAM) index, 0);
}

LONG_PTR WINAPI listbox_get_item_count(HWND p_listbox)
{
	if(p_listbox == NULL) return -1;

	return (LONG_PTR) SendMessage(p_listbox, LB_GETCOUNT, 0, 0);
}

LONG_PTR WINAPI listbox_get_sel_index(HWND p_listbox)
{
	if(p_listbox == NULL) return -1;

	return (LONG_PTR) SendMessage(p_listbox, LB_GETCURSEL, 0, 0);
}

VOID WINAPI fxparams_text_update(VOID)
{
	ULONG_PTR n_fx = 0u;
	audiortdelay_fx_params_t fx_params;

	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_PARAMS], SW_HIDE);

	tstr = TEXT("Dry Input Amplitude: ");
	tstr += __TOSTRING(p_audio->rtdelayGetDryInputAmplitude());
	tstr += TEXT("\r\n");

	n_fx = 0u;
	while(n_fx < __RTDELAY_N_FFCH)
	{
		p_audio->rtdelayGetFFParams(n_fx, &fx_params);

		tstr += TEXT("\r\nFF Delay Channel ");
		tstr += __TOSTRING(n_fx + 1u);
		tstr += TEXT(":    Delay Time: ");
		tstr += __TOSTRING(fx_params.delay);
		tstr += TEXT(" samples    Delay Amplitude: ");
		tstr += __TOSTRING(fx_params.amp);
		n_fx++;
	}

	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_PARAMS1], WM_SETTEXT, 0, (LPARAM) tstr.c_str());

	tstr = TEXT("Output Amplitude: ");
	tstr += __TOSTRING(p_audio->rtdelayGetOutputAmplitude());
	tstr += TEXT("\r\n");

	n_fx = 0u;
	while(n_fx < __RTDELAY_N_FBCH)
	{
		p_audio->rtdelayGetFBParams(n_fx, &fx_params);

		tstr += TEXT("\r\nFB Delay Channel ");
		tstr += __TOSTRING(n_fx + 1u);
		tstr += TEXT(":    Delay Time: ");
		tstr += __TOSTRING(fx_params.delay);
		tstr += TEXT(" samples    Delay Amplitude: ");
		tstr += __TOSTRING(fx_params.amp);
		n_fx++;
	}

	SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_TEXT_PARAMS2], WM_SETTEXT, 0, (LPARAM) tstr.c_str());

	ShowWindow(pp_childwnd[PBRUN_CHILDWNDINDEX_CONTAINER_PARAMS], SW_SHOW);
	return;
}

BOOL WINAPI attempt_update_param(HWND textbox, INT param)
{
	LONG_PTR n_fx;
	ULONG_PTR i_val;
	FLOAT f_val;
	BOOL b_ret;

	if(textbox == NULL)
	{
		tstr = TEXT("Error: internal error.");
		goto _l_attempt_update_param_error;
	}

	switch(param)
	{
		case ATTEMPTUPDATEPARAM_FFDELAY:
		case ATTEMPTUPDATEPARAM_FFAMP:

			n_fx = ffdelay_get_current_chsel();
			if(n_fx < 0)
			{
				tstr = TEXT("Error: no feedforward delay channel selected.");
				goto _l_attempt_update_param_error;
			}

			break;

		case ATTEMPTUPDATEPARAM_FBDELAY:
		case ATTEMPTUPDATEPARAM_FBAMP:

			n_fx = fbdelay_get_current_chsel();
			if(n_fx < 0)
			{
				tstr = TEXT("Error: no feedback delay channel selected.");
				goto _l_attempt_update_param_error;
			}

			break;
	}

	SendMessage(textbox, WM_GETTEXT, (WPARAM) TEXTBUF_SIZE_CHARS, (LPARAM) textbuf);

	switch(param)
	{
		case ATTEMPTUPDATEPARAM_DRYAMP:
		case ATTEMPTUPDATEPARAM_OUTAMP:
		case ATTEMPTUPDATEPARAM_FFAMP:
		case ATTEMPTUPDATEPARAM_FBAMP:

			try
			{
				f_val = std::stof(textbuf);
			}
			catch(...)
			{
				tstr = TEXT("Error: invalid amplitude value entered.");
				goto _l_attempt_update_param_error;
			}

			break;

		case ATTEMPTUPDATEPARAM_FFDELAY:
		case ATTEMPTUPDATEPARAM_FBDELAY:

			try
			{
				i_val = std::stoi(textbuf);
			}
			catch(...)
			{
				tstr = TEXT("Error: invalid delay time value entered.");
				goto _l_attempt_update_param_error;
			}

			if(i_val < 0)
			{
				tstr = TEXT("Error: invalid delay time value entered.");
				goto _l_attempt_update_param_error;
			}

			break;
	}

	switch(param)
	{
		case ATTEMPTUPDATEPARAM_DRYAMP:
			b_ret = p_audio->rtdelaySetDryInputAmplitude(f_val);
			break;

		case ATTEMPTUPDATEPARAM_OUTAMP:
			b_ret = p_audio->rtdelaySetOutputAmplitude(f_val);
			break;

		case ATTEMPTUPDATEPARAM_FFDELAY:
			b_ret = p_audio->rtdelaySetFFDelay((ULONG_PTR) n_fx, (ULONG_PTR) i_val);
			break;

		case ATTEMPTUPDATEPARAM_FFAMP:
			b_ret = p_audio->rtdelaySetFFAmplitude((ULONG_PTR) n_fx, f_val);
			break;

		case ATTEMPTUPDATEPARAM_FBDELAY:
			b_ret = p_audio->rtdelaySetFBDelay((ULONG_PTR) n_fx, (ULONG_PTR) i_val);
			break;

		case ATTEMPTUPDATEPARAM_FBAMP:
			b_ret = p_audio->rtdelaySetFBAmplitude((ULONG_PTR) n_fx, f_val);
			break;

		default:
			tstr = TEXT("Error: internal error.");
			goto _l_attempt_update_param_error;
	}

	if(!b_ret)
	{
		tstr = p_audio->getLastErrorMessage();
		goto _l_attempt_update_param_error;
	}

	fxparams_text_update();
	SendMessage(textbox, WM_SETTEXT, 0, (LPARAM) TEXT(""));
	return TRUE;

_l_attempt_update_param_error:

	MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	return FALSE;
}

LONG_PTR WINAPI ffdelay_get_current_chsel(VOID)
{
	LRESULT btn_state = 0;

	btn_state = SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL1], BM_GETCHECK, 0, 0);
	if(btn_state == BST_CHECKED) return 0;

	btn_state = SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL2], BM_GETCHECK, 0, 0);
	if(btn_state == BST_CHECKED) return 1;

	btn_state = SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL3], BM_GETCHECK, 0, 0);
	if(btn_state == BST_CHECKED) return 2;

	btn_state = SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FFCHSEL4], BM_GETCHECK, 0, 0);
	if(btn_state == BST_CHECKED) return 3;

	return -1;
}

LONG_PTR WINAPI fbdelay_get_current_chsel(VOID)
{
	LRESULT btn_state = 0;

	btn_state = SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL1], BM_GETCHECK, 0, 0);
	if(btn_state == BST_CHECKED) return 0;

	btn_state = SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL2], BM_GETCHECK, 0, 0);
	if(btn_state == BST_CHECKED) return 1;

	btn_state = SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL3], BM_GETCHECK, 0, 0);
	if(btn_state == BST_CHECKED) return 2;

	btn_state = SendMessage(pp_childwnd[PBRUN_CHILDWNDINDEX_RADIOBUTTON_FBCHSEL4], BM_GETCHECK, 0, 0);
	if(btn_state == BST_CHECKED) return 3;

	return -1;
}

BOOL WINAPI choosefile_proc(VOID)
{
	ULONG_PTR textlen = 0u;
	INT i32 = 0;
	OPENFILENAME ofdlg;
	const TCHAR *filters = TEXT("Wave Files\0*.wav;*.WAV\0All Files\0*.*\0\0");

	ZeroMemory(&ofdlg, sizeof(OPENFILENAME));
	ZeroMemory(textbuf, TEXTBUF_SIZE_BYTES);

	ofdlg.lStructSize = sizeof(OPENFILENAME);
	ofdlg.hwndOwner = p_mainwnd;
	ofdlg.lpstrFilter = filters;
	ofdlg.nFilterIndex = 1;
	ofdlg.lpstrFile = textbuf;
	ofdlg.nMaxFile = TEXTBUF_SIZE_CHARS;
	ofdlg.Flags = (OFN_EXPLORER | OFN_ENABLESIZING);
	ofdlg.lpstrDefExt = TEXT(".wav");

	if(!GetOpenFileName(&ofdlg))
	{
		tstr = TEXT("Error: Open File Dialog Failed.");
		goto _l_choosefile_proc_error;
	}

	tstr = textbuf;
	cstr_tolower(textbuf, TEXTBUF_SIZE_CHARS);
	textlen = (ULONG_PTR) cstr_getlength(textbuf);

	if(textlen >= 5u)
		if(cstr_compare(TEXT(".wav"), &textbuf[textlen - 4u]))
			goto _l_choosefile_proc_fileextcheck_complete;

	i32 = MessageBox(NULL, TEXT("WARNING: Selected file does not have a \".wav\" file extension.\r\nMight be incompatible with this application.\r\nDo you wish to continue?"), TEXT("WARNING: FILE EXTENSION"), (MB_ICONEXCLAMATION | MB_YESNO));

	if(i32 != IDYES)
	{
		tstr = TEXT("Error: Bad File Extension.");
		goto _l_choosefile_proc_error;
	}

_l_choosefile_proc_fileextcheck_complete:

	if(!filein_open(tstr.c_str()))
	{
		tstr = TEXT("Error: could not open file.");
		goto _l_choosefile_proc_error;
	}

	i32 = filein_get_params();
	if(i32 < 0) goto _l_choosefile_proc_error;

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	pb_params.delay_buffer_size_frames = __RTDELAY_BUFFER_SIZE_FRAMES;
	pb_params.n_ff_delays = __RTDELAY_N_FFCH;
	pb_params.n_fb_delays = __RTDELAY_N_FBCH;
	pb_params.file_dir = tstr.c_str();

	switch(i32)
	{
		case PB_I16:
			p_audio = new AudioPB_i16(&pb_params);
			break;

		case PB_I24:
			p_audio = new AudioPB_i24(&pb_params);
			break;
	}

	if(p_audio != NULL) return TRUE;

	tstr = TEXT("Error: failed to create audio object instance.");

_l_choosefile_proc_error:

	filein_close();
	MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	return FALSE;
}

BOOL WINAPI chooseaudiodev_proc(ULONG_PTR sel_index, BOOL dev_default)
{
	if(p_audio == NULL) return FALSE;

	if(dev_default) goto _l_chooseaudiodev_proc_default;

_l_chooseaudiodev_proc_index:
	if(p_audio->chooseDevice(sel_index)) return TRUE;

	goto _l_chooseaudiodev_proc_error;

_l_chooseaudiodev_proc_default:
	if(p_audio->chooseDefaultDevice()) return TRUE;

_l_chooseaudiodev_proc_error:
	tstr = TEXT("Error: failed to access audio device\r\nExtended error message: ");
	tstr += p_audio->getLastErrorMessage();

	MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	return FALSE;
}

BOOL WINAPI initaudioobj_proc(VOID)
{
	if(p_audio == NULL) return FALSE;

	if(p_audio->initialize()) return TRUE;

	tstr = TEXT("Error: failed to initialize audio object\r\nExtended error message: ");
	tstr += p_audio->getLastErrorMessage();

	MessageBox(NULL, tstr.c_str(), TEXT("ERROR"), (MB_ICONEXCLAMATION | MB_OK));
	return FALSE;
}

BOOL WINAPI filein_open(const TCHAR *filein_dir)
{
	if(filein_dir == NULL) return FALSE;

	filein_close();

	h_filein = CreateFile(filein_dir, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0u, NULL);

	return (h_filein != INVALID_HANDLE_VALUE);
}

VOID WINAPI filein_close(VOID)
{
	if(h_filein == INVALID_HANDLE_VALUE) return;

	CloseHandle(h_filein);
	h_filein = INVALID_HANDLE_VALUE;
	return;
}

INT WINAPI filein_get_params(VOID)
{
	const ULONG_PTR BUFFER_SIZE = 8192u;
	ULONG_PTR buffer_index = 0u;

	UINT8 *p_headerinfo = NULL;

	DWORD dummy_32;

	UINT32 u32 = 0u;
	UINT16 u16 = 0u;

	UINT16 bit_depth = 0u;

	p_headerinfo = (UINT8*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, BUFFER_SIZE);
	if(p_headerinfo == NULL)
	{
		tstr = TEXT("filein_get_params: Error: memory allocate failed.");
		goto _l_filein_get_params_error;
	}

	SetFilePointer(h_filein, 0, NULL, FILE_BEGIN);
	ReadFile(h_filein, p_headerinfo, (DWORD) BUFFER_SIZE, &dummy_32, NULL);
	filein_close();

	if(!compare_signature("RIFF", p_headerinfo))
	{
		tstr = TEXT("filein_get_params: Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	if(!compare_signature("WAVE", (const UINT8*) (((ULONG_PTR) p_headerinfo) + 8u)))
	{
		tstr = TEXT("filein_get_params: Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	buffer_index = 12u;

	while(TRUE)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			tstr = TEXT("filein_get_params: Error: broken header (missing subchunk \"fmt \").\r\nFile probably corrupted.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("fmt ", (const UINT8*) (((ULONG_PTR) p_headerinfo) + buffer_index))) break;

		u32 = *((UINT32*) (((ULONG_PTR) p_headerinfo) + buffer_index + 4u));
		buffer_index += (ULONG_PTR) (u32 + 8u);
	}

	if(buffer_index > (BUFFER_SIZE - 24u))
	{
		tstr = TEXT("filein_get_params: Error: broken header (error on subchunk \"fmt \").\r\nFile might be corrupted.");
		goto _l_filein_get_params_error;
	}

	u16 = *((UINT16*) (((ULONG_PTR) p_headerinfo) + buffer_index + 8u));

	if(u16 != 1u)
	{
		tstr = TEXT("filein_get_params: Error: audio encoding format not supported.");
		goto _l_filein_get_params_error;
	}

	pb_params.n_channels = *((UINT16*) (((ULONG_PTR) p_headerinfo) + buffer_index + 10u));
	pb_params.sample_rate = *((UINT32*) (((ULONG_PTR) p_headerinfo) + buffer_index + 12u));

	bit_depth = *((UINT16*) (((ULONG_PTR) p_headerinfo) + buffer_index + 22u));

	u32 = *((UINT32*) (((ULONG_PTR) p_headerinfo) + buffer_index + 4u));
	buffer_index += (ULONG_PTR) (u32 + 8u);

	while(TRUE)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			tstr = TEXT("filein_get_params: Error: broken header (missing subchunk \"data\").\r\nFile probably corrupted.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("data", (const UINT8*) (((ULONG_PTR) p_headerinfo) + buffer_index))) break;

		u32 = *((UINT32*) (((ULONG_PTR) p_headerinfo) + buffer_index + 4u));
		buffer_index += (ULONG_PTR) (u32 + 8u);
	}

	u32 = *((UINT32*) (((ULONG_PTR) p_headerinfo) + buffer_index + 4u));

	pb_params.audio_data_begin = (ULONG64) (buffer_index + 8u);
	pb_params.audio_data_end = pb_params.audio_data_begin + ((ULONG64) u32);

	HeapFree(p_processheap, 0u, p_headerinfo);
	p_headerinfo = NULL;

	switch(bit_depth)
	{
		case 16u:
			return PB_I16;

		case 24u:
			return PB_I24;
	}

	tstr = TEXT("filein_get_params: Error: audio format not supported.");

_l_filein_get_params_error:
	filein_close();
	if(p_headerinfo != NULL) HeapFree(p_processheap, 0u, p_headerinfo);
	return -1;
}

BOOL WINAPI compare_signature(const CHAR *auth, const UINT8 *buf)
{
	ULONG_PTR nbyte;

	if(auth == NULL) return FALSE;
	if(buf == NULL) return FALSE;

	for(nbyte = 0u; nbyte < 4u; nbyte++) if(auth[nbyte] != ((CHAR) buf[nbyte])) return FALSE;

	return TRUE;
}

DWORD WINAPI audiothread_proc(VOID *p_args)
{
	p_audio->runPlayback();

	PostMessage(p_mainwnd, CUSTOM_WM_PLAYBACK_FINISHED, 0, 0);
	return 0u;
}
