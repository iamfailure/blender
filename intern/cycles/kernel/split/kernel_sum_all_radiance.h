/*
 * Copyright 2011-2015 Blender Foundation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "kernel_split_common.h"

/* Since we process various samples in parallel; The output radiance of different samples
 * are stored in different locations; This kernel combines the output radiance contributed
 * by all different samples and stores them in the RenderTile's output buffer.
 */

ccl_device void kernel_sum_all_radiance(
        KernelGlobals *kg,
        ccl_global float *buffer,                    /* Output buffer of RenderTile */
        int parallel_samples, int sw, int sh, int stride,
        int buffer_offset_x,
        int buffer_offset_y,
        int buffer_stride,
        int start_sample)
{
	int x = get_global_id(0);
	int y = get_global_id(1);

	if(x < sw && y < sh) {
		buffer += ((buffer_offset_x + x) + (buffer_offset_y + y) * buffer_stride) * (kernel_data.film.pass_stride);

		ccl_global float *per_sample_output_buffer = split_state->per_sample_output_buffers;
		per_sample_output_buffer += ((x + y * stride) * parallel_samples) * (kernel_data.film.pass_stride);

		int sample_stride = (kernel_data.film.pass_stride);

		int sample_iterator = 0;
		int pass_stride_iterator = 0;
		int num_floats = kernel_data.film.pass_stride;

		for(sample_iterator = 0; sample_iterator < parallel_samples; sample_iterator++) {
			for(pass_stride_iterator = 0; pass_stride_iterator < num_floats; pass_stride_iterator++) {
				*(buffer + pass_stride_iterator) =
				        (start_sample == 0 && sample_iterator == 0)
				                ? *(per_sample_output_buffer + pass_stride_iterator)
				                : *(buffer + pass_stride_iterator) + *(per_sample_output_buffer + pass_stride_iterator);
			}
			per_sample_output_buffer += sample_stride;
		}
	}
}
