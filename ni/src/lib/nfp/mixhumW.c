#include <stdio.h>
#include "wrapper.h"

extern void NGCALLF(dmixhum1,DMIXHUM1)(double *,double *,double *,int *,
                                       double *);

extern void NGCALLF(dwmrq,DWMRQ)(double *,double *,double *,int *,
                                 double *,int *);

extern void NGCALLF(ddewtemp,DDEWTEMP)(double *,double *,double *);

extern void NGCALLF(dlclprs,DLCLPRS)(double *,double *,double *,double *);

NhlErrorTypes mixhum_ptrh_W( void )
{
/*
 * Input array variables
 */
  void *p, *tk, *rh;
  int *iswit;
  double *tmp_p, *tmp_tk, *tmp_rh;
  int ndims_p, dsizes_p[NCL_MAX_DIMENSIONS], has_missing_p;
  int ndims_tk, dsizes_tk[NCL_MAX_DIMENSIONS], has_missing_tk;
  int ndims_rh, dsizes_rh[NCL_MAX_DIMENSIONS], has_missing_rh;
  NclBasicDataTypes type_p, type_tk, type_rh;
  NclScalar missing_p, missing_tk, missing_rh;
  NclScalar missing_dp, missing_dtk, missing_drh;
  int found_missing_p, found_missing_tk, found_missing_rh, any_missing;
/*
 * Output array variables
 */
  void *qw;
  double *tmp_qw;
  NclBasicDataTypes type_qw;
  NclScalar missing_qw;
/*
 * Random variables.
 */
  int i, size_input;
/*
 * Retrieve parameters.
 *
 * Note that any of the pointer parameters can be set to NULL,
 * which implies you don't care about its value.
 */
  p = (void*)NclGetArgValue(
          0,
          4,
          &ndims_p,
          dsizes_p,
          &missing_p,
          &has_missing_p,
          &type_p,
          2);

  tk = (void*)NclGetArgValue(
          1,
          4,
          &ndims_tk,
          dsizes_tk,
          &missing_tk,
          &has_missing_tk,
          &type_tk,
          2);

  rh = (void*)NclGetArgValue(
          2,
          4,
          &ndims_rh,
          dsizes_rh,
          &missing_rh,
          &has_missing_rh,
          &type_rh,
          2);

  iswit = (int*)NclGetArgValue(
          3,
          4,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          2);
/*
 * All dimensions of all input arguments must be the same.
 */
  if(ndims_p != ndims_tk || ndims_tk != ndims_rh) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"mixhum_ptrh: The first three input arrays must have the same dimensions");
    return(NhlFATAL);
  }
  for( i = 0; i < ndims_p; i++ ) {
    if (dsizes_p[i] != dsizes_tk[i] || dsizes_tk[i] != dsizes_rh[i]) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"mixhum_ptrh: The first three input arrays must have the same dimensions");
      return(NhlFATAL);
    }
  }

/*
 * Compute the total size of the input arrays.
 */
  size_input = 1;
  for( i = 0; i < ndims_p; i++ ) size_input *= dsizes_p[i];

/*
 * allocate space for temporary input/output arrays.
 */
  if(type_p != NCL_double) {
    tmp_p = (double*)calloc(1,sizeof(double));
    if(tmp_p == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"mixhum_ptrh: Unable to allocate memory for coercing 'p' array to double precision");
      return(NhlFATAL);
    }
  }
  if(type_tk != NCL_double) {
    tmp_tk = (double*)calloc(1,sizeof(double));
    if(tmp_tk == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"mixhum_ptrh: Unable to allocate memory for coercing 'tk' array to double precision");
      return(NhlFATAL);
    }
  }
  if(type_rh != NCL_double) {
    tmp_rh = (double*)calloc(1,sizeof(double));
    if(tmp_rh == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"mixhum_ptrh: Unable to allocate memory for coercing 'rh' array to double precision");
      return(NhlFATAL);
    }
  }
  if(type_p != NCL_double && type_tk != NCL_double && type_rh != NCL_double) {
    type_qw = NCL_float;
    qw      = (void*)calloc(size_input,sizeof(float));
    tmp_qw  = (double*)calloc(1,sizeof(double));
    if(qw == NULL || tmp_qw == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"mixhum_ptrh: Unable to allocate memory for output array");
      return(NhlFATAL);
    }    
  }
  else {
    type_qw = NCL_double;
    qw = (void*)calloc(size_input,sizeof(double));
    if(qw == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"mixhum_ptrh: Unable to allocate memory for output array");
      return(NhlFATAL);
    }    
  }
