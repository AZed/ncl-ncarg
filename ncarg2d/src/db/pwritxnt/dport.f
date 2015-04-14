C
C	$Id: dport.f,v 1.5 2008/07/27 01:04:31 haley Exp $
C                                                                      
C                Copyright (C)  2000
C        University Corporation for Atmospheric Research
C                All Rights Reserved
C
C The use of this Software is governed by a License Agreement.
C

      SUBROUTINE DPORT
C
C Calling this do-nothing subroutine forces "ld" to load the following
C block data routine (but only if they are in the same ".f" file).
C
        RETURN
C
      END
CNOSPLIT
      BLOCKDATA DPORTX
C
C FOR INITIALIZATION OF CONSTANTS.
C
C
C
C FOR INITIALIZATION OF CONSTANTS IN ROUTINES CREBIN AND CREB15.
      COMMON /IDC1/ NBWD, IZERO, MA15
C FOR INITIALIZATION OF CONSTANTS IN ROUTINE CREBIN.
      COMMON /IDC2/ MASK15(15)
C FOR INITIALIZATION OF CONSTANTS IN THE MAIN PROGRAM.
      COMMON /IDC3/ IU1,IU2,IU3,IU4,IOUT,ICNUM1,ICNUM2,IDDLEN,INDLEN
C FOR INITIALIZATION OF CONSTANTS IN ROUTINE XTCH.
      COMMON /PWRC2/ INDZER
C  NOTE THAT IWORK MAY BE CHANGED TO CONTAIN THE NUMBER OF ELEMENTS
C  EQUAL TO THE VALUE OF LENWOR.
      COMMON/IDC0/LENWOR , IWORK(8626)
C
C  INITIALIZATION FLAG FOR PCRBIN
C
      COMMON/PINIT2/FLAG
      LOGICAL FLAG
C
C  INITIALIZATION FOR MASK GENERATION
C
      COMMON/PINIT1/IFRST
      LOGICAL IFRST
C
C  MASKING AND OTHER MACHINE DEPENDENT CONSTANTS WHICH MUST BE
C  CALCULATED.
C
      COMMON/PINIT/MASK(4),MASK6(64),MASK12(64),LAST6,LAST12,IBIT15,
     1     MASK14,NUM15U,NBWD1
C
C  FLAG IS USED BY PCRBIN TO DECIDE IF INITIALIZATION HAS BEEN
C  COMPLEATED
C
      DATA FLAG/.FALSE./
C
C  IFRST IS USED BY XTCH TO DECIDE IF MASKS HAVE BEEN GENERATED
C
      DATA IFRST/.FALSE./
C
C  MA15 MASKS THE RIGHTMOST 15 BITS OF A WORD(77777B)
C
      DATA MA15/32767/
C
      DATA IZERO/0/
C
C
C
C THE NUMBER OF CARD IMAGES CONTAINED IN FILES C1.PWRITX AND D1.PWRITX.
      DATA ICNUM1 /49/
C THE NUMBER OF CARD IMAGES CONTAINED IN FILES C2.PWRITX AND D2.PWRITX.
      DATA ICNUM2 /575/
C A SPECIAL INDICATOR.
      DATA INDZER /-2048/
C
C
C **********************************************************************
C
C IMPLEMENTATION DEPENDENT CONSTANTS
C
C THE UNIT NUMBER WHERE FILE PWRITX.C1 CAN BE READ.
      DATA IU1 /1/
C THE UNIT NUMBER WHERE FILE PWRITX.C2 CAN BE READ.
      DATA IU2 /2/
C THE UNIT NUMBER WHERE FILE PWRITX.D1 CAN BE READ.
      DATA IU3 /3/
C THE UNIT NUMBER WHERE FILE PWRITX.D2 CAN BE READ.
      DATA IU4 /4/
C
C THE UNIT NUMBER WHERE PROGRAM PCRBIN CAN WRITE THE BINARY FILE
C IT CREATES.
      DATA IOUT /7/
C
C
C END OF IMPLEMENTATION DEPENDENT CONSTANTS
C
C **********************************************************************
C
      END
