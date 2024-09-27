#include "audio_decode.h"
#include <pthread.h>

typedef struct {
    DemuxMp4 *demux;
    AVCodecContext *codec_ctx;
    AVFrame *frame;
    GQueue *frame_queue;
    int max_queue_size;
    pthread_t decode_thread;
    gboolean stop_thread;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
} AudioDecodePrivate;

G_DEFINE_TYPE_WITH_PRIVATE(AudioDecode, audio_decode, G_TYPE_OBJECT)
static void* audio_decode_thread_func(void *data) {
    AudioDecode *self = AUDIO_DECODE(data);
    AudioDecodePrivate *priv = audio_decode_get_instance_private(self);
    DemuxMp4FileInfo demux_info;
    demux_mp4_get_file_info(priv->demux, &demux_info);

    while (!priv->stop_thread) {
        AVPacket *packet = demux_mp4_read(priv->demux);
        if (!packet || packet->stream_index != demux_info.audio_index) {
            if (packet) {
                av_packet_free(&packet);
            }
            continue;
        }

        if (avcodec_send_packet(priv->codec_ctx, packet) < 0) {
            av_packet_free(&packet);
            continue;
        }

        while (avcodec_receive_frame(priv->codec_ctx, priv->frame) == 0) {
            AVFrame *frame_copy = av_frame_clone(priv->frame);

            pthread_mutex_lock(&priv->queue_mutex);
            while (g_queue_get_length(priv->frame_queue) >= priv->max_queue_size) {
                pthread_cond_wait(&priv->queue_cond, &priv->queue_mutex);
            }
            g_queue_push_tail(priv->frame_queue, frame_copy);
            pthread_cond_signal(&priv->queue_cond);
            pthread_mutex_unlock(&priv->queue_mutex);
        }

        av_packet_free(&packet);
    }

    return NULL;
}

static void audio_decode_class_init(AudioDecodeClass *klass) {
    // 在这里添加类的方法
}

static void audio_decode_init(AudioDecode *self) {
    AudioDecodePrivate *priv = audio_decode_get_instance_private(self);
    priv->demux = NULL;
    priv->codec_ctx = NULL;
    priv->frame = av_frame_alloc();
    priv->frame_queue = g_queue_new();
    priv->max_queue_size = 0;
    priv->stop_thread = FALSE;
    pthread_mutex_init(&priv->queue_mutex, NULL);
    pthread_cond_init(&priv->queue_cond, NULL);
}

void audio_decode_initialize(AudioDecode *self, DemuxMp4 *demux, int max_queue_size) {
    AudioDecodePrivate *priv = audio_decode_get_instance_private(self);
    priv->demux = demux;
    priv->max_queue_size = max_queue_size;

    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_AAC);
    if (!codec) {
        g_error("无法找到AAC解码器");
        return;
    }

    priv->codec_ctx = avcodec_alloc_context3(codec);
    if (!priv->codec_ctx) {
        g_error("无法分配解码器上下文");
        return;
    }

    if (avcodec_open2(priv->codec_ctx, codec, NULL) < 0) {
        g_error("无法打开解码器");
        return;
    }
}

AVFrame* audio_decode_get_frame(AudioDecode *self) {
    AudioDecodePrivate *priv = audio_decode_get_instance_private(self);

    pthread_mutex_lock(&priv->queue_mutex);
    while (g_queue_is_empty(priv->frame_queue)) {
        pthread_cond_wait(&priv->queue_cond, &priv->queue_mutex);
    }
    AVFrame *frame = g_queue_pop_head(priv->frame_queue);
    pthread_cond_signal(&priv->queue_cond);
    pthread_mutex_unlock(&priv->queue_mutex);

    return frame;
}

void audio_decode_cleanup(AudioDecode *self) {
    AudioDecodePrivate *priv = audio_decode_get_instance_private(self);
    priv->stop_thread = TRUE;
    pthread_join(priv->decode_thread, NULL);

    if (priv->codec_ctx) {
        avcodec_free_context(&priv->codec_ctx);
        priv->codec_ctx = NULL;
    }

    if (priv->frame) {
        av_frame_free(&priv->frame);
        priv->frame = NULL;
    }

    while (!g_queue_is_empty(priv->frame_queue)) {
        AVFrame *frame = g_queue_pop_head(priv->frame_queue);
        av_frame_free(&frame);
    }
    g_queue_free(priv->frame_queue);

    pthread_mutex_destroy(&priv->queue_mutex);
    pthread_cond_destroy(&priv->queue_cond);
}

void audio_decode_start(AudioDecode *self) {
    AudioDecodePrivate *priv = audio_decode_get_instance_private(self);
    priv->stop_thread = FALSE;
    pthread_create(&priv->decode_thread, NULL, audio_decode_thread_func, self);
}

void audio_decode_stop(AudioDecode *self) {
    AudioDecodePrivate *priv = audio_decode_get_instance_private(self);
    priv->stop_thread = TRUE;
    pthread_join(priv->decode_thread, NULL);
}

void audio_decode_flush(AudioDecode *self) {
    AudioDecodePrivate *priv = audio_decode_get_instance_private(self);

    pthread_mutex_lock(&priv->queue_mutex);
    while (!g_queue_is_empty(priv->frame_queue)) {
        AVFrame *frame = g_queue_pop_head(priv->frame_queue);
        av_frame_free(&frame);
    }
    pthread_cond_signal(&priv->queue_cond);
    pthread_mutex_unlock(&priv->queue_mutex);

    avcodec_flush_buffers(priv->codec_ctx);
}
