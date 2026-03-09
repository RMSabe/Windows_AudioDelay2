/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.3.1 (Interop C# version 1.0)

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

struct _procctx {
	__declspec(align(PTR_SIZE_BYTES)) HANDLE h_filein;
	__declspec(align(PTR_SIZE_BYTES)) AudioPB *p_audio;
	__declspec(align(PTR_SIZE_BYTES)) audiopb_params_t pb_params;
	__declspec(align(PTR_SIZE_BYTES)) __string *p_errmsg;
	__declspec(align(PTR_SIZE_BYTES)) __string *p_fileindir;
};

typedef struct _procctx procctx_t;

extern "C" __declspec(dllexport) procctx_t* APIENTRY _procCtxAlloc(VOID);
extern "C" __declspec(dllexport) BOOL APIENTRY _procCtxFree(procctx_t *p_procctx);

extern "C" __declspec(dllexport) BOOL APIENTRY _coreInit(procctx_t *p_procctx);
extern "C" __declspec(dllexport) VOID APIENTRY _coreDeinit(procctx_t *p_procctx);
extern "C" __declspec(dllexport) VOID APIENTRY _coreReset(procctx_t *p_procctx);

extern "C" __declspec(dllexport) const TCHAR* APIENTRY _getLastErrorMessage(procctx_t *p_procctx);
extern "C" __declspec(dllexport) BOOL APIENTRY _setFileInDir(procctx_t *p_procctx, const TCHAR *fdir);

extern "C" __declspec(dllexport) BOOL APIENTRY _preparePlayback(procctx_t *p_procctx);
extern "C" __declspec(dllexport) BOOL APIENTRY _runPlayback(procctx_t *p_procctx);
extern "C" __declspec(dllexport) VOID APIENTRY _stopPlayback(procctx_t *p_procctx);

extern "C" __declspec(dllexport) BOOL APIENTRY _startAudioObject(procctx_t *p_procctx);

extern "C" __declspec(dllexport) BOOL APIENTRY _loadAudioDeviceList(procctx_t *p_procctx);
extern "C" __declspec(dllexport) LONG_PTR APIENTRY _getAudioDeviceListEntryCount(procctx_t *p_procctx);
extern "C" __declspec(dllexport) const TCHAR* APIENTRY _getAudioDeviceListEntry(procctx_t *p_procctx, ULONG_PTR n_entry);

extern "C" __declspec(dllexport) BOOL APIENTRY _chooseDevice(procctx_t *p_procctx, ULONG_PTR index);
extern "C" __declspec(dllexport) BOOL APIENTRY _chooseDefaultDevice(procctx_t *p_procctx);

extern "C" __declspec(dllexport) FLOAT APIENTRY _getDryInputAmplitude(procctx_t *p_procctx);
extern "C" __declspec(dllexport) BOOL APIENTRY _setDryInputAmplitude(procctx_t *p_procctx, FLOAT amp);

extern "C" __declspec(dllexport) FLOAT APIENTRY _getOutputAmplitude(procctx_t *p_procctx);
extern "C" __declspec(dllexport) BOOL APIENTRY _setOutputAmplitude(procctx_t *p_procctx, FLOAT amp);

extern "C" __declspec(dllexport) LONG_PTR APIENTRY _getFFDelay(procctx_t *p_procctx, ULONG_PTR nfx);
extern "C" __declspec(dllexport) BOOL APIENTRY _setFFDelay(procctx_t *p_procctx, ULONG_PTR nfx, ULONG_PTR delay);

extern "C" __declspec(dllexport) FLOAT APIENTRY _getFFAmplitude(procctx_t *p_procctx, ULONG_PTR nfx);
extern "C" __declspec(dllexport) BOOL APIENTRY _setFFAmplitude(procctx_t *p_procctx, ULONG_PTR nfx, FLOAT amp);

extern "C" __declspec(dllexport) LONG_PTR APIENTRY _getFBDelay(procctx_t *p_procctx, ULONG_PTR nfx);
extern "C" __declspec(dllexport) BOOL APIENTRY _setFBDelay(procctx_t *p_procctx, ULONG_PTR nfx, ULONG_PTR delay);

extern "C" __declspec(dllexport) FLOAT APIENTRY _getFBAmplitude(procctx_t *p_procctx, ULONG_PTR nfx);
extern "C" __declspec(dllexport) BOOL APIENTRY _setFBAmplitude(procctx_t *p_procctx, ULONG_PTR nfx, FLOAT amp);

extern "C" __declspec(dllexport) BOOL APIENTRY _resetFFParams(procctx_t *p_procctx);
extern "C" __declspec(dllexport) BOOL APIENTRY _resetFBParams(procctx_t *p_procctx);

extern BOOL WINAPI filein_open(procctx_t *p_procctx);
extern VOID WINAPI filein_close(procctx_t *p_procctx);

extern INT WINAPI filein_get_params(procctx_t *p_procctx);
extern BOOL WINAPI compare_signature(const CHAR *auth, const UINT8 *buf);

__declspec(dllexport) procctx_t* APIENTRY _procCtxAlloc(VOID)
{
	HANDLE p_processheap = NULL;
	procctx_t *p_procctx = NULL;

	p_processheap = GetProcessHeap();
	if(p_processheap == NULL) return NULL;

	p_procctx = (procctx_t*) HeapAlloc(p_processheap, 0u, sizeof(procctx_t));
	if(p_procctx == NULL) return NULL;

	p_procctx->p_errmsg = new __string(TEXT(""));
	p_procctx->p_fileindir = new __string(TEXT(""));

	if(p_procctx->p_errmsg == NULL)
	{
		_procCtxFree(p_procctx);
		return NULL;
	}

	if(p_procctx->p_fileindir == NULL)
	{
		_procCtxFree(p_procctx);
		return NULL;
	}

	p_procctx->h_filein = INVALID_HANDLE_VALUE;
	p_procctx->p_audio = NULL;

	return p_procctx;
}

__declspec(dllexport) BOOL APIENTRY _procCtxFree(procctx_t *p_procctx)
{
	HANDLE p_processheap = NULL;

	if(p_procctx == NULL) return FALSE;

	p_processheap = GetProcessHeap();
	if(p_processheap == NULL) return FALSE;

	if(p_procctx->p_errmsg != NULL)
	{
		delete p_procctx->p_errmsg;
		p_procctx->p_errmsg = NULL;
	}

	if(p_procctx->p_fileindir != NULL)
	{
		delete p_procctx->p_fileindir;
		p_procctx->p_fileindir = NULL;
	}

	HeapFree(p_processheap, 0u, p_procctx);
	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY _coreInit(procctx_t *p_procctx)
{
	HRESULT n_ret;

	if(p_procctx == NULL) return FALSE;

	n_ret = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if((n_ret != S_OK) && (n_ret != S_FALSE))
	{
		*(p_procctx->p_errmsg) = TEXT("Error: failed to initialize COMBASEAPI.");
		goto _l__coreInit_error;
	}

	return TRUE;

_l__coreInit_error:

	_coreDeinit(p_procctx);
	return FALSE;
}

__declspec(dllexport) VOID APIENTRY _coreDeinit(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return;

	_coreReset(p_procctx);
	CoUninitialize();
	return;
}

__declspec(dllexport) VOID APIENTRY _coreReset(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return;

	filein_close(p_procctx);

	if(p_procctx->p_audio != NULL)
	{
		delete p_procctx->p_audio;
		p_procctx->p_audio = NULL;
	}

	return;
}

__declspec(dllexport) const TCHAR* APIENTRY _getLastErrorMessage(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return NULL;

	return p_procctx->p_errmsg->c_str();
}

__declspec(dllexport) BOOL APIENTRY _setFileInDir(procctx_t *p_procctx, const TCHAR *fdir)
{
	if(p_procctx == NULL) return FALSE;

	if(fdir == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: invalid parameter.");
		return FALSE;
	}

	*(p_procctx->p_fileindir) = fdir;
	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY _preparePlayback(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return FALSE;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_procctx->p_audio->initialize())
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY _runPlayback(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return FALSE;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_procctx->p_audio->runPlayback())
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) VOID APIENTRY _stopPlayback(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return;

	if(p_procctx->p_audio != NULL) p_procctx->p_audio->stopPlayback();

	return;
}

__declspec(dllexport) BOOL APIENTRY _startAudioObject(procctx_t *p_procctx)
{
	INT n_ret;

	if(p_procctx == NULL) return FALSE;

	if(!filein_open(p_procctx))
	{
		*(p_procctx->p_errmsg) = TEXT("Error: failed to open input file.");
		goto _l__startAudioObject_error;
	}

	n_ret = filein_get_params(p_procctx);
	if(n_ret < 0) goto _l__startAudioObject_error;

	if(p_procctx->p_audio != NULL)
	{
		delete p_procctx->p_audio;
		p_procctx->p_audio = NULL;
	}

	p_procctx->pb_params.delay_buffer_size_frames = RTDELAY_BUFFER_SIZE_FRAMES;
	p_procctx->pb_params.n_ff_delays = RTDELAY_N_FFCH;
	p_procctx->pb_params.n_fb_delays = RTDELAY_N_FBCH;
	p_procctx->pb_params.file_dir = p_procctx->p_fileindir->c_str();

	switch(n_ret)
	{
		case PB_I16:
			p_procctx->p_audio = new AudioPB_i16(&(p_procctx->pb_params));
			break;

		case PB_I24:
			p_procctx->p_audio = new AudioPB_i24(&(p_procctx->pb_params));
			break;
	}

	if(p_procctx->p_audio != NULL) return TRUE;

	*(p_procctx->p_errmsg) = TEXT("Error: failed to create audio object instance.");

_l__startAudioObject_error:

	filein_close(p_procctx);
	return FALSE;
}

__declspec(dllexport) BOOL APIENTRY _loadAudioDeviceList(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return FALSE;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_procctx->p_audio->loadAudioDeviceList())
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) LONG_PTR APIENTRY _getAudioDeviceListEntryCount(procctx_t *p_procctx)
{
	LONG_PTR count;

	if(p_procctx == NULL) return -1;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return -1;
	}

	count = p_procctx->p_audio->getAudioDeviceListEntryCount();
	if(count < 0) *(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();

	return count;
}

__declspec(dllexport) const TCHAR* APIENTRY _getAudioDeviceListEntry(procctx_t *p_procctx, ULONG_PTR n_entry)
{
	const TCHAR *entry = NULL;

	if(p_procctx == NULL) return NULL;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return NULL;
	}

	entry = p_procctx->p_audio->getAudioDeviceListEntry(n_entry);
	if(entry == NULL) *(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();

	return entry;
}

__declspec(dllexport) BOOL APIENTRY _chooseDevice(procctx_t *p_procctx, ULONG_PTR index)
{
	if(p_procctx == NULL) return FALSE;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_procctx->p_audio->chooseDevice(index))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY _chooseDefaultDevice(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return FALSE;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_procctx->p_audio->chooseDefaultDevice())
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) FLOAT APIENTRY _getDryInputAmplitude(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return 0.0f;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return 0.0f;
	}

	return p_procctx->p_audio->rtdelayGetDryInputAmplitude();
}

