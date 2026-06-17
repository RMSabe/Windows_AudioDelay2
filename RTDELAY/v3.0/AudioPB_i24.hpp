/*
	Real-Time Audio Delay 2 application for Windows
	Version 3.0

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

		VOID WINAPI delaybuffer_loadin(VOID) override;
		VOID WINAPI delaybuffer_loadout(VOID) override;
};

#endif /*AUDIOPB_I24_HPP*/
