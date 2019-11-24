ASCIIDOCTOR = asciidoctor

PAGES = $(patsubst %.adoc,../public/%.html,$(wildcard *.adoc))

all: $(PAGES)

list-pages:
	@echo $(PAGES)

clean:
	rm -f $(PAGES)

../public/%.html: %.adoc
	$(ASCIIDOCTOR) $< -o $@

.PHONY: all clean
