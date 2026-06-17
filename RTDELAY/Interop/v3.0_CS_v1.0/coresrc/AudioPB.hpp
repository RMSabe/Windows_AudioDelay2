/*
	Real-Time Audio Delay 2 application for Windows
	Version 3.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOPB_HPP
#define AUDIOPB_HPP

#include "globldef.h"
#include "strdef.hpp"
#include "shared.hpp"

#include "AudioDelay.hpp"

#include <mmdeviceapi.h>
#include <audioclient.h>

struct _audiopb_params {
	ULONG64 audio_data_begin;
	ULONG64 audio_data_end;
	const TCHAR *file_dir;
	ULONG_PTR sample_rate;
	ULONG_PTR n_channels;
	ULONG_PTR audiobuffer_size_frames;
	ULONG_PTR streambuffer_segment_size_frames;
	ULONG_PTR streambuffer_n_segments;
	ULONG_PTR delay_buffer_size_frames;
	ULONG_PTR n_ff_delays;
	ULONG_PTR n_fb_delays;
};

typedef struct _audiopb_params audiopb_params_t;

struct _audiodevicelist {
	IMMDeviceCollection *p_devcoll;
	TCHAR *p_devlist;
	ULONG_PTR devlist_n_entries;
};

typedef struct _audiodevicelist audiodevicelist_t;

struct _audiodevice {
	IMMDevice *p_device;
	IAudioClient *p_audioclient;
	IUnknown *p_audioservice;
};

typedef struct _audiodevice audiodevice_t;

class AudioPB {
	public:
		AudioPB(const audiopb_params_t *p_params);

		BOOL WINAPI setParameters(const audiopb_params_t *p_params);
		BOOL WINAPI initialize(VOID);
		BOOL WINAPI runPlayback(VOID);

		VOID WINAPI pausePlayback(VOID);
		VOID WINAPI resumePlayback(VOID);
		VOID WINAPI stopPlayback(VOID);

		LONG64 WINAPI getAudioDataSizeFrames(VOID);
		LONG64 WINAPI getAudioDataPositionFrames(VOID);
		BOOL WINAPI setAudioDataPositionFrames(ULONG64 position);

		BOOL WINAPI loadAudioDeviceList(VOID);
		LONG_PTR WINAPI getAudioDeviceListEntryCount(VOID);
		const TCHAR* WINAPI getAudioDeviceListEntry(ULONG_PTR index);

		BOOL WINAPI chooseDevice(ULONG_PTR index);
		BOOL WINAPI chooseDefaultDevice(VOID);

		INT WINAPI getStatus(VOID);
		__string WINAPI getLastErrorMessage(VOID);

		/* INTERNAL AudioDelay object routing methods */

		FLOAT WINAPI delayGetDryInputAmplitude(VOID);
		FLOAT WINAPI delayGetOutputAmplitude(VOID);

		BOOL WINAPI delaySetDryInputAmplitude(FLOAT amp);
		BOOL WINAPI delaySetOutputAmplitude(FLOAT amp);

		LONG_PTR WINAPI delayGetFFDelay(ULONG_PTR n_fx);
		FLOAT WINAPI delayGetFFAmplitude(ULONG_PTR n_fx);

		BOOL WINAPI delaySetFFDelay(ULONG_PTR n_fx, ULONG_PTR delay);
		BOOL WINAPI delaySetFFAmplitude(ULONG_PTR n_fx, FLOAT amp);

		LONG_PTR WINAPI delayGetFBDelay(ULONG_PTR n_fx);
		FLOAT WINAPI delayGetFBAmplitude(ULONG_PTR n_fx);

		BOOL WINAPI delaySetFBDelay(ULONG_PTR n_fx, ULONG_PTR delay);
		BOOL WINAPI delaySetFBAmplitude(ULONG_PTR n_fx, FLOAT amp);

		BOOL WINAPI delayResetFFParams(VOID);
		BOOL WINAPI delayResetFBParams(VOID);

		enum Status {
			STATUS_ERROR_INVALIDPARAMS = -5,
			STATUS_ERROR_MEMORY = -4,
			STATUS_ERROR_AUDIOHW = -3,
			STATUS_ERROR_NOFILE = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_READY = 1,
			STATUS_RUNNING = 2,
			STATUS_PAUSED = 3,
			STATUS_STOPPED = 4
		};

	protected:
		static constexpr ULONG_PTR N_CHANNELS_MIN = 1u;
		static constexpr ULONG_PTR STREAMBUFFER_N_SEGMENTS_MIN = 2u;
		static constexpr ULONG_PTR STREAMBUFFER_SEGMENT_SIZE_FRAMES_MIN = 32u;

		static constexpr ULONG_PTR AUDIODEVICELIST_ENTRYLENGTH = 256u;

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

		__declspec(align(PTR_SIZE_BYTES)) HANDLE h_filein = INVALID_HANDLE_VALUE;

		__declspec(align(PTR_SIZE_BYTES)) AudioDelay *p_delay = NULL;

		__declspec(align(PTR_SIZE_BYTES)) IMMDeviceEnumerator *p_audiodevenum = NULL;

		__declspec(align(PTR_SIZE_BYTES)) audiodevicelist_t audiodevlist = {
			.p_devcoll = NULL,
			.p_devlist = NULL,
			.devlist_n_entries = 0u
		};

		__declspec(align(PTR_SIZE_BYTES)) audiodevice_t audiodev = {
			.p_device = NULL,
			.p_audioclient = NULL,
			.p_audioservice = NULL
		};

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIOBUFFER_SIZE_FRAMES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIOBUFFER_SIZE_SAMPLES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIOBUFFER_SIZE_BYTES = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR STREAMBUFFER_SIZE_FRAMES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR STREAMBUFFER_SIZE_SAMPLES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR STREAMBUFFER_SIZE_BYTES = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR STREAMBUFFER_SEGMENT_SIZE_FRAMES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR STREAMBUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR STREAMBUFFER_SEGMENT_SIZE_BYTES = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR STREAMBUFFER_N_SEGMENTS = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR INPUTBUFFER_SIZE_BYTES = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIODELAY_BUFFER_SIZE_FRAMES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIODELAY_BUFFER_N_SEGMENTS = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIODELAY_FF_PARAMS_LENGTH = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIODELAY_FB_PARAMS_LENGTH = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR SAMPLE_RATE = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR N_CHANNELS = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR FILE_BYTES_PER_SAMPLE = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIO_BYTES_PER_SAMPLE = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR AUDIODATA_BITS_PER_SAMPLE = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR streambuffer_nseg_playout = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR delaybuffer_nseg = 0u;

		__declspec(align(PTR_SIZE_BYTES)) VOID *p_inputbuffer = NULL;
		__declspec(align(PTR_SIZE_BYTES)) VOID *p_streambuffer = NULL;

		__declspec(align(PTR_SIZE_BYTES)) __string FILEIN_DIR = TEXT("");
		__declspec(align(PTR_SIZE_BYTES)) __string err_msg = TEXT("");

		__declspec(align(4)) INT status = this->STATUS_UNINITIALIZED;

		VOID WINAPI deinitialize(VOID);

		BOOL WINAPI filein_open(VOID);
		VOID WINAPI filein_close(VOID);

		BOOL WINAPI audiodevice_init(VOID);
		VOID WINAPI audiodevice_deinit(VOID);

		BOOL WINAPI audiodevicelist_init(VOID);
		BOOL WINAPI audiodevicelist_deinit(VOID);

		BOOL WINAPI buffer_alloc(VOID);
		BOOL WINAPI buffer_free(VOID);

		VOID WINAPI playback_proc(VOID);
		VOID WINAPI playback_init(VOID);
		VOID WINAPI playback_loop(VOID);

		VOID WINAPI streambuffer_nseg_playout_update(VOID);
		VOID WINAPI delaybuffer_nseg_update(VOID);

		virtual VOID WINAPI delaybuffer_loadin(VOID) = 0;
		virtual VOID WINAPI delaybuffer_loadout(VOID) = 0;

		VOID WINAPI buffer_play(VOID);
		VOID WINAPI audiodevice_wait(VOID);
};

#endif /*AUDIOPB_HPP*/
