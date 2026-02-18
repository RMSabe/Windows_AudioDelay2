/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioPB.hpp"
#include "cstrdef.h"

#include <combaseapi.h>

AudioPB::AudioPB(const audiopb_params_t *p_params)
{
	this->setParameters(p_params);
}

BOOL WINAPI AudioPB::setParameters(const audiopb_params_t *p_params)
{
	if(p_params == NULL) return FALSE;
	if(p_params->file_dir == NULL) return FALSE;

	if(this->status > 0) return FALSE;

	this->status = this->STATUS_UNINITIALIZED;

	this->RTDELAY_BUFFER_SIZE_FRAMES = p_params->delay_buffer_size_frames;
	this->RTDELAY_FF_PARAMS_LENGTH = p_params->n_ff_delays;
	this->RTDELAY_FB_PARAMS_LENGTH = p_params->n_fb_delays;
	this->FILEIN_DIR = p_params->file_dir;
	this->AUDIO_DATA_BEGIN = p_params->audio_data_begin;
	this->AUDIO_DATA_END = p_params->audio_data_end;
	this->SAMPLE_RATE = p_params->sample_rate;
	this->N_CHANNELS = p_params->n_channels;

	return TRUE;
}

BOOL WINAPI AudioPB::initialize(VOID)
{
	audiortdelay_init_params_t delay_params;

	if(this->status > 0) return TRUE;

	this->status = this->STATUS_UNINITIALIZED;

	if(!this->filein_open())
	{
		this->status = this->STATUS_ERROR_NOFILE;
		this->err_msg = TEXT("AudioPB::initialize: Error: open input file failed.");
		return FALSE;
	}

	if(!this->audio_hw_init())
	{
		this->status = this->STATUS_ERROR_AUDIOHW;
		this->filein_close();
		return FALSE;
	}

	if(!this->buffer_alloc())
	{
		this->status = this->STATUS_ERROR_MEMALLOC;
		this->err_msg = TEXT("AudioPB::initialize: Error: memory allocate failed.");
		this->filein_close();
		this->audio_hw_deinit_all();
		return FALSE;
	}

	delay_params.buffer_size_frames = this->RTDELAY_BUFFER_SIZE_FRAMES;
	delay_params.buffer_n_segments = this->RTDELAY_BUFFER_N_SEGMENTS;
	delay_params.n_channels = (ULONG_PTR) this->N_CHANNELS;
	delay_params.n_ff_delays = this->RTDELAY_FF_PARAMS_LENGTH;
	delay_params.n_fb_delays = this->RTDELAY_FB_PARAMS_LENGTH;

	if(this->p_delay == NULL)
	{
		this->p_delay = new AudioRTDelay(&delay_params);
		if(this->p_delay == NULL)
		{
			this->status = this->STATUS_ERROR_MEMALLOC;
			this->err_msg = TEXT("AudioPB::initialize: Error: AudioRTDelay object instance failed.");
			this->filein_close();
			this->audio_hw_deinit_all();
			this->buffer_free();
			return FALSE;
		}
	}
	else this->p_delay->setInitParameters(&delay_params);

	if(!this->p_delay->initialize())
	{
		this->status = this->STATUS_ERROR_GENERIC;
		this->err_msg = this->p_delay->getLastErrorMessage();
		this->filein_close();
		this->audio_hw_deinit_all();
		this->buffer_free();
		return FALSE;
	}

	this->status = this->STATUS_READY;
	return TRUE;
}

BOOL WINAPI AudioPB::runPlayback(VOID)
{
	if(this->status != this->STATUS_READY)
	{
		this->err_msg = TEXT("AudioPB::runPlayback: Error: object is either not initialized or already running playback.");
		return FALSE;
	}

	this->status = this->STATUS_PLAYING;
	this->playback_proc();

	this->filein_close();
	this->audio_hw_deinit_device();
	this->buffer_free();

	this->status = this->STATUS_UNINITIALIZED;
	return TRUE;
}

VOID WINAPI AudioPB::stopPlayback(VOID)
{
	this->stop_playback = TRUE;
	return;
}