/*
 * Coerce missing values to double if necessary.
 */
  coerce_missing(type_tk,has_missing_tk,&missing_tk,&missing_dtk,NULL);
  coerce_missing(type_p,has_missing_p,&missing_p,&missing_dp,NULL);
  coerce_missing(type_rh,has_missing_rh,&missing_rh,&missing_drh,NULL);

  if(has_missing_tk || has_missing_p || has_missing_rh) {
    if(type_qw == NCL_double) {
      missing_qw.doubleval = ((NclTypeClass)nclTypedoubleClass)->type_class.default_mis.doubleval;
    }
    else {
      missing_qw.floatval = ((NclTypeClass)nclTypefloatClass)->type_class.default_mis.floatval;
    }
  }
/*
 * Call the Fortran version of this routine.
 */
  any_missing = 0;
  for( i = 0; i < size_input; i++ ) {
    if(type_p != NCL_double) {
/*
 * Coerce one value of p (tmp_p) to double.
 */
      coerce_subset_input_double(p,tmp_p,i,type_p,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_p to appropriate location in p.
 */
      tmp_p = &((double*)p)[i];
    }
    if(type_tk != NCL_double) {
/*
 * Coerce one value of tk (tmp_tk) to double.
 */
      coerce_subset_input_double(tk,tmp_tk,i,type_tk,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_tk to appropriate location in tk.
 */
      tmp_tk = &((double*)tk)[i];
    }
    if(type_rh != NCL_double) {
/*
 * Coerce one value of rh (tmp_rh) to double.
 */
      coerce_subset_input_double(rh,tmp_rh,i,type_rh,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_rh to appropriate location in rh.
 */
      tmp_rh = &((double*)rh)[i];
    }

    if(type_qw == NCL_double) tmp_qw = &((double*)qw)[i];

/*
 * Test for missing values.
 */
    found_missing_p  = (*tmp_p  == missing_dp.doubleval)  ? 1 : 0;
    found_missing_tk = (*tmp_tk == missing_dtk.doubleval) ? 1 : 0;
    found_missing_rh = (*tmp_rh == missing_drh.doubleval) ? 1 : 0;

    if(found_missing_p || found_missing_tk || found_missing_rh) {
      any_missing = 1;
      if(type_qw == NCL_double) {
        set_subset_output_missing(qw,i,type_qw,1,missing_qw.doubleval);
      }
      else {
        set_subset_output_missing(qw,i,type_qw,1,(double)missing_qw.floatval);
      }
    }
    else {
      NGCALLF(dmixhum1,DMIXHUM1)(tmp_p,tmp_tk,tmp_rh,iswit,tmp_qw);
/*
 * Copy output values from temporary tmp_qw to qw.
 */
      if(type_qw != NCL_double) {
        ((float*)qw)[i] = (float)(*tmp_qw);
      }
    }
  }
/*
 * free memory.
 */
  if(type_p  != NCL_double) NclFree(tmp_p);
  if(type_tk != NCL_double) NclFree(tmp_tk);
  if(type_rh != NCL_double) NclFree(tmp_rh);
  if(type_qw != NCL_double) NclFree(tmp_qw);

  if(any_missing) {
    NhlPError(NhlWARNING,NhlEUNKNOWN,"mixhum_ptrh: one or more sets of input values contained a missing value. No mixing ratio/specific humidity calculated for these sets of values.");
    return(NclReturnValue(qw,ndims_p,dsizes_p,&missing_qw,type_qw,0));
  }
  else {
    return(NclReturnValue(qw,ndims_p,dsizes_p,NULL,type_qw,0));
  }
}

NhlErrorTypes dewtemp_trh_W( void )
{
/*
 * Input array variables
 */
  void *tk, *rh;
  double *tmp_tk, *tmp_rh;
  int ndims_tk, dsizes_tk[NCL_MAX_DIMENSIONS], has_missing_tk;
  int ndims_rh, dsizes_rh[NCL_MAX_DIMENSIONS], has_missing_rh;
  NclBasicDataTypes type_tk, type_rh;
  NclScalar missing_tk, missing_rh;
  NclScalar missing_dtk, missing_drh;
  int found_missing_tk, found_missing_rh, any_missing;
/*
 * Output array variables
 */
  void *tdk;
  double *tmp_tdk;
  NclBasicDataTypes type_tdk;
  NclScalar missing_tdk;
/*
 * Random variables.
 */
  int i, size_input;
/*
 * Retrieve parameters.
 *
 * Note that any of the pointer parameters can be set to NULL,
 * which implies you don't care about its value.
 */
  tk = (void*)NclGetArgValue(
          0,
          2,
          &ndims_tk,
          dsizes_tk,
          &missing_tk,
          &has_missing_tk,
          &type_tk,
          2);

  rh = (void*)NclGetArgValue(
          1,
          2,
          &ndims_rh,
          dsizes_rh,
          &missing_rh,
          &has_missing_rh,
          &type_rh,
          2);

/*
 * Dimensions of input arguments must be the same.
 */
  if(ndims_tk != ndims_rh) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"dewtemp_trh: The input arrays must have the same dimensions");
    return(NhlFATAL);
  }
  for( i = 0; i < ndims_tk; i++ ) {
    if (dsizes_tk[i] != dsizes_rh[i]) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"dewtemp_trh: The input arrays must have the same dimensions");
      return(NhlFATAL);
    }
  }

