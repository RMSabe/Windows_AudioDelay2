/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.0.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOPB_HPP
#define AUDIOPB_HPP

#include "globldef.h"
#include "strdef.hpp"
#include "shared.hpp"

#include "AudioRTDelay.hpp"

#include <mmdeviceapi.h>
#include <audioclient.h>

struct _audiopb_params {
	SIZE_T delay_buffer_size_frames;
	SIZE_T n_ff_delays;
	SIZE_T n_fb_delays;
	const TCHAR *file_dir;
	ULONG64 audio_data_begin;
	ULONG64 audio_data_end;
	UINT32 sample_rate;
	UINT16 n_channels;
};

typedef struct _audiopb_params audiopb_params_t;

class AudioPB {
	public:
		AudioPB(const audiopb_params_t *p_params);

		BOOL WINAPI setParameters(const audiopb_params_t *p_params);
		BOOL WINAPI initialize(VOID);
		BOOL WINAPI runPlayback(VOID);
		VOID WINAPI stopPlayback(VOID);

		BOOL WINAPI loadAudioDeviceList(HWND p_listbox);
		BOOL WINAPI chooseDevice(SIZE_T index);
		BOOL WINAPI chooseDefaultDevice(VOID);

		__string WINAPI getLastErrorMessage(VOID);

		/* INTERNAL AudioRTDelay object routing methods */

		FLOAT WINAPI rtdelayGetDryInputAmplitude(VOID);
		FLOAT WINAPI rtdelayGetOutputAmplitude(VOID);

		BOOL WINAPI rtdelayGetFFParams(SIZE_T n_fx, audiortdelay_fx_params_t *p_params);
		BOOL WINAPI rtdelayGetFBParams(SIZE_T n_fx, audiortdelay_fx_params_t *p_params);

		BOOL WINAPI rtdelaySetDryInputAmplitude(FLOAT amp);
		BOOL WINAPI rtdelaySetOutputAmplitude(FLOAT amp);

		BOOL WINAPI rtdelaySetFFDelay(SIZE_T n_fx, SIZE_T delay);
		BOOL WINAPI rtdelaySetFFAmplitude(SIZE_T n_fx, FLOAT amp);

		BOOL WINAPI rtdelaySetFBDelay(SIZE_T n_fx, SIZE_T delay);
		BOOL WINAPI rtdelaySetFBAmplitude(SIZE_T n_fx, FLOAT amp);

		BOOL WINAPI rtdelayResetFFParams(VOID);
		BOOL WINAPI rtdelayResetFBParams(VOID);

		enum Status {
			STATUS_ERROR_MEMALLOC = -4,
			STATUS_ERROR_NOFILE = -3,
			STATUS_ERROR_AUDIOHW = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_READY = 1,
			STATUS_PLAYING = 2
		};

	protected:
		HANDLE h_filein = INVALID_HANDLE_VALUE;
		HANDLE p_loadthread = NULL;
		HANDLE p_playthread = NULL;

		fileptr64_t filein_size_64 = {
			.l32 = 0u,
			.h32 = 0u
		};

		fileptr64_t filein_pos_64 = {
			.l32 = 0u,
			.h32 = 0u
		};

		ULONG64 AUDIO_DATA_BEGIN = 0u;
		ULONG64 AUDIO_DATA_END = 0u;

		AudioRTDelay *p_delay = NULL;

		IMMDeviceEnumerator *p_audiodevenum = NULL;
		IMMDeviceCollection *p_audiodevcoll = NULL;

		IMMDevice *p_audiodev = NULL;
		IAudioClient *p_audiomgr = NULL;
		IAudioRenderClient *p_audioout = NULL;

		SIZE_T AUDIOBUFFER_SIZE_FRAMES = 0u;
		SIZE_T AUDIOBUFFER_SIZE_SAMPLES = 0u;
		SIZE_T AUDIOBUFFER_SIZE_BYTES = 0u;

		SIZE_T AUDIOBUFFER_SEGMENT_SIZE_FRAMES = 0u;
		SIZE_T AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		SIZE_T AUDIOBUFFER_SEGMENT_SIZE_BYTES = 0u;

		SIZE_T RTDELAY_BUFFER_SIZE_FRAMES = 0u;
		SIZE_T RTDELAY_BUFFER_N_SEGMENTS = 0u;
		SIZE_T RTDELAY_FF_PARAMS_LENGTH = 0u;
		SIZE_T RTDELAY_FB_PARAMS_LENGTH = 0u;

		SIZE_T rtdelay_nseg = 0u;

		VOID *p_bufferinput = NULL;
		VOID *p_bufferoutput0 = NULL;
		VOID *p_bufferoutput1 = NULL;

		VOID *p_audiobuffer = NULL;
		VOID *p_loadout = NULL;
		VOID *p_playout = NULL;

		__string FILEIN_DIR = TEXT("");
		__string err_msg = TEXT("");

		UINT32 SAMPLE_RATE = 0u;
		UINT16 N_CHANNELS = 0u;

		INT status = this->STATUS_UNINITIALIZED;

		BOOL stop_playback = FALSE;
		BOOL bufferout_cycle = FALSE;

		BOOL WINAPI filein_open(VOID);
		VOID WINAPI filein_close(VOID);

		virtual BOOL WINAPI audio_hw_init(VOID) = 0;

		VOID WINAPI audio_hw_deinit_device(VOID);
		VOID WINAPI audio_hw_deinit_all(VOID);

		VOID WINAPI stop_all_threads(VOID);

		virtual BOOL WINAPI buffer_alloc(VOID) = 0;
		virtual VOID WINAPI buffer_free(VOID) = 0;

		VOID WINAPI playback_proc(VOID);
		VOID WINAPI playback_init(VOID);
		VOID WINAPI playback_loop(VOID);

		VOID WINAPI rtdelay_nseg_update(VOID);
		VOID WINAPI bufferout_remap(VOID);

		virtual VOID WINAPI buffer_load_in(VOID) = 0;
		virtual VOID WINAPI buffer_load_out(VOID) = 0;
		virtual VOID WINAPI buffer_play(VOID) = 0;

		VOID WINAPI audio_hw_wait(VOID);

		DWORD WINAPI loadthread_proc(VOID *p_args);
		DWORD WINAPI playthread_proc(VOID *p_args);
};

#endif /*AUDIOPB_HPP*/
