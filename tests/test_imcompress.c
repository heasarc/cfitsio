/*
 * Tests for imcompress.c - image compression/decompression
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fitsio.h"
#include "test_macros.h"

#define test_path "test_imcompress.fits"
#define test_path2 "test_imcompress2.fits"
#define test_path3 "test_imcompress3.fits"

/*
 * Helper to create a simple test image
 */
static void
create_test_image(fitsfile **fptr, int bitpix, long nx, long ny, int *status)
{
	long naxes[2] = { nx, ny };

	fits_create_file(fptr, "!" test_path, status);
	fail_if(*status != 0);

	fits_create_img(*fptr, bitpix, 2, naxes, status);
	fail_if(*status != 0);
}

/*
 * Test setting and getting compression type
 */
static void
test_compression_type(void)
{
	fitsfile *fptr;
	int status = 0;
	int ctype;

	create_test_image(&fptr, SHORT_IMG, 64, 64, &status);

	/* Test RICE compression */
	fits_set_compression_type(fptr, RICE_1, &status);
	fail_if(status != 0);

	fits_get_compression_type(fptr, &ctype, &status);
	fail_if(status != 0);
	fail_if(ctype != RICE_1);

	/* Test GZIP compression */
	fits_set_compression_type(fptr, GZIP_1, &status);
	fail_if(status != 0);

	fits_get_compression_type(fptr, &ctype, &status);
	fail_if(status != 0);
	fail_if(ctype != GZIP_1);

	/* Test GZIP_2 compression */
	fits_set_compression_type(fptr, GZIP_2, &status);
	fail_if(status != 0);

	fits_get_compression_type(fptr, &ctype, &status);
	fail_if(status != 0);
	fail_if(ctype != GZIP_2);

	/* Test PLIO compression */
	fits_set_compression_type(fptr, PLIO_1, &status);
	fail_if(status != 0);

	fits_get_compression_type(fptr, &ctype, &status);
	fail_if(status != 0);
	fail_if(ctype != PLIO_1);

	/* Test HCOMPRESS */
	fits_set_compression_type(fptr, HCOMPRESS_1, &status);
	fail_if(status != 0);

	fits_get_compression_type(fptr, &ctype, &status);
	fail_if(status != 0);
	fail_if(ctype != HCOMPRESS_1);

	fits_close_file(fptr, &status);
	fail_if(status != 0);
}

/*
 * Test setting and getting tile dimensions
 */
static void
test_tile_dimensions(void)
{
	fitsfile *fptr;
	int status = 0;
	long dims_in[2] = { 32, 32 };
	long dims_out[2] = { 0, 0 };

	create_test_image(&fptr, SHORT_IMG, 64, 64, &status);

	fits_set_tile_dim(fptr, 2, dims_in, &status);
	fail_if(status != 0);

	fits_get_tile_dim(fptr, 2, dims_out, &status);
	fail_if(status != 0);
	fail_if(dims_out[0] != 32);
	fail_if(dims_out[1] != 32);

	fits_close_file(fptr, &status);
	fail_if(status != 0);
}

/*
 * Test setting and getting quantize level
 */
static void
test_quantize_level(void)
{
	fitsfile *fptr;
	int status = 0;
	float qlevel;

	create_test_image(&fptr, SHORT_IMG, 64, 64, &status);

	fits_set_quantize_level(fptr, 16.0f, &status);
	fail_if(status != 0);

	fits_get_quantize_level(fptr, &qlevel, &status);
	fail_if(status != 0);
	fail_if(fabs(qlevel - 16.0f) > 0.001f);

	fits_close_file(fptr, &status);
	fail_if(status != 0);
}

/*
 * Test setting and getting noise bits
 */
static void
test_noise_bits(void)
{
	fitsfile *fptr;
	int status = 0;
	int noisebits;

	create_test_image(&fptr, SHORT_IMG, 64, 64, &status);

	fits_set_noise_bits(fptr, 4, &status);
	fail_if(status != 0);

	fits_get_noise_bits(fptr, &noisebits, &status);
	fail_if(status != 0);
	fail_if(noisebits != 4);

	fits_close_file(fptr, &status);
	fail_if(status != 0);
}

/*
 * Test RICE compression roundtrip with short image
 */
