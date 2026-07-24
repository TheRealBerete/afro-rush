#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>
#include "audio.h"

#define GRAIN 1024 /* echantillons par bloc envoye a sceAudioOutOutput, doit etre multiple de 64 */

typedef struct {
    short *pcm;              /* echantillons entrelaces (L,R,L,R,...) */
    unsigned int total_frames; /* nombre de "frames" (1 frame = 1 echantillon par canal) */
    int channels;
    int freq;
} WavData;

static WavData s_wav;
static int s_port = -1;
static SceUID s_thread = -1;
static volatile int s_running = 0;

static unsigned int read_u32le(const unsigned char *p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8) | ((unsigned int)p[2] << 16) | ((unsigned int)p[3] << 24);
}

static unsigned int read_u16le(const unsigned char *p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8);
}

/* charge un WAV canonique PCM 16-bit (produit par notre script de conversion ffmpeg) */
static int load_wav(const char *path, WavData *out) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    unsigned char header[12];
    if (fread(header, 1, 12, f) != 12 ||
        memcmp(header, "RIFF", 4) != 0 || memcmp(header + 8, "WAVE", 4) != 0) {
        fclose(f);
        return -1;
    }

    int channels = 2, freq = 44100, bits = 16;
    unsigned char *data = NULL;
    unsigned int data_size = 0;

    while (fread(header, 1, 8, f) == 8) {
        unsigned int chunk_size = read_u32le(header + 4);

        if (memcmp(header, "fmt ", 4) == 0) {
            unsigned char fmt[16];
            unsigned int to_read = chunk_size < 16 ? chunk_size : 16;
            fread(fmt, 1, to_read, f);
            channels = (int)read_u16le(fmt + 2);
            freq = (int)read_u32le(fmt + 4);
            bits = (int)read_u16le(fmt + 14);
            if (chunk_size > to_read) fseek(f, (long)(chunk_size - to_read), SEEK_CUR);
        } else if (memcmp(header, "data", 4) == 0) {
            data_size = chunk_size;
            data = (unsigned char *)malloc(data_size);
            if (!data) { fclose(f); return -1; }
            fread(data, 1, data_size, f);
            break; /* le "data" ferme le fichier canonique qui nous interesse */
        } else {
            fseek(f, (long)chunk_size, SEEK_CUR);
        }

        if (chunk_size % 2 == 1) fseek(f, 1, SEEK_CUR); /* chunks alignes sur 2 octets */
    }

    fclose(f);

    if (!data || bits != 16 || channels < 1) {
        free(data);
        return -1;
    }

    out->pcm = (short *)data;
    out->channels = channels;
    out->freq = freq;
    out->total_frames = data_size / (unsigned int)(channels * (int)sizeof(short));
    return 0;
}

static int audio_thread(SceSize args, void *argp) {
    (void)args;
    (void)argp;
    short buffer[GRAIN * 2]; /* toujours envoye en stereo au port audio */
    unsigned int cursor = 0;

    while (s_running) {
        for (int i = 0; i < GRAIN; i++) {
            unsigned int frame = (cursor + (unsigned int)i) % s_wav.total_frames;
            if (s_wav.channels == 2) {
                buffer[i * 2]     = s_wav.pcm[frame * 2];
                buffer[i * 2 + 1] = s_wav.pcm[frame * 2 + 1];
            } else {
                short v = s_wav.pcm[frame];
                buffer[i * 2] = v;
                buffer[i * 2 + 1] = v;
            }
        }
        cursor = (cursor + GRAIN) % s_wav.total_frames;
        sceAudioOutOutput(s_port, buffer);
    }
    return 0;
}

int audio_start_bgm(const char *wav_path) {
    if (load_wav(wav_path, &s_wav) != 0) return -1;

    s_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, GRAIN, s_wav.freq, SCE_AUDIO_OUT_MODE_STEREO);
    if (s_port < 0) {
        free(s_wav.pcm);
        s_wav.pcm = NULL;
        return -1;
    }

    s_running = 1;
    s_thread = sceKernelCreateThread("bgm_thread", audio_thread, 0x10000100, 0x10000, 0, 0, NULL);
    if (s_thread < 0) {
        sceAudioOutReleasePort(s_port);
        s_port = -1;
        free(s_wav.pcm);
        s_wav.pcm = NULL;
        return -1;
    }
    sceKernelStartThread(s_thread, 0, NULL);
    return 0;
}

void audio_stop_bgm(void) {
    s_running = 0;
    if (s_thread >= 0) {
        sceKernelWaitThreadEnd(s_thread, NULL, NULL);
        sceKernelDeleteThread(s_thread);
        s_thread = -1;
    }
    if (s_port >= 0) {
        sceAudioOutReleasePort(s_port);
        s_port = -1;
    }
    if (s_wav.pcm) {
        free(s_wav.pcm);
        s_wav.pcm = NULL;
    }
}
