VPATH = ..
LIBDIR = ../../lib
TARGET = UTT_gauss_class

hpux_CC = gcc
alpha_osf1_CC = cc
sun_CC = gcc
sund_CC = gcc
linux_CC = gcc

linux_COPTS = -DLINUX -Dint32=int -Dfloat32=float -Dint16=short -Dintel
sun_COPTS = -DSUN -Dint32=int -Dfloat32=float -Dint16=short
sund_COPTS = -DSUN -Dint32=int -Dfloat32=float -Dint16=short
hpux_COPTS = -DHPUX -Dint32=int -Dfloat32=float -Dint16=short
alpha_osf1_COPTS = -DALPHA -Dint32=int -Dfloat32=float -Dint16=short

linux_CFLAGS = -O3
sun_CFLAGS = -O3
sund_CFLAGS = -g
hpux_CFLAGS = -O3
alpha_osf1_CFLAGS = -O2 -float


CFLAGS = ${$(arch)_COPTS} ${$(arch)_CFLAGS} -I$(VPATH) -I$(LIBDIR)/mfc_io
LIBS = -L$(LIBDIR)/mfc_io/$(arch) -lmfc_io -lm 
OBJS  = main.o io.o math.o

default: $(arch)
	(cd $(arch) ; $(MAKE) -f $(VPATH)/Makefile CC=${$(arch)_CC}  $(TARGET))

default_pure: $(arch)
	(cd $(arch) ; $(MAKE) -f $(VPATH)/Makefile CC=${$(arch)_CC}  $(TARGET)_pure)

$(OBJS): Makefile

$(TARGET): $(OBJS)
	$(CC) -o $@ $(CFLAGS) $(OBJS) $(LIBS)

$(TARGET)_pure: $(OBJS)
	purify $(CC) -o $@ $(CFLAGS) $(OBJS) $(LIBS)

$(arch):
	mkdir -p $(arch)

install:
	(cp $(arch)/$(TARGET) ../../bin/$(arch))

clean:
	(cd $(arch) ; rm -f *.o $(TARGET))

bare:
	(find . \( -name *.o -o -name $(TARGET) \) -ls -exec rm \{\} \; )







