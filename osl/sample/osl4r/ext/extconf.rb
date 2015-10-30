require 'mkmf'

CONFIG['CC'] = "g++"
CONFIG['LDSHARED'] = "g++ -shared"
$ARCH_FLAG  << " -march=native"
$INCFLAGS   << " -I../../../include"
$CPPFLAGS   << " -Wall" # -DOSL_SMP"
#$LIBPATH    << "../../../release-so/"
$LOCAL_LIBS << " ../../../release-so/libosl_search.a ../../../release-so/libosl_board.a" # -fPIC required
$LIBS       << " -lboost_filesystem-mt -lboost_thread-mt" # tcmalloc-minimal0

#if !have_library('osl')
#  puts "OSL library required -- not found."
#  exit 1
#end

create_makefile('osl4r')

File.open("Makefile", "a") << <<-EOT

check:	$(DLLIB)
	@$(RUBY) #{File.dirname(__FILE__)}/../test/tc_all.rb

check-all:	$(DLLIB)
	@OSL_TEST_LONG=t $(RUBY) #{File.dirname(__FILE__)}/../test/tc_all.rb
EOT