BOOL WINAPI AudioPB::loadAudioDeviceList(HWND p_listbox)
{
	const PROPERTYKEY* const P_PKEY = (const PROPERTYKEY*) P_PKEY_Device_FriendlyName;
	IMMDevice *p_dev = NULL;
	IPropertyStore *p_devprop = NULL;
	HRESULT n_ret = 0;
	UINT devcoll_count = 0u;
	UINT n_dev = 0u;
	PROPVARIANT propvar;

	if(this->status > 0)
	{
		this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: cannot run method, audio object already initialized.");
		return FALSE;
	}

	/*If p_listbox is NULL, all listbox related functions will fail, but the device list initialization shall continue normally.*/

	listbox_clear(p_listbox);

	this->audio_hw_deinit_all();

	n_ret = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (VOID**) &(this->p_audiodevenum));
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: CoCreateInstance (IMMDeviceEnumerator) failed.");
		return FALSE;
	}

	n_ret = this->p_audiodevenum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &(this->p_audiodevcoll));
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_all();
		this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: IMMDeviceEnumerator::EnumAudioEndpoints failed.");
		return FALSE;
	}

	n_ret = this->p_audiodevcoll->GetCount(&devcoll_count);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_all();
		this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: IMMDeviceCollection::GetCount failed.");
		return FALSE;
	}

	for(n_dev = 0u; n_dev < devcoll_count; n_dev++)
	{
		n_ret = this->p_audiodevcoll->Item(n_dev, &p_dev);
		if(n_ret != S_OK)
		{
			this->audio_hw_deinit_all();
			this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: IMMDeviceCollection::Item failed.");
			return FALSE;
		}

		n_ret = p_dev->OpenPropertyStore(STGM_READ, &p_devprop);
		if(n_ret != S_OK)
		{
			p_dev->Release();
			this->audio_hw_deinit_all();
			this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: IMMDevice::OpenPropertyStore failed.");
			return FALSE;
		}

		PropVariantInit(&propvar);

		n_ret = p_devprop->GetValue(*P_PKEY, &propvar);
		if(n_ret != S_OK)
		{
			p_devprop->Release();
			p_dev->Release();
			this->audio_hw_deinit_all();
			this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: IPropertyStore::GetValue failed.");
			return FALSE;
		}

		if(propvar.vt == VT_EMPTY) __SPRINTF(textbuf, TEXTBUF_SIZE_CHARS, TEXT("Unknown Audio Device"));
		else cstr_copy_wchar_to_tchar(propvar.pwszVal, textbuf, TEXTBUF_SIZE_CHARS);

		listbox_add_item(p_listbox, textbuf);

		PropVariantClear(&propvar);

		p_devprop->Release();
		p_devprop = NULL;

		p_dev->Release();
		p_dev = NULL;
	}

	PropVariantClear(&propvar);

	if(p_devprop != NULL) p_devprop->Release();
	if(p_dev != NULL) p_dev->Release();

	return TRUE;
}

BOOL WINAPI AudioPB::chooseDevice(ULONG_PTR index)
{
	HRESULT n_ret = 0;

	if(this->status > 0)
	{
		this->err_msg = TEXT("AudioPB::chooseDevice: Error: cannot run method, audio object is already initialized.");
		return FALSE;
	}

	if(this->p_audiodevcoll == NULL)
	{
		this->err_msg = TEXT("AudioPB::chooseDevice: Error: p_audiodevcoll is NULL.");
		return FALSE;
	}

	this->audio_hw_deinit_device();

	n_ret = this->p_audiodevcoll->Item((UINT) index, &(this->p_audiodev));
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("AudioPB::chooseDevice: Error: IMMDeviceCollection::Item failed.");
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::chooseDefaultDevice(VOID)
{
	HRESULT n_ret = 0;

	if(this->status > 0)
	{
		this->err_msg = TEXT("AudioPB::chooseDefaultDevice: Error: cannot run method, audio object is already initialized.");
		return FALSE;
	}

	if(this->p_audiodevenum == NULL)
	{
		this->err_msg = TEXT("AudioPB::chooseDefaultDevice: Error: p_audiodevenum is NULL.");
		return FALSE;
	}

	this->audio_hw_deinit_device();

	n_ret = this->p_audiodevenum->GetDefaultAudioEndpoint(eRender, eMultimedia, &(this->p_audiodev));
	if(n_ret != S_OK)
	{
		this->err_msg = TEXT("AudioPB::chooseDefaultDevice: Error: IMMDeviceEnumerator::GetDefaultAudioEndpoint failed.");
		return FALSE;
	}

	return TRUE;
}

__string WINAPI AudioPB::getLastErrorMessage(VOID)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return (TEXT("Error: audio object not initialized\r\nExtended error message: ") + this->err_msg);

	return this->err_msg;
}

