cmake_minimum_required(VERSION 3.7)
#cmake_policy(SET CMP0048 NEW) # VERSION in project (3.0)
string(TIMESTAMP MAJOR "%Y")
string(TIMESTAMP MINOR "%m")
string(TIMESTAMP PATCH "%d")

project(EmptyEpsilon VERSION ${MAJOR}.${MINOR}.${PATCH})
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


set(COMPRESS_TEXTURES "" CACHE STRING "Texture compression (list: dxt, astc)")
option(ENABLE_CRASH_LOGGER "Enable the drmingw crash logging facilities" OFF)
message(STATUS "ENABLE_CRASH_LOGGER is " ${ENABLE_CRASH_LOGGER})

if(ENABLE_CRASH_LOGGER)
 if(MINGW)
  if(NOT DEFINED DRMINGW_ROOT)
   message(FATAL_ERROR "DRMINGW_ROOT was not set. Unable to continue")
  endif(NOT DEFINED DRMINGW_ROOT)
 endif(MINGW)
endif(ENABLE_CRASH_LOGGER)

message(STATUS "EmptyEpsilon Version = ${PROJECT_VERSION}")

# Globals
# install resources, scripts, and packs to app bundle instead of system dir.
if(APPLE)
	set(CMAKE_INSTALL_PREFIX "./")
	# Set minimum target macOS version
	set(MACOSX_DEPLOYMENT_TARGET "10.10")
endif()
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

add_compile_definitions(
	VERSION_NUMBER=${PROJECT_VERSION_MAJOR}${PROJECT_VERSION_MINOR}${PROJECT_VERSION_PATCH}
	WINDOW_TITLE="${PROJECT_NAME}"
	SFML_NO_DEPRECATED_WARNINGS # SFML 2.5 gives deprication warnings on a few functions we use. But we maintain 2.3 compatibility, so ignore those warnings.

	$<$<AND:$<BOOL:${ENABLE_CRASH_LOGGER}>,$<BOOL:${MINGW}>>:ENABLE_CRASH_LOGGER>
	# Platform-specific
	$<$<BOOL:${WIN32}>:NOMINMAX>
	$<$<BOOL:${UNIX}>:RESOURCE_BASE_DIR="${CMAKE_INSTALL_PREFIX}/share/emptyepsilon/">
	$<$<BOOL:${CONFIG_DIR}>:CONFIG_DIR="${CONFIG_DIR}">
	$<$<AND:$<NOT:$<BOOL:${CONFIG_DIR}>>,$<BOOL:${UNIX}>>:CONFIG_DIR="${CMAKE_INSTALL_PREFIX}/share/emptyepsilon/">
)

# Dependencies

# SeriousProton
set(SERIOUSPROTON_WITH_JSON ON CACHE BOOL "" FORCE)
add_subdirectory(SeriousProton EXCLUDE_FROM_ALL)
add_subdirectory(meshoptimizer EXCLUDE_FROM_ALL)

# Discord
# Some magical Win 32 stuff
if(WIN32)
  set(DISCORD_ARCH "x86_64")
  if(${CMAKE_SIZEOF_VOID_P} EQUAL 4)
    set(DISCORD_ARCH "x86")
  endif()
  set(DISCORD_LIB_PATH "${PROJECT_BINARY_DIR}/discord/lib/${DISCORD_ARCH}")
  if(NOT EXISTS "${DISCORD_LIB_PATH}/discord_game_sdk.dll.lib")
    file(DOWNLOAD "https://dl-game-sdk.discordapp.net/latest/discord_game_sdk.zip" "${PROJECT_BINARY_DIR}/discord/game_sdk.zip" TIMEOUT 60 TLS_VERIFY ON)
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar -xf "${PROJECT_BINARY_DIR}/discord/game_sdk.zip" WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/discord")
    if(CMAKE_HOST_SYSTEM_NAME STREQUAL "Linux")
      # Discord uses Windows.h (capital W) and that does not play nice when cross-compiling.
      # we want to patch with a lowercase windows.h
      message(STATUS "Patching discord_game._sdk.h, using a lowercase Windows.h (cross-compilation compatibility)")
      execute_process(COMMAND sed -i "s/<Windows.h>/<windows.h>/g" c/discord_game_sdk.h WORKING_DIRECTORY "${PROJECT_BINARY_DIR}/discord")
    endif()
  endif()
  add_library(discord SHARED IMPORTED)
  set_target_properties(discord PROPERTIES
  	IMPORTED_LOCATION "${DISCORD_LIB_PATH}/discord_game_sdk.dll"
  	IMPORTED_IMPLIB "${DISCORD_LIB_PATH}/discord_game_sdk.dll.lib"
	INTERFACE_INCLUDE_DIRECTORIES "${PROJECT_BINARY_DIR}/discord/c/"
  )
