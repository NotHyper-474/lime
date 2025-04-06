#include "media/decoders/SDL_sound.h"
#include "SDL_sound.h"
#include <system/System.h>


namespace lime {


	bool SDL_sound::Decode (Resource *resource, AudioBuffer *audioBuffer) {
		Sound_Sample* sample = NULL;
		// The OGG decoder also returns 16-bit signed samples so why not?
		Sound_AudioInfo desired = {
			0,//AUDIO_S16LSB,
			0, // channels, blank for SDL_sound to determine
			0, // sample ratio
		};

		if (resource->path) {

			sample = Sound_NewSampleFromFile(resource->path, &desired, 65536);

		} else {

			// FIXME?: WAV files require ext to work due to a bug in SDL_sound
			sample = Sound_NewSampleFromMem(resource->data->b, resource->data->length, "wav", &desired, 65536);

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
				audioBuffer->dataFormat = 1;
				break;

			case AUDIO_F32LSB:
			case AUDIO_F32MSB:
				audioBuffer->bitsPerSample = 32;
				audioBuffer->dataFormat = 3;
				break;
			case AUDIO_S32LSB:
			case AUDIO_S32MSB:
				audioBuffer->bitsPerSample = 32;
				audioBuffer->dataFormat = 1;
				break;

			case AUDIO_U16LSB:
			case AUDIO_S16LSB:
			case AUDIO_U16MSB:
			case AUDIO_S16MSB:
			default:
				audioBuffer->bitsPerSample = 16;
				audioBuffer->dataFormat = 1;
				break;
		}

		// TODO: Add support for streaming sound in higher APIs

		// TODO: Do we care if duration can't be retrieved?
		Sint32 duration = Sound_GetDuration(sample);
		if (duration == -1)
		{
			LOG_SOUND("SDL_sound: Error getting duration: %s\n", Sound_GetError());
			Sound_FreeSample(sample);
			return false;
		}
		// TODO: This seems a few bits short?
		//Uint32 dataLength = (duration * audioBuffer->sampleRate * audioBuffer->channels * (audioBuffer->bitsPerSample / 8)) / 1000;

		Uint8* bytes = NULL;
		Uint32 bytesWritten = 0;

		do
		{
			Uint32 decodedBytes = Sound_Decode(sample);

			if ((sample->flags & SOUND_SAMPLEFLAG_EAGAIN))
			{
				continue;
			}

			if ((sample->flags & SOUND_SAMPLEFLAG_ERROR))
			{
				LOG_SOUND("SDL_sound Error: %s\n", Sound_GetError());
				break;
			}

			if (decodedBytes > 0)
			{
				Uint32 copySize = decodedBytes;

				audioBuffer->data->Resize(bytesWritten + copySize);
				bytes = audioBuffer->data->buffer->b;

				memcpy(bytes + bytesWritten, sample->buffer, copySize);

				bytesWritten += copySize;
			}

		} while ((sample->flags & SOUND_SAMPLEFLAG_EOF) == 0);

		Sound_FreeSample(sample);
		return bytesWritten > 0;
	}
}