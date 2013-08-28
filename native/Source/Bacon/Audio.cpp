#include "Bacon.h"
#include "BaconInternal.h"
#include "HandleArray.h"
using namespace Bacon;

#include <string>
using namespace std;

#include <gorilla/ga.h>
#include <gorilla/gau.h>

namespace {
	
	struct Sound
	{
		int m_Flags;
		string m_Path;
		ga_Sound* m_Sound;
	};
	
	struct Voice
	{
		ga_Handle* m_Handle;
		gau_SampleSourceLoop* m_Loop;
		Bacon_VoiceCallback m_Callback;
	};
	
	struct Impl
	{
		gau_Manager* m_Manager;
		ga_Mixer* m_Mixer;
		ga_StreamManager* m_StreamManager;
		
		HandleArray<Sound> m_Sounds;
		HandleArray<Voice> m_Voices;

        int m_DebugCounter_Sounds;
        int m_DebugCounter_Voices;
	};
	static Impl* s_Impl;
	
}

static int ConvertGAError(int error)
{
	if (error == GC_SUCCESS)
		return Bacon_Error_None;
	return Bacon_Error_Unknown;
}

static void VoiceCallback(ga_Handle* in_finishedHandle, void* in_context)
{
	int voiceHandle = (int)(size_t)in_context;
	Voice* voice = s_Impl->m_Voices.Get(voiceHandle);
	if (voice)
	{
		if (voice->m_Callback)
			voice->m_Callback();
		s_Impl->m_Voices.Free(voiceHandle);
        DebugOverlay_AddCounter(s_Impl->m_DebugCounter_Voices, -1);
	}
	ga_handle_destroy(in_finishedHandle);
}

void Audio_Init()
{
	gc_initialize(0);
	s_Impl = new Impl;
	s_Impl->m_Manager = gau_manager_create();
	s_Impl->m_Mixer = gau_manager_mixer(s_Impl->m_Manager);
	s_Impl->m_StreamManager = gau_manager_streamManager(s_Impl->m_Manager);

    s_Impl->m_DebugCounter_Sounds = DebugOverlay_CreateCounter("Sounds");
    s_Impl->m_DebugCounter_Voices = DebugOverlay_CreateCounter("Voices");
}

void Audio_Shutdown()
{
	gau_manager_destroy(s_Impl->m_Manager);
	delete s_Impl;
	s_Impl = nullptr;
}

void Audio_Update()
{
	gau_manager_update(s_Impl->m_Manager);
}

static const char* GetSoundFormat(int flags)
{
	if ((flags & Bacon_SoundFlags_FormatMask) == Bacon_SoundFlags_FormatWav)
		return "wav";
	else if ((flags & Bacon_SoundFlags_FormatMask) == Bacon_SoundFlags_FormatOgg)
		return "ogg";
	return "???";
}

int Bacon_LoadSound(int* outHandle, const char* path, int flags)
{
	if (!outHandle || !path)
		return Bacon_Error_InvalidArgument;
	
	if (!GetSoundFormat(flags))
		return Bacon_Error_UnsupportedFormat;
	
	*outHandle = s_Impl->m_Sounds.Alloc();
	Sound* sound = s_Impl->m_Sounds.Get(*outHandle);
	
	sound->m_Path = "";
	sound->m_Sound = nullptr;
	sound->m_Flags = flags;
	if (flags & Bacon_SoundFlags_Stream)
	{
		sound->m_Path = path;
	}
	else
	{
		sound->m_Sound = gau_load_sound_file(path, GetSoundFormat(flags));
		if (!sound->m_Sound)
			return Bacon_Error_Unknown;
	}

    DebugOverlay_AddCounter(s_Impl->m_DebugCounter_Sounds, 1);

	return Bacon_Error_None;
}

int Bacon_UnloadSound(int soundHandle)
{
	Sound* sound = s_Impl->m_Sounds.Get(soundHandle);
	if (!sound)
		return Bacon_Error_InvalidHandle;

    DebugOverlay_AddCounter(s_Impl->m_DebugCounter_Sounds, -1);

	ga_sound_release(sound->m_Sound);
	return Bacon_Error_None;
}

static ga_Handle* CreateHandle(Sound* sound, ga_FinishCallback callback, void* context, gau_SampleSourceLoop** loop)
{
	if (sound->m_Sound)
		return gau_create_handle_sound(s_Impl->m_Mixer, sound->m_Sound, callback, context, loop);
	else
		return gau_create_handle_buffered_file(s_Impl->m_Mixer, s_Impl->m_StreamManager, sound->m_Path.c_str(), GetSoundFormat(sound->m_Flags), callback, context, loop);
}

int Bacon_PlaySound(int soundHandle)
{
	Sound* sound = s_Impl->m_Sounds.Get(soundHandle);
	if (!sound)
		return Bacon_Error_InvalidHandle;
	
	ga_Handle* handle = CreateHandle(sound, gau_on_finish_destroy, nullptr, nullptr);
	if (!handle)
		return Bacon_Error_Unknown;
	
	return ConvertGAError(ga_handle_play(handle));
}

