#ifndef BASKET_ENGINE_APP_H
#define BASKET_ENGINE_APP_H

#include <glib-object.h>
#include "basket.h"

G_BEGIN_DECLS

// APPLICATION
    #define BASKET_ENGINE_TYPE_APP (basket_engine_app_get_type())
    G_DECLARE_DERIVABLE_TYPE(BasketEngineApp, basket_engine_app, BASKET_ENGINE, APP, GObject)

    struct _BasketEngineAppClass {
        GObjectClass parent_class;

        gboolean headless;

        int (*frame) (BasketEngineApp *self, double alpha, double delta);
        int (*tick) (BasketEngineApp *self, double delta);
        int (*close) (BasketEngineApp *self);
    };

    BasketEngineApp* basket_engine_app_new(void);
    int basket_engine_app_run(BasketEngineApp *self);

// IMAGE
    #define BASKET_ENGINE_TYPE_IMG (basket_engine_app_get_type())
    G_DECLARE_DERIVABLE_TYPE(BasketEngineImage, basket_engine_img, BASKET_ENGINE, IMG, GObject)

    struct _BasketEngineImageClass {
        GObjectClass parent_class;
        Image image;
    };

    BasketEngineApp* basket_engine_image_new(void);
    int basket_engine_image_free(BasketEngineApp *self);

G_END_DECLS

GBytes *basket_filesystem_read(const char *name);

gboolean basket_save_store(GBytes *bytes);
GBytes *basket_save_retrieve(void);

#endif /* BASKET_ENGINE_APP_H */