/*
 * Compute the total size of the input arrays.
 */
  size_input = 1;
  for( i = 0; i < ndims_tk; i++ ) size_input *= dsizes_tk[i];

/*
 * allocate space for temporary input/output arrays.
 */
  if(type_tk != NCL_double) {
    tmp_tk = (double*)calloc(1,sizeof(double));
    if(tmp_tk == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"dewtemp_trh: Unable to allocate memory for coercing 'tk' array to double precision");
      return(NhlFATAL);
    }
  }
  if(type_rh != NCL_double) {
    tmp_rh = (double*)calloc(1,sizeof(double));
    if(tmp_rh == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"dewtemp_trh: Unable to allocate memory for coercing 'rh' array to double precision");
      return(NhlFATAL);
    }
  }
  if(type_tk != NCL_double && type_rh != NCL_double) {
    type_tdk = NCL_float;
    tdk      = (void*)calloc(size_input,sizeof(float));
    tmp_tdk  = (double*)calloc(1,sizeof(double));
    if(tdk == NULL || tmp_tdk == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"dewtemp_trh: Unable to allocate memory for output array");
      return(NhlFATAL);
    }    
  }
  else {
    type_tdk = NCL_double;
    tdk = (void*)calloc(size_input,sizeof(double));
    if(tdk == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"dewtemp_trh: Unable to allocate memory for output array");
      return(NhlFATAL);
    }    
  }
/*
 * Coerce missing values to double if necessary.
 */
  coerce_missing(type_tk,has_missing_tk,&missing_tk,&missing_dtk,NULL);
  coerce_missing(type_rh,has_missing_rh,&missing_rh,&missing_drh,NULL);

  if(has_missing_tk || has_missing_rh) {
    if(type_tdk == NCL_double) {
      missing_tdk.doubleval = ((NclTypeClass)nclTypedoubleClass)->type_class.default_mis.doubleval;
    }
    else {
      missing_tdk.floatval = ((NclTypeClass)nclTypefloatClass)->type_class.default_mis.floatval;
    }
  }
