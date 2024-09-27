#ifndef SDL_RENDER_H
#define SDL_RENDER_H

#include <glib-object.h>
#include <SDL2/SDL.h>
#include "libavcodec/avcodec.h"
#include "video_decode.h"

G_BEGIN_DECLS

#define TYPE_SDL_RENDER (sdl_render_get_type())
G_DECLARE_DERIVABLE_TYPE(SdlRender, sdl_render, SDL, RENDER, GObject)

struct _SdlRenderClass {
    GObjectClass parent_class;
    // 在这里添加类的方法
};

void sdl_render_initialize(SdlRender *self, VideoDecode *video_decode, int width, int height);
void sdl_render_start(SdlRender *self);
void sdl_render_stop(SdlRender *self);
void sdl_render_cleanup(SdlRender *self);

G_END_DECLS

#endif
