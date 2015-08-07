:: ============================================================================
::
:: runtests.cmd
::
:: Runs checkin tests using the ch.exe on the path, in 2 variants:
::
:: -maxInterpretCount:1 -maxSimpleJitRunCount:1 -bgjit-
:: <dynapogo>
::
:: Logs are placed into:
::
:: logs\interpreted
:: logs\dynapogo
::
:: User specified variants:
:: logs\forcedeferparse
:: logs\nodeferparse
:: logs\forceundodefer
:: logs\bytecodeserialized (serialized to byte codes)
:: logs\forceserialized (force bytecode serialization internally)
::
:: ============================================================================
@echo off
setlocal

goto :main

:: ============================================================================
:: Print usage
:: ============================================================================
:printUsage

  echo Usage: runtests.cmd [-binary ^<path^>]

  rem TODO: Improve usage help

  goto :eof

:: ============================================================================
:: Main script
:: ============================================================================
:main

  set _Binary=
  set _Variants=
  set _TAGS=
  set _NOTTAGS=
  set _DIRNOTTAGS=
  set _DIRTAGS=
  set _drt=
  set _rebase=
  set _ExtraVariants=
  set _dynamicprofilecache=-dynamicprofilecache:profile.dpl
  set _dynamicprofileinput=-dynamicprofileinput:profile.dpl
  set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -DumpOnCrash

  call :parseArgs %*

  if not "%fShowUsage%" == "" (
    call :printUsage
    goto :eof
  )

  if not "%_binaryRoot%" == "" set path=%_binaryRoot%;%path%

  if "%_Binary%" == "" set _Binary=-binary:ch.exe

  :: If the user didn't specify explicit variants then do the defaults
  if "%_Variants%"=="" set _Variants=interpreted,dynapogo

  :: If the user specified extra variants to run (i.e. in addition to the defaults), include them.
  if not "%_ExtraVariants%" == "" set _Variants=%_Variants%,%_ExtraVariants%

  rem TODO: Move any apollo tests from core\test back into private unittests
  set _ExcludeApolloTests=
  if "%APOLLO%" == "1" (
      set _ExcludeApolloTests=-nottags:exclude_apollo
      set TARGET_OS=wp8
  )

  rem TODO: Move any nightly tagged tests from core\test back into private unittests
  rem if "%_nightly%" == "" (
      set _NOTTAGS=%_NOTTAGS% -nottags:nightly
  )

  rem TODO: Move any slow tagged tests from core\test back into private unittests
  if not "%_full%" == "1" (
      set _NOTTAGS=%_NOTTAGS% -nottags:Slow
  )

  if not "%NUM_RL_THREADS%" == "" (
    set _RL_THREAD_FLAGS=-threads:%NUM_RL_THREADS%
  )

  if "%_DIRS%" == "" (
    set _DIRS=-all
  )

  :: Ensure we're running from the root folder of the tests
  pushd %~dp0

  set _logsRoot=%cd%\logs
  call :doSilent del /s /q profile.dpl.*

  for %%i in (%_Variants%) do (
      set _TESTCONFIG=%%i
      call :RunOneVariant
  )

  for %%i in (%_Variants%) do (
      echo ######## Logs for %%i variant ########
      type %_logsRoot%\%%i\rl.log
      echo.
  )

  popd

  goto :eof

