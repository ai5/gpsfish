BASIC_BODULES = module/StopWatch.pm module/CsaBoard.pm
MODULES = $(BASIC_BODULES) module/UsiPosition.pm \
  module/UsiEngine.pm module/TreeNode.pm module/TreeSearch.pm
all: usi.pl csa2usi.pl
usi.pl: $(MODULES) bin/usi.pl
	-rm $@
	(echo "#!/usr/bin/perl -w"; echo "use threads;"; \
	 cat $(MODULES); echo "package main;"; cat bin/usi.pl) \
	  | grep -v 'use module::' > $@
	chmod a+x $@

csa2usi.pl: $(BASIC_BODULES) bin/csa2usi.pl
	-rm $@
	(echo "#!/usr/bin/perl -w"; \
	 cat $(BASIC_BODULES); echo "package main;"; cat bin/csa2usi.pl) \
	  | grep -v 'use module::' > $@
	chmod a+x $@
