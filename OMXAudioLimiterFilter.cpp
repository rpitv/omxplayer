/*
 *	Copyright (C) 2012 Andrew H. Armenia
 *      Copyright (C) 2005-2008 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include "OMXAudioLimiterFilter.h"
#include <stdio.h>

OMXAudioLimiterFilter::OMXAudioLimiterFilter() {
	peak = 0;
	target_peak = 3000;	/* arbitrary value for now */
}

OMXAudioLimiterFilter::OMXAudioLimiterFilter(int target) {
	peak = 0;
	target_peak = target;
}

OMXAudioLimiterFilter::~OMXAudioLimiterFilter() {

}

void OMXAudioLimiterFilter::FilterSamples(uint8_t *sample_ptr, int size) {
	int16_t *samples = (int16_t *)sample_ptr;
	int nsamples = size / sizeof(int16_t);
	int i;
	
	/* 
         * peak detector section: we react fast to new higher peaks 
         * but more slowly to decreasing volume levels
	 */
	
	/* the 0.9 here determines the decay rate, smaller numbers mean faster decay */
	peak = (float)peak * 0.9;
		
	/* attack is essentially instant in this implementation */
	for (i = 0; i < nsamples; i++) {
		/* detect both positive and negative peaks */
		if (samples[i] > peak) {
			peak = samples[i];
		}
		if (-samples[i] > peak) {
			peak = -samples[i];
		}
	}

	/*
	 * gain section: we use the ratio peak/target_peak to set the gain, reducing it if
	 * peak is greater than target_peak and increasing it if peak is less than target_peak.
	 */
	float gain = (float)target_peak / (float)peak;
	/* we will use fixed point arithmetic here for speed */
	uint32_t multiplier = gain * 256;
	uint32_t temp;

	fprintf(stderr, "compressor gain is %f\n", gain);

	for (i = 0; i < nsamples; i++) {
		temp = samples[i];
		temp *= multiplier;
		samples[i] = temp / 256;
	}
}
