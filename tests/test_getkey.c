/*
 * Tests for getkey.c - keyword reading functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fitsio.h"
#include "test_macros.h"

#define test_path "test_getkey.fits"

static void
test_ffgknjj_basic(void)
{
	/* Test ffgknjj - get indexed keywords as LONGLONG values. */
	fitsfile *f;
	int status = 0;
	long naxes[] = { 100, 200, 300 };
	LONGLONG values[3];
	int nfound;

	/* Create image with NAXIS1, NAXIS2, NAXIS3 keywords. */
	call_02(ffinit, &f, "!" test_path);
	call_04(ffphps, f, BYTE_IMG, 3, naxes);
	call_01(ffclos, f);

	call_03(ffopen, &f, test_path, READONLY);
	/* Read NAXIS1, NAXIS2, NAXIS3 as LONGLONG. */
	call_06(ffgknjj, f, "NAXIS", 1, 3, values, &nfound);
	fail_if(nfound != 3);
	fail_if(values[0] != 100);
	fail_if(values[1] != 200);
	fail_if(values[2] != 300);
	call_01(ffclos, f);
}

static void
test_ffgknjj_large_values(void)
{
	/* Test ffgknjj with values stored as LONGLONG (but small enough for file). */
	fitsfile *f;
	int status = 0;
	LONGLONG naxes[] = { 100LL, 200LL };
	LONGLONG values[2];
	int nfound;

	call_02(ffinit, &f, "!" test_path);
	call_04(ffphpsll, f, LONGLONG_IMG, 2, naxes);
	call_01(ffclos, f);

	call_03(ffopen, &f, test_path, READONLY);
	call_06(ffgknjj, f, "NAXIS", 1, 2, values, &nfound);
	fail_if(nfound != 2);
	fail_if(values[0] != 100LL);
	fail_if(values[1] != 200LL);
	call_01(ffclos, f);
}

static void
test_ffgtdmll_basic(void)
{
	/* Test ffgtdmll - decode TDIM keyword as LONGLONG. */
	fitsfile *f;
	int status = 0;
	char *ttype[] = { "DATA" };
	char *tform[] = { "12I" };
	int naxis;
	LONGLONG naxes[2];

	/* Create table with TDIM keyword. */
	call_02(ffinit, &f, "!" test_path);
	call_04(ffphps, f, BYTE_IMG, 0, NULL);
	call_08(ffcrtb, f, BINARY_TBL, 5, 1, ttype, tform, NULL, NULL);
	/* Set TDIM1 to (3,4) - 12 element array as 3x4 2D array. */
	call_04(ffpkys, f, "TDIM1", "(3,4)", "Column dimensions");
	call_01(ffclos, f);

	call_03(ffopen, &f, test_path, READONLY);
	call_03(ffmahd, f, 2, NULL);
	call_05(ffgtdmll, f, 1, 2, &naxis, naxes);
	fail_if(naxis != 2);
	fail_if(naxes[0] != 3);
	fail_if(naxes[1] != 4);
	call_01(ffclos, f);
}

static void
test_ffdtdmll_basic(void)
{
	/* Test ffdtdmll - decode TDIM string as LONGLONG. */
	fitsfile *f;
	int status = 0;
	char *ttype[] = { "DATA" };
	char *tform[] = { "60I" };
	int naxis;
	LONGLONG naxes[3];

	/* Create table to get valid fitsfile pointer. */
	call_02(ffinit, &f, "!" test_path);
	call_04(ffphps, f, BYTE_IMG, 0, NULL);
	call_08(ffcrtb, f, BINARY_TBL, 5, 1, ttype, tform, NULL, NULL);
	call_01(ffclos, f);

	/* Reopen and move to table HDU. */
	call_03(ffopen, &f, test_path, READONLY);
	call_03(ffmahd, f, 2, NULL);

	/* Decode TDIM string directly. */
	call_06(ffdtdmll, f, "(3,4,5)", 1, 3, &naxis, naxes);
	fail_if(naxis != 3);
	fail_if(naxes[0] != 3);
	fail_if(naxes[1] != 4);
	fail_if(naxes[2] != 5);
	call_01(ffclos, f);
}

