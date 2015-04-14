/*  
 *      $Id: cro.c,v 1.4 2009/05/08 05:50:31 fred Exp $
 */
/*
 *
 *      File:           cro.c
 *
 *      Author:         Fred Clare
 *                      National Center for Atmospheric Research
 *                      PO 3000, Boulder, Colorado
 *
 *      Date:           Thu Feb 28 18:03:09 MST 2008
 *
 *      Description:    This file contains the cairo device driver 
 *                      functions.
 */
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <cairo/cairo.h>
#include <cairo/cairo-ps.h>
#include <cairo/cairo-ft.h>

#include <math.h>
#include <stdlib.h>
#include <errno.h>
#include <ncarg/ncargC.h>
#include <ncarg/gksP.h>
#include <ncarg/c.h>
#include <ncarg/gks.h>
#include "gksc.h"
#include "gks.h"
#include "common.h"
#include "cro_device.h"
#include "croddi.h"
#include "cro.h"

#define PI 3.1415926
#define RINT(A) ((A) > 0 ? (int) ((A) + 0.5) : -(int) (0.5 - (A)))
#define PNG_SCALE 1.5

char *GetCPSFileName(int, char *);
char *GetCPNGFileName(int, int);
static void CROinit(CROddp *, int *);
unsigned int pack_argb(struct color_value);
struct color_value unpack_argb(unsigned int);
static int ccompar(const void *p1, const void *p2);
static void cascsrt(float xa[], int ip[], int n);
static float *csort_array;

extern void cro_SoftFill (GKSC *gksc, float angle, float spl);

/*
 *  A color table is built as colors are defined.  This
 *  table initially has two values when a workstation is
 *  opened.  Since cairo only has the concept of current
 *  color, a color value is selected from the table and
 *  made the current color each time a primitive is drawn.
 */
unsigned int *ctable;
int max_color=2;

/*
 *  Globals
 */
cairo_surface_t *cairo_surface;
cairo_t *cairo_context;
FILE   *fp;

/*
 *  Picture initialization.
 */
CROpict_init(GKSC *gksc) {
/*
 *  Put out background.
 */
  CROddp *psa = (CROddp *) gksc->ddp;

  struct color_value cval;
  double xl, yt, xr, yb;

  if (getenv("CRO_TRACE")) {
    printf("Got to CROpict_init\n");
  }

/*
 *  Save the current clip extents and reset the clipping rectangle to max.
 */
  cairo_clip_extents(cairo_context, &xl, &yt, &xr, &yb);
  cairo_reset_clip(cairo_context);

/*
 *  Get the background color and set the source to the background color.
 */
  cval = unpack_argb(ctable[0]);
  cairo_set_source_rgba (cairo_context, cval.red, cval.green, cval.blue, cval.alpha);
  
/*
 *  Set the clipping rectangle to the surface area.
 */
  cairo_move_to(cairo_context,0.,0.);
  cairo_line_to(cairo_context,psa->dspace.urx - psa->dspace.llx,0.);
  cairo_line_to(cairo_context,psa->dspace.urx - psa->dspace.llx,
                  psa->dspace.ury - psa->dspace.lly);
  cairo_line_to(cairo_context,0.,psa->dspace.ury - psa->dspace.lly);
  cairo_line_to(cairo_context,0.,0.);
  cairo_clip(cairo_context);

/*
 *  Fill the surface clip region with the background color.
 */
  cairo_move_to(cairo_context,0.,0.);
  cairo_line_to(cairo_context,psa->dspace.urx - psa->dspace.llx,0.);
  cairo_line_to(cairo_context,psa->dspace.urx - psa->dspace.llx,
                  psa->dspace.ury - psa->dspace.lly);
  cairo_line_to(cairo_context,0.,psa->dspace.ury - psa->dspace.lly);
  cairo_line_to(cairo_context,0.,0.);
  cairo_fill(cairo_context);

/*
 *  Restore the clipping rectangle to what it was on entry.
 *  cairo_clip clears the path.
 */
  cairo_move_to(cairo_context,xl,yt);
  cairo_line_to(cairo_context,xr,yt);
  cairo_line_to(cairo_context,xr,yb);
  cairo_line_to(cairo_context,xl,yb);
  cairo_line_to(cairo_context,xl,yt);
  cairo_clip(cairo_context);

  psa->pict_empty = FALSE;

  return(0);
}

/*
 * Set the current dash pattern depending on the line type.
 */
void CROset_dashpattern (CROddp *psa)
{
  float nominal_dash_size = 1., dash_size, dot_size, gap_size;
  double *dashes = (double *) NULL;
                
  dash_size =  6. * nominal_dash_size;
  dot_size  =  1. * nominal_dash_size;
  gap_size  =  4. * nominal_dash_size;
  switch (psa->attributes.linetype) {
    case SOLID_LINE:
      cairo_set_dash(cairo_context, dashes, 0, 0.);
      break;
    case DASHED_LINE:
      dashes = (double *) calloc(2,sizeof(double));
      *dashes = (double) dash_size;
      *(dashes+1) = (double) gap_size;
      cairo_set_dash(cairo_context, dashes, 2, 0.);
      break;
    case DOTTED_LINE:
      dashes = (double *) calloc(1,sizeof(double));
      *dashes = (double) dot_size;
      cairo_set_dash(cairo_context, dashes, 1, 0.);
      break;
    case DASH_DOT_LINE:
      dashes = (double *) calloc(4,sizeof(double));
      *dashes = (double) dash_size;
      *(dashes+1) = (double) gap_size;
      *(dashes+2) = (double) dot_size;
      *(dashes+3) = (double) gap_size;
      cairo_set_dash(cairo_context, dashes, 4, 0.);
      break;
    case DASH_DOT_DOT_LINE:
      dashes = (double *) calloc(3,sizeof(double));
      *dashes = (double) dash_size;
      *(dashes+1) = (double) gap_size;
      *(dashes+2) = (double) dot_size;
      *(dashes+3) = (double) gap_size;
      *(dashes+4) = (double) dot_size;
      *(dashes+5) = (double) gap_size;
      cairo_set_dash(cairo_context, dashes, 6, 0.);
      break;
    default:
      cairo_set_dash(cairo_context, dashes, 0, 0.);
      break;
  }
  free (dashes);
}

/*
 *  Return current clipping rectangle (in user corrdinates).
 */
CROClipRect GetCROClipping()
{
  static  CROClipRect rect;

  double x1,y1,x2,y2;
  cairo_clip_extents(cairo_context, &x1, &y1, &x2, &y2);
  rect.llx = x1;
  rect.urx = x2;
  rect.lly = y2;
  rect.ury = y1;
  return (rect);
}

int cro_ActivateWorkstation (GKSC *gksc) {

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_ActivateWorkstation\n");
  }

  return(0);
}


/*
 *  Set up indexing for cell arrays.  This interpolates between
 *  the ouput image size and the cell array.
 *
 *  On entry:
 *    image_height    : height of image in pixels
 *    image_width     : width of image in pixels
 *    *rows           : Has dimension ny
 *    *cols           : Has dimension nx
 *    nx              : number of cells per row
 *    ny              : number of cells per column
 *  On exit:
 *    *rows           : row[i] specifies number of pixels for a cell in row i
 *    *cols           : col[i] specifies number of pixels for a cell in col i
 */
static void get_cell_pixel_multiples(int image_width, int image_height, 
                          int *rows, int *cols, int nx, int ny) {
  int     i;
  int     left, right;
  double  inc, tmp;

/*
 * map cell array onto available pixels. Use current IEEE
 * rounding rules to determine whether a cell boundry includes
 * a boundry pixel or not. rows[i] and cols[j] contain
 * the number of pixels that make up cell[i,j] 
 */
  inc = (double) image_width / (double) nx;
  for( right = 0, tmp = 0.0,i = 0; i < nx; i++) { /* map cols*/
    left = right;
    tmp += inc;
    right =  (int) RINT(tmp);
    cols[i] = right - left;
  }

  inc = (double) image_height / (double) ny;
  for( right = 0, tmp = 0.0,i = 0; i < ny; i++) { /* map rows*/
    left = right;
    tmp += inc;
    right =  (int) RINT(tmp);
    rows[i] = right - left;
  }
}
get_cell_index_limits(GKSC *gksc, CROPoint P,CROPoint Q, int nx, int ny,
                   int *j_start, int *j_end, int *j_inc,
                   int *i_start, int *i_end, int *i_inc,
                   double *x_offset, double *y_offset) {

  CROddp *psa = (CROddp *) gksc->ddp;

  if ((P.x < Q.x) && (P.y < Q.y)){         /* P lower left, Q upper right. */
     *x_offset = (double) (P.x * psa->dspace.xspan);
     *y_offset = (double) (P.y * psa->dspace.yspan);
     *j_start = 0;
     *j_end = ny;
     *j_inc = 1;
     *i_start = 0;
     *i_end = nx;
     *i_inc = 1;
  }
  else if ((P.x < Q.x) && (P.y > Q.y)) {   /* P upper left, Q lower right. */
     *x_offset = (double) (P.x * psa->dspace.xspan);
     *y_offset = (double) (Q.y * psa->dspace.yspan);
     *j_start = ny-1;
     *j_end = -1;
     *j_inc = -1;
     *i_start = 0;
     *i_end = nx;
     *i_inc = 1;
  }
  else if ((Q.x < P.x) && (Q.y > P.y)) {   /* Q upper left, P lower right. */
     *x_offset = (double) (Q.x * psa->dspace.xspan);
     *y_offset = (double) (P.y * psa->dspace.yspan);
     *j_start = 0;
     *j_end = ny;
     *j_inc = 1;
     *i_start = nx-1;
     *i_end = -1;
     *i_inc = -1;
  }
  else if ((Q.x < P.x) && (Q.y < P.y)) {   /* Q lower left, P upper right. */
     *x_offset = (double) (Q.x * psa->dspace.xspan);
     *y_offset = (double) (Q.y * psa->dspace.yspan);
     *j_start = ny-1;
     *j_end = -1;
     *j_inc = -1;
     *i_start = nx-1;
     *i_end = -1;
     *i_inc = -1;
  }

}

