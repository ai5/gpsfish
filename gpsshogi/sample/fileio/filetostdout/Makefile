TARGET=target
CMD=filetostdout
SRCS=filetostdout.go

$(CMD): $(SRCS)
	go build -o $@

.PHONY: all
all: clean
	for goos in darwin linux ; do \
	  for goarch in amd64 ; do \
	    mkdir -p $(TARGET)/$$goos-$$goarch; \
	    env GOOS=$$goos GOARCH=$$goarch go build -o $(TARGET)/$$goos-$$goarch/$(CMD); \
	  done; \
	done

.PHONY: install
install: all
	go install
	for goos in darwin linux ; do \
	  for goarch in amd64 ; do \
	    mkdir -p ~/bin/$$goos-$$goarch; \
	    cp $(TARGET)/$$goos-$$goarch/$(CMD) ~/bin/$$goos-$$goarch; \
	  done; \
	done
	scp $(TARGET)/linux-amd64/$(CMD) komaba:bin/

.PHONY: clean
clean:
	rm -rf $(TARGET)
	go clean