__declspec(dllexport) BOOL APIENTRY _setDryInputAmplitude(procctx_t *p_procctx, FLOAT amp)
{
	if(p_procctx == NULL) return FALSE;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_procctx->p_audio->rtdelaySetDryInputAmplitude(amp))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) FLOAT APIENTRY _getOutputAmplitude(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return 0.0f;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return 0.0f;
	}

	return p_procctx->p_audio->rtdelayGetOutputAmplitude();
}

__declspec(dllexport) BOOL APIENTRY _setOutputAmplitude(procctx_t *p_procctx, FLOAT amp)
{
	if(p_procctx == NULL) return FALSE;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_procctx->p_audio->rtdelaySetOutputAmplitude(amp))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) LONG_PTR APIENTRY _getFFDelay(procctx_t *p_procctx, ULONG_PTR nfx)
{
	audiortdelay_fx_params_t fx_params;

	if(p_procctx == NULL) return -1;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return -1;
	}

	if(!p_procctx->p_audio->rtdelayGetFFParams(nfx, &fx_params))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return -1;
	}

	return (LONG_PTR) fx_params.delay;
}

__declspec(dllexport) BOOL APIENTRY _setFFDelay(procctx_t *p_procctx, ULONG_PTR nfx, ULONG_PTR delay)
{
	if(p_procctx == NULL) return FALSE;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_procctx->p_audio->rtdelaySetFFDelay(nfx, delay))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) FLOAT APIENTRY _getFFAmplitude(procctx_t *p_procctx, ULONG_PTR nfx)
{
	audiortdelay_fx_params_t fx_params;

	if(p_procctx == NULL) return 0.0f;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return 0.0f;
	}

	if(!p_procctx->p_audio->rtdelayGetFFParams(nfx, &fx_params))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return 0.0f;
	}

	return fx_params.amp;
}

