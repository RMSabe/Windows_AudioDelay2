/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioPB_i16.hpp"
#include <math.h>

AudioPB_i16::AudioPB_i16(const audiopb_params_t *p_params) : AudioPB(p_params)
{
}

AudioPB_i16::~AudioPB_i16(VOID)
{
	this->stop_all_threads();
	this->status = this->STATUS_UNINITIALIZED;

	this->filein_close();
	this->audio_hw_deinit_all();
	this->buffer_free();

	if(this->p_delay != NULL)
	{
		delete this->p_delay;
		this->p_delay = NULL;
	}
}

BOOL WINAPI AudioPB_i16::audio_hw_init(VOID)
{
	HRESULT n_ret;
	UINT32 u32;

	DWORD channel_mask = 0u;
	UINT16 n_channel = 0u;

	WAVEFORMATEXTENSIBLE wavfmt;

	if(this->p_audiodev == NULL)
	{
		this->err_msg = TEXT("AudioPB_i16::audio_hw_init: Error: p_audiodev is NULL.");
		return FALSE;
	}

	n_ret = this->p_audiodev->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (VOID**) &(this->p_audiomgr));
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i16::audio_hw_init: Error: IMMDevice::Activate failed.");
		return FALSE;
	}

	channel_mask = 0u;
	for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++) channel_mask |= (1 << n_channel);

	ZeroMemory(&wavfmt, sizeof(WAVEFORMATEXTENSIBLE));

	wavfmt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wavfmt.Format.nChannels = (WORD) this->N_CHANNELS;
	wavfmt.Format.wBitsPerSample = 16u;
	wavfmt.Format.nBlockAlign = (wavfmt.Format.nChannels)*2u;
	wavfmt.Format.nSamplesPerSec = (DWORD) this->SAMPLE_RATE;
	wavfmt.Format.nAvgBytesPerSec = (wavfmt.Format.nSamplesPerSec)*((DWORD) wavfmt.Format.nBlockAlign);
	wavfmt.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
	wavfmt.Samples.wValidBitsPerSample = 16u;
	wavfmt.dwChannelMask = channel_mask;
	wavfmt.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

	n_ret = this->p_audiomgr->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, (WAVEFORMATEX*) &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i16::audio_hw_init: Error: audio stream format not supported.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 0, 10000000, 0, (WAVEFORMATEX*) &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i16::audio_hw_init: Error: IAudioClient::Initialize failed.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetBufferSize(&u32);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i16::audio_hw_init: Error: IAudioClient::GetBufferSize failed.");
		return FALSE;
	}

	n_ret = this->p_audiomgr->GetService(__uuidof(IAudioRenderClient), (VOID**) &(this->p_audioout));
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB_i16::audio_hw_init: Error: IAudioClient::GetService failed.");
		return FALSE;
	}

	this->AUDIOBUFFER_SIZE_FRAMES = (SIZE_T) u32;
	this->AUDIOBUFFER_SIZE_SAMPLES = (this->AUDIOBUFFER_SIZE_FRAMES)*((SIZE_T) this->N_CHANNELS);
	this->AUDIOBUFFER_SIZE_BYTES = this->AUDIOBUFFER_SIZE_SAMPLES*2u;

	this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES = _get_closest_power2_ceil(this->AUDIOBUFFER_SIZE_FRAMES/2u);
	this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = (this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES)*((SIZE_T) this->N_CHANNELS);
	this->AUDIOBUFFER_SEGMENT_SIZE_BYTES = this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES*2u;

	this->RTDELAY_BUFFER_N_SEGMENTS = this->RTDELAY_BUFFER_SIZE_FRAMES/this->AUDIOBUFFER_SEGMENT_SIZE_FRAMES;

	return TRUE;
}

