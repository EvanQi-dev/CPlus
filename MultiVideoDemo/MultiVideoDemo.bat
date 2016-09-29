@echo off

rem del /Q /F debug\*.* release\*.* Makefile.*
rem del /Q /F *.plg *.idb *.pdb *.pro* *.sln *.suo *.ncb *.dsp *.vcproj.* *.vcxproj.*
del /Q /F moc\*
del /Q /F rcc\*
del /Q /F ui\*

rem 获取当前路径名
for %%i in ("%cd%") do set n=%%~nxi

rem 设置工程名
set prjName=%n%.pro

qmake -project -o %prjName%

echo DESTDIR = out	>> %prjName%
echo MOC_DIR = moc	>> %prjName%
echo RCC_DIR = rcc	>> %prjName%
echo UI_DIR  = ui	>> %prjName%

echo QT += gui network widgets multimedia >> %prjName%

echo CONFIG += qt warn_on	>> %prjName%
echo CONFIG -= flat		    >> %prjName%

echo QMAKE_CXXFLAGS_RELEASE                -= -Zc:strictStrings >> %prjName%
echo QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO -= -Zc:strictStrings >> %prjName%
echo QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:level=\'requireAdministrator\' >> %prjName%
echo QMAKE_LFLAGS_WINDOWS += /LARGEADDRESSAWARE /SAFESEH:NO >> %prjName%

echo INCLUDEPATH  += ./ffmpeg/include ./include >> %prjName%
echo QMAKE_LIBDIR += ./ffmpeg/lib ./lib >> %prjName%

echo LIBS += ws2_32.lib avcodec.lib avdevice.lib avfilter.lib avformat.lib avutil.lib postproc.lib swresample.lib swscale.lib >> %prjName%

echo CONFIG(debug, debug^|release) { >> %prjName%
echo     LIBS += log4cplusD.lib >> %prjName%
echo } else { >> %prjName%
echo     LIBS += log4cplus.lib >> %prjName%
echo } >> %prjName%

rem echo RC_FILE += ./EcloudDesktop/HSDesktop.rc >> %prjName%

qmake
qmake -r -spec win32-msvc2013 -tp vc %prjName%
pause