:: ============================================================================
:: Parse the user arguments into environment variables
:: ============================================================================
:parseArgs

  :NextArgument

  if "%1" == "-?" set fShowUsage=1& goto :ArgOk
  if "%1" == "/?" set fShowUsage=1& goto :ArgOk
  if /i "%1" == "-binary"     set _Binary=-binary:%2&                                            goto :ArgOkShift2
  if /i "%1" == "-dirs"       set _DIRS=-dirs:%2&                                                goto :ArgOkShift2
  if /i "%1" == "-win7"       set TARGET_OS=win7&                                                goto :ArgOk
  if /i "%1" == "-win8"       set TARGET_OS=win8&                                                goto :ArgOk
  if /i "%1" == "-winBlue"    set TARGET_OS=winBlue&                                             goto :ArgOk
  if /i "%1" == "-win10"      set TARGET_OS=win10&                                               goto :ArgOk
  if /i "%1" == "-nottags"    set _NOTTAGS=%_NOTTAGS% -nottags:%2&                               goto :ArgOkShift2
  if /i "%1" == "-tags"       set _TAGS=%_TAGS% -tags:%2&                                        goto :ArgOkShift2
  if /i "%1" == "-dirtags"    set _DIRTAGS=%_DIRTAGS% -dirtags:%2&                               goto :ArgOkShift2
  if /i "%1" == "-dirnottags" set _DIRNOTTAGS=%_DIRNOTTAGS% -dirnottags:%2&                      goto :ArgOkShift2
  :: TODO Consider removing -drt and exclude_drt in some reasonable manner
  if /i "%1" == "-drt"        set _drt=1& set _NOTTAGS=%_NOTTAGS% -nottags:exclude_drt&          goto :ArgOk
  if /i "%1" == "-rebase"     set _rebase=-rebase&                                               goto :ArgOk
  if /i "%1" == "-rundebug"   set _RUNDEBUG=1&                                                   goto :ArgOk
  :: TODO Figure out best way to specify build arch for tests that are excluded to specific archs
  if /i "%1" == "-platform"   set _buildArch=%2&                                                 goto :ArgOkShift2
  :: TODO Figure out best way to specify build type for tests that are excluded to specific type (chk, fre, etc)
  if /i "%1" == "-buildType"  set _buildType=%2&                                                 goto :ArgOkShift2
  if /i "%1" == "-binaryRoot" set _binaryRoot=%~f2&                                              goto :ArgOkShift2
  if /i "%1" == "-toolsRoot"  set _toolsRoot=%2&                                                 goto :ArgOkShift2
  if /i "%1" == "-variants"   set _Variants=%~2&                                                 goto :ArgOkShift2

  if /i "%1" == "-extraVariants" (
    :: Extra variants are specified by the user but not run by default.
    if "%_ExtraVariants%" == "" (
      set _ExtraVariants=%~2
    ) else (
      set _ExtraVariants=%_ExtraVariants%,%~2
    )
    goto :ArgOkShift2
  )

  :: Defined here are shorthand versions for specifying
  :: extra variants when running.
  if /i "%1" == "-parser" (
    if "%_ExtraVariants%" == "" (
      set _ExtraVariants=forcedeferparse,nodeferparse,forceundodefer
    ) else (
      set _ExtraVariants=%_ExtraVariants%,forcedeferparse,nodeferparse,forceundodefer
    )
    goto :ArgOk
  )
  if /i "%1" == "-serialization" (
    if "%_ExtraVariants%" == "" (
      set _ExtraVariants=bytecodeserialized,forceserialized
    ) else (
      set _ExtraVariants=%_ExtraVariants%,bytecodeserialized,forceserialized
    )
    goto :ArgOk
  )

  if not "%1" == "" echo Unknown argument: %1 & set fShowUsage=1

  goto :eof

  :ArgOkShift2
  shift

  :ArgOk
  shift

  goto :NextArgument

