#ifndef MEDIA_PLAYER_H
#define MEDIA_PLAYER_H

#include <glib-object.h>
#include "audio_sdl_render.h"
#include "sdl_render.h"
#include "mp4_demux.h"
#include "audio_decode.h"
#include "video_decode.h"

G_BEGIN_DECLS

#define TYPE_MEDIA_PLAYER (media_player_get_type())
G_DECLARE_DERIVABLE_TYPE(MediaPlayer, media_player, MEDIA, PLAYER, GObject)

struct _MediaPlayerClass {
    GObjectClass parent_class;
    // 在这里添加类的方法
};

void media_player_initialize(MediaPlayer *self, const char *filename);
void media_player_play(MediaPlayer *self);
void media_player_pause(MediaPlayer *self);
void media_player_stop(MediaPlayer *self);
void media_player_seek(MediaPlayer *self, int64_t timestamp);

G_END_DECLS

#endif // MEDIA_PLAYER_H
