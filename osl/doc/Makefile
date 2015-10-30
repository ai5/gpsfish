DOXYGEN ?= doxygen
all:
	doxygen
	find . -name '*.html' -exec perl -pi -e 's/td nowrap/td/g' {} \;

clean:
	rm -f  doxygen.err
	rm -rf html
	rm -rf latex

.PHONY: all clean
