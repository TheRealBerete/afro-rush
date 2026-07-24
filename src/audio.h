#ifndef AUDIO_H
#define AUDIO_H

/* Lance la musique de fond en boucle infinie et fluide (thread dedie). */
int audio_start_bgm(const char *wav_path);

/* Arrete la musique et libere les ressources. */
void audio_stop_bgm(void);

#endif
