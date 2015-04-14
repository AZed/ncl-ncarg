CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C   FFTPACK 5.0
C   Copyright (C) 1995-2004, Scientific Computing Division,
C   University Corporation for Atmospheric Research
C   Licensed under the GNU General Public License (GPL)
C
C   Authors:  Paul N. Swarztrauber and Richard A. Valent
C
C   $Id: cosqmf.f,v 1.2 2006/11/21 01:10:17 haley Exp $
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

      SUBROUTINE DCOSQMF(LOT,JUMP,N,INC,X,LENX,WSAVE,LENSAV,WORK,
     +                   LENWRK,IER)
      DOUBLE PRECISION SSQRT2
      DOUBLE PRECISION TSQX
      INTEGER LOT,JUMP,N,INC,LENX,LENSAV,LENWRK,IER
      DOUBLE PRECISION X(INC,*),WSAVE(LENSAV),WORK(LENWRK)
      LOGICAL DXERCON
C
      IER = 0
C
      IF (LENX.LT. (LOT-1)*JUMP+INC* (N-1)+1) THEN
          IER = 1
          CALL DXERFFT('COSQMF',6)
          GO TO 300
      ELSE IF (LENSAV.LT.2*N+INT(LOG(DBLE(N)))+4) THEN
          IER = 2
          CALL DXERFFT('COSQMF',8)
          GO TO 300
      ELSE IF (LENWRK.LT.LOT*N) THEN
          IER = 3
          CALL DXERFFT('COSQMF',10)
          GO TO 300
      ELSE IF (.NOT.DXERCON(INC,JUMP,N,LOT)) THEN
          IER = 4
          CALL DXERFFT('COSQMF',-1)
          GO TO 300
      END IF
C
      LJ = (LOT-1)*JUMP + 1
      IF (N-2) 102,101,103
  101 SSQRT2 = 1.D0/SQRT(2.D0)
      DO 201 M = 1,LJ,JUMP
          TSQX = SSQRT2*X(M,2)
          X(M,2) = .5D0*X(M,1) - TSQX
          X(M,1) = .5D0*X(M,1) + TSQX
  201 CONTINUE
  102 RETURN
  103 CALL DMCSQF1(LOT,JUMP,N,INC,X,WSAVE,WORK,IER1)
      IF (IER1.NE.0) THEN
          IER = 20
          CALL DXERFFT('COSQMF',-5)
      END IF
C
  300 CONTINUE
      RETURN
      END
