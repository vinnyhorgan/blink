#include "blink.h"

int main() {
    blink_Context *ctx = blink_create("Hello blink", 320, 240, 2);

    blink_Image *squinkle = blink_load_image_file("assets/squinkle.png");

    double dt;
    while (blink_update(ctx, &dt)) {
        blink_clear(ctx, blink_rgb(255, 255, 255));

        blink_draw_point(ctx, 10, 10, blink_rgb(255, 0, 0));
        blink_draw_rect(ctx, blink_rect(50, 50, 50, 50), blink_rgb(0, 255, 0));
        blink_draw_line(ctx, 150, 100, 200, 200, blink_rgb(0, 0, 255));

        blink_draw_image(ctx, squinkle, 100, 100);
        blink_draw_text(ctx, "Hello blink!", 10, 10, BLINK_BLACK);

        if (blink_mouse_down(ctx, 1)) {
            blink_draw_text(ctx, "Mouse down!", 10, 30, BLINK_BLACK);
        }
    }

    blink_destroy(ctx);

    return 0;
}