endif()

set(SOURCES
	src/main.cpp
	src/threatLevelEstimate.cpp
	src/preferenceManager.cpp
	src/pathPlanner.cpp
	src/epsilonServer.cpp
	src/particleEffect.cpp
	src/mouseCalibrator.cpp
	src/httpScriptAccess.cpp
	src/modelInfo.cpp
	src/packResourceProvider.cpp
	src/scienceDatabase.cpp
	src/commsScriptInterface.cpp
	src/modelData.cpp
	src/gameGlobalInfo.cpp
	src/GMActions.cpp
	src/script.cpp
	src/playerInfo.cpp
	src/gameStateLogger.cpp
	src/shipTemplate.cpp
	src/beamTemplate.cpp
	src/missileWeaponData.cpp
	src/factionInfo.cpp
	src/mesh.cpp
	src/scenarioInfo.cpp
	src/shaderRegistry.cpp
	src/repairCrew.cpp
	src/GMScriptCallback.cpp
	src/GMMessage.cpp
	src/tutorialGame.cpp
	src/scriptDataStorage.cpp
	src/glObjects.cpp
	src/menus/joinServerMenu.cpp
	src/menus/serverBrowseMenu.cpp
	src/menus/mainMenus.cpp
	src/menus/serverCreationScreen.cpp
	src/menus/tutorialMenu.cpp
	src/menus/optionsMenu.cpp
	src/menus/shipSelectionScreen.cpp
	src/menus/autoConnectScreen.cpp
	src/menus/hotkeyMenu.cpp
	src/screens/cinematicViewScreen.cpp
	src/screens/crewStationScreen.cpp
	src/screens/topDownScreen.cpp
	src/screens/windowScreen.cpp
	src/screens/mainScreen.cpp
	src/screens/spectatorScreen.cpp
	src/screens/crew4/operationsScreen.cpp
	src/screens/crew4/engineeringAdvancedScreen.cpp
	src/screens/crew4/tacticalScreen.cpp
	src/screens/crew6/engineeringScreen.cpp
	src/screens/crew6/scienceScreen.cpp
	src/screens/crew6/relayScreen.cpp
	src/screens/crew6/weaponsScreen.cpp
	src/screens/crew6/helmsScreen.cpp
	src/screens/crew1/singlePilotScreen.cpp
	src/screens/extra/damcon.cpp
	src/screens/extra/powerManagement.cpp
	src/screens/extra/databaseScreen.cpp
	src/screens/extra/commsScreen.cpp
	src/screens/extra/shipLogScreen.cpp
	src/screens/gm/gameMasterScreen.cpp
	src/screens/gm/objectCreationView.cpp
	src/screens/gm/globalMessageEntryView.cpp
	src/screens/gm/chatDialog.cpp
	src/screens/gm/tweak.cpp
	src/screenComponents/aimLock.cpp
	src/screenComponents/alertOverlay.cpp
	src/screenComponents/helpOverlay.cpp
	src/screenComponents/missileTubeControls.cpp
	src/screenComponents/selfDestructIndicator.cpp
	src/screenComponents/viewport3d.cpp
	src/screenComponents/viewportMainScreen.cpp
	src/screenComponents/selfDestructEntry.cpp
	src/screenComponents/dockingButton.cpp
	src/screenComponents/shieldsEnableButton.cpp
	src/screenComponents/selfDestructButton.cpp
	src/screenComponents/shieldFreqencySelect.cpp
	src/screenComponents/jumpControls.cpp
	src/screenComponents/impulseControls.cpp
	src/screenComponents/frequencyCurve.cpp
	src/screenComponents/noiseOverlay.cpp
	src/screenComponents/powerDamageIndicator.cpp
	src/screenComponents/beamTargetSelector.cpp
	src/screenComponents/shipInternalView.cpp
	src/screenComponents/beamFrequencySelector.cpp
	src/screenComponents/radarView.cpp
	src/screenComponents/rawScannerDataRadarOverlay.cpp
	src/screenComponents/scanTargetButton.cpp
	src/screenComponents/snapSlider.cpp
	src/screenComponents/indicatorOverlays.cpp
	src/screenComponents/openCommsButton.cpp
	src/screenComponents/combatManeuver.cpp
	src/screenComponents/rotatingModelView.cpp
	src/screenComponents/shipDestroyedPopup.cpp
	src/screenComponents/warpControls.cpp
	src/screenComponents/targetsContainer.cpp
	src/screenComponents/globalMessage.cpp
	src/screenComponents/commsOverlay.cpp
	src/screenComponents/jumpIndicator.cpp
	src/screenComponents/scanningDialog.cpp
	src/screenComponents/signalQualityIndicator.cpp
	src/screenComponents/mainScreenControls.cpp
	src/screenComponents/databaseView.cpp
	src/screenComponents/shipsLogControl.cpp
	src/screenComponents/onScreenKeyboard.cpp
	src/screenComponents/hackingDialog.cpp
	src/screenComponents/customShipFunctions.cpp
	src/screenComponents/scrollingBanner.cpp
	src/screenComponents/lightsOut.cpp
	src/screenComponents/miniGame.cpp
	src/screenComponents/mineSweeper.cpp
	src/screenComponents/impulseSound.cpp
	src/gui/colorConfig.cpp
	src/gui/hotkeyBinder.cpp
	src/gui/hotkeyConfig.cpp
	src/gui/joystickConfig.cpp
	src/gui/mouseRenderer.cpp
	src/gui/scriptError.cpp
	src/gui/gui2_slider.cpp
	src/gui/gui2_togglebutton.cpp
	src/gui/gui2_arrow.cpp
	src/gui/gui2_selector.cpp
	src/gui/gui2_canvas.cpp
	src/gui/gui2_rotationdial.cpp
	src/gui/gui2_textentry.cpp
	src/gui/gui2_label.cpp
	src/gui/gui2_image.cpp
	src/gui/gui2_autolayout.cpp
	src/gui/gui2_arrowbutton.cpp
	src/gui/gui2_entrylist.cpp
	src/gui/gui2_progressbar.cpp
	src/gui/gui2_progressslider.cpp
	src/gui/gui2_scrolltext.cpp
	src/gui/gui2_advancedscrolltext.cpp
	src/gui/gui2_button.cpp
	src/gui/gui2_resizabledialog.cpp
	src/gui/debugRenderer.cpp
	src/gui/gui2_element.cpp
	src/gui/gui2_keyvaluedisplay.cpp
	src/gui/gui2_listbox.cpp
	src/gui/gui2_scrollbar.cpp
	src/gui/gui2_container.cpp
	src/gui/gui2_panel.cpp
	src/gui/gui2_overlay.cpp
	src/spaceObjects/missiles/missileWeapon.cpp
	src/spaceObjects/missiles/EMPMissile.cpp
	src/spaceObjects/missiles/homingMissile.cpp
	src/spaceObjects/missiles/hvli.cpp
	src/spaceObjects/missiles/nuke.cpp
	src/spaceObjects/spaceStation.cpp
	src/spaceObjects/spaceship.cpp
	src/spaceObjects/wormHole.cpp
	src/spaceObjects/spaceObject.cpp
	src/spaceObjects/nebula.cpp
	src/spaceObjects/explosionEffect.cpp
	src/spaceObjects/cpuShip.cpp
	src/spaceObjects/asteroid.cpp
	src/spaceObjects/mine.cpp
	src/spaceObjects/blackHole.cpp
	src/spaceObjects/playerSpaceship.cpp
	src/spaceObjects/beamEffect.cpp
	src/spaceObjects/electricExplosionEffect.cpp
	src/spaceObjects/supplyDrop.cpp
	src/spaceObjects/warpJammer.cpp
	src/spaceObjects/scanProbe.cpp
	src/spaceObjects/artifact.cpp
	src/spaceObjects/shipTemplateBasedObject.cpp
	src/spaceObjects/planet.cpp
	src/spaceObjects/zone.cpp
	src/spaceObjects/spaceshipParts/beamWeapon.cpp
	src/spaceObjects/spaceshipParts/weaponTube.cpp
	src/ai/fighterAI.cpp
	src/ai/ai.cpp
	src/ai/aiFactory.cpp
	src/ai/evasionAI.cpp
	src/ai/missileVolleyAI.cpp
	src/hardware/hardwareController.cpp
	src/hardware/hardwareMappingEffects.cpp
	src/hardware/serialDriver.cpp
	src/hardware/devices/dmx512SerialDevice.cpp
	src/hardware/devices/enttecDMXProDevice.cpp
	src/hardware/devices/sACNDMXDevice.cpp
	src/hardware/devices/uDMXDevice.cpp
	src/hardware/devices/virtualOutputDevice.cpp
	src/hardware/devices/philipsHueDevice.cpp
	$<$<BOOL:${WIN32}>:EmptyEpsilon.rc>
	$<$<BOOL:${WIN32}>:src/discord.cpp>
)

