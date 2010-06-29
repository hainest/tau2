s/\(.*\)#ENDIF#\(.*\)/\2\1#ENDIF#/g
s/^CONFIG_ARCH=\(.*\)/CONFIG_ARCH=default/g
s/^TAU_ARCH=\(.*\)/TAU_ARCH=default/g
s@^TAUROOT=\(.*\)@TAUROOT=@g
s@^TULIPDIR=\(.*\)@TULIPDIR=@g
s@^TAUGCCLIBOPTS=\(.*\)@TAUGCCLIBOPTS=@g
s@^TAUGCCLIBDIR=\(.*\)@TAUGCCLIBDIR=@g
s@^TAUGFORTRANLIBDIR=\(.*\)@TAUGFORTRANLIBDIR=@g
s@^PAPIDIR=\(.*\)@PAPIDIR=@g
s@^PAPISUBDIR=\(.*\)@PAPISUBDIR=@g
s@^PDTDIR=\(.*\)@PDTDIR=@g
s@^PDTCOMPDIR=\(.*\)@PDTCOMPDIR=@g
s@^DYNINSTDIR=\(.*\)@DYNINSTDIR=@g
s@^JDKDIR=\(.*\)@JDKDIR=@g
s@^OPARIDIR=\(.*\)@OPARIDIR=@g
s@^TAU_OPARI_TOOL=\(.*\)@TAU_OPARI_TOOL=@g
s@^EPILOGDIR=\(.*\)@EPILOGDIR=@g
s@^TAUEXTRASHLIBOPTS=\(.*\)@TAUEXTRASHLIBOPTS=@g
s@^EPILOGBINDIR=\(.*\)@EPILOGBINDIR=@g
s@^EPILOGLIBDIR=\(.*\)@EPILOGLIBDIR=@g
s@^EPILOGEXTRALINKCMD=\(.*\)@EPILOGEXTRALINKCMD=@g
s@^EPILOGINCDIR=\(.*\)@EPILOGINCDIR=@g
s@^PERFINCDIR=\(.*\)@PERFINCDIR=@g
s@^PERFLIBDIR=\(.*\)@PERFLIBDIR=@g
s@^PERFLIBRARY=\(.*\)@PERFLIBRARY=@g
s@^VAMPIRTRACEDIR=\(.*\)@VAMPIRTRACEDIR=@g
s@^VAMPIRTRACEINCS=\(.*\)@VAMPIRTRACEINCS=@g
s@^VAMPIRTRACELIBS=\(.*\)@VAMPIRTRACELIBS=@g
s@^VAMPIRTRACEMPILIBS=\(.*\)@VAMPIRTRACEMPILIBS=@g
s@^HPCTOOLKIT_INCLUDE=\(.*\)@HPCTOOLKIT_INCLUDE=@g
s@^HPCTOOLKIT_LINK=\(.*\)@HPCTOOLKIT_LINK=@g
s@^SCOREPDIR=\(.*\)@SCOREPDIR=@g
s@^SCOREPINCS=\(.*\)@SCOREPINCS=@g
s@^SCOREPLIBS=\(.*\)@SCOREPLIBS=@g
s@^SCOREPMPILIBS=\(.*\)@SCOREPMPILIBS=@g
s@^EXTRADIR=\(.*\)@EXTRADIR=@g
s@^EXTRADIRCXX=\(.*\)@EXTRADIRCXX=@g
s@^VTFDIR=\(.*\)@VTFDIR=@g
s@^OTFDIR=\(.*\)@OTFDIR=@g
s@^SLOG2DIR=\(.*\)@SLOG2DIR=@g
s@^PYTHON_INCDIR=\(.*\)@PYTHON_INCDIR=@g
s@^PYTHON_LIBDIR=\(.*\)@PYTHON_LIBDIR=@g
s@^TAU_CONFIG=\(.*\)@TAU_CONFIG=@g
s@^TAU_MPI_INC=\(.*\)@TAU_MPI_INC=@g
s@^TAU_MPI_LIB=\(.*\)@TAU_MPI_LIB=@g
s@^TAU_MPI_FLIB=\(.*\)@TAU_MPI_FLIB=@g
s@^TAU_MPI_NOWRAP_LIB=\(.*\)@TAU_MPI_NOWRAP_LIB=@g
s@^TAU_MPI_NOWRAP_FLIB=\(.*\)@TAU_MPI_NOWRAP_FLIB=@g
s@^TAU_MPILIB_DIR=\(.*\)@TAU_MPILIB_DIR=@g
s@^TAU_PREFIX_INSTALL_DIR=\(.*\)@TAU_PREFIX_INSTALL_DIR=@g
s/#PVM_INSTALLED#\(.*\)/\1#PVM_INSTALLED#/g
s/^PVM_ARCH=\(.*\)/PVM_ARCH=default/g
s@^PVM_DIR=\(.*\)@PVM_DIR=default@g
s@^CONFIG_CC=\(.*\)@CONFIG_CC=gcc@g
s@^CONFIG_CXX=\(.*\)@CONFIG_CXX=g++@g
s@^PCXX_OPT=\(.*\)@PCXX_OPT=-g@g
s@^USER_OPT=\(.*\)@USER_OPT=-g@g
s@^PDT_CXX=\(.*\)@PDT_CXX=@g
s@^TAU_SHMEM_INC=\(.*\)@TAU_SHMEM_INC=@g
s@^TAU_SHMEM_LIB=\(.*\)@TAU_SHMEM_LIB=@g
s@^CHARMDIR=\(.*\)@CHARMDIR=@g
s@^FORCEIA32=\(.*\)@FORCEIA32=@g
s@^KTAU_INCDIR=\(.*\)@KTAU_INCDIR=@g
s@^KTAU_INCUSERDIR=\(.*\)@KTAU_INCUSERDIR=@g
s@^KTAU_LIB=\(.*\)@KTAU_LIB=@g
s@^FULL_CXX=\(.*\)@FULL_CXX=@g
s@^FULL_CC=\(.*\)@FULL_CC=@g
s@^UPCNETWORK=\(.*\)@UPCNETWORK=@g
s,^IFORTLIBDIR=.*$,IFORTLIBDIR=,g
s,^BFDINCLUDE=.*$,BFDINCLUDE=,g
s,^BFDLINK=.*$,BFDLINK=,g
s,^BFDLIBS=.*$,BFDLIBS=,g
s,^PERFSUITEINCLUDE=.*$,PERFSUITEINCLUDE=,g
s,^PERFSUITELINK=.*$,PERFSUITELINK=,g
s,^PDTARCHITECTURE=.*$,PDTARCHITECTURE=,g
s,^DWARFOPTS=.*$,DWARFOPTS=,g
s@^MRNET_ROOT=\(.*\)@MRNET_ROOT=@g
s@^MRNET_LW_OPTS=\(.*\)@MRNET_LW_OPTS=@g
s@^MRNET_INCLUDE=\(.*\)@MRNET_INCLUDE=@g
s@^MRNET_LIBS=\(.*\)@MRNET_LIBS=@g

