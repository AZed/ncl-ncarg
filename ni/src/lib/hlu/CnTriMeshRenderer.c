/*
 *      $Id: CnTriMeshRenderer.c,v 1.17 2010/03/27 18:58:25 dbrown Exp $
 */
/************************************************************************
*									*
*			     Copyright (C)  1992			*
*	     University Corporation for Atmospheric Research		*
*			     All Rights Reserved			*
*									*
************************************************************************/
/*
 *	File:		
 *
 *	Author:		David I. Brown
 *			National Center for Atmospheric Research
 *			PO 3000, Boulder, Colorado
 *
 *	Date:		Thu Apr 23 12:01:04 MDT 1998
 *
 *	Description:	
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ncarg/hlu/hluutil.h>
#include <ncarg/hlu/CnTriMeshRendererP.h>
#include <ncarg/hlu/WorkstationI.h>
#include <ncarg/hlu/MapTransObj.h>
#include <ncarg/hlu/IrregularTransObjP.h>
#include <ncarg/hlu/TriMeshTransObj.h>
#ifdef BuildTRIANGLE
#define REAL double
#include <ncarg/hlu/triangle.h>
#endif

#define Oset(field) \
    NhlOffset(NhlCnTriMeshRendererLayerRec,cntrimeshrenderer.field)

static NhlResource resources[] = {
	{NhlNtriMeshUpdateMode,NhlCtriMeshUpdateMode,NhlTInteger,
		 sizeof(int),Oset(update_mode),NhlTImmediate,
		 _NhlUSET((NhlPointer) TRIMESH_NEWMESH),_NhlRES_PRIVATE,NULL}
};

static NhlErrorTypes CnTriMeshRendererInitialize(
#if	NhlNeedProto
        NhlClass	class,
        NhlLayer	req,
        NhlLayer	new,
        _NhlArgList	args,
        int             num_args
#endif
);

static NhlErrorTypes CnTriMeshRendererDestroy(
#if	NhlNeedProto
        NhlLayer        /* inst */
#endif
);

static NhlErrorTypes CnTriMeshRender(
#if     NhlNeedProto
        NhlLayer                instance,
        NhlContourPlotLayer     cnl,
	NhlDrawOrder            order,
	NhlString		entry_name
#endif
);


extern int (_NHLCALLF(ctdrpl,CTDRPL))(
#if	NhlNeedProto
	float *xcs, 
	float *ycs,
	int *ncs,
	int *iai,
	int *iag,
	int *nai
#endif
);

extern int (_NHLCALLF(hluctfill,HLUCTFILL))(
#if	NhlNeedProto
	float *xcs, 
	float *ycs, 
	int *ncs, 
	int *iai, 
	int *iag, 
	int *nai
#endif
);

extern void  (_NHLCALLF(hluctscae,HLUCTSCAE))(
#if	NhlNeedProto
	int		*icra,
	int		*ica1,
	int		*icam,
	int		*ican,
	float		*xcpf,
	float		*ycpf,
	float		*xcqf,
	float		*ycqf,
	int		*ind1,
	int		*ind2,
	int		*icaf,
	int		*iaid		      
#endif
);

extern void   (_NHLCALLF(hluctchcl,HLUCTCHCL))(
#if	NhlNeedProto
	int	*iflg
#endif
);

extern void   (_NHLCALLF(hluctchhl,HLUCTCHHL))(
#if	NhlNeedProto
	int	*iflg
#endif
);

extern void   (_NHLCALLF(hluctchll,HLUCTCHLL))(
#if	NhlNeedProto
	int	*iflg
#endif
);

extern void   (_NHLCALLF(hluctmxyz,HLUCTMXYZ))(
#if	NhlNeedProto
	int	*imap,
	float	*xinp,
	float	*yinp,
	float   *zinp,
	float	*xotp,
	float	*yotp
#endif
);

static int   (_NHLCALLF(rtmi,RTMI))(
#if	NhlNeedProto
	int	*idim,
	int	*jdim,
	int     *iini,
	int     *jini,
	int     *iino,
	int     *jino
#endif
);

static void   load_hluct_routines(
#if	NhlNeedProto
	NhlBoolean	flag
#endif
);


static NhlErrorTypes CnTriMeshWriteCellData
#if	NhlNeedProto
(
	float		*rpnt,
	int             *iedg,
	int             *itri,
	int		icam,
	int		ican,
	float		xcpf,
	float		ycpf,
	float		xcqf,
	float		ycqf,
	char		*entry_name
#endif
	);

NhlCnTriMeshRendererClassRec NhlcnTriMeshRendererClassRec = {
	{
/* class_name 		*/      "cnTriMeshRendererClass",
/* nrm_class 		*/      NrmNULLQUARK,
/* layer_size 		*/      sizeof(NhlCnTriMeshRendererLayerRec),
/* class_inited 	*/	False,
/* superclass		*/      (NhlClass)&NhlobjClassRec,
/* cvt_table		*/	NULL,

/* layer_resources 	*/   	resources,
/* num_resources 	*/     	NhlNumber(resources),
/* all_resources 	*/	NULL,
/* callbacks		*/	NULL,
/* num_callbacks	*/	0,
/* class_callbacks	*/	NULL,
/* num_class_callbacks	*/	0,

/* class_part_initialize */     NULL,
/* class_initialize 	*/  	NULL,
/* layer_initialize 	*/  	CnTriMeshRendererInitialize,
/* layer_set_values 	*/  	NULL,
/* layer_set_values_hook */  	NULL,
/* layer_get_values 	*/  	NULL,
/* layer_reparent 	*/  	NULL,
/* layer_destroy 	*/    	CnTriMeshRendererDestroy,
	},
	{
/* render */		        CnTriMeshRender
	},
	{
/* foo */		        0
	}
};

NhlClass NhlcnTriMeshRendererClass = (NhlClass)&NhlcnTriMeshRendererClassRec;

typedef enum { 
	cnInt,
	cnFloat,
	cnString 
} _cnParamType;

typedef struct _cnCt_Params {
	NhlString	name;
	_cnParamType	type;
} cnCt_Params;

static cnCt_Params Ct_Params[] = {
{"HCL", cnFloat},
{"HCS", cnFloat}, 
{"HCF", cnInt}, 
{"HLX", cnInt}, 
{"HLY", cnInt}, 
{"IWM", cnInt}, 
{"PC1", cnFloat},
{"PC2", cnFloat}, 
{"PC3", cnFloat}, 
{"PC4", cnFloat}, 
{"PC5", cnFloat}, 
{"PC6", cnFloat},
{"PIC", cnInt}, 
{"PIE", cnInt},
{"PW1", cnFloat}, 
{"PW2", cnFloat}, 
{"PW3", cnFloat},
{"PW4", cnFloat}, 
{"RC1", cnFloat}, 
{"RC2", cnFloat}, 
{"RC3", cnFloat}, 
{"RWC", cnInt}, 
{"RWG", cnInt}, 
{"RWM", cnInt},
};

static float LowLabelFactor = 1.0;
#define NhlDASHBUFSIZE	128

static NhlContourPlotLayer	Cnl = NULL;
static NhlContourPlotLayerPart	*Cnp = NULL;
static NhlCnTriMeshRendererLayerPart   *Tmp = NULL;

static int Lopn = 4;
static int Loen = 5;
static int Lotn = 4;


/*
 * Function: rtmi_
 *
 * Description: 
 *		
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
static int (_NHLCALLF(rtmi,RTMI))
#if	NhlNeedProto
(
	int	*idim,
	int	*jdim,
	int     *iini,
	int     *jini,
	int     *iino,
	int     *jino
)
#else
(idim,jdim,iini,jini,iino,jino)
	int	*idim;
	int	*jdim;
	int     *iini;
	int     *jini;
	int     *iino;
	int     *jino;
#endif
{
	/* assume simplest situation for now */

	*jino = *jini;
	*iino = *iini;

#if 0
	*iino = *iini;
	if (*jini == *jdim) {
		if (*iini > *idim / 2 && *iini < *idim - 1) {
			*iino = *idim - *iini - 1;
		}
		else if (*iini == *idim) {
			*iino = 1;
		}
		else {
			*iino = *iini;
		}
		printf("ini %d outi %d\n",*iini, *iino);
	}

#endif
	return 0;
}

static NhlErrorTypes BuildTriangularMesh 
#if	NhlNeedProto
(
	NhlCnTriMeshRendererLayerPart *tmp,
	NhlContourPlotLayer     cnl,
	NhlString entry_name
)
#else
(tmp,cnl,entry_name)
        NhlCnTriMeshRendererLayerPart *tmp;
	NhlContourPlotLayer     cnl;
	NhlString entry_name;
#endif
{
	NhlContourPlotLayerPart	*cnp = &cnl->contourplot;
	int *iscr;
	float missing_val;
	int coords_alloced = 0;
	int idim = cnp->sfp->fast_len;
	int jdim = cnp->sfp->slow_len;
	int idm1 = cnp->sfp->fast_len - 1;
	int jdm1 = cnp->sfp->slow_len - 1;

	int mnop = idim * jdim;
	int mnoe = 3 *idm1 * jdm1 + idm1 + jdm1;
	int mnot = 2 * idm1 * jdm1;
	int mpnt = mnop * Lopn;
	int medg = mnoe * Loen;
	int mtri = mnot * Lotn;
	float *rpnt;
	int *iedg, *itri;
	int npnt,nedg,ntri;
	float *rlat,*rlon,*rdat;
	int i,j;

	if (tmp->npnt > 0)
		NhlFree(tmp->rpnt);
	if (tmp->nedg > 0)
		NhlFree(tmp->iedg);
	if (tmp->ntri > 0)
		NhlFree(tmp->itri);

	iscr = NhlMalloc(4 * idim * jdim * sizeof(int));
	rpnt = NhlMalloc(mpnt * sizeof(float));
	iedg = NhlMalloc(medg * sizeof(int));
	itri = NhlMalloc(mtri * sizeof(int));

	if (! iscr || ! rpnt || ! iedg || ! itri) {
		NHLPERROR((NhlFATAL,ENOMEM,NULL));
		return NhlFATAL;
	}
	
	missing_val = cnp->sfp->missing_value_set ?
		cnp->sfp->missing_value : -FLT_MAX;

	if (cnp->sfp->x_arr && cnp->sfp->x_arr->num_dimensions == 2) {
		float x[9],y[9];
			
		
		/* 
		 * since the triangular mesh algorithm does not handle
		 * coordinates not defined at the grid points we 
		 * must interpolate bounding coordinates to the 
		 * cell centers as best we can.
		 */
		if (! (cnp->sfp->xc_is_bounds || cnp->sfp->yc_is_bounds)) {
			rlat = (float*)cnp->sfp->y_arr->data;
			rlon = (float*)cnp->sfp->x_arr->data;
		}
		else if (cnp->sfp->xc_is_bounds && cnp->sfp->yc_is_bounds) {
			int jd, jbd, jbdp;
			float *tlat, *tlon;
			int cidim = idim + 1; 
			tlat = (float*)cnp->sfp->y_arr->data;
			tlon = (float*)cnp->sfp->x_arr->data;
			coords_alloced = 1;
			rlat = NhlMalloc(idim * jdim * sizeof(float));
			rlon = NhlMalloc(idim * jdim * sizeof(float));
			for (j = 0; j < jdim; j++) {
				jd = j * idim;
				jbd = j * cidim;
				jbdp = (j+1) * cidim;
				for (i = 0; i < idim; i++) {
				     *(rlon+jd+i) = 
				       (*(tlon+jbd+i) + *(tlon+jbd+i+1) +
				        *(tlon+jbdp+i) + *(tlon+jbdp+i+1)) 
					     / 4.0;
				     *(rlat+jd+i) = 
				       (*(tlat+jbd+i) + *(tlat+jbd+i+1) +
				        *(tlat+jbdp+i) + *(tlat+jbdp+i+1)) 
					     / 4.0;
				}
			}
		}
		else if (cnp->sfp->xc_is_bounds) {
			int jd, jbd;
			float *tlat, *tlon;
			int cidim = idim + 1; 
			tlat = (float*)cnp->sfp->y_arr->data;
			tlon = (float*)cnp->sfp->x_arr->data;
			coords_alloced = 1;
			rlat = NhlMalloc(idim * jdim * sizeof(float));
			rlon = NhlMalloc(idim * jdim * sizeof(float));
			for (j = 0; j < jdim; j++) {
				jd = j * idim;
				jbd = j * cidim;
				for (i = 0; i < idim; i++) {
				     *(rlon+jd+i) = 
				       (*(tlon+jbd+i) + *(tlon+jbd+i+1)) / 2.0;
				     *(rlat+jd+i) = 
				       (*(tlat+jbd+i) + *(tlat+jbd+i+1)) / 2.0;
				}
			}
		}
		else if (cnp->sfp->yc_is_bounds) {
			int jd, jdp;
			float *tlat, *tlon;
			tlat = (float*)cnp->sfp->y_arr->data;
			tlon = (float*)cnp->sfp->x_arr->data;
			coords_alloced = 1;
			rlat = NhlMalloc(idim * jdim * sizeof(float));
			rlon = NhlMalloc(idim * jdim * sizeof(float));
			for (j = 0; j < jdim; j++) {
				jd = j * idim;
				jdp = (j + 1) * idim;
				for (i = 0; i < idim; i++) {
				     *(rlon+jd+i) = 
				       (*(tlon+jd+i) + *(tlon+jdp+i)) / 2.0;
				     *(rlat+jd+i) = 
				       (*(tlat+jd+i) + *(tlat+jdp+i)) / 2.0;
				}
			}
		}
	}
	else {
		float *x;
		rlat = NhlMalloc(idim * jdim * sizeof(float));
		rlon = NhlMalloc(idim * jdim * sizeof(float));
		coords_alloced = 1;
		if (cnp->sfp->y_arr) {
			float y;
			for (j = 0; j < jdim; j++) {
				y = ((float*)cnp->sfp->y_arr->data)[j];
				for (i = 0; i < idim; i++) {
					*(rlat + j*idim+i) = y;
				}
			}
		}
		else {
			float y;
			float step = (cnp->sfp->y_end - cnp->sfp->y_start) /
				(jdim - 1);
			for (j = 0; j < jdim; j++) {
				y = cnp->sfp->y_start + j * step;
				for (i = 0; i < idim; i++) {
					*(rlat + j*idim+i) = y;
				}
			}
		}
		if (cnp->sfp->x_arr) {
			float *x = ((float*)cnp->sfp->x_arr->data);
			for (j = 0; j < jdim; j++) {
				memcpy(rlon + j*idim,x,idim * sizeof(float));
			}
		}
		else {
			float x;
			float step = (cnp->sfp->x_end - cnp->sfp->x_start) /
				(idim - 1);
			for (i = 0; i < idim; i++) {
				x = cnp->sfp->x_start + i * step;
				for (j = 0; j < jdim; j++) {
					*(rlon + j*idim + i) = x;
				}
			}
		}
	}
	
	rdat = (float*)cnp->sfp->d_arr->data;

	if (tmp->ezmap) {
		c_cttmrg(idim,jdim,rlat,rlon,rdat,
			 iscr,missing_val,
			 _NHLCALLF(rtmi,RTMI),
			 rpnt,mpnt,&npnt,Lopn,
			 iedg,medg,&nedg,Loen,
			 itri,mtri,&ntri,Lotn);
	}
	else {
		_NHLCALLF(trmrgr,TRMRGR)
			(&idim,&jdim,rlon,rlat,rdat,
			 iscr,&missing_val,
			 rpnt,&mpnt,&npnt,&Lopn,
			 iedg,&medg,&nedg,&Loen,
			 itri,&mtri,&ntri,&Lotn);
	}

	NhlFree(iscr);
	if (coords_alloced) {
		NhlFree(rlon);
		NhlFree(rlat);
	}

	tmp->npnt = npnt;
	tmp->nedg = nedg;
	tmp->ntri = ntri;
	tmp->rpnt = rpnt;
	tmp->iedg = iedg;
	tmp->itri = itri;
	tmp->update_mode = TRIMESH_NOUPDATE;

	return NhlNOERROR;

}

#define DEGTORAD 0.017453292519943

static NhlErrorTypes BuildNativeMesh 
#if	NhlNeedProto
(
	NhlCnTriMeshRendererLayerPart *tmp,
	NhlContourPlotLayer     cnl,
	NhlString entry_name
)
#else
(tmp,cnl,entry_name)
        NhlCnTriMeshRendererLayerPart *tmp;
	NhlContourPlotLayer     cnl;
	NhlString entry_name;
