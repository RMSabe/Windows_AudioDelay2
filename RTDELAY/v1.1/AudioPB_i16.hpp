/*
	Real-Time Audio Delay 2 application for Windows
	Version 1.1

	Author: Rafael Sabe
	Email: rafaelmsabe@gmail.com
*/

#ifndef AUDIOPB_I16_HPP
#define AUDIOPB_I16_HPP

#include "AudioPB.hpp"

class AudioPB_i16 : public AudioPB {
	public:
		AudioPB_i16(const audiopb_params_t *p_params);
		~AudioPB_i16(VOID);

	private:
		static constexpr FLOAT SAMPLE_FACTOR = 32768.0f;

		BOOL WINAPI audio_hw_init(VOID) override;
		BOOL WINAPI buffer_alloc(VOID) override;
		VOID WINAPI buffer_free(VOID) override;
		VOID WINAPI buffer_load_in(VOID) override;
		VOID WINAPI buffer_load_out(VOID) override;
};

#endif /*AUDIOPB_I16_HPP*/
