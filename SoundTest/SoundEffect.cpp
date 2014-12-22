#include "stdafx.h"

#define HALF_NOTE 1.059463094359 // HALF_NOTE ^ 12 = 2
#define PI 3.14159265358979

#include <iostream>
#include "SoundEffect.h"

using namespace std;

SoundEffect::SoundEffect() {
        m_data = NULL;
}

SoundEffect::SoundEffect(const int noteInfo[], const int arraySize, int side) {
        // Initialize the sound format we will request from sound card
        m_waveFormat.wFormatTag = WAVE_FORMAT_PCM;     // Uncompressed sound format
        m_waveFormat.nChannels = 2;                    // 1 = Mono, 2 = Stereo
        m_waveFormat.wBitsPerSample = 8;               // Bits per sample per channel
        m_waveFormat.nSamplesPerSec = 11025;           // Sample Per Second
        m_waveFormat.nBlockAlign = m_waveFormat.nChannels * m_waveFormat.wBitsPerSample / 8;
        m_waveFormat.nAvgBytesPerSec = m_waveFormat.nSamplesPerSec * m_waveFormat.nBlockAlign;
        m_waveFormat.cbSize = 0;

        int dataLength = 0, moment = (m_waveFormat.nSamplesPerSec / 75);
        double period = 2.0 * PI / (double) m_waveFormat.nSamplesPerSec;

        // Calculate how long we need the sound buffer to be
        for (int i = 1; i < arraySize; i += 2)
            dataLength += (noteInfo[i] != 0) ? noteInfo[i] * moment : moment;

        // Allocate the array
		char* data = new char[dataLength];

        int placeInData = 0;

        // Make the sound buffer
        for (int i = 0; i < arraySize; i += 2)
        {
            int relativePlaceInData = placeInData;

            while ((relativePlaceInData - placeInData) < ((noteInfo[i + 1] != 0) ? noteInfo[i + 1] * moment : moment))
            {
                // Generate the sound wave (as a sinusoid)
                // - x will have a range of -1 to +1
                double x = sin((relativePlaceInData - placeInData) * 55 * pow(HALF_NOTE, noteInfo[i]) * period);

                // Scale x to a range of 0-255 (signed char) for 8 bit sound reproduction
                data[relativePlaceInData] = (char) (127 * x + 128);

                relativePlaceInData++;
            }

            placeInData = relativePlaceInData;
        }

		// now interleave
		interleaveData(data, dataLength, side);
		delete[] data;
}

void SoundEffect::interleaveData(char data[], int dataLength, int side) {
	m_bufferSize = dataLength * m_waveFormat.nChannels;
	m_data = new char[m_bufferSize];

	for(int i = 0; i < dataLength; i++) {
		for(int j = 0; j < m_waveFormat.nChannels; j++) {
			if (j == side)
				m_data[j + (i*m_waveFormat.nChannels)] = data[i];
			else
				m_data[j + (i*m_waveFormat.nChannels)] = (char)128;
		}
	}

}


/**
 * mono version
 */
SoundEffect::SoundEffect(const int noteInfo[], const int arraySize) {
        // Initialize the sound format we will request from sound card
        m_waveFormat.wFormatTag = WAVE_FORMAT_PCM;     // Uncompressed sound format
        m_waveFormat.nChannels = 1;                    // 1 = Mono, 2 = Stereo
        m_waveFormat.wBitsPerSample = 8;               // Bits per sample per channel
        m_waveFormat.nSamplesPerSec = 11025;           // Sample Per Second
        m_waveFormat.nBlockAlign = m_waveFormat.nChannels * m_waveFormat.wBitsPerSample / 8;
        m_waveFormat.nAvgBytesPerSec = m_waveFormat.nSamplesPerSec * m_waveFormat.nBlockAlign;
        m_waveFormat.cbSize = 0;

        int dataLength = 0, moment = (m_waveFormat.nSamplesPerSec / 75);
        double period = 2.0 * PI / (double) m_waveFormat.nSamplesPerSec;

        // Calculate how long we need the sound buffer to be
        for (int i = 1; i < arraySize; i += 2)
            dataLength += (noteInfo[i] != 0) ? noteInfo[i] * moment : moment;

        // Allocate the array
        m_data = new char[m_bufferSize = dataLength];

        int placeInData = 0;

        // Make the sound buffer
        for (int i = 0; i < arraySize; i += 2)
        {
            int relativePlaceInData = placeInData;

            while ((relativePlaceInData - placeInData) < ((noteInfo[i + 1] != 0) ? noteInfo[i + 1] * moment : moment))
            {
                // Generate the sound wave (as a sinusoid)
                // - x will have a range of -1 to +1
                double x = sin((relativePlaceInData - placeInData) * 55 * pow(HALF_NOTE, noteInfo[i]) * period);

                // Scale x to a range of 0-255 (signed char) for 8 bit sound reproduction
                m_data[relativePlaceInData] = (char) (127 * x + 128);

                relativePlaceInData++;
            }

            placeInData = relativePlaceInData;
        }
}