:: ============================================================================
:: Run one variant
:: ============================================================================
:RunOneVariant

  set _OLD_CC_FLAGS=%EXTRA_CC_FLAGS%
  set EXTRA_RL_FLAGS=-appendtestnametoextraccflags
  set _exclude_serialized=

  if "%_TESTCONFIG%"=="interpreted" (
      set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -maxInterpretCount:1 -maxSimpleJitRunCount:1 -bgjit- %_dynamicprofilecache%
  )
  if "%_TESTCONFIG%"=="nonative" (
      set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -nonative
      set EXTRA_RL_FLAGS=-nottags:exclude_interpreted -nottags:fails_interpreted
  )
  if "%_TESTCONFIG%"=="dynapogo"    (
      set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -forceNative -off:simpleJit -bgJitDelay:0 %_dynamicprofileinput%
  )

  :: Variants after here are user supplied variants (not run by default).
  if "%_TESTCONFIG%"=="forcedeferparse" (
      set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -forceDeferParse %_dynamicprofilecache%
      set _exclude_forcedeferparse=-nottags:exclude_forcedeferparse
  )
  if "%_TESTCONFIG%"=="nodeferparse" (
      set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -noDeferParse %_dynamicprofilecache%
      set _exclude_nodeferparse=-nottags:exclude_nodeferparse
  )
  if "%_TESTCONFIG%"=="forceundodefer" (
      set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -forceUndoDefer %_dynamicprofilecache%
      set _exclude_forceundodefer=-nottags:exclude_forceundodefer
  )
  if "%_TESTCONFIG%"=="bytecodeserialized" (
      set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -recreatebytecodefile -serialized:%TEMP%\ByteCode
      set _exclude_serialized=-nottags:exclude_serialized
  )
  if "%_TESTCONFIG%"=="forceserialized" (
      set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -forceserialized
      set EXTRA_RL_FLAGS=
      set _exclude_serialized=-nottags:exclude_serialized
  )

  echo ############# Starting %_TESTCONFIG% variant #############

  call :do del /q %_logsRoot%\rl*
  call :do md %_logsRoot%\%_TESTCONFIG%

  echo %_TESTCONFIG% > %_logsRoot%\_currentRun.tmp

  set _rlArgs=%_Binary%
  set _rlArgs=%_rlArgs% -nottags:fail
  set _rlArgs=%_rlArgs% %_RL_THREAD_FLAGS%
  set _rlArgs=%_rlArgs% %_DIRS%
  set _rlArgs=%_rlArgs% -verbose
  set _rlArgs=%_rlArgs% %_TAGS%
  set _rlArgs=%_rlArgs% %_NOTTAGS%
  set _rlArgs=%_rlArgs% %_DIRTAGS%
  set _rlArgs=%_rlArgs% %_DIRNOTTAGS%
  set _rlArgs=%_rlArgs% -nottags:fails_%_TESTCONFIG%
  set _rlArgs=%_rlArgs% -nottags:fail_%TARGET_OS%
  set _rlArgs=%_rlArgs% -nottags:exclude_%_TESTCONFIG%
  set _rlArgs=%_rlArgs% -nottags:exclude_%_buildArch%
  set _rlArgs=%_rlArgs% -nottags:exclude_%TARGET_OS%
  set _rlArgs=%_rlArgs% -nottags:exclude_%_buildType%
  set _rlArgs=%_rlArgs% %_exclude_serialized%
  set _rlArgs=%_rlArgs% %_exclude_forcedeferparse%
  set _rlArgs=%_rlArgs% %_exclude_nodeferparse%
  set _rlArgs=%_rlArgs% %_exclude_forceundodefer%
  set _rlArgs=%_rlArgs% %_ExcludeApolloTests%
  set _rlArgs=%_rlArgs% -exe
  set _rlArgs=%_rlArgs% %EXTRA_RL_FLAGS%
  set _rlArgs=%_rlArgs% %_rebase%

  set REGRESS=%CD%

  call :do rl %_rlArgs%

  call :do move /Y %_logsRoot%\*.log %_logsRoot%\%_TESTCONFIG%
  del /Q %_logsRoot%\_currentRun.tmp

  set EXTRA_CC_FLAGS=%_OLD_CC_FLAGS%

  goto :eof

:: ============================================================================
:: Echo a command line before executing it
:: ============================================================================
:do

  echo ^>^> %*
  cmd /s /c "%*"

  goto :eof

:: ============================================================================
:: Echo a command line before executing it and redirect the command's output
:: to nul
:: ============================================================================
:doSilent

  echo ^>^> %* ^> nul 2^>&1
  cmd /s /c "%*" > nul 2>&1

  goto :eof
