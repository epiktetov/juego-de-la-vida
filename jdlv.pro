#!qmake -- Qt project file for jdlv
TARGET   = jdlv
HEADERS += jdlv.h   elmundo.h                 elvista.h
SOURCES += jdlv.cpp elmundo.cpp elmundo-a.cpp elvista.cpp
macx {
  ICON = jdlv.icns
  QMAKE_INFO_PLIST = jdlv.Info.plist
  QMAKE_PKGINFO_TYPEINFO = "^epi"
}
CONFIG += debug
#^keep until version 1.0
OBJECTS_DIR = obj
RESOURCES = jdlv.qrc
# - - - - - - - - - - - - - - - -
greaterThan(QT_MAJOR_VERSION,4) {
  QT += widgets
}
unix: QtPLATF = unix
macx: QtPLATF = macx
win32:QtPLATF = win32
QMAKE_CFLAGS   += -std=c99
QMAKE_CXXFLAGS += -DQtPLATF=\'\"$$QtPLATF\"\'

version.target = version.h
version.depends = .git $$HEADERS $$SOURCES
version.commands = @echo \"const char jdlvVERSION[]=\\\"\"\"`git describe --tags --dirty`\"\"\\\";\" >version.h;git status
QMAKE_EXTRA_TARGETS += version
PRE_TARGETDEPS += version.h
