/*
 * Tests for region.c region file functions.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "fitsio.h"
#include "region.h"
#include "test_macros.h"

#define region_file "test_region.reg"
#define region_fits_file "test_region.fits"

static void
write_region_file(const char *content)
{
	FILE *f = fopen(region_file, "w");
	if (f) {
		fputs(content, f);
		fclose(f);
	}
}

static void
test_circle_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("circle(100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != circle_rgn);
	fail_if(rgn->Shapes[0].sign != 1);

	/* Test points inside and outside */
	fail_if(fits_in_region(100, 100, rgn) != 1);  /* center */
	fail_if(fits_in_region(140, 100, rgn) != 1);  /* inside edge */
	fail_if(fits_in_region(200, 100, rgn) != 0);  /* outside */
	fail_if(fits_in_region(0, 0, rgn) != 0);      /* far outside */

	fits_free_region(rgn);
}

static void
test_annulus_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("annulus(100,100,30,60)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != annulus_rgn);

	/* Test points inside, outside, and in hole */
	fail_if(fits_in_region(100, 100, rgn) != 0);  /* center (hole) */
	fail_if(fits_in_region(145, 100, rgn) != 1);  /* in annulus */
	fail_if(fits_in_region(200, 100, rgn) != 0);  /* outside */

	fits_free_region(rgn);
}

static void
test_box_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("box(100,100,40,60,0)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != box_rgn);

	/* Test points inside and outside */
	fail_if(fits_in_region(100, 100, rgn) != 1);  /* center */
	fail_if(fits_in_region(115, 125, rgn) != 1);  /* corner inside */
	fail_if(fits_in_region(150, 150, rgn) != 0);  /* outside */

	fits_free_region(rgn);
}

static void
test_rotbox_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("rotbox(100,100,40,60,45)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != box_rgn);  /* rotbox is still box_rgn */

	/* Test points */
	fail_if(fits_in_region(100, 100, rgn) != 1);  /* center */

	fits_free_region(rgn);
}

static void
test_ellipse_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("ellipse(100,100,50,30,0)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != ellipse_rgn);

	/* Test points */
	fail_if(fits_in_region(100, 100, rgn) != 1);  /* center */
	fail_if(fits_in_region(140, 100, rgn) != 1);  /* along major axis */
	fail_if(fits_in_region(100, 120, rgn) != 1);  /* along minor axis */
	fail_if(fits_in_region(200, 200, rgn) != 0);  /* outside */

	fits_free_region(rgn);
}

static void
test_elliptannulus_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("elliptannulus(100,100,20,10,40,20,0,0)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != elliptannulus_rgn);

	/* Test points */
	fail_if(fits_in_region(100, 100, rgn) != 0);  /* center (hole) */
	fail_if(fits_in_region(130, 100, rgn) != 1);  /* in annulus */

	fits_free_region(rgn);
}

static void
test_rectangle_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("rectangle(50,50,150,150,0)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != rectangle_rgn);

	/* Test points */
	fail_if(fits_in_region(100, 100, rgn) != 1);  /* center */
	fail_if(fits_in_region(60, 60, rgn) != 1);    /* inside */
	fail_if(fits_in_region(200, 200, rgn) != 0);  /* outside */

	fits_free_region(rgn);
}

static void
test_diamond_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("diamond(100,100,60,40,0)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != diamond_rgn);

	/* Test points */
	fail_if(fits_in_region(100, 100, rgn) != 1);  /* center */
	fail_if(fits_in_region(200, 200, rgn) != 0);  /* outside */

	fits_free_region(rgn);
}

static void
test_rhombus_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("rhombus(100,100,60,40,0)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != diamond_rgn);

	fits_free_region(rgn);
}

static void
test_sector_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("sector(100,100,0,90)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != sector_rgn);

	/* Test points in different quadrants */
	fail_if(fits_in_region(150, 150, rgn) != 1);  /* first quadrant */
	fail_if(fits_in_region(50, 150, rgn) != 0);   /* second quadrant */
	fail_if(fits_in_region(50, 50, rgn) != 0);    /* third quadrant */

	fits_free_region(rgn);
}

static void
test_pie_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("pie(100,100,-45,45)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != sector_rgn);

	fits_free_region(rgn);
}

