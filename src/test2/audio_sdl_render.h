#ifndef AUDIO_SDL_RENDER_H
#define AUDIO_SDL_RENDER_H

#include <glib-object.h>
#include <SDL2/SDL.h>
#include "audio_decode.h"

G_BEGIN_DECLS

#define TYPE_AUDIO_SDL_RENDER (audio_sdl_render_get_type())
G_DECLARE_DERIVABLE_TYPE(AudioSdlRender, audio_sdl_render, AUDIO, SDL_RENDER, GObject)

struct _AudioSdlRenderClass {
    GObjectClass parent_class;
    // 在这里添加类的方法
};

void audio_sdl_render_initialize(AudioSdlRender *self, AudioDecode *audio_decode);
void audio_sdl_render_play(AudioSdlRender *self);
void audio_sdl_render_stop(AudioSdlRender *self);

G_END_DECLS

#endif // AUDIO_SDL_RENDER_H
