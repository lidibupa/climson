CC=clang
CFLAGS=-I/usr/include -L/usr/lib -L/usr/lib64 -ltwilio -lcurl
BIN=bin
default: climson

climson: 
	@mkdir -p $(BIN)
	$(CC) $(CFLAGS) -o $(BIN)/climson climson.c

install: 
	@cp $(BIN)/climson /usr/bin/climson
devinstall: 
	ln -sf $(CURDIR)/$(BIN)/climson /usr/bin/climson

remove: 
	rm -f /usr/bin/climson

clean: 
	rm -rf $(BIN)
