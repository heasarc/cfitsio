/*
 * Tests for the fitsverify utility (utilities/fvrf_*.c).
 *
 * Routines that can be driven on their own are called in process: their
 * source file is #included, and the symbols utilities/ftverify.c would
 * otherwise provide are stubbed below.  Defects that only show up on a
 * whole file are exercised by writing that file and running the fitsverify
 * binary over it (not on Windows).
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
#include <unistd.h>
#include <sys/wait.h>
#endif

#include "fitsio.h"
#include "test_macros.h"

/* Globals that utilities/ftverify.c owns in the real program. */
int prhead = 0;
int testdata = 1;
int testfill = 1;
int testcsum = 1;
int totalhdu = 0;
int err_report = 0;
int heasarc_conv = 1;
int testhierarch = 0;
int prstat = 0;

/* Only reached after MAXERRORS reports, which these tests stay well under. */
void update_parfile(int nerr, int nwrn) { (void)nerr; (void)nwrn; }
void close_report(FILE *out) { (void)out; }

#include "../utilities/fvrf_key.c"
#include "../utilities/fvrf_misc.c"

/*--------------------------------------------------------------------------
 * get_cmp: complex keyword values (utilities/fvrf_key.c)
 *------------------------------------------------------------------------*/

/* Parse a complex keyword value and return the accumulated error mask. */
static unsigned long
parse_cmp(const char *value, char *kvalue, kwdtyp *ktype)
{
	char card[FLEN_CARD];
	char *p = card;
	unsigned long stat = 0;

	memset(card, 0, sizeof(card));
	strcpy(card, value);
	kvalue[0] = '\0';
	*ktype = UNKNOWN;
	get_cmp(&p, kvalue, ktype, &stat);
	return stat;
}

static void
test_complex_keyword_values(void)
{
	char kvalue[FLEN_CARD];
	kwdtyp ktype;
	unsigned long stat;

	stat = parse_cmp("(1,2)", kvalue, &ktype);
	fail_if(stat != 0);
	fail_if(ktype != CMI_KEY);

	stat = parse_cmp("(1.5,-2.5)", kvalue, &ktype);
	fail_if(stat != 0);
	fail_if(ktype != CMF_KEY);

	/* No comma: used to write through a NULL pointer. */
	stat = parse_cmp("(1 2)", kvalue, &ktype);
	fail_if(!(stat & NO_COMMA));

	stat = parse_cmp("(1 2", kvalue, &ktype);
	fail_if(!(stat & NO_COMMA));
	fail_if(!(stat & NO_TRAIL_PAREN));

	stat = parse_cmp("(", kvalue, &ktype);
	fail_if(!(stat & NO_COMMA));

	stat = parse_cmp("(1,2", kvalue, &ktype);
	fail_if(stat & NO_COMMA);
	fail_if(!(stat & NO_TRAIL_PAREN));

	stat = parse_cmp("(1,2,3)", kvalue, &ktype);
	fail_if(!(stat & TOO_MANY_COMMA));
}

#ifndef _WIN32
/*--------------------------------------------------------------------------
 * Running the fitsverify binary over a file built for the purpose
 *------------------------------------------------------------------------*/

#define FITSVERIFY	"./fitsverify"
#define TESTFILE	"test_fitsverify.fits"
#define REPORT		"test_fitsverify_report.txt"

/* Write one 80 column card, blank padded. */
static void
put_card(FILE *fp, long *nbytes, const char *card)
{
	char buf[81];

	snprintf(buf, sizeof(buf), "%-80.80s", card);
	fwrite(buf, 1, 80, fp);
	*nbytes += 80;
}

