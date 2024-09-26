#ifndef DECODE_H
#define DECODE_H

#include <glib-object.h>
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/avutil.h"

G_BEGIN_DECLS

#define TYPE_DECODE (decode_get_type())
G_DECLARE_DERIVABLE_TYPE(Decode, decode, DECODE, GObject)

G_DEFINE_AUTOPTR_CLEANUP_FUNC(Decode, g_object_unref) // 添加这行

struct _DecodeClass {
    GObjectClass parent_class;
    // 在这里添加类的方法
};

void decode_initialize(Decode *self, const char *filename);
AVFrame* decode_frame(Decode *self);
void decode_cleanup(Decode *self);

G_END_DECLS

#endif