/*
 * Call the Fortran version of this routine.
 */
  any_missing = 0;
  for( i = 0; i < size_input; i++ ) {
    if(type_tk != NCL_double) {
/*
 * Coerce one value of tk (tmp_tk) to double.
 */
      coerce_subset_input_double(tk,tmp_tk,i,type_tk,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_tk to appropriate location in tk.
 */
      tmp_tk = &((double*)tk)[i];
    }
    if(type_rh != NCL_double) {
/*
 * Coerce one value of rh (tmp_rh) to double.
 */
      coerce_subset_input_double(rh,tmp_rh,i,type_rh,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_rh to appropriate location in rh.
 */
      tmp_rh = &((double*)rh)[i];
    }

    if(type_tdk == NCL_double) tmp_tdk = &((double*)tdk)[i];

/*
 * Test for missing values.
 */
    found_missing_tk = (*tmp_tk == missing_dtk.doubleval) ? 1 : 0;
    found_missing_rh = (*tmp_rh == missing_drh.doubleval) ? 1 : 0;

    if(found_missing_tk || found_missing_rh) {
      any_missing = 1;
      if(type_tdk == NCL_double) {
        set_subset_output_missing(tdk,i,type_tdk,1,missing_tdk.doubleval);
      }
      else {
        set_subset_output_missing(tdk,i,type_tdk,1,(double)missing_tdk.floatval);
      }
    }
    else {
      NGCALLF(ddewtemp,DDEWTEMP)(tmp_tk,tmp_rh,tmp_tdk);
/*
 * Copy output values from temporary tmp_tdk to tdk.
 */
      if(type_tdk != NCL_double) {
        ((float*)tdk)[i] = (float)(*tmp_tdk);
      }
    }
  }
/*
 * free memory.
 */
  if(type_tk != NCL_double) NclFree(tmp_tk);
  if(type_rh != NCL_double) NclFree(tmp_rh);
  if(type_tdk != NCL_double) NclFree(tmp_tdk);

  if(any_missing) {
    NhlPError(NhlWARNING,NhlEUNKNOWN,"dewtemp_trh: one or more sets of input values contained a missing value. No mixing ratio/specific humidity calculated for these sets of values.");
    return(NclReturnValue(tdk,ndims_tk,dsizes_tk,&missing_tdk,type_tdk,0));
  }
  else {
    return(NclReturnValue(tdk,ndims_tk,dsizes_tk,NULL,type_tdk,0));
  }
}


NhlErrorTypes lclvl_W( void )
{
/*
 * Input array variables
 */
  void *p, *tk, *tdk;
  double *tmp_p, *tmp_tk, *tmp_tdk;
  int ndims_p, dsizes_p[NCL_MAX_DIMENSIONS], has_missing_p;
  int ndims_tk, dsizes_tk[NCL_MAX_DIMENSIONS], has_missing_tk;
  int ndims_tdk, dsizes_tdk[NCL_MAX_DIMENSIONS], has_missing_tdk;
  NclBasicDataTypes type_p, type_tk, type_tdk;
  NclScalar missing_p, missing_tk, missing_tdk;
  NclScalar missing_dp, missing_dtk, missing_dtdk;
  int found_missing_p, found_missing_tk, found_missing_tdk, any_missing;
/*
 * Output array variables
 */
  void *plcl;
  double *tmp_plcl;
  NclBasicDataTypes type_plcl;
  NclScalar missing_plcl;
/*
 * Random variables.
 */
  int i, size_input;
/*
 * Retrieve parameters.
 *
 * Note that any of the pointer parameters can be set to NULL,
 * which implies you don't care about its value.
 */
  p = (void*)NclGetArgValue(
          0,
          3,
          &ndims_p,
          dsizes_p,
          &missing_p,
          &has_missing_p,
          &type_p,
          2);

  tk = (void*)NclGetArgValue(
          1,
          3,
          &ndims_tk,
          dsizes_tk,
          &missing_tk,
          &has_missing_tk,
          &type_tk,
          2);

  tdk = (void*)NclGetArgValue(
          2,
          3,
          &ndims_tdk,
          dsizes_tdk,
          &missing_tdk,
          &has_missing_tdk,
          &type_tdk,
          2);

/*
 * All dimensions of all input arguments must be the same.
 */
  if(ndims_p != ndims_tk || ndims_tk != ndims_tdk) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"lclvl: The first three input arrays must have the same dimensions");
    return(NhlFATAL);
  }
  for( i = 0; i < ndims_p; i++ ) {
    if (dsizes_p[i] != dsizes_tk[i] || dsizes_tk[i] != dsizes_tdk[i]) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"lclvl: The first three input arrays must have the same dimensions");
      return(NhlFATAL);
    }
  }