static void
test_rice_compress_short(void)
{
	fitsfile *infptr, *outfptr;
	int status = 0;
	long naxes[2] = { 64, 64 };
	short *original, *decompressed;
	int i, is_compressed;

	original = malloc(64 * 64 * sizeof *original);
	decompressed = malloc(64 * 64 * sizeof *decompressed);
	fail_if(original == NULL || decompressed == NULL);

	/* Create gradient data */
	for (i = 0; i < 64 * 64; i += 1) {
		original[i] = (short)(i % 1000);
	}

	/* Create input image */
	fits_create_file(&infptr, "!" test_path, &status);
	fail_if(status != 0);
	fits_create_img(infptr, SHORT_IMG, 2, naxes, &status);
	fail_if(status != 0);
	fits_write_img(infptr, TSHORT, 1, 64 * 64, original, &status);
	fail_if(status != 0);
	fits_close_file(infptr, &status);
	fail_if(status != 0);

	/* Open for reading and compress to new file */
	fits_open_file(&infptr, test_path, READONLY, &status);
	fail_if(status != 0);

	fits_create_file(&outfptr, "!" test_path2, &status);
	fail_if(status != 0);

	fits_set_compression_type(outfptr, RICE_1, &status);
	fail_if(status != 0);

	fits_img_compress(infptr, outfptr, &status);
	fail_if(status != 0);

	fits_close_file(infptr, &status);
	fits_close_file(outfptr, &status);
	fail_if(status != 0);

	/* Verify compressed file - compressed image is in HDU 2 */
	fits_open_file(&outfptr, test_path2, READONLY, &status);
	fail_if(status != 0);

	fits_movabs_hdu(outfptr, 2, NULL, &status);
	fail_if(status != 0);

	is_compressed = fits_is_compressed_image(outfptr, &status);
	fail_if(status != 0);
	fail_if(is_compressed != 1);

	/* Read back decompressed data */
	fits_read_img(outfptr, TSHORT, 1, 64 * 64, NULL, decompressed, NULL,
		&status);
	fail_if(status != 0);

	/* Verify lossless roundtrip */
	for (i = 0; i < 64 * 64; i += 1) {
		fail_if(decompressed[i] != original[i]);
	}

	fits_close_file(outfptr, &status);
	fail_if(status != 0);

	free(original);
	free(decompressed);
}

/*
 * Test GZIP compression roundtrip
 */
static void
test_gzip_compress(void)
{
	fitsfile *infptr, *outfptr;
	int status = 0;
	long naxes[2] = { 32, 32 };
	int *original, *decompressed;
	int i;

	original = malloc(32 * 32 * sizeof *original);
	decompressed = malloc(32 * 32 * sizeof *decompressed);
	fail_if(original == NULL || decompressed == NULL);

	/* Create test data */
	for (i = 0; i < 32 * 32; i += 1) {
		original[i] = i * 100;
	}

	/* Create input image */
	fits_create_file(&infptr, "!" test_path, &status);
	fail_if(status != 0);
	fits_create_img(infptr, LONG_IMG, 2, naxes, &status);
	fail_if(status != 0);
	fits_write_img(infptr, TINT, 1, 32 * 32, original, &status);
	fail_if(status != 0);
	fits_close_file(infptr, &status);
	fail_if(status != 0);

	/* Compress with GZIP */
	fits_open_file(&infptr, test_path, READONLY, &status);
	fail_if(status != 0);

	fits_create_file(&outfptr, "!" test_path2, &status);
	fail_if(status != 0);

	fits_set_compression_type(outfptr, GZIP_1, &status);
	fail_if(status != 0);

	fits_img_compress(infptr, outfptr, &status);
	fail_if(status != 0);

	fits_close_file(infptr, &status);
	fits_close_file(outfptr, &status);
	fail_if(status != 0);

	/* Read back - compressed image is in HDU 2 */
	fits_open_file(&outfptr, test_path2, READONLY, &status);
	fail_if(status != 0);

	fits_movabs_hdu(outfptr, 2, NULL, &status);
	fail_if(status != 0);

	fits_read_img(outfptr, TINT, 1, 32 * 32, NULL, decompressed, NULL,
		&status);
	fail_if(status != 0);

	/* Verify lossless */
	for (i = 0; i < 32 * 32; i += 1) {
		fail_if(decompressed[i] != original[i]);
	}

	fits_close_file(outfptr, &status);
	fail_if(status != 0);

	free(original);
	free(decompressed);
}

/*
 * Test PLIO compression (good for mask images)
 */
