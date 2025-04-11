#include "media/decoders/SDL_sound.h"
#include <system/System.h>

#if LIME_SDL3_SOUND

#include "SDL3_sound/SDL_sound.h"

#define AUDIO_U8 SDL_AUDIO_U8
#define AUDIO_S8 SDL_AUDIO_S8
#define AUDIO_F32LSB SDL_AUDIO_F32LE
#define AUDIO_F32MSB SDL_AUDIO_F32BE
#define AUDIO_S32LSB SDL_AUDIO_S32LE
#define AUDIO_S32MSB SDL_AUDIO_S32BE
#define AUDIO_S16LSB SDL_AUDIO_S16LE
#define AUDIO_S16MSB SDL_AUDIO_S16BE

#else
#include "SDL_sound.h"
#endif

namespace lime {


	bool SDL_sound::Decode (Resource *resource, AudioBuffer *audioBuffer) {
		Sound_Sample* sample = NULL;

		if (resource->path) {

			sample = Sound_NewSampleFromFile(resource->path, NULL, 65536);

		} else {

			// FIXME?: WAV files require ext to work due to a bug in SDL_sound
			sample = Sound_NewSampleFromMem(resource->data->b, resource->data->length, "wav", NULL, 65536);

		}

		if (!sample) {

			LOG_SOUND("SDL_sound: %s\n", Sound_GetError());
			return false;

		}

		audioBuffer->sampleRate = (int)sample->desired.rate;
		audioBuffer->channels = (int)sample->desired.channels;
		audioBuffer->dataFormat = 1;

		switch (sample->desired.format)
		{
			case AUDIO_U8:
			case AUDIO_S8:
				audioBuffer->bitsPerSample = 8;
				break;

			#if LIME_SDL2_SOUND
			case AUDIO_U16LSB:
			case AUDIO_U16MSB:
			#endif
			case AUDIO_S16LSB:
			case AUDIO_S16MSB:
			default:
				audioBuffer->bitsPerSample = 16;
				break;

			case AUDIO_F32LSB:
			case AUDIO_F32MSB:
				audioBuffer->dataFormat = 3;
			case AUDIO_S32LSB:
			case AUDIO_S32MSB:
				audioBuffer->bitsPerSample = 32;
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
				audioBuffer->data->Resize(0);
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