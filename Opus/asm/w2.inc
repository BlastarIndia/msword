;       W2.INC
;
;       Standard include file for all opus assembly files.
;       (Parallels w2.n).
;


ifdef DEBUG
PROFILE=1
endif

ifdef PROFILE
ifndef ?USEBP
; Enable PROFILING of native routines
?PROFILE = 1
endif
endif