/*
 * Compute the total size of the input arrays.
 */
  size_input = 1;
  for( i = 0; i < ndims_p; i++ ) size_input *= dsizes_p[i];

/*
 * allocate space for temporary input/output arrays.
 */
  if(type_p != NCL_double) {
    tmp_p = (double*)calloc(1,sizeof(double));
    if(tmp_p == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"lclvl: Unable to allocate memory for coercing 'p' array to double precision");
      return(NhlFATAL);
    }
  }
  if(type_tk != NCL_double) {
    tmp_tk = (double*)calloc(1,sizeof(double));
    if(tmp_tk == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"lclvl: Unable to allocate memory for coercing 'tk' array to double precision");
      return(NhlFATAL);
    }
  }
  if(type_tdk != NCL_double) {
    tmp_tdk = (double*)calloc(1,sizeof(double));
    if(tmp_tdk == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"lclvl: Unable to allocate memory for coercing 'tdk' array to double precision");
      return(NhlFATAL);
    }
  }
  if(type_p != NCL_double && type_tk != NCL_double && type_tdk != NCL_double) {
    type_plcl = NCL_float;
    plcl      = (void*)calloc(size_input,sizeof(float));
    tmp_plcl  = (double*)calloc(1,sizeof(double));
    if(plcl == NULL || tmp_plcl == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"lclvl: Unable to allocate memory for output array");
      return(NhlFATAL);
    }    
  }
  else {
    type_plcl = NCL_double;
    plcl = (void*)calloc(size_input,sizeof(double));
    if(plcl == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"lclvl: Unable to allocate memory for output array");
      return(NhlFATAL);
    }    
  }
/*
 * Coerce missing values to double if necessary.
 */
  coerce_missing(type_tk,has_missing_tk,&missing_tk,&missing_dtk,NULL);
  coerce_missing(type_p,has_missing_p,&missing_p,&missing_dp,NULL);
  coerce_missing(type_tdk,has_missing_tdk,&missing_tdk,&missing_dtdk,NULL);

  if(has_missing_tk || has_missing_p || has_missing_tdk) {
    if(type_plcl == NCL_double) {
      missing_plcl.doubleval = ((NclTypeClass)nclTypedoubleClass)->type_class.default_mis.doubleval;
    }
    else {
      missing_plcl.floatval = ((NclTypeClass)nclTypefloatClass)->type_class.default_mis.floatval;
    }
  }