static void
test_plio_compress(void)
{
	fitsfile *infptr, *outfptr;
	int status = 0;
	long naxes[2] = { 64, 64 };
	short *original, *decompressed;
	int i;

	original = malloc(64 * 64 * sizeof *original);
	decompressed = malloc(64 * 64 * sizeof *decompressed);
	fail_if(original == NULL || decompressed == NULL);

	/* Create mask-like data (mostly zeros with some ones) */
	for (i = 0; i < 64 * 64; i += 1) {
		original[i] = (i % 10 == 0) ? 1 : 0;
	}

	/* Create input image */
	fits_create_file(&infptr, "!" test_path, &status);
	fail_if(status != 0);
	fits_create_img(infptr, SHORT_IMG, 2, naxes, &status);
	fail_if(status != 0);
	fits_write_img(infptr, TSHORT, 1, 64 * 64, original, &status);
	fail_if(status != 0);
	fits_close_file(infptr, &status);
	fail_if(status != 0);

	/* Compress with PLIO */
	fits_open_file(&infptr, test_path, READONLY, &status);
	fail_if(status != 0);

	fits_create_file(&outfptr, "!" test_path2, &status);
	fail_if(status != 0);

	fits_set_compression_type(outfptr, PLIO_1, &status);
	fail_if(status != 0);

	fits_img_compress(infptr, outfptr, &status);
	fail_if(status != 0);

	fits_close_file(infptr, &status);
	fits_close_file(outfptr, &status);
	fail_if(status != 0);

	/* Read back - compressed image is in HDU 2 */
	fits_open_file(&outfptr, test_path2, READONLY, &status);
	fail_if(status != 0);

	fits_movabs_hdu(outfptr, 2, NULL, &status);
	fail_if(status != 0);

	fits_read_img(outfptr, TSHORT, 1, 64 * 64, NULL, decompressed, NULL,
		&status);
	fail_if(status != 0);

	/* Verify lossless */
	for (i = 0; i < 64 * 64; i += 1) {
		fail_if(decompressed[i] != original[i]);
	}

	fits_close_file(outfptr, &status);
	fail_if(status != 0);

	free(original);
	free(decompressed);
}

/*
 * Test HCOMPRESS compression
 */
static void
test_hcompress_compress(void)
{
	fitsfile *infptr, *outfptr;
	int status = 0;
	long naxes[2] = { 64, 64 };
	int *original, *decompressed;
	int i;

	original = malloc(64 * 64 * sizeof *original);
	decompressed = malloc(64 * 64 * sizeof *decompressed);
	fail_if(original == NULL || decompressed == NULL);

	/* Create gradient data */
	for (i = 0; i < 64 * 64; i += 1) {
		original[i] = (i % 64) + (i / 64) * 100;
	}

	/* Create input image */
	fits_create_file(&infptr, "!" test_path, &status);
	fail_if(status != 0);
	fits_create_img(infptr, LONG_IMG, 2, naxes, &status);
	fail_if(status != 0);
	fits_write_img(infptr, TINT, 1, 64 * 64, original, &status);
	fail_if(status != 0);
	fits_close_file(infptr, &status);
	fail_if(status != 0);

	/* Compress with HCOMPRESS */
	fits_open_file(&infptr, test_path, READONLY, &status);
	fail_if(status != 0);

	fits_create_file(&outfptr, "!" test_path2, &status);
	fail_if(status != 0);

	fits_set_compression_type(outfptr, HCOMPRESS_1, &status);
	fail_if(status != 0);

	fits_img_compress(infptr, outfptr, &status);
	fail_if(status != 0);

	fits_close_file(infptr, &status);
	fits_close_file(outfptr, &status);
	fail_if(status != 0);

	/* Read back - compressed image is in HDU 2 */
	fits_open_file(&outfptr, test_path2, READONLY, &status);
	fail_if(status != 0);

	fits_movabs_hdu(outfptr, 2, NULL, &status);
	fail_if(status != 0);

	fits_read_img(outfptr, TINT, 1, 64 * 64, NULL, decompressed, NULL,
		&status);
	fail_if(status != 0);

	/* Verify lossless (with default settings) */
	for (i = 0; i < 64 * 64; i += 1) {
		fail_if(decompressed[i] != original[i]);
	}

	fits_close_file(outfptr, &status);
	fail_if(status != 0);

	free(original);
	free(decompressed);
}

/*
 * Test fits_is_compressed_image on uncompressed image
 */