#endif
{
	NhlContourPlotLayerPart	*cnp = &cnl->contourplot;
	int coords_alloced = 0;
	int mnot  = cnp->sfp->element_nodes->len_dimensions[0];
	int mnop = cnp->sfp->fast_len;
	int mnoe = 3 * mnot;
	int mpnt = mnop * Lopn;
	int medg = mnoe * Loen;
	int mtri = mnot * Lotn;
	float *rpnt;
	int *el;
	int *iedg, *itri;
	int *ippp,*ippe;
	int npnt,nedg;
	float *rlat,*rlon,*rdat;
	float tbuf[5021][12];
	int kbuf = 173;
	int mbuf = 5021;
	int nppp = 0;
	int nppe = 0;
	int nbuf = 0;
	int ntri = 0;
	int i,j;
	int ix_offset = cnp->sfp->ix_start;
	int err_num;
	char *e_msg;
	char *e_text;
	
	if (tmp->npnt > 0)
		NhlFree(tmp->rpnt);
	if (tmp->nedg > 0)
		NhlFree(tmp->iedg);
	if (tmp->ntri > 0)
		NhlFree(tmp->itri);

	rpnt = NhlMalloc(mpnt * sizeof(float));
	iedg = NhlMalloc(medg * sizeof(int));
	itri = NhlMalloc(mtri * sizeof(int));
	ippp = NhlMalloc(2 * mnop * sizeof(int));
	ippe = NhlMalloc(2 * mnoe * sizeof(int));
	
	if (! (rpnt && iedg && itri && ippp && ippe )) {
		NHLPERROR((NhlFATAL,ENOMEM,NULL));
		return NhlFATAL;
	}
	    
	rlat = (float*)cnp->sfp->y_arr->data;
	rlon = (float*)cnp->sfp->x_arr->data;
	rdat = (float*)cnp->sfp->d_arr->data;
	el = (int*) cnp->sfp->element_nodes->data;

	    

	for (i = 0; i < mnot; i++) {
		int *ep;
		int e0,e1,e2;
		if (nbuf >= mbuf) 
			_NHLCALLF(cttmtl,CTTMTL)
				(&kbuf,tbuf,&mbuf,&nbuf,
				 ippp,&mnop,&nppp,
				 ippe,&mnoe,&nppe,
				 rpnt,&mpnt,&npnt,&Lopn,
				 iedg,&medg,&nedg,&Loen,
				 itri,&mtri,&ntri,&Lotn);
		if (c_nerro(&err_num) != 0) {
			e_msg = c_semess(0);
			e_text = "%s: %s";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name,e_msg);
			return NhlFATAL;
		}

		ep = el + i * 3;
		e0 = *ep - ix_offset;
		e1 = *(ep+1) - ix_offset;
		e2 = *(ep+2) - ix_offset;
		if (tmp->ezmap) {
			tbuf[nbuf][0] = cos(DEGTORAD * rlat[e0]) *
				cos(DEGTORAD * rlon[e0]);
			tbuf[nbuf][1] = cos(DEGTORAD * rlat[e0]) *
				sin(DEGTORAD * rlon[e0]);
			tbuf[nbuf][2] = sin(DEGTORAD * rlat[e0]);
			tbuf[nbuf][3] = rdat[e0];
			tbuf[nbuf][4] = cos(DEGTORAD * rlat[e1]) *
				cos(DEGTORAD * rlon[e1]);
			tbuf[nbuf][5] = cos(DEGTORAD * rlat[e1]) *
				sin(DEGTORAD * rlon[e1]);
			tbuf[nbuf][6] = sin(DEGTORAD * rlat[e1]);
			tbuf[nbuf][7] = rdat[e1];
			tbuf[nbuf][8] = cos(DEGTORAD * rlat[e2]) *
				cos(DEGTORAD * rlon[e2]);
			tbuf[nbuf][9] = cos(DEGTORAD * rlat[e2]) *
				sin(DEGTORAD * rlon[e2]);
			tbuf[nbuf][10] = sin(DEGTORAD * rlat[e2]);
			tbuf[nbuf][11] = rdat[e2];
		}
		else {
			tbuf[nbuf][0] = rlon[e0];
			tbuf[nbuf][1] = rlat[e0];
			tbuf[nbuf][2] = 0.0;
			tbuf[nbuf][3] = rdat[e0];

			tbuf[nbuf][4] = rlon[e1];
			tbuf[nbuf][5] = rlat[e1];
			tbuf[nbuf][6] = 0.0;
			tbuf[nbuf][7] = rdat[e1];

			tbuf[nbuf][8] = rlon[e2];
			tbuf[nbuf][9] = rlat[e2];
			tbuf[nbuf][10] = 0.0;
			tbuf[nbuf][11] = rdat[e2];
		}			
		if (! cnp->sfp->missing_value_set)
			nbuf++;
		else if (tbuf[nbuf][3] != cnp->sfp->missing_value && 
			 tbuf[nbuf][7] != cnp->sfp->missing_value &&
			 tbuf[nbuf][11] != cnp->sfp->missing_value) {
			nbuf++;
		}
	}
	if (nbuf > 0) {
		_NHLCALLF(cttmtl,CTTMTL)
			(&nbuf,tbuf,&mbuf,&nbuf,
			 ippp,&mnop,&nppp,
			 ippe,&mnoe,&nppe,
			 rpnt,&mpnt,&npnt,&Lopn,
			 iedg,&medg,&nedg,&Loen,
			 itri,&mtri,&ntri,&Lotn);

		if (c_nerro(&err_num) != 0) {
			e_msg = c_semess(0);
			e_text = "%s: %s";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name,e_msg);
			return NhlFATAL;
		}
	}
	tmp->npnt = npnt;
	tmp->nedg = nedg;
	tmp->ntri = ntri;
	tmp->rpnt = rpnt;
	tmp->iedg = iedg;
	tmp->itri = itri;
	tmp->update_mode = TRIMESH_NOUPDATE;
	NhlFree(ippp);
	NhlFree(ippe);

	return NhlNOERROR;

}

typedef struct _vertex {
	float x;
	float y;
	int node;
	int vert;
} Vertex;

static int vcomp(const void *vv1, const void *vv2)
{
	Vertex *v1 = (Vertex *) vv1;
	Vertex *v2 = (Vertex *) vv2;

	if (v1->x < v2->x)
		return -1;
	if (v1->x > v2->x)
		return 1;
	if (v1->y < v2->y)
		return -1;
	if (v1->y > v2->y)
		return 1;
	if (v1->node < v2->node)
		return -1;
	if (v1->node > v2->node)
		return 1;
	return 0;
}

static void AddElement
#if	NhlNeedProto
(
	int *el,
	Vertex *v,
	int vcount,
	NhlBoolean ezmap,
	float *x,
	float *y
	)
#else 
(el,v,vcount,ezmap,x,y)
	int *el;
	Vertex *v;
	int vcount;
	NhlBoolean ezmap;
	float *x;
	float *y;
#endif       
{
	int n0,n1,n2,n3;
	float detleft, detright,det;
	float xt[3],yt[3];
	int i;

	n0 = v->node;
	n1 = (v+1)->node;
	n2 = (v+2)->node;
	if (vcount > 3) { /* extra node */
		if (n1 == n0) {
			n1 = (v+3)->node;
		}
		else if (n2 == n1 || n2 == n0) {
			n2 = (v+3)->node;
		}
	}
	xt[0] = x[n0];
	xt[1] = x[n1];
	xt[2] = x[n2];
	yt[0] = y[n0];
	yt[1] = y[n1];
	yt[2] = y[n2];

	if (ezmap) {
		float min, max;
		min = max = xt[0];
		for (i = 1; i < 3; i++) {
			if (xt[i] < min)
				min = xt[i];
			if (xt[i] > max) 
				max = xt[i];
		}
		if (max - min > 180) {
			for (i = 0; i < 3; i++) {
				if (xt[i] - min > 180) {
					xt[i] -= 360;
				}
			}
		}
	}
			
	detleft = (xt[0] - xt[2]) * (yt[1] - yt[2]);
       	detright = (yt[0] - yt[2]) * (xt[1] - xt[2]);
	det = detleft - detright;
	if (det > 0) {
	    el[0] = n0;
	    el[1] = n1;
	    el[2] = n2;
	}
	else {
		el[0] = n0;
		el[1] = n2;
		el[2] = n1;
	}
	return;
}


static int *GetTriangleNodes
#if	NhlNeedProto
(
	NhlGenArray xnodes,
	NhlGenArray ynodes,
	NhlGenArray xbounds,
	NhlGenArray ybounds,
	NhlBoolean ezmap,
	int *ntri
)
#else
(xnodes,ynodes,xbounds,ybounds,ezmap,ntri)
	NhlGenArray xnodes;
	NhlGenArray ynodes;
	NhlGenArray xbounds;
	NhlGenArray ybounds;
	NhlBoolean ezmap;
	int *ntri;
#endif
{
	int ncount = xnodes->len_dimensions[0];
	int nvert = xbounds->len_dimensions[1];
	int tcount = xbounds->len_dimensions[0] * xbounds->len_dimensions[1];
	Vertex *verts;
	float *x = (float *) xnodes->data;
	float *y = (float *) ynodes->data;
	float *xb = (float *) xbounds->data;
	float *yb = (float *) ybounds->data;
	int i,j;
	int *el;
	int nmatched;
	int tfound;

	verts = NhlMalloc(tcount * sizeof(Vertex));

	for (i = 0; i < ncount; i++) {
		for (j = 0; j < nvert; j++) {
			int ix = i * nvert +j;
			Vertex *v = verts + ix;
			v->node = i;
			v->vert = j;
			v->x = *(xb + ix);
			v->y = *(yb + ix);
		}
	}
	qsort(verts,ncount * nvert,sizeof(Vertex),vcomp);
	
	/*
	 * There should now be sets of three vertices in a row that 
	 * are co-located. However, because of possible boundaries there 
	 * may be singles or doubles interspersed. Each set of three
	 * represents a triangle in the mesh, with the vertex as its 
	 * center. Put the nodes into the triangle element list. 
	 * There should be at most ncount * nvert / 3 triangles with
	 * three elements each.
	 */
	
	el = (int *) NhlMalloc(sizeof(int) * ncount * nvert);

	tfound = 0;
	for (i = 0; i < ncount * nvert; ) {
		nmatched = 0;
		for (j = i + 1;;j++) {
			if (j == ncount * nvert)
				break;
			if (! (verts[j].x == verts[i].x &&
			       verts[j].y == verts[i].y))
				break;
			nmatched++;
		}
		if (nmatched > 1) {
			AddElement(el + 3 * tfound,
				   &verts[i],nmatched+1,ezmap,x,y);
			tfound++;
		}
		else {
			printf("unmatched node %d %d %f %f\n",verts[i].node,
			       verts[i].vert, verts[i].x, verts[i].y);
			if (nmatched == 1) {
				printf("unmatched node %d %d %f %f\n",
				       verts[i+1].node,verts[i+1].vert,
				       verts[i+1].x, verts[i+1].y);
			}
		}
		i = j;
	}
	NhlFree(verts);

	*ntri = tfound;
	return el;
}
static NhlErrorTypes BuildNativeMeshFromBounds 
#if	NhlNeedProto
(
	NhlCnTriMeshRendererLayerPart *tmp,
	NhlContourPlotLayer     cnl,
	NhlString entry_name
)
#else
(tmp,cnl,entry_name)
        NhlCnTriMeshRendererLayerPart *tmp;
	NhlContourPlotLayer     cnl;
	NhlString entry_name;
#endif
{
	NhlContourPlotLayerPart	*cnp = &cnl->contourplot;
	int coords_alloced = 0;
	int mnot;
	int mtri;
	int mnop = cnp->sfp->fast_len;
	int mnoe;
	int mpnt = mnop * Lopn;
	int medg;
	float *rpnt;
	int *el;
	int *iedg, *itri;
	int *ippp,*ippe;
	int npnt,nedg;
	float *rlat,*rlon,*rdat;
	float tbuf[5021][12];
	int kbuf = 173;
	int mbuf = 5021;
	int nppp = 0;
	int nppe = 0;
	int nbuf = 0;
	int ntri = 0;
	int i,j;
	int ix_offset = 0;
	int err_num;
	char *e_msg;
	char *e_text;
	
	if (tmp->npnt > 0)
		NhlFree(tmp->rpnt);
	if (tmp->nedg > 0)
		NhlFree(tmp->iedg);
	if (tmp->ntri > 0)
		NhlFree(tmp->itri);

	el = GetTriangleNodes(cnp->sfp->x_arr,cnp->sfp->y_arr,
			      cnp->sfp->x_cell_bounds,cnp->sfp->y_cell_bounds,
			      tmp->ezmap,&mnot);
	mtri = mnot * Lotn;
	mnoe = 3 * mnot;
	medg = mnoe * Loen;

	rpnt = NhlMalloc(mpnt * sizeof(float));
	iedg = NhlMalloc(medg * sizeof(int));
	itri = NhlMalloc(mtri * sizeof(int));
	ippp = NhlMalloc(2 * mnop * sizeof(int));
	ippe = NhlMalloc(2 * mnoe * sizeof(int));
	
	if (! (rpnt && iedg && itri && ippp && ippe )) {
		NHLPERROR((NhlFATAL,ENOMEM,NULL));
		return NhlFATAL;
	}
	    
	rlat = (float*)cnp->sfp->y_arr->data;
	rlon = (float*)cnp->sfp->x_arr->data;
	rdat = (float*)cnp->sfp->d_arr->data;


	for (i = 0; i < mnot; i++) {
		int *ep;
		int e0,e1,e2;
		if (nbuf >= mbuf) 
			_NHLCALLF(cttmtl,CTTMTL)
				(&kbuf,tbuf,&mbuf,&nbuf,
				 ippp,&mnop,&nppp,
				 ippe,&mnoe,&nppe,
				 rpnt,&mpnt,&npnt,&Lopn,
				 iedg,&medg,&nedg,&Loen,
				 itri,&mtri,&ntri,&Lotn);
		if (c_nerro(&err_num) != 0) {
			e_msg = c_semess(0);
			e_text = "%s: %s";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name,e_msg);
			return NhlFATAL;
		}

		ep = el + i * 3;
		e0 = *ep - ix_offset;
		e1 = *(ep+1) - ix_offset;
		e2 = *(ep+2) - ix_offset;
		if (tmp->ezmap) {
			tbuf[nbuf][0] = cos(DEGTORAD * rlat[e0]) *
				cos(DEGTORAD * rlon[e0]);
			tbuf[nbuf][1] = cos(DEGTORAD * rlat[e0]) *
				sin(DEGTORAD * rlon[e0]);
			tbuf[nbuf][2] = sin(DEGTORAD * rlat[e0]);
			tbuf[nbuf][3] = rdat[e0];
			tbuf[nbuf][4] = cos(DEGTORAD * rlat[e1]) *
				cos(DEGTORAD * rlon[e1]);
			tbuf[nbuf][5] = cos(DEGTORAD * rlat[e1]) *
				sin(DEGTORAD * rlon[e1]);
			tbuf[nbuf][6] = sin(DEGTORAD * rlat[e1]);
			tbuf[nbuf][7] = rdat[e1];
			tbuf[nbuf][8] = cos(DEGTORAD * rlat[e2]) *
				cos(DEGTORAD * rlon[e2]);
			tbuf[nbuf][9] = cos(DEGTORAD * rlat[e2]) *
				sin(DEGTORAD * rlon[e2]);
			tbuf[nbuf][10] = sin(DEGTORAD * rlat[e2]);
			tbuf[nbuf][11] = rdat[e2];
		}
		else {
			tbuf[nbuf][0] = rlon[e0];
			tbuf[nbuf][1] = rlat[e0];
			tbuf[nbuf][2] = 0.0;
			tbuf[nbuf][3] = rdat[e0];

			tbuf[nbuf][4] = rlon[e1];
			tbuf[nbuf][5] = rlat[e1];
			tbuf[nbuf][6] = 0.0;
			tbuf[nbuf][7] = rdat[e1];

			tbuf[nbuf][8] = rlon[e2];
			tbuf[nbuf][9] = rlat[e2];
			tbuf[nbuf][10] = 0.0;
			tbuf[nbuf][11] = rdat[e2];
		}			
		if (! cnp->sfp->missing_value_set)
			nbuf++;
		else if (tbuf[nbuf][3] != cnp->sfp->missing_value && 
			 tbuf[nbuf][7] != cnp->sfp->missing_value &&
			 tbuf[nbuf][11] != cnp->sfp->missing_value) {
			nbuf++;
		}
	}
	if (nbuf > 0) {
		_NHLCALLF(cttmtl,CTTMTL)
			(&nbuf,tbuf,&mbuf,&nbuf,
			 ippp,&mnop,&nppp,
			 ippe,&mnoe,&nppe,
			 rpnt,&mpnt,&npnt,&Lopn,
			 iedg,&medg,&nedg,&Loen,
			 itri,&mtri,&ntri,&Lotn);

		if (c_nerro(&err_num) != 0) {
			e_msg = c_semess(0);
			e_text = "%s: %s";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name,e_msg);
			return NhlFATAL;
		}
	}
	tmp->npnt = npnt;
	tmp->nedg = nedg;
	tmp->ntri = ntri;
	tmp->rpnt = rpnt;
	tmp->iedg = iedg;
	tmp->itri = itri;
	tmp->update_mode = TRIMESH_NOUPDATE;
	NhlFree(ippp);
	NhlFree(ippe);
	NhlFree(el);

	return NhlNOERROR;

}

#ifdef BuildTRIANGLE
		 
static NhlErrorTypes BuildDelaunayMesh 
#if	NhlNeedProto
(
	NhlCnTriMeshRendererLayerPart *dyp,
	NhlContourPlotLayer     cnl,
	NhlString entry_name
)
#else
(dyp,cnl,entry_name)
        NhlCnTriMeshRendererLayerPart *dyp;
	NhlContourPlotLayer     cnl;
	NhlString entry_name;
#endif
{
	NhlContourPlotLayerPart	*cnp = &cnl->contourplot;
	int coords_alloced = 0;
	int mnop = cnp->sfp->fast_len;
	int mpnt = mnop * Lopn;
	int mnot;
	int mnoe;
	int medg;
	int mtri;
	float *rpnt;
	int *el;
	int *iedg, *itri;
	int *ippp,*ippe;
	int npnt,nedg;
	float *rlat,*rlon,*rdat;
	double *points;
	float *dat;
	int pcount;
	float tbuf[5021][12];
	int kbuf = 173;
	int mbuf = 5021;
	int nppp = 0;
	int nppe = 0;
	int nbuf = 0;
	int ntri = 0;
	int i,j;
	int ix_offset = 0;
	int err_num;
	char *e_msg;
	char *e_text;
	struct triangulateio in,out;
	char *flags;
	
	if (dyp->npnt > 0)
		NhlFree(dyp->rpnt);
	if (dyp->nedg > 0)
		NhlFree(dyp->iedg);
	if (dyp->ntri > 0)
		NhlFree(dyp->itri);

	    
	rlat = (float*)cnp->sfp->y_arr->data;
	rlon = (float*)cnp->sfp->x_arr->data;
	rdat = (float*)cnp->sfp->d_arr->data;
	points = (double *)NhlMalloc(2 * mnop * sizeof(double));
	dat = (float *) NhlMalloc(mnop * sizeof(float));
	pcount = 0;
	if (dyp->ezmap) { /* transform points into projection space
			   throwing away points outside the map limits */ 
		for (i = 0; i < mnop; i++) {
			float xt,yt;
			j = pcount * 2;
			c_maptra(rlat[i],rlon[i],&xt,&yt);
			if (xt > 1e10 || yt > 1e10)
				continue;
			points[j] = (double)xt;
			points[j+1] = (double)yt;
			dat[pcount] = rdat[i];
			pcount++;
		}
	}
	else {
		for (i = 0; i < mnop; i++) {
			j = pcount * 2;
			points[j] = (double)rlon[i];
			points[j+1] = (double)rlat[i];
			dat[pcount] = rdat[i];
			pcount++;
		}
	}
	memset(&in,0,sizeof(struct triangulateio));
	memset(&out,0,sizeof(struct triangulateio));
	in.pointlist = points;
	in.numberofpoints = pcount;
	in.numberofpointattributes = 0;
	in.numberoftriangles = 0;

	if (cnp->verbose_triangle_info) {
		flags = "IBzV";
	}
	else {
		flags = "IBzQ";
	}
	triangulate(flags,&in,&out,NULL);

	points = out.pointlist;
	el = out.trianglelist;
	mnop = out.numberofpoints;
	mnot = out.numberoftriangles;
	mnoe = 3 * mnot;
	mpnt = mnop * Lopn;
	medg = mnoe * Loen;
	mtri = mnot * Lotn;
	rpnt = NhlMalloc(mpnt * sizeof(float));
	iedg = NhlMalloc(medg * sizeof(int));
	itri = NhlMalloc(mtri * sizeof(int));
	ippp = NhlMalloc(2 * mnop * sizeof(int));
	ippe = NhlMalloc(2 * mnoe * sizeof(int));
	
	if (! (rpnt && iedg && itri && ippp && ippe )) {
		NHLPERROR((NhlFATAL,ENOMEM,NULL));
		return NhlFATAL;
	}
	    
	for (i = 0; i < mnot; i++) {
		int *ep;
		int e0,e1,e2;
		if (nbuf >= mbuf) 
			_NHLCALLF(cttmtl,CTTMTL)
				(&kbuf,tbuf,&mbuf,&nbuf,
				 ippp,&mnop,&nppp,
				 ippe,&mnoe,&nppe,
				 rpnt,&mpnt,&npnt,&Lopn,
				 iedg,&medg,&nedg,&Loen,
				 itri,&mtri,&ntri,&Lotn);
		if (c_nerro(&err_num) != 0) {
			e_msg = c_semess(0);
			e_text = "%s: %s";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name,e_msg);
			return NhlFATAL;
		}

		ep = el + i * 3;
		e0 = *ep - ix_offset;
		e1 = *(ep+1) - ix_offset;
		e2 = *(ep+2) - ix_offset;
		tbuf[nbuf][0] = (float)points[2*e0];
		tbuf[nbuf][1] = (float)points[2*e0+1];
		tbuf[nbuf][2] = 0.0;
		tbuf[nbuf][3] = (float)dat[e0];

		tbuf[nbuf][4] = points[2*e1];
		tbuf[nbuf][5] = points[2*e1+1];
		tbuf[nbuf][6] = 0.0;
		tbuf[nbuf][7] = dat[e1];

		tbuf[nbuf][8] = points[2*e2];
		tbuf[nbuf][9] = points[2*e2+1];
		tbuf[nbuf][10] = 0.0;
		tbuf[nbuf][11] = dat[e2];
		
		if (! cnp->sfp->missing_value_set)
			nbuf++;
		else if (tbuf[nbuf][3] != cnp->sfp->missing_value && 
			 tbuf[nbuf][7] != cnp->sfp->missing_value &&
			 tbuf[nbuf][11] != cnp->sfp->missing_value) {
			nbuf++;
		}
	}
	if (nbuf > 0) {
		_NHLCALLF(cttmtl,CTTMTL)
			(&nbuf,tbuf,&mbuf,&nbuf,
			 ippp,&mnop,&nppp,
			 ippe,&mnoe,&nppe,
			 rpnt,&mpnt,&npnt,&Lopn,
			 iedg,&medg,&nedg,&Loen,
			 itri,&mtri,&ntri,&Lotn);

		if (c_nerro(&err_num) != 0) {
			e_msg = c_semess(0);
			e_text = "%s: %s";
			NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name,e_msg);
			return NhlFATAL;
		}
	}
	dyp->npnt = npnt;
	dyp->nedg = nedg;
	dyp->ntri = ntri;
	dyp->rpnt = rpnt;
	dyp->iedg = iedg;
	dyp->itri = itri;
	dyp->update_mode = TRIMESH_NOUPDATE;
	NhlFree(ippp);
	NhlFree(ippe);
	NhlFree(points);
	NhlFree(dat);

	return NhlNOERROR;

}
#endif

