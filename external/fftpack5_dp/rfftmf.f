CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC
C
C   FFTPACK 5.0
C   Copyright (C) 1995-2004, Scientific Computing Division,
C   University Corporation for Atmospheric Research
C   Licensed under the GNU General Public License (GPL)
C
C   Authors:  Paul N. Swarztrauber and Richard A. Valent
C
C   $Id: rfftmf.f,v 1.2 2006-11-21 01:10:19 haley Exp $
C
CCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCC

      SUBROUTINE DRFFTMF(LOT,JUMP,N,INC,R,LENR,WSAVE,LENSAV,WORK,
     +                   LENWRK,IER)
      INTEGER LOT,JUMP,N,INC,LENR,LENSAV,LENWRK,IER
      DOUBLE PRECISION R(LENR),WSAVE(LENSAV),WORK(LENWRK)
      LOGICAL DXERCON
C
      IER = 0
C
      IF (LENR.LT. (LOT-1)*JUMP+INC* (N-1)+1) THEN
          IER = 1
          CALL DXERFFT('RFFTMF ',6)
      ELSE IF (LENSAV.LT.N+INT(LOG(DBLE(N)))+4) THEN
          IER = 2
          CALL DXERFFT('RFFTMF ',8)
      ELSE IF (LENWRK.LT.LOT*N) THEN
          IER = 3
          CALL DXERFFT('RFFTMF ',10)
      ELSE IF (.NOT.DXERCON(INC,JUMP,N,LOT)) THEN
          IER = 4
          CALL DXERFFT('RFFTMF ',-1)
      END IF
C
      IF (N.EQ.1) RETURN
C
      CALL DMRFTF1(LOT,JUMP,N,INC,R,WORK,WSAVE,WSAVE(N+1))
      RETURN
      END