static void
test_ffghbnll_basic(void)
{
	/* Test ffghbnll - get binary table header info as LONGLONG. */
	fitsfile *f;
	int status = 0;
	char *ttype[] = { "COL1", "COL2" };
	char *tform[] = { "1J", "1E" };
	LONGLONG nrows;
	int tfields;
	LONGLONG pcount;
	char extname[FLEN_VALUE];

	call_02(ffinit, &f, "!" test_path);
	call_04(ffphps, f, BYTE_IMG, 0, NULL);
	call_08(ffcrtb, f, BINARY_TBL, 100, 2, ttype, tform, NULL, "DATA");
	call_01(ffclos, f);

	call_03(ffopen, &f, test_path, READONLY);
	call_03(ffmahd, f, 2, NULL);
	call_09(ffghbnll, f, 99, &nrows, &tfields, NULL, NULL, NULL, extname, &pcount);
	fail_if(nrows != 100);
	fail_if(tfields != 2);
	fail_if(strcmp(extname, "DATA") != 0);
	call_01(ffclos, f);
}

static void
test_ffghtbll_basic(void)
{
	/* Test ffghtbll - get ASCII table header info as LONGLONG. */
	fitsfile *f;
	int status = 0;
	char *ttype[] = { "VALUE" };
	char *tform[] = { "I10" };
	long tbcol[] = { 1 };
	LONGLONG rowlen, nrows;
	int tfields;
	char extname[FLEN_VALUE];

	call_02(ffinit, &f, "!" test_path);
	call_04(ffphps, f, BYTE_IMG, 0, NULL);
	/* Use ffitab for ASCII tables: naxis1=10 (row length), naxis2=20 (rows). */
	call_09(ffitab, f, 10LL, 20LL, 1, ttype, tbcol, tform, NULL, "ATABLE");
	call_01(ffclos, f);

	call_03(ffopen, &f, test_path, READONLY);
	call_03(ffmahd, f, 2, NULL);
	call_10(ffghtbll, f, 99, &rowlen, &nrows, &tfields, NULL, NULL, NULL, NULL, extname);
	fail_if(nrows != 20);
	fail_if(tfields != 1);
	fail_if(strcmp(extname, "ATABLE") != 0);
	call_01(ffclos, f);
}

static void
test_ffhdr2str_basic(void)
{
	/* Test ffhdr2str - copy header to string. */
	fitsfile *f;
	int status = 0;
	long naxes[] = { 10, 10 };
	char *header;
	int nkeys;

	call_02(ffinit, &f, "!" test_path);
	call_04(ffphps, f, BYTE_IMG, 2, naxes);
	call_01(ffclos, f);

	call_03(ffopen, &f, test_path, READONLY);
	call_06(ffhdr2str, f, 0, NULL, 0, &header, &nkeys);
	fail_if(nkeys < 5);  /* Should have SIMPLE, BITPIX, NAXIS, NAXIS1, NAXIS2, END */
	fail_if(strncmp(header, "SIMPLE", 6) != 0);
	free(header);
	call_01(ffclos, f);
}

static void
test_ffverifydate(void)
{
	/* Test ffverifydate - validate date values. */
	int status = 0;

	/* Valid date. */
	call_03(ffverifydate, 2024, 6, 15);

	/* Invalid month - should set status. */
	status = 0;
	ffverifydate(2024, 13, 15, &status);
	fail_if(status == 0);

	/* Invalid day - should set status. */
	status = 0;
	ffverifydate(2024, 2, 30, &status);
	fail_if(status == 0);
}

static void
test_ffdt2s(void)
{
	/* Test ffdt2s - convert date to string. */
	int status = 0;
	char datestr[32];

	call_04(ffdt2s, 2024, 6, 15, datestr);
	fail_if(strcmp(datestr, "2024-06-15") != 0);

	call_04(ffdt2s, 1999, 12, 31, datestr);
	fail_if(strcmp(datestr, "1999-12-31") != 0);
}

static void
test_ffs2dt(void)
{
	/* Test ffs2dt - convert string to date. */
	int status = 0;
	int year, month, day;

	call_04(ffs2dt, "2024-06-15", &year, &month, &day);
	fail_if(year != 2024);
	fail_if(month != 6);
	fail_if(day != 15);

	/* Old format dd/mm/yy. */
	call_04(ffs2dt, "31/12/99", &year, &month, &day);
	fail_if(year != 1999);
	fail_if(month != 12);
	fail_if(day != 31);
}