/* Write count copies of one byte. */
static void
put_bytes(FILE *fp, long *nbytes, int byte, long count)
{
	long i;

	for (i = 0; i < count; i++) {
		fputc(byte, fp);
		(*nbytes)++;
	}
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

/* Start TESTFILE with an empty primary array that allows extensions. */
static FILE *
open_testfile(long *nbytes)
{
	FILE *fp = fopen(TESTFILE, "wb");

	fail_if(fp == NULL);
	*nbytes = 0;
	put_card(fp, nbytes, "SIMPLE  =                    T");
	put_card(fp, nbytes, "BITPIX  =                    8");
	put_card(fp, nbytes, "NAXIS   =                    0");
	put_card(fp, nbytes, "EXTEND  =                    T");
	put_card(fp, nbytes, "END");
	pad_block(fp, nbytes);
	return fp;
}

static void
close_testfile(FILE *fp, long *nbytes)
{
	pad_block(fp, nbytes);
	fail_if(fclose(fp) != 0);
}

/*
 * Run fitsverify on TESTFILE, with its output in REPORT, and return its exit
 * status.  fork/exec rather than system(), so that a fatal signal is seen
 * as such rather than as the shell's 128+n exit status.
 */
static int
check_no_crash(const char *what)
{
	pid_t pid;
	int st = 0;

	pid = fork();
	fail_if(pid < 0);
	if (pid == 0) {
		if (freopen(REPORT, "w", stdout) == NULL)
			_exit(126);
		if (dup2(fileno(stdout), fileno(stderr)) < 0)
			_exit(126);
		execl(FITSVERIFY, FITSVERIFY, TESTFILE, (char *)NULL);
		_exit(127);
	}
	fail_if(waitpid(pid, &st, 0) != pid);

	if (WIFSIGNALED(st))
		fprintf(stderr, "fitsverify died on %s with signal %d\n",
			what, WTERMSIG(st));
	fail_if(WIFSIGNALED(st));
	/* 126/127: the binary could not be run at all. */
	fail_if(WEXITSTATUS(st) == 126 || WEXITSTATUS(st) == 127);
	return WEXITSTATUS(st);
}

/* Count occurrences of a string in REPORT. */
static int
count_in_report(const char *needle)
{
	FILE *fp = fopen(REPORT, "r");
	char line[4096];
	int nfound = 0;

	fail_if(fp == NULL);
	while (fgets(line, sizeof(line), fp) != NULL) {
		char *p = line;

		while ((p = strstr(p, needle)) != NULL) {
			nfound++;
			p += strlen(needle);
		}
	}
	fclose(fp);
	return nfound;
}

/*--------------------------------------------------------------------------
 * test_bin_ext: the "rAw" TFORM check (utilities/fvrf_head.c)
 *------------------------------------------------------------------------*/

/* A one column binary table with the given TFORM. */
static void
write_bintable_column(const char *tform)
{
	long n;
	FILE *fp = open_testfile(&n);
	char card[81];

	put_card(fp, &n, "XTENSION= 'BINTABLE'");
	put_card(fp, &n, "BITPIX  =                    8");
	put_card(fp, &n, "NAXIS   =                    2");
	put_card(fp, &n, "NAXIS1  =                   10");
	put_card(fp, &n, "NAXIS2  =                    1");
	put_card(fp, &n, "PCOUNT  =                    0");
	put_card(fp, &n, "GCOUNT  =                    1");
	put_card(fp, &n, "TFIELDS =                    1");
	snprintf(card, sizeof(card), "TFORM1  = '%-8s'", tform);
	put_card(fp, &n, card);
	put_card(fp, &n, "TTYPE1  = 'COL1    '");
	put_card(fp, &n, "END");
	pad_block(fp, &n);

	put_bytes(fp, &n, 'x', 10);
	close_testfile(fp, &n);
}

static void
test_tform_substring_width(void)
{
	/* A zero substring width used to raise SIGFPE. */
	static const char *tforms[] = { "10A0", "0A0", "3A0", "10A", "10A5" };
	size_t i;

	for (i = 0; i < sizeof(tforms) / sizeof(tforms[0]); i++) {
		write_bintable_column(tforms[i]);
		check_no_crash(tforms[i]);
	}

	write_bintable_column("10A0");
	check_no_crash("10A0");
	fail_if(count_in_report("substring width must not be zero") != 1);
}

/*--------------------------------------------------------------------------
 * iterdata: the bit column justification report (utilities/fvrf_data.c)
 *------------------------------------------------------------------------*/

/*
 * A binary table holding a single X column of nbits bits, all set.  nbits
 * is not a multiple of 8, so fill bits are set and the "not left
 * justified" report is produced.
 */
static void
write_bit_column_table(int nbits)
{
	long n;
	FILE *fp = open_testfile(&n);
	long nbytes = (nbits + 7) / 8;
	char card[81];

	put_card(fp, &n, "XTENSION= 'BINTABLE'");
	put_card(fp, &n, "BITPIX  =                    8");
	put_card(fp, &n, "NAXIS   =                    2");
	snprintf(card, sizeof(card), "NAXIS1  = %20ld", nbytes);
	put_card(fp, &n, card);
	put_card(fp, &n, "NAXIS2  =                    1");
	put_card(fp, &n, "PCOUNT  =                    0");
	put_card(fp, &n, "GCOUNT  =                    1");
	put_card(fp, &n, "TFIELDS =                    1");
	snprintf(card, sizeof(card), "TFORM1  = '%dX'", nbits);
	put_card(fp, &n, card);
	put_card(fp, &n, "TTYPE1  = 'BITS    '");
	put_card(fp, &n, "END");
	pad_block(fp, &n);

	put_bytes(fp, &n, 0xff, nbytes);
	close_testfile(fp, &n);
}

static void
test_bit_column_report(void)
{
	/* 100 bits: the report fits in errmes. */
	write_bit_column_table(100);
	check_no_crash("a 100X column");
	fail_if(count_in_report("is not left justified.") != 1);
	fail_if(count_in_report("0x") != 13);

	/* 3996 bits: one element per byte used to overflow errmes. */
	write_bit_column_table(3996);
	check_no_crash("a 3996X column");
	fail_if(count_in_report("... is not left justified.") != 1);
	fail_if(count_in_report("0x") > 256 / 5);
}

/*--------------------------------------------------------------------------
 * init_hdu: the card number printed for the first cards of an HDU
 * (utilities/fvrf_head.c)
 *------------------------------------------------------------------------*/

static void
test_first_card_keyword_index(void)
{
	long n;
	FILE *fp = open_testfile(&n);

	/* An XTENSION value that is not a string. */
	put_card(fp, &n, "XTENSION=                 1234");
	put_card(fp, &n, "BITPIX  =                    8");
	put_card(fp, &n, "NAXIS   =                    2");
	put_card(fp, &n, "NAXIS1  =                   10");
	put_card(fp, &n, "NAXIS2  =                    1");
	put_card(fp, &n, "PCOUNT  =                    0");
	put_card(fp, &n, "GCOUNT  =                    1");
	put_card(fp, &n, "TFIELDS =                    1");
	put_card(fp, &n, "TFORM1  = '10A     '");
	put_card(fp, &n, "TTYPE1  = 'COL1    '");
	put_card(fp, &n, "END");
	pad_block(fp, &n);
	put_bytes(fp, &n, 'x', 10);
	close_testfile(fp, &n);

	/* The card number used to be an uninitialised stack value. */
	check_no_crash("an XTENSION value that is not a string");
	fail_if(count_in_report(
	    "Keyword #1, XTENSION: \"1234\" is not a string.") != 1);
}

/*--------------------------------------------------------------------------
 * test_agap: the column template (utilities/fvrf_data.c)
 *------------------------------------------------------------------------*/

/* A single column ASCII table with the given row length, start and format. */
static void
write_ascii_table(long naxis1, long tbcol, const char *tform)
{
	long n;
	FILE *fp = open_testfile(&n);
	char card[81];

	put_card(fp, &n, "XTENSION= 'TABLE   '");
	put_card(fp, &n, "BITPIX  =                    8");
	put_card(fp, &n, "NAXIS   =                    2");
	snprintf(card, sizeof(card), "NAXIS1  = %20ld", naxis1);
	put_card(fp, &n, card);
	put_card(fp, &n, "NAXIS2  =                    1");
	put_card(fp, &n, "PCOUNT  =                    0");
	put_card(fp, &n, "GCOUNT  =                    1");
	put_card(fp, &n, "TFIELDS =                    1");
	put_card(fp, &n, "TTYPE1  = 'COL1    '");
	snprintf(card, sizeof(card), "TBCOL1  = %20ld", tbcol);
	put_card(fp, &n, card);
	snprintf(card, sizeof(card), "TFORM1  = '%-8s'", tform);
	put_card(fp, &n, card);
	put_card(fp, &n, "END");
	pad_block(fp, &n);

	put_bytes(fp, &n, '1', naxis1);
	close_testfile(fp, &n);
}

static void
test_ascii_column_template(void)
{
	write_ascii_table(10, 1, "A10");
	check_no_crash("an ASCII table");

	/*
	 * CFITSIO does not range check TBCOLn when NAXIS1 is 0, and the
	 * template used to be written at TBCOLn regardless of its size.
	 */
	write_ascii_table(0, 1, "A20");
	check_no_crash("NAXIS1 = 0, TBCOL1 = 1");

	write_ascii_table(0, 100000000, "A20");
	check_no_crash("NAXIS1 = 0, TBCOL1 = 100000000");
}

static void
run_fitsverify_tests(void)
{
	if (access(FITSVERIFY, X_OK) != 0) {
		fprintf(stderr, "%s has not been built; skipping the tests "
			"that need it\n", FITSVERIFY);
		return;
	}

	test_tform_substring_width();
	test_bit_column_report();
	test_first_card_keyword_index();
	test_ascii_column_template();

	remove(TESTFILE);
	remove(REPORT);
}
#endif /* !_WIN32 */

int
main(void)
{
	test_complex_keyword_values();
#ifndef _WIN32
	run_fitsverify_tests();
#endif
	return 0;
}
