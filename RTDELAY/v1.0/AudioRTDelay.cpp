/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.0

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#include "AudioRTDelay.hpp"

AudioRTDelay::AudioRTDelay(const audiortdelay_init_params_t *p_params)
{
	this->setInitParameters(p_params);
}

AudioRTDelay::~AudioRTDelay(VOID)
{
	this->status = this->STATUS_UNINITIALIZED;
	this->buffer_free();
}

BOOL WINAPI AudioRTDelay::setInitParameters(const audiortdelay_init_params_t *p_params)
{
	this->status = this->STATUS_UNINITIALIZED;

	if(p_params == NULL)
	{
		this->err_msg = TEXT("AudioRTDelay::setInitParameters: Error: init params object pointer is NULL.");
		return FALSE;
	}

	this->BUFFER_SIZE_FRAMES = p_params->buffer_size_frames;
	this->BUFFER_N_SEGMENTS = p_params->buffer_n_segments;
	this->N_CHANNELS = p_params->n_channels;
	this->P_FF_PARAMS_LENGTH = p_params->n_ff_delays;
	this->P_FB_PARAMS_LENGTH = p_params->n_fb_delays;

	return TRUE;
}

BOOL WINAPI AudioRTDelay::initialize(VOID)
{
	if(this->status == this->STATUS_INITIALIZED) return TRUE;

	this->status = this->STATUS_UNINITIALIZED;

	if(this->BUFFER_SIZE_FRAMES < this->BUFFER_SIZE_FRAMES_MIN)
	{
		this->err_msg = TEXT("AudioRTDelay::initialize: Error: BUFFER SIZE FRAMES value is invalid.");
		return FALSE;
	}

	if(this->BUFFER_N_SEGMENTS < this->BUFFER_N_SEGMENTS_MIN)
	{
		this->err_msg = TEXT("AudioRTDelay::initialize: Error: BUFFER N SEGMENTS value is invalid.");
		return FALSE;
	}

	if(this->N_CHANNELS < this->N_CHANNELS_MIN)
	{
		this->err_msg = TEXT("AudioRTDelay::initialize: Error: N CHANNELS value is invalid.");
		return FALSE;
	}

	this->BUFFER_SIZE_SAMPLES = (this->BUFFER_SIZE_FRAMES)*(this->N_CHANNELS);
	this->BUFFER_SIZE_BYTES = this->BUFFER_SIZE_SAMPLES*sizeof(FLOAT);

	this->BUFFER_SEGMENT_SIZE_FRAMES = this->BUFFER_SIZE_FRAMES/this->BUFFER_N_SEGMENTS;
	this->BUFFER_SEGMENT_SIZE_SAMPLES = (this->BUFFER_SEGMENT_SIZE_FRAMES)*(this->N_CHANNELS);
	this->BUFFER_SEGMENT_SIZE_BYTES = this->BUFFER_SEGMENT_SIZE_SAMPLES*sizeof(FLOAT);

	this->P_FF_PARAMS_SIZE = this->P_FF_PARAMS_LENGTH*sizeof(audiortdelay_fx_params_t);
	this->P_FB_PARAMS_SIZE = this->P_FB_PARAMS_LENGTH*sizeof(audiortdelay_fx_params_t);

	if(!this->buffer_alloc())
	{
		this->status = this->STATUS_ERROR_MEMALLOC;
		this->err_msg = TEXT("AudioRTDelay::initialize: Error: memory allocate failed.");
		return FALSE;
	}

	this->status = this->STATUS_INITIALIZED;
	return TRUE;
}