int cro_Cellarray(GKSC *gksc) {


  int nx,ny;
  int i,j,k,l,image_width, image_height, kount, stat;
  int j_start,j_end,j_inc,i_start,i_end,i_inc;
  int *rows, *cols;
  unsigned int *iar;
  unsigned char *data;
  cairo_surface_t *cell_image;

  CROPoint P,Q,R;

  CROddp *psa = (CROddp *) gksc->ddp;
  CROPoint *corners = (CROPoint *) gksc->p.list;


  int *iptr = (int *) gksc->i.list;
  int *colia = (int *) gksc->x.list;

  double x_offset, y_offset;

  Gfloat tred, tgreen, tblue, talpha;
  struct color_value cval;
  cairo_pattern_t *pattern;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_Cellarray\n");
  }

/*
 *  Save current color.
 */
  pattern = cairo_get_source(cairo_context);
  if (cairo_pattern_get_rgba(pattern,&tred,&tgreen,&tblue,&talpha) !=        
           CAIRO_STATUS_SUCCESS) {
    printf("cro_Text: can only retrieve current color for solid patterns\n");
    return(1);
  }
  cval = unpack_argb(ctable[psa->attributes.text_colr_ind]);

  if (psa->pict_empty) {
    CROpict_init(gksc);
  }

  nx = iptr[0];
  ny = iptr[1];
  P = corners[0];
  Q = corners[1];
  R = corners[2];

  image_width = RINT(fabs(Q.x - P.x)*(psa->dspace.xspan));
  image_height = RINT(fabs(Q.y - P.y)*(psa->dspace.yspan));
  rows = (int *) calloc (ny,sizeof(int));
  cols = (int *) calloc (nx,sizeof(int));

  iar = (unsigned int *) calloc(image_width*image_height,sizeof(unsigned int));
  get_cell_pixel_multiples(image_width, image_height, rows, cols, nx, ny);
  get_cell_index_limits(gksc,P,Q,nx,ny, &j_start,&j_end,&j_inc,
                                   &i_start,&i_end,&i_inc,
                                   &x_offset,&y_offset);
  kount = 0;
  for (j = j_start; j != j_end; j=j+j_inc) {
    for (l = 0; l < rows[j]; l++) {
      for (i = i_start; i != i_end; i=i+i_inc) {
        for (k = 0; k < cols[i]; k++) {
          iar[kount] = ctable[colia[j*nx+i]];
          kount++;
        }
      }
    }
  }
  data = (unsigned char *)iar;

  cell_image = cairo_image_surface_create_for_data (data,
                 CAIRO_FORMAT_ARGB32,image_width,image_height,
                 4*image_width);
  stat = cairo_surface_status(cell_image);
 
  cairo_set_source_surface (cairo_context, cell_image, x_offset, y_offset);
  cairo_surface_destroy(cell_image);
  cairo_paint(cairo_context);

/*
 *  Restore color.
 */
  cairo_set_source_rgba(cairo_context,
                          cval.red, cval.green, cval.blue, cval.alpha);
  return(0);
} 
int cro_ClearWorkstation(GKSC *gksc) {

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_ClearWorkstation\n");
  }

  CROddp  *psa;
  psa = (CROddp *) gksc->ddp;

  cairo_stroke(cairo_context);
  cairo_show_page(cairo_context);
  
  if (psa->wks_type == CPS) {
    (void) fflush(psa->file_pointer);
  }
  else if (psa->wks_type == CPNG) {
    cairo_surface_write_to_png (cairo_surface, 
       GetCPNGFileName(psa->wks_id, psa->frame_count));
    psa->frame_count++;
  }

  psa->pict_empty = TRUE;

  return(0);
}

int cro_CloseWorkstation(GKSC *gksc) {
  CROddp   *psa = (CROddp *) gksc->ddp;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_CloseWorkstation\n");
  }

  psa->pict_empty = TRUE;
  if (psa->wks_type == CPS) {
    fclose(fp);
  }

  cairo_destroy(cairo_context);

  return(0);
}
int cro_DeactivateWorkstation(GKSC *gksc) {

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_DeactivateWorkstation\n");
  }

  return(0);
}
int cro_Esc(GKSC *gksc) {

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_Esc\n");
  }

  return(0);
}
int cro_FillArea(GKSC *gksc) {
  CROPoint *pptr = (CROPoint *) gksc->p.list;
  CROddp   *psa = (CROddp *) gksc->ddp;
  int      npoints = gksc->p.num, i;
  
  float  clwidth;
  struct color_value cval;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_FillArea\n");
  }

  clwidth = (float) cairo_get_line_width(cairo_context);


  if (psa->pict_empty) {
    CROpict_init(gksc);
  }

  cairo_set_line_width(cairo_context,1.1*(psa->dspace.yspan)*(psa->sfill_spacing));

  cval = unpack_argb(ctable[psa->attributes.fill_colr_ind]);
  cairo_set_source_rgba(cairo_context,
                          cval.red, cval.green, cval.blue, cval.alpha);

  switch(psa->attributes.fill_int_style) {
    case HOLLOW_FILL:   /* Put out polyline */
      cairo_move_to(cairo_context, pptr[0].x * (float) psa->dspace.xspan,
                                   pptr[0].y * (float) psa->dspace.yspan);
      for (i = 1; i < npoints; i++) {
        cairo_line_to(cairo_context, pptr[i].x * (float) psa->dspace.xspan,
                                     pptr[i].y * (float) psa->dspace.yspan);
      }
      cairo_line_to(cairo_context, pptr[0].x * (float) psa->dspace.xspan,
                                   pptr[0].y * (float) psa->dspace.yspan);
      cairo_stroke(cairo_context);
      break;
    case SOLID_FILL:
      cairo_move_to(cairo_context, pptr[0].x * (float) psa->dspace.xspan,
                                   pptr[0].y * (float) psa->dspace.yspan);
      for (i = 1; i < npoints; i++) {
        cairo_line_to(cairo_context, pptr[i].x * (float) psa->dspace.xspan,
                                     pptr[i].y * (float) psa->dspace.yspan);
      }
      cairo_line_to(cairo_context, pptr[0].x * (float) psa->dspace.xspan,
                                   pptr[0].y * (float) psa->dspace.yspan);
      cairo_fill(cairo_context);
      break;
    case PATTERN_FILL:  /* currently not implemented, issue polyline */
      cairo_move_to(cairo_context, pptr[0].x * (float) psa->dspace.xspan,
                                   pptr[0].y * (float) psa->dspace.yspan);
      for (i = 1; i < npoints; i++) {
        cairo_line_to(cairo_context, pptr[i].x * (float) psa->dspace.xspan,
                                     pptr[i].y * (float) psa->dspace.yspan);
      }
      cairo_stroke(cairo_context);
      break;
    case HATCH_FILL:
      switch (psa->attributes.fill_style_ind) {
        case HORIZONTAL_HATCH:
          cro_SoftFill (gksc, 0., psa->hatch_spacing);  
          cairo_stroke(cairo_context);
          break;
        case VERTICAL_HATCH:
          cro_SoftFill (gksc, 90., psa->hatch_spacing);  
          cairo_stroke(cairo_context);
          break;
        case POSITIVE_HATCH:
          cro_SoftFill (gksc, 45., psa->hatch_spacing);  
          cairo_stroke(cairo_context);
          break;
        case NEGATIVE_HATCH:
          cro_SoftFill (gksc, 135., psa->hatch_spacing);  
          cairo_stroke(cairo_context);
          break;
        case HORIZ_VERT_HATCH:
          cro_SoftFill (gksc, 0., psa->hatch_spacing);  
          cro_SoftFill (gksc, 90., psa->hatch_spacing);  
          cairo_stroke(cairo_context);
          break;
        case POS_NEG_HATCH:
          cro_SoftFill (gksc, 45., psa->hatch_spacing);  
          cro_SoftFill (gksc, 135., psa->hatch_spacing);  
          cairo_stroke(cairo_context);
          break;
        default:
          cairo_move_to(cairo_context, pptr[0].x * (float) psa->dspace.xspan,
                                       pptr[0].y * (float) psa->dspace.yspan);
          for (i = 1; i < npoints; i++) {
            cairo_line_to(cairo_context, pptr[i].x * (float) psa->dspace.xspan,
                                         pptr[i].y * (float) psa->dspace.yspan);
          }
          cairo_line_to(cairo_context, pptr[0].x * (float) psa->dspace.xspan,
                                       pptr[0].y * (float) psa->dspace.yspan);
          cairo_stroke(cairo_context);
          break;
      }
      break;
    default:
      cairo_move_to(cairo_context, pptr[0].x * (float) psa->dspace.xspan,
                                   pptr[0].y * (float) psa->dspace.yspan);
      for (i = 1; i < npoints; i++) {
        cairo_line_to(cairo_context, pptr[i].x * (float) psa->dspace.xspan,
                                     pptr[i].y * (float) psa->dspace.yspan);
      }
      cairo_line_to(cairo_context, pptr[0].x * (float) psa->dspace.xspan,
                                   pptr[0].y * (float) psa->dspace.yspan);
      cairo_stroke(cairo_context);
      break;
  }
  cairo_set_line_width(cairo_context,clwidth);
  return(0);
}
int cro_GetColorRepresentation(GKSC *gksc) {
  CROddp   *psa = (CROddp *) gksc->ddp;

  int      *xptr   = (int *) gksc->x.list;
  CROColor *rgbptr = (CROColor *) gksc->rgb.list;
  struct   color_value cval;


  int     index   = xptr[0];

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_GetColorRepresentation\n");
  }

  cval = unpack_argb(ctable[index]);

  rgbptr[0].r = cval.red;
  rgbptr[0].g = cval.green;
  rgbptr[0].b = cval.blue;

  return(0);
}

