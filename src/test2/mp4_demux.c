#include "mp4_demux.h"
#include <pthread.h>

// 定义私有数据结构
typedef struct {
    AVFormatContext *fmt_ctx;
    AVCodecContext *video_codec_ctx;
    AVCodecContext *audio_codec_ctx;
    AVCodec *video_codec;
    AVCodec *audio_codec;
    AVStream *video_stream;
    AVStream *audio_stream;
    int video_index;
    int audio_index;
    int subtitle_index;
    int64_t start_time;
    GQueue *packet_queue;
    int max_queue_size;
    pthread_t demux_thread;
    gboolean stop_thread;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond;
} DemuxMp4Private;

// 使用 G_DEFINE_TYPE_WITH_PRIVATE 宏定义类型并关联私有数据
G_DEFINE_TYPE_WITH_PRIVATE(DemuxMp4, demux_mp4, G_TYPE_OBJECT)

static void* demux_mp4_thread_func(void *data) {
    DemuxMp4 *self = DEMUX_MP4(data);
    DemuxMp4Private *priv = demux_mp4_get_instance_private(self);

    while (!priv->stop_thread) {
        AVPacket *packet = av_packet_alloc();
        if (!packet) {
            g_error("无法分配AVPacket");
            continue;
        }
        if (av_read_frame(priv->fmt_ctx, packet) >= 0) {
            pthread_mutex_lock(&priv->queue_mutex);
            while (g_queue_get_length(priv->packet_queue) >= priv->max_queue_size) {
                pthread_cond_wait(&priv->queue_cond, &priv->queue_mutex);
            }
            g_queue_push_tail(priv->packet_queue, packet);
            pthread_cond_signal(&priv->queue_cond);
            pthread_mutex_unlock(&priv->queue_mutex);
        } else {
            av_packet_free(&packet);
        }
    }

    return NULL;
}

static void demux_mp4_init(DemuxMp4 *self) {
    // 初始化代码
    DemuxMp4Private *priv = demux_mp4_get_instance_private(self);
    priv->fmt_ctx = NULL;
    priv->video_codec_ctx = NULL;
    priv->audio_codec_ctx = NULL;
    priv->video_codec = NULL;
    priv->audio_codec = NULL;
    priv->video_stream = NULL;
    priv->audio_stream = NULL;
    priv->video_index = -1;
    priv->audio_index = -1;
    priv->subtitle_index = -1;
    priv->start_time = AV_NOPTS_VALUE;
    priv->packet_queue = g_queue_new();
    priv->max_queue_size = 0;
    priv->stop_thread = FALSE;
    pthread_mutex_init(&priv->queue_mutex, NULL);
    pthread_cond_init(&priv->queue_cond, NULL);
}

static void demux_mp4_class_init(DemuxMp4Class *klass) {
    // 类初始化代码
    // 可以在这里添加类方法的初始化代码
}

void demux_mp4_open(DemuxMp4 *self, const char *filename, int max_queue_size) {
    DemuxMp4Private *priv = demux_mp4_get_instance_private(self);
    priv->max_queue_size = max_queue_size;
    // 打开文件并初始化格式上下文
    if (avformat_open_input(&priv->fmt_ctx, filename, NULL, NULL) < 0) {
        g_error("无法打开文件: %s", filename);
        return;
    }

    // 检索流信息
    if (avformat_find_stream_info(priv->fmt_ctx, NULL) < 0) {
        g_error("无法检索流信息");
        return;
    }

    // 查找视频流、音频流和字幕流的索引
    priv->video_index = -1;
    priv->audio_index = -1;
    priv->subtitle_index = -1;
    for (unsigned int i = 0; i < priv->fmt_ctx->nb_streams; i++) {
        AVStream *stream = priv->fmt_ctx->streams[i];
        AVCodecParameters *codecpar = stream->codecpar;
        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO && priv->video_index == -1) {
            priv->video_index = i;
            priv->video_stream = stream;
            priv->video_codec = avcodec_find_decoder(codecpar->codec_id);
            if (!priv->video_codec) {
                g_error("无法找到视频解码器");
                return;
            }
            priv->video_codec_ctx = avcodec_alloc_context3(priv->video_codec);
            if (!priv->video_codec_ctx) {
                g_error("无法分配视频解码器上下文");
                return;
            }
            if (avcodec_parameters_to_context(priv->video_codec_ctx, codecpar) < 0) {
                g_error("无法复制视频解码器参数");
                return;
            }
            if (avcodec_open2(priv->video_codec_ctx, priv->video_codec, NULL) < 0) {
                g_error("无法打开视频解码器");
                return;
            }
        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO && priv->audio_index == -1) {
            priv->audio_index = i;
            priv->audio_stream = stream;
            priv->audio_codec = avcodec_find_decoder(codecpar->codec_id);
            if (!priv->audio_codec) {
                g_error("无法找到音频解码器");
                return;
            }
            priv->audio_codec_ctx = avcodec_alloc_context3(priv->audio_codec);
            if (!priv->audio_codec_ctx) {
                g_error("无法分配音频解码器上下文");
                return;
            }
            if (avcodec_parameters_to_context(priv->audio_codec_ctx, codecpar) < 0) {
                g_error("无法复制音频解码器参数");
                return;
            }
            if (avcodec_open2(priv->audio_codec_ctx, priv->audio_codec, NULL) < 0) {
                g_error("无法打开音频解码器");
                return;
            }
        } else if (codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE && priv->subtitle_index == -1) {
            priv->subtitle_index = i;
        }
    }

    // 设置开始时间
    priv->start_time = priv->fmt_ctx->start_time;

    // 启动demux线程
    priv->stop_thread = FALSE;
    pthread_create(&priv->demux_thread, NULL, demux_mp4_thread_func, self);
}

