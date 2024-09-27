#include "audio_sdl_render.h"
#include <glib.h>

#define DEFAULT_QUEUE_DURATION_MS 500

typedef struct {
    AudioDecode *audio_decode;
    SDL_AudioDeviceID audio_device;
    gboolean is_playing;
    pthread_t render_thread;
    gboolean stop_thread;
    GQueue *pcm_queue;
    GMutex queue_mutex;
    guint queue_max_size;
} AudioSdlRenderPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(AudioSdlRender, audio_sdl_render, G_TYPE_OBJECT)

static void audio_callback(void *userdata, Uint8 *stream, int len) {
    AudioSdlRender *self = AUDIO_SDL_RENDER(userdata);
    AudioSdlRenderPrivate *priv = audio_sdl_render_get_instance_private(self);

    g_mutex_lock(&priv->queue_mutex);
    while (len > 0) {
        if (g_queue_is_empty(priv->pcm_queue)) {
            SDL_memset(stream, 0, len);
            break;
        }

        AVFrame *frame = g_queue_pop_head(priv->pcm_queue);
        g_mutex_unlock(&priv->queue_mutex);

        int data_size = av_samples_get_buffer_size(NULL, frame->channels, frame->nb_samples, frame->format, 1);
        if (data_size > len) {
            data_size = len;
        }

        SDL_MixAudioFormat(stream, frame->data[0], AUDIO_S16SYS, data_size, SDL_MIX_MAXVOLUME);
        len -= data_size;
        stream += data_size;

        av_frame_free(&frame);
        g_mutex_lock(&priv->queue_mutex);
    }
    g_mutex_unlock(&priv->queue_mutex);
}

static void* render_thread_func(void *data) {
    AudioSdlRender *self = AUDIO_SDL_RENDER(data);
    AudioSdlRenderPrivate *priv = audio_sdl_render_get_instance_private(self);

    while (!priv->stop_thread) {
        AVFrame *frame = audio_decode_get_frame(priv->audio_decode);
        if (frame) {
            g_mutex_lock(&priv->queue_mutex);
            if (g_queue_get_length(priv->pcm_queue) < priv->queue_max_size) {
                g_queue_push_tail(priv->pcm_queue, frame);
            } else {
                av_frame_free(&frame);
            }
            g_mutex_unlock(&priv->queue_mutex);
        }
        SDL_Delay(10);
    }

    return NULL;
}

static void audio_sdl_render_class_init(AudioSdlRenderClass *klass) {
    // 在这里添加类的方法
}

static void audio_sdl_render_init(AudioSdlRender *self) {
    AudioSdlRenderPrivate *priv = audio_sdl_render_get_instance_private(self);
    priv->audio_decode = NULL;
    priv->audio_device = 0;
    priv->is_playing = FALSE;
    priv->stop_thread = FALSE;
    priv->pcm_queue = g_queue_new();
    g_mutex_init(&priv->queue_mutex);
    priv->queue_max_size = DEFAULT_QUEUE_DURATION_MS / 10; // Assuming 10ms per frame
}

void audio_sdl_render_initialize(AudioSdlRender *self, AudioDecode *audio_decode) {
    AudioSdlRenderPrivate *priv = audio_sdl_render_get_instance_private(self);
    priv->audio_decode = audio_decode;

    SDL_AudioSpec wanted_spec, obtained_spec;
    SDL_zero(wanted_spec);
    wanted_spec.freq = 44100;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = 2;
    wanted_spec.samples = 1024;
    wanted_spec.callback = audio_callback;
    wanted_spec.userdata = self;

    priv->audio_device = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &obtained_spec, 0);
    if (priv->audio_device == 0) {
        g_error("无法打开音频设备: %s", SDL_GetError());
        return;
    }

    pthread_create(&priv->render_thread, NULL, render_thread_func, self);
}

void audio_sdl_render_play(AudioSdlRender *self) {
    AudioSdlRenderPrivate *priv = audio_sdl_render_get_instance_private(self);
    priv->is_playing = TRUE;
}

void audio_sdl_render_stop(AudioSdlRender *self) {
    AudioSdlRenderPrivate *priv = audio_sdl_render_get_instance_private(self);
    priv->is_playing = FALSE;
    priv->stop_thread = TRUE;
    pthread_join(priv->render_thread, NULL);
    SDL_CloseAudioDevice(priv->audio_device);

    g_mutex_clear(&priv->queue_mutex);
    g_queue_free(priv->pcm_queue);
}
