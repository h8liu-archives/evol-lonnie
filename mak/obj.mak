$o/$n/_s:
	-md $(subst /,\,$(@D))
	echo stamp > $(subst /,\,$@)

headers += $(wildcard $s/$n/*.h) $i/evol/base.h $i/evol/server.h
sources := $(wildcard $s/$n/*.c)

$o/$n/%.o: $s/$n/%.c $(headers) $o/$n/_s
	$(CC) $(CFLAGS) $(filter %.c,$^) -c -o $@

objects := $(addprefix $o/$n/,$(patsubst %.c,%.o,$(notdir $(sources))))