if(ANDROID)
    add_library(${PROJECT_NAME} SHARED ${SOURCES})
else()
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE WIN32 ${SOURCES})
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
	"${PROJECT_SOURCE_DIR}/src"
	${SFML_INCLUDE_DIR}
	${SDL2_INCLUDE_DIRS}
	$<$<AND:$<BOOL:${MINGW}>,$<BOOL:${ENABLE_CRASH_LOGGER}>>:${DRMINGW_ROOT}/include>
)

target_compile_options(${PROJECT_NAME} PUBLIC
	$<$<NOT:$<BOOL:${MSVC}>>:-Wall -Werror=return-type>
	$<$<BOOL:${MSVC}>:/MP /permissive->
)
set_target_properties(${PROJECT_NAME} PROPERTIES
	#LINK_FLAGS $<$<SYSTEM_NAME:Darwin>:-framework OpenGL -framework Foundation>
	MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/osx/MacOSXBundleInfo.plist.in
	MACOSX_BUNDLE_ICON_FILE "${PROJECT_NAME}.icns"
)

if(ANDROID)
  target_link_libraries(${PROJECT_NAME}
	  GLESv1_CM ${SFML_main_LIBRARY} log android EGL
	  "-Wl,--whole-archive ${SFML_INSTALL_PATH}/lib/${ANDROID_ABI}/libsfml-main.a -Wl,--no-whole-archive")
