CC=xlc_r
#  With TAU

CWD=`pwd`
include ../../../../include/include/Makefile 
include ../../../include/Makefile
TAU_MAKEFILE=$(TAU_LIB_DIR)/Makefile.tau$(TAU_CONFIG)
include ${TAU_MAKEFILE}
TAUOP='-optCompile=-I$(CWD)/hdfwrapper/hdf5_wrapper -optLinking="-L$(CWD)/hdfwrapper/hdf5_wrapper/ -lhdf5_wrap" -optVerbose '
#CC = $(TAU_COMPILER) $(TAUOP) $(TAU_CC)
CC = tau_cc.sh -tau_options=$(TAUOP) 

#HDF5DIR=/pkgs/hdf5-1.6.6-64bit
HDF5DIR=/usr/local/packages/hdf5-1.6.5
CFLAGS=-I$(HDF5DIR)/include
LIBS=-L$(HDF5DIR)/lib -lhdf5 -lm

all: checkhdf wrap run
checkhdf: wrap checkhdf.c 
	$(CC) $(CFLAGS) checkhdf.c -o checkhdf $(LIBS)
wrap:
	mkdir -p hdfwrapper; cp -r $(HDF5DIR)/include/* hdfwrapper
	cp select.tau hdfwrapper
	cd hdfwrapper; $(PDTDIR)/$(PDTARCHDIR)/bin/cparse hdf5.h; 
	cd hdfwrapper; tau_wrap hdf5.h.pdb hdf5.h -o hdf5.inst.c -f select.tau -g hdf5
	cd hdfwrapper/hdf5_wrapper; gmake  TAU_MAKEFILE=$(TAU_MAKEFILE) AR='$(TAU_AR)'

run: checkhdf wrap
	./checkhdf
	pprof

clean:
	/bin/rm -rf checkhdf.o checkhdf profile.* *.trc *.edf MULT* hdfwrapper SDS.h5
