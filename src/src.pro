TEMPLATE = subdirs
SUBDIRS += libmoebius libtoolkit app
# Helper library only reliant on Qt Core. Might be useful outside Moebius Toolkit.
libmoebius.file = libmoebius.pro
# Library to split application specific functionality for testing. Might use GUI
# libraries, and it's unlikely to be useful outside the MT application.
libtoolkit.file = libtoolkit.pro
# The Moebius Toolkit app, which relies on both libraries.
app.file = application.pro
app.depends = libmoebius libtoolkit
