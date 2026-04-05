/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.4 (Interop C# version 1.0)

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "globldef.h"
#include "cstrdef.h"
#include "strdef.hpp"
#include "shared.hpp"

#include <combaseapi.h>

#include "AudioRTDelay.hpp"
#include "AudioPB.hpp"
#include "AudioPB_i16.hpp"
#include "AudioPB_i24.hpp"

#define RTDELAY_BUFFER_SIZE_FRAMES 65536U
#define RTDELAY_N_FFCH 4U
#define RTDELAY_N_FBCH 4U

#define PB_I16 1
#define PB_I24 2

__declspec(align(PTR_SIZE_BYTES)) AudioPB *p_audio = NULL;
__declspec(align(PTR_SIZE_BYTES)) audiopb_params_t pb_params;
__declspec(align(PTR_SIZE_BYTES)) __string err_msg = TEXT("");
__declspec(align(PTR_SIZE_BYTES)) __string filein_dir = TEXT("");
__declspec(align(PTR_SIZE_BYTES)) HANDLE h_filein = INVALID_HANDLE_VALUE;

extern "C" __declspec(dllexport) BOOL APIENTRY Initialize(VOID);
extern "C" __declspec(dllexport) VOID APIENTRY Deinitialize(VOID);
extern "C" __declspec(dllexport) VOID APIENTRY Reset(VOID);
extern "C" __declspec(dllexport) const TCHAR* APIENTRY GetLastErrorMessage(VOID);
extern "C" __declspec(dllexport) BOOL APIENTRY SetFileInDirectory(const TCHAR *fdir);
extern "C" __declspec(dllexport) BOOL APIENTRY LoadFile_CreateAudioObject(VOID);
extern "C" __declspec(dllexport) BOOL APIENTRY LoadAudioDeviceList(VOID);
extern "C" __declspec(dllexport) LONG_PTR APIENTRY GetAudioDeviceListEntryCount(VOID);
extern "C" __declspec(dllexport) const TCHAR* APIENTRY GetAudioDeviceListEntry(ULONG_PTR n_entry);
extern "C" __declspec(dllexport) BOOL APIENTRY ChooseDevice(ULONG_PTR index);
extern "C" __declspec(dllexport) BOOL APIENTRY ChooseDefaultDevice(VOID);
extern "C" __declspec(dllexport) BOOL APIENTRY InitializeAudioObject(VOID);
extern "C" __declspec(dllexport) BOOL APIENTRY RunPlayback(VOID);
extern "C" __declspec(dllexport) VOID APIENTRY StopPlayback(VOID);
extern "C" __declspec(dllexport) FLOAT APIENTRY GetDryInputAmplitude(VOID);
extern "C" __declspec(dllexport) BOOL APIENTRY SetDryInputAmplitude(FLOAT amp);
extern "C" __declspec(dllexport) FLOAT APIENTRY GetOutputAmplitude(VOID);
extern "C" __declspec(dllexport) BOOL APIENTRY SetOutputAmplitude(FLOAT amp);
extern "C" __declspec(dllexport) LONG_PTR APIENTRY GetFFDelay(ULONG_PTR nfx);
extern "C" __declspec(dllexport) BOOL APIENTRY SetFFDelay(ULONG_PTR nfx, ULONG_PTR delay);
extern "C" __declspec(dllexport) FLOAT APIENTRY GetFFAmplitude(ULONG_PTR nfx);
extern "C" __declspec(dllexport) BOOL APIENTRY SetFFAmplitude(ULONG_PTR nfx, FLOAT amp);
extern "C" __declspec(dllexport) LONG_PTR APIENTRY GetFBDelay(ULONG_PTR nfx);
extern "C" __declspec(dllexport) BOOL APIENTRY SetFBDelay(ULONG_PTR nfx, ULONG_PTR delay);
extern "C" __declspec(dllexport) FLOAT APIENTRY GetFBAmplitude(ULONG_PTR nfx);
extern "C" __declspec(dllexport) BOOL APIENTRY SetFBAmplitude(ULONG_PTR nfx, FLOAT amp);
extern "C" __declspec(dllexport) BOOL APIENTRY ResetFFParams(VOID);
extern "C" __declspec(dllexport) BOOL APIENTRY ResetFBParams(VOID);

extern BOOL WINAPI filein_open(VOID);
extern VOID WINAPI filein_close(VOID);

extern INT WINAPI filein_get_params(VOID);
extern BOOL WINAPI compare_signature(const CHAR *auth, const UINT8 *buf);

__declspec(dllexport) BOOL APIENTRY Initialize(VOID)
{
	HRESULT n_ret;

	p_processheap = GetProcessHeap();
	if(p_processheap == NULL)
	{
		err_msg = TEXT("Error: failed to retrieve process heap.");
		goto _l_Initialize_error;
	}

	n_ret = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if((n_ret != S_OK) && (n_ret != S_FALSE))
	{
		err_msg = TEXT("Error: failed to initialize COMBASEAPI.");
		goto _l_Initialize_error;
	}

	return TRUE;

_l_Initialize_error:

	Deinitialize();
	return FALSE;
}

__declspec(dllexport) VOID APIENTRY Deinitialize(VOID)
{
	Reset();
	CoUninitialize();
	return;
}

__declspec(dllexport) VOID APIENTRY Reset(VOID)
{
	filein_close();

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	return;
}

__declspec(dllexport) const TCHAR* APIENTRY GetLastErrorMessage(VOID)
{
	return err_msg.c_str();
}

__declspec(dllexport) BOOL APIENTRY SetFileInDirectory(const TCHAR *fdir)
{
	if(fdir == NULL)
	{
		err_msg = TEXT("Error: invalid parameter.");
		return FALSE;
	}

	filein_dir = fdir;
	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY LoadFile_CreateAudioObject(VOID)
{
	INT n_ret;

	if(!filein_open())
	{
		err_msg = TEXT("Error: failed to open input file.");
		goto _l_LoadFile_CreateAudioObject_error;
	}

	n_ret = filein_get_params();
	if(n_ret < 0) goto _l_LoadFile_CreateAudioObject_error;

	if(p_audio != NULL)
	{
		delete p_audio;
		p_audio = NULL;
	}

	pb_params.delay_buffer_size_frames = RTDELAY_BUFFER_SIZE_FRAMES;
	pb_params.n_ff_delays = RTDELAY_N_FFCH;
	pb_params.n_fb_delays = RTDELAY_N_FBCH;
	pb_params.file_dir = filein_dir.c_str();

	switch(n_ret)
	{
		case PB_I16:
			p_audio = new AudioPB_i16(&pb_params);
			break;

		case PB_I24:
			p_audio = new AudioPB_i24(&pb_params);
			break;
	}

	if(p_audio != NULL) return TRUE;

	err_msg = TEXT("Error: failed to create audio object instance.");

_l_LoadFile_CreateAudioObject_error:

	filein_close();
	return FALSE;
}

__declspec(dllexport) BOOL APIENTRY LoadAudioDeviceList(VOID)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->loadAudioDeviceList())
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) LONG_PTR APIENTRY GetAudioDeviceListEntryCount(VOID)
{
	LONG_PTR count;

	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return -1;
	}

	count = p_audio->getAudioDeviceListEntryCount();
	if(count < 0) err_msg = p_audio->getLastErrorMessage();

	return count;
}

