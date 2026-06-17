/*
	Audio Delay 2 application for Windows
	Version 3.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioDelay.hpp"

AudioDelay::AudioDelay(const audiodelay_init_params_t *p_params)
{
	this->setInitParameters(p_params);
}

AudioDelay::~AudioDelay(VOID)
{
	this->deinitialize();
}

BOOL WINAPI AudioDelay::setInitParameters(const audiodelay_init_params_t *p_params)
{
	this->status = this->STATUS_UNINITIALIZED;

	if(p_params == NULL)
	{
		this->err_msg = TEXT("AudioDelay::setInitParameters: Error: init params object pointer is NULL.");
		return FALSE;
	}

	this->BUFFER_SIZE_FRAMES = _get_closest_power2_ceil(p_params->buffer_size_frames);
	this->BUFFER_N_SEGMENTS = _get_closest_power2_ceil(p_params->buffer_n_segments);
	this->N_CHANNELS = p_params->n_channels;
	this->P_FF_PARAMS_LENGTH = p_params->n_ff_delays;
	this->P_FB_PARAMS_LENGTH = p_params->n_fb_delays;

	return TRUE;
}

BOOL WINAPI AudioDelay::initialize(VOID)
{
	if(this->status == this->STATUS_INITIALIZED) return TRUE;

	this->status = this->STATUS_UNINITIALIZED;

	if(this->BUFFER_SIZE_FRAMES < this->BUFFER_SIZE_FRAMES_MIN)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioDelay::initialize: Error: invalid buffer size.");
		return FALSE;
	}

	if(this->BUFFER_N_SEGMENTS < this->BUFFER_N_SEGMENTS_MIN)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioDelay::initialize: Error: invalid buffer segment count.");
		return FALSE;
	}

	if(this->N_CHANNELS < this->N_CHANNELS_MIN)
	{
		this->status = this->STATUS_ERROR_INVALIDPARAMS;
		this->err_msg = TEXT("AudioDelay::initialize: Error: invalid number of channels.");
		return FALSE;
	}

	this->BUFFER_SIZE_SAMPLES = (this->BUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->BUFFER_SIZE_BYTES = (this->BUFFER_SIZE_SAMPLES)*sizeof(FLOAT);

	this->BUFFER_SEGMENT_SIZE_FRAMES = (this->BUFFER_SIZE_FRAMES)/(this->BUFFER_N_SEGMENTS);
	this->BUFFER_SEGMENT_SIZE_SAMPLES = (this->BUFFER_SEGMENT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->BUFFER_SEGMENT_SIZE_BYTES = (this->BUFFER_SEGMENT_SIZE_SAMPLES)*sizeof(FLOAT);

	this->P_FF_PARAMS_SIZE = (this->P_FF_PARAMS_LENGTH)*sizeof(audiodelay_fx_params_t);
	this->P_FB_PARAMS_SIZE = (this->P_FB_PARAMS_LENGTH)*sizeof(audiodelay_fx_params_t);

	if(!this->buffer_alloc())
	{
		this->status = this->STATUS_ERROR_MEMORY;
		return FALSE;
	}

	this->status = this->STATUS_INITIALIZED;
	return TRUE;
}

BOOL WINAPI AudioDelay::runDSP(ULONG_PTR n_segment)
{
	FLOAT *p_curr_seg_in = NULL;
	FLOAT *p_curr_seg_out = NULL;

	ULONG_PTR n_prevsample = 0u;
	ULONG_PTR n_currsample = 0u;
	ULONG_PTR n_channel = 0u;

	ULONG_PTR n_frame = 0u;
	ULONG_PTR n_fx = 0u;
	ULONG_PTR prev_buf_nframe = 0u;

	ULONG_PTR n_delay = 0u;
	FLOAT f_amp = 0.0f;

	if(this->status < 1) return FALSE;

	if(n_segment >= this->BUFFER_N_SEGMENTS)
	{
		this->err_msg = TEXT("AudioDelay::runDSP: Error: given segment index is out of bounds.");
		return FALSE;
	}

	p_curr_seg_in = (FLOAT*) (((ULONG_PTR) (this->p_bufferinput)) + n_segment*(this->BUFFER_SEGMENT_SIZE_BYTES));
	p_curr_seg_out = (FLOAT*) (((ULONG_PTR) (this->p_bufferoutput)) + n_segment*(this->BUFFER_SEGMENT_SIZE_BYTES));

	for(n_frame = 0u; n_frame < this->BUFFER_SEGMENT_SIZE_FRAMES; n_frame++)
	{
		n_currsample = n_frame*(this->N_CHANNELS);
		f_amp = this->dryinput_amp;

		for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
		{
			p_curr_seg_out[n_currsample] = f_amp*(p_curr_seg_in[n_currsample]);
			n_currsample++;
		}

		n_fx = 0u;
		while(n_fx < this->P_FF_PARAMS_LENGTH)
		{
			n_delay = (ULONG_PTR) this->p_ff_params[n_fx].delay;
			f_amp = this->p_ff_params[n_fx].amp;

			if(f_amp == 0.0f)
			{
				n_fx++;
				continue;
			}

			this->retrieve_prev_nframe(n_segment, n_frame, n_delay, &prev_buf_nframe, NULL, NULL);

			n_currsample = n_frame*(this->N_CHANNELS);
			n_prevsample = prev_buf_nframe*(this->N_CHANNELS);

			for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
			{
				p_curr_seg_out[n_currsample] += f_amp*(this->p_bufferinput[n_prevsample]);
				n_currsample++;
				n_prevsample++;
			}

			n_fx++;
		}

		n_fx = 0u;
		while(n_fx < this->P_FB_PARAMS_LENGTH)
		{
			n_delay = (ULONG_PTR) this->p_fb_params[n_fx].delay;
			f_amp = this->p_fb_params[n_fx].amp;

			if(f_amp == 0.0f)
			{
				n_fx++;
				continue;
			}

			this->retrieve_prev_nframe(n_segment, n_frame, n_delay, &prev_buf_nframe, NULL, NULL);

			n_currsample = n_frame*(this->N_CHANNELS);
			n_prevsample = prev_buf_nframe*(this->N_CHANNELS);

			for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
			{
				p_curr_seg_out[n_currsample] += f_amp*(this->p_bufferoutput[n_prevsample]);
				n_currsample++;
				n_prevsample++;
			}

			n_fx++;
		}

		n_currsample = n_frame*(this->N_CHANNELS);
		f_amp = this->output_amp;

		for(n_channel = 0u; n_channel < this->N_CHANNELS; n_channel++)
		{
			p_curr_seg_out[n_currsample] *= f_amp;
			n_currsample++;
		}
	}

	return TRUE;
}

FLOAT* WINAPI AudioDelay::getInputBuffer(VOID)
{
	if(this->status < 1) return NULL;

	return this->p_bufferinput;
}

FLOAT* WINAPI AudioDelay::getOutputBuffer(VOID)
{
	if(this->status < 1) return NULL;

	return this->p_bufferoutput;
}

FLOAT* WINAPI AudioDelay::getInputBufferSegment(ULONG_PTR n_segment)
{
	if(this->status < 1) return NULL;
	if(n_segment >= this->BUFFER_N_SEGMENTS)
	{
		this->err_msg = TEXT("AudioDelay::getInputBufferSegment: Error: given segment index is out of bounds.");
		return NULL;
	}

	return (FLOAT*) (((ULONG_PTR) (this->p_bufferinput)) + n_segment*(this->BUFFER_SEGMENT_SIZE_BYTES));
}

FLOAT* WINAPI AudioDelay::getOutputBufferSegment(ULONG_PTR n_segment)
{
	if(this->status < 1) return NULL;
	if(n_segment >= this->BUFFER_N_SEGMENTS)
	{
		this->err_msg = TEXT("AudioDelay::getOutputBufferSegment: Error: given segment index is out of bounds.");
		return NULL;
	}

	return (FLOAT*) (((ULONG_PTR) (this->p_bufferoutput)) + n_segment*(this->BUFFER_SEGMENT_SIZE_BYTES));
}

FLOAT WINAPI AudioDelay::getDryInputAmplitude(VOID)
{
	return this->dryinput_amp;
}

FLOAT WINAPI AudioDelay::getOutputAmplitude(VOID)
{
	return this->output_amp;
}

BOOL WINAPI AudioDelay::getFFParams(ULONG_PTR n_fx, audiodelay_fx_params_t *p_params)
{
	if(this->status < 1) return FALSE;

	if(n_fx >= this->P_FF_PARAMS_LENGTH)
	{
		this->err_msg = TEXT("AudioDelay::getFFParams: Error: given fx index is out of bounds.");
		return FALSE;
	}

	if(p_params == NULL)
	{
		this->err_msg = TEXT("AudioDelay::getFFParams: Error: given params object pointer is NULL.");
		return FALSE;
	}

	CopyMemory(p_params, &(this->p_ff_params[n_fx]), sizeof(audiodelay_fx_params_t));
	return TRUE;
}

BOOL WINAPI AudioDelay::getFBParams(ULONG_PTR n_fx, audiodelay_fx_params_t *p_params)
{
	if(this->status < 1) return FALSE;

	if(n_fx >= this->P_FB_PARAMS_LENGTH)
	{
		this->err_msg = TEXT("AudioDelay::getFBParams: Error: given fx index is out of bounds.");
		return FALSE;
	}

	if(p_params == NULL)
	{
		this->err_msg = TEXT("AudioDelay::getFBParams: Error: given params object pointer is NULL.");
		return FALSE;
	}

	CopyMemory(p_params, &(this->p_fb_params[n_fx]), sizeof(audiodelay_fx_params_t));
	return TRUE;
}

BOOL WINAPI AudioDelay::setDryInputAmplitude(FLOAT amp)
{
	if(this->status < 1) return FALSE;

	this->dryinput_amp = amp;
	return TRUE;
}

BOOL WINAPI AudioDelay::setOutputAmplitude(FLOAT amp)
{
	if(this->status < 1) return FALSE;

	this->output_amp = amp;
	return TRUE;
}

BOOL WINAPI AudioDelay::setFFDelay(ULONG_PTR n_fx, ULONG_PTR delay)
{
	if(this->status < 1) return FALSE;

	if(n_fx >= this->P_FF_PARAMS_LENGTH)
	{
		this->err_msg = TEXT("AudioDelay::setFFDelay: Error: given fx index is out of bounds.");
		return FALSE;
	}

	if(delay >= this->BUFFER_SIZE_FRAMES)
	{
		this->err_msg = TEXT("AudioDelay::setFFDelay: Error: given delay time value is too big.");
		return FALSE;
	}

	this->p_ff_params[n_fx].delay = (UINT32) delay;
	return TRUE;
}

BOOL WINAPI AudioDelay::setFFAmplitude(ULONG_PTR n_fx, FLOAT amp)
{
	if(this->status < 1) return FALSE;

	if(n_fx >= this->P_FF_PARAMS_LENGTH)
	{
		this->err_msg = TEXT("AudioDelay::setFFAmplitude: Error: given fx index is out of bounds.");
		return FALSE;
	}

	this->p_ff_params[n_fx].amp = amp;
	return TRUE;
}

BOOL WINAPI AudioDelay::setFBDelay(ULONG_PTR n_fx, ULONG_PTR delay)
{
	if(this->status < 1) return FALSE;

	if(n_fx >= this->P_FB_PARAMS_LENGTH)
	{
		this->err_msg = TEXT("AudioDelay::setFBDelay: Error: given fx index is out of bounds.");
		return FALSE;
	}

	if(delay >= this->BUFFER_SIZE_FRAMES)
	{
		this->err_msg = TEXT("AudioDelay::setFBDelay: Error: given delay time value is too big.");
		return FALSE;
	}

	this->p_fb_params[n_fx].delay = (UINT32) delay;
	return TRUE;
}

BOOL WINAPI AudioDelay::setFBAmplitude(ULONG_PTR n_fx, FLOAT amp)
{
	if(this->status < 1) return FALSE;

	if(n_fx >= this->P_FB_PARAMS_LENGTH)
	{
		this->err_msg = TEXT("AudioDelay::setFBAmplitude: Error: given fx index is out of bounds.");
		return FALSE;
	}

	this->p_fb_params[n_fx].amp = amp;
	return TRUE;
}

BOOL WINAPI AudioDelay::resetFFParams(VOID)
{
	if(this->status < 1) return FALSE;

	if(this->P_FF_PARAMS_LENGTH) ZeroMemory(this->p_ff_params, this->P_FF_PARAMS_SIZE);

	return TRUE;
}

BOOL WINAPI AudioDelay::resetFBParams(VOID)
{
	if(this->status < 1) return FALSE;

	if(this->P_FB_PARAMS_LENGTH) ZeroMemory(this->p_fb_params, this->P_FB_PARAMS_SIZE);

	return TRUE;
}

INT WINAPI AudioDelay::getStatus(VOID)
{
	return this->status;
}

__string WINAPI AudioDelay::getLastErrorMessage(VOID)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return TEXT("AudioDelay object not initialized\r\nExtended error message: ") + this->err_msg;

	return this->err_msg;
}

VOID WINAPI AudioDelay::deinitialize(VOID)
{
	this->status = this->STATUS_UNINITIALIZED;
	this->buffer_free();

	return;
}

BOOL WINAPI AudioDelay::buffer_alloc(VOID)
{
	/*Clear any previous allocations*/
	if(!this->buffer_free()) return FALSE;

	if(!this->buffer_fxparams_alloc()) return FALSE;

	if(p_processheap == NULL)
	{
		this->err_msg = TEXT("AudioDelay::buffer_alloc: Error: p_processheap is NULL.");
		return FALSE;
	}

	this->p_bufferinput = (FLOAT*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BUFFER_SIZE_BYTES);
	this->p_bufferoutput = (FLOAT*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BUFFER_SIZE_BYTES);

	if(this->p_bufferinput == NULL)
	{
		this->buffer_free();
		this->err_msg = TEXT("AudioDelay::buffer_alloc: Error: failed to allocate heap memory.");
		return FALSE;
	}

	if(this->p_bufferoutput == NULL)
	{
		this->buffer_free();
		this->err_msg = TEXT("AudioDelay::buffer_alloc: Error: failed to allocate heap memory.");
		return FALSE;
	}

	return TRUE;
}

