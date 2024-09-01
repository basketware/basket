#include "vala.h"
#include "glib.h"

// Application
    G_DEFINE_TYPE(BasketEngineApp, basket_engine_app, G_TYPE_OBJECT)

    static int
    basket_engine_app_frame_default(BasketEngineApp *self, double alpha, double delta)
    {
        (void)self, (void)alpha, (void)delta;
        return 0;
    }

    static int
    basket_engine_app_tick_default(BasketEngineApp *self, double delta)
    {
        (void)self, (void)delta;
        return 0;
    }

    static int
    basket_engine_app_close_default(BasketEngineApp *self)
    {
        (void)self;
        return 0;
    }

    static void
    basket_engine_app_class_init(BasketEngineAppClass *klass)
    {
        klass->frame = basket_engine_app_frame_default;
        klass->tick = basket_engine_app_tick_default;
        klass->close = basket_engine_app_close_default;
    }

    static void
    basket_engine_app_init(BasketEngineApp *self)
    {
        (void)self;
    }

    BasketEngineApp*
    basket_engine_app_new(void)
    {
        return g_object_new(BASKET_ENGINE_TYPE_APP, NULL);
    }

    int
    basket_engine_app_run(BasketEngineApp *self)
    {
        BasketEngineAppClass *klass = BASKET_ENGINE_APP_GET_CLASS(self);

        Application raw = {
            .userdata = self,
            .frame = (int (*)(void*, double, double)) klass->frame,
            .tick  = (int (*)(void*, double))         klass->tick,
            .close = (int (*)(void*, int))            klass->close,
        };

        return eng_main(raw, klass->headless);
    }

GBytes *basket_filesystem_read(const char *name) {
    u32 size = 0;
    char *data = fs_read(name, &size);

    if (data == NULL)
        return NULL;

    return g_bytes_new_from_bytes((void *)data, 0, size);
}

gboolean basket_save_store(GBytes *bytes) {
    gsize size;
    const char *mem = g_bytes_get_data(bytes, &size);
    return sav_store(mem, size);
}

GBytes *basket_save_retrieve(void) {
    u32 size = 0;
    char *data = sav_retrieve(&size);

    if (data == NULL)
        return NULL;

    return g_bytes_new_from_bytes((void *)data, 0, size);
}

void basket_image_init_from_bytes (GBytes *bytes, Image *out, GError **error) {
    gsize size;
    const char *mem = g_bytes_get_data(bytes, &size);
    img_init(out, mem, size);
}