__declspec(dllexport) const TCHAR* APIENTRY GetAudioDeviceListEntry(ULONG_PTR n_entry)
{
	const TCHAR *entry = NULL;

	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return NULL;
	}

	entry = p_audio->getAudioDeviceListEntry(n_entry);
	if(entry == NULL) err_msg = p_audio->getLastErrorMessage();

	return entry;
}

__declspec(dllexport) BOOL APIENTRY ChooseDevice(ULONG_PTR index)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->chooseDevice(index))
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY ChooseDefaultDevice(VOID)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->chooseDefaultDevice())
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY InitializeAudioObject(VOID)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->initialize())
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY RunPlayback(VOID)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->runPlayback())
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) VOID APIENTRY StopPlayback(VOID)
{
	if(p_audio != NULL) p_audio->stopPlayback();
	return;
}

__declspec(dllexport) FLOAT APIENTRY GetDryInputAmplitude(VOID)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return 0.0f;
	}

	return p_audio->rtdelayGetDryInputAmplitude();
}

__declspec(dllexport) BOOL APIENTRY SetDryInputAmplitude(FLOAT amp)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->rtdelaySetDryInputAmplitude(amp))
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) FLOAT APIENTRY GetOutputAmplitude(VOID)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return 0.0f;
	}

	return p_audio->rtdelayGetOutputAmplitude();
}

__declspec(dllexport) BOOL APIENTRY SetOutputAmplitude(FLOAT amp)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->rtdelaySetOutputAmplitude(amp))
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) LONG_PTR APIENTRY GetFFDelay(ULONG_PTR nfx)
{
	audiortdelay_fx_params_t fx_params;

	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return -1;
	}

	if(!p_audio->rtdelayGetFFParams(nfx, &fx_params))
	{
		err_msg = p_audio->getLastErrorMessage();
		return -1;
	}

	return (LONG_PTR) fx_params.delay;
}

