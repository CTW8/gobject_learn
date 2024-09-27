#include "video_decode.h"
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
    int video_index; // 添加video_index字段
} VideoDecodePrivate;

G_DEFINE_TYPE_WITH_PRIVATE(VideoDecode, video_decode, G_TYPE_OBJECT)
static void* video_decode_thread_func(void *data) {
    VideoDecode *self = VIDEO_DECODE(data);
    VideoDecodePrivate *priv = video_decode_get_instance_private(self);
    DemuxMp4FileInfo demux_info;
    demux_mp4_get_file_info(priv->demux, &demux_info);
    priv->video_index = demux_info.video_index; // 从demux_info中获取video_index

    while (!priv->stop_thread) {
        AVPacket *packet = demux_mp4_read(priv->demux);
        if (!packet || packet->stream_index != priv->video_index) {
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

static void video_decode_class_init(VideoDecodeClass *klass) {
    // 在这里添加类的方法
}

static void video_decode_init(VideoDecode *self) {
    VideoDecodePrivate *priv = video_decode_get_instance_private(self);
    priv->demux = NULL;
    priv->codec_ctx = NULL;
    priv->frame = av_frame_alloc();
    priv->frame_queue = g_queue_new();
    priv->max_queue_size = 0;
    priv->stop_thread = FALSE;
    priv->video_index = -1; // 初始化video_index
    pthread_mutex_init(&priv->queue_mutex, NULL);
    pthread_cond_init(&priv->queue_cond, NULL);
}

void video_decode_initialize(VideoDecode *self, DemuxMp4 *demux, int max_queue_size) {
    VideoDecodePrivate *priv = video_decode_get_instance_private(self);
    priv->demux = demux;
    priv->max_queue_size = max_queue_size;

    AVCodec *codec = avcodec_find_decoder(AV_CODEC_ID_H264);
    if (!codec) {
        g_error("无法找到H264解码器");
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

    pthread_create(&priv->decode_thread, NULL, video_decode_thread_func, self);
}

void video_decode_decode_packet(VideoDecode *self) {
    // 该方法不再需要实现，因为解码在独立线程中进行
}

AVFrame* video_decode_get_frame(VideoDecode *self) {
    VideoDecodePrivate *priv = video_decode_get_instance_private(self);

    pthread_mutex_lock(&priv->queue_mutex);
    while (g_queue_is_empty(priv->frame_queue)) {
        pthread_cond_wait(&priv->queue_cond, &priv->queue_mutex);
    }
    AVFrame *frame = g_queue_pop_head(priv->frame_queue);
    pthread_cond_signal(&priv->queue_cond);
    pthread_mutex_unlock(&priv->queue_mutex);

    return frame;
}

void video_decode_cleanup(VideoDecode *self) {
    VideoDecodePrivate *priv = video_decode_get_instance_private(self);
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

void video_decode_flush(VideoDecode *self) {
    VideoDecodePrivate *priv = video_decode_get_instance_private(self);

    pthread_mutex_lock(&priv->queue_mutex);
    while (!g_queue_is_empty(priv->frame_queue)) {
        AVFrame *frame = g_queue_pop_head(priv->frame_queue);
        av_frame_free(&frame);
    }
    pthread_cond_signal(&priv->queue_cond);
    pthread_mutex_unlock(&priv->queue_mutex);

    avcodec_flush_buffers(priv->codec_ctx);
}
