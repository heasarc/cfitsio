/*
 * Tests for the fitsverify utility (utilities/fvrf_*.c).
 *
 * Routines that can be driven on their own are called in process: their
 * source file is #included, and the symbols utilities/ftverify.c would
 * otherwise provide are stubbed below.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int
main(void)
{
	test_complex_keyword_values();
	return 0;
}