int cro_OpenWorkstation(GKSC *gksc) {

  char   *sptr = (char *) gksc->s.list;
  CROddp *psa;
  char *ctmp;
  int    *pint;
  extern int orig_wks_id;
  static  CROClipRect rect;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_OpenWorkstation\n");
  }


/*
 *  Provide the gksc with the device dependent data.
 */
  psa = (CROddp *) malloc (sizeof (CROddp));
  if (psa == (CROddp *) NULL) {
     ESprintf(ERR_CRO_MEMORY, "CRO: malloc(%d)", sizeof(CROddp));
     return(ERR_CRO_MEMORY);
  }
  gksc->ddp = (GKSC_Ptr) psa;

  pint = (int *)(gksc->i.list);
  psa->wks_type = *(pint+1);
  psa->wks_id = orig_wks_id;

  
/*
 *  Initialize all transformations as well as the device coordinate
 *  space (store these in the device dependent data).
 */
  TransformSetWindow(&(psa->tsystem), 0.0, 0.0, 1.0, 1.0);
  TransformSetViewport(&(psa->tsystem),0.0, 0.0, 1.0, 1.0);
  TransformSetNDScreenSpace(&(psa->tsystem),0.0, 0.0, 1.0, 1.0);
  TransformSetScreenSpace(&(psa->tsystem),0.0, 0.0, 1.0, 1.0);
  psa->transform = TransformGetTransform(&psa->tsystem);
  

  CROinit(psa, pint+2);       /* Initialize local data. */

/*
 *  Create the Postscript workstation.
 */
  if (psa->wks_type == CPS) {
    psa->output_file = GetCPSFileName(psa->wks_id, sptr);
    if (strncmp(psa->output_file, "stdout", 6) == 0) {
      fp = stdout;
    }
    else {
      fp = fopen(psa->output_file,"w");
    }
    if (fp == (FILE *) NULL) {
      ctmp = (char *) calloc(strlen(psa->output_file)+3, 1);
      strcat(ctmp,"\"");
      strcat(ctmp+1,psa->output_file);
      strcat(ctmp+1+strlen(psa->output_file),"\"");
      ESprintf(ERR_OPN_CRO, "CRO: fopen(%s, \"w\")", ctmp);
      free(ctmp);
      return(ERR_OPN_CRO);
    }
    psa->file_pointer = fp;
    cairo_surface = cairo_ps_surface_create (psa->output_file, 612, 792);
  } 
/*
 *  Create the PNG workstation.
 */
  else if (psa->wks_type == CPNG) {
/*
 *  Ultimately should provide a user-settable scale factor instead
 *  of using PNG_SCALE  for width and height here.  The numbers below are
 *  the default PS page limits of (612,792) scaled by PNG_SCALE.  The
 *  translation offsets and scale factor for PNG output below also
 *  reflect this scale factor (default offsets for PS are (36,666)).
 *
 */
    cairo_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
                       PNG_SCALE*612,PNG_SCALE*792);
  }
  cairo_context = cairo_create (cairo_surface);

/*
 *  Set fill rule to even/odd.
 */
  cairo_set_fill_rule(cairo_context,CAIRO_FILL_RULE_EVEN_ODD);

/*
 *  Set default line cap and line join to round.
 */
  cairo_set_line_cap(cairo_context, CAIRO_LINE_CAP_ROUND);
  cairo_set_line_join(cairo_context, CAIRO_LINE_JOIN_ROUND);
  cairo_surface_destroy(cairo_surface);

/*
 *  Set the default linewidth.
 */
  cairo_set_line_width(cairo_context, psa->attributes.linewidth);

/*
 *  Set clipping rectangle to max.
 */
  cairo_new_path(cairo_context);
  cairo_move_to(cairo_context, psa->dspace.llx, psa->dspace.lly);
  cairo_line_to(cairo_context, psa->dspace.llx + psa->dspace.xspan, 
                               psa->dspace.lly);
  cairo_line_to(cairo_context, psa->dspace.llx + psa->dspace.xspan, 
                               psa->dspace.lly + psa->dspace.yspan);
  cairo_line_to(cairo_context, psa->dspace.llx, 
                               psa->dspace.lly + psa->dspace.yspan);
  cairo_line_to(cairo_context, psa->dspace.llx, psa->dspace.lly);
  cairo_clip(cairo_context);


/*
 *  Translate and flip (since GKS origin is at bottom left) to center the plot.
 *  Since the final plot is to be flipped top to bottom, we want
 *  to translate in the Y direction to the original top.
 */
  if (psa->wks_type == CPS) {
    cairo_translate(cairo_context,psa->dspace.llx,psa->dspace.ury);
    cairo_scale(cairo_context,1.,-1.);
  }
  else if (psa->wks_type == CPNG) {
    cairo_translate(cairo_context,PNG_SCALE*psa->dspace.llx,
                                  PNG_SCALE*psa->dspace.ury);
    cairo_scale(cairo_context,PNG_SCALE,-PNG_SCALE);
  }

  rect = GetCROClipping();

/*
 *  Initialize color table.
 */
  ctable = (unsigned int *) calloc(max_color,sizeof(unsigned int));

/*
 *  Define the default foreground (black) and background (white)
 *  colors and draw the background.
 */
  ctable[0] = 0xFFFFFFFF;
  ctable[1] = 0xFF000000;
  cairo_set_source_rgba (cairo_context, 0., 0., 0., 1.);

  return(0);
}

int  cro_Polyline(GKSC *gksc) {
  CROPoint *pptr = (CROPoint *) gksc->p.list;
  CROddp   *psa = (CROddp *) gksc->ddp;

  int npoints = gksc->p.num, i, ier, ltype;
  struct color_value cval;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_Polyline\n");
  }

  if (psa->pict_empty) {
    CROpict_init(gksc);
  }

/*
 *  Set the dash pattern based on the line type.
 */

  cval = unpack_argb(ctable[psa->attributes.line_colr_ind]);
  cairo_set_source_rgba(cairo_context,
                          cval.red, cval.green, cval.blue, cval.alpha);

  cairo_set_line_width(cairo_context,(psa->nominal_width_scale) * (psa->attributes.linewidth));
  cairo_new_sub_path(cairo_context);

/*
 *  Use butt ends if not solid line.
 */
  ginq_linetype(&ier,&ltype);
  if (ltype != 1) {
    cairo_set_line_cap(cairo_context,CAIRO_LINE_CAP_BUTT);
  }
  else {
    cairo_set_line_cap(cairo_context,CAIRO_LINE_CAP_ROUND);
  }
  CROset_dashpattern (psa);

  cairo_move_to(cairo_context, pptr[0].x * (float) psa->dspace.xspan,
                               pptr[0].y * (float) psa->dspace.yspan);
  for (i = 1; i < npoints; i++) {
    cairo_line_to(cairo_context, pptr[i].x * (float) psa->dspace.xspan,
                                 pptr[i].y * (float) psa->dspace.yspan);
  }
  cairo_stroke(cairo_context);

/*
 *  Set line cap back to round in case it was changed.
 */
  cairo_set_line_cap(cairo_context,CAIRO_LINE_CAP_ROUND);

  return (0);
}