static void
test_point_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("point(100,100)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != point_rgn);

	/* Point matches only at exact location */
	fail_if(fits_in_region(100, 100, rgn) != 1);
	fail_if(fits_in_region(101, 100, rgn) != 0);

	fits_free_region(rgn);
}

static void
test_line_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("line(50,50,150,150)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != line_rgn);

	/* Points on line */
	fail_if(fits_in_region(50, 50, rgn) != 1);
	fail_if(fits_in_region(100, 100, rgn) != 1);

	fits_free_region(rgn);
}

static void
test_polygon_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Square polygon */
	write_region_file("polygon(50,50,150,50,150,150,50,150)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != poly_rgn);

	/* Test points */
	fail_if(fits_in_region(100, 100, rgn) != 1);  /* center */
	fail_if(fits_in_region(60, 60, rgn) != 1);    /* inside */
	fail_if(fits_in_region(200, 200, rgn) != 0);  /* outside */
	fail_if(fits_in_region(30, 30, rgn) != 0);    /* outside */

	fits_free_region(rgn);
}

static void
test_panda_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("panda(100,100,0,90,2,20,60,2)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != panda_rgn);

	/* Test points */
	fail_if(fits_in_region(100, 100, rgn) != 0);  /* center (hole) */
	fail_if(fits_in_region(140, 140, rgn) != 1);  /* in panda sector */

	fits_free_region(rgn);
}

static void
test_epanda_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("epanda(100,100,0,90,2,20,10,40,20,2,0)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != epanda_rgn);

	fits_free_region(rgn);
}

static void
test_bpanda_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("bpanda(100,100,0,90,2,30,20,60,40,2,0)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != bpanda_rgn);

	fits_free_region(rgn);
}

static void
test_boxannulus_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("box(100,100,20,20,40,40,0,0)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != boxannulus_rgn);

	/* Test points */
	fail_if(fits_in_region(100, 100, rgn) != 0);  /* center (hole) */
	fail_if(fits_in_region(115, 100, rgn) != 1);  /* in annulus */

	fits_free_region(rgn);
}

static void
test_excluded_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("circle(100,100,50)\n-circle(100,100,20)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 2);
	fail_if(rgn->Shapes[1].sign != 0);  /* excluded */

	/* Test that inner region is excluded */
	fail_if(fits_in_region(100, 100, rgn) != 0);  /* center excluded */
	fail_if(fits_in_region(140, 100, rgn) != 1);  /* outer ring included */

	fits_free_region(rgn);
}

static void
test_plus_sign_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("+circle(100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].sign != 1);

	fits_free_region(rgn);
}

