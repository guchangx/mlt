/*
 * Copyright (c) 2022-2025 Meltytech, LLC
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "image_proc.h"

#include <framework/mlt_log.h>
#include <framework/mlt_slices.h>

#include <math.h>
#include <stdlib.h>

typedef struct
{
    mlt_image src;
    mlt_image dst;
    int radius;
} blur_slice_desc;

static int blur_h_proc_rgba(int id, int index, int jobs, void *data)
{
    (void) id; // unused
    blur_slice_desc *desc = ((blur_slice_desc *) data);
    int slice_line_start,
        slice_height = mlt_slices_size_slice(jobs, index, desc->src->height, &slice_line_start);
    int slice_line_end = slice_line_start + slice_height;
    int accumulator[] = {0, 0, 0, 0};
    int x = 0;
    int y = 0;
    int step = 4;
    int linesize = step * desc->src->width;
    int radius = desc->radius;

    if (desc->radius > (desc->src->width / 2)) {
        radius = desc->src->width / 2;
    }
    double diameter = (radius * 2) + 1;

    for (y = slice_line_start; y < slice_line_end; y++) {
        uint8_t *first = desc->src->planes[0] + (y * linesize);
        uint8_t *last = first + linesize - step;
        uint8_t *s1 = first;
        uint8_t *s2 = first;
        uint8_t *d = desc->dst->planes[0] + (y * linesize);
        accumulator[0] = first[0] * (radius + 1);
        accumulator[1] = first[1] * (radius + 1);
        accumulator[2] = first[2] * (radius + 1);
        accumulator[3] = first[3] * (radius + 1);

        for (x = 0; x < radius; x++) {
            accumulator[0] += s1[0];
            accumulator[1] += s1[1];
            accumulator[2] += s1[2];
            accumulator[3] += s1[3];
            s1 += step;
        }
        for (x = 0; x <= radius; x++) {
            accumulator[0] += s1[0] - first[0];
            accumulator[1] += s1[1] - first[1];
            accumulator[2] += s1[2] - first[2];
            accumulator[3] += s1[3] - first[3];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            d[3] = lrint((double) accumulator[3] / diameter);
            s1 += step;
            d += step;
        }
        for (x = radius + 1; x < desc->src->width - radius; x++) {
            accumulator[0] += s1[0] - s2[0];
            accumulator[1] += s1[1] - s2[1];
            accumulator[2] += s1[2] - s2[2];
            accumulator[3] += s1[3] - s2[3];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            d[3] = lrint((double) accumulator[3] / diameter);
            s1 += step;
            s2 += step;
            d += step;
        }
        for (x = desc->src->width - radius; x < desc->src->width; x++) {
            accumulator[0] += last[0] - s2[0];
            accumulator[1] += last[1] - s2[1];
            accumulator[2] += last[2] - s2[2];
            accumulator[3] += last[3] - s2[3];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            d[3] = lrint((double) accumulator[3] / diameter);
            s2 += step;
            d += step;
        }
    }
    return 0;
}

static int blur_v_proc_rgba(int id, int index, int jobs, void *data)
{
    (void) id; // unused
    blur_slice_desc *desc = ((blur_slice_desc *) data);
    int slice_row_start,
        slice_width = mlt_slices_size_slice(jobs, index, desc->src->width, &slice_row_start);
    int slice_row_end = slice_row_start + slice_width;
    int accumulator[] = {0, 0, 0, 0};
    int x = 0;
    int y = 0;
    int step = 4;
    int linesize = step * desc->src->width;
    int radius = desc->radius;

    if (desc->radius > (desc->src->height / 2)) {
        radius = desc->src->height / 2;
    }
    double diameter = (radius * 2) + 1;

    for (x = slice_row_start; x < slice_row_end; x++) {
        uint8_t *first = desc->src->planes[0] + (x * step);
        uint8_t *last = first + (linesize * (desc->src->height - 1));
        uint8_t *s1 = first;
        uint8_t *s2 = first;
        uint8_t *d = desc->dst->planes[0] + (x * step);
        accumulator[0] = first[0] * (radius + 1);
        accumulator[1] = first[1] * (radius + 1);
        accumulator[2] = first[2] * (radius + 1);
        accumulator[3] = first[3] * (radius + 1);

        for (y = 0; y < radius; y++) {
            accumulator[0] += s1[0];
            accumulator[1] += s1[1];
            accumulator[2] += s1[2];
            accumulator[3] += s1[3];
            s1 += linesize;
        }
        for (y = 0; y <= radius; y++) {
            accumulator[0] += s1[0] - first[0];
            accumulator[1] += s1[1] - first[1];
            accumulator[2] += s1[2] - first[2];
            accumulator[3] += s1[3] - first[3];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            d[3] = lrint((double) accumulator[3] / diameter);
            s1 += linesize;
            d += linesize;
        }
        for (y = radius + 1; y < desc->src->height - radius; y++) {
            accumulator[0] += s1[0] - s2[0];
            accumulator[1] += s1[1] - s2[1];
            accumulator[2] += s1[2] - s2[2];
            accumulator[3] += s1[3] - s2[3];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            d[3] = lrint((double) accumulator[3] / diameter);
            s1 += linesize;
            s2 += linesize;
            d += linesize;
        }
        for (y = desc->src->height - radius; y < desc->src->height; y++) {
            accumulator[0] += last[0] - s2[0];
            accumulator[1] += last[1] - s2[1];
            accumulator[2] += last[2] - s2[2];
            accumulator[3] += last[3] - s2[3];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            d[3] = lrint((double) accumulator[3] / diameter);
            s2 += linesize;
            d += linesize;
        }
    }
    return 0;
}

static int blur_h_proc_rgbx(int id, int index, int jobs, void *data)
{
    (void) id; // unused
    blur_slice_desc *desc = ((blur_slice_desc *) data);
    int slice_line_start,
        slice_height = mlt_slices_size_slice(jobs, index, desc->src->height, &slice_line_start);
    int slice_line_end = slice_line_start + slice_height;
    int accumulator[] = {0, 0, 0};
    int x = 0;
    int y = 0;
    int step = 4;
    int linesize = step * desc->src->width;
    int radius = desc->radius;

    if (desc->radius > (desc->src->width / 2)) {
        radius = desc->src->width / 2;
    }
    double diameter = (radius * 2) + 1;

    for (y = slice_line_start; y < slice_line_end; y++) {
        uint8_t *first = desc->src->planes[0] + (y * linesize);
        uint8_t *last = first + linesize - step;
        uint8_t *s1 = first;
        uint8_t *s2 = first;
        uint8_t *d = desc->dst->planes[0] + (y * linesize);
        accumulator[0] = first[0] * (radius + 1);
        accumulator[1] = first[1] * (radius + 1);
        accumulator[2] = first[2] * (radius + 1);

        for (x = 0; x < radius; x++) {
            accumulator[0] += s1[0];
            accumulator[1] += s1[1];
            accumulator[2] += s1[2];
            s1 += step;
        }
        for (x = 0; x <= radius; x++) {
            accumulator[0] += s1[0] - first[0];
            accumulator[1] += s1[1] - first[1];
            accumulator[2] += s1[2] - first[2];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            s1 += step;
            d += step;
        }
        for (x = radius + 1; x < desc->src->width - radius; x++) {
            accumulator[0] += s1[0] - s2[0];
            accumulator[1] += s1[1] - s2[1];
            accumulator[2] += s1[2] - s2[2];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            s1 += step;
            s2 += step;
            d += step;
        }
        for (x = desc->src->width - radius; x < desc->src->width; x++) {
            accumulator[0] += last[0] - s2[0];
            accumulator[1] += last[1] - s2[1];
            accumulator[2] += last[2] - s2[2];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            s2 += step;
            d += step;
        }
    }
    return 0;
}

static int blur_v_proc_rgbx(int id, int index, int jobs, void *data)
{
    (void) id; // unused
    blur_slice_desc *desc = ((blur_slice_desc *) data);
    int slice_row_start,
        slice_width = mlt_slices_size_slice(jobs, index, desc->src->width, &slice_row_start);
    int slice_row_end = slice_row_start + slice_width;
    int accumulator[] = {0, 0, 0};
    int x = 0;
    int y = 0;
    int step = 4;
    int linesize = step * desc->src->width;
    int radius = desc->radius;

    if (desc->radius > (desc->src->height / 2)) {
        radius = desc->src->height / 2;
    }
    double diameter = (radius * 2) + 1;

    for (x = slice_row_start; x < slice_row_end; x++) {
        uint8_t *first = desc->src->planes[0] + (x * step);
        uint8_t *last = first + (linesize * (desc->src->height - 1));
        uint8_t *s1 = first;
        uint8_t *s2 = first;
        uint8_t *d = desc->dst->planes[0] + (x * step);
        accumulator[0] = first[0] * (radius + 1);
        accumulator[1] = first[1] * (radius + 1);
        accumulator[2] = first[2] * (radius + 1);

        for (y = 0; y < radius; y++) {
            accumulator[0] += s1[0];
            accumulator[1] += s1[1];
            accumulator[2] += s1[2];
            s1 += linesize;
        }
        for (y = 0; y <= radius; y++) {
            accumulator[0] += s1[0] - first[0];
            accumulator[1] += s1[1] - first[1];
            accumulator[2] += s1[2] - first[2];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            s1 += linesize;
            d += linesize;
        }
        for (y = radius + 1; y < desc->src->height - radius; y++) {
            accumulator[0] += s1[0] - s2[0];
            accumulator[1] += s1[1] - s2[1];
            accumulator[2] += s1[2] - s2[2];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            s1 += linesize;
            s2 += linesize;
            d += linesize;
        }
        for (y = desc->src->height - radius; y < desc->src->height; y++) {
            accumulator[0] += last[0] - s2[0];
            accumulator[1] += last[1] - s2[1];
            accumulator[2] += last[2] - s2[2];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            s2 += linesize;
            d += linesize;
        }
    }
    return 0;
}

static int blur_h_proc_rgba64(int id, int index, int jobs, void *data)
{
    (void) id; // unused
    blur_slice_desc *desc = ((blur_slice_desc *) data);
    int slice_line_start,
        slice_height = mlt_slices_size_slice(jobs, index, desc->src->height, &slice_line_start);
    int slice_line_end = slice_line_start + slice_height;
    int accumulator[] = {0, 0, 0, 0};
    int x = 0;
    int y = 0;
    int step = 4;
    int linesize = step * desc->src->width;
    int radius = desc->radius;

    if (desc->radius > (desc->src->width / 2)) {
        radius = desc->src->width / 2;
    }
    double diameter = (radius * 2) + 1;

    for (y = slice_line_start; y < slice_line_end; y++) {
        uint16_t *first = (uint16_t *) desc->src->planes[0] + (y * linesize);
        uint16_t *last = first + linesize - step;
        uint16_t *s1 = first;
        uint16_t *s2 = first;
        uint16_t *d = (uint16_t *) desc->dst->planes[0] + (y * linesize);
        accumulator[0] = first[0] * (radius + 1);
        accumulator[1] = first[1] * (radius + 1);
        accumulator[2] = first[2] * (radius + 1);
        accumulator[3] = first[3] * (radius + 1);

        for (x = 0; x < radius; x++) {
            accumulator[0] += s1[0];
            accumulator[1] += s1[1];
            accumulator[2] += s1[2];
            accumulator[3] += s1[3];
            s1 += step;
        }
        for (x = 0; x <= radius; x++) {
            accumulator[0] += s1[0] - first[0];
            accumulator[1] += s1[1] - first[1];
            accumulator[2] += s1[2] - first[2];
            accumulator[3] += s1[3] - first[3];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            d[3] = lrint((double) accumulator[3] / diameter);
            s1 += step;
            d += step;
        }
        for (x = radius + 1; x < desc->src->width - radius; x++) {
            accumulator[0] += s1[0] - s2[0];
            accumulator[1] += s1[1] - s2[1];
            accumulator[2] += s1[2] - s2[2];
            accumulator[3] += s1[3] - s2[3];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            d[3] = lrint((double) accumulator[3] / diameter);
            s1 += step;
            s2 += step;
            d += step;
        }
        for (x = desc->src->width - radius; x < desc->src->width; x++) {
            accumulator[0] += last[0] - s2[0];
            accumulator[1] += last[1] - s2[1];
            accumulator[2] += last[2] - s2[2];
            accumulator[3] += last[3] - s2[3];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            d[3] = lrint((double) accumulator[3] / diameter);
            s2 += step;
            d += step;
        }
    }
    return 0;
}

static int blur_v_proc_rgba64(int id, int index, int jobs, void *data)
{
    (void) id; // unused
    blur_slice_desc *desc = ((blur_slice_desc *) data);
    int slice_row_start,
        slice_width = mlt_slices_size_slice(jobs, index, desc->src->width, &slice_row_start);
    int slice_row_end = slice_row_start + slice_width;
    int accumulator[] = {0, 0, 0, 0};
    int x = 0;
    int y = 0;
    int step = 4;
    int linesize = step * desc->src->width;
    int radius = desc->radius;

    if (desc->radius > (desc->src->height / 2)) {
        radius = desc->src->height / 2;
    }
    double diameter = (radius * 2) + 1;

    for (x = slice_row_start; x < slice_row_end; x++) {
        uint16_t *first = (uint16_t *) desc->src->planes[0] + (x * step);
        uint16_t *last = first + (linesize * (desc->src->height - 1));
        uint16_t *s1 = first;
        uint16_t *s2 = first;
        uint16_t *d = (uint16_t *) desc->dst->planes[0] + (x * step);
        accumulator[0] = first[0] * (radius + 1);
        accumulator[1] = first[1] * (radius + 1);
        accumulator[2] = first[2] * (radius + 1);
        accumulator[3] = first[3] * (radius + 1);

        for (y = 0; y < radius; y++) {
            accumulator[0] += s1[0];
            accumulator[1] += s1[1];
            accumulator[2] += s1[2];
            accumulator[3] += s1[3];
            s1 += linesize;
        }
        for (y = 0; y <= radius; y++) {
            accumulator[0] += s1[0] - first[0];
            accumulator[1] += s1[1] - first[1];
            accumulator[2] += s1[2] - first[2];
            accumulator[3] += s1[3] - first[3];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            d[3] = lrint((double) accumulator[3] / diameter);
            s1 += linesize;
            d += linesize;
        }
        for (y = radius + 1; y < desc->src->height - radius; y++) {
            accumulator[0] += s1[0] - s2[0];
            accumulator[1] += s1[1] - s2[1];
            accumulator[2] += s1[2] - s2[2];
            accumulator[3] += s1[3] - s2[3];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            d[3] = lrint((double) accumulator[3] / diameter);
            s1 += linesize;
            s2 += linesize;
            d += linesize;
        }
        for (y = desc->src->height - radius; y < desc->src->height; y++) {
            accumulator[0] += last[0] - s2[0];
            accumulator[1] += last[1] - s2[1];
            accumulator[2] += last[2] - s2[2];
            accumulator[3] += last[3] - s2[3];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            d[3] = lrint((double) accumulator[3] / diameter);
            s2 += linesize;
            d += linesize;
        }
    }
    return 0;
}

static int blur_h_proc_rgbx64(int id, int index, int jobs, void *data)
{
    (void) id; // unused
    blur_slice_desc *desc = ((blur_slice_desc *) data);
    int slice_line_start,
        slice_height = mlt_slices_size_slice(jobs, index, desc->src->height, &slice_line_start);
    int slice_line_end = slice_line_start + slice_height;
    int accumulator[] = {0, 0, 0};
    int x = 0;
    int y = 0;
    int step = 4;
    int linesize = step * desc->src->width;
    int radius = desc->radius;

    if (desc->radius > (desc->src->width / 2)) {
        radius = desc->src->width / 2;
    }
    double diameter = (radius * 2) + 1;

    for (y = slice_line_start; y < slice_line_end; y++) {
        uint16_t *first = (uint16_t *) desc->src->planes[0] + (y * linesize);
        uint16_t *last = first + linesize - step;
        uint16_t *s1 = first;
        uint16_t *s2 = first;
        uint16_t *d = (uint16_t *) desc->dst->planes[0] + (y * linesize);
        accumulator[0] = first[0] * (radius + 1);
        accumulator[1] = first[1] * (radius + 1);
        accumulator[2] = first[2] * (radius + 1);

        for (x = 0; x < radius; x++) {
            accumulator[0] += s1[0];
            accumulator[1] += s1[1];
            accumulator[2] += s1[2];
            s1 += step;
        }
        for (x = 0; x <= radius; x++) {
            accumulator[0] += s1[0] - first[0];
            accumulator[1] += s1[1] - first[1];
            accumulator[2] += s1[2] - first[2];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            s1 += step;
            d += step;
        }
        for (x = radius + 1; x < desc->src->width - radius; x++) {
            accumulator[0] += s1[0] - s2[0];
            accumulator[1] += s1[1] - s2[1];
            accumulator[2] += s1[2] - s2[2];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            s1 += step;
            s2 += step;
            d += step;
        }
        for (x = desc->src->width - radius; x < desc->src->width; x++) {
            accumulator[0] += last[0] - s2[0];
            accumulator[1] += last[1] - s2[1];
            accumulator[2] += last[2] - s2[2];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            s2 += step;
            d += step;
        }
    }
    return 0;
}

static int blur_v_proc_rgbx64(int id, int index, int jobs, void *data)
{
    (void) id; // unused
    blur_slice_desc *desc = ((blur_slice_desc *) data);
    int slice_row_start,
        slice_width = mlt_slices_size_slice(jobs, index, desc->src->width, &slice_row_start);
    int slice_row_end = slice_row_start + slice_width;
    int accumulator[] = {0, 0, 0};
    int x = 0;
    int y = 0;
    int step = 4;
    int linesize = step * desc->src->width * 2;
    int radius = desc->radius;

    if (desc->radius > (desc->src->height / 2)) {
        radius = desc->src->height / 2;
    }
    double diameter = (radius * 2) + 1;

    for (x = slice_row_start; x < slice_row_end; x++) {
        uint16_t *first = (uint16_t *) desc->src->planes[0] + (x * step);
        uint16_t *last = first + (linesize * (desc->src->height - 1));
        uint16_t *s1 = first;
        uint16_t *s2 = first;
        uint16_t *d = (uint16_t *) desc->dst->planes[0] + (x * step);
        accumulator[0] = first[0] * (radius + 1);
        accumulator[1] = first[1] * (radius + 1);
        accumulator[2] = first[2] * (radius + 1);

        for (y = 0; y < radius; y++) {
            accumulator[0] += s1[0];
            accumulator[1] += s1[1];
            accumulator[2] += s1[2];
            s1 += linesize;
        }
        for (y = 0; y <= radius; y++) {
            accumulator[0] += s1[0] - first[0];
            accumulator[1] += s1[1] - first[1];
            accumulator[2] += s1[2] - first[2];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            s1 += linesize;
            d += linesize;
        }
        for (y = radius + 1; y < desc->src->height - radius; y++) {
            accumulator[0] += s1[0] - s2[0];
            accumulator[1] += s1[1] - s2[1];
            accumulator[2] += s1[2] - s2[2];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            s1 += linesize;
            s2 += linesize;
            d += linesize;
        }
        for (y = desc->src->height - radius; y < desc->src->height; y++) {
            accumulator[0] += last[0] - s2[0];
            accumulator[1] += last[1] - s2[1];
            accumulator[2] += last[2] - s2[2];
            d[0] = lrint((double) accumulator[0] / diameter);
            d[1] = lrint((double) accumulator[1] / diameter);
            d[2] = lrint((double) accumulator[2] / diameter);
            s2 += linesize;
            d += linesize;
        }
    }
    return 0;
}

/** Perform a box blur
 *
 * This function uses a sliding window accumulator method - applied
 * horizontally first and then vertically.
 *
 * \param self the Image object
 * \param hradius the radius of the horizontal blur in pixels
 * \param vradius radius of the vertical blur in pixels
 * \param preserve_alpha exclude the alpha channel from the blur operation
 */