else()
	target_link_libraries(${PROJECT_NAME} PUBLIC
	seriousproton meshoptimizer
	${SDL2_LIBRARIES}
	$<$<BOOL:${WIN32}>:wsock32> $<$<BOOL:${WIN32}>:iphlpapi> $<$<BOOL:${WIN32}>:discord>
	$<$<AND:$<BOOL:${MINGW}>,$<BOOL:${ENABLE_CRASH_LOGGER}>>:${DRMINGW_ROOT}/lib/exchndl>
)	  
endif()

if(COMPRESS_TEXTURES)
	message(STATUS "Compressing textures to ${COMPRESS_TEXTURES}")

	include(ExternalProject)
	set(texconverter_prefix "${PROJECT_BINARY_DIR}/texconverter")

    ExternalProject_Add(texconverter
        SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/texconverter"
        CMAKE_ARGS
			-DCMAKE_BUILD_TYPE:STRING=Release
            -DCMAKE_INSTALL_PREFIX:PATH=${texconverter_prefix}
		INSTALL_COMMAND
			${CMAKE_COMMAND} --build . --target install --config Release
    )
	
	set(texconverter_bin "${texconverter_prefix}/bin/texconverter${CMAKE_EXECUTABLE_SUFFIX}")

	file(GLOB_RECURSE png_textures LIST_DIRECTORIES false RELATIVE "${CMAKE_SOURCE_DIR}/resources" "*.png")
	list(FILTER png_textures EXCLUDE REGEX "^\\.\\.")
	string(REPLACE ".png" "" png_textures "${png_textures}")

	# Filter out generated textures.
	list(FILTER png_textures EXCLUDE REGEX "-dxt$")
	list(FILTER png_textures EXCLUDE REGEX "-astc$")
	set(ktx_textures)
	foreach(png_texture IN LISTS png_textures)
		get_filename_component(basename "${png_texture}" NAME)
		get_filename_component(texture_dir "${png_texture}" DIRECTORY)
		set(texture_abspath "${CMAKE_SOURCE_DIR}/resources/${texture_dir}")
		set(ktx_outputs)
		if("astc" IN_LIST COMPRESS_TEXTURES)
			list(APPEND ktx_outputs "${texture_abspath}/${basename}-astc.ktx")
		endif()
		if("dxt" IN_LIST COMPRESS_TEXTURES)
			list(APPEND ktx_outputs "${texture_abspath}/${basename}-dxt.ktx")
		endif()
		add_custom_command(
			OUTPUT ${ktx_outputs}
			COMMAND "${texconverter_bin}" single "${basename}.png" ${COMPRESS_TEXTURES}
			WORKING_DIRECTORY "${texture_abspath}"
			MAIN_DEPENDENCY "${texture_abspath}/${basename}.png"
			DEPENDS texconverter
			COMMAND_EXPAND_LISTS
			VERBATIM
		)
		list(APPEND ktx_textures "${ktx_outputs}")
	endforeach()
	add_custom_target(convert_png DEPENDS ${ktx_textures})

	file(GLOB_RECURSE pack_files LIST_DIRECTORIES false RELATIVE "${CMAKE_SOURCE_DIR}/packs" "*.pack")
	list(FILTER pack_files EXCLUDE REGEX "^\\.\\.")
	string(REPLACE ".pack" "" pack_files "${pack_files}")

	list(FILTER pack_files EXCLUDE REGEX "-dxt$")
	list(FILTER pack_files EXCLUDE REGEX "-astc$")
	set(repacked)
	foreach(packfile IN LISTS pack_files)
		get_filename_component(basename "${packfile}" NAME)
		get_filename_component(pack_dir "${packfile}" DIRECTORY)
		set(pack_abspath "${CMAKE_SOURCE_DIR}/packs/${pack_dir}")
		set(texpack_outputs)
		if("astc" IN_LIST COMPRESS_TEXTURES)
			list(APPEND texpack_outputs "${pack_abspath}/${basename}-astc.pack")
		endif()
		if("dxt" IN_LIST COMPRESS_TEXTURES)
			list(APPEND texpack_outputs "${pack_abspath}/${basename}-dxt.pack")
		endif()

		add_custom_command(
			OUTPUT ${texpack_outputs}
			COMMAND "${texconverter_bin}" pack "${basename}.pack" ${COMPRESS_TEXTURES}
			WORKING_DIRECTORY "${pack_abspath}"
				MAIN_DEPENDENCY "${pack_abspath}/${basename}.pack"
				DEPENDS texconverter
				COMMAND_EXPAND_LISTS
				VERBATIM
		)

		list(APPEND repacked "${texpack_outputs}")
	endforeach()
	add_custom_target(convert_packs DEPENDS ${repacked})
	add_custom_target(convert_textures DEPENDS convert_png convert_packs)
	add_dependencies(${PROJECT_NAME} convert_textures)
