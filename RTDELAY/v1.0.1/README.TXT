Real-Time Audio Delay 2 for Windows
Version 1.0.1

This application supports .wav files, 16bit encoding. (Hopefully more formats will be added in future versions).

This audio effect is the same as the GNU-Linux_AudioDelay2 project (https://github.com/RMSabe/GNU-Linux_AudioDelay2). Same logic, same controls, pretty much the same code, but for Windows.

Unfortunately, I still didn't figure out completely how Windows MMDeviceAPI works, so there are still a lot of limitations to this application:

1. It will only access the playback audio device in exclusive mode.
(I tried using shared mode, but that doesn't use either WAVE or IEEE FLOAT streaming format, instead it uses another format that I don't know how works).
This means that if any other application is using the intended audio device, this application will not work.
If this application is running, no other application will have access to the audio device.

2. For now, it only plays 16bit audio, and probably will only play stereo audio format.

Latest Update: some bug fixes.

Author: Rafael Sabe
Email: rafaelmsabe@gmail.com