BOOL WINAPI AudioRTDelay::runDSP(SIZE_T n_segment)
{
	FLOAT *p_curr_seg_in = NULL;
	FLOAT *p_curr_seg_out = NULL;

	SIZE_T n_prevsample = 0u;
	SIZE_T n_currsample = 0u;
	SIZE_T n_channel = 0u;

	SIZE_T n_frame = 0u;
	SIZE_T n_fx = 0u;
	SIZE_T prev_buf_nframe = 0u;

	SIZE_T n_delay = 0u;
	FLOAT f_amp = 0.0f;

	if(this->status < 1) return FALSE;

	if(n_segment >= this->BUFFER_N_SEGMENTS)
	{
		this->err_msg = TEXT("AudioRTDelay::runDSP: Error: given segment index is out of bounds.");
		return FALSE;
	}

	p_curr_seg_in = this->pp_bufferinput_segments[n_segment];
	p_curr_seg_out = this->pp_bufferoutput_segments[n_segment];

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
			n_delay = (SIZE_T) this->p_ff_params[n_fx].delay;
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
			n_delay = (SIZE_T) this->p_fb_params[n_fx].delay;
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

FLOAT* WINAPI AudioRTDelay::getInputBuffer(VOID)
{
	if(this->status < 1) return NULL;

	return this->p_bufferinput;
}

FLOAT* WINAPI AudioRTDelay::getOutputBuffer(VOID)
{
	if(this->status < 1) return NULL;

	return this->p_bufferoutput;
}

FLOAT* WINAPI AudioRTDelay::getInputBufferSegment(SIZE_T n_segment)
{
	if(this->status < 1) return NULL;
	if(n_segment >= this->BUFFER_N_SEGMENTS)
	{
		this->err_msg = TEXT("AudioRTDelay::getInputBufferSegment: Error: given segment index is out of bounds.");
		return NULL;
	}

	return this->pp_bufferinput_segments[n_segment];
}

FLOAT* WINAPI AudioRTDelay::getOutputBufferSegment(SIZE_T n_segment)
{
	if(this->status < 1) return NULL;
	if(n_segment >= this->BUFFER_N_SEGMENTS)
	{
		this->err_msg = TEXT("AudioRTDelay::getOutputBufferSegment: Error: given segment index is out of bounds.");
		return NULL;
	}

	return this->pp_bufferoutput_segments[n_segment];
}

FLOAT WINAPI AudioRTDelay::getDryInputAmplitude(VOID)
{
	return this->dryinput_amp;
}

FLOAT WINAPI AudioRTDelay::getOutputAmplitude(VOID)
{
	return this->output_amp;
}

BOOL WINAPI AudioRTDelay::getFFParams(SIZE_T n_fx, audiortdelay_fx_params_t *p_params)
{
	if(this->status < 1) return FALSE;

	if(n_fx >= this->P_FF_PARAMS_LENGTH)
	{
		this->err_msg = TEXT("AudioRTDelay::getFFParams: Error: given fx index is out of bounds.");
		return FALSE;
	}

	if(p_params == NULL)
	{
		this->err_msg = TEXT("AudioRTDelay::getFFParams: Error: given params object pointer is NULL.");
		return FALSE;
	}

	CopyMemory(p_params, &(this->p_ff_params[n_fx]), sizeof(audiortdelay_fx_params_t));
	return TRUE;
}

BOOL WINAPI AudioRTDelay::getFBParams(SIZE_T n_fx, audiortdelay_fx_params_t *p_params)
{
	if(this->status < 1) return FALSE;

	if(n_fx >= this->P_FB_PARAMS_LENGTH)
	{
		this->err_msg = TEXT("AudioRTDelay::getFBParams: Error: given fx index is out of bounds.");
		return FALSE;
	}

	if(p_params == NULL)
	{
		this->err_msg = TEXT("AudioRTDelay::getFBParams: Error: given params object pointer is NULL.");
		return FALSE;
	}

	CopyMemory(p_params, &(this->p_fb_params[n_fx]), sizeof(audiortdelay_fx_params_t));
	return TRUE;
}

BOOL WINAPI AudioRTDelay::setDryInputAmplitude(FLOAT amp)
{
	if(this->status < 1) return FALSE;

	this->dryinput_amp = amp;
	return TRUE;
}

BOOL WINAPI AudioRTDelay::setOutputAmplitude(FLOAT amp)
{
	if(this->status < 1) return FALSE;

	this->output_amp = amp;
	return TRUE;
}

BOOL WINAPI AudioRTDelay::setFFDelay(SIZE_T n_fx, SIZE_T delay)
{
	if(this->status < 1) return FALSE;

	if(n_fx >= this->P_FF_PARAMS_LENGTH)
	{
		this->err_msg = TEXT("AudioRTDelay::setFFDelay: Error: given fx index is out of bounds.");
		return FALSE;
	}

	if(delay >= this->BUFFER_SIZE_FRAMES)
	{
		this->err_msg = TEXT("AudioRTDelay::setFFDelay: Error: given delay time value is too big.");
		return FALSE;
	}

	this->p_ff_params[n_fx].delay = (UINT32) delay;
	return TRUE;
}

BOOL WINAPI AudioRTDelay::setFFAmplitude(SIZE_T n_fx, FLOAT amp)
{
	if(this->status < 1) return FALSE;

	if(n_fx >= this->P_FF_PARAMS_LENGTH)
	{
		this->err_msg = TEXT("AudioRTDelay::setFFAmplitude: Error: given fx index is out of bounds.");
		return FALSE;
	}

	this->p_ff_params[n_fx].amp = amp;
	return TRUE;
}

BOOL WINAPI AudioRTDelay::setFBDelay(SIZE_T n_fx, SIZE_T delay)
{
	if(this->status < 1) return FALSE;

	if(n_fx >= this->P_FB_PARAMS_LENGTH)
	{
		this->err_msg = TEXT("AudioRTDelay::setFBDelay: Error: given fx index is out of bounds.");
		return FALSE;
	}

	if(delay >= this->BUFFER_SIZE_FRAMES)
	{
		this->err_msg = TEXT("AudioRTDelay::setFBDelay: Error: given delay time value is too big.");
		return FALSE;
	}

	this->p_fb_params[n_fx].delay = (UINT32) delay;
	return TRUE;
}

BOOL WINAPI AudioRTDelay::setFBAmplitude(SIZE_T n_fx, FLOAT amp)
{
	if(this->status < 1) return FALSE;

	if(n_fx >= this->P_FB_PARAMS_LENGTH)
	{
		this->err_msg = TEXT("AudioRTDelay::setFBAmplitude: Error: given fx index is out of bounds.");
		return FALSE;
	}

	this->p_fb_params[n_fx].amp = amp;
	return TRUE;
}

BOOL WINAPI AudioRTDelay::resetFFParams(VOID)
{
	if(this->status < 1) return FALSE;

	if(this->P_FF_PARAMS_LENGTH) ZeroMemory(this->p_ff_params, this->P_FF_PARAMS_SIZE);

	return TRUE;
}

BOOL WINAPI AudioRTDelay::resetFBParams(VOID)
{
	if(this->status < 1) return FALSE;

	if(this->P_FB_PARAMS_LENGTH) ZeroMemory(this->p_fb_params, this->P_FB_PARAMS_SIZE);

	return TRUE;
}

__string WINAPI AudioRTDelay::getLastErrorMessage(VOID)
{
	if(this->status == this->STATUS_UNINITIALIZED)
		return TEXT("AudioRTDelay object not initialized\r\nExtended error message: ") + this->err_msg;

	return this->err_msg;
}

BOOL WINAPI AudioRTDelay::buffer_alloc(VOID)
{
	SIZE_T n_seg = 0u;

	this->buffer_free(); /*Clear any previous allocations*/

	if(!this->buffer_fxparams_alloc()) return FALSE;

	this->p_bufferinput = (FLOAT*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BUFFER_SIZE_BYTES);
	this->p_bufferoutput = (FLOAT*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->BUFFER_SIZE_BYTES);

	this->pp_bufferinput_segments = (FLOAT**) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, (this->BUFFER_N_SEGMENTS*sizeof(FLOAT*)));
	this->pp_bufferoutput_segments = (FLOAT**) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, (this->BUFFER_N_SEGMENTS*sizeof(FLOAT*)));

	if(this->p_bufferinput == NULL)
	{
		this->buffer_free();
		return FALSE;
	}

	if(this->p_bufferoutput == NULL)
	{
		this->buffer_free();
		return FALSE;
	}

	if(this->pp_bufferinput_segments == NULL)
	{
		this->buffer_free();
		return FALSE;
	}

	if(this->pp_bufferoutput_segments == NULL)
	{
		this->buffer_free();
		return FALSE;
	}

	for(n_seg = 0u; n_seg < this->BUFFER_N_SEGMENTS; n_seg++)
	{
		this->pp_bufferinput_segments[n_seg] = (FLOAT*) (((SIZE_T) this->p_bufferinput) + n_seg*(this->BUFFER_SEGMENT_SIZE_BYTES));
		this->pp_bufferoutput_segments[n_seg] = (FLOAT*) (((SIZE_T) this->p_bufferoutput) + n_seg*(this->BUFFER_SEGMENT_SIZE_BYTES));
	}

	return TRUE;
}