endif()


include(InstallRequiredSystemLibraries)

if(WIN32)
if(MINGW)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libstdc++-6.dll OUTPUT_VARIABLE MINGW_STDCPP_DLL OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libgcc_s_sjlj-1.dll OUTPUT_VARIABLE MINGW_LIBGCC_DLL OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND ${CMAKE_CXX_COMPILER} -print-file-name=libwinpthread-1.dll OUTPUT_VARIABLE MINGW_PTHREAD_DLL OUTPUT_STRIP_TRAILING_WHITESPACE)
  install(FILES ${MINGW_STDCPP_DLL} ${MINGW_LIBGCC_DLL} ${MINGW_PTHREAD_DLL} DESTINATION .)
endif()
  install(FILES ${SFML_ROOT}/bin/OpenAL32.dll DESTINATION .)
  #install(FILES ${SFML_ROOT}/bin/libFLAC-8.dll DESTINATION .)
  #install(FILES ${SFML_ROOT}/bin/libjpeg-62.dll DESTINATION .)
  #install(FILES ${SFML_ROOT}/bin/libogg-0.dll DESTINATION .)
  #install(FILES ${SFML_ROOT}/bin/libvorbis-0.dll DESTINATION .)
  #install(FILES ${SFML_ROOT}/bin/libvorbisenc-2.dll DESTINATION .)
  #install(FILES ${SFML_ROOT}/bin/libvorbisfile-3.dll DESTINATION .)
  install(FILES ${SFML_ROOT}/bin/sfml-audio-2.dll DESTINATION .)
  install(FILES ${SFML_ROOT}/bin/sfml-graphics-2.dll DESTINATION .)
  install(FILES ${SFML_ROOT}/bin/sfml-network-2.dll DESTINATION .)
  install(FILES ${SFML_ROOT}/bin/sfml-system-2.dll DESTINATION .)
  install(FILES ${SFML_ROOT}/bin/sfml-window-2.dll DESTINATION .)
  if(ENABLE_CRASH_LOGGER AND MINGW)
    install(FILES ${DRMINGW_ROOT}/bin/dbghelp.dll DESTINATION .)
    install(FILES ${DRMINGW_ROOT}/bin/exchndl.dll DESTINATION .)
    install(FILES ${DRMINGW_ROOT}/bin/mgwhelp.dll DESTINATION .)
    install(FILES ${DRMINGW_ROOT}/bin/symsrv.dll DESTINATION .)
    install(FILES ${DRMINGW_ROOT}/bin/symsrv.yes DESTINATION .)
  endif()
  install(FILES "${DISCORD_LIB_PATH}/discord_game_sdk.dll" DESTINATION .)

  install(DIRECTORY resources DESTINATION .)
  install(DIRECTORY scripts DESTINATION .)
  install(DIRECTORY packs DESTINATION .)
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
elseif(APPLE)
  install(FILES logo.icns DESTINATION "EmptyEpsilon.app/Contents/Resources" RENAME "${PROJECT_NAME}.icns")

  install(DIRECTORY resources DESTINATION "EmptyEpsilon.app/Contents/Resources")
  install(DIRECTORY scripts DESTINATION "EmptyEpsilon.app/Contents/Resources")
  install(DIRECTORY packs DESTINATION "EmptyEpsilon.app/Contents/Resources")
