all: tea

# zclib
LDFLAGS := -lzc

tea_src := ciphers/tea.c

tea:
	gcc $(LDFLAGS) tea_test.c $(tea_src) -o tea

clean:
	rm -rf tea 
