/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.3

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
	ULONG_PTR delay_buffer_size_frames;
	ULONG_PTR n_ff_delays;
	ULONG_PTR n_fb_delays;
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

		BOOL WINAPI loadAudioDeviceList(VOID);
		LONG_PTR WINAPI getAudioDeviceListEntryCount(VOID);
		const TCHAR* WINAPI getAudioDeviceListEntry(ULONG_PTR index);

		BOOL WINAPI chooseDevice(ULONG_PTR index);
		BOOL WINAPI chooseDefaultDevice(VOID);

		__string WINAPI getLastErrorMessage(VOID);

		/* INTERNAL AudioRTDelay object routing methods */

		FLOAT WINAPI rtdelayGetDryInputAmplitude(VOID);
		FLOAT WINAPI rtdelayGetOutputAmplitude(VOID);

		BOOL WINAPI rtdelayGetFFParams(ULONG_PTR n_fx, audiortdelay_fx_params_t *p_params);
		BOOL WINAPI rtdelayGetFBParams(ULONG_PTR n_fx, audiortdelay_fx_params_t *p_params);

		BOOL WINAPI rtdelaySetDryInputAmplitude(FLOAT amp);
		BOOL WINAPI rtdelaySetOutputAmplitude(FLOAT amp);

		BOOL WINAPI rtdelaySetFFDelay(ULONG_PTR n_fx, ULONG_PTR delay);
		BOOL WINAPI rtdelaySetFFAmplitude(ULONG_PTR n_fx, FLOAT amp);

		BOOL WINAPI rtdelaySetFBDelay(ULONG_PTR n_fx, ULONG_PTR delay);
		BOOL WINAPI rtdelaySetFBAmplitude(ULONG_PTR n_fx, FLOAT amp);

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
		static constexpr ULONG_PTR AUDIODEVICELIST_ENTRYLENGTH = 256u;

		__declspec(align(PTR_SIZE_BYTES)) HANDLE h_filein = INVALID_HANDLE_VALUE;

		__declspec(align(PTR_SIZE_BYTES)) fileptr64_t filein_size_64 = {
			.l32 = 0u,
			.h32 = 0u
		};

		__declspec(align(PTR_SIZE_BYTES)) fileptr64_t filein_pos_64 = {
			.l32 = 0u,
			.h32 = 0u
		};

		__declspec(align(PTR_SIZE_BYTES)) ULONG64 AUDIO_DATA_BEGIN = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG64 AUDIO_DATA_END = 0u;

		__declspec(align(PTR_SIZE_BYTES)) AudioRTDelay *p_delay = NULL;

		__declspec(align(PTR_SIZE_BYTES)) IMMDeviceEnumerator *p_audiodevenum = NULL;
		__declspec(align(PTR_SIZE_BYTES)) IMMDeviceCollection *p_audiodevcoll = NULL;

		__declspec(align(PTR_SIZE_BYTES)) IMMDevice *p_audiodev = NULL;
		__declspec(align(PTR_SIZE_BYTES)) IAudioClient *p_audiomgr = NULL;
		__declspec(align(PTR_SIZE_BYTES)) IAudioRenderClient *p_audioout = NULL;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIOBUFFER_SIZE_FRAMES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIOBUFFER_SIZE_SAMPLES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIOBUFFER_SIZE_BYTES = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIOBUFFER_SEGMENT_SIZE_FRAMES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIOBUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIOBUFFER_SEGMENT_SIZE_BYTES = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR RTDELAY_BUFFER_SIZE_FRAMES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR RTDELAY_BUFFER_N_SEGMENTS = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR RTDELAY_FF_PARAMS_LENGTH = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR RTDELAY_FB_PARAMS_LENGTH = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR rtdelay_nseg = 0u;

		__declspec(align(PTR_SIZE_BYTES)) VOID *p_bufferinput = NULL;
		__declspec(align(PTR_SIZE_BYTES)) VOID *p_bufferoutput0 = NULL;
		__declspec(align(PTR_SIZE_BYTES)) VOID *p_bufferoutput1 = NULL;

		__declspec(align(PTR_SIZE_BYTES)) VOID *p_audiobuffer = NULL;
		__declspec(align(PTR_SIZE_BYTES)) VOID *p_loadout = NULL;
		__declspec(align(PTR_SIZE_BYTES)) VOID *p_playout = NULL;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR audiodevicelist_n_entries = 0u;
		__declspec(align(PTR_SIZE_BYTES)) TCHAR *p_audiodevicelist = NULL;

		__declspec(align(PTR_SIZE_BYTES)) __string FILEIN_DIR = TEXT("");
		__declspec(align(PTR_SIZE_BYTES)) __string err_msg = TEXT("");

		__declspec(align(4)) UINT32 SAMPLE_RATE = 0u;
		__declspec(align(2)) UINT16 N_CHANNELS = 0u;

		__declspec(align(4)) INT status = this->STATUS_UNINITIALIZED;

		__declspec(align(4)) BOOL stop_playback = FALSE;
		__declspec(align(4)) BOOL bufferout_cycle = FALSE;

		VOID WINAPI deinitialize(VOID);

		BOOL WINAPI filein_open(VOID);
		VOID WINAPI filein_close(VOID);

		virtual BOOL WINAPI audio_hw_init(VOID) = 0;

		VOID WINAPI audio_hw_deinit_device(VOID);
		VOID WINAPI audio_hw_deinit_all(VOID);

		virtual BOOL WINAPI buffer_alloc(VOID) = 0;
		virtual VOID WINAPI buffer_free(VOID) = 0;

		BOOL WINAPI audiodevicelist_alloc(VOID);
		VOID WINAPI audiodevicelist_free(VOID);

		VOID WINAPI playback_proc(VOID);
		VOID WINAPI playback_init(VOID);
		VOID WINAPI playback_loop(VOID);

		VOID WINAPI rtdelay_nseg_update(VOID);
		VOID WINAPI bufferout_remap(VOID);

		virtual VOID WINAPI buffer_load_in(VOID) = 0;
		virtual VOID WINAPI buffer_load_out(VOID) = 0;

		VOID WINAPI buffer_play(VOID);
		VOID WINAPI audio_hw_wait(VOID);
};

#endif /*AUDIOPB_HPP*/
