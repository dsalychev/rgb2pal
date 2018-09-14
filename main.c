#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>
#include <math.h>
#include <float.h>

#define MAX_MATRIX_SIZE		100
#ifndef M_PI
#define M_PI			3.14159265358979323
#endif

struct PaletteColor {
	uint32_t i;
	uint32_t rgb;
};

static struct PaletteColor palette[65536];	/* Palette from a file */
static uint32_t pal_size;			/* Actual palette size */
static double m_a[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE];
static double m_b[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE];

/* Load palette from a file. */
static uint32_t load_palette(FILE *pal);

/* Print a nearest color to the given one and its index from a palette. */
static void print_nearest(uint32_t rgb);

/* Multiply matrices Md and Ms with variable sizes:
 *
 * 	(Mdr x Mdc) x (Msr x Msc)
 *
 * Destination matrix Md should have enough space to store resulted matrix
 * which size is Mdr x Msc. */
static void multiply_varm(uint64_t md_r, uint64_t md_c,
                          uint64_t ms_r, uint64_t ms_c,
                          void *_md, void *_ms);

/* rgb2pal accepts these parameters:
 * 1. Color as 24-bit RGB hex value (for example, aa87b4)
 * 2. Path to the palette file (see terminal256.palette)
 * Index of the nearest color from the palette will be printed on stdout, and
 * 0 will be returned. Non-zero value will be returned in case of error.
 *
 * Palette file is a simple ASCII text file in form:
 *	# Comment line
 *	# Another comment line
 *	# Each lines is a <color_index>=<RGB_hex_24_bit>
 *	1=5566ab
 *	2=5577bc
 *
 * Usage:
 *	$ rgb2pal bc5475 terminal256.palette
 * 	47
 */
int main(int argc, char *argv[])
{
	FILE *pal;
	uint32_t rgb_color;
	int ret, r;

	/* Initialize variables */
	ret = 0;
	pal = NULL;

	/* At least two arguments are expected */
	if (argc >= 3) {
		/* Read an input RGB color */
		r = sscanf(argv[1], "%" SCNx32, &rgb_color);
		if (r != 1) {
			ret = 1;
		}
		/* Open a palette to find a nearest color */
		pal = fopen(argv[2], "r");
		if (pal == NULL) {
			ret = 1;
		}

		if (ret == 0) {
			pal_size = load_palette(pal);
			fclose(pal);
			if (pal_size > 0) {
				print_nearest(rgb_color);
			} else {
				ret = 1;
			}
		}
	} else {
		ret = 1;
	}

	return ret;
}

static uint32_t load_palette(FILE *pal)
{
	char buff[128];
	char *str;
	uint32_t i;
	int r;

	i = 0;
	str = fgets(buff, sizeof buff, pal);

	while ((i < (sizeof palette)) && (str != NULL)) {
		/* We may have a comment line in buffer */
		if (buff[0] == '#') {
			str = fgets(buff, sizeof buff, pal);
			continue;
		}

		/* Trying to read a palette record */
		r = sscanf(buff, "%" SCNu32 "=%" SCNx32,
		           &palette[i].i, &palette[i].rgb);
		if (r != 2) {
			/* Stop immediately in case of an incorrect format. */
			i = 0;
			break;
		}
		i++;
		str = fgets(buff, sizeof buff, pal);
	}

	pal_size = i;
	return i;
}

/* RGB will be transformed into a YUV colorspace in order to find similar
 * colors according to the "average human perception".
 *
 * See for details: https://stackoverflow.com/a/5392276 */
