      DOUBLE PRECISION FUNCTION DSECND( )
*
*  -- LAPACK auxiliary routine (version 3.0) --
*     Univ. of Tennessee, Univ. of California Berkeley, NAG Ltd.,
*     Courant Institute, Argonne National Lab, and Rice University
*     September 30, 1994
*
*  Purpose
*  =======
*
*  DSECND returns the user time for a process in seconds.
*  This version gets the time from the system function ETIME.
*
* =====================================================================
*
*     .. Local Scalars ..
      REAL               T1
*     ..
*     .. Local Arrays ..
      REAL               TARRAY( 2 )
*     ..
*     .. External Functions ..
*     NOTE: On IBMs, you may have to change ETIME to ETIME_
      REAL               ETIME_
      EXTERNAL           ETIME_
*     ..
*     .. Executable Statements ..
*
      T1 = ETIME_( TARRAY )
      DSECND = TARRAY( 1 )
      RETURN
*
*     End of DSECND
*
      END