FLOAT WINAPI AudioPB::rtdelayGetDryInputAmplitude(VOID)
{
	if(this->status < 1) return 0.0f;

	return this->p_delay->getDryInputAmplitude();
}

FLOAT WINAPI AudioPB::rtdelayGetOutputAmplitude(VOID)
{
	if(this->status < 1) return 0.0f;

	return this->p_delay->getOutputAmplitude();
}

BOOL WINAPI AudioPB::rtdelayGetFFParams(ULONG_PTR n_fx, audiortdelay_fx_params_t *p_params)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->getFFParams(n_fx, p_params))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::rtdelayGetFBParams(ULONG_PTR n_fx, audiortdelay_fx_params_t *p_params)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->getFBParams(n_fx, p_params))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::rtdelaySetDryInputAmplitude(FLOAT amp)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->setDryInputAmplitude(amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::rtdelaySetOutputAmplitude(FLOAT amp)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->setOutputAmplitude(amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::rtdelaySetFFDelay(ULONG_PTR n_fx, ULONG_PTR delay)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->setFFDelay(n_fx, delay))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::rtdelaySetFFAmplitude(ULONG_PTR n_fx, FLOAT amp)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->setFFAmplitude(n_fx, amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::rtdelaySetFBDelay(ULONG_PTR n_fx, ULONG_PTR delay)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->setFBDelay(n_fx, delay))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::rtdelaySetFBAmplitude(ULONG_PTR n_fx, FLOAT amp)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->setFBAmplitude(n_fx, amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::rtdelayResetFFParams(VOID)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->resetFFParams())
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::rtdelayResetFBParams(VOID)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->resetFBParams())
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::filein_open(VOID)
{
	this->filein_close();

	this->h_filein = CreateFile(this->FILEIN_DIR.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0u, NULL);
	if(this->h_filein == INVALID_HANDLE_VALUE) return FALSE;

	this->filein_size_64.l32 = (ULONG32) GetFileSize(this->h_filein, (DWORD*) &(this->filein_size_64.h32));
	return TRUE;
}

VOID WINAPI AudioPB::filein_close(VOID)
{
	if(this->h_filein == INVALID_HANDLE_VALUE) return;

	CloseHandle(this->h_filein);
	this->h_filein = INVALID_HANDLE_VALUE;
	*((ULONG64*) &(this->filein_size_64)) = 0u;

	return;
}

VOID WINAPI AudioPB::audio_hw_deinit_device(VOID)
{
	if(this->p_audiomgr != NULL) this->p_audiomgr->Stop();

	if(this->p_audioout != NULL)
	{
		this->p_audioout->Release();
		this->p_audioout = NULL;
	}

	if(this->p_audiomgr != NULL)
	{
		this->p_audiomgr->Release();
		this->p_audiomgr = NULL;
	}

	if(this->p_audiodev != NULL)
	{
		this->p_audiodev->Release();
		this->p_audiodev = NULL;
	}

	return;
}

VOID WINAPI AudioPB::audio_hw_deinit_all(VOID)
{
	this->audio_hw_deinit_device();

	if(this->p_audiodevcoll != NULL)
	{
		this->p_audiodevcoll->Release();
		this->p_audiodevcoll = NULL;
	}

	if(this->p_audiodevenum != NULL)
	{
		this->p_audiodevenum->Release();
		this->p_audiodevenum = NULL;
	}

	return;
}

VOID WINAPI AudioPB::playback_proc(VOID)
{
	this->playback_init();
	this->playback_loop();

	return;
}

