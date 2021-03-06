DEFINES += GITHUBGIST_LIBRARY

# GitHubGist files

SOURCES += \
    src/gistplugin.cpp \
    src/gistmanager.cpp \
    src/optionspage.cpp \
    src/settings.cpp

HEADERS += \
    src/gistplugin.h \
    src/gistplugin_global.h \
    src/gistpluginconstants.h \
    src/gistmanager.h \
    src/optionspage.h \
    src/settings.h

# Qt Creator linking

## set the QTC_SOURCE environment variable to override the setting here
QTCREATOR_SOURCES = $$(QTC_SOURCE)
isEmpty(QTCREATOR_SOURCES):QTCREATOR_SOURCES=

## set the QTC_BUILD environment variable to override the setting here
IDE_BUILD_TREE = $$(QTC_BUILD)
isEmpty(IDE_BUILD_TREE):IDE_BUILD_TREE=

## uncomment to build plugin into user config directory
## <localappdata>/plugins/<ideversion>
##    where <localappdata> is e.g.
##    "%LOCALAPPDATA%\QtProject\qtcreator" on Windows Vista and later
##    "$XDG_DATA_HOME/data/QtProject/qtcreator" or "~/.local/share/data/QtProject/qtcreator" on Linux
##    "~/Library/Application Support/QtProject/Qt Creator" on Mac
# USE_USER_DESTDIR = yes

include($$QTCREATOR_SOURCES/src/qtcreatorplugin.pri)

FORMS += \
    src/optionspage.ui

RESOURCES += \
    githubgist.qrc
