GPSSHOGI_HOME = ../../..
include $(GPSSHOGI_HOME)/makefile.conf
-include makefile.local

protoc_middleman: recordDb.proto
	$(OSL_HOME)/lib/third_party/$(PROTOBUF)/src/protoc --cpp_out=. $<
	@touch $@
recordDb.pb.h recordDb.pb.cc: protoc_middleman

clean:
	-rm *.pb.*
	-rm protoc_middleman