VOID WINAPI AudioRTDelay::buffer_free(VOID)
{
	if(this->p_bufferinput != NULL)
	{
		HeapFree(p_processheap, 0u, this->p_bufferinput);
		this->p_bufferinput = NULL;
	}

	if(this->p_bufferoutput != NULL)
	{
		HeapFree(p_processheap, 0u, this->p_bufferoutput);
		this->p_bufferoutput = NULL;
	}

	if(this->pp_bufferinput_segments != NULL)
	{
		HeapFree(p_processheap, 0u, this->pp_bufferinput_segments);
		this->pp_bufferinput_segments = NULL;
	}

	if(this->pp_bufferoutput_segments != NULL)
	{
		HeapFree(p_processheap, 0u, this->pp_bufferoutput_segments);
		this->pp_bufferoutput_segments = NULL;
	}

	this->buffer_fxparams_free();
	return;
}

BOOL WINAPI AudioRTDelay::buffer_fxparams_alloc(VOID)
{
	this->buffer_fxparams_free(); /*Clear any previous allocations*/

	if(this->P_FF_PARAMS_LENGTH)
	{
		this->p_ff_params = (audiortdelay_fx_params_t*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->P_FF_PARAMS_SIZE);
		if(this->p_ff_params == NULL)
		{
			this->buffer_fxparams_free();
			return FALSE;
		}
	}

	if(this->P_FB_PARAMS_LENGTH)
	{
		this->p_fb_params = (audiortdelay_fx_params_t*) HeapAlloc(p_processheap, HEAP_ZERO_MEMORY, this->P_FB_PARAMS_SIZE);
		if(this->p_fb_params == NULL)
		{
			this->buffer_fxparams_free();
			return FALSE;
		}
	}

	return TRUE;
}