AVPacket* demux_mp4_read(DemuxMp4 *self) {
    DemuxMp4Private *priv = demux_mp4_get_instance_private(self);

    pthread_mutex_lock(&priv->queue_mutex);
    while (g_queue_is_empty(priv->packet_queue)) {
        pthread_cond_wait(&priv->queue_cond, &priv->queue_mutex);
    }
    AVPacket *packet = g_queue_pop_head(priv->packet_queue);
    pthread_cond_signal(&priv->queue_cond);
    pthread_mutex_unlock(&priv->queue_mutex);

    return packet;
}

void demux_mp4_seek(DemuxMp4 *self, int64_t timestamp) {
    DemuxMp4Private *priv = demux_mp4_get_instance_private(self);

    // 清空队列
    pthread_mutex_lock(&priv->queue_mutex);
    while (!g_queue_is_empty(priv->packet_queue)) {
        AVPacket *packet = g_queue_pop_head(priv->packet_queue);
        av_packet_free(&packet);
    }
    pthread_cond_signal(&priv->queue_cond);
    pthread_mutex_unlock(&priv->queue_mutex);

    if (av_seek_frame(priv->fmt_ctx, -1, timestamp, AVSEEK_FLAG_BACKWARD) < 0) {
        g_error("无法跳转到指定时间戳");
    }
}

void demux_mp4_close(DemuxMp4 *self) {
    DemuxMp4Private *priv = demux_mp4_get_instance_private(self);
    priv->stop_thread = TRUE;
    pthread_join(priv->demux_thread, NULL);

    if (priv->video_codec_ctx) {
        avcodec_free_context(&priv->video_codec_ctx);
        priv->video_codec_ctx = NULL;
    }
    if (priv->audio_codec_ctx) {
        avcodec_free_context(&priv->audio_codec_ctx);
        priv->audio_codec_ctx = NULL;
    }
    if (priv->fmt_ctx) {
        avformat_close_input(&priv->fmt_ctx);
        priv->fmt_ctx = NULL;
    }

    while (!g_queue_is_empty(priv->packet_queue)) {
        AVPacket *packet = g_queue_pop_head(priv->packet_queue);
        av_packet_free(&packet);
    }
    g_queue_free(priv->packet_queue);

    pthread_mutex_destroy(&priv->queue_mutex);
    pthread_cond_destroy(&priv->queue_cond);
}

void demux_mp4_get_file_info(DemuxMp4 *self, DemuxMp4FileInfo *info) {
    DemuxMp4Private *priv = demux_mp4_get_instance_private(self);
    info->video_track_count = 0;
    info->audio_track_count = 0;
    info->duration = priv->fmt_ctx->duration;
    info->video_width = 0;
    info->video_height = 0;
    info->video_frame_rate = 0.0;
    info->video_bit_rate = 0;
    info->audio_sample_rate = 0;
    info->audio_channels = 0;
    info->audio_sample_format[0] = '\0';
    info->video_index = priv->video_index;
    info->audio_index = priv->audio_index;

    for (unsigned int i = 0; i < priv->fmt_ctx->nb_streams; i++) {
        AVStream *stream = priv->fmt_ctx->streams[i];
        AVCodecParameters *codecpar = stream->codecpar;
        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            info->video_track_count++;
            if (info->video_track_count == 1) {
                info->video_width = codecpar->width;
                info->video_height = codecpar->height;
                info->video_frame_rate = av_q2d(stream->avg_frame_rate);
                info->video_bit_rate = codecpar->bit_rate;
            }
        } else if (codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            info->audio_track_count++;
            if (info->audio_track_count == 1) {
                info->audio_sample_rate = codecpar->sample_rate;
                info->audio_channels = codecpar->ch_layout.nb_channels;
                av_get_sample_fmt_string(info->audio_sample_format, sizeof(info->audio_sample_format), codecpar->format);
            }
        }
    }
}
