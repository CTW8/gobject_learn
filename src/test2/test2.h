#include <glib-object.h>

G_BEGIN_DECLS

#define TYPE_DEMUX_MP4 (demux_mp4_get_type())
G_DECLARE_FINAL_TYPE(DemuxMp4, demux_mp4, DEMUX, MP4, GObject)

struct _DemuxMp4 {
    GObject parent_instance;
    // 在这里添加类的属性
};

G_END_DECLS