void mlt_image_box_blur(mlt_image self, int hradius, int vradius, int preserve_alpha)
{
    if (self->format != mlt_image_rgba && self->format != mlt_image_rgba64) {
        mlt_log(NULL,
                MLT_LOG_ERROR,
                "Image type %s not supported by box blur\n",
                mlt_image_format_name(self->format));
        return;
    }
    // The horizontal blur is performed into a temporary image.
    // The vertical blur is performed back into the original image.
    struct mlt_image_s tmpimage;
    mlt_image_set_values(&tmpimage, NULL, self->format, self->width, self->height);
    mlt_image_alloc_data(&tmpimage);
    if (self->alpha) {
        mlt_image_alloc_alpha(&tmpimage);
    }

    mlt_slices_proc h_proc;
    mlt_slices_proc v_proc;
    if (preserve_alpha && self->format == mlt_image_rgba) {
        h_proc = blur_h_proc_rgbx;
        v_proc = blur_v_proc_rgbx;
    } else if (self->format == mlt_image_rgba) {
        h_proc = blur_h_proc_rgba;
        v_proc = blur_v_proc_rgba;
    } else if (preserve_alpha && self->format == mlt_image_rgba64) {
        h_proc = blur_h_proc_rgbx64;
        v_proc = blur_v_proc_rgbx64;
    } else { // self->format == mlt_image_rgba64
        h_proc = blur_h_proc_rgba64;
        v_proc = blur_v_proc_rgba64;
    }

    blur_slice_desc desc;
    desc.src = self, desc.dst = &tmpimage, desc.radius = hradius,
    mlt_slices_run_normal(0, h_proc, &desc);
    desc.src = &tmpimage, desc.dst = self, desc.radius = vradius,
    mlt_slices_run_normal(0, v_proc, &desc);

    mlt_image_close(&tmpimage);
}

