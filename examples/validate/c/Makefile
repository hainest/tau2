include $(TAU_TEST_MAKEFILE)

CXX             = $(TAU_CXX)
CC              = $(TAU_CC)
CFLAGS          = $(TAU_INCLUDE) $(TAU_DEFS) $(TAU_SHMEM_INC)
LIBS            = $(TAU_LIBS) $(TAU_MPI_LIBS) $(TAU_SHMEM_LIB) $(TAU_LIBS) $(LEXTRA1) $(EXTRALIBS) 
LDFLAGS         = $(USER_OPT) $(TAU_OPENMP_OPTION)
RM              = /bin/rm -f
TARGET          = simple

##############################################

all:            $(TARGET)

install:        $(TARGET)

$(TARGET):      $(TARGET).o
	$(CXX) $(LDFLAGS) $(TARGET).o -o $@ $(LIBS)

$(TARGET).o : $(TARGET).c
	$(CC)  $(CFLAGS) -c $(TARGET).c

clean:
	$(RM) $(TARGET).o $(TARGET) profile.* *.trc *.edf *.elg
##############################################
