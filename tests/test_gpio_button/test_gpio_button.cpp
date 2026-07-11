#include <stdio.h>
#include <gpiod.h>
#include "zipolib/z_time.h"

int main(void) {
    const char *chip_path = "/dev/gpiochip0";
    unsigned int offset = 26;

    struct gpiod_chip *chip = gpiod_chip_open(chip_path);
    struct gpiod_line_settings *settings = gpiod_line_settings_new();

    // Set line direction to INPUT and listen for both RISING and FALLING edges
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_INPUT);
    gpiod_line_settings_set_edge_detection(settings, GPIOD_LINE_EDGE_BOTH);
    gpiod_line_settings_set_bias(settings, GPIOD_LINE_BIAS_PULL_UP);
    struct gpiod_line_config *line_cfg = gpiod_line_config_new();
    gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings);

    struct gpiod_line_request *request = gpiod_chip_request_lines(chip, NULL, line_cfg);
    if (!request) {
        perror("Line request failed");
        return 1;
    }

    // Allocate an event buffer to efficiently fetch kernel events without reallocation
    struct gpiod_edge_event_buffer *buffer = gpiod_edge_event_buffer_new(16);

    printf("Waiting for edge events on GPIO %u...\n", offset);
    U64 ms_last_change=z_time_get_ticks_ms();
    U64 ms_debounce_threshold=50;

    enum gpiod_line_value val_last=gpiod_line_request_get_value(request, offset);
    enum gpiod_line_value val_steady=val_last;
    enum gpiod_line_value val_candidate=val_last;
    while (1) {

        int ret= gpiod_line_request_wait_edge_events(request,100000);
        if (ret < 0) {
            perror("Error reading edge events");
            break;
        }
        printf("ret= %d\n", ret);
                fflush(stdout);

        U64 now=z_time_get_ticks_ms();

        if (ret==0) {
            if (val_candidate==val_steady) {
                printf("val_steady=val_candidate %u\r", val_steady);

                continue;

            }
            printf("val_steady=%u val_candidate %u\r", val_steady,val_candidate);

            if ( now-ms_last_change > ms_debounce_threshold) {

                val_steady=val_candidate;
                printf("\nval_steady= %u\n", val_steady);
                fflush(stdout);
                continue;

            }


        }


        printf("calling gpiod_line_request_read_edge_events...\n");
                fflush(stdout);


        ret = gpiod_line_request_read_edge_events(request, buffer, 16);
        if (ret < 0) {
            perror("Error reading edge events");
            break;
        }
        printf("... gpiod_line_request_read_edge_events out\n");
                fflush(stdout);

        // This blocks until an interrupt event happens on the pin


        enum gpiod_line_value val = gpiod_line_request_get_value(request, offset);
            val_candidate=val;
            ms_last_change=now;
                printf("\nval_candidate= %u\n", val_candidate);
                fflush(stdout);

            continue;


        // Iterate through all intercepted events inside the buffer
        for (int i = 0; i < ret; i++) {
            struct gpiod_edge_event *event = gpiod_edge_event_buffer_get_event(buffer, i);
            enum gpiod_edge_event_type type = gpiod_edge_event_get_event_type(event);
            //printf("Event detected: %s\n",             type == GPIOD_EDGE_EVENT_RISING_EDGE ? "RISING (Pressed)" : "FALLING (Released)");
        }
    }

    gpiod_edge_event_buffer_free(buffer);
    gpiod_line_request_release(request);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);
    return 0;
}