BOOL WINAPI AudioDelay::buffer_free(VOID)
{
	if(p_processheap == NULL)
	{
		this->err_msg = TEXT("AudioDelay::buffer_free: Error: p_processheap is NULL.");
		return FALSE;
	}

	if(this->p_bufferinput != NULL)
	{
		if(!HeapFree(p_processheap, 0u, this->p_bufferinput))
		{
			this->err_msg = TEXT("AudioDelay::buffer_free: Error: failed to release heap memory.");
			return FALSE;
		}

		this->p_bufferinput = NULL;
	}

	if(this->p_bufferoutput != NULL)
	{
		if(!HeapFree(p_processheap, 0u, this->p_bufferoutput))
		{
			this->err_msg = TEXT("AudioDelay::buffer_free: Error: failed to release heap memory.");
			return FALSE;
		}

		this->p_bufferoutput = NULL;
	}

	return this->buffer_fxparams_free();
}

BOOL WINAPI AudioDelay::buffer_fxparams_alloc(VOID)
{
	/*Clear any previous allocations*/
	if(!this->buffer_fxparams_free()) return FALSE;

	if(p_processheap == NULL)
	{
		this->err_msg = TEXT("AudioDelay::buffer_fxparams_alloc: Error: p_processheap is NULL.");
		return FALSE;
	}

	if(this->P_FF_PARAMS_LENGTH)
	{
		this->p_ff_params = (audiodelay_fx_params_t*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->P_FF_PARAMS_SIZE);
		if(this->p_ff_params == NULL)
		{
			this->buffer_fxparams_free();
			this->err_msg = TEXT("AudioDelay::buffer_fxparams_alloc: Error: failed to allocate heap memory.");
			return FALSE;
		}
	}

	if(this->P_FB_PARAMS_LENGTH)
	{
		this->p_fb_params = (audiodelay_fx_params_t*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->P_FB_PARAMS_SIZE);
		if(this->p_fb_params == NULL)
		{
			this->buffer_fxparams_free();
			this->err_msg = TEXT("AudioDelay::buffer_fxparams_alloc: Error: failed to allocate heap memory.");
			return FALSE;
		}
	}

	return TRUE;
}