/*
 * Function:	CnTriMeshRendererInitialize
 *
 * Description: 
 *
 * In Args: 	class	objects layer_class
 *		req	instance record of requested values
 *		new	instance record of new object
 *		args	list of resources and values for reference
 *		num_args 	number of elements in args.
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:
 */
/*ARGSUSED*/
static NhlErrorTypes
CnTriMeshRendererInitialize
#if	NhlNeedProto
(
	NhlClass	class,
	NhlLayer	req,
	NhlLayer	new,
	_NhlArgList	args,
	int		num_args
)
#else
(class,req,new,args,num_args)
        NhlClass   class;
        NhlLayer        req;
        NhlLayer        new;
        _NhlArgList     args;
        int             num_args;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*entry_name = "CnTriMeshRendererInitialize";
	char			*e_text;
	NhlCnTriMeshRendererLayer tml = (NhlCnTriMeshRendererLayer) new;
	NhlCnTriMeshRendererLayerPart *tmp =  &tml->cntrimeshrenderer;
	NhlContourPlotLayer     cnl;
	NhlContourPlotLayerPart	*cnp;

	load_hluct_routines(False);
	
	cnl = (NhlContourPlotLayer) tml->base.parent;
	cnp =  &cnl->contourplot;

	tmp->rpnt = NULL;
	tmp->iedg = tmp->itri = NULL;
	tmp->npnt = tmp->nedg = tmp->ntri = 0;


        return ret;
}

/*
 * Function:	CnTriMeshRendererDestroy
 *
 * Description:
 *
 * In Args:	inst		instance record pointer
 *
 * Out Args:	NONE
 *
 * Return Values:	ErrorConditions
 *
 * Side Effects:	NONE
 */
static NhlErrorTypes CnTriMeshRendererDestroy
#if	NhlNeedProto
(NhlLayer inst)
#else
(inst)
NhlLayer inst;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR;
	NhlCnTriMeshRendererLayer tml = (NhlCnTriMeshRendererLayer) inst;
	NhlCnTriMeshRendererLayerPart *tmp =  &tml->cntrimeshrenderer;

	if (tmp->npnt > 0)
		NhlFree(tmp->rpnt);
	if (tmp->nedg > 0)
		NhlFree(tmp->iedg);
	if (tmp->ntri > 0)
		NhlFree(tmp->itri);

	return ret;
}

/*
 * Function:	SetCtParams
 *
 * Description:	
 *
 * In Args:	layer	ContourPlot instance
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: NONE
 */	

static NhlErrorTypes SetCtParams
#if	NhlNeedProto
(NhlContourPlotLayer cnl,NhlString entry_name)
#else
(cnl,entry_name)
        NhlContourPlotLayer cnl;
	NhlString	entry_name;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR;
	NhlContourPlotLayerPart	*cnp = &cnl->contourplot;
	NhlString		*sp;
	int			i,j;
	char			param[4];
	float			value;

	if (cnp->conpack_params == NULL)
		return NhlNOERROR;

	sp = (NhlString *) cnp->conpack_params->data;

	for (i = 0; i < cnp->conpack_params->num_elements; i++) {
		NhlBoolean matched = False;
		_cnParamType type;
		if (sp[i] != NULL && sp[i][0] != '\0') {
			value = 0.0;
			sscanf(sp[i],"%3s:%f",&param[0],&value);
			for (j = 0; j < NhlNumber(Ct_Params); j ++) {
				if (! strcmp(Ct_Params[j].name,param)) {
					matched = True;
					type = Ct_Params[j].type;
					break;
				}
			}
			if (matched && type == cnInt) {
				c_ctseti(param,(int) value);
			}
			else if (matched && type == cnFloat) {
				c_ctsetr(param,value);
			}
			else {
				char * e_text = 
              "%s: %s is invalid Conpack param or cannot be from HLU library";
				NhlPError(NhlWARNING,
					  NhlEUNKNOWN,e_text,entry_name,param);
				ret = MIN(ret,NhlWARNING);
			}
		}
	}
	return ret;
}

/*
 * Function:	SetRegionAttrs
 *
 * Description:	
 *
 * In Args:	layer	ContourPlot instance
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: Updates various internal parameters in Conpack,Plotchar,
 *		 etc.
 *		 
 */	

static void SetRegionAttrs
#if	NhlNeedProto
(
	NhlContourPlotLayer	cl,
	NhlcnRegionAttrs *reg_attrs, 
	int cpix
)
#else
(cl,reg_attrs,cpix)
	NhlContourPlotLayer	cl;
	NhlcnRegionAttrs *reg_attrs;
	int		 cpix;
#endif
{
	reg_attrs->gks_pcolor = reg_attrs->perim_color == NhlTRANSPARENT ?
                NhlTRANSPARENT : _NhlGetGksCi(cl->base.wkptr,
                                              reg_attrs->perim_color);
	reg_attrs->gks_fcolor = reg_attrs->fill_color  == NhlTRANSPARENT ?
		NhlTRANSPARENT : _NhlGetGksCi(cl->base.wkptr,
                                              reg_attrs->fill_color);
	
	c_ctseti("PAI",cpix);
	if (! reg_attrs->perim_on)
		c_ctseti("CLU",0);
	else if (cpix == -1 && cl->contourplot.missing_val_perim_grid_bound_on)
		c_ctseti("CLU",2);
	else
		c_ctseti("CLU",1);

	if (cpix == -1)
		c_ctseti("AIA",98);
	else
		c_ctseti("AIA",-1);

	return;

}

/*
 * Function:	UpdateLineAndLabelParams
 *
 * Description:	
 *
 * In Args:	layer	ContourPlot instance
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: Updates various internal parameters in Conpack,Plotchar,
 *		 etc.
 *		 
 */	

static NhlErrorTypes UpdateLineAndLabelParams
#if	NhlNeedProto
(
	NhlContourPlotLayer	cl,
	NhlBoolean	*do_lines,
	NhlBoolean	*do_labels
)
#else
(cl,do_lines,do_labels)
        NhlContourPlotLayer	cl;
	NhlBoolean	*do_lines;
	NhlBoolean	*do_labels;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR;
	NhlContourPlotLayerPart	*cnp = &(cl->contourplot);
	float			*clvp;
	int			*clup;
	int			i,j;
	float			height;

	cnp->line_lbls.text = (NhlString *) cnp->llabel_strings->data;
	if (cnp->line_lbls.mono_color) {
                if (cnp->line_lbls.color == NhlTRANSPARENT)
                        cnp->line_lbls.gks_color =  NhlTRANSPARENT;
                else
                        cnp->line_lbls.gks_color =
                                 _NhlGetGksCi(cl->base.wkptr,
                                                     cnp->line_lbls.color);
        }
	else
		cnp->line_lbls.colors =  cnp->gks_llabel_colors;
        if (cnp->line_lbls.back_color == NhlTRANSPARENT)
                cnp->line_lbls.gks_bcolor = NhlTRANSPARENT;
        else
                cnp->line_lbls.gks_bcolor =
                        _NhlGetGksCi(cl->base.wkptr,
                                     cnp->line_lbls.back_color);
	/* 
	 * If the perim color is transparent the line will not be
	 * drawn, but just in case, set the gks color to the foreground
	 */
        if (cnp->line_lbls.perim_lcolor == NhlTRANSPARENT)
                cnp->line_lbls.gks_plcolor = 
                        _NhlGetGksCi(cl->base.wkptr,NhlFOREGROUND);
        else
                cnp->line_lbls.gks_plcolor =
                        _NhlGetGksCi(cl->base.wkptr,
                                     cnp->line_lbls.perim_lcolor);

        if (cnp->high_lbls.color == NhlTRANSPARENT)
                cnp->high_lbls.gks_color =  NhlTRANSPARENT;
        else
                cnp->high_lbls.gks_color =
                         _NhlGetGksCi(cl->base.wkptr,
                                             cnp->high_lbls.color);
        if (cnp->high_lbls.back_color == NhlTRANSPARENT)
                cnp->high_lbls.gks_bcolor = NhlTRANSPARENT;
        else
                cnp->high_lbls.gks_bcolor =
                        _NhlGetGksCi(cl->base.wkptr,
                                     cnp->high_lbls.back_color);
        if (cnp->high_lbls.perim_lcolor == NhlTRANSPARENT)
                cnp->high_lbls.gks_plcolor = 
                        _NhlGetGksCi(cl->base.wkptr,NhlFOREGROUND);
        else
                cnp->high_lbls.gks_plcolor =
                        _NhlGetGksCi(cl->base.wkptr,
                                     cnp->high_lbls.perim_lcolor);


        if (cnp->low_lbls.color == NhlTRANSPARENT)
                cnp->low_lbls.gks_color =  NhlTRANSPARENT;
        else
                cnp->low_lbls.gks_color =
                         _NhlGetGksCi(cl->base.wkptr,
                                             cnp->low_lbls.color);
/* 
 * the low label background can be transparent iff (and only if) the high
 * label background is transparent, but otherwise the color can be 
 * different. If the low label background is set transparent when
 * the high label is not transparent, default to the background color.
 */
        if (cnp->low_lbls.back_color == NhlTRANSPARENT)
                cnp->low_lbls.gks_bcolor =  
			_NhlGetGksCi(cl->base.wkptr,NhlBACKGROUND);
        else
                cnp->low_lbls.gks_bcolor =
                        _NhlGetGksCi(cl->base.wkptr,
                                     cnp->low_lbls.back_color);
        if (cnp->low_lbls.perim_lcolor == NhlTRANSPARENT)
                cnp->low_lbls.gks_plcolor = 
                        _NhlGetGksCi(cl->base.wkptr,NhlFOREGROUND);
        else
                cnp->low_lbls.gks_plcolor =
                        _NhlGetGksCi(cl->base.wkptr,
                                     cnp->low_lbls.perim_lcolor);

/*	SetRegionAttrs(cl,&cnp->grid_bound,-1); */
	SetRegionAttrs(cl,&cnp->missing_val,-1);
	SetRegionAttrs(cl,&cnp->out_of_range,-2);

	*do_lines = True;
	*do_labels = False;

	gset_line_colr_ind((Gint)_NhlGetGksCi(cl->base.wkptr,0));
	gset_text_colr_ind((Gint)_NhlGetGksCi(cl->base.wkptr,0));
	gset_linewidth(1.0);

	c_ctseti("CLS",0);		/* Conpack not to select levels */
	c_ctseti("NCL",cnp->level_count); 
	clvp = (float *) cnp->levels->data;
	clup = (int *) cnp->level_flags->data;
	c_ctseti("DPU",-1); /* dash pattern use flag */

	if (cnp->mono_line_color) {
		cnp->gks_line_colors[0] = cnp->line_color == NhlTRANSPARENT ?
			NhlTRANSPARENT :
			_NhlGetGksCi(cl->base.wkptr,cnp->line_color);
	} else {
		cnp->gks_line_colors[0] =
			((int *)cnp->line_colors->data)[0] == NhlTRANSPARENT ?
			NhlTRANSPARENT :
			_NhlGetGksCi(cl->base.wkptr,
				     ((int *)cnp->line_colors->data)[0]);
	}
	if (cnp->mono_line_color && cnp->gks_line_colors[0] == NhlTRANSPARENT)
		*do_lines = False;
	if (! cnp->lines_on)
		*do_lines = False;

	for (i=0; i<cnp->level_count; i++) {
		int pai,aia,aib;
		NhlcnLevelUseMode flag;
		NhlBoolean blank = True;
		char *cp;

		pai = i+1;
		aib = NhlcnAREAID_OFFSET+i;
		aia = NhlcnAREAID_OFFSET+i+1;
		c_ctseti("PAI",pai);
		c_ctsetr("CLV",(float)clvp[i]);
		c_ctseti("AIB",aib);
		c_ctseti("AIA",aia);

		flag = cnp->mono_level_flag ? 
			cnp->level_flag : (NhlcnLevelUseMode) clup[i];

		if (! *do_lines) {
			switch (flag) {
			case NhlNOLINE:
			case NhlLINEONLY:
			default:
				flag = NhlNOLINE;
				break;
			case NhlLABELONLY:
			case NhlLINEANDLABEL:
				flag = NhlLABELONLY;
				break;
			}
		}

#if 0
		printf("pai %d,clv %f,aib %d,aia %d\n",pai,clvp[i],aib,aia);
#endif
		cp = ((NhlString*)cnp->line_lbls.text)[i];
		if (cp) {
			for (j = 0; j < strlen(cp); j++) {
				if (! isgraph(cp[j]))
					continue;
				blank = False;
			}
		}
		if (blank) {
			switch (flag) {
			case NhlNOLINE:
			case NhlLABELONLY:
			default:
				flag = NhlNOLINE;
				break;
			case NhlLINEONLY:
			case NhlLINEANDLABEL:
				flag = NhlLINEONLY;
				break;
			}
		}
		c_ctseti("CLU",flag);
		c_ctsetc("LLT",cp);
	}
	if (cnp->level_selection_mode != NhlEXPLICITLEVELS)
		c_ctsetr("CIU",(float)cnp->level_spacing);
 
/* Set up for labels */

/* Conpack not to render the Informational label */
	c_ctsetc("ILT"," ");

/* Line labels */
	if (! cnp->line_lbls.on) {
		c_ctseti("LLP",0); 
	}
	else if (cnp->llabel_placement == NhlCONSTANT) {
		*do_labels = True;
		c_ctseti("LLP",1);
#if 0
		c_ctsetr("DPS",
			 (float)(cnp->line_lbls.real_height / cl->view.width));
		c_ctsetr("DPV",(float).015);
#endif
#if 0
		c_ctsetr("RC3",(float)0.0);
		c_ctseti("LLP",2);
		if (cnp->line_lbls.angle < 0.0) 
			c_ctseti("LLO",1); /* angle to contour direction */
		else {
			c_ctseti("LLO",0); /* fixed angle  */
			c_ctsetr("LLA",(float)cnp->line_lbls.angle);
		}
#endif

	}
	else if (cnp->llabel_placement == NhlRANDOMIZED) {
		*do_labels = True;
		c_ctseti("LLP",2);
		if (cnp->line_lbls.angle < 0.0) 
			c_ctseti("LLO",1); /* angle to contour direction */
		else {
			c_ctseti("LLO",0); /* fixed angle  */
			c_ctsetr("LLA",(float)cnp->line_lbls.angle);
		}
		if (cnp->llabel_density > 0.0) {
			float rc1 = 0.25 / cnp->llabel_density;
			float rc2 = 0.25 / cnp->llabel_density;
			float rc3 = 0.05 / cnp->llabel_density;	
			c_ctsetr("RC1",rc1);
			c_ctsetr("RC2",rc2);
			c_ctsetr("RC3",rc3);
		}
	}
	else {
		*do_labels = True;
		c_ctseti("LLP",3);
		if (cnp->line_lbls.angle < 0.0) 
			c_ctseti("LLO",1); /* angle to contour direction */
		else {
			c_ctseti("LLO",0); /* fixed angle  */
			c_ctsetr("LLA",(float)cnp->line_lbls.angle);
		}
		if (cnp->llabel_density > 0.0) {
			float pc1 = 1.0;
			float pc2 = 5.0;
			float pc3 = 60.0;
                        float pc4 = 0.05;
			float pc5 = 0.15;
                        float pc6 = 0.30;
			float pw1 = 2.0;
                        float pw2 = 0.0;
                        float pw3 = 1.0;
                        float pw4 = 1.0;

			pc6 /= cnp->llabel_density;
			pc3 = pc3 + 30 * (cnp->llabel_density - 1);
			pc1 *= cnp->llabel_density;
			pc5 *= cnp->llabel_density;

			c_ctsetr("PC1",pc1);
			c_ctsetr("PC2",pc2);
			c_ctsetr("PC3",pc3);
			c_ctsetr("PC4",pc4);
			c_ctsetr("PC5",pc5);
			c_ctsetr("PC6",pc6);
			c_ctsetr("PW1",pw1);
			c_ctsetr("PW2",pw2);
			c_ctsetr("PW3",pw3);
			c_ctsetr("PW4",pw4);
		}
	}

	if (*do_labels) {
		height = cnp->line_lbls.real_height / cl->view.width;
		c_ctsetr("LLS",(float)height);
		c_ctsetr("LLW", 
			 (float) (height * cnp->line_lbls.perim_space));
		if (cnp->line_lbls.back_color == NhlTRANSPARENT) {
			if (cnp->line_lbls.perim_lcolor == NhlTRANSPARENT ||
			    ! cnp->line_lbls.perim_on) 
				c_ctseti("LLB",0); 
			else
				c_ctseti("LLB",1);
		}
		else {
			c_ctseti("LBC",cnp->line_lbls.back_color);
			if (cnp->line_lbls.perim_lcolor == NhlTRANSPARENT ||
			    ! cnp->line_lbls.perim_on) 
				c_ctseti("LLB",2);
			else
				c_ctseti("LLB",3);
		}
	}