__declspec(dllexport) BOOL APIENTRY SetFFDelay(ULONG_PTR nfx, ULONG_PTR delay)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->rtdelaySetFFDelay(nfx, delay))
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) FLOAT APIENTRY GetFFAmplitude(ULONG_PTR nfx)
{
	audiortdelay_fx_params_t fx_params;

	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return 0.0f;
	}

	if(!p_audio->rtdelayGetFFParams(nfx, &fx_params))
	{
		err_msg = p_audio->getLastErrorMessage();
		return 0.0f;
	}

	return fx_params.amp;
}

__declspec(dllexport) BOOL APIENTRY SetFFAmplitude(ULONG_PTR nfx, FLOAT amp)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->rtdelaySetFFAmplitude(nfx, amp))
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) LONG_PTR APIENTRY GetFBDelay(ULONG_PTR nfx)
{
	audiortdelay_fx_params_t fx_params;

	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return -1;
	}

	if(!p_audio->rtdelayGetFBParams(nfx, &fx_params))
	{
		err_msg = p_audio->getLastErrorMessage();
		return -1;
	}

	return (LONG_PTR) fx_params.delay;
}

__declspec(dllexport) BOOL APIENTRY SetFBDelay(ULONG_PTR nfx, ULONG_PTR delay)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->rtdelaySetFBDelay(nfx, delay))
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) FLOAT APIENTRY GetFBAmplitude(ULONG_PTR nfx)
{
	audiortdelay_fx_params_t fx_params;

	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return 0.0f;
	}

	if(!p_audio->rtdelayGetFBParams(nfx, &fx_params))
	{
		err_msg = p_audio->getLastErrorMessage();
		return 0.0f;
	}

	return fx_params.amp;
}

__declspec(dllexport) BOOL APIENTRY SetFBAmplitude(ULONG_PTR nfx, FLOAT amp)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->rtdelaySetFBAmplitude(nfx, amp))
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY ResetFFParams(VOID)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->rtdelayResetFFParams())
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY ResetFBParams(VOID)
{
	if(p_audio == NULL)
	{
		err_msg = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_audio->rtdelayResetFBParams())
	{
		err_msg = p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI filein_open(VOID)
{
	filein_close();

	h_filein = CreateFile(filein_dir.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);

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
	ULONG_PTR buffer_index;

	UINT8 *p_headerinfo = NULL;

	DWORD dummy_32;
	UINT32 u32;
	UINT16 u16;

	UINT16 bit_depth;

	if(p_processheap == NULL)
	{
		err_msg = TEXT("Error: p_processheap is NULL.");
		goto _l_filein_get_params_error;
	}

	p_headerinfo = (UINT8*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, BUFFER_SIZE);
	if(p_headerinfo == NULL)
	{
		err_msg = TEXT("Error: failed to allocate heap memory.");
		goto _l_filein_get_params_error;
	}

	SetFilePointer(h_filein, 0, NULL, FILE_BEGIN);
	ReadFile(h_filein, p_headerinfo, (DWORD) BUFFER_SIZE, &dummy_32, NULL);
	filein_close();

	if(!compare_signature("RIFF", p_headerinfo))
	{
		err_msg = TEXT("Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	if(!compare_signature("WAVE", (const UINT8*) (((ULONG_PTR) p_headerinfo) + 8u)))
	{
		err_msg = TEXT("Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	buffer_index = 12u;

	while(TRUE)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			err_msg = TEXT("Error: broken file header.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("fmt ", (const UINT8*) (((ULONG_PTR) p_headerinfo) + buffer_index))) break;

		u32 = *((UINT32*) (((ULONG_PTR) p_headerinfo) + buffer_index + 4u));
		buffer_index += (ULONG_PTR) (u32 + 8u);
	}

	if(buffer_index > (BUFFER_SIZE - 24u))
	{
		err_msg = TEXT("Error: broken file header.");
		goto _l_filein_get_params_error;
	}

	u16 = *((UINT16*) (((ULONG_PTR) p_headerinfo) + buffer_index + 8u));
	if(u16 != 1u)
	{
		err_msg = TEXT("Error: audio encoding format not supported.");
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
			err_msg = TEXT("Error: broken file header.");
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

	err_msg = TEXT("Error: audio format not supported.");

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

__declspec(noreturn) VOID WINAPI app_exit(UINT exit_code, const TCHAR *exit_msg)
{
	Deinitialize();

	if(exit_msg != NULL) MessageBox(NULL, exit_msg, TEXT("PROCESS EXIT CALLED"), (MB_ICONSTOP | MB_OK));

	ExitProcess(exit_code);

	while(TRUE) Sleep(16u);
}
