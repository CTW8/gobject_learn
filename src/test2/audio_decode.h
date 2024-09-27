#ifndef AUDIO_DECODE_H
#define AUDIO_DECODE_H

#include <glib-object.h>
#include "mp4_demux.h"
#include "libavcodec/avcodec.h"

G_BEGIN_DECLS

#define TYPE_AUDIO_DECODE (audio_decode_get_type())
G_DECLARE_DERIVABLE_TYPE(AudioDecode, audio_decode, AUDIO, DECODE, GObject)

struct _AudioDecodeClass {
    GObjectClass parent_class;
    // 在这里添加类的方法
};

void audio_decode_initialize(AudioDecode *self, DemuxMp4 *demux, int max_queue_size);
AVFrame* audio_decode_get_frame(AudioDecode *self);
void audio_decode_cleanup(AudioDecode *self);
void audio_decode_start(AudioDecode *self);
void audio_decode_stop(AudioDecode *self);
void audio_decode_flush(AudioDecode *self);  // 添加flush函数声明

G_END_DECLS

#endif