static void
test_fftm2s(void)
{
	/* Test fftm2s - convert date/time to string. */
	int status = 0;
	char datestr[32];

	call_08(fftm2s, 2024, 6, 15, 10, 30, 45.5, 3, datestr);
	fail_if(strncmp(datestr, "2024-06-15T10:30:45", 19) != 0);
}

static void
test_ffs2tm(void)
{
	/* Test ffs2tm - convert string to date/time. */
	int status = 0;
	int year, month, day, hour, minute;
	double second;

	call_07(ffs2tm, "2024-06-15T10:30:45.5", &year, &month, &day, &hour, &minute, &second);
	fail_if(year != 2024);
	fail_if(month != 6);
	fail_if(day != 15);
	fail_if(hour != 10);
	fail_if(minute != 30);
	fail_if(second < 45.4 || second > 45.6);
}

static void
test_ffgsdt(void)
{
	/* Test ffgsdt - get system date. */
	int status = 0;
	int day, month, year;

	call_03(ffgsdt, &day, &month, &year);
	/* Should return reasonable values. */
	fail_if(year < 2020 || year > 2100);
	fail_if(month < 1 || month > 12);
	fail_if(day < 1 || day > 31);
}

static void
test_ffpknjj(void)
{
	/* Test ffpknjj - write indexed LONGLONG keywords. */
	fitsfile *f;
	int status = 0;
	long naxes[] = { 10, 10 };
	LONGLONG values[] = { 100LL, 200LL, 300LL };
	LONGLONG result[3];
	int nfound;

	call_02(ffinit, &f, "!" test_path);
	call_04(ffphps, f, BYTE_IMG, 2, naxes);
	/* Write TEST1, TEST2, TEST3 keywords. */
	call_06(ffpknjj, f, "TEST", 1, 3, values, NULL);
	call_01(ffclos, f);

	call_03(ffopen, &f, test_path, READONLY);
	call_06(ffgknjj, f, "TEST", 1, 3, result, &nfound);
	fail_if(nfound != 3);
	fail_if(result[0] != 100LL);
	fail_if(result[1] != 200LL);
	fail_if(result[2] != 300LL);
	call_01(ffclos, f);
}

/*
 * ffgttb validates the mandatory table keywords, so exercising it needs
 * headers the library itself would never write.  Build them by hand.
 */

/* Write one 80 column card, blank padded. */
static void
put_card(FILE *fp, long *nbytes, const char *card)
{
	char buf[81];

	snprintf(buf, sizeof(buf), "%-80.80s", card);
	fwrite(buf, 1, 80, fp);
	*nbytes += 80;
}

/* Pad with blanks out to the next 2880 byte block. */
static void
pad_block(FILE *fp, long *nbytes)
{
	while (*nbytes % 2880) {
		fputc(' ', fp);
		(*nbytes)++;
	}
}

/*
 * An empty primary array followed by a one column binary table.  The three
 * cards ffgttb is being tested on are passed in whole.
 */
static void
write_table_file(const char *path, const char *naxis1, const char *naxis2,
		 const char *tfields)
{
	FILE *fp = fopen(path, "wb");
	long n = 0;
	int i;

	fail_if(fp == NULL);

	put_card(fp, &n, "SIMPLE  =                    T");
	put_card(fp, &n, "BITPIX  =                    8");
	put_card(fp, &n, "NAXIS   =                    0");
	put_card(fp, &n, "EXTEND  =                    T");
	put_card(fp, &n, "END");
	pad_block(fp, &n);

	put_card(fp, &n, "XTENSION= 'BINTABLE'");
	put_card(fp, &n, "BITPIX  =                    8");
	put_card(fp, &n, "NAXIS   =                    2");
	put_card(fp, &n, naxis1);
	put_card(fp, &n, naxis2);
	put_card(fp, &n, "PCOUNT  =                    0");
	put_card(fp, &n, "GCOUNT  =                    1");
	put_card(fp, &n, tfields);
	put_card(fp, &n, "TFORM1  = '10A     '");
	put_card(fp, &n, "TTYPE1  = 'COL1    '");
	put_card(fp, &n, "END");
	pad_block(fp, &n);

	for (i = 0; i < 10; i++) {
		fputc('x', fp);
		n++;
	}
	pad_block(fp, &n);

	fail_if(fclose(fp) != 0);
}

