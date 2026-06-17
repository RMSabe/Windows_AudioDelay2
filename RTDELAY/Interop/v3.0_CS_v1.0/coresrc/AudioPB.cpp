/*
	Real-Time Audio Delay 2 application for Windows
	Version 3.0

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
	if(this->status > 0)
	{
		this->err_msg = TEXT("AudioPB::setParameters: Error: cannot set parameters, AudioPB object already initialized.");
		return FALSE;
	}

	if(p_params == NULL)
	{
		this->err_msg = TEXT("AudioPB::setParameters: Error: given params object is invalid.");
		return FALSE;
	}

	if(p_params->file_dir == NULL)
	{
		this->err_msg = TEXT("AudioPB::setParameters: Error: given parameter file_dir is invalid.");
		return FALSE;
	}

	this->status = this->STATUS_UNINITIALIZED;

	this->AUDIO_DATA_BEGIN = p_params->audio_data_begin;
	this->AUDIO_DATA_END = p_params->audio_data_end;
	this->FILEIN_DIR = p_params->file_dir;
	this->SAMPLE_RATE = p_params->sample_rate;
	this->N_CHANNELS = p_params->n_channels;
	this->AUDIOBUFFER_SIZE_FRAMES = _get_closest_power2_ceil(p_params->audiobuffer_size_frames);
	this->STREAMBUFFER_SEGMENT_SIZE_FRAMES = _get_closest_power2_ceil(p_params->streambuffer_segment_size_frames);
	this->STREAMBUFFER_N_SEGMENTS = _get_closest_power2_ceil(p_params->streambuffer_n_segments);
	this->AUDIODELAY_BUFFER_SIZE_FRAMES = _get_closest_power2_ceil(p_params->delay_buffer_size_frames);
	this->AUDIODELAY_FF_PARAMS_LENGTH = p_params->n_ff_delays;
	this->AUDIODELAY_FB_PARAMS_LENGTH = p_params->n_fb_delays;

	return TRUE;
}

BOOL WINAPI AudioPB::initialize(VOID)
{
	audiodelay_init_params_t delay_params;

	if(this->status > 0) return TRUE;

	this->status = this->STATUS_UNINITIALIZED;

	if(!this->SAMPLE_RATE)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioPB::initialize: Error: invalid sample rate.");
		return FALSE;
	}

	if(this->N_CHANNELS < this->N_CHANNELS_MIN)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioPB::initialize: Error: invalid number of channels.");
		return FALSE;
	}

	if(this->STREAMBUFFER_SEGMENT_SIZE_FRAMES < this->STREAMBUFFER_SEGMENT_SIZE_FRAMES_MIN)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioPB::initialize: Error: invalid stream buffer segment size.");
		return FALSE;
	}

	if(this->STREAMBUFFER_N_SEGMENTS < this->STREAMBUFFER_N_SEGMENTS_MIN)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioPB::initialize: Error: invalid stream buffer segment count.");
		return FALSE;
	}

	if(this->AUDIOBUFFER_SIZE_FRAMES < this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioPB::initialize: Error: invalid audio buffer size. (audio buffer is smaller than stream buffer segment).");
		return FALSE;
	}

	if(this->AUDIODELAY_BUFFER_SIZE_FRAMES < this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioPB::initialize: Error: invalid delay buffer size. (delay buffer is smaller than stream buffer segment).");
		return FALSE;
	}

	this->AUDIOBUFFER_SIZE_SAMPLES = (this->AUDIOBUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->AUDIOBUFFER_SIZE_BYTES = (this->AUDIOBUFFER_SIZE_SAMPLES)*(this->AUDIO_BYTES_PER_SAMPLE);

	this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES = (this->STREAMBUFFER_SEGMENT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->STREAMBUFFER_SEGMENT_SIZE_BYTES = (this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES)*(this->AUDIO_BYTES_PER_SAMPLE);

	this->STREAMBUFFER_SIZE_FRAMES = (this->STREAMBUFFER_N_SEGMENTS)*(this->STREAMBUFFER_SEGMENT_SIZE_FRAMES);
	this->STREAMBUFFER_SIZE_SAMPLES = (this->STREAMBUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->STREAMBUFFER_SIZE_BYTES = (this->STREAMBUFFER_SIZE_SAMPLES)*(this->AUDIO_BYTES_PER_SAMPLE);

	this->INPUTBUFFER_SIZE_BYTES = (this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES)*(this->FILE_BYTES_PER_SAMPLE);

	this->AUDIODELAY_BUFFER_N_SEGMENTS = (this->AUDIODELAY_BUFFER_SIZE_FRAMES)/(this->STREAMBUFFER_SEGMENT_SIZE_FRAMES);

	if(!this->filein_open())
	{
		this->status = this->STATUS_ERROR_NOFILE;
		this->err_msg = TEXT("AudioPB::initialize: Error: open input file failed.");
		return FALSE;
	}

	if(!this->audiodevice_init())
	{
		this->status = this->STATUS_ERROR_AUDIOHW;
		this->filein_close();
		return FALSE;
	}

	if(!this->buffer_alloc())
	{
		this->status = this->STATUS_ERROR_MEMORY;
		this->filein_close();
		this->audiodevice_deinit();
		return FALSE;
	}

	delay_params.buffer_size_frames = this->AUDIODELAY_BUFFER_SIZE_FRAMES;
	delay_params.buffer_n_segments = this->AUDIODELAY_BUFFER_N_SEGMENTS;
	delay_params.n_channels = this->N_CHANNELS;
	delay_params.n_ff_delays = this->AUDIODELAY_FF_PARAMS_LENGTH;
	delay_params.n_fb_delays = this->AUDIODELAY_FB_PARAMS_LENGTH;

	if(this->p_delay == NULL)
	{
		this->p_delay = new AudioDelay(&delay_params);
		if(this->p_delay == NULL)
		{
			this->status = this->STATUS_ERROR_MEMORY;
			this->err_msg = TEXT("AudioPB::initialize: Error: AudioDelay object instance failed.");
			this->filein_close();
			this->audiodevice_deinit();
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
		this->audiodevice_deinit();
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
		this->err_msg = TEXT("AudioPB::runPlayback: Error: AudioPB object is either not initialized or already running playback.");
		return FALSE;
	}

	this->playback_proc();

	this->filein_close();
	this->audiodevice_deinit();
	this->buffer_free();

	this->status = this->STATUS_UNINITIALIZED;
	return TRUE;
}

VOID WINAPI AudioPB::pausePlayback(VOID)
{
	if(this->status == this->STATUS_RUNNING)
	{
		this->status = this->STATUS_PAUSED;
		this->audiodevice_wait();
		this->audiodev.p_audioclient->Stop();
	}

	return;
}

VOID WINAPI AudioPB::resumePlayback(VOID)
{
	if(this->status == this->STATUS_PAUSED)
	{
		this->audiodev.p_audioclient->Start();
		this->status = this->STATUS_RUNNING;
	}

	return;
}

VOID WINAPI AudioPB::stopPlayback(VOID)
{
	if(this->status > this->STATUS_READY) this->status = this->STATUS_STOPPED;
	return;
}

LONG64 WINAPI AudioPB::getAudioDataSizeFrames(VOID)
{
	ULONG_PTR _bytes_per_frame;

	if(this->status < 1) return -1;

	_bytes_per_frame = (this->FILE_BYTES_PER_SAMPLE)*(this->N_CHANNELS);

	return (LONG64) ((this->AUDIO_DATA_END - this->AUDIO_DATA_BEGIN)/((ULONG64) _bytes_per_frame));
}

LONG64 WINAPI AudioPB::getAudioDataPositionFrames(VOID)
{
	ULONG_PTR _bytes_per_frame;

	if(this->status < 1) return -1;

	_bytes_per_frame = (this->FILE_BYTES_PER_SAMPLE)*(this->N_CHANNELS);

	return (LONG64) ((*((ULONG64*) &(this->filein_pos_64)) - this->AUDIO_DATA_BEGIN)/((ULONG64) _bytes_per_frame));
}

BOOL WINAPI AudioPB::setAudioDataPositionFrames(ULONG64 position)
{
	ULONG64 _audiodata_size_frames;
	ULONG_PTR _bytes_per_frame;

	if(this->status < 1) return FALSE;

	_audiodata_size_frames = (ULONG64) this->getAudioDataSizeFrames();
	if(position >= _audiodata_size_frames)
	{
		this->err_msg = TEXT("AudioPB::setAudioDataPositionFrames: Error: invalid position value.");
		return FALSE;
	}

	_bytes_per_frame = (this->FILE_BYTES_PER_SAMPLE)*(this->N_CHANNELS);

	*((ULONG64*) &(this->filein_pos_64)) = this->AUDIO_DATA_BEGIN + position*((ULONG64) _bytes_per_frame);

	return TRUE;
}

BOOL WINAPI AudioPB::loadAudioDeviceList(VOID)
{
	const PROPERTYKEY* const P_PKEY = (const PROPERTYKEY*) P_PKEY_Device_FriendlyName;
	IMMDevice *p_dev = NULL;
	IPropertyStore *p_devprop = NULL;
	ULONG_PTR audiodevicelist_byteoffset = 0u;
	HRESULT n_ret = 0;
	UINT n_dev = 0u;
	PROPVARIANT propvar;

	this->audiodevicelist_deinit();

	if(this->p_audiodevenum == NULL)
	{
		n_ret = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, __uuidof(IMMDeviceEnumerator), (VOID**) &(this->p_audiodevenum));
		if(n_ret != S_OK)
		{
			this->status = this->STATUS_ERROR_AUDIOHW;
			this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: CoCreateInstance (IMMDeviceEnumerator) failed.");
			return FALSE;
		}
	}

	if(!this->audiodevicelist_init()) return FALSE;

	audiodevicelist_byteoffset = 0u;

	for(n_dev = 0u; n_dev < (UINT) this->audiodevlist.devlist_n_entries; n_dev++)
	{
		n_ret = this->audiodevlist.p_devcoll->Item(n_dev, &p_dev);
		if(n_ret != S_OK)
		{
			this->audiodevicelist_deinit();
			this->status = this->STATUS_ERROR_AUDIOHW;
			this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: IMMDeviceCollection::Item failed.");
			return FALSE;
		}

		n_ret = p_dev->OpenPropertyStore(STGM_READ, &p_devprop);
		if(n_ret != S_OK)
		{
			p_dev->Release();
			this->audiodevicelist_deinit();
			this->status = this->STATUS_ERROR_AUDIOHW;
			this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: IMMDevice::OpenPropertyStore failed.");
			return FALSE;
		}

		PropVariantInit(&propvar);

		n_ret = p_devprop->GetValue(*P_PKEY, &propvar);
		if(n_ret != S_OK)
		{
			p_devprop->Release();
			p_dev->Release();
			this->audiodevicelist_deinit();
			this->status = this->STATUS_ERROR_AUDIOHW;
			this->err_msg = TEXT("AudioPB::loadAudioDeviceList: Error: IPropertyStore::GetValue failed.");
			return FALSE;
		}

		if(propvar.vt == VT_EMPTY) __SPRINTF((TCHAR*) (((ULONG_PTR) this->audiodevlist.p_devlist) + audiodevicelist_byteoffset), this->AUDIODEVICELIST_ENTRYLENGTH, TEXT("Unknown Audio Device"));
		else cstr_copy_wchar_to_tchar(propvar.pwszVal, (TCHAR*) (((ULONG_PTR) this->audiodevlist.p_devlist) + audiodevicelist_byteoffset), this->AUDIODEVICELIST_ENTRYLENGTH);

		PropVariantClear(&propvar);

		p_devprop->Release();
		p_devprop = NULL;

		p_dev->Release();
		p_dev = NULL;

		audiodevicelist_byteoffset += (this->AUDIODEVICELIST_ENTRYLENGTH)*(sizeof(TCHAR));
	}

	PropVariantClear(&propvar);

	if(p_devprop != NULL) p_devprop->Release();
	if(p_dev != NULL) p_dev->Release();

	this->status = this->STATUS_UNINITIALIZED;
	return TRUE;
}

LONG_PTR WINAPI AudioPB::getAudioDeviceListEntryCount(VOID)
{
	return (LONG_PTR) this->audiodevlist.devlist_n_entries;
}

const TCHAR* WINAPI AudioPB::getAudioDeviceListEntry(ULONG_PTR index)
{
	if(this->audiodevlist.p_devlist == NULL)
	{
		this->err_msg = TEXT("AudioPB::getAudioDeviceListEntry: Error: list is not loaded.");
		return NULL;
	}

	if(index >= this->audiodevlist.devlist_n_entries)
	{
		this->err_msg = TEXT("AudioPB::getAudioDeviceListEntry: Error: given entry index is out of bounds.");
		return NULL;
	}

	return (const TCHAR*) (((ULONG_PTR) (this->audiodevlist.p_devlist)) + index*(this->AUDIODEVICELIST_ENTRYLENGTH)*sizeof(TCHAR));
}

BOOL WINAPI AudioPB::chooseDevice(ULONG_PTR index)
{
	HRESULT n_ret = 0;

	if(this->status > 0)
	{
		this->err_msg = TEXT("AudioPB::chooseDevice: Error: cannot run method, AudioPB object is already initialized.");
		return FALSE;
	}

	if(this->audiodevlist.p_devcoll == NULL)
	{
		this->err_msg = TEXT("AudioPB::chooseDevice: Error: audio device list is not loaded.");
		return FALSE;
	}

	this->audiodevice_deinit();

	n_ret = this->audiodevlist.p_devcoll->Item((UINT) index, &(this->audiodev.p_device));
	if(n_ret != S_OK)
	{
		this->status = this->STATUS_ERROR_AUDIOHW;
		this->err_msg = TEXT("AudioPB::chooseDevice: Error: IMMDeviceCollection::Item failed.");
		return FALSE;
	}

	this->status = this->STATUS_UNINITIALIZED;
	return TRUE;
}

BOOL WINAPI AudioPB::chooseDefaultDevice(VOID)
{
	HRESULT n_ret = 0;

	if(this->status > 0)
	{
		this->err_msg = TEXT("AudioPB::chooseDefaultDevice: Error: cannot run method, AudioPB object is already initialized.");
		return FALSE;
	}

	if(this->p_audiodevenum == NULL)
	{
		this->err_msg = TEXT("AudioPB::chooseDefaultDevice: Error: audio device enumerator is not loaded.");
		return FALSE;
	}

	this->audiodevice_deinit();

	n_ret = this->p_audiodevenum->GetDefaultAudioEndpoint(eRender, eMultimedia, &(this->audiodev.p_device));
	if(n_ret != S_OK)
	{
		this->status = this->STATUS_ERROR_AUDIOHW;
		this->err_msg = TEXT("AudioPB::chooseDefaultDevice: Error: IMMDeviceEnumerator::GetDefaultAudioEndpoint failed.");
		return FALSE;
	}

	this->status = this->STATUS_UNINITIALIZED;
	return TRUE;
}

INT WINAPI AudioPB::getStatus(VOID)
{
	return this->status;
}

__string WINAPI AudioPB::getLastErrorMessage(VOID)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return (TEXT("Error: AudioPB object not initialized\r\nExtended error message: ") + this->err_msg);

	return this->err_msg;
}

FLOAT WINAPI AudioPB::delayGetDryInputAmplitude(VOID)
{
	if(this->status < 1) return 0.0f;

	return this->p_delay->getDryInputAmplitude();
}

FLOAT WINAPI AudioPB::delayGetOutputAmplitude(VOID)
{
	if(this->status < 1) return 0.0f;

	return this->p_delay->getOutputAmplitude();
}

BOOL WINAPI AudioPB::delaySetDryInputAmplitude(FLOAT amp)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->setDryInputAmplitude(amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::delaySetOutputAmplitude(FLOAT amp)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->setOutputAmplitude(amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

LONG_PTR WINAPI AudioPB::delayGetFFDelay(ULONG_PTR n_fx)
{
	audiodelay_fx_params_t _params;

	if(this->status < 1) return -1;

	if(!this->p_delay->getFFParams(n_fx, &_params))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return -1;
	}

	return (LONG_PTR) _params.delay;
}

FLOAT WINAPI AudioPB::delayGetFFAmplitude(ULONG_PTR n_fx)
{
	audiodelay_fx_params_t _params;

	if(this->status < 1) return 0.0f;

	if(!this->p_delay->getFFParams(n_fx, &_params))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return 0.0f;
	}

	return _params.amp;
}

BOOL WINAPI AudioPB::delaySetFFDelay(ULONG_PTR n_fx, ULONG_PTR delay)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->setFFDelay(n_fx, delay))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::delaySetFFAmplitude(ULONG_PTR n_fx, FLOAT amp)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->setFFAmplitude(n_fx, amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

LONG_PTR WINAPI AudioPB::delayGetFBDelay(ULONG_PTR n_fx)
{
	audiodelay_fx_params_t _params;

	if(this->status < 1) return -1;

	if(!this->p_delay->getFBParams(n_fx, &_params))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return -1;
	}

	return (LONG_PTR) _params.delay;
}

FLOAT WINAPI AudioPB::delayGetFBAmplitude(ULONG_PTR n_fx)
{
	audiodelay_fx_params_t _params;

	if(this->status < 1) return 0.0f;

	if(!this->p_delay->getFBParams(n_fx, &_params))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return 0.0f;
	}

	return _params.amp;
}

BOOL WINAPI AudioPB::delaySetFBDelay(ULONG_PTR n_fx, ULONG_PTR delay)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->setFBDelay(n_fx, delay))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::delaySetFBAmplitude(ULONG_PTR n_fx, FLOAT amp)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->setFBAmplitude(n_fx, amp))
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::delayResetFFParams(VOID)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->resetFFParams())
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::delayResetFBParams(VOID)
{
	if(this->status < 1) return FALSE;

	if(!this->p_delay->resetFBParams())
	{
		this->err_msg = this->p_delay->getLastErrorMessage();
		return FALSE;
	}

	return TRUE;
}

VOID WINAPI AudioPB::deinitialize(VOID)
{
	this->status = this->STATUS_UNINITIALIZED;

	this->filein_close();
	this->audiodevice_deinit();
	this->buffer_free();

	this->audiodevicelist_deinit();

	if(this->p_audiodevenum != NULL)
	{
		this->p_audiodevenum->Release();
		this->p_audiodevenum = NULL;
	}

	if(this->p_delay != NULL)
	{
		delete this->p_delay;
		this->p_delay = NULL;
	}

	return;
}

BOOL WINAPI AudioPB::filein_open(VOID)
{
	this->filein_close();

	this->h_filein = CreateFile(this->FILEIN_DIR.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
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

BOOL WINAPI AudioPB::audiodevice_init(VOID)
{
	ULONG64 audiobuffer_time;

	ULONG_PTR n_channel;
	HRESULT n_ret;
	UINT32 u32;

	DWORD channel_mask;

	WAVEFORMATEXTENSIBLE wavfmt;

	if(this->audiodev.p_audioservice != NULL)
	{
		this->audiodev.p_audioservice->Release();
		this->audiodev.p_audioservice = NULL;
	}

	if(this->audiodev.p_audioclient != NULL)
	{
		this->audiodev.p_audioclient->Stop();
		this->audiodev.p_audioclient->Release();
		this->audiodev.p_audioclient = NULL;
	}

	if(this->audiodev.p_device == NULL)
	{
		this->err_msg = TEXT("AudioPB::audiodevice_init: Error: audio device is not loaded.");
		return FALSE;
	}

	n_ret = this->audiodev.p_device->Activate(__uuidof(IAudioClient), CLSCTX_ALL, NULL, (VOID**) &(this->audiodev.p_audioclient));
	if(n_ret != S_OK)
	{
		this->audiodevice_deinit();
		this->err_msg = TEXT("AudioPB::audiodevice_init: Error: IMMDevice::Activate failed.");
		return FALSE;
	}

	channel_mask = 0u;
	for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++) channel_mask |= (1 << n_channel);

	ZeroMemory(&wavfmt, sizeof(WAVEFORMATEXTENSIBLE));

	wavfmt.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
	wavfmt.Format.nChannels = (WORD) this->N_CHANNELS;
	wavfmt.Format.wBitsPerSample = (WORD) (this->AUDIO_BYTES_PER_SAMPLE)*8u;
	wavfmt.Format.nBlockAlign = (wavfmt.Format.nChannels)*((WORD) this->AUDIO_BYTES_PER_SAMPLE);
	wavfmt.Format.nSamplesPerSec = (DWORD) this->SAMPLE_RATE;
	wavfmt.Format.nAvgBytesPerSec = (wavfmt.Format.nSamplesPerSec)*((DWORD) wavfmt.Format.nBlockAlign);
	wavfmt.Format.cbSize = sizeof(WAVEFORMATEXTENSIBLE) - sizeof(WAVEFORMATEX);
	wavfmt.Samples.wValidBitsPerSample = (WORD) this->AUDIODATA_BITS_PER_SAMPLE;
	wavfmt.dwChannelMask = channel_mask;
	wavfmt.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

	n_ret = this->audiodev.p_audioclient->IsFormatSupported(AUDCLNT_SHAREMODE_EXCLUSIVE, (WAVEFORMATEX*) &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->audiodevice_deinit();
		this->err_msg = TEXT("AudioPB::audiodevice_init: Error: audio stream format not supported.");
		return FALSE;
	}

	audiobuffer_time = ((ULONG64) this->AUDIOBUFFER_SIZE_FRAMES)*10000000/((ULONG64) this->SAMPLE_RATE);

	n_ret = this->audiodev.p_audioclient->Initialize(AUDCLNT_SHAREMODE_EXCLUSIVE, 0, (REFERENCE_TIME) audiobuffer_time, 0, (WAVEFORMATEX*) &wavfmt, NULL);
	if(n_ret != S_OK)
	{
		this->audiodevice_deinit();
		this->err_msg = TEXT("AudioPB::audiodevice_init: Error: IAudioClient::Initialize failed.");
		return FALSE;
	}

	/*n_ret = this->p_audiomgr->GetBufferSize(&u32);
	if(n_ret != S_OK)
	{
		this->audio_hw_deinit_device();
		this->err_msg = TEXT("AudioPB::audiodevice_init: Error: IAudioClient::GetBufferSize failed.");
		return FALSE;
	}*/

	n_ret = this->audiodev.p_audioclient->GetService(__uuidof(IAudioRenderClient), (VOID**) &(this->audiodev.p_audioservice));
	if(n_ret != S_OK)
	{
		this->audiodevice_deinit();
		this->err_msg = TEXT("AudioPB::audiodevice_init: Error: IAudioClient::GetService failed.");
		return FALSE;
	}

	return TRUE;
}