/*
 * In order to allow user control of the high and low attributes 
 * individually set the appropriate part of the flag on if either 
 * the high or the low is on. Further distinguishing between high and low
 * occurs in the low level routine cpchhl_
 */
	if (! cnp->high_lbls.on)
		c_ctsetc("HIT"," ");
	else 
		c_ctsetc("HIT",(NhlString)cnp->high_lbls.text);

	if (! cnp->low_lbls.on)
		c_ctsetc("LOT"," ");
	else
		c_ctsetc("LOT",(NhlString)cnp->low_lbls.text);

/*
 * Due to the way Conpack works it is not possible to have different text
 * sizes, white space, background and perim on/off settings for the high
 * and low labels. The high labels take precedence, so set up accordingly.
 * Background and perim can have different colors, except that if the
 * high background or perim is transparent (emulated by turning these
 * features off) then the corresponding low feature must also become 
 * transparent. 
 * This means that 
 * cnLowLabelFontHeightF
 * cnLowLabelAngleF
 * cnLowLabelPerimSpacingF
 * cnLowLabelPerimOn
 * are always ignored.
 * cnLowLabelBackgroundColor and cnLowLabelPerimColor can be set independently
 * of the corresponding HighLabel resource if that resource is not set to 
 * transparent. However, if the low label resource is set to transparent in
 * this case, it will be coerced to transparent.
 * 
 * It could be possible to set the low label font height independently of
 * the high label font height, but it will require a more sophisticated
 * method than the LowLabelFactor which is commented out below. 
 */

	if (cnp->high_lbls.on || cnp->low_lbls.on) {
		*do_labels = True;
		height = cnp->high_lbls.real_height / cl->view.width;
#if 0
		LowLabelFactor =  cnp->high_lbls.real_height /
			cnp->low_lbls.real_height;
#endif
		c_ctsetr("HLS",(float)height);
		c_ctsetr("HLW",(float)(cnp->high_lbls.perim_space  * height));
		c_ctsetr("HLA",(float)cnp->high_lbls.angle);
		c_ctseti("HLO", (int) cnp->high_low_overlap);

		if (cnp->high_lbls.back_color == NhlTRANSPARENT) {
			if (cnp->high_lbls.perim_lcolor == NhlTRANSPARENT ||
			    ! cnp->high_lbls.perim_on) 
				c_ctseti("HLB",0); 
			else
				c_ctseti("HLB",1);
		}
		else {
			if (cnp->high_lbls.perim_lcolor == NhlTRANSPARENT ||
			    ! cnp->high_lbls.perim_on)
				c_ctseti("HLB",2);
			else
				c_ctseti("HLB",3);
		}
	}

	c_pcsetc("FC",":");
	return ret;

}

/*
 * Function:	UpdateFillInfo
 *
 * Description:	
 *
 * In Args:	layer	ContourPlot instance
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: sets the do_fill Boolean flag depending on whether 
 *		 fill is to be done.
 *		 
 */	

static NhlErrorTypes UpdateFillInfo
#if	NhlNeedProto
(
	NhlContourPlotLayer	cl,
	NhlBoolean	*do_fill
)
#else
(cl,do_fill)
        NhlContourPlotLayer	cl;
	NhlBoolean	*do_fill;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR;
	NhlContourPlotLayerPart	*cnp = &(cl->contourplot);
#if 0
 	NhlBoolean	color_fill, pattern_fill;

	color_fill = (cnp->mono_fill_color && 
		      cnp->fill_color == NhlTRANSPARENT) ? False : True;
	pattern_fill = (cnp->mono_fill_pattern && 
			cnp->fill_pattern == NhlHOLLOWFILL) ? False : True;

	if (color_fill &&  pattern_fill &&  
	    (cnp->fill_on || cnp->raster_mode_on)) {
	}
#endif 
/*
 * Since the missing value fill resources are not supposed to be affected
 * by the mono flags, you cannot optimize the fill away if mono fill color is
 * true and fill color is transparent or mono fill pattern is true and the
 * fill pattern is hollow. So just keep it simple.
 * 
 */
	if (cnp->fill_on) {
		*do_fill = True;
		return ret;
	}

	*do_fill = False;
	return ret;
}


/*
 * Function:	ContourAbortDraw
 *
 * Description:	cleans up if a fatal error occurs while drawing
 *
 * In Args:	layer	ContourPlot instance
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: NONE
 */	

static void ContourAbortDraw
#if	NhlNeedProto
(
	NhlContourPlotLayer	cnl
)
#else
(cnl)
	NhlContourPlotLayer	cnl;
#endif
{
	NhlContourPlotLayerPart	*cnp = &cnl->contourplot;
	NhlTransformLayerPart	*tfp = &(cnl->trans);
	char *e_text;

	Cnp = NULL;
	Cnl = NULL;

	if (cnp->aws != NULL) {
		_NhlIdleWorkspace(cnp->aws);
		cnp->aws = NULL;
	}
	if (cnp->cws != NULL) {
		_NhlIdleWorkspace(cnp->cws);
		cnp->cws = NULL;
	}
	if (cnp->fws != NULL) {
		_NhlIdleWorkspace(cnp->fws);
		cnp->fws = NULL;
	}
	if (cnp->iws != NULL) {
		_NhlIdleWorkspace(cnp->iws);
		cnp->iws = NULL;
	}

	if (cnl->view.use_segments && cnp->current_trans_dat) {
		_NhlEndSegment(cnp->current_trans_dat);
		cnp->current_trans_dat = NULL;
	}

	if (cnp->wk_active) {
		_NhlDeactivateWorkstation(cnl->base.wkptr);
		cnp->wk_active = False;
	}

	if (cnp->low_level_log_on) {
		NhlVASetValues(tfp->trans_obj->base.id,
			       NhlNtrLowLevelLogOn,False,NULL);
                cnp->low_level_log_on = False;
        }

	e_text = "%s: draw error";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,"ContourPlotDraw");
}

/*
 * Function:	AddDataBoundToAreamap
 *
 * Description:	
 *
 * In Args:	
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: 
 *		 
 */	

static NhlErrorTypes AddDataBoundToAreamap
#if	NhlNeedProto
(
	NhlContourPlotLayer	cl,
	NhlString	entry_name
)
#else
(cl,entry_name)
	NhlContourPlotLayer	cl;
	NhlString	entry_name;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR;
	char			*e_text;
	NhlContourPlotLayerPart	*cnp = 
		(NhlContourPlotLayerPart *) &cl->contourplot;
	int			i;
	int			status;
	NhlBoolean		ezmap = False;
	int			xrev,yrev;
	float			xa[5],ya[5];

#define _cnBBOXGID 3
#if 0
#define _cnMAPBOUNDINC	3700
#endif
#define _cnMAPBOUNDINC	100

	if (cnp->trans_obj->base.layer_class->base_class.class_name ==
	    NhlmapTransObjClass->base_class.class_name) {
		ezmap = True;
	}

#if 0
	gset_linewidth(4.0);
	gset_line_colr_ind(30);
	c_arseti("RC(1)",1);
	c_arseti("RC(3)",2);
#endif
	c_arseti("RC",1);
	if (! ezmap) {
		float twlx,twrx,twby,twuy;
		float gwlx,gwrx,gwby,gwuy;
		float txmin,txmax,tymin,tymax;
		float gxmin,gxmax,gymin,gymax;
		NhlBoolean lbox, rbox, bbox, tbox;
		float	   xeps,yeps;

		ret = NhlVAGetValues(cnp->trans_obj->base.id,
				     NhlNtrXMinF,&txmin,
				     NhlNtrXMaxF,&txmax,
				     NhlNtrYMinF,&tymin,
				     NhlNtrYMaxF,&tymax,
				     NULL);

		_NhlDataToWin(cnp->trans_obj,&txmin,&tymin,
			      1,&twlx,&twby,&status,
			      NULL,NULL);
		if (status) {
			e_text = "%s: data boundary is out of range";
			NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
			ret = MIN(ret,NhlWARNING);
			return ret;
		}

		_NhlDataToWin(cnp->trans_obj,&txmax,&tymax,
			      1,&twrx,&twuy,&status,
			      NULL,NULL);
		if (status) {
			e_text = "%s: data boundary is out of range";
			NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
			ret = MIN(ret,NhlWARNING);
			return ret;
		}

		gxmin = MAX(txmin,cnp->xlb);
		gxmax = MIN(txmax,cnp->xub);
		gymin = MAX(tymin,cnp->ylb);
		gymax = MIN(tymax,cnp->yub);

		_NhlDataToWin(cnp->trans_obj,&gxmin,&gymin,
			      1,&gwlx,&gwby,&status,
			      NULL,NULL);
		if (status) {
			e_text = "%s: data boundary is out of range";
			NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
			ret = MIN(ret,NhlWARNING);
			return ret;
		}

		_NhlDataToWin(cnp->trans_obj,&gxmax,&gymax,
			      1,&gwrx,&gwuy,&status,
			      NULL,NULL);
		if (status) {
			e_text = "%s: data boundary is out of range";
			NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,entry_name);
			ret = MIN(ret,NhlWARNING);
			return ret;
		}

		xrev = twlx > twrx;
		yrev = twby > twuy;
/*
 * added a hack to prevent fill dropout in certain cases, where because
 * of floating point precision issues in the mapping routines, contour
 * lines were being removed because they didn't quite touch the viewport
 * edge. Now a very thin rectangle is drawn just to the inside of each
 * viewport edge in this situation.
 */
		xeps = 1e-5 * fabs(twrx-twlx);
		yeps = 1e-5 * fabs(twuy-twby);

		if (! xrev) {
			if (gwlx >= twlx && gwlx - twlx < xeps)
				gwlx = twlx + xeps;
			if (gwrx <= twrx && twrx - gwrx < xeps)
				gwrx = twrx - xeps;
			lbox = gwlx > twlx;
			rbox = gwrx < twrx;
		}
		else {
			if (gwrx >= twrx && gwrx - twrx < xeps)
				gwrx = twrx + xeps;
			if (gwlx <= twlx && twlx - gwlx < xeps)
				gwlx = twlx - xeps;
			lbox = gwlx < twlx;
			rbox = gwrx > twrx;
		}
		if (! yrev) {
			if (gwby >= twby && gwby - twby < xeps)
				gwby = twby + yeps;
			if (gwuy <= twuy && twuy - gwuy < yeps)
				gwuy = twuy - yeps;
			bbox = gwby > twby;
			tbox = gwuy < twuy;
		}
		else {
			if (gwuy >= twuy && gwuy - twuy < yeps)
				gwuy = twuy + yeps;
			if (gwby <= twby && twby - gwby < yeps)
				gwby = twby - yeps;
			bbox = gwby > twby;
			tbox = gwuy < twuy;
		}

/*
 * This code from 'added a hack' above to the end of 'if (! ezmap)' below was not working properly
 * for log plots where the lower values get stretched by log scaling. It sometime resulted in 
 * boxes that were big enough to be visible. The solution is to start with NDC values and convert them
 * to data values using the current transformation. That is what the code below does. If it succeeds
 * (st is 0) then the inside coordinates of the skinny boxes are replaced. (Are there situations where
 * it would fail? -- it seems safest to allow for that possibility.
 */
		{
			float xn[4],yn[4];
			float xe,ye;
			int st;
			float oor;
			float x,y,w,h;
			x = cl->view.x;
			y = cl->view.y;
			w = cl->view.width;
			h = cl->view.height;

			xe = 1e-5 * w;
			ye = 1e-5 * h;
			xn[0] = x + xe;
			xn[1] = x + w - xe;
			xn[2] = xn[1];
			xn[3] = xn[0];
			yn[0] = y - h + ye;
			yn[1] = yn[0];
			yn[2] = y - ye;
			yn[3] = yn[2];

			NhlNDCToData(cl->base.id,xn,yn,4,xn,yn,NULL,NULL,&st,&oor);
			if (! st) {
				_NhlDataToWin(cnp->trans_obj,xn,yn,
					      4,xn,yn,&st,NULL,NULL);
			}
			if (! st) {
				gwlx = xn[0];
				gwrx = xn[1];
				gwby = yn[0];
				gwuy = yn[2];
			}
		}

		if (lbox) {
				
			xa[0] = xa[1] = xa[4] = twlx;
			xa[2] = xa[3] = gwlx;
			ya[0] = ya[3] = ya[4] = twuy;
			ya[1] = ya[2] = twby;

			if (! (xrev || yrev) || (xrev && yrev)) 
				_NhlAredam(cnp->aws,xa,ya,
					   5,_cnBBOXGID,9999,0,entry_name);
			else
				_NhlAredam(cnp->aws,xa,ya,
					   5,_cnBBOXGID,0,9999,entry_name);
		}
		if (rbox) {
			xa[0] = xa[1] = xa[4] = gwrx;
			xa[2] = xa[3] = twrx;
			ya[0] = ya[3] = ya[4] = twuy;
			ya[1] = ya[2] = twby;
			if (! (xrev || yrev) || (xrev && yrev)) 
				_NhlAredam(cnp->aws,xa,ya,
					   5,_cnBBOXGID,9999,0,entry_name);
			else
				_NhlAredam(cnp->aws,xa,ya,
					   5,_cnBBOXGID,0,9999,entry_name);
		}
		if (bbox) {
			xa[0] = xa[1] = xa[4] = gwlx;
			xa[2] = xa[3] = gwrx;
			ya[0] = ya[3] = ya[4] = gwby;
			ya[1] = ya[2] = twby;
			if (! (xrev || yrev) || (xrev && yrev)) 
				_NhlAredam(cnp->aws,xa,ya,
					   5,_cnBBOXGID,9999,0,entry_name);
			else
				_NhlAredam(cnp->aws,xa,ya,
					   5,_cnBBOXGID,0,9999,entry_name);
		}
		if (tbox) {
			xa[0] = xa[1] = xa[4] = gwlx;
			xa[2] = xa[3] = gwrx;
			ya[0] = ya[3] = ya[4] = twuy;
			ya[1] = ya[2] = gwuy;
			if (! (xrev || yrev) || (xrev && yrev)) 
				_NhlAredam(cnp->aws,xa,ya,
					   5,_cnBBOXGID,9999,0,entry_name);
			else
				_NhlAredam(cnp->aws,xa,ya,
					   5,_cnBBOXGID,0,9999,entry_name);
		}
	}
	else {
		NhlBoolean	started = False;
		float		xinc,yinc; 
		int		j;
		char		cval[4];

		if (! cnp->fix_fill_bleed)
			return NhlNOERROR;
		xa[0] = xa[3] = xa[4] = cnp->xlb;
		xa[1] = xa[2] = cnp->xub;
		ya[0] = ya[1] = ya[4] = cnp->ylb;
		ya[2] = ya[3] = cnp->yub;

		for (i=0;  i < 4; i++) {
			xinc = (xa[i+1] - xa[i]) / _cnMAPBOUNDINC;
			yinc = (ya[i+1] - ya[i]) / _cnMAPBOUNDINC;
			if (! started) {
				_NhlMapita(cnp->aws,ya[i],xa[i],
					   0,3,0,-1,entry_name);
				started = True;
			}
			for (j = 0; j < _cnMAPBOUNDINC + 1; j++) {
				_NhlMapita(cnp->aws,ya[i]+j*yinc,xa[i]+j*xinc,
					   1,3,0,-1,entry_name);
			}
		}
		_NhlMapiqa(cnp->aws,3,0,-1,entry_name);

		c_mpgetc("OU",cval,3);
		c_mpsetc("OU","NO");
		c_mpseti("G2",3);
		c_mpseti("VS",1);
		_NhlMapbla(cnp->aws,entry_name);
		c_mpsetc("OU",cval);
	}
	return NhlNOERROR;
}

/*
 * Function:	cnInitAreamap
 *
 * Description:	
 *
 * In Args:	
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: NONE
 */	

static NhlErrorTypes cnInitAreamap
#if	NhlNeedProto
(
	NhlContourPlotLayer	cnl,
	NhlString	entry_name
)
#else
(cnl,entry_name)
        NhlContourPlotLayer cnl;
	NhlString	entry_name;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*e_text;
	NhlContourPlotLayerPart	*cnp = &(cnl->contourplot);

	if (cnp->aws_id < 1) {
		cnp->aws_id = 
			_NhlNewWorkspace(NhlwsAREAMAP,
					 NhlwsNONE,200000*sizeof(int));
		if (cnp->aws_id < 1) 
			return MIN(ret,(NhlErrorTypes)cnp->aws_id);
	}
	if ((cnp->aws = _NhlUseWorkspace(cnp->aws_id)) == NULL) {
		e_text = 
			"%s: error reserving label area map workspace";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(ret);
	}

#if 0
	c_arseti("lc",(int) (cnp->amap_crange * 
		 MIN(cnl->view.width,cnl->view.height)));
#endif
	subret = _NhlArinam(cnp->aws,entry_name);
	if ((ret = MIN(subret,ret)) < NhlWARNING) return ret;

	return ret;
}

static float Xsoff,Xeoff,Ysoff,Yeoff;

/*
 * Function:	cnInitCellArray
 *
 * Description:	
 *
 * In Args:	
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: NONE
 */	

static NhlErrorTypes cnInitCellArray
#if	NhlNeedProto
(
	NhlContourPlotLayer	cnl,
	int		*msize,
	int		*nsize,
	NhlBoundingBox	*bbox,
	float		*min_cell_size,
	NhlString	entry_name
)
#else
(cnl,entry_name)
        NhlContourPlotLayer cnl;
	NhlString	entry_name;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*e_text;
	NhlContourPlotLayerPart	*cnp = &(cnl->contourplot);
        int dunits,dwidth,dheight;
        int max_msize, max_nsize;
        NhlBoolean xlinear,ylinear;
	int mcount,ncount;

	c_ctseti("CAF", -1);
	xlinear = True;
        ylinear = True;
	if (cnp->sfp->d_arr->num_dimensions == 1) {
		/* x and y cells are meaningless, but we need something */
		mcount = ncount = sqrt(cnp->sfp->fast_len);
	} else {
		mcount = cnp->sfp->fast_len;
		ncount = cnp->sfp->slow_len;
	}
	subret = CnGetDataBound(cnl,bbox,&xlinear,&ylinear,
			      &mcount,&ncount,&Xsoff,&Xeoff,&Ysoff,&Yeoff,entry_name);
	if ((ret = MIN(ret,subret)) < NhlWARNING) return ret;
        
        max_msize = (int) ((bbox->r - bbox->l) / cnp->min_cell_size);
        max_nsize = (int) ((bbox->t - bbox->b) / cnp->min_cell_size);

        subret = NhlVAGetValues(cnl->base.wkptr->base.id,
                                NhlNwkVSWidthDevUnits,&dunits,
                                NULL);
	if ((ret = MIN(ret,subret)) < NhlWARNING)
		return ret;

        dwidth = dunits * (bbox->r - bbox->l);
        dheight = dunits * (bbox->t - bbox->b);
	*min_cell_size = MAX(1.0/dunits,cnp->min_cell_size);

        if (cnp->sticky_cell_size_set) {
                if ((bbox->r - bbox->l) / cnp->cell_size <= 1.0 ||
                    (bbox->t - bbox->b) / cnp->cell_size <= 1.0) {
                        e_text = 
                                "%s: invalid value for %s: defaulting";
                        NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,
                                  entry_name,NhlNcnRasterCellSizeF);
                        ret = NhlWARNING;
                        cnp->sticky_cell_size_set = False;
                }
        }
                
	if (cnp->sticky_cell_size_set) {
		*msize = (int) ((bbox->r - bbox->l) / cnp->cell_size + 0.5);
		*nsize = (int) ((bbox->t - bbox->b) / cnp->cell_size + 0.5);
	}
        else if (cnp->raster_sample_factor <= 0.0) {
                *msize = mcount;
                *nsize = ncount;
        }
        else if (cnp->raster_smoothing_on) {
                *msize = dwidth * cnp->raster_sample_factor;
                *nsize = dheight * cnp->raster_sample_factor;
        }
        else {
                if (! xlinear)
                        *msize = dwidth * cnp->raster_sample_factor;
                else
                        *msize = MIN(dwidth,mcount)
				* cnp->raster_sample_factor;
                if (! ylinear)
                        *nsize = dheight * cnp->raster_sample_factor;
                else
                        *nsize = MIN(dheight,ncount)
                                * cnp->raster_sample_factor;
        }
        
        if (!cnp->sticky_cell_size_set && cnp->raster_sample_factor > 0.0) {
                *msize = MIN(*msize,max_msize);
                *nsize = MIN(*nsize,max_nsize);
                cnp->cell_size = (bbox->r - bbox->l) / (float) *msize;
        }
	
	if (cnp->cws_id < 1) {
		cnp->cws_id = 
			_NhlNewWorkspace(NhlwsOTHER,NhlwsNONE,
					 (*msize * *nsize) * sizeof(int));
		if (cnp->cws_id < 1) 
			return MIN(ret,(NhlErrorTypes)cnp->cws_id);
	}
	if ((cnp->cws = _NhlUseWorkspace(cnp->cws_id)) == NULL) {
		e_text = 
			"%s: error reserving cell array workspace";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(NhlFATAL);
	}
	return ret;
}

