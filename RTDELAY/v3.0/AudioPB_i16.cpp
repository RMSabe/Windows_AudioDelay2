/*
	Real-Time Audio Delay 2 application for Windows
	Version 3.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioPB_i16.hpp"
#include <math.h>

AudioPB_i16::AudioPB_i16(const audiopb_params_t *p_params) : AudioPB(p_params)
{
	this->AUDIO_BYTES_PER_SAMPLE = 2u;
	this->FILE_BYTES_PER_SAMPLE = 2u;
	this->AUDIODATA_BITS_PER_SAMPLE = 16u;
}

AudioPB_i16::~AudioPB_i16(VOID)
{
	this->deinitialize();
}

VOID WINAPI AudioPB_i16::delaybuffer_loadin(VOID)
{
	ULONG_PTR n_sample = 0u;
	FLOAT *p_loadseg_f32 = NULL;
	INT16 *p_input = NULL;

	FLOAT factor = 0.0f;
	FLOAT f32 = 0.0f;

	DWORD dummy_32;

	if(*((ULONG64*) &(this->filein_pos_64)) >= this->AUDIO_DATA_END)
	{
		this->status = this->STATUS_STOPPED;
		return;
	}

	p_loadseg_f32 = this->p_delay->getInputBufferSegment(this->delaybuffer_nseg);
	if(p_loadseg_f32 == NULL)
	{
		this->err_msg = TEXT("AudioPB_i16::delaybuffer_loadin: Error: AudioDelay::getInputBufferSegment returned NULL.\r\nExtended Error Message: ") + this->p_delay->getLastErrorMessage();
		app_exit((UINT) -1, this->err_msg.c_str());
	}

	p_input = (INT16*) this->p_inputbuffer;

	ZeroMemory(p_input, this->INPUTBUFFER_SIZE_BYTES);

	SetFilePointer(this->h_filein, (LONG) this->filein_pos_64.l32, (LONG*) &(this->filein_pos_64.h32), FILE_BEGIN);
	ReadFile(this->h_filein, p_input, (DWORD) this->INPUTBUFFER_SIZE_BYTES, &dummy_32, NULL);
	*((ULONG64*) &(this->filein_pos_64)) += (ULONG64) this->INPUTBUFFER_SIZE_BYTES;

	factor = this->SAMPLE_FACTOR;

	for(n_sample = 0u; n_sample < this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		f32 = (FLOAT) p_input[n_sample];
		f32 /= factor;
		p_loadseg_f32[n_sample] = f32;
	}

	return;
}

VOID WINAPI AudioPB_i16::delaybuffer_loadout(VOID)
{
	ULONG_PTR n_sample = 0u;
	ULONG_PTR output_nseg = 0u;
	FLOAT *p_loadseg_f32 = NULL;
	INT16 *p_output = NULL;

	FLOAT factor = 0.0f;
	FLOAT f32 = 0.0f;

	p_loadseg_f32 = this->p_delay->getOutputBufferSegment(this->delaybuffer_nseg);
	if(p_loadseg_f32 == NULL)
	{
		this->err_msg = TEXT("AudioPB_i16::delaybuffer_loadout: Error: AudioDelay::getOutputBufferSegment returned NULL.\r\nExtended Error Message: ") + this->p_delay->getLastErrorMessage();
		app_exit((UINT) -1, this->err_msg.c_str());
	}

	output_nseg = (this->streambuffer_nseg_playout + 1u)%(this->STREAMBUFFER_N_SEGMENTS);
	p_output = (INT16*) (((ULONG_PTR) (this->p_streambuffer)) + output_nseg*(this->STREAMBUFFER_SEGMENT_SIZE_BYTES));

	factor = this->SAMPLE_FACTOR - 1.0f;

	for(n_sample = 0u; n_sample < this->STREAMBUFFER_SEGMENT_SIZE_SAMPLES; n_sample++)
	{
		f32 = p_loadseg_f32[n_sample];

		if(f32 > 1.0f) f32 = 1.0f;
		else if(f32 < -1.0f) f32 = -1.0f;

		f32 *= factor;

		p_output[n_sample] = (INT16) roundf(f32);
	}

	return;
}
