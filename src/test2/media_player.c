#include "media_player.h"

typedef struct {
    DemuxMp4 *demux;
    AudioSdlRender *audio_render;
    SdlRender *video_render;
    AudioDecode *audio_decode;
    VideoDecode *video_decode;
    gboolean is_playing;
} MediaPlayerPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(MediaPlayer, media_player, G_TYPE_OBJECT)

static void media_player_class_init(MediaPlayerClass *klass) {
    // 在这里添加类的方法
}

static void media_player_init(MediaPlayer *self) {
    MediaPlayerPrivate *priv = media_player_get_instance_private(self);
    priv->demux = g_object_new(TYPE_DEMUX_MP4, NULL);
    priv->audio_render = g_object_new(TYPE_AUDIO_SDL_RENDER, NULL);
    priv->video_render = g_object_new(TYPE_SDL_RENDER, NULL);
    priv->audio_decode = g_object_new(TYPE_AUDIO_DECODE, NULL);
    priv->video_decode = g_object_new(TYPE_VIDEO_DECODE, NULL);
    priv->is_playing = FALSE;
}

void media_player_initialize(MediaPlayer *self, const char *filename) {
    MediaPlayerPrivate *priv = media_player_get_instance_private(self);
    demux_mp4_open(priv->demux, filename, 500);
    DemuxMp4FileInfo info;
    demux_mp4_get_file_info(priv->demux, &info);
    audio_sdl_render_initialize(priv->audio_render, priv->audio_decode);
    sdl_render_initialize(priv->video_render, priv->video_decode, info.video_width, info.video_height);
}

void media_player_play(MediaPlayer *self) {
    MediaPlayerPrivate *priv = media_player_get_instance_private(self);
    if (!priv->is_playing) {
        priv->is_playing = TRUE;
        audio_sdl_render_play(priv->audio_render);
        sdl_render_start(priv->video_render);
    }
}

void media_player_pause(MediaPlayer *self) {
    MediaPlayerPrivate *priv = media_player_get_instance_private(self);
    if (priv->is_playing) {
        priv->is_playing = FALSE;
        audio_sdl_render_stop(priv->audio_render);
        sdl_render_stop(priv->video_render);
    }
}

void media_player_stop(MediaPlayer *self) {
    MediaPlayerPrivate *priv = media_player_get_instance_private(self);
    if (priv->is_playing) {
        priv->is_playing = FALSE;
        audio_sdl_render_stop(priv->audio_render);
        sdl_render_stop(priv->video_render);
        demux_mp4_close(priv->demux);
    }
}

void media_player_seek(MediaPlayer *self, int64_t timestamp) {
    MediaPlayerPrivate *priv = media_player_get_instance_private(self);
    demux_mp4_seek(priv->demux, timestamp);
}
