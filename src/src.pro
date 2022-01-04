TEMPLATE = subdirs
SUBDIRS += lib app
lib.file = libmoebius.pro
app.file = application.pro
app.depends = lib
