

default: check ../bin/$(arch)
	(cd lib;   $(MAKE) $@)
	(cd wave2mfcc;   $(MAKE) $@)
	(cd UTT_Kseg;    $(MAKE) $@)
	(cd UTT_cluster; $(MAKE) $@)
	(cd UTT_findsil; $(MAKE) $@)
	(cd UTT_gauss_class; $(MAKE) $@)

../bin/$(arch):
	mkdir ../bin/$(arch)

check:
	@-if test "$(arch)" = "" ; \
	then echo "Error: the environment variable 'arch' must be set"; exit 1; \
	fi
	@-if test "$(MAKE)" = "" ; \
	then echo "Error: the environment variable 'MAKE' must be set"; exit 1; \
	fi

sphere_lib:
	(cd lib; $(MAKE) $@)

install:
	(cd wave2mfcc;   $(MAKE) $@)
	(cd UTT_Kseg;    $(MAKE) $@)
	(cd UTT_cluster; $(MAKE) $@)
	(cd UTT_findsil; $(MAKE) $@)
	(cd UTT_gauss_class; $(MAKE) $@)

sphere_clean:
	(cd lib/sphere; $(MAKE) clean)

clean:
	(cd lib;   $(MAKE) $@)
	(cd wave2mfcc;   $(MAKE) $@)
	(cd UTT_Kseg;    $(MAKE) $@)
	(cd UTT_cluster; $(MAKE) $@)
	(cd UTT_findsil; $(MAKE) $@)
	(cd UTT_gauss_class; $(MAKE) $@)

sphere_bare:
	(cd lib/sphere; $(MAKE) bare)

bare:
	(cd lib;   $(MAKE) $@)
	(cd wave2mfcc;   $(MAKE) $@)
	(cd UTT_Kseg;    $(MAKE) $@)
	(cd UTT_cluster; $(MAKE) $@)
	(cd UTT_findsil; $(MAKE) $@)
	(cd UTT_gauss_class; $(MAKE) $@)