/*
 * Function:	cnInitDataArray
 *
 * Description:	
 *
 * In Args:	
 *
 * Out Args:	NONE
 *
 * Return Values: Error Conditions
 *
 * Side Effects: NONE
 */	

static NhlErrorTypes cnInitDataArray
#if	NhlNeedProto
(
	NhlContourPlotLayer	cnl,
	int		*msize,
	int		*nsize,
	NhlBoundingBox	*bbox,
	float		*min_cell_size,
	NhlString	entry_name
)
#else
(cnl,entry_name)
        NhlContourPlotLayer cnl;
	NhlString	entry_name;
#endif
{
	NhlErrorTypes		ret = NhlNOERROR, subret = NhlNOERROR;
	char			*e_text;
	NhlContourPlotLayerPart	*cnp = &(cnl->contourplot);
        int dunits,dwidth,dheight;
        int max_msize, max_nsize;
        NhlBoolean xlinear,ylinear;
	int mcount,ncount;

	c_ctseti("CAF", -1);
	xlinear = True;
        ylinear = True;
	if (cnp->sfp->d_arr->num_dimensions == 1) {
		/* x and y cells are meaningless, but we need something */
		mcount = ncount = sqrt(cnp->sfp->fast_len);
	} else {
		mcount = cnp->sfp->fast_len;
		ncount = cnp->sfp->slow_len;
	}
	subret = CnGetDataBound(cnl,bbox,&xlinear,&ylinear,
			      &mcount,&ncount,&Xsoff,&Xeoff,&Ysoff,&Yeoff,entry_name);
	if ((ret = MIN(ret,subret)) < NhlWARNING) return ret;
        
        max_msize = (int) ((bbox->r - bbox->l) / cnp->min_cell_size);
        max_nsize = (int) ((bbox->t - bbox->b) / cnp->min_cell_size);

        subret = NhlVAGetValues(cnl->base.wkptr->base.id,
                                NhlNwkVSWidthDevUnits,&dunits,
                                NULL);
	if ((ret = MIN(ret,subret)) < NhlWARNING)
		return ret;

        dwidth = dunits * (bbox->r - bbox->l);
        dheight = dunits * (bbox->t - bbox->b);
	*min_cell_size = MAX(1.0/dunits,cnp->min_cell_size);

        if (cnp->sticky_cell_size_set) {
                if ((bbox->r - bbox->l) / cnp->cell_size <= 1.0 ||
                    (bbox->t - bbox->b) / cnp->cell_size <= 1.0) {
                        e_text = 
                                "%s: invalid value for %s: defaulting";
                        NhlPError(NhlWARNING,NhlEUNKNOWN,e_text,
                                  entry_name,NhlNcnRasterCellSizeF);
                        ret = NhlWARNING;
                        cnp->sticky_cell_size_set = False;
                }
        }
                
	if (cnp->sticky_cell_size_set) {
		*msize = (int) ((bbox->r - bbox->l) / cnp->cell_size + 0.5);
		*nsize = (int) ((bbox->t - bbox->b) / cnp->cell_size + 0.5);
	}
        else if (cnp->raster_sample_factor <= 0.0) {
                *msize = mcount;
                *nsize = ncount;
        }
        else if (cnp->raster_smoothing_on) {
                *msize = dwidth * cnp->raster_sample_factor;
                *nsize = dheight * cnp->raster_sample_factor;
        }
        else {
                if (! xlinear)
                        *msize = dwidth * cnp->raster_sample_factor;
                else
                        *msize = MIN(dwidth,mcount)
				* cnp->raster_sample_factor;
                if (! ylinear)
                        *nsize = dheight * cnp->raster_sample_factor;
                else
                        *nsize = MIN(dheight,ncount)
                                * cnp->raster_sample_factor;
        }
        
        if (!cnp->sticky_cell_size_set && cnp->raster_sample_factor > 0.0) {
                *msize = MIN(*msize,max_msize);
                *nsize = MIN(*nsize,max_nsize);
                cnp->cell_size = (bbox->r - bbox->l) / (float) *msize;
        }
	return ret;
}

static NhlErrorTypes InitMesh
#if	NhlNeedProto
(
        NhlContourPlotLayer     cnl,
	NhlCnTriMeshRendererLayerPart	  *tmp,
	NhlString		entry_name
        )
#else
(cnl,tmp)
        NhlContourPlotLayer     cnl;
	NhlCnTriMeshRendererLayerPart	  *tmp;
	NhlString		entry_name;
#endif
{
	NhlContourPlotLayerPart 	  *cnp = &cnl->contourplot;
        NhlErrorTypes ret = NhlNOERROR;

	if (tmp->update_mode > TRIMESH_NOUPDATE || ! tmp->npnt) {
		if (cnp->sfp->grid_type == NhlMESHGRID) {
			if (cnp->sfp->element_nodes) {
				ret = BuildNativeMesh(tmp,cnl,entry_name);
			}
			else if (cnp->sfp->x_cell_bounds && 
				 cnp->sfp->y_cell_bounds) {
				ret = BuildNativeMeshFromBounds
					(tmp,cnl,entry_name);
			}				
			else {
#ifdef BuildTRIANGLE
				ret = BuildDelaunayMesh(tmp,cnl,entry_name);
#else
				NhlPError(NhlFATAL,NhlEUNKNOWN,
					  "Cannot create triangular mesh: supply additional resources or build with Triangle package");
				return NhlFATAL;
#endif
			}
		}
		else {
			ret = BuildTriangularMesh(tmp,cnl,entry_name);
		}
	}

	_NhlCtmesh(tmp->rpnt,tmp->npnt,Lopn,
		   tmp->iedg,tmp->nedg,Loen,
		   tmp->itri,tmp->ntri,Lotn,
		   cnp->fws,cnp->iws,entry_name);

	return ret;
}

static NhlErrorTypes CnTriMeshRender
#if	NhlNeedProto
(
	NhlLayer		instance,
        NhlContourPlotLayer     cnl,
	NhlDrawOrder            order,
	NhlString		entry_name
        )
#else
(instance,cnl,order,entry_name)
	NhlLayer		instance;
        NhlContourPlotLayer     cnl;
	NhlDrawOrder            order;
	NhlString		entry_name;
#endif
{
        NhlCnTriMeshRendererLayer tml = (NhlCnTriMeshRendererLayer) instance;
	NhlCnTriMeshRendererLayerPart	  *tmp = &tml->cntrimeshrenderer;
	NhlContourPlotLayerPart 	  *cnp = &cnl->contourplot;
	NhlTransformLayerPart 		  *tfp = &cnl->trans;
	NhlString e_text;
        NhlErrorTypes ret = NhlNOERROR,subret = NhlNOERROR;
	float rwrk[10000];
	int iwrk[1000];
	int lrwk = 10000, liwk = 1000; 
	int mesh_inited = 0;
        Gint            err_ind;
        Gclip           clip_ind_rect;

	Cnl = cnl;
	Cnp = cnp;
	Tmp = tmp;


	ginq_clip(&err_ind,&clip_ind_rect);
        gset_clip_ind(GIND_CLIP);
	
	c_ctrset();

	SetCtParams(cnl,entry_name);
/*
 * Only set the ORV parameter if overlaying on EZMAP. It can cause
 * problems otherwise. (Not sure yet whether it is needed in some cases
 * though, and perhaps not needed in certain Ezmap cases.
 */
	if (cnp->trans_obj->base.layer_class->base_class.class_name ==
	    NhlmapTransObjClass->base_class.class_name) {
		NhlVAGetValues(cnp->trans_obj->base.id, 
			       NhlNtrOutOfRangeF, &cnp->out_of_range_val,
			       NULL);
		tmp->ezmap = 1;
		c_ctsetr("ORV",cnp->out_of_range_val);
		if (cnp->sfp->d_arr->num_dimensions == 1 &&
		    ! (cnp->sfp->element_nodes ||
		       (cnp->sfp->x_cell_bounds && cnp->sfp->y_cell_bounds))) {
			c_ctseti("MAP",Nhlcn1DMESHMAPVAL);
			tmp->update_mode = TRIMESH_NEWMESH;
		}
		else {
			c_ctseti("MAP",NhlcnMAPVAL);
		}
	}
	else {
		tmp->ezmap = 0;
		c_ctseti("MAP",NhlcnTRIMESHMAPVAL);
	}

	c_ctseti("WSO", 3);		/* error recovery on */
	c_ctseti("NVS",0);		/* no vertical strips */
	c_ctseti("HLE",1);              /* search for equal high/lows */
        c_ctseti("SET",0);
        c_ctseti("RWC",500);
        c_ctseti("RWG",1500);

	c_ctsetr("PIT",MAX(0.0,cnp->max_point_distance));
	
        if (cnp->smoothing_on) {
                c_ctsetr("T2D",cnp->smoothing_tension);
                c_ctsetr("SSL",cnp->smoothing_distance);
        }
        else {
                c_ctsetr("T2D",(float)0.0);
        }
	gset_fill_colr_ind((Gint)_NhlGetGksCi(cnl->base.wkptr,0));

	subret = UpdateLineAndLabelParams(cnl,&cnp->do_lines,&cnp->do_labels);
	if ((ret = MIN(subret,ret)) < NhlWARNING) {
		ContourAbortDraw(cnl);
		gset_clip_ind(clip_ind_rect.clip_ind);
		return ret;
	}

	subret = UpdateFillInfo(cnl, &cnp->do_fill);
	if ((ret = MIN(subret,ret)) < NhlWARNING) {
		ContourAbortDraw(cnl);
		gset_clip_ind(clip_ind_rect.clip_ind);
		return ret;
	}


/* Retrieve workspace pointers */

	if ((cnp->fws = _NhlUseWorkspace(cnp->fws_id)) == NULL) {
		e_text = "%s: error reserving float workspace";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		ContourAbortDraw(cnl);
		gset_clip_ind(clip_ind_rect.clip_ind);
		return(ret);
	}
	if ((cnp->iws = _NhlUseWorkspace(cnp->iws_id)) == NULL) {
		e_text = "%s: error reserving integer workspace";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		ContourAbortDraw(cnl);
		gset_clip_ind(clip_ind_rect.clip_ind);
		return(ret);
	}

	/* Draw the contours */

		 
	if ((ret = MIN(subret,ret)) < NhlWARNING) {
		ContourAbortDraw(cnl);
		gset_clip_ind(clip_ind_rect.clip_ind);
		return ret;
	}
#if 0
	{ /* for debugging */
		float flx,frx,fby,fuy,wlx,wrx,wby,wuy; int ll;
		c_getset(&flx,&frx,&fby,&fuy,&wlx,&wrx,&wby,&wuy,&ll);
		printf("getset - %f,%f,%f,%f,%f,%f,%f,%f\n",
		       flx,frx,fby,fuy,wlx,wrx,wby,wuy); 
  	}
#endif


	if (cnp->output_gridded_data) {
		int msize,nsize;
		NhlBoundingBox bbox;
		float min_cell_size;

		if (! mesh_inited) {
			subret = InitMesh(cnl,tmp,entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				ContourAbortDraw(cnl);
				gset_clip_ind(clip_ind_rect.clip_ind);
				return ret;
			}
			mesh_inited = 1;
		}
		subret = cnInitDataArray(cnl,&msize,&nsize,&bbox,
					 &min_cell_size,entry_name);
		if ((ret = MIN(subret,ret)) < NhlWARNING) {
			gset_clip_ind(clip_ind_rect.clip_ind);
			ContourAbortDraw(cnl);
			return ret;
		}
		subret = CnTriMeshWriteCellData
			(tmp->rpnt,tmp->iedg,tmp->itri,
			 msize,nsize,
			 bbox.l,bbox.b,bbox.r,bbox.t,
			 entry_name);
		if ((ret = MIN(subret,ret)) < NhlWARNING) {
			ContourAbortDraw(cnl);
			gset_clip_ind(clip_ind_rect.clip_ind);
			return ret;
		}
	}
	else if (cnp->do_fill && cnp->fill_order == order) {

		if (cnp->fill_mode == NhlAREAFILL) {
			if (! mesh_inited) {
				subret = InitMesh(cnl,tmp,entry_name);
				if ((ret = MIN(subret,ret)) < NhlWARNING) {
					ContourAbortDraw(cnl);
					gset_clip_ind(clip_ind_rect.clip_ind);
					return ret;
				}
				mesh_inited = 1;
			}
			if (cnp->aws == NULL) {
				subret = cnInitAreamap(cnl,entry_name);
				if ((ret = MIN(subret,ret)) < NhlWARNING) {
					gset_clip_ind(clip_ind_rect.clip_ind);
					ContourAbortDraw(cnl);
					return ret;
				}
			}
			if (! cnp->aws) {
				e_text = "%s: Error reserving workspace";
				NhlPError(NhlFATAL,NhlEUNKNOWN,
					  e_text,entry_name);
				gset_clip_ind(clip_ind_rect.clip_ind);
				ContourAbortDraw(cnl);
				return NhlFATAL;
			}
			subret = AddDataBoundToAreamap(cnl,entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				gset_clip_ind(clip_ind_rect.clip_ind);
				ContourAbortDraw(cnl);
				return ret;
			}

			subret = _NhlCtclam(tmp->rpnt,tmp->iedg,tmp->itri,
					    cnp->fws,cnp->iws,
					    cnp->aws,entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				gset_clip_ind(clip_ind_rect.clip_ind);
				ContourAbortDraw(cnl);
				return ret;
			}

			if (cnp->dump_area_map)
				_NhlDumpAreaMap(cnp->aws,entry_name);

			subret = _NhlArscam(cnp->aws,
					    (_NHLCALLF(hluctfill,HLUCTFILL)),
					    entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				gset_clip_ind(clip_ind_rect.clip_ind);
				ContourAbortDraw(cnl);
				return ret;
			}
			subret = _NhlIdleWorkspace(cnp->aws);
			ret = MIN(subret,ret);
			cnp->aws = NULL;
		}
		else if (cnp->fill_mode == NhlCELLFILL) {
			_NhlCellFill((NhlLayer)cnl,entry_name);
		}
		else { /* NhlRASTERFILL */
			int msize,nsize;
			float min_cell_size;
			NhlBoundingBox bbox;

			if (! mesh_inited) {
				subret = InitMesh(cnl,tmp,entry_name);
				if ((ret = MIN(subret,ret)) < NhlWARNING) {
					gset_clip_ind(clip_ind_rect.clip_ind);
					ContourAbortDraw(cnl);
					return ret;
				}
				mesh_inited = 1;
			}
			subret = cnInitCellArray(cnl,&msize,&nsize,&bbox,
						 &min_cell_size,entry_name);
 			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				gset_clip_ind(clip_ind_rect.clip_ind);
				ContourAbortDraw(cnl);
				return ret;
			}
			subret = _NhlCtcica(tmp->rpnt,tmp->iedg,tmp->itri,
					    cnp->fws,cnp->iws,cnp->cws,
					    msize,msize,nsize,
					    bbox.l,bbox.b,bbox.r,bbox.t,
					    min_cell_size,
					    cnp->raster_smoothing_on,
					    entry_name);
 			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				gset_clip_ind(clip_ind_rect.clip_ind);
				ContourAbortDraw(cnl);
				return ret;
			}
			if (cnp->cws != NULL) {
				subret = _NhlIdleWorkspace(cnp->cws);
				ret = MIN(subret,ret);
				cnp->cws = NULL;
			}
		}
	}

	if (! cnp->output_gridded_data &&
	    cnp->line_order == order &&
	    (cnp->do_lines || cnp->missing_val.perim_on ||
	     cnp->grid_bound.perim_on || cnp->out_of_range.perim_on)) {
		if (cnp->do_labels && cnp->label_masking) {
			if (! mesh_inited) {
				subret = InitMesh(cnl,tmp,entry_name);
				if ((ret = MIN(subret,ret)) < NhlWARNING) {
					gset_clip_ind(clip_ind_rect.clip_ind);
					ContourAbortDraw(cnl);
					return ret;
				}
				mesh_inited = 1;
			}
			c_ctseti("GIL",5);
			if (cnp->aws == NULL) {
				subret = cnInitAreamap(cnl,entry_name);
				if ((ret = MIN(subret,ret)) < NhlWARNING) {
					gset_clip_ind(clip_ind_rect.clip_ind);
					ContourAbortDraw(cnl);
					return ret;
				}
			}
			if (! cnp->aws) {
				e_text = "%s: Error reserving workspace";
				NhlPError(NhlFATAL,NhlEUNKNOWN,
					  e_text,entry_name);
				gset_clip_ind(clip_ind_rect.clip_ind);
				ContourAbortDraw(cnl);
				return NhlFATAL;
			}
			c_pcsetr("PH",(float)cnp->line_lbls.pheight);
			c_pcsetr("PW",(float)cnp->line_lbls.pwidth);
			c_pcsetr("CS",(float)cnp->line_lbls.cspacing);
			c_pcseti("FN",cnp->line_lbls.font);
			c_pcseti("QU",cnp->line_lbls.quality);
			c_pcsetc("FC",cnp->line_lbls.fcode);
			subret = _NhlCtlbam(tmp->rpnt,tmp->iedg,tmp->itri,
					    cnp->fws,cnp->iws,
					    cnp->aws,entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				gset_clip_ind(clip_ind_rect.clip_ind);
				ContourAbortDraw(cnl);
				return ret;
			}
			subret = _NhlCtcldm(tmp->rpnt,tmp->iedg,tmp->itri,
					    cnp->fws,cnp->iws,cnp->aws,
					    (_NHLCALLF(ctdrpl,CTDRPL)),
					    entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				gset_clip_ind(clip_ind_rect.clip_ind);
				ContourAbortDraw(cnl);
				return ret;
			}
			subret = _NhlIdleWorkspace(cnp->aws);
			ret = MIN(subret,ret);
			cnp->aws = NULL;
		}
		else {
			if (! mesh_inited) {
				subret = InitMesh(cnl,tmp,entry_name);
				if ((ret = MIN(subret,ret)) < NhlWARNING) {
					ContourAbortDraw(cnl);
					return ret;
				}
				mesh_inited = 1;
			}
			subret = _NhlCtcldr(tmp->rpnt,tmp->iedg,tmp->itri,
					    cnp->fws,cnp->iws,entry_name);

			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				gset_clip_ind(clip_ind_rect.clip_ind);
				ContourAbortDraw(cnl);
				return ret;
			}
		}
	}
	
	if (! cnp->output_gridded_data &&
	    cnp->do_labels && cnp->label_order == order) {
		if (! mesh_inited) {
			subret = InitMesh(cnl,tmp,entry_name);
			if ((ret = MIN(subret,ret)) < NhlWARNING) {
				gset_clip_ind(clip_ind_rect.clip_ind);
				ContourAbortDraw(cnl);
				return ret;
			}
			mesh_inited = 1;
		}
		cnp->line_lbls.count = 0;
		cnp->high_lbls.count = 0;
		cnp->low_lbls.count = 0;
		gset_fill_int_style(GSTYLE_SOLID);

		c_pcsetr("PH",(float)cnp->line_lbls.pheight);
		c_pcsetr("PW",(float)cnp->line_lbls.pwidth);
		c_pcsetr("CS",(float)cnp->line_lbls.cspacing);
		c_pcseti("FN",cnp->line_lbls.font);
		c_pcseti("QU",cnp->line_lbls.quality);
		c_pcsetc("FC",cnp->line_lbls.fcode);
		_NhlCtlbdr(tmp->rpnt,tmp->iedg,tmp->itri,
			 cnp->fws,cnp->iws,entry_name);
		if ((ret = MIN(subret,ret)) < NhlWARNING) {
			gset_clip_ind(clip_ind_rect.clip_ind);
			ContourAbortDraw(cnl);
			return ret;
		}
	}

	if (cnp->fws != NULL) {
		subret = _NhlIdleWorkspace(cnp->fws);
		ret = MIN(subret,ret);
		cnp->fws = NULL;
	}
	if (cnp->iws != NULL) {
		subret = _NhlIdleWorkspace(cnp->iws);
		cnp->iws = NULL;
		ret = MIN(subret,ret);
	}
	gset_clip_ind(clip_ind_rect.clip_ind);

	return MIN(subret,ret);
}