BOOL WINAPI AudioDelay::buffer_fxparams_free(VOID)
{
	if(p_processheap == NULL)
	{
		this->err_msg = TEXT("AudioDelay::buffer_fxparams_free: Error: p_processheap is NULL.");
		return FALSE;
	}

	if(this->p_ff_params != NULL)
	{
		if(!HeapFree(p_processheap, 0u, this->p_ff_params))
		{
			this->err_msg = TEXT("AudioDelay::buffer_fxparams_free: Error: failed to release heap memory.");
			return FALSE;
		}

		this->p_ff_params = NULL;
	}

	if(this->p_fb_params != NULL)
	{
		if(!HeapFree(p_processheap, 0u, this->p_fb_params))
		{
			this->err_msg = TEXT("AudioDelay::buffer_fxparams_free: Error: failed to release heap memory.");
			return FALSE;
		}

		this->p_fb_params = NULL;
	}

	return TRUE;
}

BOOL WINAPI AudioDelay::retrieve_prev_nframe(ULONG_PTR curr_buf_nframe, ULONG_PTR n_delay, ULONG_PTR *p_prev_buf_nframe, ULONG_PTR *p_prev_nseg, ULONG_PTR *p_prev_seg_nframe)
{
	const ULONG_PTR _BUFFER_SIZE_BITMASK = (this->BUFFER_SIZE_FRAMES - 1u);
	ULONG_PTR prev_buf_nframe = 0u;
	ULONG_PTR prev_nseg = 0u;
	ULONG_PTR prev_seg_nframe = 0u;

	if(curr_buf_nframe >= this->BUFFER_SIZE_FRAMES) return FALSE;
	if(n_delay >= this->BUFFER_SIZE_FRAMES) return FALSE;

	prev_buf_nframe = ((curr_buf_nframe - n_delay) & _BUFFER_SIZE_BITMASK); /*Safe to use as long as BUFFER_SIZE_FRAMES is a power of 2.*/

	/*if(n_delay > curr_buf_nframe) prev_buf_nframe = this->BUFFER_SIZE_FRAMES - (n_delay - curr_buf_nframe);
	else prev_buf_nframe = curr_buf_nframe - n_delay;*/

	prev_nseg = prev_buf_nframe/(this->BUFFER_SEGMENT_SIZE_FRAMES);
	prev_seg_nframe = prev_buf_nframe%(this->BUFFER_SEGMENT_SIZE_FRAMES);

	if(p_prev_buf_nframe != NULL) *p_prev_buf_nframe = prev_buf_nframe;
	if(p_prev_nseg != NULL) *p_prev_nseg = prev_nseg;
	if(p_prev_seg_nframe != NULL) *p_prev_seg_nframe = prev_seg_nframe;

	return TRUE;
}

BOOL WINAPI AudioDelay::retrieve_prev_nframe(ULONG_PTR curr_nseg, ULONG_PTR curr_seg_nframe, ULONG_PTR n_delay, ULONG_PTR *p_prev_buf_nframe, ULONG_PTR *p_prev_nseg, ULONG_PTR *p_prev_seg_nframe)
{
	ULONG_PTR curr_buf_nframe = 0u;

	if(curr_nseg >= this->BUFFER_N_SEGMENTS) return FALSE;
	if(curr_seg_nframe >= this->BUFFER_SEGMENT_SIZE_FRAMES) return FALSE;

	curr_buf_nframe = curr_nseg*(this->BUFFER_SEGMENT_SIZE_FRAMES) + curr_seg_nframe;

	return this->retrieve_prev_nframe(curr_buf_nframe, n_delay, p_prev_buf_nframe, p_prev_nseg, p_prev_seg_nframe);
}
