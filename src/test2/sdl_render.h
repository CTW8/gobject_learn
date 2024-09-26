#ifndef SDL_RENDER_H
#define SDL_RENDER_H

#include <glib-object.h>
#include <SDL2/SDL.h>
#include "libavcodec/avcodec.h"

G_BEGIN_DECLS

#define TYPE_SDL_RENDER (sdl_render_get_type())
G_DECLARE_DERIVABLE_TYPE(SdlRender, sdl_render, SDL, RENDER, GObject)

struct _SdlRenderClass {
    GObjectClass parent_class;
    // 在这里添加类的方法
};

void sdl_render_initialize(SdlRender *self, int width, int height);
void sdl_render_display_frame(SdlRender *self, AVFrame *frame);
void sdl_render_cleanup(SdlRender *self);

G_END_DECLS

#endif