static void print_nearest(uint32_t rgb)
{
	uint32_t i, j, k, l;
	double dr, dg, db;
	double dist, min;
	const double yuv[3][3] = {
		{  0.299,    0.587,    0.114 },
		{ -0.14713, -0.28886,  0.436 },
		{  0.615,   -0.51499, -0.10001 }
	};
	double Yuv[3][3];
	double crgb[3][1] = {	/* Given color */
		{ 0.0 },	/* R */
		{ 0.0 },	/* G */
		{ 0.0 }		/* B */
	};
	double prgb[3][1] = {	/* Palette color */
		{ 0.0 },	/* R */
		{ 0.0 },	/* G */
		{ 0.0 }		/* B */
	};

	min = DBL_MAX;
	j = 0;
	for (i = 0; i < pal_size; i++) {
		crgb[0][0] = (rgb>>16)&0xFFU;
		crgb[1][0] = (rgb>>8)&0xFFU;
		crgb[2][0] = (rgb)&0xFFU;
		prgb[0][0] = (palette[i].rgb>>16)&0xFFU;
		prgb[1][0] = (palette[i].rgb>>8)&0xFFU;
		prgb[2][0] = (palette[i].rgb)&0xFFU;

		for (k = 0; k < 3; k++) {
			for (l = 0; l < 3; l++) {
				Yuv[k][l] = yuv[k][l];
			}
		}
		multiply_varm(3, 3, 3, 1, Yuv, crgb);
		crgb[0][0] = Yuv[0][0];
		crgb[1][0] = Yuv[1][0];
		crgb[2][0] = Yuv[2][0];

		for (k = 0; k < 3; k++) {
			for (l = 0; l < 3; l++) {
				Yuv[k][l] = yuv[k][l];
			}
		}
		multiply_varm(3, 3, 3, 1, Yuv, prgb);
		prgb[0][0] = Yuv[0][0];
		prgb[1][0] = Yuv[1][0];
		prgb[2][0] = Yuv[2][0];

		dr = (crgb[0][0] - prgb[0][0]);
		dg = (crgb[1][0] - prgb[1][0]);
		db = (crgb[2][0] - prgb[2][0]);
		dist = sqrt(dr*dr + dg*dg + db*db);
		if (dist < min) {
			min = dist;
			j = i;
		}
	}
	printf("%" PRIu32 " #%" PRIx32 "\n", palette[j].i, palette[j].rgb);
}

static void multiply_varm(uint64_t md_r, uint64_t md_c,
                          uint64_t ms_r, uint64_t ms_c,
                          void *_md, void *_ms)
{
	uint64_t i, j, k;
	double *md = (double *) _md;
	double *ms = (double *) _ms;

	if (md_r > MAX_MATRIX_SIZE ||
	    md_c > MAX_MATRIX_SIZE ||
	    ms_r > MAX_MATRIX_SIZE ||
	    ms_c > MAX_MATRIX_SIZE) {
		fprintf(stderr, "Size of a matrix is too big,"
				" max size is: %d\n", MAX_MATRIX_SIZE);
		for (i = 0; i < md_r; i++) {
			for (j = 0; j < md_c; j++) {
				md[md_c*i+j] = 0.0;
			}
		}
		return;
	}
	if (md_c != ms_r) {
		fprintf(stderr, "Number of destination matrix columns should "
				"be the same as a number of source matrix "
				"rows: columns=%" PRIu64
				", rows=%" PRIu64 "\n", md_c, ms_r);
		for (i = 0; i < md_r; i++) {
			for (j = 0; j < md_c; j++) {
				md[md_c*i+j] = 0.0;
			}
		}
		return;
	}

	for (i = 0; i < md_r; i++) {
		for (j = 0; j < md_c; j++) {
			m_a[i][j] = md[md_c*i+j];
			md[md_c*i+j] = 0.0;
		}
	}
	for (i = 0; i < ms_r; i++) {
		for (j = 0; j < ms_c; j++) {
			m_b[i][j] = ms[ms_c*i+j];
		}
	}

	for (i = 0; i < md_r; i++) {
		for (j = 0; j < ms_c; j++) {
			/* Mdc == Msr at this point */
			for (k = 0; k < md_c; k++) {
				md[md_c*i+j] += m_a[i][k] * m_b[k][j];
			}
		}
	}
}