static inline void blur_rgba8(int width, int height, uint8_t *data, int amount, int steps)
{
    float inv_steps = 1.0f / (float) steps;
    float amount_factor = amount * 0.01f;
    float inv_h = (height > 1) ? 2.0f / (float) (height - 1) : 0.0f;
    float inv_w = (width > 1) ? 2.0f / (float) (width - 1) : 0.0f;

#pragma omp parallel for schedule(guided)
    for (int y = 0; y < height; ++y) {
        float y_norm = -1.0f + (float) y * inv_h;
        int dst_row_offset = y * width * 4;

        for (int x = 0; x < width; ++x) {
            float x_norm = -1.0f + (float) x * inv_w;
            int dst_idx = dst_row_offset + (x * 4);

            float sum_r = (float) data[dst_idx];
            float sum_g = (float) data[dst_idx + 1];
            float sum_b = (float) data[dst_idx + 2];
            float sum_a = (float) data[dst_idx + 3];

            for (int step = 1; step < steps; ++step) {
                float scale = 1.0f - amount_factor * ((float) step * inv_steps);

                float mapped_y_float = ((y_norm * scale) + 1.0f) * 0.5f * (float) (height - 1);
                float mapped_x_float = ((x_norm * scale) + 1.0f) * 0.5f * (float) (width - 1);

                int mapped_y = CLAMP((int) mapped_y_float, 0, height - 1);
                int mapped_x = CLAMP((int) mapped_x_float, 0, width - 1);

                int src_idx = (mapped_y * width + mapped_x) * 4;

                sum_r += (float) data[src_idx];
                sum_g += (float) data[src_idx + 1];
                sum_b += (float) data[src_idx + 2];
                sum_a += (float) data[src_idx + 3];
            }

            data[dst_idx] = (uint8_t) CLAMP(sum_r * inv_steps, 0.0f, 255.0f);
            data[dst_idx + 1] = (uint8_t) CLAMP(sum_g * inv_steps, 0.0f, 255.0f);
            data[dst_idx + 2] = (uint8_t) CLAMP(sum_b * inv_steps, 0.0f, 255.0f);
            data[dst_idx + 3] = (uint8_t) CLAMP(sum_a * inv_steps, 0.0f, 255.0f);
        }
    }
}

