VPATH = ..
TARGET = libfe.a

hpux_CC = gcc
alpha_osf1_CC = cc
sun_CC = gcc
sund_CC = gcc
linux_CC = gcc

sun_CFLAGS = -Bstatic -O4 -dalign  -DSUN=1  -Dint32=int -Dint16=short -Dfloat32=float
sund_CFLAGS = -Bstatic -g -dalign  -DSUN=1  -Dint32=int -Dint16=short -Dfloat32=float
hpux_CFLAGS = -DMEM_DEBUG=0 -DHPUX -O  -Dint32=int -Dint16=short -Dfloat32=float
alpha_osf1_CFLAGS = -O2 -DALPHA -Dint32=int -Dint16=short -Dfloat32=float
linux_CFLAGS = -O -Dint32=int -Dint16=short -Dfloat32=float -Dintel

CFLAGS = ${$(arch)_CFLAGS} -I$(VPATH)
OBJS  = alloc2d.o combine.o cospi.o \
	fe.o fft.o log2.o mel_filter.o melcep_frame.o \
	prehamfoldpad.o rotate.o scale_cep.o tables.o

default: $(arch)
	(cd $(arch) ; $(MAKE) -f $(VPATH)/Makefile CC=${$(arch)_CC}  $(TARGET))

$(TARGET): $(OBJS)
	ar crv $@ $?
	ranlib $@

$(arch):
	mkdir $(arch)
clean:
	(cd $(arch); rm -f *.o core  *.c.* *.a)

bare:
	(find . \( -name \*.o -o -name \*.a -o -name \*.c.\* \) -ls -exec rm \{\} \; )
