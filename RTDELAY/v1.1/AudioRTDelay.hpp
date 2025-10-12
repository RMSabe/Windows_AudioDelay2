/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIORTDELAY_HPP
#define AUDIORTDELAY_HPP

#include "globldef.h"
#include "strdef.hpp"
#include "shared.hpp"

struct _audiortdelay_init_params {
	SIZE_T buffer_size_frames;
	SIZE_T buffer_n_segments;
	SIZE_T n_channels;
	SIZE_T n_ff_delays;
	SIZE_T n_fb_delays;
};

struct _audiortdelay_fx_params {
	UINT32 delay;
	FLOAT amp;
};

typedef struct _audiortdelay_init_params audiortdelay_init_params_t;
typedef struct _audiortdelay_fx_params audiortdelay_fx_params_t;

class AudioRTDelay {
	public:
		AudioRTDelay(const audiortdelay_init_params_t *p_params);
		~AudioRTDelay(VOID);

		BOOL WINAPI setInitParameters(const audiortdelay_init_params_t *p_params);
		BOOL WINAPI initialize(VOID);
		BOOL WINAPI runDSP(SIZE_T n_segment);

		FLOAT* WINAPI getInputBuffer(VOID);
		FLOAT* WINAPI getOutputBuffer(VOID);

		FLOAT* WINAPI getInputBufferSegment(SIZE_T n_segment);
		FLOAT* WINAPI getOutputBufferSegment(SIZE_T n_segment);

		FLOAT WINAPI getDryInputAmplitude(VOID);
		FLOAT WINAPI getOutputAmplitude(VOID);

		BOOL WINAPI getFFParams(SIZE_T n_fx, audiortdelay_fx_params_t *p_params);
		BOOL WINAPI getFBParams(SIZE_T n_fx, audiortdelay_fx_params_t *p_params);

		BOOL WINAPI setDryInputAmplitude(FLOAT amp);
		BOOL WINAPI setOutputAmplitude(FLOAT amp);

		BOOL WINAPI setFFDelay(SIZE_T n_fx, SIZE_T delay);
		BOOL WINAPI setFFAmplitude(SIZE_T n_fx, FLOAT amp);

		BOOL WINAPI setFBDelay(SIZE_T n_fx, SIZE_T delay);
		BOOL WINAPI setFBAmplitude(SIZE_T n_fx, FLOAT amp);

		BOOL WINAPI resetFFParams(VOID);
		BOOL WINAPI resetFBParams(VOID);

		__string WINAPI getLastErrorMessage(VOID);

		enum Status {
			STATUS_ERROR_MEMALLOC = -2,
			STATUS_ERROR_GENERIC = -1,
			STATUS_UNINITIALIZED = 0,
			STATUS_INITIALIZED = 1
		};

	private:
		static constexpr SIZE_T BUFFER_SIZE_FRAMES_MIN = 128u; /*Probably not a good idea having a buffer smaller than this anyway*/
		static constexpr SIZE_T BUFFER_N_SEGMENTS_MIN = 2u; /*Must have at least 2 segments*/
		static constexpr SIZE_T N_CHANNELS_MIN = 1u;

		SIZE_T BUFFER_SIZE_FRAMES = 0u;
		SIZE_T BUFFER_SIZE_SAMPLES = 0u;
		SIZE_T BUFFER_SIZE_BYTES = 0u;

		SIZE_T BUFFER_N_SEGMENTS = 0u;

		SIZE_T BUFFER_SEGMENT_SIZE_FRAMES = 0u;
		SIZE_T BUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		SIZE_T BUFFER_SEGMENT_SIZE_BYTES = 0u;

		SIZE_T N_CHANNELS = 0u;

		/*
			..._PARAMS_LENGTH = size (in number of elements)
			..._PARAMS_SIZE = size (in number of bytes)
		*/

		SIZE_T P_FF_PARAMS_LENGTH = 0u;
		SIZE_T P_FF_PARAMS_SIZE = 0u;

		SIZE_T P_FB_PARAMS_LENGTH = 0u;
		SIZE_T P_FB_PARAMS_SIZE = 0u;

		FLOAT *p_bufferinput = NULL;
		FLOAT *p_bufferoutput = NULL;

		FLOAT **pp_bufferinput_segments = NULL;
		FLOAT **pp_bufferoutput_segments = NULL;

		audiortdelay_fx_params_t *p_ff_params = NULL;
		audiortdelay_fx_params_t *p_fb_params = NULL;

		FLOAT dryinput_amp = 0.0f;
		FLOAT output_amp = 0.0f;

		__string err_msg = TEXT("");
		INT status = this->STATUS_UNINITIALIZED;

		BOOL WINAPI buffer_alloc(VOID);
		VOID WINAPI buffer_free(VOID);

		BOOL WINAPI buffer_fxparams_alloc(VOID);
		VOID WINAPI buffer_fxparams_free(VOID);

		/*
			retrieve_prev_nframe(): calculate the previous (delayed) frame index from the current frame index and the delay time value (number of frames).

			Naming:

			..._nframe: frame index in context.

			..._buf_nframe: frame index within the whole buffer.
			..._seg_nframe: frame index within a specific buffer segment
			..._nseg: buffer segment index in context.

			curr_...: current index.
			prev_...: previous (delayed) index.

			Inputs:

			retrieve_prev_nframe(curr_buf_nframe, n_delay, ...):
			curr_buf_nframe: the current buffer frame index.
			n_delay: number of frames to be delayed.

			retrieve_prev_nframe(curr_nseg, curr_seg_nframe, n_delay, ...):
			curr_nseg & curr_seg_nframe: the current segment index and frame index within that segment.
			n_delay: number of frames to be delayed.

			Outputs:

			p_prev_buf_nframe: pointer to variable that receives the delayed buffer frame index. Set to NULL if unused.
			p_prev_nseg: pointer to variable that receives the delayed segment index. Set to NULL if unused.
			p_prev_seg_nframe: pointer to variable that receives the delayed segment frame index. Set to NULL if unused.

			returns TRUE if successful, FALSE otherwise.
		*/

		BOOL WINAPI retrieve_prev_nframe(SIZE_T curr_buf_nframe, SIZE_T n_delay, SIZE_T *p_prev_buf_nframe, SIZE_T *p_prev_nseg, SIZE_T *p_prev_seg_nframe);
		BOOL WINAPI retrieve_prev_nframe(SIZE_T curr_nseg, SIZE_T curr_seg_nframe, SIZE_T n_delay, SIZE_T *p_prev_buf_nframe, SIZE_T *p_prev_nseg, SIZE_T *p_prev_seg_nframe);

};

#endif /*AUDIORTDELAY_HPP*/