#define HERO(A,B,C) \
	sqrt(MAX(0.,((A)+(B)+(C))*((B)+(C)-(A))*((A)+(C)-(B))*((A)+(B)-(C))))
/*
 * Function:  _NhlTriMeshRasterFill
 *
 * Description: performs a discrete raster fill - 
 * replaces Conpack routine CPCICA - Conpack must be initialized, etc.
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

NhlErrorTypes _NhlTriMeshRasterFill
#if	NhlNeedProto
(
	float		*rpnt,
	int             *iedg,
	int             *itri,
	int		*cell,
	int		ica1,
	int		icam,
	int		ican,
	float		xcpf,
	float		ycpf,
	float		xcqf,
	float		ycqf,
	char		*entry_name
)
#else
(rpnt,iedg,itri,cell,ica1,icam,ican,
 xcpf,ycpf,xcqf,ycqf,entry_name)
	float		*rpnt,
	int             *iedg,
	int             *itri,
	int		*cell;
	int		ica1;
	int		icam;
	int		ican;
	float		xcpf;
	float		ycpf;
	float		xcqf;
	float		ycqf;
	char		*entry_name;
#endif
{
	NhlErrorTypes	ret = NhlNOERROR;
	char		*e_text;

	float		xmn,xmx,ymn,ymx;
	int		i,j,k,n,indx,indy,icaf,map,imap,iaid;
	float		xccf,xccd,xcci,yccf,yccd,ycci;
	float		zval,orv,spv;
        float		*levels;
	float		cxstep,cystep;
	float           xsoff,xeoff,ysoff,yeoff;
	NhlBoolean      x_isbound,y_isbound;

	float           tol1,tol2;
	int             ipp1,ipp2,ipp3;
	float           xcu1,xcu2,xcu3,ycu1,ycu2,ycu3;
	float           xcf1,xcf2,xcf3,ycf1,ycf2,ycf3;
	float           xd12,xd23,xd31,yd12,yd23,yd31;
	float           fva1,fva2,fva3;
	float           dn12,dn23,dn31;
	float           area;
	int             bound1,bound2;
	int             ibeg,iend,jbeg,jend;

	
        if (Cnp == NULL) {
		e_text = "%s: invalid call to _NhlRasterFill";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(NhlFATAL);
        }
        levels = (float*) Cnp->levels->data;
        
/* 
 * replacement for CPCICA
 */
	c_ctgetr("ORV",&orv);
	c_ctgeti("CAF",&icaf);
	c_ctgeti("MAP",&map);


	cxstep = (xcqf-xcpf)/(float)icam;
	cystep = (ycqf-ycpf)/(float)ican;
 	x_isbound = Cnp->sfp->xc_is_bounds;
 	y_isbound = Cnp->sfp->yc_is_bounds;

	xsoff = Xsoff + .5 * (1.0 - Xsoff);
	xeoff = Xeoff + .5 * (1.0 - Xeoff);
	ysoff = Ysoff + .5 * (1.0 - Ysoff);
	yeoff = Yeoff + .5 * (1.0 - Yeoff);

	tol1 = 0.00001 * MIN(Cnl->view.width,Cnl->view.height);
	tol2 = 0.5 * MIN(Cnl->view.width,Cnl->view.height);
	
/*
 *      initialize cell array with the missing value.
 */      
	for (j = 0; j < ican; j++) {
		for (i = 0; i < icam; i++) {
			*(cell + j * ica1 + i) = Cnp->missing_val.gks_fcolor;
		}
	}

/*
 * Now overwrite out-of-range areas with the out-of-range color
 */
	if (Tmp->ezmap) {
		imap = -map;
		zval = 0;
		for (j = 0; j < ican; j++) {
			if (j == 0)
				yccf = ycpf + ysoff * cystep;
			else if (j == ican - 1)
				yccf = ycpf + (ican - yeoff) * cystep;
			else
				yccf = ycpf + (j + ysoff) * cystep;
			yccd = c_cfuy(yccf);
			for (i = 0; i < icam; i++) {
				if (i == 0)
					xccf = xcpf + xsoff * cxstep;
				else if (i == icam - 1)
					xccf = xcpf + (icam - xeoff) * cxstep; 
				else
					xccf = xcpf + (i+xsoff) * cxstep;
				xccd = c_cfux(xccf);
				(_NHLCALLF(hluctmxyz,HLUCTMXYZ))
					(&imap,&xccd,&yccd,&zval,&xcci,&ycci);
				if (xcci == orv) {
					*(cell + j * ica1 + i) = Cnp->out_of_range.gks_fcolor;
				}
			}
		}
	}

/*
 * examine each triangle in turn
 */

	for (n = 0; n <= Tmp->ntri - Lotn; n += Lotn) {
	     if (itri[n+3] != 0)
		     continue;

/*
 * project point 1; if invisible skip it.
 */
	     if (iedg[itri[n]] == iedg[itri[n+1]] ||
		 iedg[itri[n]] == iedg[itri[n+1]+1]) {
		     ipp1 = iedg[itri[n]];
	     }
	     else {
		     ipp1 = iedg[itri[n]+1];
	     }
	     (_NHLCALLF(hluctmxyz,HLUCTMXYZ))
		     (&map,&rpnt[ipp1],&rpnt[ipp1+1],&rpnt[ipp1+2],
		      &xcu1,&ycu1);
	     if (orv != 0.0 && (xcu1 == orv || ycu1 == orv))
		     continue;

/*
 * project point 2; if invisible skip the triangle
 */
	     
	     if (iedg[itri[n+1]] == iedg[itri[n+2]] ||
		 iedg[itri[n+1]] == iedg[itri[n+2]+1]) {
		     ipp2 = iedg[itri[n+1]];
	     }
	     else {
		     ipp2 = iedg[itri[n+1]+1];
	     }
	     
	     (_NHLCALLF(hluctmxyz,HLUCTMXYZ))
		     (&map,&rpnt[ipp2],&rpnt[ipp2+1],&rpnt[ipp2+2],
		      &xcu2,&ycu2);
	     if (orv != 0.0  && (xcu2 == orv || ycu2 == orv))
		     continue;	     

/*
 * project point 3; if invisible skip the triangle
 */
	     
	     if (iedg[itri[n+2]] == iedg[itri[n]] ||
		 iedg[itri[n+2]] == iedg[itri[n]+1]) {
		     ipp3 = iedg[itri[n+2]];
	     }
	     else {
		     ipp3 = iedg[itri[n+2]+1];
	     }
	     
	     (_NHLCALLF(hluctmxyz,HLUCTMXYZ))
		     (&map,&rpnt[ipp3],&rpnt[ipp3+1],&rpnt[ipp3+2],
		      &xcu3,&ycu3);
	     if (orv != 0.0 && (xcu3 == orv || ycu2 == orv))
		     continue;	     

	     xcf1 = c_cufx(xcu1);
	     ycf1 = c_cufy(ycu1);
	     xcf2 = c_cufx(xcu2);
	     ycf2 = c_cufy(ycu2);
	     xcf3 = c_cufx(xcu3);
	     ycf3 = c_cufy(ycu3);

	     
	     xd12 = xcf2 - xcf1;
	     yd12 = ycf2 - ycf1;
	     xd23 = xcf3 - xcf2;
	     yd23 = ycf3 - ycf2;
	     xd31 = xcf1 - xcf3;
	     yd31 = ycf1 - ycf3;

/*
 * skip triangle if too small or too large
 */
	     if ((fabs(xd12) < tol1 && fabs(yd12) < tol1) ||
		 (fabs(xd23) < tol1 && fabs(yd23) < tol1) ||
		 (fabs(xd31) < tol1 && fabs(yd31) < tol1))
		     continue;

	     if ((fabs(xd12) > tol2 || fabs(yd12) > tol2) ||
		 (fabs(xd23) > tol2 || fabs(yd23) > tol2) ||
		 (fabs(xd31) > tol2 || fabs(yd31) > tol2))
		     continue;

/*
 * get the field values at the 3 points of the triangle
 */
	     fva1 = rpnt[ipp1+3];
	     fva2 = rpnt[ipp2+3];
	     fva3 = rpnt[ipp3+3];

/*
 * compute triangle lengths and the area
 */
	     dn12 = sqrt(xd12*xd12 + yd12 * yd12);
	     dn23 = sqrt(xd23*xd23 + yd23 * yd23);
	     dn31 = sqrt(xd31*xd31 + yd31 * yd31);

/*
 * Now set loop limits to examine center points of all cells that overlap
 * the bounding box of the triangle
 */
	     
	     bound1 = MAX(0,
			  MIN(icam-1,(int)
			      ((MIN(xcf1,MIN(xcf2,xcf3)) - xcpf) /
			       (xcqf-xcpf) * (float) icam)));
	     bound2 = MAX(0,
			  MIN(icam-1,(int)
			      ((MAX(xcf1,MAX(xcf2,xcf3)) - xcpf) /
			       (xcqf-xcpf) * (float) icam)));

	     ibeg = MIN(bound1,bound2);
	     iend = MAX(bound1,bound2);

	     bound1 = MAX(0,
			  MIN(ican-1,(int)
			      ((MIN(ycf1,MIN(ycf2,ycf3)) - ycpf) /
			       (ycqf-ycpf) * (float) ican)));
	     bound2 = MAX(0,
			  MIN(ican-1,(int)
			      ((MAX(ycf1,MAX(ycf2,ycf3)) - ycpf) /
			       (ycqf-ycpf) * (float) ican)));

	     jbeg = MIN(bound1,bound2);
	     jend = MAX(bound1,bound2);

/*
 * find each cell whose center point lies within the triangle and
 * set its color index appropriately
 */
	     for (j = jbeg; j <= jend; j++) {
		     float ts12,ts23,ts31;
		     float dnc1,dnc2,dnc3;
		     float yfp,xfp;
		     int jplus = j+1;
		     float a1,a2,a3;

		     yfp = ycpf + ((float)j+.5)/ican * (ycqf - ycpf);

		     for (i = ibeg; i <= iend; i++) {
			     float atot;
			     int iplus = i+1;
			     xfp = xcpf + ((float)i+.5)/icam * (xcqf - xcpf);
			     ts12 = (yd12*xfp-xd12*yfp-yd12*xcf1+xd12*ycf1)/
				     dn12;
			     ts23 = (yd23*xfp-xd23*yfp-yd23*xcf2+xd23*ycf2)/
				     dn23;
			     ts31 = (yd31*xfp-xd31*yfp-yd31*xcf3+xd31*ycf3)/
				     dn31;
			     if ((ts12 < 0.00001 && ts23 < 0.00001 &&
				  ts31 < 0.00001) ||
				 (ts12 > -0.00001 && ts23 > -0.00001 &&
				  ts31 > -0.00001)) {
				     float xd1,xd2,xd3,yd1,yd2,yd3;
				     float fvali;
				     xd1 = xfp - xcf1;
				     xd2 = xfp - xcf2;
				     xd3 = xfp - xcf3;
				     yd1 = yfp - ycf1;
				     yd2 = yfp - ycf2;
				     yd3 = yfp - ycf3;
				     dnc1 = sqrt(xd1*xd1 + yd1*yd1);
				     dnc2 = sqrt(xd2*xd2 + yd2*yd2);
				     dnc3 = sqrt(xd3*xd3 + yd3*yd3);
				     a1 = HERO(dn23,dnc2,dnc3);
				     a2 = HERO(dn31,dnc3,dnc1);
				     a3 = HERO(dn12,dnc1,dnc2);
				     atot = a1 + a2 + a3;
				     if (atot == 0.0) 
					     continue;

				     if (a1 > a2 && a1 > a3)
					     fvali = fva1;
				     else if (a2 > a1 && a2 > a3)
					     fvali = fva2;
				     else
					     fvali = fva3;
#if 0				     
						     
				     fvali = (fva1 * a1 + 
					      fva2 * a2 + fva3 * a3) / atot;
#endif
				     iaid = -1;
				     for (k=0; k < Cnp->level_count; k++) {
					     if (fvali < levels[k]) {
						  iaid = NhlcnAREAID_OFFSET+k;
						  break;
					     }
				     }
				     if (iaid == -1)
					     iaid = NhlcnAREAID_OFFSET +
						     Cnp->level_count;     
				     (_NHLCALLF(hluctscae,HLUCTSCAE))
					     (cell,&ica1,&icam,&ican,
					      &xcpf,&ycpf,&xcqf,&ycqf,
					      &iplus,&jplus,&icaf,&iaid);
			     }
		     }
	     }
	}

	return ret;
}

/*
 * Function:  CnTriMeshWriteCellData
 *
 * Description: Writes out the interpolated data associated with each
 * cell. This is a way of interpolating from one grid to another.
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

NhlErrorTypes CnTriMeshWriteCellData
#if	NhlNeedProto
(
	float		*rpnt,
	int             *iedg,
	int             *itri,
	int		icam,
	int		ican,
	float		xcpf,
	float		ycpf,
	float		xcqf,
	float		ycqf,
	char		*entry_name
)
#else
(rpnt,iedg,itri,ica1,icam,ican,
 xcpf,ycpf,xcqf,ycqf,entry_name)
	float		*rpnt,
	int             *iedg,
	int             *itri,
	int		icam;
	int		ican;
	float		xcpf;
	float		ycpf;
	float		xcqf;
	float		ycqf;
	char		*entry_name;
#endif
{
	NhlErrorTypes	ret = NhlNOERROR;
	char		*e_text;

	double		xmn,xmx,ymn,ymx;
	int		i,j,k,n,indx,indy,icaf,map,iaid;
	double		xccf,xccd,xcci,yccf,yccd,ycci;
	float		zval,orv,spv;
        float		*levels;
	double		cxstep,cystep;
	double          xsoff,xeoff,ysoff,yeoff;
	NhlBoolean      x_isbound,y_isbound;

	double           tol1,tol2;
	int             ipp1,ipp2,ipp3;
	float           xcu1,xcu2,xcu3,ycu1,ycu2,ycu3;
	double           xcf1,xcf2,xcf3,ycf1,ycf2,ycf3;
	double           xd12,xd23,xd31,yd12,yd23,yd31;
	double           fva1,fva2,fva3;
	double           dn12,dn23,dn31;
	double           area;
	int             bound1,bound2;
	int             ibeg,iend,jbeg,jend;
	float           *data;
	float           *xarray;
	float           *yarray;
	float           init_val;
	FILE 		*fp;
	float           out_of_range;
	int             count;
	float wlx,wrx,wby,wuy,wxstep,wystep;
	int licam,lican;

	printf("in CnWriteCellData\n");
        if (Cnp == NULL) {
		e_text = "%s: invalid call to _NhlRasterFill";
		NhlPError(NhlFATAL,NhlEUNKNOWN,e_text,entry_name);
		return(NhlFATAL);
        }
	NhlVAGetValues(Cnp->trans_obj->base.id,
		       NhlNtrOutOfRangeF,&out_of_range,NULL);

	if (Cnp->sfp->missing_value_set) {
		init_val = Cnp->sfp->missing_value;
	}
	else {
		init_val = 1E32;
	}

        levels = (float*) Cnp->levels->data;
	wlx = c_cfux(xcpf);
	wrx = c_cfux(xcqf);
	wby = c_cfuy(ycpf);
	wuy = c_cfuy(ycqf);
	wxstep = (wrx - wlx) / (icam); 
	wystep = (wuy - wby) / (ican); 

        
/* 
 * replacement for CTCICA
 */
	c_ctgetr("ORV",&orv);
	c_ctgeti("CAF",&icaf);
	c_ctgeti("MAP",&map);


	cxstep = (xcqf-xcpf)/(double)icam;
	cystep = (ycqf-ycpf)/(double)ican;
 	x_isbound = Cnp->sfp->xc_is_bounds;
 	y_isbound = Cnp->sfp->yc_is_bounds;
	xsoff = Xsoff + .5 * (1.0 - Xsoff);
	xeoff = Xeoff + .5 * (1.0 - Xeoff);
	ysoff = Ysoff + .5 * (1.0 - Ysoff);
	yeoff = Yeoff + .5 * (1.0 - Yeoff);

	tol1 = 0.00001 * MIN(Cnl->view.width,Cnl->view.height);
	tol2 = 0.5 * MIN(Cnl->view.width,Cnl->view.height);
	