BOOL WINAPI AudioPB_i16::buffer_alloc(VOID)
{
	this->buffer_free();

	this->p_bufferinput = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);
	this->p_bufferoutput0 = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);
	this->p_bufferoutput1 = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);

	if(this->p_bufferinput == NULL)
	{
		this->buffer_free();
		return FALSE;
	}

	if(this->p_bufferoutput0 == NULL)
	{
		this->buffer_free();
		return FALSE;
	}

	if(this->p_bufferoutput1 == NULL)
	{
		this->buffer_free();
		return FALSE;
	}

	return TRUE;
}

VOID WINAPI AudioPB_i16::buffer_free(VOID)
{
	if(this->p_bufferinput != NULL)
	{
		HeapFree(p_processheap, 0u, this->p_bufferinput);
		this->p_bufferinput = NULL;
	}

	if(this->p_bufferoutput0 != NULL)
	{
		HeapFree(p_processheap, 0u, this->p_bufferoutput0);
		this->p_bufferoutput0 = NULL;
	}

	if(this->p_bufferoutput1 != NULL)
	{
		HeapFree(p_processheap, 0u, this->p_bufferoutput1);
		this->p_bufferoutput1 = NULL;
	}

	return;
}

VOID WINAPI AudioPB_i16::buffer_load_in(VOID)
{
	SIZE_T n_sample = 0u;
	FLOAT *p_loadseg_f32 = NULL;
	INT16 *p_input = NULL;

	FLOAT factor = 0.0f;
	FLOAT f32 = 0.0f;

	DWORD dummy_32;

	if(*((ULONG64*) &(this->filein_pos_64)) >= this->AUDIO_DATA_END)
	{
		this->stop_playback = TRUE;
		return;
	}

	p_loadseg_f32 = this->p_delay->getInputBufferSegment(this->rtdelay_nseg);
	if(p_loadseg_f32 == NULL)
	{
		this->err_msg = TEXT("AudioPB_i16::buffer_load_in: Error: AudioRTDelay::getInputBufferSegment returned NULL.\r\nExtended Error Message: ") + this->p_delay->getLastErrorMessage();
		app_exit(1u, this->err_msg.c_str());
	}

	p_input = (INT16*) this->p_bufferinput;

	ZeroMemory(p_input, this->AUDIOBUFFER_SEGMENT_SIZE_BYTES);

	SetFilePointer(this->h_filein, (LONG) this->filein_pos_64.l32, (LONG*) &(this->filein_pos_64.h32), FILE_BEGIN);
	ReadFile(this->h_filein, p_input, (DWORD) this->AUDIOBUFFER_SEGMENT_SIZE_BYTES, &dummy_32, NULL);
	*((ULONG64*) &(this->filein_pos_64)) += (ULONG64) this->AUDIOBUFFER_SEGMENT_SIZE_BYTES;

	factor = this->SAMPLE_FACTOR;

	for(n_sample = 0u; n_sample < this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		f32 = (FLOAT) p_input[n_sample];
		f32 /= factor;
		p_loadseg_f32[n_sample] = f32;
	}

	return;
}

VOID WINAPI AudioPB_i16::buffer_load_out(VOID)
{
	SIZE_T n_sample = 0u;
	FLOAT *p_loadseg_f32 = NULL;
	INT16 *p_output = NULL;

	FLOAT factor = 0.0f;
	FLOAT f32 = 0.0f;

	p_loadseg_f32 = this->p_delay->getOutputBufferSegment(this->rtdelay_nseg);
	if(p_loadseg_f32 == NULL)
	{
		this->err_msg = TEXT("AudioPB_i16::buffer_load_out: Error: AudioRTDelay::getOutputBufferSegment returned NULL.\r\nExtended Error Message: ") + this->p_delay->getLastErrorMessage();
		app_exit(1u, this->err_msg.c_str());
	}

	p_output = (INT16*) this->p_loadout;

	factor = this->SAMPLE_FACTOR - 1.0f;

	for(n_sample = 0u; n_sample < this->AUDIOBUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		f32 = p_loadseg_f32[n_sample];

		if(f32 > 1.0f) f32 = 1.0f;
		else if(f32 < -1.0f) f32 = -1.0f;

		f32 *= factor;

		p_output[n_sample] = (INT16) roundf(f32);
	}

	return;
}
