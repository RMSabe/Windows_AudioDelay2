/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOPB_I24_HPP
#define AUDIOPB_I24_HPP

#include "AudioPB.hpp"

class AudioPB_i24 : public AudioPB {
	public:
		AudioPB_i24(const audiopb_params_t *p_params);
		~AudioPB_i24(VOID);

	private:
		static constexpr FLOAT SAMPLE_FACTOR = 8388608.0f;

		SIZE_T BUFFERIN_SIZE_BYTES = 0u;

		BOOL WINAPI audio_hw_init(VOID) override;
		BOOL WINAPI buffer_alloc(VOID) override;
		VOID WINAPI buffer_free(VOID) override;
		VOID WINAPI buffer_load_in(VOID) override;
		VOID WINAPI buffer_load_out(VOID) override;
};

#endif /*AUDIOPB_I24_HPP*/