VOID WINAPI AudioRTDelay::buffer_fxparams_free(VOID)
{
	if(this->p_ff_params != NULL)
	{
		HeapFree(p_processheap, 0u, this->p_ff_params);
		this->p_ff_params = NULL;
	}

	if(this->p_fb_params != NULL)
	{
		HeapFree(p_processheap, 0u, this->p_fb_params);
		this->p_fb_params = NULL;
	}

	return;
}

BOOL WINAPI AudioRTDelay::retrieve_prev_nframe(SIZE_T curr_buf_nframe, SIZE_T n_delay, SIZE_T *p_prev_buf_nframe, SIZE_T *p_prev_nseg, SIZE_T *p_prev_seg_nframe)
{
	SIZE_T prev_buf_nframe = 0u;
	SIZE_T prev_nseg = 0u;
	SIZE_T prev_seg_nframe = 0u;

	if(curr_buf_nframe >= this->BUFFER_SIZE_FRAMES) return FALSE;
	if(n_delay >= this->BUFFER_SIZE_FRAMES) return FALSE;

	if(n_delay > curr_buf_nframe) prev_buf_nframe = this->BUFFER_SIZE_FRAMES - (n_delay - curr_buf_nframe);
	else prev_buf_nframe = curr_buf_nframe - n_delay;

	prev_nseg = prev_buf_nframe/this->BUFFER_SEGMENT_SIZE_FRAMES;
	prev_seg_nframe = prev_buf_nframe%this->BUFFER_SEGMENT_SIZE_FRAMES;

	if(p_prev_buf_nframe != NULL) *p_prev_buf_nframe = prev_buf_nframe;
	if(p_prev_nseg != NULL) *p_prev_nseg = prev_nseg;
	if(p_prev_seg_nframe != NULL) *p_prev_seg_nframe = prev_seg_nframe;

	return TRUE;
}

BOOL WINAPI AudioRTDelay::retrieve_prev_nframe(SIZE_T curr_nseg, SIZE_T curr_seg_nframe, SIZE_T n_delay, SIZE_T *p_prev_buf_nframe, SIZE_T *p_prev_nseg, SIZE_T *p_prev_seg_nframe)
{
	SIZE_T curr_buf_nframe = 0u;

	if(curr_nseg >= this->BUFFER_N_SEGMENTS) return FALSE;
	if(curr_seg_nframe >= this->BUFFER_SEGMENT_SIZE_FRAMES) return FALSE;

	curr_buf_nframe = curr_nseg*(this->BUFFER_SEGMENT_SIZE_FRAMES) + curr_seg_nframe;

	return this->retrieve_prev_nframe(curr_buf_nframe, n_delay, p_prev_buf_nframe, p_prev_nseg, p_prev_seg_nframe);
}