elseif(ANDROID)
  android_apk(EmptyEpsilon)
else()
	include(GNUInstallDirs)
	
	set(resource_root "${CMAKE_INSTALL_DATAROOTDIR}/emptyepsilon/")
	set(bin_root "${CMAKE_INSTALL_BINDIR}")
	if(CMAKE_CROSSCOMPILING AND "${CMAKE_SYSTEM_PROCESSOR}" STREQUAL "armv7")
		set(resource_root ".")
		set(bin_root ".")
		install(PROGRAMS "${CMAKE_SYSROOT}/${SFML_ROOT}/lib/libsfml-audio.so.2.5.1"
				DESTINATION "${bin_root}"
				RENAME libsfml-audio.so.2.5)
		install(PROGRAMS "${CMAKE_SYSROOT}/${SFML_ROOT}/lib/libsfml-network.so.2.5.1"
				DESTINATION "${bin_root}"
				RENAME libsfml-network.so.2.5)
		install(PROGRAMS "${CMAKE_SYSROOT}/${SFML_ROOT}/lib/libsfml-system.so.2.5.1"
				DESTINATION "${bin_root}"
				RENAME libsfml-system.so.2.5)
	endif()
  
  set(PNG_EXCLUDE)
  set(KTX_EXCLUDE EXCLUDE)

  if(COMPRESS_TEXTURES)
	set(PNG_EXCLUDE EXCLUDE)
	set(KTX_EXCLUDE)
  endif()

  install(DIRECTORY resources
		DESTINATION "${resource_root}"
		FILES_MATCHING
			PATTERN "*.png" ${PNG_EXCLUDE}
			PATTERN "*.ktx" ${KTX_EXCLUDE}
			PATTERN "*"
	)
  install(DIRECTORY scripts packs DESTINATION "${resource_root}")
  install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION "${bin_root}")
