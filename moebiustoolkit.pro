TEMPLATE = subdirs
SUBDIRS += src
!wasm:SUBDIRS += tests
OTHER_FILES = features/moebius_functions.prf
CONFIG += ordered # TODO: Use proper dependencies if the list of directories grows.
