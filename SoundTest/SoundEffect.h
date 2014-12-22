#pragma once
#include <Windows.h>
#include <mmreg.h>
#include <mmsystem.h>
#include <math.h>


class SoundEffect
{
public:
	SoundEffect(void);
	SoundEffect(const int noteInfo[], const int arraySize, int side); // 0 = left, 1 = right, 2 = both
	SoundEffect(const int noteInfo[], const int arraySize); // mono, for reference of how it used to be
	SoundEffect(SoundEffect& otherInstance);
	~SoundEffect();
	SoundEffect& operator=(SoundEffect& otherInstance);
	void Play();

private:
    HWAVEOUT m_waveOut; // Handle to sound card output
    WAVEFORMATEX m_waveFormat; // The sound format
    WAVEHDR m_waveHeader; // WAVE header for our sound data
    HANDLE m_done; // Event Handle that tells us the sound has finished being played.
                   // This is a very efficient way to put the program to sleep
                   // while the sound card is processing the sound buffer

    char* m_data; // Sound data buffer
    int m_bufferSize; // Size of sound data buffer
	void interleaveData(char data[], int dataSize, int side);
};