endif()

find_package(PythonInterp)
if(PYTHONINTERP_FOUND)
  add_custom_command(
        OUTPUT ${PROJECT_BINARY_DIR}/script_reference.html
        COMMAND ${PYTHON_EXECUTABLE} ${PROJECT_SOURCE_DIR}/compile_script_docs.py ${PROJECT_BINARY_DIR}/script_reference.html
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        COMMENT "Building script reference documentation.")
  add_custom_target(script_reference ALL DEPENDS ${CMAKE_BINARY_DIR}/script_reference.html)
  
  # Matches install logic above.
  if(WIN32)
  	install(FILES ${CMAKE_BINARY_DIR}/script_reference.html DESTINATION .)
  elseif(APPLE)
  	install(FILES ${CMAKE_BINARY_DIR}/script_reference.html DESTINATION "EmptyEpsilon.app/Contents/Resources")
  elseif(NOT ANDROID)
	# DOCDIR already has PROJECT_NAME (EmptyEpsilon) appended (from CMake docs)
  	install(FILES ${CMAKE_BINARY_DIR}/script_reference.html DESTINATION "${CMAKE_INSTALL_DOCDIR}")
  endif()
endif()

add_custom_target(update_locale
	COMMAND sed -i "/^#: /d" resources/locale/en.po
	COMMAND sed -i "/^#: /d" resources/locale/tutorial.en.po
	COMMAND xgettext --keyword=tr:1c,2 --keyword=tr:1 --keyword=trMark:1c,2 --keyword=trMark:1 --omit-header -j -d resources/locale/en ${SOURCES}
	COMMAND xgettext --keyword=_:1c,2 --keyword=_:1 --omit-header -j -d resources/locale/en scripts/shipTemplates_*.lua scripts/comms_ship.lua scripts/comms_station.lua scripts/comms_supply_drop.lua scripts/factionInfo.lua scripts/science_db.lua
	COMMAND xgettext --keyword=_:1c,2 --keyword=_:1 --omit-header -j -d resources/locale/tutorial.en scripts/tutorial_*.lua
	WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})

set(CPACK_PACKAGE_EXECUTABLES ${PROJECT_NAME})
set(CPACK_PACKAGE_FILE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_CONTACT "https://github.com/daid/")
if(NOT DEFINED CPACK_GENERATOR)
  if(WIN32)
    set(CPACK_GENERATOR "ZIP")
  elseif(UNIX)
    set(CPACK_GENERATOR "DEB")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsfml")
  endif()
endif()

include(CPack)
