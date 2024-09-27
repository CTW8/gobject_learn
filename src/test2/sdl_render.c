#include "sdl_render.h"
#include <pthread.h>

typedef struct {
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    int width;
    int height;
    VideoDecode *video_decode;
    pthread_t render_thread;
    gboolean stop_thread;
    pthread_mutex_t render_mutex;
} SdlRenderPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(SdlRender, sdl_render, G_TYPE_OBJECT)

static void* sdl_render_thread_func(void *data) {
    SdlRender *self = SDL_RENDER(data);
    SdlRenderPrivate *priv = sdl_render_get_instance_private(self);

    while (!priv->stop_thread) {
        AVFrame *frame = video_decode_get_frame(priv->video_decode);
        if (frame) {
            SDL_UpdateYUVTexture(priv->texture, NULL,
                                 frame->data[0], frame->linesize[0],
                                 frame->data[1], frame->linesize[1],
                                 frame->data[2], frame->linesize[2]);

            SDL_RenderClear(priv->renderer);
            SDL_RenderCopy(priv->renderer, priv->texture, NULL, NULL);
            SDL_RenderPresent(priv->renderer);

            av_frame_free(&frame);
        }
    }

    return NULL;
}

static void sdl_render_class_init(SdlRenderClass *klass) {
    // 在这里添加类的方法
}

static void sdl_render_init(SdlRender *self) {
    SdlRenderPrivate *priv = sdl_render_get_instance_private(self);
    priv->window = NULL;
    priv->renderer = NULL;
    priv->texture = NULL;
    priv->width = 0;
    priv->height = 0;
    priv->video_decode = NULL;
    priv->stop_thread = FALSE;
    pthread_mutex_init(&priv->render_mutex, NULL);
}

void sdl_render_initialize(SdlRender *self, VideoDecode *video_decode, int width, int height) {
    SdlRenderPrivate *priv = sdl_render_get_instance_private(self);
    priv->width = width;
    priv->height = height;
    priv->video_decode = video_decode;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        g_error("Could not initialize SDL - %s\n", SDL_GetError());
        return;
    }

    priv->window = SDL_CreateWindow("SDL Render",
                                    SDL_WINDOWPOS_UNDEFINED,
                                    SDL_WINDOWPOS_UNDEFINED,
                                    width, height,
                                    SDL_WINDOW_SHOWN);
    if (!priv->window) {
        g_error("Could not create window - %s\n", SDL_GetError());
        return;
    }

    priv->renderer = SDL_CreateRenderer(priv->window, -1, SDL_RENDERER_ACCELERATED);
    if (!priv->renderer) {
        g_error("Could not create renderer - %s\n", SDL_GetError());
        return;
    }

    priv->texture = SDL_CreateTexture(priv->renderer,
                                      SDL_PIXELFORMAT_YV12,
                                      SDL_TEXTUREACCESS_STREAMING,
                                      width, height);
    if (!priv->texture) {
        g_error("Could not create texture - %s\n", SDL_GetError());
        return;
    }

    pthread_create(&priv->render_thread, NULL, sdl_render_thread_func, self);
}

void sdl_render_start(SdlRender *self) {
    SdlRenderPrivate *priv = sdl_render_get_instance_private(self);
    pthread_mutex_lock(&priv->render_mutex);
    priv->stop_thread = FALSE;
    pthread_mutex_unlock(&priv->render_mutex);
}

void sdl_render_stop(SdlRender *self) {
    SdlRenderPrivate *priv = sdl_render_get_instance_private(self);
    pthread_mutex_lock(&priv->render_mutex);
    priv->stop_thread = TRUE;
    pthread_mutex_unlock(&priv->render_mutex);
    pthread_join(priv->render_thread, NULL);
}

void sdl_render_cleanup(SdlRender *self) {
    SdlRenderPrivate *priv = sdl_render_get_instance_private(self);
    sdl_render_stop(self);

    if (priv->texture) {
        SDL_DestroyTexture(priv->texture);
        priv->texture = NULL;
    }

    if (priv->renderer) {
        SDL_DestroyRenderer(priv->renderer);
        priv->renderer = NULL;
    }

    if (priv->window) {
        SDL_DestroyWindow(priv->window);
        priv->window = NULL;
    }

    SDL_Quit();
    pthread_mutex_destroy(&priv->render_mutex);
}
