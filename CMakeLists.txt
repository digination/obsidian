cmake_minimum_required(VERSION 2.6)

project(obsidian)


SET (CMAKE_C_FLAGS "-g -fPIE")

#Déclaration de l'exécutable
add_executable(
	obsidian
	src/obsidian.c
	src/obsidian.h
	src/lib/config.c
	src/lib/config.h
	src/lib/dexp.c
	src/lib/dexp.h
        src/lib/network.c
        src/lib/network.h
        src/lib/fsevents.c
        src/lib/fsevents.h
        src/lib/peer.h
        src/lib/utils.c
        src/lib/utils.h
        src/lib/common.h
        src/lib/crypto.h
        src/lib/crypto.c
        src/lib/filter.h
        src/lib/filter.c
        src/lib/statinfos.h
)


find_library (PTHREAD pthread) 
find_library (SSL ssl)


target_link_libraries(
        obsidian
        pthread
        ssl
)

install (TARGETS obsidian DESTINATION bin)
install (FILES "src/conf/obsidian.conf" DESTINATION etc)