/*
 * Call the Fortran version of this routine.
 */
  any_missing = 0;
  for( i = 0; i < size_input; i++ ) {
    if(type_p != NCL_double) {
/*
 * Coerce one value of p (tmp_p) to double.
 */
      coerce_subset_input_double(p,tmp_p,i,type_p,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_p to appropriate location in p.
 */
      tmp_p = &((double*)p)[i];
    }
    if(type_tk != NCL_double) {
/*
 * Coerce one value of tk (tmp_tk) to double.
 */
      coerce_subset_input_double(tk,tmp_tk,i,type_tk,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_tk to appropriate location in tk.
 */
      tmp_tk = &((double*)tk)[i];
    }
    if(type_tdk != NCL_double) {
/*
 * Coerce one value of tdk (tmp_tdk) to double.
 */
      coerce_subset_input_double(tdk,tmp_tdk,i,type_tdk,1,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_tdk to appropriate location in tdk.
 */
      tmp_tdk = &((double*)tdk)[i];
    }

    if(type_plcl == NCL_double) tmp_plcl = &((double*)plcl)[i];

/*
 * Test for missing values.
 */
    found_missing_p   = (*tmp_p   == missing_dp.doubleval)   ? 1 : 0;
    found_missing_tk  = (*tmp_tk  == missing_dtk.doubleval)  ? 1 : 0;
    found_missing_tdk = (*tmp_tdk == missing_dtdk.doubleval) ? 1 : 0;

    if(found_missing_p || found_missing_tk || found_missing_tdk) {
      any_missing = 1;
      if(type_plcl == NCL_double) {
        set_subset_output_missing(plcl,i,type_plcl,1,missing_plcl.doubleval);
      }
      else {
        set_subset_output_missing(plcl,i,type_plcl,1,(double)missing_plcl.floatval);
      }
    }
    else {
      NGCALLF(dlclprs,DLCLPRS)(tmp_p,tmp_tk,tmp_tdk,tmp_plcl);
/*
 * Copy output values from temporary tmp_plcl to plcl.
 */
      if(type_plcl != NCL_double) {
        ((float*)plcl)[i] = (float)(*tmp_plcl);
      }
    }
  }
/*
 * free memory.
 */
  if(type_p    != NCL_double) NclFree(tmp_p);
  if(type_tk   != NCL_double) NclFree(tmp_tk);
  if(type_tdk  != NCL_double) NclFree(tmp_tdk);
  if(type_plcl != NCL_double) NclFree(tmp_plcl);

  if(any_missing) {
    NhlPError(NhlWARNING,NhlEUNKNOWN,"lclvl: one or more sets of input values contained a missing value. No mixing ratio/specific humidity calculated for these sets of values.");
    return(NclReturnValue(plcl,ndims_p,dsizes_p,&missing_plcl,type_plcl,0));
  }
  else {
    return(NclReturnValue(plcl,ndims_p,dsizes_p,NULL,type_plcl,0));
  }
}


NhlErrorTypes mixhum_ptd_W( void )
{
/*
 * Input array variables
 */
  void *p, *tdk;
  double *tmp_p, *tmp_tdk;
  int *iswit;
  int ndims_p, dsizes_p[NCL_MAX_DIMENSIONS], has_missing_p;
  int ndims_tdk, dsizes_tdk[NCL_MAX_DIMENSIONS], has_missing_tdk;
  NclBasicDataTypes type_p, type_tdk;
  NclScalar missing_p, missing_tdk;
  NclScalar missing_dp, missing_dtdk;
  int found_missing_p, found_missing_tdk, any_missing;
/*
 * Output array variables
 */
  void *q;
  double *tmp_q;
  NclBasicDataTypes type_q;
  NclScalar missing_q;
/*
 * Random variables.
 */
  int i, index, size_q, size_leftmost, nmax;
/*
 * Retrieve parameters.
 *
 * Note that any of the pointer parameters can be set to NULL,
 * which implies you don't care about its value.
 */
  p = (void*)NclGetArgValue(
          0,
          3,
          &ndims_p,
          dsizes_p,
          &missing_p,
          &has_missing_p,
          &type_p,
          2);

  tdk = (void*)NclGetArgValue(
          1,
          3,
          &ndims_tdk,
          dsizes_tdk,
          &missing_tdk,
          &has_missing_tdk,
          &type_tdk,
          2);

  iswit = (int*)NclGetArgValue(
          2,
          3,
          NULL,
          NULL,
          NULL,
          NULL,
          NULL,
          2);

/*
 * tdk and p must be same dimensioniality.
 */
  if(ndims_p != ndims_tdk) {
    NhlPError(NhlFATAL,NhlEUNKNOWN,"mixhum_ptd: The two input arrays must have the same dimensions");
    return(NhlFATAL);
  }
  for( i = 0; i < ndims_p; i++ ) {
    if (dsizes_p[i] != dsizes_tdk[i]) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"mixhum_ptd: The two input arrays must have the same dimensions");
      return(NhlFATAL);
    }
  }

/*
 * Compute the total size of the input arrays.
 */
  size_leftmost = 1;
  for( i = 0; i < ndims_p-1; i++ ) size_leftmost *= dsizes_p[i];

  nmax   = dsizes_p[ndims_p-1];
  size_q = size_leftmost * nmax;

/*
 * Allocate space for temporary input/output arrays.
 */
  if(type_p != NCL_double) {
    tmp_p = (double*)calloc(nmax,sizeof(double));
    if(tmp_p == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"mixhum_ptd: Unable to allocate memory for coercing 'p' array to double precision");
      return(NhlFATAL);
    }
  }
  if(type_tdk != NCL_double) {
    tmp_tdk = (double*)calloc(nmax,sizeof(double));
    if(tmp_tdk == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"mixhum_ptd: Unable to allocate memory for coercing 'tdk' array to double precision");
      return(NhlFATAL);
    }
  }

  if(type_p != NCL_double && type_tdk != NCL_double) {
    type_q = NCL_float;
    q      = (void*)calloc(size_q,sizeof(float));
    tmp_q  = (double*)calloc(nmax,sizeof(double));
    if(q == NULL || tmp_q == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"mixhum_ptd: Unable to allocate memory for output array");
      return(NhlFATAL);
    }    
  }
  else {
    type_q = NCL_double;
    q = (void*)calloc(size_q,sizeof(double));
    if(q == NULL) {
      NhlPError(NhlFATAL,NhlEUNKNOWN,"mixhum_ptd: Unable to allocate memory for output array");
      return(NhlFATAL);
    }    
  }
