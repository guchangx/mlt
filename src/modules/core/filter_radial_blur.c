/*
 * filter_box_blur.c
 * Copyright (C) 2011-2025 Meltytech, LLC
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

#include <framework/mlt_filter.h>
#include <framework/mlt_frame.h>
#include <framework/mlt_image.h>
#include <framework/mlt_log.h>
#include <framework/mlt_profile.h>

#include <math.h>

static int filter_get_image(mlt_frame frame,
                            uint8_t **image,
                            mlt_image_format *format,
                            int *width,
                            int *height,
                            int writable)
{
    int error = 0;
    mlt_filter filter = (mlt_filter) mlt_frame_pop_service(frame);
    mlt_properties properties = MLT_FILTER_PROPERTIES(filter);
    mlt_position position = mlt_filter_get_position(filter, frame);
    mlt_position length = mlt_filter_get_length2(filter, frame);

    int amount = (int) mlt_properties_anim_get_double(properties, "amount", position, length);
    int steps = (int) mlt_properties_anim_get_double(properties, "steps", position, length);
    int preserve_alpha = mlt_properties_get_int(properties, "preserve_alpha");

    if (amount == 0 || steps <= 1) {
        // Nothing to blur
        error = mlt_frame_get_image(frame, image, format, width, height, writable);
    } else {
        // Get the image
        if (*format != mlt_image_rgba64) {
            *format = mlt_image_rgba;
        }
        error = mlt_frame_get_image(frame, image, format, width, height, 1);
        if (error == 0) {
            struct mlt_image_s img;
            mlt_image_set_values(&img, *image, *format, *width, *height);
            mlt_image_radial_blur(&img, amount, steps, preserve_alpha);
        }
    }
    return error;
}

/** Filter processing.
*/

static mlt_frame filter_process(mlt_filter filter, mlt_frame frame)
{
    mlt_frame_push_service(frame, filter);
    mlt_frame_push_get_image(frame, filter_get_image);

    return frame;
}

/** Constructor for the filter.
*/

mlt_filter filter_radial_blur_init(mlt_profile profile,
                                   mlt_service_type type,
                                   const char *id,
                                   char *arg)
{
    (void) profile;
    (void) type;
    (void) id;
    (void) arg;
    mlt_filter filter = mlt_filter_new();
    if (filter != NULL) {
        filter->process = filter_process;
        mlt_properties_set_int(MLT_FILTER_PROPERTIES(filter), "amount", 10);
        mlt_properties_set_int(MLT_FILTER_PROPERTIES(filter), "steps", 5);
    }
    return filter;
}