/*
 *      initialize data array.
 *      make the data array larger by 2 because the outer edges
 *      never get written using this algorithm.
 */      
	
	licam = icam + 2;
	lican = ican + 2;
	data = NhlMalloc(licam * lican * sizeof(float));
	if (!data) {
		NHLPERROR((NhlFATAL,ENOMEM,NULL));
		return NhlFATAL;
	}
	for (j = 0; j < lican; j++) {
		for (i = 0; i < licam; i++) {
			*(data + j * licam + i) = init_val;
		}
	}
/*
 * examine each triangle in turn
 */

	for (n = 0; n < Tmp->ntri - Lotn; n += Lotn) {
	     if (itri[n+3] != 0)
		     continue;

/*
 * project point 1; if invisible skip it.
 */
	     if (iedg[itri[n]] == iedg[itri[n+1]] ||
		 iedg[itri[n]] == iedg[itri[n+1]+1]) {
		     ipp1 = iedg[itri[n]];
	     }
	     else {
		     ipp1 = iedg[itri[n]+1];
	     }
	     (_NHLCALLF(hluctmxyz,HLUCTMXYZ))
		     (&map,&rpnt[ipp1],&rpnt[ipp1+1],&rpnt[ipp1+2],
		      &xcu1,&ycu1);
	     if (orv != 0.0 && (xcu1 == orv || ycu1 == orv))
		     continue;

/*
 * project point 2; if invisible skip the triangle
 */
	     
	     if (iedg[itri[n+1]] == iedg[itri[n+2]] ||
		 iedg[itri[n+1]] == iedg[itri[n+2]+1]) {
		     ipp2 = iedg[itri[n+1]];
	     }
	     else {
		     ipp2 = iedg[itri[n+1]+1];
	     }
	     
	     (_NHLCALLF(hluctmxyz,HLUCTMXYZ))
		     (&map,&rpnt[ipp2],&rpnt[ipp2+1],&rpnt[ipp2+2],
		      &xcu2,&ycu2);
	     if (orv != 0.0  && (xcu2 == orv || ycu2 == orv))
		     continue;	     

/*
 * project point 3; if invisible skip the triangle
 */
	     
	     if (iedg[itri[n+2]] == iedg[itri[n]] ||
		 iedg[itri[n+2]] == iedg[itri[n]+1]) {
		     ipp3 = iedg[itri[n+2]];
	     }
	     else {
		     ipp3 = iedg[itri[n+2]+1];
	     }
	     
	     (_NHLCALLF(hluctmxyz,HLUCTMXYZ))
		     (&map,&rpnt[ipp3],&rpnt[ipp3+1],&rpnt[ipp3+2],
		      &xcu3,&ycu3);
	     if (orv != 0.0 && (xcu3 == orv || ycu2 == orv))
		     continue;	     

	     xcf1 = (double)c_cufx(xcu1);
	     ycf1 = (double)c_cufy(ycu1);
	     xcf2 = (double)c_cufx(xcu2);
	     ycf2 = (double)c_cufy(ycu2);
	     xcf3 = (double)c_cufx(xcu3);
	     ycf3 = (double)c_cufy(ycu3);

	     
	     xd12 = xcf2 - xcf1;
	     yd12 = ycf2 - ycf1;
	     xd23 = xcf3 - xcf2;
	     yd23 = ycf3 - ycf2;
	     xd31 = xcf1 - xcf3;
	     yd31 = ycf1 - ycf3;

/*
 * skip triangle if too small or too large
 */
	     if ((fabs(xd12) < tol1 && fabs(yd12) < tol1) ||
		 (fabs(xd23) < tol1 && fabs(yd23) < tol1) ||
		 (fabs(xd31) < tol1 && fabs(yd31) < tol1))
		     continue;

	     if ((fabs(xd12) > tol2 || fabs(yd12) > tol2) ||
		 (fabs(xd23) > tol2 || fabs(yd23) > tol2) ||
		 (fabs(xd31) > tol2 || fabs(yd31) > tol2))
		     continue;

/*
 * get the field values at the 3 points of the triangle
 */
	     fva1 = rpnt[ipp1+3];
	     fva2 = rpnt[ipp2+3];
	     fva3 = rpnt[ipp3+3];

/*
 * compute triangle lengths and the area
 */
	     dn12 = sqrt(xd12*xd12 + yd12 * yd12);
	     dn23 = sqrt(xd23*xd23 + yd23 * yd23);
	     dn31 = sqrt(xd31*xd31 + yd31 * yd31);

/*
 * Now set loop limits to examine center points of all cells that overlap
 * the bounding box of the triangle
 */
	     
	     bound1 = MAX(0,
			  MIN(licam-1,(int)
			      ((MIN(xcf1,MIN(xcf2,xcf3)) - xcpf) /
			       (xcqf-xcpf) * (float) licam)));
	     bound2 = MAX(0,
			  MIN(licam-1,(int)
			      ((MAX(xcf1,MAX(xcf2,xcf3)) - xcpf) /
			       (xcqf-xcpf) * (float) licam)));

	     ibeg = MIN(bound1,bound2);
	     iend = MAX(bound1,bound2);

	     bound1 = MAX(0,
			  MIN(lican-1,(int)
			      ((MIN(ycf1,MIN(ycf2,ycf3)) - ycpf) /
			       (ycqf-ycpf) * (float) lican)));
	     bound2 = MAX(0,
			  MIN(lican-1,(int)
			      ((MAX(ycf1,MAX(ycf2,ycf3)) - ycpf) /
			       (ycqf-ycpf) * (float) lican)));

	     jbeg = MIN(bound1,bound2);
	     jend = MAX(bound1,bound2);

/*
 * find each cell whose center point lies within the triangle and
 * set its color index appropriately
 */
	     for (j = jbeg; j <= jend; j++) {
		     double ts12,ts23,ts31;
		     double dnc1,dnc2,dnc3;
		     double yfp,xfp;
		     int jplus = j+1;
		     double a1,a2,a3;

		     yfp = ycpf + ((double)j+.5)/lican * (ycqf - ycpf);

		     for (i = ibeg; i <= iend; i++) {
			     int iplus = i+1;
			     double atot;
			     xfp = xcpf + ((float)i+.5)/licam * (xcqf - xcpf);
			     ts12 = (yd12*xfp-xd12*yfp-yd12*xcf1+xd12*ycf1)/
				     dn12;
			     ts23 = (yd23*xfp-xd23*yfp-yd23*xcf2+xd23*ycf2)/
				     dn23;
			     ts31 = (yd31*xfp-xd31*yfp-yd31*xcf3+xd31*ycf3)/
				     dn31;
			     if ((ts12 < 0.00001 && ts23 < 0.00001 &&
				  ts31 < 0.00001) ||
				 (ts12 > -0.00001 && ts23 > -0.00001 &&
				  ts31 > -0.00001)) {
				     float xd1,xd2,xd3,yd1,yd2,yd3;
				     float fvali;
				     xd1 = xfp - xcf1;
				     xd2 = xfp - xcf2;
				     xd3 = xfp - xcf3;
				     yd1 = yfp - ycf1;
				     yd2 = yfp - ycf2;
				     yd3 = yfp - ycf3;
				     dnc1 = sqrt(xd1*xd1 + yd1*yd1);
				     dnc2 = sqrt(xd2*xd2 + yd2*yd2);
				     dnc3 = sqrt(xd3*xd3 + yd3*yd3);
				     a1 = HERO(dn23,dnc2,dnc3);
				     a2 = HERO(dn31,dnc3,dnc1);
				     a3 = HERO(dn12,dnc1,dnc2);
				     atot = a1 + a2 + a3;
#if 0
				     if (a1 > a2 && a1 > a3)
					     fvali = fva1;
				     else if (a2 > a1 && a2 > a3)
					     fvali = fva2;
				     else
					     fvali = fva3;
#endif
				     if (atot == 0.0)
					     continue;

				     fvali = (fva1 * a1 +
					      fva2 * a2 + fva3 * a3) / atot;

				     *(data + j * licam + i) = (float)fvali;
			     }
		     }
	     }
	}
        fp = fopen("tmp.bin","w");
	for (j = 1; j <= ican; j++) {
		float           *d = data + j * licam + 1;
		count = fwrite(d,sizeof(float),icam,fp);
		if (count < icam) {
			NhlPError(NhlFATAL,NhlEUNKNOWN,
				  "Error writing output file\n");
			return NhlFATAL;
		}
	}
	fclose(fp);

	fp = fopen("tmp-lon.bin","w");
	for (i = 0; i < icam; i++) {
		float lon = wlx + i * wxstep;
		fwrite(&lon,sizeof(float),1,fp);
	}
	fclose(fp);
	fp = fopen("tmp-lat.bin","w");
	for (j = 0; j < ican; j++) {
		float lat = wby + j * wystep;
		fwrite(&lat,sizeof(float),1,fp);
	}
	fclose(fp);

	NhlFree(data);

	return ret;
}

/*
 * Function:  hluctfill
 *
 * Description: C version of APR user routine called from within ARSCAM 
 *		to fill areas based on the area ID.
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
int (_NHLCALLF(hluctfill,HLUCTFILL))
#if	NhlNeedProto
(
	float *xcs, 
	float *ycs, 
	int *ncs, 
	int *iai, 
	int *iag, 
	int *nai
)
#else
(xcs,ycs,ncs,iai,iag,nai)
	float *xcs; 
	float *ycs; 
	int *ncs; 
	int *iai; 
	int *iag; 
	int *nai;
#endif
{
	int i;
	int pat_ix, col_ix;
	float fscale;
	int *colp, *patp;
	float *sclp;

	if (Cnp == NULL) return 0;

	for (i = 0; i < *nai; i++) {
#if 0
		printf("i %d iai %d iag %d\n",i,iai[i],iag[i]);
#endif
		if (iai[i] == 9999) {
			return 0;
		}
		if (iag[i] == 5 && iai[i] == -1) {
			return 0;
		}
	}

	colp = (int *) Cnp->fill_colors->data;
	patp = (int *) Cnp->fill_patterns->data;
	sclp = (float *) Cnp->fill_scales->data;
	for (i = 0; i < *nai; i++) {
		if (iag[i] == 3) {
			if (iai[i] > 99 && 
			    iai[i] < 100 + Cnp->fill_count) {
				int ix = iai[i] - 100;
				col_ix = Cnp->mono_fill_color ? 
					Cnp->fill_color : colp[ix];
				pat_ix = Cnp->mono_fill_pattern ?
					Cnp->fill_pattern : patp[ix];
				fscale = Cnp->mono_fill_scale ?
					Cnp->fill_scale : sclp[ix];
			}
			else {
				NhlcnRegionAttrs *reg_attrs;
#if 0
		                printf("i %d iai %d iag %d\n",i,iai[i],iag[i]);
#endif

				switch (iai[i]) {
				case 98:
					reg_attrs = &Cnp->missing_val;
					break;
				default:
					return 0;
				}
				col_ix = reg_attrs->fill_color;
				pat_ix = reg_attrs->fill_pat;
				fscale = reg_attrs->fill_scale;
			}
			NhlVASetValues(Cnl->base.wkptr->base.id,
				       _NhlNwkFillIndex, pat_ix,
				       _NhlNwkFillColor, col_ix,
				       _NhlNwkFillScaleFactorF,fscale,
				       _NhlNwkFillBackground,
				       Cnp->fill_background_color,
				       _NhlNwkFillDotSizeF,Cnp->fill_dot_size,
				       _NhlNwkEdgesOn,0,
				       NULL);
			
			_NhlSetFillInfo(Cnl->base.wkptr,(NhlLayer) Cnl);
			_NhlWorkstationFill(Cnl->base.wkptr,xcs,ycs,*ncs);
		}
	}
	return 0;
}


/*
 * Function:  hluctscae
 *
 * Description: 
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
void  (_NHLCALLF(hluctscae,HLUCTSCAE))
#if	NhlNeedProto
(
	int		*icra,
	int		*ica1,
	int		*icam,
	int		*ican,
	float		*xcpf,
	float		*ycpf,
	float		*xcqf,
	float		*ycqf,
	int		*ind1,
	int		*ind2,
	int		*icaf,
	int		*iaid		      
)
#else
(icra,ica1,icam,ican,xcpf,ycpf,xcqf,ycqf,ind1,ind2,icaf,iaid)
	int		*icra;
	int		*ica1;
	int		*icam;
	int		*ican;
	float		*xcpf;
	float		*ycpf;
	float		*xcqf;
	float		*ycqf;
	int		*ind1;
	int		*ind2;
	int		*icaf;
	int		*iaid;
#endif
{
	int col_ix;

	if (Cnp == NULL) {
		_NHLCALLF(ctscae,CTSCAE)
			(icra,ica1,icam,ican,xcpf,ycpf,xcqf,ycqf,
			 ind1,ind2,icaf,iaid);
		return;
	}
	/* no support in cell arrays for transparent, so it's necessary
	   to reset transparent color indexes to background */
	   

	if (*iaid > 99 && *iaid < 100 + Cnp->fill_count) {
		col_ix = Cnp->gks_fill_colors[*iaid - 100];
		if (col_ix < 0) col_ix = NhlBACKGROUND;
	}
	else if (*iaid == 98) {
#if 0
		printf("hluctscae iaid = %d\n",*iaid);
#endif
		col_ix = Cnp->missing_val.gks_fcolor;
		if (col_ix < 0) col_ix = NhlBACKGROUND;
	}
	else {
#if 0
		printf("hluctscae iaid = %d\n",*iaid);
#endif
		col_ix = NhlBACKGROUND;
	}
	*(icra + ((*ind2 - 1) * *ica1 + (*ind1 - 1))) = col_ix;

	return;
}

/*
 * Function:  hluctchcl
 *
 * Description: C version of the CPCHCL function that is called from
 *              the Conpack CPCLDR and CPCLDM functions. 
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
void   (_NHLCALLF(hluctchcl,HLUCTCHCL))
#if	NhlNeedProto
(
	int	*iflg
)
#else
(iflg)
	int	*iflg;
#endif

{

	char func[] = "HLUCTCHCL";
	int i, pai, dpix;
	char buffer[NhlDASHBUFSIZE];
	int lcol;
	float thickness, tf;
	float *thp;
	int   *dpp;

	if (Cnp == NULL) {
		_NHLCALLF(ctchcl,CTCHCL)(iflg);
		return;
	}

	dpp = (int *) Cnp->line_dash_patterns->data;
	thp = (float *) Cnp->line_thicknesses->data;

	if (*iflg != 1) return;

	c_ctgeti("PAI", &pai);
	if (pai > 0 && pai < 256) {
		if (! Cnp->do_lines) return;
		thickness = Cnp->mono_line_thickness ? 
			Cnp->line_thickness : thp[pai-1];
		lcol = Cnp->mono_line_color ? 
			Cnp->gks_line_colors[0] : Cnp->gks_line_colors[pai-1];
		dpix = Cnp->mono_line_dash_pattern ? 
			Cnp->line_dash_pattern : dpp[pai-1];
	}
	else {
		NhlcnRegionAttrs *reg_attrs;
#if 0
		printf("hluctchcl pai: %d\n", pai);
#endif
		switch (pai) {
		case -1:
			reg_attrs = &Cnp->missing_val;
			break;
		case -2:
			reg_attrs = &Cnp->out_of_range;
			break;
		default:
			return;
		}
		thickness = reg_attrs->perim_thick;
		lcol = reg_attrs->gks_pcolor;
		dpix = reg_attrs->perim_dpat;
	}
		
	memset((void *) buffer,'\0', sizeof(buffer)*sizeof(char));

	c_pcseti("FN",0);
	c_pcseti("CL",1);
 	c_pcseti("CC",-1);
	c_pcseti("OC",-1);
	(void)_NhlLLErrCheckPrnt(NhlWARNING,func);
	
	/*
	 * Reset DashPack so we know what state we are starting from.
	 */
	_NHLCALLF(dprset,DPRSET)();
	(void)_NhlLLErrCheckPrnt(NhlWARNING,func);

	if (dpix < 0) 
		dpix = NhlSOLIDLINE;
	else if (dpix > Cnp->dtable_len)
	    dpix = 1 + (dpix - 1) % Cnp->dtable_len;
	strncpy(buffer,Cnp->dtable[dpix],sizeof(buffer) - 1);

	tf = Cnp->line_dash_seglen / (strlen(buffer)+.5);
	c_dpsetr("WOG",(float)tf);
	(void)_NhlLLErrCheckPrnt(NhlWARNING,func);
	c_dpsetr("WOS",(float)tf);
	(void)_NhlLLErrCheckPrnt(NhlWARNING,func);

	if (lcol == NhlTRANSPARENT) {
		for (i = 0; i < strlen(buffer); i++)
			buffer[i] = '_';
	}
	else{
	        gset_line_colr_ind((Gint)lcol);
		(void)_NhlLLErrCheckPrnt(NhlWARNING,func);
	}

        gset_linewidth(thickness);
	(void)_NhlLLErrCheckPrnt(NhlWARNING,func);


	if (pai > 0 && Cnp->llabel_placement == NhlCONSTANT) {
		int buff_size = sizeof(buffer) - strlen(buffer) - 1;
		char *tchar = &buffer[strlen(buffer)];
		char *ts = ((NhlString *) Cnp->line_lbls.text)[pai-1];
		NhlcnLevelUseMode *lup = 
			(NhlcnLevelUseMode *) Cnp->level_flags->data;
		NhlcnLevelUseMode flag;
		NhlColorIndex llcol;
		int j;
		NhlBoolean do_label;

		llcol = Cnp->line_lbls.mono_color ?
			 Cnp->line_lbls.gks_color : 
				Cnp->line_lbls.colors[pai-1];
		
		flag = Cnp->mono_level_flag ? 
			Cnp->level_flag : lup[pai-1];

		do_label = Cnp->line_lbls.on && flag > NhlLINEONLY;

		if (llcol == NhlTRANSPARENT && do_label) {
			/*
			 * Put spaces in for label.
			 */
			j = MIN(strlen(ts) * 2 + 1,buff_size);
			for(i=0;i < j-1;i+=2){
				tchar[i] = ' ';
				tchar[i+1] = '|';
			}
		}
		else if (do_label) {
			/*
			 * Add breaks in at each space of the label.
			 */
			i=0;
			j=0;
			while (i < buff_size && ts[j] != '\0'){
				if (ts[j] == ' ')
					tchar[i++] = '|';
				tchar[i++] = ts[j++];
			}
			c_pcseti("OC",llcol);
			c_pcseti("CC",llcol);
		}
		c_pcsetr("PH",(float)Cnp->line_lbls.pheight);
		c_pcsetr("PW",(float)Cnp->line_lbls.pwidth);
		c_pcsetr("CS",(float)Cnp->line_lbls.cspacing);
		c_pcseti("FN",Cnp->line_lbls.font);
		c_pcseti("QU",Cnp->line_lbls.quality);
		c_pcsetc("FC",Cnp->line_lbls.fcode);
		c_pcsetr("CL",(float)Cnp->line_lbls.thickness);
		(void)_NhlLLErrCheckPrnt(NhlWARNING,func);
	}

	c_dpsetc("DPT",buffer);
	(void)_NhlLLErrCheckPrnt(NhlWARNING,func);
	c_dpsetr("WOC",(float)Cnp->line_lbls.real_height);
	(void)_NhlLLErrCheckPrnt(NhlWARNING,func);

	return;
}