int cro_Polymarker(GKSC *gksc) {
  CROPoint *pptr = (CROPoint *) gksc->p.list;
  CROddp   *psa = (CROddp *) gksc->ddp;
  int      npoints = gksc->p.num, i;
  double red, green, blue, alpha;

  int marker_type, current_line_color;
  float marker_size, orig_line_width, xc, yc, mscale;
  struct color_value cval;
  cairo_line_cap_t orig_cap_type;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_Polymarker\n");
  }

  if (psa->pict_empty) {
    CROpict_init(gksc);
  }

  marker_type = psa->attributes.marker_type;
  marker_size = ((psa->transform.y_scale) * (psa->attributes.marker_size));

/*
 *  Retrieve current line color index so we can reset it
 *  at the end, since we will be using Polylines for the 
 *  markers.
 */
  current_line_color = psa->attributes.line_colr_ind;
  cval = unpack_argb(ctable[psa->attributes.marker_colr_ind]);
  cairo_set_source_rgba(cairo_context,
                            cval.red, cval.green, cval.blue, cval.alpha);

/*
 *  Get the current setting for line cap and set it to round so that
 *  a dot will be drawn in the degenerate case.  Do the same for
 *  linewidth.
 */
  orig_cap_type = cairo_get_line_cap(cairo_context);
  cairo_set_line_cap(cairo_context, CAIRO_LINE_CAP_ROUND);
  orig_line_width = cairo_get_line_width(cairo_context);
  cairo_set_line_width(cairo_context, 1.0);

  switch (marker_type) {
  case    DOT_MARKER: 
    
/*
 *  Dot markers cannot be scaled.
 */
    cairo_set_line_cap(cairo_context,CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(cairo_context, 0.5);
    for (i = 0; i < npoints; i++) {
      cairo_move_to(cairo_context, pptr[i].x * (float) psa->dspace.xspan,
                                   pptr[i].y * (float) psa->dspace.yspan);
      cairo_line_to(cairo_context, pptr[i].x * (float) psa->dspace.xspan,
                                   pptr[i].y * (float) psa->dspace.yspan);
      cairo_stroke(cairo_context);
    }
    cairo_set_line_width(cairo_context, orig_line_width);
    break;
  case    PLUS_MARKER:
    for (i = 0; i < npoints; i++) {
      xc = pptr[i].x * (float) psa->dspace.xspan;
      yc = pptr[i].y * (float) psa->dspace.yspan;
      mscale = 2.75;
      cairo_move_to(cairo_context, xc, yc-mscale*marker_size);
      cairo_line_to(cairo_context, xc, yc+mscale*marker_size);

      cairo_stroke(cairo_context);
      cairo_move_to(cairo_context, xc-mscale*marker_size, yc);
      cairo_line_to(cairo_context, xc+mscale*marker_size, yc);
      cairo_stroke(cairo_context);
    }
    break;
  case    STAR_MARKER:
    for (i = 0; i < npoints; i++) {
      xc = pptr[i].x * (float) psa->dspace.xspan;
      yc = pptr[i].y * (float) psa->dspace.yspan;
      mscale = 2.75*marker_size;

      cairo_move_to(cairo_context, xc, yc-mscale);
      cairo_line_to(cairo_context, xc, yc+mscale);
      cairo_stroke(cairo_context);

      cairo_move_to(cairo_context, xc-0.866*mscale, yc-0.5*mscale);
      cairo_line_to(cairo_context, xc+0.866*mscale, yc+0.5*mscale);
      cairo_stroke(cairo_context);

      cairo_move_to(cairo_context, xc-0.866*mscale, yc+0.5*mscale);
      cairo_line_to(cairo_context, xc+0.866*mscale, yc-0.5*mscale);
      cairo_stroke(cairo_context);

    }
    break;
  case    CIRCLE_MARKER:
    for (i = 0; i < npoints; i++) {
      xc = pptr[i].x * (float) psa->dspace.xspan;
      yc = pptr[i].y * (float) psa->dspace.yspan;
      mscale = 2.75;
      cairo_move_to(cairo_context, xc, yc);
      cairo_new_sub_path(cairo_context);
      cairo_arc(cairo_context, xc, yc, mscale*marker_size, 0., 2.*M_PI);
      cairo_stroke(cairo_context);
    }
    break;
  case    X_MARKER:
    for (i = 0; i < npoints; i++) {
      xc = pptr[i].x * (float) psa->dspace.xspan;
      yc = pptr[i].y * (float) psa->dspace.yspan;
      mscale = 3.*marker_size;

      cairo_move_to(cairo_context, xc-0.707*mscale, yc-0.707*mscale);
      cairo_line_to(cairo_context, xc+0.707*mscale, yc+0.707*mscale);
      cairo_stroke(cairo_context);

      cairo_move_to(cairo_context, xc-0.707*mscale, yc+0.707*mscale);
      cairo_line_to(cairo_context, xc+0.707*mscale, yc-0.707*mscale);
      cairo_stroke(cairo_context);

    }
    break;
  default:
    for (i = 0; i < npoints; i++) {
      xc = pptr[i].x * (float) psa->dspace.xspan;
      yc = pptr[i].y * (float) psa->dspace.yspan;
      mscale = 2.75*marker_size;

      cairo_move_to(cairo_context, xc, yc-mscale);
      cairo_line_to(cairo_context, xc, yc+mscale);
      cairo_stroke(cairo_context);

      cairo_move_to(cairo_context, xc-0.866*mscale, yc-0.5*mscale);
      cairo_line_to(cairo_context, xc+0.866*mscale, yc+0.5*mscale);
      cairo_stroke(cairo_context);

      cairo_move_to(cairo_context, xc-0.866*mscale, yc+0.5*mscale);
      cairo_line_to(cairo_context, xc+0.866*mscale, yc-0.5*mscale);
      cairo_stroke(cairo_context);

    }
    break;
  }
/*
 *  Restore line cap type and line width.
 */
  cairo_set_line_cap(cairo_context, orig_cap_type);
  cairo_set_line_width(cairo_context, orig_line_width);
  return(0);
}
int cro_SetCharacterExpansionFactor(GKSC *gksc) {


  CROddp   *psa  = (CROddp *) gksc->ddp;
  float    *fptr = (float *) gksc->f.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetCharacterExpansionFactor\n");
  }

  psa->attributes.char_expan = fptr[0];
  return(0);
}
int cro_SetCharacterHeightAndUpVector(GKSC *gksc) {


  CROddp   *psa = (CROddp *) gksc->ddp;

  float   *fptr = (float *) gksc->f.list;

  double  up_x   = (double) fptr[0];
  double  up_y   = (double) fptr[2];
  double  base_x = (double) fptr[1];
  double  base_y = (double) fptr[3];

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetCharacterHeightAndUpVector\n");
  }

/*
 *  Transform to workstation viewport space.
 */
  up_x   *= (psa->transform).x_scale;
  up_y   *= (psa->transform).y_scale;
  base_x *= (psa->transform).x_scale;
  base_y *= (psa->transform).y_scale;
  psa->attributes.char_up_vec_x   = (float) up_x;
  psa->attributes.char_up_vec_y   = (float) up_y;
  psa->attributes.char_base_vec_x = (float) base_x;
  psa->attributes.char_base_vec_y = (float) base_y;
  psa->attributes.char_ht = MAG(up_x,up_y);

  return(0);
}
int cro_SetCharacterSpacing(GKSC *gksc) {


  CROddp *psa  = (CROddp *) gksc->ddp;
  float  *fptr = (float *) gksc->f.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetCharacterSpacing\n");
  }

  psa->attributes.char_space = fptr[0];
  return(0);
}

int cro_SetClipIndicator(GKSC *gksc) {
  CROClipRect rect;


  CROddp   *psa = (CROddp *) gksc->ddp;
  int      *iptr = (int *) gksc->i.list;
  CROPoint *pptr = (CROPoint *) gksc->p.list;


  int      clip_flag;
  float    pllx, purx, plly, pury;
  float    tllx, turx, tlly, tury;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetClipIndicator\n");
  }

  clip_flag = iptr[0];
  
  pllx      = pptr[0].x;
  plly      = pptr[0].y;
  purx      = pptr[1].x;
  pury      = pptr[1].y;

  psa->attributes.norm_tran = iptr[1];   /* GKS normalization number */

  if (clip_flag) {
/*
 *  Turn clipping on.
 */
    cairo_new_path(cairo_context);
/*
 *  Set the clip rectangle.
 */
    cairo_reset_clip(cairo_context);
    cairo_move_to(cairo_context,pllx*psa->dspace.xspan,plly*psa->dspace.yspan);
    cairo_line_to(cairo_context,purx*psa->dspace.xspan,plly*psa->dspace.yspan);
    cairo_line_to(cairo_context,purx*psa->dspace.xspan,pury*psa->dspace.yspan);
    cairo_line_to(cairo_context,pllx*psa->dspace.xspan,pury*psa->dspace.yspan);
    cairo_line_to(cairo_context,pllx*psa->dspace.xspan,plly*psa->dspace.yspan);
    cairo_clip(cairo_context);

    psa->attributes.clip_ind = 1;
  }
  else {
    psa->attributes.clip_ind = 0;
    cairo_reset_clip(cairo_context);
    cairo_new_path(cairo_context);
    cairo_move_to(cairo_context,0,0);
    cairo_line_to(cairo_context,psa->dspace.xspan,0);
    cairo_line_to(cairo_context,psa->dspace.xspan,psa->dspace.yspan);
    cairo_line_to(cairo_context,0,psa->dspace.yspan);
    cairo_line_to(cairo_context,0,0);
    cairo_clip(cairo_context);
  }
  return(0);
}

