
VPATH = ..
TARGET = libmfc_io.a

hpux_CFLAGS = -O -Dint32="int" -Dint16="short int" -Dfloat32="float" -Dnorint
alpha_osf1_CFLAGS = -O2 -float -DALPHA -Dint32="int" -Dint16="short int" -Dfloat32="float"
sun_CFLAGS = -O -Dint32="int" -Dint16="short int" -Dfloat32="float"
sund_CFLAGS = -g -Dint32="int" -Dint16="short int" -Dfloat32="float"
linux_CFLAGS = -O -Dint32="int" -Dint16="short int" -Dfloat32="float" -Dintel

sun_CC = gcc
sund_CC = gcc
hpux_CC = gcc
alpha_osf1_CC = cc
linux_CC = cc

CFLAGS = ${$(arch)_CFLAGS} -I$(VPATH)
OBJS  = mfc_io.o

default: $(arch)
	echo $($(arch)_CC)
	(cd $(arch) ; $(MAKE) -f $(VPATH)/Makefile CC=${$(arch)_CC}  $(TARGET))

$(arch):
	mkdir $(arch) 

$(TARGET): $(OBJS)
	echo $(CC)
	ar crv $@ $?
	ranlib $@

clean:
	(cd $(arch); rm -f *.o core  *.c.* *.a)

bare:
	(find . \( -name \*.o -o -name \*.a  -o -name \*.c.\* \) -ls -exec rm \{\} \; )

#.c.o:
#	echo $(CC)
#	$(CC) -c -o $@ $< $(CFLAGS)

