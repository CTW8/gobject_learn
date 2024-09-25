#include "test2.h"

G_DEFINE_TYPE(DemuxMp4, demux_mp4, G_TYPE_OBJECT)

static void demux_mp4_init(DemuxMp4 *self) {
    // 初始化代码
}

static void demux_mp4_class_init(DemuxMp4Class *klass) {
    // 类初始化代码

}

DemuxMp4 *demux_mp4_new() {
    return g_object_new(TYPE_DEMUX_MP4, NULL);
}

void demux_mp4_set_value(DemuxMp4 *self, int value) {
    g_object_set(self, "demux-mp4-value", value, NULL);
}

int demux_mp4_get_value(DemuxMp4 *self) {
    return 0;
}

