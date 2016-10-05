VPATH = ..
LIBDIR = ../../lib
TARGET = wave2mfcc

hpux_CC = gcc
alpha_osf1_CC = cc
sun_CC = gcc
sund_CC = gcc
linux_CC = gcc

hpux_CFLAGS = -O -Dint32="int" -Dint16="short int" -Dfloat32="float" -Dnorint
alpha_osf1_CFLAGS = -O2 -float -DALPHA -Dint32="int" -Dint16="short int" -Dfloat32="float"
sun_CFLAGS = -O -Dint32="int" -Dint16="short int" -Dfloat32="float" -Dsun
linux_CFLAGS = -O -Dint32="int" -Dint16="short int" -Dfloat32="float" -Dintel

# NIST is the sphere directory, you will have to change these!

CFLAGS = ${$(arch)_CFLAGS} \
	-I$(LIBDIR)/mfc_io/ \
	-I$(LIBDIR)/fe/ \
        -I$(LIBDIR)/sphere/include

LIBS = 	-L$(LIBDIR)/mfc_io/$(arch) \
	-L$(LIBDIR)/fe/$(arch) \
	-L$(LIBDIR)/sphere/lib/$(arch) \
	-lmfc_io -lfe -lsp -lutil -lm


OBJS = 	main.o file.o signal.o fir.o fft.o complex.o f_memmove.o

default: $(arch)
	(cd $(arch) ; $(MAKE) -f $(VPATH)/Makefile CC=${$(arch)_CC}  $(TARGET))

default_pure: $(arch)
	(cd $(arch) ; $(MAKE) -f $(VPATH)/Makefile CC=${$(arch)_CC}  $(TARGET)_pure)

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(CFLAGS) $(LIBS)

$(TARGET)_pure: $(OBJS)
	purify $(CC) -o $@ $(CFLAGS) $(OBJS) $(LIBS)

$(arch):
	mkdir $(arch)

install:
	(cp $(arch)/$(TARGET) ../../bin/$(arch))

clean:
	(cd $(arch) ; rm -f *.o $(TARGET))

bare:
	(find . \( -name *.o -o -name $(TARGET) \) -ls -exec rm \{\} \; )