__declspec(dllexport) BOOL APIENTRY _setFFAmplitude(procctx_t *p_procctx, ULONG_PTR nfx, FLOAT amp)
{
	if(p_procctx == NULL) return FALSE;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_procctx->p_audio->rtdelaySetFFAmplitude(nfx, amp))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) LONG_PTR APIENTRY _getFBDelay(procctx_t *p_procctx, ULONG_PTR nfx)
{
	audiortdelay_fx_params_t fx_params;

	if(p_procctx == NULL) return -1;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return -1;
	}

	if(!p_procctx->p_audio->rtdelayGetFBParams(nfx, &fx_params))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return -1;
	}

	return (LONG_PTR) fx_params.delay;
}

__declspec(dllexport) BOOL APIENTRY _setFBDelay(procctx_t *p_procctx, ULONG_PTR nfx, ULONG_PTR delay)
{
	if(p_procctx == NULL) return FALSE;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_procctx->p_audio->rtdelaySetFBDelay(nfx, delay))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) FLOAT APIENTRY _getFBAmplitude(procctx_t *p_procctx, ULONG_PTR nfx)
{
	audiortdelay_fx_params_t fx_params;

	if(p_procctx == NULL) return 0.0f;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return 0.0f;
	}

	if(!p_procctx->p_audio->rtdelayGetFBParams(nfx, &fx_params))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return 0.0f;
	}

	return fx_params.amp;
}

