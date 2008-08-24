CC = gcc
AR = ar
LPTHREAD = -lpthreadGC2
LSOCK32 = -lwsock32

# Source and used library directories
i = inc
s = src
l = lib

# Output directories
o = obj
r = release

CFLAGS = -I ./$i -L ./$l -L ./$r -O3 -Wall

.PHONY: all
all: $r/libevols.a $r/libevolp.a sample.exe simple.exe 

# Make output directories

$r/_s:
	-md $(subst /,\,$(@D))
	echo stamp > $(subst /,\,$@)

$o/_s:	
	-md $(subst /,\,$(@D))
	echo stamp > $(subst /,\,$@)


# Server (library) : libevols.a

n = server
headers = $i/evol/base.h $i/evol/server.h
include mak/obj.mak
$r/libevols.a: $(objects) $r/_s
	$(AR) cqs $@ $^


# Player (library) : libevolp.a

n = player
headers = $i/evol/base.h $i/evol/player.h
include mak/obj.mak
$r/libevolp.a: $(objects) $r/_s
	$(AR) cqs $@ $^


# Sample game program

n = sample
headers = $i/evol/base.h $i/evol/server.h $i/evol/sample/base.h
include mak/obj.mak
sample.exe: $(objects) $r/libevols.a
	$(CC) $(CFLAGS) $(filter %.o,$^) -o $@ -levols -lgdi32 $(LPTHREAD) $(LSOCK32)


# Simple player program

n := simple
headers = $i/evol/base.h $i/evol/player.h $i/evol/sample/base.h
include mak/obj.mak
simple.exe: $(objects) $r/libevolp.a
	$(CC) -mno-cygwin $(CFLAGS) $(filter %.o,$^) -o $@ -lmingw32 -levolp

# Clean

.PHONY: clean srcclean distclean logclean

clean:
	-rm $o $r -r
	-rm *.exe

srcclean:
	-del *.*~ /S
	-del *~ /S
	-rm tags

logclean:
	-rm *.log

distclean: srcclean clean logclean

# Run

.PHONY: run

run: sample.exe
	sample.exe