static void
test_multiple_regions(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("circle(50,50,20)\ncircle(150,150,20)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 2);

	/* Test points in each region */
	fail_if(fits_in_region(50, 50, rgn) != 1);
	fail_if(fits_in_region(150, 150, rgn) != 1);
	fail_if(fits_in_region(100, 100, rgn) != 0);  /* between regions */

	fits_free_region(rgn);
}

static void
test_comment_lines(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("# This is a comment\ncircle(100,100,50)\n# Another comment\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);

	fits_free_region(rgn);
}

static void
test_format_pixel(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("# format: pixel\ncircle(100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);

	fits_free_region(rgn);
}

static void
test_image_prefix(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("image;circle(100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);

	fits_free_region(rgn);
}

static void
test_physical_prefix(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("physical;circle(100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);

	fits_free_region(rgn);
}

static void
test_linear_prefix(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("linear;circle(100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);

	fits_free_region(rgn);
}

static void
test_global_line(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("global color=green\ncircle(100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);

	fits_free_region(rgn);
}

static void
test_image_format_line(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* ds9/POW format with standalone format code */
	write_region_file("image\ncircle(100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);

	fits_free_region(rgn);
}

static void
test_physical_format_line(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("physical\ncircle(100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);

	fits_free_region(rgn);
}

static void
test_missing_file(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	fits_read_ascii_region("nonexistent_region_file.reg", NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_empty_file(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* empty file should fail */
}

static void
test_bad_syntax_unmatched_paren(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("circle(100,100,50\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_bad_syntax_double_open(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("circle((100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_bad_syntax_double_close(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("circle(100,100,50))\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_unrecognized_shape(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("unknownshape(100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_wrong_param_count_circle(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("circle(100,100)\n");  /* missing radius */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_wrong_param_count_annulus(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("annulus(100,100,30)\n");  /* missing outer radius */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_wrong_param_count_polygon(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Polygon needs at least 6 params (3 points) and even count */
	write_region_file("polygon(100,100,200,100)\n");  /* only 2 points */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_ellipse_with_4_params(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Ellipse with 4 params defaults angle to 0 */
	write_region_file("ellipse(100,100,50,30)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->Shapes[0].shape != ellipse_rgn);

	fits_free_region(rgn);
}

static void
test_box_with_4_params(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Box with 4 params defaults angle to 0 */
	write_region_file("box(100,100,50,30)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->Shapes[0].shape != box_rgn);

	fits_free_region(rgn);
}

static void
test_ellipse_6_params_annulus(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Ellipse with 6+ params becomes elliptannulus */
	write_region_file("ellipse(100,100,20,10,40,20)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->Shapes[0].shape != elliptannulus_rgn);

	fits_free_region(rgn);
}

static void
test_box_6_params_annulus(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Box with 6+ params becomes boxannulus */
	write_region_file("box(100,100,20,20,40,40)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->Shapes[0].shape != boxannulus_rgn);

	fits_free_region(rgn);
}

static void
test_rotrectangle(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("rotrectangle(50,50,150,150,45)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->Shapes[0].shape != rectangle_rgn);

	fits_free_region(rgn);
}

static void
test_rotdiamond(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("rotdiamond(100,100,50,30,45)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->Shapes[0].shape != diamond_rgn);

	fits_free_region(rgn);
}

static void
test_rotrhombus(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("rotrhombus(100,100,50,30,45)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->Shapes[0].shape != diamond_rgn);

	fits_free_region(rgn);
}

static void
test_sector_wraparound(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Sector that wraps around 180 degrees */
	write_region_file("sector(100,100,170,-170)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->Shapes[0].shape != sector_rgn);

	/* Point to the left (180 degrees) should be included */
	fail_if(fits_in_region(50, 100, rgn) != 1);
	/* Point to the right should be excluded */
	fail_if(fits_in_region(150, 100, rgn) != 0);

	fits_free_region(rgn);
}

static void
test_format_after_shapes_error(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Format after shapes should error */
	write_region_file("circle(100,100,50)\n# format: pixel\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_unknown_format_error(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("# format: unknown\ncircle(100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_galactic_not_supported(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("galactic;circle(100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail - galactic not supported */
}

static void
test_ecliptic_not_supported(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("ecliptic;circle(100,100,50)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail - ecliptic not supported */
}

static void
test_fits_read_rgnfile(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* fits_read_rgnfile tries FITS first, then ASCII */
	write_region_file("circle(100,100,50)\n");
	fits_read_rgnfile(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	fits_free_region(rgn);
}

static void
test_blank_lines(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("\n\ncircle(100,100,50)\n\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);

	fits_free_region(rgn);
}

static void
test_trailing_comment(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("circle(100,100,50) # comment\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);

	fits_free_region(rgn);
}

static void
test_complex_polygon(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* L-shaped polygon */
	write_region_file("polygon(0,0,50,0,50,50,100,50,100,100,0,100)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->Shapes[0].shape != poly_rgn);
	fail_if(rgn->Shapes[0].param.poly.nPts != 12);

	/* Test point inside L */
	fail_if(fits_in_region(25, 75, rgn) != 1);
	/* Test point in corner outside L */
	fail_if(fits_in_region(75, 25, rgn) != 0);

	fits_free_region(rgn);
}

static void
test_fk5_format(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* FK5 format line alone */
	write_region_file("fk5\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	/* This should succeed but with 0 shapes or fail for empty */
	/* Behavior depends on implementation */
}

static void
test_fk4_format(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* FK4 format line alone */
	write_region_file("fk4\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
}

static void
test_icrs_format(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* ICRS format line alone */
	write_region_file("icrs\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
}

static void
test_fk5_with_circle(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* FK5 format with a circle shape - requires WCS to work fully */
	write_region_file("fk5\ncircle(12.5,45.0,0.1)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	/* Without WCS, this will fail with status 505 (WCS error) */
	/* The important thing is it parses the format and shape names */
	if (rgn) {
		fits_free_region(rgn);
	}
}

static void
test_icrs_with_circle(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* ICRS format with a circle shape - requires WCS */
	write_region_file("icrs\ncircle(180.0,30.0,0.5)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	if (rgn) {
		fits_free_region(rgn);
	}
}

static void
test_j2000_format(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* J2000 format with a circle shape - requires WCS */
	write_region_file("j2000\ncircle(90.0,45.0,0.1)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	if (rgn) {
		fits_free_region(rgn);
	}
}

static void
test_hhmmss_coords(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* HHMMSS format with pixel format to avoid WCS requirement */
	write_region_file("image\ncircle(12:30:00,+45:00:00,10)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	/* Colon format in image coords gets parsed but values differ */
	if (rgn) {
		fits_free_region(rgn);
	}
}

static void
test_hhmmss_negative_dec(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Test format parsing with colons - may need WCS for full behavior */
	write_region_file("image\ncircle(6:15:30,-30:45:15,10)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	if (rgn) {
		fits_free_region(rgn);
	}
}

static void
test_colon_in_param(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Colon in parameter - test parsing */
	write_region_file("image\ncircle(1:00:00,45.0,10)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	if (rgn) {
		fits_free_region(rgn);
	}
}

static void
test_very_long_line(void)
{
	SAORegion *rgn = NULL;
	int status = 0;
	char longline[2000];
	int i;

	/* Create a polygon with many vertices to make a long line */
	strcpy(longline, "polygon(");
	for (i = 0; i < 50; i += 1) {
		char buf[30];
		sprintf(buf, "%.1f,%.1f", 100.0 + i * 2, 100.0 + (i % 5) * 10);
		if (i > 0) {
			strcat(longline, ",");
		}
		strcat(longline, buf);
	}
	strcat(longline, ")\n");

	write_region_file(longline);
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != poly_rgn);

	fits_free_region(rgn);
}

static void
test_elliptannulus_7_params(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* With 7 params via "ellipse" keyword, angle is duplicated */
	write_region_file("ellipse(100,100,20,10,40,20,45)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->Shapes[0].shape != elliptannulus_rgn);

	fits_free_region(rgn);
}

static void
test_boxannulus_7_params(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* With 7 params, angle is duplicated */
	write_region_file("box(100,100,20,20,40,40,45)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->Shapes[0].shape != boxannulus_rgn);

	fits_free_region(rgn);
}

static void
test_epanda_10_params(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("epanda(100,100,0,90,2,20,10,40,20,2)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->Shapes[0].shape != epanda_rgn);

	fits_free_region(rgn);
}

static void
test_bpanda_10_params(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("bpanda(100,100,0,90,2,30,20,60,40,2)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->Shapes[0].shape != bpanda_rgn);

	fits_free_region(rgn);
}

static void
test_fits_in_region_center(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Test fits_in_region at exact center for various shapes */
	write_region_file("circle(0,0,10)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);

	/* Center of coordinate system */
	fail_if(fits_in_region(0, 0, rgn) != 1);

	fits_free_region(rgn);
}

static void
test_excluded_polygon(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Polygon with exclusion - tests shallow copy handling in free */
	write_region_file("polygon(0,0,100,0,100,100,0,100)\n-polygon(25,25,75,25,75,75,25,75)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	/* Point in outer polygon but outside inner (excluded) */
	fail_if(fits_in_region(10, 10, rgn) != 1);
	/* Point in inner excluded polygon */
	fail_if(fits_in_region(50, 50, rgn) != 0);

	fits_free_region(rgn);
}

static void
test_multiple_excluded_regions(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("circle(100,100,50)\n-circle(100,100,10)\n-circle(120,100,10)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	/* Tests component handling with multiple exclusions */
	fail_if(fits_in_region(100, 100, rgn) != 0);  /* excluded center */
	fail_if(fits_in_region(120, 100, rgn) != 0);  /* excluded offset */
	fail_if(fits_in_region(80, 100, rgn) != 1);   /* included area */

	fits_free_region(rgn);
}

static void
test_panda_center_excluded(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Panda has inner radius - center should be excluded */
	write_region_file("panda(100,100,0,360,1,20,60,1)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	fail_if(fits_in_region(100, 100, rgn) != 0);  /* center excluded */
	fail_if(fits_in_region(140, 100, rgn) != 1);  /* in panda ring */

	fits_free_region(rgn);
}

static void
test_bpanda_in_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("bpanda(100,100,-45,45,1,20,20,60,60,1,0)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	/* Test center excluded */
	fail_if(fits_in_region(100, 100, rgn) != 0);
	/* Test point in correct angle and radius */
	fail_if(fits_in_region(125, 100, rgn) != 1);
	/* Test point outside angle range */
	fail_if(fits_in_region(100, 125, rgn) != 0);

	fits_free_region(rgn);
}

static void
test_epanda_in_region(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("epanda(100,100,-45,45,1,10,5,30,15,1,0)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	/* Test center excluded */
	fail_if(fits_in_region(100, 100, rgn) != 0);
	/* Test point in correct angle and elliptical radius */
	fail_if(fits_in_region(120, 100, rgn) != 1);

	fits_free_region(rgn);
}

static void
test_many_regions_realloc(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* 11 regions to trigger realloc of Shapes array (initial alloc is 10) */
	write_region_file(
		"circle(10,10,5)\n"
		"circle(20,10,5)\n"
		"circle(30,10,5)\n"
		"circle(40,10,5)\n"
		"circle(50,10,5)\n"
		"circle(60,10,5)\n"
		"circle(70,10,5)\n"
		"circle(80,10,5)\n"
		"circle(90,10,5)\n"
		"circle(100,10,5)\n"
		"circle(110,10,5)\n"
	);
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 11);

	fits_free_region(rgn);
}

static void
test_wrong_param_count_sector(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("sector(100,100,45)\n");  /* missing end angle */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_wrong_param_count_point(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("point(100)\n");  /* missing Y */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_wrong_param_count_line(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("line(100,100,200)\n");  /* missing y2 */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_wrong_param_count_panda(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("panda(100,100,0,90,2,20,60)\n");  /* missing nrad */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_wrong_param_count_epanda(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("epanda(100,100,0,90,2,20,10,40)\n");  /* too few params */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_wrong_param_count_bpanda(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("bpanda(100,100,0,90,2,30,20,60)\n");  /* too few params */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_wrong_param_count_rectangle(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("rectangle(50,50,150)\n");  /* too few params */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_wrong_param_count_diamond(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("diamond(100,100,60)\n");  /* missing height */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_polygon_odd_params(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Polygon with odd number of coords - invalid */
	write_region_file("polygon(100,100,200,100,150)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_polygon_on_edge(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Square polygon - test point exactly on edge */
	write_region_file("polygon(0,0,100,0,100,100,0,100)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	/* Points exactly on edges should be included */
	fail_if(fits_in_region(50, 0, rgn) != 1);   /* on bottom edge */
	fail_if(fits_in_region(0, 50, rgn) != 1);   /* on left edge */
	fail_if(fits_in_region(100, 50, rgn) != 1); /* on right edge */
	fail_if(fits_in_region(50, 100, rgn) != 1); /* on top edge */

	fits_free_region(rgn);
}

static void
test_polygon_vertex(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	/* Triangle - test at vertices */
	write_region_file("polygon(0,0,100,0,50,100)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	/* Points at vertices should be included */
	fail_if(fits_in_region(0, 0, rgn) != 1);
	fail_if(fits_in_region(100, 0, rgn) != 1);
	fail_if(fits_in_region(50, 100, rgn) != 1);

	fits_free_region(rgn);
}

static void
test_ellipse_rotated(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("ellipse(100,100,50,30,45)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	/* Center should be in region */
	fail_if(fits_in_region(100, 100, rgn) != 1);

	fits_free_region(rgn);
}

static void
test_box_rotated(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("box(100,100,40,60,45)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	/* Center should be in region */
	fail_if(fits_in_region(100, 100, rgn) != 1);

	fits_free_region(rgn);
}

static void
test_diamond_rotated(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("diamond(100,100,60,40,45)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	/* Center should be in region */
	fail_if(fits_in_region(100, 100, rgn) != 1);

	fits_free_region(rgn);
}

static void
test_rectangle_rotated(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("rectangle(50,50,150,150,45)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	/* Center should be in region */
	fail_if(fits_in_region(100, 100, rgn) != 1);

	fits_free_region(rgn);
}

static void
test_elliptannulus_rotated(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("elliptannulus(100,100,20,10,40,20,30,45)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	/* Center should be excluded (hole) */
	fail_if(fits_in_region(100, 100, rgn) != 0);

	fits_free_region(rgn);
}

static void
test_boxannulus_rotated(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("box(100,100,20,20,40,40,30,45)\n");
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status != 0);
	fail_if(rgn == NULL);

	/* Center should be excluded (hole) */
	fail_if(fits_in_region(100, 100, rgn) != 0);

	fits_free_region(rgn);
}

static void
test_ellipse_wrong_params(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("ellipse(100,100,50)\n");  /* missing semi-minor axis */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_box_wrong_params(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("box(100,100,50)\n");  /* missing height */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_ellipse_too_many_params(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("ellipse(100,100,20,10,40,20,30,45,99)\n");  /* 9 params too many */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

static void
test_box_too_many_params(void)
{
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_file("box(100,100,20,20,40,40,30,45,99)\n");  /* 9 params too many */
	fits_read_ascii_region(region_file, NULL, &rgn, &status);
	fail_if(status == 0);  /* should fail */
}

/*
 * The WCS shared by the REGION extension below and by the callers reading
 * it back: every numeric parameter is the same on both sides, so only the
 * projection type can ever differ.  The circle sits 1000 pixels from the
 * reference pixel, far enough off-axis that a projection change moves it
 * by a readily measurable amount.
 */
#define rgn_crval1 10.0
#define rgn_crval2 20.0
#define rgn_crpix1 50.0
#define rgn_crpix2 50.0
#define rgn_cdelt1 (-0.01)
#define rgn_cdelt2 0.01
#define rgn_x 1050.0
#define rgn_y 50.0

/*
 * Write a REGION extension holding a single circle, with the region's own
 * WCS expressed as a TAN projection.  COMPONENT is an optional column, so
 * it is written or omitted on request.
 */
static void
write_region_extension(int with_component)
{
	fitsfile *f;
	int status = 0;
	char *ttype[] = {"X", "Y", "SHAPE", "R", "ROTANG", "COMPONENT"};
	char *tform[] = {"1D", "1D", "20A", "1D", "1D", "1J"};
	char *shape[] = {"CIRCLE"};
	double x = rgn_x, y = rgn_y, r = 5.0, rot = 0.0;
	int component = 1;
	int ncols = with_component ? 6 : 5;

	remove(region_fits_file);
	call_02(ffinit, &f, region_fits_file);
	call_04(ffcrim, f, BYTE_IMG, 0, NULL);
	call_08(ffcrtb, f, BINARY_TBL, 1, ncols, ttype, tform, NULL, "REGION");

	call_06(ffpcld, f, 1, 1, 1, 1, &x);
	call_06(ffpcld, f, 2, 1, 1, 1, &y);
	call_06(ffpcls, f, 3, 1, 1, 1, shape);
	call_06(ffpcld, f, 4, 1, 1, 1, &r);
	call_06(ffpcld, f, 5, 1, 1, 1, &rot);
	if (with_component)
		call_06(ffpclk, f, 6, 1, 1, 1, &component);

	call_05(ffpkyd, f, "TCRVL1", rgn_crval1, 10, NULL);
	call_05(ffpkyd, f, "TCRPX1", rgn_crpix1, 10, NULL);
	call_05(ffpkyd, f, "TCDLT1", rgn_cdelt1, 10, NULL);
	call_04(ffpkys, f, "TCTYP1", "RA---TAN", NULL);
	call_05(ffpkyd, f, "TCRVL2", rgn_crval2, 10, NULL);
	call_05(ffpkyd, f, "TCRPX2", rgn_crpix2, 10, NULL);
	call_05(ffpkyd, f, "TCDLT2", rgn_cdelt2, 10, NULL);
	call_04(ffpkys, f, "TCTYP2", "DEC--TAN", NULL);

	call_01(ffclos, f);
}

/*
 * The caller's WCS: the region's numbers, with the projection type asked
 * for.
 */
static void
caller_wcs(WCSdata *wcs, const char *type)
{
	memset(wcs, 0, sizeof(*wcs));
	wcs->exists = 1;
	wcs->xrefval = rgn_crval1;
	wcs->yrefval = rgn_crval2;
	wcs->xrefpix = rgn_crpix1;
	wcs->yrefpix = rgn_crpix2;
	wcs->xinc = rgn_cdelt1;
	wcs->yinc = rgn_cdelt2;
	wcs->rot = 0.0;
	strcpy(wcs->type, type);
}

/*
 * Where the stored circle lands once carried through world coordinates
 * from the region's TAN frame into a caller frame of the given type --
 * the same two steps region.c performs.  Deriving it rather than writing
 * a constant keeps the expectation tied to the library's own arithmetic.
 */
static void
transformed_position(char *type, double *x, double *y)
{
	int status = 0;
	double xpos, ypos;

	fail_st(ffwldp(rgn_x, rgn_y, rgn_crval1, rgn_crval2, rgn_crpix1,
		       rgn_crpix2, rgn_cdelt1, rgn_cdelt2, 0.0, "-TAN",
		       &xpos, &ypos, &status));
	fail_st(ffxypx(xpos, ypos, rgn_crval1, rgn_crval2, rgn_crpix1,
		       rgn_crpix2, rgn_cdelt1, rgn_cdelt2, 0.0, type,
		       x, y, &status));

	/* Guard the fixture: if the two frames ever stop disagreeing, the
	   tests below would pass without a transform being applied. */
	fail_if(fabs(*x - rgn_x) < 1.0);
}

/*
 * fits_read_fits_region() must convert the region's coordinates into the
 * caller's WCS when the two describe different projections.  Everything
 * but the projection type is identical here, so the type is the only
 * thing that can request the transform.
 */
static void
test_fits_region_wcs_transform(void)
{
	fitsfile *f;
	SAORegion *rgn = NULL;
	WCSdata wcs;
	int status = 0;
	double expect_x, expect_y;

	write_region_extension(1);
	caller_wcs(&wcs, "-CAR");
	transformed_position("-CAR", &expect_x, &expect_y);

	call_03(ffopen, &f, region_fits_file, READONLY);
	call_04(ffmnhd, f, BINARY_TBL, "REGION", 0);

	/* Note: fits_read_fits_region() closes the file itself. */
	call_03(fits_read_fits_region, f, &wcs, &rgn);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].shape != circle_rgn);

	fail_if(fabs(rgn->Shapes[0].param.gen.p[0] - expect_x) > 0.01);
	fail_if(fabs(rgn->Shapes[0].param.gen.p[1] - expect_y) > 0.01);

	fits_free_region(rgn);
	remove(region_fits_file);
}

/*
 * The matching case: identical projections must be read back unchanged.
 */
static void
test_fits_region_wcs_no_transform(void)
{
	fitsfile *f;
	SAORegion *rgn = NULL;
	WCSdata wcs;
	int status = 0;

	write_region_extension(1);
	caller_wcs(&wcs, "-TAN");

	call_03(ffopen, &f, region_fits_file, READONLY);
	call_04(ffmnhd, f, BINARY_TBL, "REGION", 0);

	call_03(fits_read_fits_region, f, &wcs, &rgn);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);

	fail_if(fabs(rgn->Shapes[0].param.gen.p[0] - rgn_x) > 0.01);
	fail_if(fabs(rgn->Shapes[0].param.gen.p[1] - rgn_y) > 0.01);

	fits_free_region(rgn);
	remove(region_fits_file);
}

/*
 * COMPONENT is an optional column, so a REGION extension without one must
 * read normally -- and must still be transformed into the caller's WCS.
 * The shapes then all belong to component 1.
 */
static void
test_fits_region_no_component_column(void)
{
	fitsfile *f;
	SAORegion *rgn = NULL;
	WCSdata wcs;
	int status = 0;
	double expect_x, expect_y;

	write_region_extension(0);
	caller_wcs(&wcs, "-CAR");
	transformed_position("-CAR", &expect_x, &expect_y);

	call_03(ffopen, &f, region_fits_file, READONLY);
	call_04(ffmnhd, f, BINARY_TBL, "REGION", 0);

	call_03(fits_read_fits_region, f, &wcs, &rgn);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].comp != 1);

	/* The region's own WCS must still have been read, so the
	   coordinates arrive in the caller's frame. */
	fail_if(fabs(rgn->Shapes[0].param.gen.p[0] - expect_x) > 0.01);
	fail_if(fabs(rgn->Shapes[0].param.gen.p[1] - expect_y) > 0.01);

	fits_free_region(rgn);
	remove(region_fits_file);
}

/*
 * The same extension read with no input WCS at all: nothing to transform
 * into, so the stored coordinates come back as they were written.
 */
static void
test_fits_region_no_component_no_wcs(void)
{
	fitsfile *f;
	SAORegion *rgn = NULL;
	int status = 0;

	write_region_extension(0);

	call_03(ffopen, &f, region_fits_file, READONLY);
	call_04(ffmnhd, f, BINARY_TBL, "REGION", 0);

	call_03(fits_read_fits_region, f, NULL, &rgn);
	fail_if(rgn == NULL);
	fail_if(rgn->nShapes != 1);
	fail_if(rgn->Shapes[0].comp != 1);

	fail_if(fabs(rgn->Shapes[0].param.gen.p[0] - rgn_x) > 0.01);
	fail_if(fabs(rgn->Shapes[0].param.gen.p[1] - rgn_y) > 0.01);

	fits_free_region(rgn);
	remove(region_fits_file);
}

static void
cleanup(void)
{
	remove(region_file);
	remove(region_fits_file);
}

int
main(void)
{
	/* Basic shape tests */
	test_circle_region();
	test_annulus_region();
	test_box_region();
	test_rotbox_region();
	test_ellipse_region();
	test_elliptannulus_region();
	test_rectangle_region();
	test_diamond_region();
	test_rhombus_region();
	test_sector_region();
	test_pie_region();
	test_point_region();
	test_line_region();
	test_polygon_region();
	test_panda_region();
	test_epanda_region();
	test_bpanda_region();
	test_boxannulus_region();

	/* Inclusion/exclusion tests */
	test_excluded_region();
	test_plus_sign_region();
	test_multiple_regions();
	test_excluded_polygon();
	test_multiple_excluded_regions();

	/* Format and prefix tests */
	test_comment_lines();
	test_format_pixel();
	test_image_prefix();
	test_physical_prefix();
	test_linear_prefix();
	test_global_line();
	test_image_format_line();
	test_physical_format_line();

	/* Parameter variation tests */
	test_ellipse_with_4_params();
	test_box_with_4_params();
	test_ellipse_6_params_annulus();
	test_box_6_params_annulus();
	test_rotrectangle();
	test_rotdiamond();
	test_rotrhombus();
	test_elliptannulus_7_params();
	test_boxannulus_7_params();
	test_epanda_10_params();
	test_bpanda_10_params();

	/* Edge cases */
	test_sector_wraparound();
	test_blank_lines();
	test_trailing_comment();
	test_complex_polygon();

	/* fits_in_region tests */
	test_fits_in_region_center();
	test_panda_center_excluded();
	test_bpanda_in_region();
	test_epanda_in_region();

	/* API tests */
	test_fits_read_rgnfile();
	test_fits_region_wcs_transform();
	test_fits_region_wcs_no_transform();
	test_fits_region_no_component_column();
	test_fits_region_no_component_no_wcs();
	test_fk5_format();
	test_fk4_format();
	test_icrs_format();
	test_fk5_with_circle();
	test_icrs_with_circle();
	test_j2000_format();
	test_hhmmss_coords();
	test_hhmmss_negative_dec();
	test_colon_in_param();
	test_very_long_line();

	/* Error cases */
	test_missing_file();
	test_empty_file();
	test_bad_syntax_unmatched_paren();
	test_bad_syntax_double_open();
	test_bad_syntax_double_close();
	test_unrecognized_shape();
	test_wrong_param_count_circle();
	test_wrong_param_count_annulus();
	test_wrong_param_count_polygon();
	test_format_after_shapes_error();
	test_unknown_format_error();
	test_galactic_not_supported();
	test_ecliptic_not_supported();
	test_wrong_param_count_sector();
	test_wrong_param_count_point();
	test_wrong_param_count_line();
	test_wrong_param_count_panda();
	test_wrong_param_count_epanda();
	test_wrong_param_count_bpanda();
	test_wrong_param_count_rectangle();
	test_wrong_param_count_diamond();
	test_polygon_odd_params();
	test_ellipse_wrong_params();
	test_box_wrong_params();
	test_ellipse_too_many_params();
	test_box_too_many_params();

	/* Stress tests */
	test_many_regions_realloc();

	/* Polygon edge cases */
	test_polygon_on_edge();
	test_polygon_vertex();

	/* Rotation tests */
	test_ellipse_rotated();
	test_box_rotated();
	test_diamond_rotated();
	test_rectangle_rotated();
	test_elliptannulus_rotated();
	test_boxannulus_rotated();

	cleanup();
	return 0;
}