int cro_SetColorRepresentation(GKSC *gksc) {
  int      *xptr   = (int *) gksc->x.list;
  CROColor *rgbptr = (CROColor *) gksc->rgb.list;
  CROddp   *psa = (CROddp *) gksc->ddp;

  unsigned index = (unsigned) xptr[0];
  double xl, yt, xr, yb;

  struct color_value cval;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetColorRepresentation\n");
  }

  if (max_color < index+1) {
    max_color = index+1;
    ctable = (unsigned int*) realloc(ctable,max_color*sizeof(unsigned int));
  }
  cval.alpha = 1.;
  cval.red   = (float) rgbptr[0].r;
  cval.green = (float) rgbptr[0].g;
  cval.blue  = (float) rgbptr[0].b;

  ctable[index] = pack_argb(cval);
/*
 *  Setting index 0 specifies the background color, so fill the
 *  plotting area with that color.
 */
  if (index == 0) {
/*
 *  Save the current clip extents and reset the clipping rectangle to max.
 */
    cairo_clip_extents(cairo_context, &xl, &yt, &xr, &yb);
    cairo_reset_clip(cairo_context);

/*
 *  Set the source to the background color.
 */
    cairo_set_source_rgba (cairo_context, cval.red, cval.green, cval.blue, cval.alpha);
  
/*
 *  Set the clipping rectangle to the surface area.
 */
    cairo_move_to(cairo_context,0.,0.);
    cairo_line_to(cairo_context,psa->dspace.urx - psa->dspace.llx,0.);
    cairo_line_to(cairo_context,psa->dspace.urx - psa->dspace.llx,
                    psa->dspace.ury - psa->dspace.lly);
    cairo_line_to(cairo_context,0.,psa->dspace.ury - psa->dspace.lly);
    cairo_line_to(cairo_context,0.,0.);
    cairo_clip(cairo_context);
  
/*
 *  Fill the surface clip region with the background color.
 */
    cairo_move_to(cairo_context,0.,0.);
    cairo_line_to(cairo_context,psa->dspace.urx - psa->dspace.llx,0.);
    cairo_line_to(cairo_context,psa->dspace.urx - psa->dspace.llx,
                    psa->dspace.ury - psa->dspace.lly);
    cairo_line_to(cairo_context,0.,psa->dspace.ury - psa->dspace.lly);
    cairo_line_to(cairo_context,0.,0.);
    cairo_fill(cairo_context);

/*
 *  Restore the clipping rectangle to what it was on entry.
 *  cairo_clip clears the path.
 */
    cairo_move_to(cairo_context,xl,yt);
    cairo_line_to(cairo_context,xr,yt);
    cairo_line_to(cairo_context,xr,yb);
    cairo_line_to(cairo_context,xl,yb);
    cairo_line_to(cairo_context,xl,yt);
    cairo_clip(cairo_context);
  }

  return(0);
}
int cro_SetFillAreaColorIndex(GKSC *gksc) {


  CROddp   *psa = (CROddp *) gksc->ddp;

  int *xptr = (int *) gksc->x.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetFillAreaColorIndex\n");
  }
  psa->attributes.fill_colr_ind = (unsigned int) xptr[0];
  return(0);
}
int cro_SetFillAreaInteriorStyle(GKSC *gksc) {
  CROddp *psa = (CROddp *) gksc->ddp;
  int    *iptr = (int *) gksc->i.list;


  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetFillAreaInteriorStyle\n");
  }

  psa->attributes.fill_int_style = iptr[0];

  return(0);
}
int cro_SetFillAreaStyleIndex(GKSC *gksc) {
  CROddp *psa = (CROddp *) gksc->ddp;
  
  int    *iptr = (int *) gksc->i.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetFillAreaStyleIndex\n");
  }

  psa->attributes.fill_style_ind = iptr[0];
  return(0);
}
int cro_SetLineWidthScaleFactor(GKSC *gksc) {


  CROddp *psa = (CROddp *) gksc->ddp; 
  float  *fptr = (float *) gksc->f.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetLineWidthScaleFactor\n");
  }

  if (fptr[0] <= 0.2) {
    psa->attributes.linewidth = 0.2;
  }
  else {
    psa->attributes.linewidth = fptr[0];
  }
  return(0);
}
int cro_SetLinetype(GKSC *gksc) {

  CROddp *psa = (CROddp *) gksc->ddp;

  int   *iptr = (int *) gksc->i.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetLinetype\n");
  }

  psa->attributes.linetype = iptr[0];
  return(0);
}

int cro_SetMarkerSizeScaleFactor(GKSC *gksc) {


  CROddp   *psa = (CROddp *) gksc->ddp;
  float *fptr = (float *) gksc->f.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_setMarkerSizeScaleFactor\n");
  }

  psa->attributes.marker_size = fptr[0];
  return(0);
}

int cro_SetMarkerType(GKSC *gksc) {


  CROddp   *psa = (CROddp *) gksc->ddp;

  int *iptr = (int *) gksc->i.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetMarkerType\n");
  }

  psa->attributes.marker_type = iptr[0];
  return(0);
}

int cro_SetPolylineColorIndex(GKSC *gksc)
{
  CROddp   *psa = (CROddp *) gksc->ddp;
  int *xptr = (int *) gksc->x.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetPolylineColorIndex\n");
  }
  psa->attributes.line_colr_ind = (unsigned int) xptr[0];
  return(0);
}

int cro_SetPolymarkerColorIndex(GKSC *gksc) {


  CROddp   *psa = (CROddp *) gksc->ddp;
  int *xptr = (int *) gksc->x.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetPolymarkerColorIndex\n");
  }

  psa->attributes.marker_colr_ind = (unsigned int) xptr[0];
  return(0);
}
int cro_SetTextAlignment(GKSC *gksc) {
  CROddp   *psa = (CROddp *) gksc->ddp;
  int *xptr = (int *) gksc->i.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetTextAlignment\n");
  }

  psa->attributes.text_align_horiz = (unsigned int) xptr[0];
  psa->attributes.text_align_vert  = (unsigned int) xptr[1];
  return(0);
}
int cro_SetTextColorIndex(GKSC *gksc) {


  CROddp   *psa = (CROddp *) gksc->ddp;
  int *xptr = (int *) gksc->x.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetTextColorIndex\n");
  }

  psa->attributes.text_colr_ind = (unsigned int) xptr[0];
  return(0);
}
int cro_SetTextFontAndPrecision(GKSC *gksc) {


  CROddp   *psa = (CROddp *) gksc->ddp;
  static int ifst=0;


/*
 *  Extract only the font number since we will use only the
 *  highest precision.
 */
  int *iptr = (int *) gksc->i.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetTextFontAndPrecision\n");
  }
  
  if (ifst == 0) {
    psa->attributes.text_font = 0;
    ifst++;
  }
  else {
    psa->attributes.text_font = abs(iptr[0]);
  }

  return(0);
}
int cro_SetTextPath(GKSC *gksc) {

  CROddp *psa = (CROddp *) gksc->ddp;
  int   *iptr = (int *) gksc->i.list;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetTextPath\n");
  }

  psa->attributes.text_path = iptr[0];
  return(0);
}
int cro_SetViewport(GKSC *gksc) {


  CROddp   *psa = (CROddp *) gksc->ddp;
  float   *fptr = (float *) gksc->f.list;

  CROClipRect *Crect;
  int         rec_chg = 0;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetViewport\n");
  }

/*
 *  If the workstation viewport has changed, update the
 *  transformation.
 */
  if ((psa->tsystem.viewport.llx != fptr[0]) ||
      (psa->tsystem.viewport.urx != fptr[1]) ||
      (psa->tsystem.viewport.lly != fptr[2]) ||
      (psa->tsystem.viewport.ury != fptr[3])) {
    TransformSetViewport(&psa->tsystem, fptr[0], fptr[2], fptr[1], fptr[3]);
    psa->transform = TransformGetTransform(&psa->tsystem);
  }
  return(0);
}
int cro_SetWindow(GKSC *gksc) {


  CROddp *psa = (CROddp *) gksc->ddp;
  float  *fptr = (float *) gksc->f.list;

  CROClipRect *Crect;
  int         rec_chg = 0;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_SetWindow\n");
  }

  if ((fptr[0] >= fptr[1]) || (fptr[2] >= fptr[3])) {
    return (ERR_INV_RECT);
  }