/*
 * Function:	Substitute
 *
 * Description: substitutes a string for a Conpack substitution sequence.
 *
 * In Args:	
 *
 * Out Args:	NONE
 *
 * Return Values:	Error Conditions
 *
 * Side Effects:	Objects created and destroyed.
 */
static void Substitute
#if	NhlNeedProto
(
	char		*buf,
	int		replace_count,
	char		*subst
)
#else 
(buf,replace_count,subst)
	char		*buf;
	int		replace_count;
	char		*subst;
#endif
{
	int subst_count,add,buflen;
	char *from, *to;

	buflen = strlen(buf);
	subst_count = strlen(subst);
	if (subst_count - replace_count < 0) {
		for (from = buf+replace_count,to = buf+subst_count; ;
		     to++,from++) { 
			*to = *from;
			if (*from == '\0')
				break;
		}
	}
	else if ((add = subst_count - replace_count) > 0) {
		for (from = buf + buflen,to = buf + buflen + add; 
		     from >= buf + replace_count;)
			*to-- = *from--;
	}
	strncpy(buf,subst,subst_count);
}

/*
 * Function:  hluctchhl
 *
 * Description: 
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
void   (_NHLCALLF(hluctchhl,HLUCTCHHL))
#if	NhlNeedProto
(
	int	*iflg
)
#else
(iflg)
	int	*iflg;
#endif

{
	char buf[128];
	char *fstr,*sub;
	float zdv;
	NhlFormatRec *frec;
	int *fwidth, *sig_digits, *left_sig_digit, *point_pos, *exp_switch_len, *exp_field_width;

	if (Cnp == NULL) {
		_NHLCALLF(ctchhl,CTCHHL)(iflg);
		return;
	}

#if 0
	{ /* for debugging */
		float flx,frx,fby,fuy,wlx,wrx,wby,wuy; int ll;
		c_getset(&flx,&frx,&fby,&fuy,&wlx,&wrx,&wby,&wuy,&ll);
		printf("getset - %f,%f,%f,%f,%f,%f,%f,%f\n",
		       flx,frx,fby,fuy,wlx,wrx,wby,wuy); 
  	}
#endif
	switch (*iflg) {
	case 1:
	case 3:
	case 5:
	case 7:
		frec = &Cnp->max_data_format;
		fwidth = frec->field_width_flag == NhlffUNSPECED ? NULL : &frec->field_width;
		sig_digits = frec->sig_digits_flag == NhlffUNSPECED ? NULL : &frec->sig_digits;
		left_sig_digit = frec->left_sig_digit_flag == NhlffUNSPECED ? NULL : &frec->left_sig_digit;
		point_pos =  frec->point_position_flag == NhlffUNSPECED ? NULL : &frec->point_position;
		exp_switch_len = frec->exp_switch_flag == NhlffUNSPECED ? NULL : &frec->exp_switch_len;
		exp_field_width = frec->exp_field_width_flag == NhlffUNSPECED ? NULL : &frec->exp_field_width;
		/* drop through */
	default:
		break;
	}
	switch (*iflg) {
	case 1:
		if (! Cnp->high_lbls.on) {
			c_ctsetc("CTM"," ");
			return;
		}
		if ( Cnp->high_lbls.gks_color > NhlTRANSPARENT) {
			c_pcseti("CC", Cnp->high_lbls.gks_color);
			c_pcseti("OC", Cnp->high_lbls.gks_color);
		}
		c_pcsetr("PH",(float)Cnp->high_lbls.pheight);
		c_pcsetr("PW",(float)Cnp->high_lbls.pwidth);
		c_pcsetr("CS",(float)Cnp->high_lbls.cspacing);
		c_pcseti("FN",Cnp->high_lbls.font);
		c_pcseti("QU",Cnp->high_lbls.quality);
		c_pcsetc("FC",Cnp->high_lbls.fcode);
		gset_linewidth(Cnp->high_lbls.thickness);

		strcpy(buf,(char *)Cnp->high_lbls.text);
		if ((sub = strstr(buf,"$ZDV$")) == NULL) {
			return;
		}
		c_ctgetr("zdv",&zdv);
		zdv /= Cnp->label_scale_factor;
		fstr = _NhlFormatFloat(&Cnp->high_lbls.format,zdv,
				       fwidth, sig_digits,
				       left_sig_digit, exp_field_width,
				       exp_switch_len, point_pos,
				       Cnp->high_lbls.fcode[0],
				       "ContourPlotDraw");
		Substitute(sub,5,fstr);
		c_ctsetc("CTM",buf);
		break;
	case 2:
		if (! Cnp->high_lbls.on) return;
		gset_fill_colr_ind(Cnp->high_lbls.gks_bcolor);
		break;
	case 3:
		if (! Cnp->high_lbls.on) {
			c_ctsetc("CTM"," ");
			return;
		}
		if ( Cnp->high_lbls.gks_color > NhlTRANSPARENT) {
			c_pcseti("CC", Cnp->high_lbls.gks_color);
			c_pcseti("OC", Cnp->high_lbls.gks_color);
		}
		else {
			c_ctsetc("CTM"," ");
			return;
		}
		c_pcsetr("PH",(float)Cnp->high_lbls.pheight);
		c_pcsetr("PW",(float)Cnp->high_lbls.pwidth);
		c_pcsetr("CS",(float)Cnp->high_lbls.cspacing);
		c_pcseti("FN",Cnp->high_lbls.font);
		c_pcseti("QU",Cnp->high_lbls.quality);
		c_pcsetc("FC",Cnp->high_lbls.fcode);
		gset_linewidth((float)Cnp->high_lbls.thickness);

		strcpy(buf,(char *)Cnp->high_lbls.text);
		if ((sub = strstr(buf,"$ZDV$")) == NULL) {
			return;
		}
		c_ctgetr("zdv",&zdv);
		zdv /= Cnp->label_scale_factor;
		fstr = _NhlFormatFloat(&Cnp->high_lbls.format,zdv,
				       fwidth, sig_digits,
				       left_sig_digit, exp_field_width,
				       exp_switch_len, point_pos,
				       Cnp->high_lbls.fcode[0],
				       "ContourPlotDraw");
		Substitute(sub,5,fstr);
		c_ctsetc("CTM",buf);
		Cnp->high_lbls.count++;
		break;
	case 4:
		gset_line_colr_ind(Cnp->high_lbls.gks_plcolor);
		gset_linewidth(Cnp->high_lbls.perim_lthick);
		break;
	case 5:
		if (! Cnp->low_lbls.on) {
			c_ctsetc("CTM"," ");
			return;
		}
		if (Cnp->low_lbls.gks_color > NhlTRANSPARENT) {
			c_pcseti("CC", Cnp->low_lbls.gks_color);
			c_pcseti("OC", Cnp->low_lbls.gks_color);
		}
		c_pcsetr("PH",(float)Cnp->low_lbls.pheight * LowLabelFactor);
		c_pcsetr("PW",(float)Cnp->low_lbls.pwidth * LowLabelFactor);
		c_pcsetr("CS",(float)Cnp->low_lbls.cspacing);
		c_pcseti("FN",Cnp->low_lbls.font);
		c_pcseti("QU",Cnp->low_lbls.quality);
		c_pcsetc("FC",Cnp->low_lbls.fcode);
		gset_linewidth((float)Cnp->low_lbls.thickness);
		strcpy(buf,(char *)Cnp->low_lbls.text);
		if ((sub = strstr(buf,"$ZDV$")) == NULL) {
			return;
		}
		c_ctgetr("zdv",&zdv);
		zdv /= Cnp->label_scale_factor;
		fstr = _NhlFormatFloat(&Cnp->low_lbls.format,zdv,
				       fwidth, sig_digits,
				       left_sig_digit, exp_field_width,
				       exp_switch_len, point_pos,
				       Cnp->low_lbls.fcode[0],
				       "ContourPlotDraw");
		Substitute(sub,5,fstr);
		c_ctsetc("CTM",buf);
		break;
	case 6:
		if (! Cnp->low_lbls.on) return;
		gset_fill_colr_ind(Cnp->low_lbls.gks_bcolor);
		break;
	case 7:
		if (! Cnp->low_lbls.on) {
			c_ctsetc("CTM"," ");
			return;
		}
		if (Cnp->low_lbls.gks_color > NhlTRANSPARENT) {
			c_pcseti("CC", Cnp->low_lbls.gks_color);
			c_pcseti("OC", Cnp->low_lbls.gks_color);
		}
		else {
			c_ctsetc("CTM"," ");
			return;
		}
		c_pcsetr("PH",(float)Cnp->low_lbls.pheight * LowLabelFactor);
		c_pcsetr("PW",(float)Cnp->low_lbls.pwidth * LowLabelFactor);
		c_pcsetr("CS",(float)Cnp->low_lbls.cspacing);
		c_pcseti("FN",Cnp->low_lbls.font);
		c_pcseti("QU",Cnp->low_lbls.quality);
		c_pcsetc("FC",Cnp->low_lbls.fcode);
		gset_linewidth((float)Cnp->low_lbls.thickness);
		strcpy(buf,(char *)Cnp->low_lbls.text);
		if ((sub = strstr(buf,"$ZDV$")) == NULL) {
			return;
		}
		c_ctgetr("zdv",&zdv);
		zdv /= Cnp->label_scale_factor;
		fstr = _NhlFormatFloat(&Cnp->low_lbls.format,zdv,
				       fwidth, sig_digits,
				       left_sig_digit, exp_field_width,
				       exp_switch_len, point_pos,
				       Cnp->low_lbls.fcode[0],
				       "ContourPlotDraw");
		Substitute(sub,5,fstr);
		c_ctsetc("CTM",buf);
		Cnp->low_lbls.count++;
		break;
	case 8:
		gset_line_colr_ind(Cnp->low_lbls.gks_plcolor);
		gset_linewidth(Cnp->low_lbls.perim_lthick);
		break;
	default:
		break;
	}

	return;
}

/*
 * Function:  hluctchll
 *
 * Description: 
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
void   (_NHLCALLF(hluctchll,HLUCTCHLL))
#if	NhlNeedProto
(
	int	*iflg
)
#else
(iflg)
	int	*iflg;
#endif

{

	int pai;
	static int llcol;

	if (Cnp == NULL) {
		_NHLCALLF(ctchll,CTCHLL)(iflg);
		return;
	}
	if (Cnp->llabel_placement == NhlCONSTANT)
		return;

	if (*iflg == 1) {
		c_pcsetr("PH",(float)Cnp->line_lbls.pheight);
		c_pcsetr("PW",(float)Cnp->line_lbls.pwidth);
		c_pcsetr("CS",(float)Cnp->line_lbls.cspacing);
		c_pcseti("FN",Cnp->line_lbls.font);
		c_pcseti("QU",Cnp->line_lbls.quality);
		c_pcsetc("FC",Cnp->line_lbls.fcode);
		gset_linewidth((float)Cnp->line_lbls.thickness);
	}
	else if (*iflg == 2) {
		if (Cnp->line_lbls.gks_bcolor > NhlTRANSPARENT)
			gset_fill_colr_ind(Cnp->line_lbls.gks_bcolor);
	}
	else if (*iflg == 3) {
		c_ctgeti("PAI", &pai);
		if (pai > 0) {
			pai -= 1;

			llcol = Cnp->line_lbls.mono_color ?
				 Cnp->line_lbls.gks_color : 
					Cnp->line_lbls.colors[pai];
			if (llcol > NhlTRANSPARENT) {
				c_pcseti("CC",llcol);
				c_pcseti("OC",llcol);
			}
			else {
				c_ctsetc("CTM"," ");
			}
			c_pcsetr("PH",(float)Cnp->line_lbls.pheight);
			c_pcsetr("PW",(float)Cnp->line_lbls.pwidth);
			c_pcsetr("CS",(float)Cnp->line_lbls.cspacing);
			c_pcseti("FN",Cnp->line_lbls.font);
			c_pcseti("QU",Cnp->line_lbls.quality);
			c_pcsetc("FC",Cnp->line_lbls.fcode);
			gset_linewidth((float)Cnp->line_lbls.thickness);
		}
		Cnp->line_lbls.count++;
	}
	else if (*iflg == 4) {
		gset_line_colr_ind(Cnp->line_lbls.gks_plcolor);
		gset_linewidth(Cnp->line_lbls.perim_lthick);
	}

	return;
}


/* low level overlay mapping functions */

static void OverlayMapXY
#if	NhlNeedProto
(
        NhlTransformLayerPart *tfp,
        float *xin,
        float *yin,
        float* xout,
        float* yout)
#else
(tfp,xin,yin,xout,yout)
	NhlTransformLayerPart *tfp;
        float *xin;
        float *yin;
        float *xout;
        float *yout;
#endif
{
        int status = 0;

        if (! tfp->overlay_trans_obj ||
            tfp->overlay_trans_obj == tfp->trans_obj) {
		_NhlCompcToWin(tfp->trans_obj,xin,yin,1,xout,yout,
			       &status,NULL,NULL);
	}
        else {
		_NhlCompcToData(tfp->trans_obj,xin,yin,1,xout,yout,
				&status,NULL,NULL);

		if (status) return;
#if 0
		fprintf (stderr,"inter: %f %f : ",*xout,*yout);
#endif

		_NhlDataToWin(tfp->overlay_trans_obj,
			     xout,yout,1,xout,yout,&status,NULL,NULL);
        }

#if 0
	fprintf (stderr,"%f %f : %f %f \n",*xin,*yin,*xout,*yout);
#endif

	return;
}


static void OverlayInvMapXY
#if	NhlNeedProto
(
        NhlTransformLayerPart *tfp,
        float *xin,
        float *yin,
        float* xout,
        float* yout)
#else
(tfp,xin,yin,xout,yout)
	NhlTransformLayerPart *tfp;
        float *xin;
        float *yin;
        float *xout;
        float *yout;
#endif
{
        int status = 0;

        if (! tfp->overlay_trans_obj ||
            tfp->overlay_trans_obj == tfp->trans_obj) {
		_NhlWinToCompc(tfp->trans_obj,xin,yin,1,xout,yout,
			       &status,NULL,NULL);
	}
        else {
		_NhlWinToData(tfp->overlay_trans_obj,
			      xin,yin,1,xout,yout,
			      &status,NULL,NULL);

		if (status) return;
#if 0
		fprintf (stderr,"inter: %f %f : ",*xout,*yout);
#endif

		_NhlDataToCompc(tfp->trans_obj,xout,yout,1,xout,yout,
				&status,NULL,NULL);
        }

#if 0
	fprintf (stderr,"%f %f : %f %f \n",*xin,*yin,*xout,*yout);
#endif

	return;
}

/*
 * Function:  hluctmxyz
 *
 * Description: 
 *
 * In Args:
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
void   (_NHLCALLF(hluctmxyz,HLUCTMXYZ))
#if	NhlNeedProto
(
	int	*imap,
	float	*xinp,
	float	*yinp,
	float	*zinp,
	float	*xotp,
	float	*yotp
)
#else
(imap,xinp,yinp,zinp,xotp,yotp)
	int	*imap;
	float	*xinp;
	float	*yinp;
	float	*zinp,
	float	*xotp;
	float	*yotp;
#endif

{
	int status;
	float xtmp,ytmp;
	float rtod = 57.2957795130823;

	if (Cnp == NULL) {
		_NHLCALLF(ctmxyz,CTMXYZ)(imap,xinp,yinp,zinp,xotp,yotp);
		return;
	}

        if (*imap == - Nhlcn1DMESHMAPVAL && Tmp->ezmap) {
		OverlayInvMapXY(&Cnl->trans,xinp,yinp,xotp,yotp);
	}
        else if (abs(*imap) != NhlcnMAPVAL) {
                *xotp = *xinp;
                *yotp = *yinp;
        }
	else if (Cnl->trans.overlay_status == _tfCurrentOverlayMember &&
		 ! Cnl->trans.do_ndc_overlay) { 
		if (*imap > 0) {
			ytmp = rtod*asin(*zinp/
					 sqrt(*xinp * *xinp + *yinp * *yinp +
					      *zinp * *zinp));
			if (*xinp == 0 && *yinp == 0) {
				xtmp = 0.0;
			}
			else {
				xtmp = rtod * atan2(*yinp,*xinp);
			}
#if 0
			if (xtmp < Cnp->xlb)
				xtmp += 360.0;
			if (xtmp > Cnp->xub) 
				xtmp -= 360.0;
#endif
			OverlayMapXY(&Cnl->trans,&xtmp,&ytmp,xotp,yotp);
		}
		else
			OverlayInvMapXY(&Cnl->trans,xinp,yinp,xotp,yotp);
	}
	else {
		/* I don't know if this branch is ever taken any more */
		if (*imap > 0) {
			ytmp = rtod*asin(*zinp/
					 sqrt(*xinp * *xinp + *yinp * *yinp +
					      *zinp * *zinp));
			if (*xinp == 0 && *yinp == 0) {
				xtmp = 0.0;
			}
			else {
				xtmp = rtod * atan2(*yinp,*xinp);
			}
			if (xtmp < Cnp->xlb)
				xtmp += 360.0;
			if (xtmp > Cnp->xub) 
				xtmp -= 360.0;
			_NhlCompcToWin((NhlLayer)Cnp->trans_obj,
				       &xtmp,&ytmp,1,xotp,yotp,
				       &status,NULL,NULL);
		}
		else { 
			_NhlWinToCompc((NhlLayer)Cnp->trans_obj,
				       xinp,yinp,1,xotp,yotp,
				       &status,NULL,NULL);

		}
		
	}
	return;
}


/*
 * Function:  load_hluct_routines
 *
 * Description: Forces the hluct... routines to load from the HLU library
 *
 * In Args:   NhlBoolean flag - should always be False - dont actually
 *			        want to call the routines.
 *
 * Out Args:
 *
 * Return Values:
 *
 * Side Effects: 
 */

/*ARGSUSED*/
static void   load_hluct_routines
#if	NhlNeedProto
(
	NhlBoolean	flag
)
#else
(flag)
	NhlBoolean	flag;
#endif
{
	int idum;
	float fdum;


	if (flag) {
		_NHLCALLF(hluctmxyz,HLUCTMXYZ)
			(&idum,&fdum,&fdum,&fdum,&fdum,&fdum);
		_NHLCALLF(hluctchll,HLUCTCHLL)(&idum);
		_NHLCALLF(hluctchhl,HLUCTCHHL)(&idum);
		_NHLCALLF(hluctchcl,HLUCTCHCL)(&idum);
		_NHLCALLF(hluctscae,HLUCTSCAE)(&idum,&idum,&idum,&idum,
					       &fdum,&fdum,&fdum,&fdum,
					       &idum,&idum,&idum,&idum);
	}
	return;
}

void _NhlSetCnl
#if	NhlNeedProto
(
	NhlContourPlotLayer cnl
)
#else
(cnl)
	NhlContourPlotLayer cnl;

#endif
{

	if (! cnl) {
		Cnl = NULL;
		Cnp = NULL;
	}
	else {
		Cnl = cnl;
		Cnp = &cnl->contourplot;
	}
	return;
}