VOID WINAPI AudioPB::audiodevice_deinit(VOID)
{
	if(this->audiodev.p_audioservice != NULL)
	{
		this->audiodev.p_audioservice->Release();
		this->audiodev.p_audioservice = NULL;
	}

	if(this->audiodev.p_audioclient != NULL)
	{
		this->audiodev.p_audioclient->Stop();
		this->audiodev.p_audioclient->Release();
		this->audiodev.p_audioclient = NULL;
	}

	if(this->audiodev.p_device != NULL)
	{
		this->audiodev.p_device->Release();
		this->audiodev.p_device = NULL;
	}

	return;
}

BOOL WINAPI AudioPB::audiodevicelist_init(VOID)
{
	HRESULT n_ret;
	UINT devcoll_count;

	if(p_processheap == NULL)
	{
		this->err_msg = TEXT("AudioPB::audiodevicelist_init: Error: p_processheap is NULL.");
		return FALSE;
	}

	if(this->p_audiodevenum == NULL)
	{
		this->err_msg = TEXT("AudioPB::audiodevicelist_init: Error: audio device enumerator not initialized.");
		return FALSE;
	}

	if(!this->audiodevicelist_deinit()) return FALSE;

	n_ret = this->p_audiodevenum->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &(this->audiodevlist.p_devcoll));
	if(n_ret != S_OK)
	{
		this->status = this->STATUS_ERROR_AUDIOHW;
		this->err_msg = TEXT("AudioPB::audiodevicelist_init: Error: IMMDeviceEnumerator::EnumAudioEndpoints failed.");
		return FALSE;
	}

	n_ret = this->audiodevlist.p_devcoll->GetCount(&devcoll_count);
	if(n_ret != S_OK)
	{
		this->audiodevicelist_deinit();
		this->status = this->STATUS_ERROR_AUDIOHW;
		this->err_msg = TEXT("AudioPB::audiodevicelist_init: Error: IMMDeviceCollection::GetCount failed.");
		return FALSE;
	}

	if(!devcoll_count)
	{
		this->audiodevicelist_deinit();
		this->err_msg = TEXT("AudioPB::audiodevicelist_init: Error: no audio devices found.");
		return FALSE;
	}

	this->audiodevlist.devlist_n_entries = (ULONG_PTR) devcoll_count;

	this->audiodevlist.p_devlist = (TCHAR*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, (this->audiodevlist.devlist_n_entries)*(this->AUDIODEVICELIST_ENTRYLENGTH)*sizeof(TCHAR));
	if(this->audiodevlist.p_devlist == NULL)
	{
		this->audiodevicelist_deinit();
		this->status = this->STATUS_ERROR_MEMORY;
		this->err_msg = TEXT("AudioPB::audiodevicelist_init: Error: failed to allocate heap memory.");
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::audiodevicelist_deinit(VOID)
{
	if(p_processheap == NULL)
	{
		this->err_msg = TEXT("AudioPB::audiodevicelist_deinit: Error: p_processheap is NULL.");
		return FALSE;
	}

	if(this->audiodevlist.p_devlist != NULL)
	{
		if(!HeapFree(p_processheap, 0u, this->audiodevlist.p_devlist))
		{
			this->status = this->STATUS_ERROR_MEMORY;
			this->err_msg = TEXT("AudioPB::audiodevicelist_deinit: Error: failed to release heap memory.");
			return FALSE;
		}

		this->audiodevlist.p_devlist = NULL;
	}

	this->audiodevlist.devlist_n_entries = 0u;

	if(this->audiodevlist.p_devcoll != NULL)
	{
		this->audiodevlist.p_devcoll->Release();
		this->audiodevlist.p_devcoll = NULL;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::buffer_alloc(VOID)
{
	if(!this->buffer_free()) return FALSE;

	if(p_processheap == NULL)
	{
		this->err_msg = TEXT("AudioPB::buffer_alloc: Error: p_processheap is NULL.");
		return FALSE;
	}

	this->p_streambuffer = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->STREAMBUFFER_SIZE_BYTES);
	this->p_inputbuffer = HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->INPUTBUFFER_SIZE_BYTES);

	if(this->p_streambuffer == NULL)
	{
		this->buffer_free();
		this->err_msg = TEXT("AudioPB::buffer_alloc: Error: failed to allocate heap memory.");
		return FALSE;
	}

	if(this->p_inputbuffer == NULL)
	{
		this->buffer_free();
		this->err_msg = TEXT("AudioPB::buffer_alloc: Error: failed to allocate heap memory.");
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioPB::buffer_free(VOID)
{
	if(p_processheap == NULL)
	{
		this->err_msg = TEXT("AudioPB::buffer_free: Error: p_processheap is NULL.");
		return FALSE;
	}

	if(this->p_streambuffer != NULL)
	{
		if(!HeapFree(p_processheap, 0u, this->p_streambuffer))
		{
			this->err_msg = TEXT("AudioPB::buffer_free: Error: failed to release heap memory.");
			return FALSE;
		}

		this->p_streambuffer = NULL;
	}

	if(this->p_inputbuffer != NULL)
	{
		if(!HeapFree(p_processheap, 0u, this->p_inputbuffer))
		{
			this->err_msg = TEXT("AudioPB::buffer_free: Error: failed to release heap memory.");
			return FALSE;
		}

		this->p_inputbuffer = NULL;
	}

	return TRUE;
}

VOID WINAPI AudioPB::playback_proc(VOID)
{
	this->playback_init();
	this->playback_loop();

	this->audiodev.p_audioclient->Stop();
	return;
}

VOID WINAPI AudioPB::playback_init(VOID)
{
	BYTE *p_audiobuffer = NULL;

	*((ULONG64*) &(this->filein_pos_64)) = this->AUDIO_DATA_BEGIN;

	this->p_delay->setDryInputAmplitude(1.0f);
	this->p_delay->setOutputAmplitude(1.0f);

	this->p_delay->resetFFParams();
	this->p_delay->resetFBParams();

	this->streambuffer_nseg_playout = 0u;
	this->delaybuffer_nseg = 0u;

	((IAudioRenderClient*) (this->audiodev.p_audioservice))->GetBuffer((UINT32) this->AUDIOBUFFER_SIZE_FRAMES, &p_audiobuffer);

	ZeroMemory(p_audiobuffer, this->AUDIOBUFFER_SIZE_BYTES);

	((IAudioRenderClient*) (this->audiodev.p_audioservice))->ReleaseBuffer((UINT32) this->AUDIOBUFFER_SIZE_FRAMES, 0u);

	this->audiodev.p_audioclient->Start();

	this->status = this->STATUS_RUNNING;

	this->audiodevice_wait();
	return;
}

VOID WINAPI AudioPB::playback_loop(VOID)
{
	while(TRUE)
	{
		if((this->status < 1) || (this->status == this->STATUS_STOPPED)) break;

		if(this->status == this->STATUS_PAUSED)
		{
			Sleep(1u);
			continue;
		}

		this->buffer_play();

		this->delaybuffer_loadin();
		this->p_delay->runDSP(this->delaybuffer_nseg);
		this->delaybuffer_loadout();

		this->streambuffer_nseg_playout_update();
		this->delaybuffer_nseg_update();

		this->audiodevice_wait();
	}

	return;
}

VOID WINAPI AudioPB::streambuffer_nseg_playout_update(VOID)
{
	this->streambuffer_nseg_playout++;
	this->streambuffer_nseg_playout %= this->STREAMBUFFER_N_SEGMENTS;

	return;
}

VOID WINAPI AudioPB::delaybuffer_nseg_update(VOID)
{
	this->delaybuffer_nseg++;
	this->delaybuffer_nseg %= this->AUDIODELAY_BUFFER_N_SEGMENTS;

	return;
}

VOID WINAPI AudioPB::buffer_play(VOID)
{
	VOID *p_out = NULL;
	BYTE *p_audiobuffer = NULL;
	HRESULT n_ret = 0;

	p_out = (VOID*) (((ULONG_PTR) (this->p_streambuffer)) + (this->streambuffer_nseg_playout)*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));

	n_ret = ((IAudioRenderClient*) (this->audiodev.p_audioservice))->GetBuffer((UINT32) this->STREAMBUFFER_SEGMENT_SIZE_FRAMES, &p_audiobuffer);
	if(n_ret != S_OK) app_exit((UINT) -1, TEXT("AudioPB::buffer_play: Error: IAudioRenderClient::GetBuffer failed."));

	CopyMemory(p_audiobuffer, p_out, this->STREAMBUFFER_SEGMENT_SIZE_BYTES);

	n_ret = ((IAudioRenderClient*) (this->audiodev.p_audioservice))->ReleaseBuffer((UINT32) this->STREAMBUFFER_SEGMENT_SIZE_FRAMES, 0u);
	if(n_ret != S_OK) app_exit((UINT) -1, TEXT("AudioPB::buffer_play: Error: IAudioRenderClient::ReleaseBuffer failed."));

	return;
}

VOID WINAPI AudioPB::audiodevice_wait(VOID)
{
	ULONG_PTR n_frames_free = 0u;
	UINT32 u32 = 0u;

	do{
		this->audiodev.p_audioclient->GetCurrentPadding(&u32);

		n_frames_free = this->AUDIOBUFFER_SIZE_FRAMES - ((ULONG_PTR) u32);

		Sleep(1u);
	}while(n_frames_free < this->STREAMBUFFER_SEGMENT_SIZE_FRAMES);

	return;
}