/*
 *  If the workstation window has changed, update the
 *  transformation.
 */
  if ((psa->tsystem.window.llx != fptr[0]) ||
      (psa->tsystem.window.urx != fptr[1]) ||
      (psa->tsystem.window.lly != fptr[2]) ||
      (psa->tsystem.window.ury != fptr[3])) {
    TransformSetWindow(&psa->tsystem, fptr[0], fptr[2], fptr[1], fptr[3]);
    psa->transform = TransformGetTransform(&psa->tsystem);
  }
  return(0);
}

int cro_Text(GKSC *gksc) {
/*
 *  Supports the GKS GTX functions.
 */
  CROPoint *pptr = (CROPoint *) gksc->p.list;
  CROddp   *psa = (CROddp *) gksc->ddp;
  char     *sptr = (char *) gksc->s.list, *font_path, *font_name, *db_path;
  char     single_char[2];
  struct   color_value cval;
  float    base_mag, tcos, cprod, cang;
  int      slen, error;
  double   *dashes, horiz_len, left_space, right_space;

  float    xc, yc, xpos, ypos, X_height, scl, x_del, y_del;
  static   int kount=0;

  static FT_Library library;
  static FT_Face    face;
  cairo_font_face_t *font_face;
  cairo_matrix_t fmatrix;
  cairo_text_extents_t textents;
  cairo_font_extents_t fextents;

  cairo_text_extents(cairo_context, sptr, &textents);

  cairo_get_font_matrix (cairo_context,&fmatrix);

  cairo_matrix_scale (&fmatrix,1.,-1.);
  cairo_set_font_matrix(cairo_context,&fmatrix);
  cairo_get_font_matrix (cairo_context,&fmatrix);
  if (getenv("CRO_TRACE")) {
    printf("Got to cro_Text\n");
  }

  if (psa->pict_empty) {
    CROpict_init(gksc);
  }


/*
 *  Input coordinates are in NDC space.
 */
  xc = pptr[0].x;
  yc = pptr[0].y;

/*
 *  Initialize the Freetype library and font faces (this is
 *  done only once).
 */
  if (kount == 0) {
    error = FT_Init_FreeType( &library );
    if ( error )
    {
      printf("Error initializing FreeType library\n");
      return 1;
    }
/*
 *  Establish the default font faces.
 */

/*
 * The following is strictly ad hoc.  Ultimately there should be a
 * table associating font numbers (psa->attributes.text_font) with 
 * font names, or some other way to associate such.  Here we are 
 * just using Vera.ttf as the single available font.
 *
 * Also, should need to get the font database path only once.
 */
    font_name = "Vera.ttf";
    db_path = (char *) GetNCARGPath("ftfonts");  /* Path for font database */
    font_path = (char *)calloc(strlen(db_path) + strlen("/") + 
                strlen(font_name) + 1, sizeof(char));
    strcpy(font_path, db_path);
    strcat(font_path, "/");
    strcat(font_path, font_name);
    error = FT_New_Face( library, font_path, 0, &face );
    if ( error == FT_Err_Unknown_File_Format )
    {
        printf("The font file could be opened and read, \n but it appears that its font format is unsupported\n");
        return 1;
    }
    free(font_path);   
    kount++;
  }

  font_face = cairo_ft_font_face_create_for_ft_face(face,0);

  cairo_set_font_face(cairo_context, font_face);
  cairo_font_extents(cairo_context, &fextents);

/*
 *  Character height.
 */
  cairo_set_font_size (cairo_context, 
      (psa->attributes.char_ht * psa->dspace.yspan/.728));
  cairo_get_font_matrix (cairo_context,&fmatrix);

/*
 *  Get height of capital X.
 */
  single_char[0] = 'X';
  single_char[1] = 0;
  cairo_text_extents(cairo_context, single_char, &textents);

  X_height = textents.height;
  cairo_matrix_scale (&fmatrix,1.,-1.);
  cairo_set_font_matrix(cairo_context,&fmatrix);
  cairo_text_extents(cairo_context, sptr, &textents);

  slen = strlen(sptr);

/*
 *  Character color.
 */
  cval = unpack_argb(ctable[psa->attributes.text_colr_ind]);
  cairo_set_source_rgba(cairo_context,
                          cval.red, cval.green, cval.blue, cval.alpha);

/*
 *  Horizontal and vertical alignments.  Get the text extents and
 *  adjust the X and Y positions accordingly.  
 */
  if (psa->attributes.text_align_horiz <= NORMAL_ALIGNMENT_HORIZ || 
      psa->attributes.text_align_horiz > RIGHT_ALIGNMENT_HORIZ) {
    psa->attributes.text_align_horiz = LEFT_ALIGNMENT_HORIZ;
  }

  cairo_text_extents(cairo_context, sptr, &textents);
  horiz_len = textents.width;
/*
 *  The text extents returned from the above include white space
 *  at the left of the first character and the right of the last
 *  character.  GKS wants to align at character edges, so we subtract
 *  off the small additional white spaces.
 */
  single_char[0] = *sptr;
  single_char[1] = 0;
  cairo_text_extents(cairo_context, single_char, &textents);
  left_space = textents.x_bearing;
  single_char[0] = *(sptr+strlen(sptr)-1);
  single_char[1] = 0;
  cairo_text_extents(cairo_context, single_char, &textents);
  right_space = textents.x_advance - textents.width - textents.x_bearing;

  xpos = xc*psa->dspace.xspan;
  if (psa->attributes.text_align_horiz == LEFT_ALIGNMENT_HORIZ) {
    x_del = -left_space;
  }
  else if (psa->attributes.text_align_horiz == CENTER_ALIGNMENT_HORIZ) {
    x_del = - 0.5*horiz_len - left_space;
  }
  else if (psa->attributes.text_align_horiz == RIGHT_ALIGNMENT_HORIZ) {
    x_del = - horiz_len - right_space;
  }

/*
 *  Rotation angle.
 */
/*
 *  Find the text angle for Plotchar based on the base vector 
 *  and its angle with the vector (1.,0.) using the dot product divided by
 *  the magnitudes to get the arccos.
 */
  base_mag = sqrt(psa->attributes.char_base_vec_x * 
                  psa->attributes.char_base_vec_x +
                  psa->attributes.char_base_vec_y * 
                  psa->attributes.char_base_vec_y);
  tcos = psa->attributes.char_base_vec_x/base_mag;
  if (tcos > 1.) tcos = 1.;
  if (tcos < -1.) tcos = -1.;
  cang = acos(tcos);
  cprod = psa->attributes.char_base_vec_y;  /* cross product */
  if (cprod > 0.) cang = - cang;
  ypos = yc*psa->dspace.yspan;

  cairo_save(cairo_context);
  cairo_move_to(cairo_context,xpos,ypos);
  cairo_rotate(cairo_context,-cang);

  switch (psa->attributes.text_align_vert) {
  case NORMAL_ALIGNMENT_VERT:
    y_del = 0.;
    break;
  case TOP_ALIGNMENT_VERT:
    y_del = -1.25*X_height;
    break;
  case CAP_ALIGNMENT_VERT:
    y_del = -X_height;
    break;
  case HALF_ALIGNMENT_VERT:
    y_del = -0.5*X_height;
    break;
  case BASE_ALIGNMENT_VERT:
    y_del = 0.;
    break;
  case BOTTOM_ALIGNMENT_VERT:
    y_del = 0.3*X_height;
    break;
  }
  cairo_rel_move_to(cairo_context,x_del,y_del);
  cairo_show_text (cairo_context, sptr);
  cairo_restore(cairo_context);

  return(0);
}
int cro_UpdateWorkstation(GKSC *gksc) {


  CROddp   *psa = (CROddp *) gksc->ddp;

  if (getenv("CRO_TRACE")) {
    printf("Got to cro_UpdateWorkstation\n");
  }
                 
  (void) fflush(psa->file_pointer);
  return(0);  
}
/*
 *  Set up the file name for the Postscript output file.
 */
char *GetCPSFileName(int wkid, char *file_name) {
  static char tname[257];
  static char *tch;
  int i;

  for (i = 0; i < 257; i++) {
    tname[i] = 0;
  }

/*
 *  A setting of the environment variable NCARG_GKS_CPSOUTPUT
 *  takes precedence in establishing the root name of the
 *  output files, otherwise it is "cairo".  If the file name
 *  supplied by NCARG_GKS_CPSOUTPUT ends in ".ps" then the
 *  file name provided by NCARG_GKS_CPSOUTPUT will be used,
 *  otherwise the file name provided by NCARG_GKS_CPSOUTPUT
 *  will be used as the root name with a ".ps" appended.
 *
 *  In the default case (i.e. NCARG_GKS_CPSOUTPUT is not set)
 *  the output file name will be "cairoX.ps" where "X" is the
 *  workstation ID.
 */
  tch = getenv("NCARG_GKS_CPSOUTPUT");
  if ( (tch != (char *) NULL) && (strlen(tch) > 0)) {
    if (strlen(tch) >= 3) {
      if (strncmp(tch + strlen(tch) - 3, ".ps", 3) == 0) {
        strncpy(tname, tch, strlen(tch) - 3);
      }
      else {
        strncpy(tname, tch, strlen(tch));
      }
      strcat(tname,".ps");
      return (tname);
    }
    strcat(tname,tch);
    strcat(tname,".ps");
    return (tname);
  }
  else {
    (void) sprintf(tname,"cairo%d.ps",wkid);
    return(tname);
  } 
}

