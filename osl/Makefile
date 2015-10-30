-include makefile.local
include makefile.conf
all: 
	$(MAKE) -C core/osl
	$(MAKE) -C std/osl
	$(MAKE) -C full/osl
debug:
	$(MAKE) DEBUG=t all make-test
clean:
	$(MAKE) -C core/osl clean
	$(MAKE) -C std/osl clean
	$(MAKE) -C full/osl clean
	$(MAKE) -C core/test clean
	$(MAKE) -C std/test clean
	$(MAKE) -C full/test clean
make-test:
	$(MAKE) -C core/test
	$(MAKE) -C std/test 
	$(MAKE) -C full/test
run-test: make-test
	(cd core/test; ./testAll) && (cd std/test; ./testAll) && (cd full/test; ./testAll)



# ETAGS ?= etags
# 
# all : third-party
# 	$(MAKE) -C lib
# 	$(MAKE) -C test
# 	$(MAKE) -C sample
# third-party: lib/third_party/$(TCMALLOC)/Makefile lib/third_party/$(GTEST)/Makefile lib/third_party/$(PROTOBUF)/Makefile
# ifeq ($(USE_TCMALLOC),1)
# 	$(MAKE) -C lib/third_party/$(TCMALLOC)
# endif
# 	$(MAKE) -C lib/third_party/$(GTEST)
# 	$(MAKE) -C lib/third_party/$(PROTOBUF)
# 
# prof : 
# 	$(MAKE) PROFILE=true -C lib
# 
# clean:
# 	$(MAKE) -C lib clean
# 	$(MAKE) -C release clean
# 	$(MAKE) -C profile clean
# 	$(MAKE) -C test clean
# 	$(MAKE) -C sample clean
# 	$(MAKE) -C doc clean
# cleandeps:
# 	rm -rf lib/.deps lib/.objs lib/lib*.a
# 	rm -rf release/.deps release/.objs release/lib*.a
# 	rm -rf profile/.deps profile/.objs profile/lib*.a
# 	rm -rf sample/.deps sample/.objs
# 	rm -rf test/.deps test/.objs
# 
# tags:
# 	-rm TAGS
# 	for d in include/osl lib; do \
#           find $$d \( -name "*.h" -o -name "*.cc" -o -name "*.tcc" \) -exec $(ETAGS) --append --language=c++ {} \;; \
# 	done
# html:
# 	cd doc; $(MAKE)
# 
# lib/third_party/$(TCMALLOC)/Makefile: lib/third_party/$(TCMALLOC)/configure
# 	cd lib/third_party/$(TCMALLOC); env CXX=$(CXX) ./configure --disable-cpu-profiler --disable-heap-profiler --disable-heap-checker --disable-debugalloc --enable-minimal --enable-shared=no
# lib/third_party/$(GTEST)/Makefile: lib/third_party/$(GTEST)/configure
# 	cd lib/third_party/$(GTEST); env CXX=$(CXX) ./configure 
# lib/third_party/$(PROTOBUF)/Makefile: lib/third_party/$(PROTOBUF)/configure
# 	cd lib/third_party/$(PROTOBUF); env CXX=$(CXX) ./configure 
# 
# add-utf-8-bom:
# 	find . \( -name "*.h" -o -name "*.tcc" -o -name "*.cc" \) -exec nkf -w8 --overwrite {} \;
# 
# remove-utf-8-bom:
# 	find . \( -name "*.h" -o -name "*.tcc" -o -name "*.cc" \) -exec nkf -w --overwrite {} \;