static inline void blur_rgba64(int width, int height, uint16_t *data, int amount, int steps)
{
    float inv_steps = 1.0f / (float) steps;
    float amount_factor = amount * 0.01f;
    float inv_h = (height > 1) ? 2.0f / (float) (height - 1) : 0.0f;
    float inv_w = (width > 1) ? 2.0f / (float) (width - 1) : 0.0f;

#pragma omp parallel for schedule(guided)
    for (int y = 0; y < height; ++y) {
        float y_norm = -1.0f + (float) y * inv_h;
        int dst_row_offset = y * width * 4;

        for (int x = 0; x < width; ++x) {
            float x_norm = -1.0f + (float) x * inv_w;
            int dst_idx = dst_row_offset + (x * 4);

            float sum_r = (float) data[dst_idx];
            float sum_g = (float) data[dst_idx + 1];
            float sum_b = (float) data[dst_idx + 2];
            float sum_a = (float) data[dst_idx + 3];

            for (int step = 1; step < steps; ++step) {
                float scale = 1.0f - amount_factor * ((float) step * inv_steps);

                float mapped_y_float = ((y_norm * scale) + 1.0f) * 0.5f * (float) (height - 1);
                float mapped_x_float = ((x_norm * scale) + 1.0f) * 0.5f * (float) (width - 1);

                int mapped_y = CLAMP((int) mapped_y_float, 0, height - 1);
                int mapped_x = CLAMP((int) mapped_x_float, 0, width - 1);

                int src_idx = (mapped_y * width + mapped_x) * 4;

                sum_r += (float) data[src_idx];
                sum_g += (float) data[src_idx + 1];
                sum_b += (float) data[src_idx + 2];
                sum_a += (float) data[src_idx + 3];
            }

            data[dst_idx] = (uint16_t) CLAMP(sum_r * inv_steps, 0.0f, 65535.0f);
            data[dst_idx + 1] = (uint16_t) CLAMP(sum_g * inv_steps, 0.0f, 65535.0f);
            data[dst_idx + 2] = (uint16_t) CLAMP(sum_b * inv_steps, 0.0f, 65535.0f);
            data[dst_idx + 3] = (uint16_t) CLAMP(sum_a * inv_steps, 0.0f, 65535.0f);
        }
    }
}

void mlt_image_radial_blur(mlt_image self, int amount, int steps, int preserve_alpha)
{
    if (self->format != mlt_image_rgba && self->format != mlt_image_rgba64) {
        mlt_log(NULL,
                MLT_LOG_ERROR,
                "Image type %s not supported by radial blur\n",
                mlt_image_format_name(self->format));
        return;
    }

    if (steps <= 1 || amount == 0)
        return;

    if (self->format == mlt_image_rgba) {
        blur_rgba8(self->width, self->height, (uint8_t *) self->planes[0], amount, steps);
    } else if (self->format == mlt_image_rgba64) {
        blur_rgba64(self->width, self->height, (uint16_t *) self->planes[0], amount, steps);
    }
}