/*
 *  Set up the file name for the PNG output file.
 */
char *GetCPNGFileName(int wks_id, int frame_count)
{
  static char rname[257],ctmp[257];
  static char *tch;
  int i;

  for (i = 0; i < 257; i++) {
    ctmp[i] = 0;
    rname[i] = 0;
  }

/*
 *  A setting of the environment variable NCARG_GKS_CPNGOUTPUT
 *  takes precedence in establishing the root name of the 
 *  output files, otherwise it is "cairo".  If the file name
 *  supplied by NCARG_GKS_CPNGOUTPUT ends in ".png" then the
 *  file name provided by NCARG_GKS_CPNGOUTPUT will be used,
 *  otherwise the file name provided by NCARG_GKS_CPNGOUTPUT
 *  will be used as the root name with a ".png" appended.
 *  The ultimate form of the output file names will be 
 *  cairoX.nnnnnn.png where "X" is the GKS workstation ID
 *  and "nnnnnn" is the frame number, starting with "1".
 */
  tch = getenv("NCARG_GKS_CPNGOUTPUT");
  if ( (tch != (char *) NULL) && (strlen(tch) > 0)) {
    if (strlen(tch) >= 4) {
      if (strncmp(tch + strlen(tch) - 4, ".png", 4) == 0) {
        strncpy(rname, tch, strlen(tch) - 4);
      }
      else {
        strncpy(rname, tch, strlen(tch));
      }
      (void) sprintf(ctmp,".%06d.png",frame_count+1);
      strcat(rname,ctmp);
      return (rname);
    }
    else {
      (void) sprintf(ctmp,".%06d.png",frame_count+1);
      strcat(rname,tch);
      strcat(rname,ctmp);
      return (rname);
    }
  }
  else {
    (void) sprintf(ctmp,"cairo%d.%06d.png",wks_id,frame_count+1);
    return ctmp;
  }
}

static void CROinit(CROddp *psa, int *coords)
{
  int     i, cllx, clly, curx, cury;
  float   rscale;

  psa->hatch_spacing = CRO_HATCH_SPACING;
  psa->path_size = MAX_PATH;
  psa->line_join = ROUND;
  psa->line_cap  = ROUNDED;
  psa->nominal_width_scale = 0.5;
  psa->full_background = FALSE;
  psa->suppress_flag   = SUPPRESS_FLAG;
  psa->miter_limit     = MITER_LIMIT_DEFAULT;
  psa->sfill_spacing   = CRO_FILL_SPACING;
  psa->frame_count     = 0;

/*
 *  Flag to suppress putting out background color rectangle.
 */
  psa->suppress_flag = *(coords+6);

  cllx = *coords;
  clly = *(coords+1);
  curx = *(coords+2);
  cury = *(coords+3);
  if ((cllx != -9999) && (clly != -9999) && 
      (curx != -9999) && (cury != -9999)) {
    psa->dspace.llx = (int) (((float) cllx));
    psa->dspace.urx = (int) (((float) curx));
    psa->dspace.lly = (int) (((float) clly));
    psa->dspace.ury = (int) (((float) cury));
  }
  else {
    psa->dspace.llx = (int) (((float) LLX_DEFAULT));
    psa->dspace.urx = (int) (((float) URX_DEFAULT));
    psa->dspace.lly = (int) (((float) LLY_DEFAULT));
    psa->dspace.ury = (int) (((float) URY_DEFAULT));
  }
  psa->dspace.xspan = ((psa->dspace.urx) - (psa->dspace.llx));
  psa->dspace.yspan = ((psa->dspace.ury) - (psa->dspace.lly));

  psa->cro_clip.llx = psa->dspace.llx;
  psa->cro_clip.lly = psa->dspace.lly;
  psa->cro_clip.urx = psa->dspace.urx;
  psa->cro_clip.ury = psa->dspace.ury;
  psa->cro_clip.null = FALSE;

  psa->background = FALSE;
  psa->pict_empty = TRUE;
  psa->page_number = 1;

  psa->attributes.linetype         = LINETYPE_DEFAULT;
  psa->attributes.linetype_set     = LINETYPE_DEFAULT;
  psa->attributes.linewidth        = LINEWIDTH_DEFAULT;
  psa->attributes.linewidth_set    = LINEWIDTH_DEFAULT;
  psa->attributes.line_colr_ind    = LINE_COLR_DEFAULT;
  psa->attributes.marker_type      = MARKER_TYPE_DEFAULT;
  psa->attributes.marker_size      = MARKER_SIZE_DEFAULT;
  psa->attributes.marker_colr_ind  = MARKER_COLR_IND_DEFAULT;
  psa->attributes.text_font        = TEXT_FONT_DEFAULT;
  psa->attributes.text_prec        = TEXT_PREC_DEFAULT;
  psa->attributes.char_expan       = CHAR_EXPAN_DEFAULT;
  psa->attributes.char_space       = CHAR_SPACE_DEFAULT;
  psa->attributes.text_colr_ind    = TEXT_COLR_IND_DEFAULT;
  psa->attributes.char_ht          = CHAR_HT_DEFAULT;
  psa->attributes.char_up_vec_x    = CHAR_UP_VEC_X_DEFAULT;
  psa->attributes.char_up_vec_y    = CHAR_UP_VEC_Y_DEFAULT;
  psa->attributes.char_base_vec_x  = CHAR_BASE_VEC_X_DEFAULT;
  psa->attributes.char_base_vec_y  = CHAR_BASE_VEC_y_DEFAULT;
  psa->attributes.text_path        = TEXT_PATH_DEFAULT;
  psa->attributes.text_align_horiz = TEXT_ALIGN_HORIZ_DEFAULT;
  psa->attributes.text_align_vert  = TEXT_ALIGN_VERT_DEFAULT;
  psa->attributes.fill_int_style   = FILL_INT_STYLE_DEFAULT;
  psa->attributes.fill_style_ind   = FILL_STYLE_IND_DEFAULT;
  psa->attributes.fill_colr_ind    = FILL_COLR_IND_DEFAULT;
  psa->attributes.cro_colr_ind     = CRO_COLR_IND_DEFAULT;
  psa->attributes.clip_ind         = CLIP_IND_DEFAULT;

}

unsigned int pack_argb(struct color_value cval) {
/*
 *  Store as rgba
 */
  int ir,ig,ib,ia;
  unsigned int rval;
  ia = (int) (cval.alpha*255.);
  ir = (int) (cval.red*255.);
  ig = (int) (cval.green*255.);
  ib = (int) (cval.blue*255.);
  rval = ib;
  rval = (ig << 8)  | rval;
  rval = (ir << 16) | rval;
  rval = (ia << 24) | rval;
  return rval;
}
struct color_value unpack_argb(unsigned int argb) {
  struct color_value cval;
  cval.blue = ((float) (argb & 255)/255.);
  cval.green = ((float) ((argb >> 8) & 255)/255.);
  cval.red = ((float) ((argb >> 16) & 255)/255.);
  cval.alpha = ((float) ((argb >> 24) & 255)/255.);
  return cval;
}
/*
 *      File:           cro_fill.c
 *
 *      Author:         Fred Clare
 *                      National Center for Atmospheric Research
 *                      PO 3000, Boulder, Colorado
 *
 *      Date:           Wed Sep 24 14:48:18 MDT 2008
 *
 *      Description:    This file contains routines for doing software
 *                      fill.  The argument "angle" specifies the angle
 *                      of the fill lines; the spacing between the fill
 *                      lines is extracted from the device dependent data.
 *                      This program is a conversion of the algorithm
 *                      implemented in Fortran in the NCAR Softfill package.
 */
