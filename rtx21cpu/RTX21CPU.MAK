PROJ = RTX21CPU
PROJFILE = RTX21CPU.MAK
DEBUG = 1

PWBRMAKE  = pwbrmake
NMAKEBSC1  = set
NMAKEBSC2  = nmake
BRFLAGS  =  /o $(PROJ).bsc /Es /Em /Iu
BROWSE  = 1
CVFLAGS  =  /50 /F
CFLAGS_G  = /W2 /BATCH /FR$*.sbr
AFLAGS_G  = /W2 /WX /FR$*.sbr
ASM  = ml
AFLAGS_D  = /Zi /Zf /Fl
AFLAGS_R  = /Zf /Fl /nologo
CC  = cl
CFLAGS_D  = /Gi$(PROJ).mdt /Zi /Od
CFLAGS_R  = /Ot /Oi /Ol /Oe /Og /Gs
MAPFILE_D  = $(PROJ).map
MAPFILE_R  = $(PROJ).map
LFLAGS_G  =  /NOI  /BATCH
LFLAGS_D  =  /CO /M /NOF /NOP
LFLAGS_R  =  /CO /M /NOF /NOP
LINKER  = link
ILINK  = ilink
LRF  = echo > NUL

OBJS  = RTX21CPU.obj CONSOLA.obj FX429.obj FX429MSG.obj INTERV25.obj\
        V25INI.obj
SBRS  = RTX21CPU.sbr CONSOLA.sbr FX429.sbr FX429MSG.sbr INTERV25.sbr\
        V25INI.sbr

all: $(PROJ).exe

.SUFFIXES:
.SUFFIXES: .sbr .obj .asm

RTX21CPU.obj : RTX21CPU.ASM constRAC.inc

RTX21CPU.sbr : RTX21CPU.ASM constRAC.inc

CONSOLA.obj : CONSOLA.ASM constRAC.inc

CONSOLA.sbr : CONSOLA.ASM constRAC.inc

FX429.obj : FX429.ASM constRAC.inc

FX429.sbr : FX429.ASM constRAC.inc

FX429MSG.obj : FX429MSG.ASM constRAC.inc

FX429MSG.sbr : FX429MSG.ASM constRAC.inc

INTERV25.obj : INTERV25.ASM constRAC.inc

INTERV25.sbr : INTERV25.ASM constRAC.inc

V25INI.obj : V25INI.ASM constRAC.inc

V25INI.sbr : V25INI.ASM constRAC.inc


$(PROJ).bsc : $(SBRS)
        $(PWBRMAKE) @<<
$(BRFLAGS) $(SBRS)
<<

$(PROJ).exe : $(OBJS)
!IF $(DEBUG)
        $(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_D)
$(LLIBS_G: = +^
) +
$(LLIBS_D: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_D);
<<
!ELSE
        $(LRF) @<<$(PROJ).lrf
$(RT_OBJS: = +^
) $(OBJS: = +^
)
$@
$(MAPFILE_R)
$(LLIBS_G: = +^
) +
$(LLIBS_R: = +^
) +
$(LIBS: = +^
)
$(DEF_FILE) $(LFLAGS_G) $(LFLAGS_R);
<<
!ENDIF
        $(LINKER) @$(PROJ).lrf
        $(NMAKEBSC1) MAKEFLAGS=
        $(NMAKEBSC2) $(NMFLAGS) -f $(PROJFILE) $(PROJ).bsc


.asm.sbr :
!IF $(DEBUG)
        $(ASM) /Zs $(AFLAGS_G) $(AFLAGS_D) /FR$@ $<
!ELSE
        $(ASM) /Zs $(AFLAGS_G) $(AFLAGS_R) /FR$@ $<
!ENDIF

.asm.obj :
!IF $(DEBUG)
        $(ASM) /c $(AFLAGS_G) $(AFLAGS_D) /Fo$@ $<
!ELSE
        $(ASM) /c $(AFLAGS_G) $(AFLAGS_R) /Fo$@ $<
!ENDIF


run: $(PROJ).exe
        $(PROJ).exe $(RUNFLAGS)

debug: $(PROJ).exe
        CV $(CVFLAGS) $(PROJ).exe $(RUNFLAGS)