static void
test_is_compressed_uncompressed(void)
{
	fitsfile *fptr;
	int status = 0;
	int is_compressed;

	create_test_image(&fptr, SHORT_IMG, 32, 32, &status);

	is_compressed = fits_is_compressed_image(fptr, &status);
	fail_if(status != 0);
	fail_if(is_compressed != 0);

	fits_close_file(fptr, &status);
	fail_if(status != 0);
}

/*
 * Test byte image compression
 */
static void
test_compress_byte_image(void)
{
	fitsfile *infptr, *outfptr;
	int status = 0;
	long naxes[2] = { 32, 32 };
	unsigned char *original, *decompressed;
	int i;

	original = malloc(32 * 32 * sizeof *original);
	decompressed = malloc(32 * 32 * sizeof *decompressed);
	fail_if(original == NULL || decompressed == NULL);

	/* Create test data */
	for (i = 0; i < 32 * 32; i += 1) {
		original[i] = (unsigned char)(i % 256);
	}

	/* Create input image */
	fits_create_file(&infptr, "!" test_path, &status);
	fail_if(status != 0);
	fits_create_img(infptr, BYTE_IMG, 2, naxes, &status);
	fail_if(status != 0);
	fits_write_img(infptr, TBYTE, 1, 32 * 32, original, &status);
	fail_if(status != 0);
	fits_close_file(infptr, &status);
	fail_if(status != 0);

	/* Compress */
	fits_open_file(&infptr, test_path, READONLY, &status);
	fail_if(status != 0);

	fits_create_file(&outfptr, "!" test_path2, &status);
	fail_if(status != 0);

	fits_set_compression_type(outfptr, RICE_1, &status);
	fail_if(status != 0);

	fits_img_compress(infptr, outfptr, &status);
	fail_if(status != 0);

	fits_close_file(infptr, &status);
	fits_close_file(outfptr, &status);
	fail_if(status != 0);

	/* Read back - compressed image is in HDU 2 */
	fits_open_file(&outfptr, test_path2, READONLY, &status);
	fail_if(status != 0);

	fits_movabs_hdu(outfptr, 2, NULL, &status);
	fail_if(status != 0);

	fits_read_img(outfptr, TBYTE, 1, 32 * 32, NULL, decompressed, NULL,
		&status);
	fail_if(status != 0);

	/* Verify */
	for (i = 0; i < 32 * 32; i += 1) {
		fail_if(decompressed[i] != original[i]);
	}

	fits_close_file(outfptr, &status);
	fail_if(status != 0);

	free(original);
	free(decompressed);
}

/*
 * Test dither seed setting
 */
static void
test_dither_seed(void)
{
	fitsfile *fptr;
	int status = 0;
	int seed;

	create_test_image(&fptr, SHORT_IMG, 32, 32, &status);

	fits_set_dither_seed(fptr, 1234, &status);
	fail_if(status != 0);

	fits_get_dither_seed(fptr, &seed, &status);
	fail_if(status != 0);
	fail_if(seed != 1234);

	fits_close_file(fptr, &status);
	fail_if(status != 0);
}

/*
 * The variable-length string in a given row of the table built below
 */
static void
vla_string(char *buf, size_t size, long row)
{
	int k, n = (int)(row % 15);
	int p = snprintf(buf, size, "obj_");

	for (k = 0; k < n; k += 1) {
		buf[p] = 'x';
		p += 1;
	}
	buf[p] = '\0';
}

/*
 * Round trip a table containing variable-length array columns through
 * fits_compress_table / fits_uncompress_table (what "fpack -table" does).
 *
 * A variable-length *string* column ('1PA') used to be sized as if its field
 * held one byte per row rather than an 8 byte 'P' descriptor, so
 * fits_uncompress_table reserved too little room for the column and gunzipped
 * past the end of its buffer, corrupting the heap - see heasarc/cfitsio
 * issue #134.
 */
