#include "led-matrix-c.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>

#define LED_ON_TIME 15
#define LED_OFF_TIME 20

volatile bool interrupt_received = false;
static void InterruptHandler(int signo)
{
    interrupt_received = true;
}

int main(int argc, char **argv)
{
    struct RGBLedMatrixOptions options;
    struct RGBLedMatrix *matrix;
    struct LedCanvas *offscreen_canvas;
    int width, height;
    int counter;

    memset(&options, 0, sizeof(options));
    options.rows = 64;
    options.cols = 64;
    options.chain_length = 1;

    /* This supports all the led commandline options. Try --led-help */
    matrix = led_matrix_create_from_options(&options, &argc, &argv);
    if (matrix == NULL)
        return 1;

    /* Let's do an example with double-buffering. We create one extra
     * buffer onto which we draw, which is then swapped on each refresh.
     * This is typically a good aproach for animations and such.
     */
    offscreen_canvas = led_matrix_create_offscreen_canvas(matrix);

    led_canvas_get_size(offscreen_canvas, &width, &height);

    fprintf(stderr, "Size: %dx%d. Hardware gpio mapping: %s\n",
            width, height, options.hardware_mapping);
    struct color_t
    {
        uint8_t R;
        uint8_t G;
        uint8_t B;
    };
    int nColors = 8;
    struct color_t colors[nColors];
    struct color_t color1 = {255, 0, 0};
    struct color_t color2 = {0, 255, 0};
    struct color_t color3 = {0, 255, 255};
    struct color_t color4 = {0, 0, 255};
    struct color_t color5 = {255, 0, 255};
    struct color_t color6 = {255, 255, 255};
    struct color_t color7 = {122, 122, 255};
    struct color_t color8 = {122, 255, 122};

    colors[0] = color1;
    colors[1] = color2;
    colors[2] = color3;
    colors[3] = color4;
    colors[4] = color5;
    colors[5] = color6;
    colors[6] = color7;
    colors[7] = color8;
    counter = 0;
    signal(SIGTERM, InterruptHandler);
    signal(SIGINT, InterruptHandler);
    while (!interrupt_received)
    {
        for (int i = 0; i < 1000; ++i)
        {
            struct color_t currentColor = colors[counter];
            led_canvas_fill(offscreen_canvas, 0, 0, 0);
            if (0 == i % LED_OFF_TIME)
            {
                for (int j = 0; j < LED_ON_TIME; j++)
                {
                    led_canvas_fill(offscreen_canvas, currentColor.R, currentColor.G, currentColor.B);
                    offscreen_canvas = led_matrix_swap_on_vsync(matrix, offscreen_canvas);
                }
                counter++;
                if (counter >= nColors)
                {
                    counter = 0;
                }
            }

            /* Now, we swap the canvas. We give swap_on_vsync the buffer we
             * just have drawn into, and wait until the next vsync happens.
             * we get back the unused buffer to which we'll draw in the next
             * iteration.
             */
            offscreen_canvas = led_matrix_swap_on_vsync(matrix, offscreen_canvas);
        }
    }

    /*
     * Make sure to always call led_matrix_delete() in the end to reset the
     * display. Installing signal handlers for defined exit is a good idea.
     */
    led_matrix_delete(matrix);
    fprintf(stdout, "\ninterrupt received, shutting down...\n");

    return 0;
}