VOID WINAPI AudioPB::playback_init(VOID)
{
	HRESULT n_ret = 0;

	*((ULONG64*) &(this->filein_pos_64)) = this->AUDIO_DATA_BEGIN;

	this->p_delay->setDryInputAmplitude(1.0f);
	this->p_delay->setOutputAmplitude(1.0f);

	this->p_delay->resetFFParams();
	this->p_delay->resetFBParams();

	this->rtdelay_nseg = 0u;

	this->stop_playback = FALSE;

	n_ret = this->p_audioout->GetBuffer((UINT32) this->AUDIOBUFFER_SIZE_FRAMES, (BYTE**) &(this->p_audiobuffer));
	if(n_ret != S_OK) app_exit(0xffffffff, TEXT("AudioPB::playback_init: Error: IAudioRenderClient::GetBuffer failed."));

	ZeroMemory(this->p_audiobuffer, this->AUDIOBUFFER_SIZE_BYTES);

	n_ret = this->p_audioout->ReleaseBuffer((UINT32) this->AUDIOBUFFER_SIZE_FRAMES, 0u);
	if(n_ret != S_OK) app_exit(0xffffffff, TEXT("AudioPB::playback_init: Error: IAudioRenderClient::ReleaseBuffer failed."));

	n_ret = this->p_audiomgr->Start();
	if(n_ret != S_OK) app_exit(0xffffffff, TEXT("AudioPB::playback_init: Error: IAudioClient::Start failed."));

	this->bufferout_remap();

	this->buffer_load_in();
	this->p_delay->runDSP(this->rtdelay_nseg);
	this->buffer_load_out();
	this->rtdelay_nseg_update();
	this->bufferout_remap();

	this->audio_hw_wait();

	return;
}

VOID WINAPI AudioPB::playback_loop(VOID)
{
	while(!this->stop_playback)
	{
		this->buffer_play();

		this->buffer_load_in();
		this->p_delay->runDSP(this->rtdelay_nseg);
		this->buffer_load_out();

		this->audio_hw_wait();

		this->rtdelay_nseg_update();
		this->bufferout_remap();
	}

	return;
}

VOID WINAPI AudioPB::rtdelay_nseg_update(VOID)
{
	this->rtdelay_nseg++;
	this->rtdelay_nseg %= this->RTDELAY_BUFFER_N_SEGMENTS;

	return;
}

VOID WINAPI AudioPB::bufferout_remap(VOID)
{
	if(this->bufferout_cycle)
	{
		this->p_loadout = this->p_bufferoutput1;
		this->p_playout = this->p_bufferoutput0;
	}
	else
	{
		this->p_loadout = this->p_bufferoutput0;
		this->p_playout = this->p_bufferoutput1;
	}

	this->bufferout_cycle = !this->bufferout_cycle;
	return;
}

VOID WINAPI AudioPB::buffer_play(VOID)
{
	HRESULT n_ret = 0;

	n_ret = this->p_audioout->GetBuffer((UINT32) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, (BYTE**) &(this->p_audiobuffer));
	if(n_ret != S_OK) app_exit(0xffffffff, TEXT("AudioPB::buffer_play: Error: IAudioRenderClient::GetBuffer failed."));

	CopyMemory(this->p_audiobuffer, this->p_playout, this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);

	n_ret = this->p_audioout->ReleaseBuffer((UINT32) this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES, 0u);
	if(n_ret != S_OK) app_exit(0xffffffff, TEXT("AudioPB::buffer_play: Error: IAudioRenderClient::ReleaseBuffer failed."));

	return;
}

VOID WINAPI AudioPB::audio_hw_wait(VOID)
{
	ULONG_PTR n_frames_free = 0u;
	HRESULT n_ret = 0;
	UINT32 u32 = 0u;

	do{
		n_ret = this->p_audiomgr->GetCurrentPadding(&u32);
		if(n_ret != S_OK) app_exit(0xffffffff, TEXT("AudioPB::audio_hw_wait: Error: IAudioClient::GetCurrentPadding failed."));

		n_frames_free = this->AUDIOBUFFER_SIZE_FRAMES - ((ULONG_PTR) u32);

		Sleep(1u);
	}while(n_frames_free < this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES);

	return;
}