__declspec(dllexport) BOOL APIENTRY _setFBAmplitude(procctx_t *p_procctx, ULONG_PTR nfx, FLOAT amp)
{
	if(p_procctx == NULL) return FALSE;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_procctx->p_audio->rtdelaySetFBAmplitude(nfx, amp))
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY _resetFFParams(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return FALSE;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_procctx->p_audio->rtdelayResetFFParams())
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

__declspec(dllexport) BOOL APIENTRY _resetFBParams(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return FALSE;

	if(p_procctx->p_audio == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio object is not ready.");
		return FALSE;
	}

	if(!p_procctx->p_audio->rtdelayResetFBParams())
	{
		*(p_procctx->p_errmsg) = p_procctx->p_audio->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI filein_open(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return FALSE;

	filein_close(p_procctx);

	p_procctx->h_filein = CreateFile(p_procctx->p_fileindir->c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);

	return (p_procctx->h_filein != INVALID_HANDLE_VALUE);
}

VOID WINAPI filein_close(procctx_t *p_procctx)
{
	if(p_procctx == NULL) return;

	if(p_procctx->h_filein == INVALID_HANDLE_VALUE) return;

	CloseHandle(p_procctx->h_filein);
	p_procctx->h_filein = INVALID_HANDLE_VALUE;
	return;
}

INT WINAPI filein_get_params(procctx_t *p_procctx)
{
	const ULONG_PTR BUFFER_SIZE = 8192u;
	ULONG_PTR buffer_index;

	HANDLE p_processheap = NULL;
	UINT8 *p_headerinfo = NULL;

	DWORD dummy_32;
	UINT32 u32;
	UINT16 u16;

	UINT16 bit_depth;

	if(p_procctx == NULL) return -1;

	p_processheap = GetProcessHeap();
	if(p_processheap == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: failed to retrieve process heap.");
		return -1;
	}

	p_headerinfo = (UINT8*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, BUFFER_SIZE);
	if(p_headerinfo == NULL)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: failed to allocate heap memory.");
		goto _l_filein_get_params_error;
	}

	SetFilePointer(p_procctx->h_filein, 0, NULL, FILE_BEGIN);
	ReadFile(p_procctx->h_filein, p_headerinfo, (DWORD) BUFFER_SIZE, &dummy_32, NULL);
	filein_close(p_procctx);

	if(!compare_signature("RIFF", p_headerinfo))
	{
		*(p_procctx->p_errmsg) = TEXT("Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	if(!compare_signature("WAVE", (const UINT8*) (((ULONG_PTR) p_headerinfo) + 8u)))
	{
		*(p_procctx->p_errmsg) = TEXT("Error: file format not supported.");
		goto _l_filein_get_params_error;
	}

	buffer_index = 12u;

	while(TRUE)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			*(p_procctx->p_errmsg) = TEXT("Error: broken header.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("fmt ", (const UINT8*) (((ULONG_PTR) p_headerinfo) + buffer_index))) break;

		u32 = *((UINT32*) (((ULONG_PTR) p_headerinfo) + buffer_index + 4u));
		buffer_index += (ULONG_PTR) (u32 + 8u);
	}

	if(buffer_index > (BUFFER_SIZE - 24u))
	{
		*(p_procctx->p_errmsg) = TEXT("Error: broken header.");
		goto _l_filein_get_params_error;
	}

	u16 = *((UINT16*) (((ULONG_PTR) p_headerinfo) + buffer_index + 8u));
	if(u16 != 1u)
	{
		*(p_procctx->p_errmsg) = TEXT("Error: audio encoding format not supported.");
		goto _l_filein_get_params_error;
	}

	p_procctx->pb_params.n_channels = *((UINT16*) (((ULONG_PTR) p_headerinfo) + buffer_index + 10u));
	p_procctx->pb_params.sample_rate = *((UINT32*) (((ULONG_PTR) p_headerinfo) + buffer_index + 12u));
	bit_depth = *((UINT16*) (((ULONG_PTR) p_headerinfo) + buffer_index + 22u));

	u32 = *((UINT32*) (((ULONG_PTR) p_headerinfo) + buffer_index + 4u));
	buffer_index += (ULONG_PTR) (u32 + 8u);

	while(TRUE)
	{
		if(buffer_index > (BUFFER_SIZE - 8u))
		{
			*(p_procctx->p_errmsg) = TEXT("Error: broken header.");
			goto _l_filein_get_params_error;
		}

		if(compare_signature("data", (const UINT8*) (((ULONG_PTR) p_headerinfo) + buffer_index))) break;

		u32 = *((UINT32*) (((ULONG_PTR) p_headerinfo) + buffer_index + 4u));
		buffer_index += (ULONG_PTR) (u32 + 8u);
	}

	u32 = *((UINT32*) (((ULONG_PTR) p_headerinfo) + buffer_index + 4u));

	p_procctx->pb_params.audio_data_begin = (ULONG64) (buffer_index + 8u);
	p_procctx->pb_params.audio_data_end = p_procctx->pb_params.audio_data_begin + ((ULONG64) u32);

	HeapFree(p_processheap, 0u, p_headerinfo);
	p_headerinfo = NULL;

	switch(bit_depth)
	{
		case 16u:
			return PB_I16;

		case 24u:
			return PB_I24;
	}

	*(p_procctx->p_errmsg) = TEXT("Error: audio format not supported.");

_l_filein_get_params_error:

	filein_close(p_procctx);
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
	if(exit_msg != NULL) MessageBox(NULL, exit_msg, TEXT("PROCESS EXIT CALLED"), (MB_ICONSTOP | MB_OK));

	ExitProcess(exit_code);

	while(TRUE) Sleep(16u);
}