void cro_SoftFill (GKSC *gksc, float angle, float spl)
{
  float   xco, yco, spi, rangle, tmp, smalld=.000001, *rst, tmpx, tmpy;
  int     *ind, nnd, nra;
  int     jnd, knd, ipt, ipe, indx1, indx2, previous_point, following_point;
  int     i, isp, ipx, lnd, ip1, ip2, in1, in2, jn1, jn2, jnt;
  int     ocounter, counter_inc=3;
  CROPoint *points, opoint;
  CROddp   *psa;

  psa = (CROddp *) gksc->ddp;
  points = (CROPoint *) (gksc->p).list;
  nra = (gksc->p).num;

  rangle = 0.017453292519943 * angle;   /* converts angle to radians */

/*
 *  Allocate memory.  
 *
 *    rst --  The first nra elements of the rst array are used to 
 *            store the directed distances of the points in the given
 *            polygon from the base line "xco*x+yco*y=0" (see code below 
 *            for the computation of xco and yco).  The second nra 
 *            elements (starting with rst[nra]) of rst are used to 
 *            store the points of intersection of the current fill line 
 *            with the line segments of the polygon (only one number is 
 *            required since we know the distance of the current fill 
 *            line from the base line).
 *
 *    ind --  The first nra elements of ind are used to store a permutation
 *            vector that orders the directed distances in rst[0] through
 *            rst[nra-1].  The second nra elements of rst, starting with
 *            rst[nra], is a permutation vector ordering the points of
 *            intersection stored in rst[nra], rst[nra+1], ... .  The
 *            third nra elements of ind are pointers to the points of
 *            intersection of the current line with the points of the
 *            input polygon.  These pointers are stored backwards in
 *            ind beginning with ind[3*nra].  Point "n" in this list
 *            refers to the line segment in the original polygon that
 *            begins at the previous point and terminates at point "n".
 */

  rst = (float *) malloc (2 * nra * sizeof(float));
  ind = (int   *) malloc (3 * nra * sizeof(int  ));
  nnd = 3*nra;

/* 
 *  Compute the constants "xco" and "yco" such that any line having an
 *  equation of the form "xco*x+yco*y=c" will have the desired angle.
 */
  xco = (float) (-sin ((double) rangle));
  yco = (float) ( cos ((double) rangle));

/* 
 *  Compute the directed distances of the given points from the line
 *  "xco*x+yco*y=0".
 */
  for (i = 0; i < nra; ++i)
    rst[i] = xco * points[i].x + yco * points[i].y;

/* 
 *  Generate a list of indices of the distances, sorted by increasing
 *  distance.  rst[ind[1]], rst[ind[2]], ... rst[ind[nra]] 
 *  is a list of the directed distances of the given points, in increasing 
 *  numerical order.
 */
  cascsrt (rst, ind, nra);

/* 
 *  Draw lines at distances "spi" from the line "xco*x+yco*y=0" which are
 *  multiples of "spl" between the smallest and largest point distances.
 *  jnd points to the index of that point having the greatest distance
 *  less than the distance of the last line drawn (initially 0) and knd
 *  points to the end of the list of line segments which the last line
 *  drawn intersected - it is stored backwards at the end of ind - the
 *  initial value specifies that this list is empty.
 */
  jnd = -1;
  knd = nnd;

/* 
 *  ipt is the index of the next point past the last line drawn and ipe
 *  is the index of the last point.
 */
  ipt = ind[0];
  ipe = ind[nra - 1];
  indx1 = (int) (rst[ipt] / spl) - 1;
  indx2 = (int) (rst[ipe] / spl) + 1;
  ocounter = 0;
  for (isp = indx1; isp <= indx2; isp++)
  {
    spi = (float) isp * spl;

/* 
 *  Advance jnd to reflect the number of points passed over by the
 *  algorithm and update the list of pointers to intersecting lines.
 */
    while ((jnd < nra - 1) && (spi > rst[ipt]))
    {
      previous_point = (ipt + nra - 1) % nra;
      following_point = (ipt + 1) % nra;
      if (rst[previous_point] < rst[ipt])
      {
        ipx = previous_point;

/* 
 *  Remove intersecting line
 */
        if (knd < nnd)
        {
          for (i = knd; i < nnd; ++i)
          {
            if (ind[i] == ipx)
            {
              ind[i] = ind[knd];
              ++knd;
              break;
            }
          }
        }
      }
      else if (rst[previous_point] > rst[ipt])
      {

/* 
 *  Add an intersecting line.
 */
        ipx = previous_point;
        --knd;
        ind[knd] = ipx;
      }
      if (rst[ipt] > rst[following_point])
      {
        ipx = ipt;

/* 
 *  Remove intersecting line
 */
        if (knd < nnd)
        {
          for (i = knd; i < nnd; ++i)
          {
            if (ind[i] == ipx)
            {
              ind[i] = ind[knd];
              ++knd;
              break;
            }
          }
        }
      }
      else if (rst[ipt] < rst[following_point])
      {

        /* 
         *  Add an intersecting line.
         */
        ipx = ipt;
        --knd;
        ind[knd] = ipx;
      }
      ++jnd;
      ipt = ind[jnd + 1];
    }
/* 
 *  Compute a set of values representing the intersection points of the
 *  current line with the line segments of the polygon.
 */
    if (knd < nnd)
    {
      lnd = nra - 1;
      if (fabs (xco) > fabs (yco))
      {
        for (i = knd; i < nnd; ++i)
        {
          ip1 = ind[i];
          ip2 = (ind[i] + 1) % nra;
          ++lnd;
          tmp = xco * (points[ip2].x - points[ip1].x) 
                + yco * (points[ip2].y - points[ip1].y);
          if (fabs(tmp) > smalld)
          {
            rst[lnd] = (spi * (points[ip2].y - points[ip1].y) - xco *
              (points[ip1].x*points[ip2].y - points[ip2].x*points[ip1].y))/tmp;
          }
          else
          {
            rst[lnd] = .5*(points[ip1].y + points[ip2].y);
          }
        }
      }
      else
      {
        for (i = knd; i < nnd; ++i)
        {
          ip1 = ind[i];
          ip2 = (ind[i] + 1) % nra;
          ++lnd;
          tmp = xco * (points[ip2].x - points[ip1].x) 
                + yco * (points[ip2].y - points[ip1].y); 
          if (fabs(tmp) > smalld)
          {
            rst[lnd] = (spi * (points[ip2].x - points[ip1].x) + yco *
             (points[ip1].x*points[ip2].y - points[ip2].x*points[ip1].y))/tmp;
          }
          else
          {
            rst[lnd] = .5*(points[ip1].x+points[ip2].x);
          } 
        }
      }

/* 
 *  Put these values in ascending order.  Actually, once again, 
 *  we set up an index array specifying the order.
 */
      cascsrt (rst + nra, ind + nra, lnd - nra + 1);

/* 
 *  Draw the line segments specified by the list.
 */
      in1 = nra;
      if (fabs (xco) > fabs (yco))
      {
        while (in1 < lnd)
        {
          jn1 = nra + ind[in1];
          in2 = in1 + 1;
          for(;;)
          {
            jn2 = nra + ind[in2];
            if (in2 >= lnd)
              break;
            jnt = nra + ind[in2 + 1];
            if ((rst[jnt] - rst[jn2]) > smalld)
              break;
            in2 += 2;
          }
          if (rst[jn2] - rst[jn1] > smalld)
          {
            opoint.x = (spi - yco * rst[jn1]) / xco;
            opoint.y = rst[jn1];
            tmpx = opoint.x * (float) psa->dspace.xspan;
            tmpy = opoint.y * (float) psa->dspace.yspan;
            cairo_move_to(cairo_context,tmpx,tmpy);
            opoint.x = (spi - yco * rst[jn2]) / xco;
            opoint.y = rst[jn2];
            tmpx = opoint.x * (float) psa->dspace.xspan;
            tmpy = opoint.y * (float) psa->dspace.yspan;
            cairo_line_to(cairo_context,tmpx,tmpy);
            cairo_stroke(cairo_context);
          }
          in1 = in2 + 1;
        }
      }
      else
      {
        while (in1 < lnd)
        {
          jn1 = nra + ind[in1];
          in2 = in1 + 1;
          for(;;)
          {
            jn2 = nra + ind[in2];
            if (in2 >= lnd)
              break;
            jnt = nra + ind[in2 + 1];
            if (rst[jnt] - rst[jn2] > smalld)
              break;
            in2 += 2;
          }
          if (rst[jn2] - rst[jn1] > smalld)
          {
            opoint.x = rst[jn1];
            opoint.y = (spi - xco * rst[jn1]) / yco;
            tmpx = opoint.x * (float) psa->dspace.xspan;
            tmpy = opoint.y * (float) psa->dspace.yspan;
            cairo_move_to(cairo_context,tmpx,tmpy);
            opoint.x = rst[jn2];
            opoint.y = (spi - xco * rst[jn2]) / yco;
            tmpx = opoint.x * (float) psa->dspace.xspan;
            tmpy = opoint.y * (float) psa->dspace.yspan;
            cairo_line_to(cairo_context,tmpx,tmpy);
            cairo_stroke(cairo_context);
          }
          in1 = in2 + 1;
        }
      }
    }
  }
  free (rst);
  free (ind);
}


/*
 *  Given an array of  n  floating values in  xa, 
 *  cascsrt returns a permutation vector ip such that
 * 
 *   xa[ip[i]] <= xa[ip[j]]
 *         for all i,j such that  1 <= i <= j <= n .
 *
 *  This function uses the C library function qsort.
 */

static void cascsrt(float xa[], int ip[], int n)

{
        int i;

        if (n <= 0) {
                return;
        }
        else if (n == 1) {
                ip[0] = 0;
                return;
        }

        csort_array = xa;

        for (i = 0; i < n; i++) {
                ip[i] = i;
        }

        qsort ( (void *) ip, n, sizeof(ip[0]), ccompar);
        return;
}

static int ccompar(const void *p1, const void *p2)
{
        float difference;
        int *i1,*i2;
        i1 = (int *) p1;
        i2 = (int *) p2;
        difference = csort_array[*i1] - csort_array[*i2];
        if (difference < 0.0) return (-1);
        if (difference > 0.0) return ( 1);
        return (0);
}
