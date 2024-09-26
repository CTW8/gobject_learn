#include "decode.h"

typedef struct {
    AVFormatContext *fmt_ctx;
    AVCodecContext *codec_ctx;
    AVFrame *frame;
    AVPacket *packet;
    int video_stream_index;
} DecodePrivate;

G_DEFINE_TYPE_WITH_PRIVATE(Decode, decode, G_TYPE_OBJECT)

static void decode_class_init(DecodeClass *klass) {
    // 在这里添加类的方法
}

static void decode_init(Decode *self) {
    DecodePrivate *priv = decode_get_instance_private(self);
    priv->fmt_ctx = NULL;
    priv->codec_ctx = NULL;
    priv->frame = av_frame_alloc();
    priv->packet = av_packet_alloc();
    priv->video_stream_index = -1;
}

void decode_initialize(Decode *self, const char *filename) {
    DecodePrivate *priv = decode_get_instance_private(self);

    if (avformat_open_input(&priv->fmt_ctx, filename, NULL, NULL) < 0) {
        g_error("无法打开输入文件");
        return;
    }

    if (avformat_find_stream_info(priv->fmt_ctx, NULL) < 0) {
        g_error("无法获取流信息");
        return;
    }

    for (unsigned int i = 0; i < priv->fmt_ctx->nb_streams; i++) {
        AVStream *stream = priv->fmt_ctx->streams[i];
        AVCodecParameters *codecpar = stream->codecpar;
        if (codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            priv->video_stream_index = i;
            AVCodec *codec = avcodec_find_decoder(codecpar->codec_id);
            if (!codec) {
                g_error("无法找到解码器");
                return;
            }
            priv->codec_ctx = avcodec_alloc_context3(codec);
            if (!priv->codec_ctx) {
                g_error("无法分配解码器上下文");
                return;
            }
            if (avcodec_parameters_to_context(priv->codec_ctx, codecpar) < 0) {
                g_error("无法复制解码器参数到上下文");
                return;
            }
            if (avcodec_open2(priv->codec_ctx, codec, NULL) < 0) {
                g_error("无法打开解码器");
                return;
            }
            break;
        }
    }

    if (priv->video_stream_index == -1) {
        g_error("没有找到视频流");
        return;
    }
}

AVFrame* decode_frame(Decode *self) {
    DecodePrivate *priv = decode_get_instance_private(self);

    while (av_read_frame(priv->fmt_ctx, priv->packet) >= 0) {
        if (priv->packet->stream_index == priv->video_stream_index) {
            if (avcodec_send_packet(priv->codec_ctx, priv->packet) < 0) {
                g_error("无法发送包到解码器");
                return NULL;
            }
            if (avcodec_receive_frame(priv->codec_ctx, priv->frame) == 0) {
                av_packet_unref(priv->packet);
                return priv->frame;
            }
        }
        av_packet_unref(priv->packet);
    }

    return NULL;
}

void decode_cleanup(Decode *self) {
    DecodePrivate *priv = decode_get_instance_private(self);

    if (priv->frame) {
        av_frame_free(&priv->frame);
    }
    if (priv->packet) {
        av_packet_free(&priv->packet);
    }
    if (priv->codec_ctx) {
        avcodec_free_context(&priv->codec_ctx);
    }
    if (priv->fmt_ctx) {
        avformat_close_input(&priv->fmt_ctx);
    }
}
