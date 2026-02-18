/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.2

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIORTDELAY_HPP
#define AUDIORTDELAY_HPP

#include "globldef.h"
#include "strdef.hpp"
#include "shared.hpp"

struct _audiortdelay_init_params {
	ULONG_PTR buffer_size_frames;
	ULONG_PTR buffer_n_segments;
	ULONG_PTR n_channels;
	ULONG_PTR n_ff_delays;
	ULONG_PTR n_fb_delays;
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
		BOOL WINAPI runDSP(ULONG_PTR n_segment);

		FLOAT* WINAPI getInputBuffer(VOID);
		FLOAT* WINAPI getOutputBuffer(VOID);

		FLOAT* WINAPI getInputBufferSegment(ULONG_PTR n_segment);
		FLOAT* WINAPI getOutputBufferSegment(ULONG_PTR n_segment);

		FLOAT WINAPI getDryInputAmplitude(VOID);
		FLOAT WINAPI getOutputAmplitude(VOID);

		BOOL WINAPI getFFParams(ULONG_PTR n_fx, audiortdelay_fx_params_t *p_params);
		BOOL WINAPI getFBParams(ULONG_PTR n_fx, audiortdelay_fx_params_t *p_params);

		BOOL WINAPI setDryInputAmplitude(FLOAT amp);
		BOOL WINAPI setOutputAmplitude(FLOAT amp);

		BOOL WINAPI setFFDelay(ULONG_PTR n_fx, ULONG_PTR delay);
		BOOL WINAPI setFFAmplitude(ULONG_PTR n_fx, FLOAT amp);

		BOOL WINAPI setFBDelay(ULONG_PTR n_fx, ULONG_PTR delay);
		BOOL WINAPI setFBAmplitude(ULONG_PTR n_fx, FLOAT amp);

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
		static constexpr ULONG_PTR BUFFER_SIZE_FRAMES_MIN = 128u; /*Probably not a good idea having a buffer smaller than this anyway*/
		static constexpr ULONG_PTR BUFFER_N_SEGMENTS_MIN = 2u; /*Must have at least 2 segments*/
		static constexpr ULONG_PTR N_CHANNELS_MIN = 1u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR BUFFER_SIZE_FRAMES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR BUFFER_SIZE_SAMPLES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR BUFFER_SIZE_BYTES = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR BUFFER_N_SEGMENTS = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR BUFFER_SEGMENT_SIZE_FRAMES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR BUFFER_SEGMENT_SIZE_SAMPLES = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR BUFFER_SEGMENT_SIZE_BYTES = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR N_CHANNELS = 0u;

		/*
			..._PARAMS_LENGTH = size (in number of elements)
			..._PARAMS_SIZE = size (in number of bytes)
		*/

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR P_FF_PARAMS_LENGTH = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR P_FF_PARAMS_SIZE = 0u;

		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR P_FB_PARAMS_LENGTH = 0u;
		__declspec(align(PTR_SIZE_BYTES)) ULONG_PTR P_FB_PARAMS_SIZE = 0u;

		__declspec(align(PTR_SIZE_BYTES)) FLOAT *p_bufferinput = NULL;
		__declspec(align(PTR_SIZE_BYTES)) FLOAT *p_bufferoutput = NULL;

		__declspec(align(PTR_SIZE_BYTES)) FLOAT **pp_bufferinput_segments = NULL;
		__declspec(align(PTR_SIZE_BYTES)) FLOAT **pp_bufferoutput_segments = NULL;

		__declspec(align(PTR_SIZE_BYTES)) audiortdelay_fx_params_t *p_ff_params = NULL;
		__declspec(align(PTR_SIZE_BYTES)) audiortdelay_fx_params_t *p_fb_params = NULL;

		__declspec(align(4)) FLOAT dryinput_amp = 0.0f;
		__declspec(align(4)) FLOAT output_amp = 0.0f;

		__declspec(align(PTR_SIZE_BYTES)) __string err_msg = TEXT("");
		__declspec(align(4)) INT status = this->STATUS_UNINITIALIZED;

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

		BOOL WINAPI retrieve_prev_nframe(ULONG_PTR curr_buf_nframe, ULONG_PTR n_delay, ULONG_PTR *p_prev_buf_nframe, ULONG_PTR *p_prev_nseg, ULONG_PTR *p_prev_seg_nframe);
		BOOL WINAPI retrieve_prev_nframe(ULONG_PTR curr_nseg, ULONG_PTR curr_seg_nframe, ULONG_PTR n_delay, ULONG_PTR *p_prev_buf_nframe, ULONG_PTR *p_prev_nseg, ULONG_PTR *p_prev_seg_nframe);

};

#endif /*AUDIORTDELAY_HPP*/
