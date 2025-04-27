// light_pcapng.c
// Created on: Jul 23, 2016

// Copyright (c) 2016 Radu Velea

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "light_pcapng.h"

#include "light_debug.h"
#include "light_internal.h"
#include "light_util.h"
#include "light_platform.h"

#include <stdlib.h>
#include <string.h>

// Documentation from: https://github.com/pcapng/pcapng

static struct _light_option *__parse_options(uint32_t **memory, const int32_t max_len)
{
	if (max_len <= 0) {
		return NULL;
	}
	else {
		struct _light_option *opt = calloc(1, sizeof(struct _light_option));
		uint16_t actual_length;
		uint16_t alignment = sizeof(uint32_t);

		uint16_t *local_memory = (uint16_t*)*memory;
		uint16_t remaining_size;

		opt->custom_option_code = *local_memory++;
		opt->option_length = *local_memory++;

		// Validate option_length
		if (opt->option_length > max_len - 2 * sizeof(*local_memory)) {
			free(opt);
			return NULL;
		}

		actual_length = (opt->option_length % alignment) == 0 ?
			opt->option_length :
			(opt->option_length / alignment + 1) * alignment;

		// Validate actual_length
		if (actual_length <= 0 || actual_length > max_len - 2 * sizeof(*local_memory)) {
			free(opt);
			return NULL;
		}
		opt->data = calloc(1, actual_length);
		memcpy(opt->data, local_memory, actual_length);
		local_memory += (sizeof(**memory) / sizeof(*local_memory)) * (actual_length / alignment);

		*memory = (uint32_t*)local_memory;
		remaining_size = max_len - actual_length - 2 * sizeof(*local_memory);

		if (opt->custom_option_code == 0) {
			DCHECK_ASSERT(opt->option_length, 0, light_stop);
			DCHECK_ASSERT(remaining_size, 0, light_stop);

			if (remaining_size) {
				// XXX: Treat the remaining data as garbage and discard it form the trace.
				*memory += remaining_size / sizeof(uint32_t);
			}
		}
		else {
			opt->next_option = __parse_options(memory, remaining_size);
		}

		return opt;
	}
}
