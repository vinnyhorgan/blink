#include "blink.h"

int main() {
    blink_Context *ctx = blink_create("Hello blink", 640, 480);

    while (blink_update(ctx)) {}

    blink_destroy(ctx);

    return 0;
}
