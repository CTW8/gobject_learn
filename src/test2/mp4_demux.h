#ifndef DEMUX_MP4_H
#define DEMUX_MP4_H

#include <glib-object.h>
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
#include "libavutil/samplefmt.h"
#include "libavutil/channel_layout.h"
#include "libavutil/timestamp.h"
#include "libavutil/imgutils.h"
#include "libavutil/parseutils.h"
#include "libavutil/opt.h"
#include "libavfilter/avfilter.h"
G_BEGIN_DECLS

#define TYPE_DEMUX_MP4 (demux_mp4_get_type())
G_DECLARE_DERIVABLE_TYPE(DemuxMp4, demux_mp4, DEMUX, MP4, GObject)

struct _DemuxMp4Class {
    GObjectClass parent_class;
    // 在这里添加类的方法

};

void demux_mp4_open(DemuxMp4 *self, const char *filename, int max_queue_size);
AVPacket* demux_mp4_read(DemuxMp4 *self);
void demux_mp4_seek(DemuxMp4 *self, int64_t timestamp);
void demux_mp4_close(DemuxMp4 *self);

typedef struct {
    int video_track_count;
    int audio_track_count;
    int64_t duration;
    int video_width;
    int video_height;
    double video_frame_rate;
    int64_t video_bit_rate;
    int audio_sample_rate;
    int audio_channels;
    char audio_sample_format[32];
    int video_index;
    int audio_index;
} DemuxMp4FileInfo;

void demux_mp4_get_file_info(DemuxMp4 *self, DemuxMp4FileInfo *info);


G_END_DECLS

#endif 