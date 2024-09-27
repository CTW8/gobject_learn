#ifndef VIDEO_DECODE_H
#define VIDEO_DECODE_H

#include <glib-object.h>
#include "mp4_demux.h"
#include "libavcodec/avcodec.h"

G_BEGIN_DECLS

#define TYPE_VIDEO_DECODE (video_decode_get_type())
G_DECLARE_DERIVABLE_TYPE(VideoDecode, video_decode, VIDEO, DECODE, GObject)

struct _VideoDecodeClass {
    GObjectClass parent_class;
    // 在这里添加类的方法
};

void video_decode_initialize(VideoDecode *self, DemuxMp4 *demux, int max_queue_size);
void video_decode_decode_packet(VideoDecode *self);
AVFrame* video_decode_get_frame(VideoDecode *self);
void video_decode_cleanup(VideoDecode *self);
void video_decode_flush(VideoDecode *self);  // 添加flush函数声明

G_END_DECLS

#endif
