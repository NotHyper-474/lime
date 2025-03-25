#include "media/decoders/SDL_sound.h"
#include "SDL_sound.h"
#include <system/System.h>


namespace lime {


	bool SDL_sound::Decode (Resource *resource, AudioBuffer *audioBuffer) {
		Sound_Sample* sample = NULL;
		// The OGG decoder also returns 16-bit signed samples so why not?
		Sound_AudioInfo desired = {
			AUDIO_S16LSB,
			0, // channels, blank for SDL_sound to determine
			0, // sample ratio
		};

		if (resource->path) {

			sample = Sound_NewSampleFromFile(resource->path, &desired, 65536);

		} else {

			sample = Sound_NewSampleFromMem(resource->data->b, resource->data->length, NULL, &desired, 65536);

		}

		if (!sample) {

			printf("%s\n", Sound_GetError());
			return false;

		}

		audioBuffer->sampleRate = (int)sample->desired.rate;
		audioBuffer->channels = (int)sample->desired.channels;

		switch (sample->desired.format)
		{
			case AUDIO_U8:
			case AUDIO_S8:
				audioBuffer->bitsPerSample = 8;
				break;

			case AUDIO_F32LSB:
			case AUDIO_F32MSB:
				audioBuffer->bitsPerSample = 32;
				break;

			case AUDIO_U16LSB:
			case AUDIO_S16LSB:
			case AUDIO_U16MSB:
			case AUDIO_S16MSB:
			default:
				audioBuffer->bitsPerSample = 16;
				break;
		}

		// TODO: Add support for streaming sound in higher APIs
		Sint32 duration = Sound_GetDuration(sample);
		if (duration == -1)
		{
			LOG_SOUND("SDL_sound: Error getting duration: %s\n", Sound_GetError());
			Sound_FreeSample(sample);
			return false;
		}
		// TODO: This seems a few bits short?
		Uint32 dataLength = (duration * audioBuffer->sampleRate * audioBuffer->channels * (audioBuffer->bitsPerSample / 8)) / 1000;

		Uint8* bytes = NULL;
		Uint32 bytesWritten = 0;
		Uint32 decodedBytes = 0;
		Uint8* decodedPtr = NULL;

		while (bytesWritten < dataLength) {

			if (((sample->flags & SOUND_SAMPLEFLAG_ERROR) == 0) && ((sample->flags & SOUND_SAMPLEFLAG_EOF) == 0)) {

                decodedBytes = Sound_Decode(sample);
                decodedPtr = (unsigned char*)sample->buffer;

            }

            if (decodedBytes == 0)
            {
                // memset(bytes + bytesWritten, '\0', dataLength - bytesWritten);  /* write silence. */
                break;
            }
			else
			{
				Uint32 copySize = decodedBytes;
				// int copySize = dataLength - bytesWritten;
				// if (copySize > decodedBytes) copySize = decodedBytes;

				audioBuffer->data->Resize (bytesWritten + copySize);
				bytes = audioBuffer->data->buffer->b;

				memcpy(bytes + bytesWritten, decodedPtr, copySize);

				bytesWritten += copySize;
				decodedPtr += copySize;
				decodedBytes -= copySize;
			}

		}

		Sound_FreeSample(sample);
		return decodedBytes != 0;
	}
}