int Bacon_CreateVoice(int* outHandle, int soundHandle, int voiceFlags)
{
	Sound* sound = s_Impl->m_Sounds.Get(soundHandle);
	if (!sound)
		return Bacon_Error_InvalidHandle;
	
	*outHandle = s_Impl->m_Voices.Alloc();
	Voice* voice = s_Impl->m_Voices.Get(*outHandle);
	voice->m_Callback = nullptr;
	voice->m_Loop = nullptr;
	gau_SampleSourceLoop** loop = (voiceFlags & Bacon_VoiceFlags_Loop) ? &voice->m_Loop : nullptr;
	voice->m_Handle = CreateHandle(sound, VoiceCallback, (void*)(size_t)*outHandle, loop);
	if (!voice->m_Handle)
		return Bacon_Error_Unknown;
	
    DebugOverlay_AddCounter(s_Impl->m_DebugCounter_Voices, 1);

	return Bacon_Error_None;
}

int Bacon_DestroyVoice(int voiceHandle)
{
	Voice* voice = s_Impl->m_Voices.Get(voiceHandle);
	if (!voice)
		return Bacon_Error_InvalidHandle;
	
	ga_handle_destroy(voice->m_Handle);
	s_Impl->m_Voices.Free(voiceHandle);

    DebugOverlay_AddCounter(s_Impl->m_DebugCounter_Voices, -1);

	return Bacon_Error_None;
}

int Bacon_PlayVoice(int voiceHandle)
{
	Voice* voice = s_Impl->m_Voices.Get(voiceHandle);
	if (!voice)
		return Bacon_Error_InvalidHandle;

	return ConvertGAError(ga_handle_play(voice->m_Handle));
}

int Bacon_StopVoice(int voiceHandle)
{
	Voice* voice = s_Impl->m_Voices.Get(voiceHandle);
	if (!voice)
		return Bacon_Error_InvalidHandle;
	
	return ConvertGAError(ga_handle_stop(voice->m_Handle));
}

int Bacon_SetVoiceGain(int voiceHandle, float gain)
{
	Voice* voice = s_Impl->m_Voices.Get(voiceHandle);
	if (!voice)
		return Bacon_Error_InvalidHandle;
	
	return ConvertGAError(ga_handle_setParamf(voice->m_Handle, GA_HANDLE_PARAM_GAIN, gain));
}

int Bacon_SetVoicePitch(int voiceHandle, float pitch)
{
	Voice* voice = s_Impl->m_Voices.Get(voiceHandle);
	if (!voice)
		return Bacon_Error_InvalidHandle;
	
	return ConvertGAError(ga_handle_setParamf(voice->m_Handle, GA_HANDLE_PARAM_PITCH, pitch));
}

int Bacon_SetVoicePan(int voiceHandle, float pan)
{
	Voice* voice = s_Impl->m_Voices.Get(voiceHandle);
	if (!voice)
		return Bacon_Error_InvalidHandle;
	
	return ConvertGAError(ga_handle_setParamf(voice->m_Handle, GA_HANDLE_PARAM_PAN, pan));
}

int Bacon_SetVoiceLoopPoints(int voiceHandle, int startSample, int endSample)
{
	Voice* voice = s_Impl->m_Voices.Get(voiceHandle);
	if (!voice)
		return Bacon_Error_InvalidHandle;
	
	if (!voice->m_Loop)
		return Bacon_Error_NotLooping;
	
	gau_sample_source_loop_set(voice->m_Loop, startSample, endSample);
	return Bacon_Error_None;
}

int Bacon_SetVoiceCallback(int voiceHandle, Bacon_VoiceCallback callback)
{
	Voice* voice = s_Impl->m_Voices.Get(voiceHandle);
	if (!voice)
		return Bacon_Error_InvalidHandle;

	voice->m_Callback = callback;
	return Bacon_Error_None;
}

int Bacon_IsVoicePlaying(int voiceHandle, int* playing)
{
	Voice* voice = s_Impl->m_Voices.Get(voiceHandle);
	if (!voice)
		return Bacon_Error_InvalidHandle;
	
	*playing = ga_handle_playing(voice->m_Handle);
	return Bacon_Error_None;
}

int Bacon_GetVoicePosition(int voiceHandle, int* sample)
{
	Voice* voice = s_Impl->m_Voices.Get(voiceHandle);
	if (!voice)
		return Bacon_Error_InvalidHandle;
	
	*sample = ga_handle_tell(voice->m_Handle, GA_TELL_PARAM_CURRENT);
	return Bacon_Error_None;
}

int Bacon_SetVoicePosition(int voiceHandle, int sample)
{
	Voice* voice = s_Impl->m_Voices.Get(voiceHandle);
	if (!voice)
		return Bacon_Error_InvalidHandle;
	
	return ConvertGAError(ga_handle_seek(voice->m_Handle, sample));
}
