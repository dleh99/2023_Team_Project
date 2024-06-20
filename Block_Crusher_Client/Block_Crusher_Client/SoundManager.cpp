#pragma once
#include "SoundManager.h"

SoundManager::SoundManager()
{
	FMOD_System_Create(&m_SoundSystem);
	FMOD_System_Init(m_SoundSystem, CHANNEL_NUM, FMOD_INIT_NORMAL, NULL);

	FMOD_System_CreateSound(m_SoundSystem, "Sound\\Stage.mp3", FMOD_LOOP_NORMAL, 0, &m_Stage_soundFile);
	FMOD_System_CreateSound(m_SoundSystem, "Sound\\Battle.mp3", FMOD_LOOP_NORMAL, 0, &m_Battle_soundFile);
	FMOD_System_CreateSound(m_SoundSystem, "Sound\\Gun.mp3", FMOD_LOOP_OFF, 0, &m_effect_sound[GUN]);
	FMOD_System_CreateSound(m_SoundSystem, "Sound\\Death.mp3", FMOD_LOOP_OFF, 0, &m_effect_sound[DEATH]);
}

void SoundManager::StartStageSound()
{
	FMOD_Channel_Stop(m_sound_channel[0]);
	FMOD_System_PlaySound(m_SoundSystem, m_Stage_soundFile, NULL, 0, &m_sound_channel[0]);
	FMOD_Channel_SetVolume(m_sound_channel[0], 0.02);
}

void SoundManager::StartBattleSound()
{
	FMOD_Channel_Stop(m_sound_channel[0]);
	FMOD_System_PlaySound(m_SoundSystem, m_Battle_soundFile, NULL, 0, &m_sound_channel[0]);
	FMOD_Channel_SetVolume(m_sound_channel[0], 0.02);
}

void SoundManager::EffectSound(SOUND tag)
{
	FMOD_BOOL playing = false;
	for (int i = 1; i < CHANNEL_NUM; ++i) {
		//m_sound_channel[i]->isPlaying(&playing);
		playing = false;
		FMOD_Channel_IsPlaying(m_sound_channel[i], &playing);
		if (false == playing) {
			//std::cout << i << std::endl;
			FMOD_System_PlaySound(m_SoundSystem, m_effect_sound[tag], NULL, 0, &m_sound_channel[i]);
			FMOD_Channel_SetVolume(m_sound_channel[i], 0.02);
			++i;
			break;
		}
	}
}