/*
 * Coerce missing values to double if necessary.
 */
  coerce_missing(type_tdk,has_missing_tdk,&missing_tdk,&missing_dtdk,NULL);
  coerce_missing(type_p,has_missing_p,&missing_p,&missing_dp,NULL);

  if(has_missing_tdk || has_missing_p) {
    if(type_q == NCL_double) {
      missing_q.doubleval = ((NclTypeClass)nclTypedoubleClass)->type_class.default_mis.doubleval;
    }
    else {
      missing_q.floatval = ((NclTypeClass)nclTypefloatClass)->type_class.default_mis.floatval;
    }
  }
/*
 * Call the Fortran version of this routine.
 */
  index = any_missing = 0;
  for( i = 0; i < size_leftmost; i++ ) {
    if(type_p != NCL_double) {
/*
 * Coerce one value of p (tmp_p) to double.
 */
      coerce_subset_input_double(p,tmp_p,index,type_p,nmax,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_p to appropriate location in p.
 */
      tmp_p = &((double*)p)[index];
    }
    if(type_tdk != NCL_double) {
/*
 * Coerce one value of tk (tmp_tdk) to double.
 */
      coerce_subset_input_double(tdk,tmp_tdk,index,type_tdk,nmax,0,NULL,NULL);
    }
    else {
/*
 * Point tmp_tdk to appropriate location in tdk.
 */
      tmp_tdk = &((double*)tdk)[index];
    }

    if(type_q == NCL_double) tmp_q = &((double*)q)[index];
/*
 * Test for missing values.
 */
    found_missing_p   = (*tmp_p  == missing_dp.doubleval)  ? 1 : 0;
    found_missing_tdk = (*tmp_tdk == missing_dtdk.doubleval) ? 1 : 0;

    if(found_missing_p || found_missing_tdk) {
      any_missing = 1;
      if(type_q == NCL_double) {
        set_subset_output_missing(q,index,type_q,nmax,missing_q.doubleval);
      }
      else {
        set_subset_output_missing(q,index,type_q,nmax,(double)missing_q.floatval);
      }
    }
    else {
      NGCALLF(dwmrq,DWMRQ)(tmp_p,tmp_tdk,&missing_dtdk.doubleval,&nmax,tmp_q,
                           iswit);
/*
 * Copy output values from temporary tmp_q to q.
 */
      if(type_q != NCL_double) {
        coerce_output_float_only(q,tmp_q,nmax,index);
      }
    }
    index += nmax;
  }
/*
 * Free memory.
 */
  if(type_p   != NCL_double) NclFree(tmp_p);
  if(type_tdk != NCL_double) NclFree(tmp_tdk);
  if(type_q   != NCL_double) NclFree(tmp_q);

  if(any_missing) {
    NhlPError(NhlWARNING,NhlEUNKNOWN,"mixhum_ptd: one or more sets of input values contained a missing value. No mixing ratio/specific humidity calculated for these sets of values.");
    return(NclReturnValue(q,ndims_p,dsizes_p,&missing_q,type_q,0));
  }
  else {
    return(NclReturnValue(q,ndims_p,dsizes_p,NULL,type_q,0));
  }
}

