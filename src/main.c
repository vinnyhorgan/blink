#include "blink.h"

int main() {
    blink_Context *ctx = blink_create("Hello blink", 320, 240, BLINK_SCALE2X);

    while (blink_update(ctx)) {
        blink_clear(ctx, blink_rgb(255, 255, 255));

        blink_draw_point(ctx, 10, 10, blink_rgb(255, 0, 0));
        blink_draw_rect(ctx, blink_rect(50, 50, 50, 50), blink_rgb(0, 255, 0));
        blink_draw_line(ctx, 150, 100, 200, 200, blink_rgb(0, 0, 255));
    }

    blink_destroy(ctx);

    return 0;
}
