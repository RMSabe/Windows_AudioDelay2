/*
	Real-Time Audio Delay 2 application for Windows
	Version 3.0

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

		VOID WINAPI delaybuffer_loadin(VOID) override;
		VOID WINAPI delaybuffer_loadout(VOID) override;
};

#endif /*AUDIOPB_I16_HPP*/