SoundEffect::SoundEffect(SoundEffect& otherInstance) {
        m_bufferSize = otherInstance.m_bufferSize;
        m_waveFormat = otherInstance.m_waveFormat;

        if (m_bufferSize > 0)
        {
            m_data = new char[m_bufferSize];

            for (int i = 0; i < otherInstance.m_bufferSize; i++)
                m_data[i] = otherInstance.m_data[i];
        }
}

SoundEffect::~SoundEffect() {
        if (m_bufferSize > 0)
            delete [] m_data;
}

SoundEffect& SoundEffect::operator=(SoundEffect& otherInstance) {
        if (m_bufferSize > 0)
            delete [] m_data;

        m_bufferSize = otherInstance.m_bufferSize;
        m_waveFormat = otherInstance.m_waveFormat;

        if (m_bufferSize > 0)
        {
            m_data = new char[m_bufferSize];

            for (int i = 0; i < otherInstance.m_bufferSize; i++)
                m_data[i] = otherInstance.m_data[i];
        }

        return *this;
}

void SoundEffect::Play() {
        // Create our "Sound is Done" event
        m_done = CreateEvent (0, FALSE, FALSE, 0);

        // Open the audio device
        if (waveOutOpen(&m_waveOut, 0, &m_waveFormat, (DWORD) m_done, 0, CALLBACK_EVENT) != MMSYSERR_NOERROR) 
        {
            cout << "Sound card cannot be opened." << endl;
            return;
        }

        // Create the wave header for our sound buffer
        m_waveHeader.lpData = m_data;
        m_waveHeader.dwBufferLength = m_bufferSize;
        m_waveHeader.dwFlags = 0;
        m_waveHeader.dwLoops = 0;

        // Prepare the header for playback on sound card
        if (waveOutPrepareHeader(m_waveOut, &m_waveHeader, sizeof(m_waveHeader)) != MMSYSERR_NOERROR)
        {
            cout << "Error preparing Header!" << endl;
            return;
        }

        // Play the sound!
        ResetEvent(m_done); // Reset our Event so it is non-signaled, it will be signaled again with buffer finished

        if (waveOutWrite(m_waveOut, &m_waveHeader, sizeof(m_waveHeader)) != MMSYSERR_NOERROR)
        {
            cout << "Error writing to sound card!" << endl;
            return;
        }

        // Wait until sound finishes playing
        if (WaitForSingleObject(m_done, INFINITE) != WAIT_OBJECT_0)
        {
            cout << "Error waiting for sound to finish" << endl;
            return;
        }

        // Unprepare our wav header
        if (waveOutUnprepareHeader(m_waveOut, &m_waveHeader,sizeof(m_waveHeader)) != MMSYSERR_NOERROR)
        {
            cout << "Error unpreparing header!" << endl;
            return;
        }

        // Close the wav device
        if (waveOutClose(m_waveOut) != MMSYSERR_NOERROR)
        {
            cout << "Sound card cannot be closed!" << endl;
            return;
        }

        // Release our event handle
        CloseHandle(m_done);
}