/* Status left by moving to the table, or 0 if it was accepted. */
static int
move_to_table(void)
{
	fitsfile *f;
	int status = 0;
	int closestatus = 0;
	int hdutype = 0;

	call_03(ffopen, &f, test_path, READONLY);
	ffmahd(f, 2, &hdutype, &status);
	ffclos(f, &closestatus);
	fail_if(closestatus != 0);
	ffcmsg();
	return status;
}

static void
test_ffgttb_bad_keywords(void)
{
	/*
	 * A value that is not a positive integer has to be reported as such.
	 * These three checks used to compare where they meant to assign, so
	 * ffgttb returned 0 with the status still set to NOT_POS_INT: the
	 * caller carried on into ffbinit with uninitialised keyword values
	 * and reported whatever error that ran into next.
	 */
	write_table_file(test_path, "NAXIS1  =                   -1",
			 "NAXIS2  =                    1",
			 "TFIELDS =                    1");
	fail_if(move_to_table() != BAD_NAXES);

	write_table_file(test_path, "NAXIS1  =                   10",
			 "NAXIS2  =                   -1",
			 "TFIELDS =                    1");
	fail_if(move_to_table() != BAD_NAXES);

	/* Not an integer at all. */
	write_table_file(test_path, "NAXIS1  = 'abc'",
			 "NAXIS2  =                    1",
			 "TFIELDS =                    1");
	fail_if(move_to_table() != BAD_NAXES);

	write_table_file(test_path, "NAXIS1  =                   10",
			 "NAXIS2  =                    1",
			 "TFIELDS =                   -1");
	fail_if(move_to_table() != BAD_TFIELDS);

	/*
	 * More than 999 fields cannot be written as valid FITS - TFORM1000
	 * is a nine character keyword name - and the limit is checked for,
	 * but the result of the check was being discarded.
	 */
	write_table_file(test_path, "NAXIS1  =                   10",
			 "NAXIS2  =                    1",
			 "TFIELDS =                 1000");
	fail_if(move_to_table() != BAD_TFIELDS);
}

static void
test_ffgttb_good_keywords(void)
{
	fitsfile *f;
	int status = 0;
	int hdutype = 0;
	LONGLONG nrows = 0;
	int ncols = 0;

	write_table_file(test_path, "NAXIS1  =                   10",
			 "NAXIS2  =                    1",
			 "TFIELDS =                    1");

	call_03(ffopen, &f, test_path, READONLY);
	call_03(ffmahd, f, 2, &hdutype);
	fail_if(hdutype != BINARY_TBL);
	call_02(ffgnrwll, f, &nrows);
	call_02(ffgncl, f, &ncols);
	fail_if(nrows != 1);
	fail_if(ncols != 1);
	call_01(ffclos, f);

	/*
	 * Zero is a legal value for these keywords, so ffgttb has nothing to
	 * say about this header; the complaint comes from the later check
	 * that the column widths add up to NAXIS1.
	 */
	write_table_file(test_path, "NAXIS1  =                    0",
			 "NAXIS2  =                    0",
			 "TFIELDS =                    1");
	fail_if(move_to_table() != BAD_ROW_WIDTH);
}

int
main(void)
{
	test_ffgknjj_basic();
	test_ffgknjj_large_values();
	test_ffgtdmll_basic();
	test_ffdtdmll_basic();
	test_ffghbnll_basic();
	test_ffghtbll_basic();
	test_ffhdr2str_basic();

	/* Date/time functions */
	test_ffverifydate();
	test_ffdt2s();
	test_ffs2dt();
	test_fftm2s();
	test_ffs2tm();
	test_ffgsdt();

	/* Keyword writing */
	test_ffpknjj();

	/* Mandatory table keywords */
	test_ffgttb_good_keywords();
	test_ffgttb_bad_keywords();

	return 0;
}
