TEMPLATE = subdirs
SUBDIRS += src tests
OTHER_FILES = features/moebius_functions.prf
CONFIG += ordered # TODO: Use proper dependencies if the list of directories grows.
