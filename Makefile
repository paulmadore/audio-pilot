PREFIX = /usr/local
INSTALL = /usr/bin/install -c
INSTALLDATA = /usr/bin/install -c -D -m 644
CC = gcc -Wall `pkg-config --cflags --libs gtk+-3.0`
all: transcoderaudioedition

transcoderaudioedition: src/main.o src/misc.o src/ui.o src/callbacks.o
	$(CC) -Xlinker -zmuldefs src/main.o src/misc.o src/ui.o src/callbacks.o -o transcoderaudioedition

main.o : src/main.c
	$(CC) -c -o src/main.o src/main.c
	
misc.o : src/misc.c
	$(CC) -c -o src/misc.o src/misc.c
	
ui.o : src/ui.c
	$(CC) -c -o src/ui.o src/ui.c
	
callbacks.o : src/callbacks.c
	$(CC) -c -o src/callbacks.o src/callbacks.c
	
clean:
	rm -rf src/*o transcoderaudioedition
	
install:all
	$(INSTALL) transcoderaudioedition $(PREFIX)/bin/transcoderaudioedition
	$(INSTALLDATA) data/transcoderaudioedition.desktop $(PREFIX)/share/applications/transcoderaudioedition.desktop
	$(INSTALLDATA) data/transcoderaudioedition.png $(PREFIX)/share/TranscoderAudioEdition/transcoderaudioedition.png

uninstall:
	rm -rf $(PREFIX)/bin/transcoderaudioedition
	rm -rf $(PREFIX)/share/applications/transcoderaudioedition.desktop
	rm -rf $(PREFIX)/share/TranscoderAudioEdition/transcoderaudioedition.png