static void
test_compress_table_vla(void)
{
	fitsfile *in, *out, *back;
	int status = 0, anynull, ztable = 0;
	long nrows = 600, nback = 0, i;
	char *ttype[] = { "vstr", "vflt", "vint", "fixstr" };
	char *tform[] = { "1PA", "1PE", "1PJ", "8A" };
	char *tunit[] = { "", "", "", "" };

	/* Build the uncompressed table.  It has to be larger than 5760 bytes,
	   otherwise fits_compress_table just copies it verbatim. */
	fits_create_file(&in, "!" test_path, &status);
	fail_if(status != 0);
	fits_create_tbl(in, BINARY_TBL, 0, 4, ttype, tform, tunit, "DATA",
		&status);
	fail_if(status != 0);

	for (i = 0; i < nrows; i += 1) {
		char str[64], fixed[16], *cell[1];
		float f[16];
		int jv[16];
		int k, n = 1 + (int)(i % 12);

		vla_string(str, sizeof str, i);
		cell[0] = str;
		fits_write_col(in, TSTRING, 1, i + 1, 1, 1, cell, &status);

		for (k = 0; k < n; k += 1) {
			f[k] = (float)(i + k) * 1.5f;
			jv[k] = (int)(i * 100 + k);
		}
		fits_write_col(in, TFLOAT, 2, i + 1, 1, n, f, &status);
		fits_write_col(in, TINT, 3, i + 1, 1, n, jv, &status);

		snprintf(fixed, sizeof fixed, "row%04ld", i);
		cell[0] = fixed;
		fits_write_col(in, TSTRING, 4, i + 1, 1, 1, cell, &status);
	}
	fail_if(status != 0);
	fits_close_file(in, &status);
	fail_if(status != 0);

	/* Compress it */
	fits_open_file(&in, test_path, READONLY, &status);
	fail_if(status != 0);
	fits_movabs_hdu(in, 2, NULL, &status);
	fail_if(status != 0);
	fits_create_file(&out, "!" test_path2, &status);
	fail_if(status != 0);
	fits_compress_table(in, out, &status);
	fail_if(status != 0);
	fits_close_file(in, &status);
	fits_close_file(out, &status);
	fail_if(status != 0);

	/* The table must really have been compressed, or this test would
	   silently exercise nothing */
	fits_open_file(&out, test_path2, READONLY, &status);
	fail_if(status != 0);
	fits_movabs_hdu(out, 2, NULL, &status);
	fail_if(status != 0);
	fits_read_key(out, TLOGICAL, "ZTABLE", &ztable, NULL, &status);
	fail_if(status != 0);
	fail_if(!ztable);

	/* Uncompress it again */
	fits_create_file(&back, "!" test_path3, &status);
	fail_if(status != 0);
	fits_uncompress_table(out, back, &status);
	fail_if(status != 0);
	fits_close_file(out, &status);
	fits_close_file(back, &status);
	fail_if(status != 0);

	/* Every value must have survived the round trip */
	fits_open_file(&back, test_path3, READONLY, &status);
	fail_if(status != 0);
	fits_movabs_hdu(back, 2, NULL, &status);
	fail_if(status != 0);
	fits_get_num_rows(back, &nback, &status);
	fail_if(status != 0);
	fail_if(nback != nrows);

	for (i = 0; i < nrows; i += 1) {
		char expect[64], got[64], *cell[1];
		float f[16];
		int jv[16];
		long repeat, offset;
		int k, n = 1 + (int)(i % 12);

		cell[0] = got;

		vla_string(expect, sizeof expect, i);
		fits_read_col(back, TSTRING, 1, i + 1, 1, 1, "", cell,
			&anynull, &status);
		fail_if(strcmp(got, expect) != 0);

		fits_read_descript(back, 2, i + 1, &repeat, &offset, &status);
		fail_if(repeat != n);
		fits_read_col(back, TFLOAT, 2, i + 1, 1, n, NULL, f, &anynull,
			&status);
		fits_read_col(back, TINT, 3, i + 1, 1, n, NULL, jv, &anynull,
			&status);
		for (k = 0; k < n; k += 1) {
			fail_if(f[k] != (float)(i + k) * 1.5f);
			fail_if(jv[k] != (int)(i * 100 + k));
		}

		snprintf(expect, sizeof expect, "row%04ld", i);
		fits_read_col(back, TSTRING, 4, i + 1, 1, 1, "", cell,
			&anynull, &status);
		fail_if(strcmp(got, expect) != 0);
	}
	fail_if(status != 0);

	fits_close_file(back, &status);
	fail_if(status != 0);
}

int
main(void)
{
	test_compression_type();
	test_tile_dimensions();
	test_quantize_level();
	test_noise_bits();
	test_rice_compress_short();
	test_gzip_compress();
	test_plio_compress();
	test_hcompress_compress();
	test_is_compressed_uncompressed();
	test_compress_byte_image();
	test_compress_table_vla();
	test_dither_seed();

	remove(test_path);
	remove(test_path2);
	remove(test_path3);

	return 0;
}
