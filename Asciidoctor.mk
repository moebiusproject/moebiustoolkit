public/index.html: README.adoc
	asciidoctor -o public/index.html $<
