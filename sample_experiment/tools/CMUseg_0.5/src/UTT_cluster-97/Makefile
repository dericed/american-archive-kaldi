VPATH = ..
LIBDIR = ../../lib
TARGET = UTT_cluster

hpux_CC = gcc
alpha_osf1_CC = cc
sun_CC = gcc
linux_CC = gcc

hpux_CFLAGS = -O -Dint32="int" -Dint16="short int" -Dfloat32="float" -Dnorint
alpha_osf1_CFLAGS = -O2 -float -DALPHA -Dint32="int" -Dint16="short int" -Dfloat32="float"
sun_CFLAGS = -O -Dint32="int" -Dint16="short int" -Dfloat32="float"
linux_CFLAGS = -O -Dint32="int" -Dint16="short int" -Dfloat32="float" -Dintel

CFLAGS = ${$(arch)_CFLAGS} \
	-I$(LIBDIR)/mfc_io \

LIBS =	-L$(LIBDIR)/mfc_io/$(arch) -lmfc_io -lm
OBJS  = main.o io.o math.o alloc2d.o

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


