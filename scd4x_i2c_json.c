/*
 * Read sensor, write to json file, wait 5 secs, repeat.
 *
 * Values can be trivially extracted from the file using the 'jq'
 * utility or built-in JSON support of pretty much any script
 * languange out there.
 *
 * (c) Gerd Hoffmann <kraxel@redhat.com>
 *
 * Based on scd4x_i2c_example_usage.c which is ...
 *
 * -----------------------------------------------------------------------
 *
 * Copyright (c) 2021, Sensirion AG
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of Sensirion AG nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* ---------------------- systemd unit example -------------------------
[Unit]
Description=scd4x sensor data (read)

[Service]
Type=simple
Restart=always
User=pi
Group=i2c
ExecStart=/home/pi/projects/raspberry-pi-i2c-scd4x/scd4x_i2c_json /var/lib/scd/values.json

[Install]
WantedBy=multi-user.target
----------------------------------------------------------------------- */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "scd4x_i2c.h"
#include "sensirion_common.h"
#include "sensirion_i2c_hal.h"

int main(int argc, char *argv[])
{
    char *outfile;
    char *tmpfile;
    size_t tmpsize;
    int16_t error = 0;
    int fd;
    FILE *fp;

    uint16_t serial_0;
    uint16_t serial_1;
    uint16_t serial_2;
    uint16_t co2;
    float temperature;
    float humidity;

    if (argc < 2) {
        fprintf(stderr, "usage: %s <json-file>\n", argv[0]);
        exit(1);
    }
    outfile = argv[1];
    tmpsize = strlen(outfile) + 16;
    tmpfile = malloc(tmpsize);

    sensirion_i2c_hal_init();

    // Clean up potential SCD40 states
    scd4x_wake_up();
    scd4x_stop_periodic_measurement();
    scd4x_reinit();

    error = scd4x_get_serial_number(&serial_0, &serial_1, &serial_2);
    if (error) {
        fprintf(stderr, "Error executing scd4x_get_serial_number(): %i\n", error);
        exit(1);
    }

    error = scd4x_start_periodic_measurement();
    if (error) {
        fprintf(stderr, "Error executing scd4x_start_periodic_measurement(): %i\n",
                error);
        exit(1);
    }

    for (;;) {
        sensirion_i2c_hal_sleep_usec(5000000);
        error = scd4x_read_measurement(&co2, &temperature, &humidity);

        if (error) {
            fprintf(stderr, "Error executing scd4x_read_measurement(): %i\n", error);
            continue;
        }
        if (co2 == 0) {
            fprintf(stderr, "Invalid sample detected, skipping.\n");
            continue;
        }

        snprintf(tmpfile, tmpsize, "%s.XXXXXXXXXX", outfile);
        fd = mkstemp(tmpfile);
        if (fd < 0) {
            perror("mkstemp");
            exit(1);
        }

        fp = fdopen(fd, "w");
        if (NULL == fp) {
            perror("fopen");
            exit(1);
        }

        fprintf(fp, "{"
                " \"serial\" : \"%04x%04x%04x\","
                " \"timestamp\" : %lu,"
                " \"co2\" : %u,"
                " \"temperature\" : %.1f,"
                " \"humidity\" : %.1f "
                "}\n",
                serial_0, serial_1, serial_2,
                (unsigned long)time(NULL),
                co2, temperature, humidity);

        fclose(fp);
        rename(tmpfile, outfile);
    }
    return 0;
}
