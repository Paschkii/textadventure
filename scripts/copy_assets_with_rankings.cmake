cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED SOURCE_DIR OR NOT DEFINED DEST_DIR)
    message(FATAL_ERROR "copy_assets_with_rankings.cmake requires SOURCE_DIR and DEST_DIR")
endif()

set(rankings_rel "assets/data/rankings.json")
set(rankings_dest "${DEST_DIR}/${rankings_rel}")
set(rankings_backup "")
set(rankings_preserved FALSE)

if(EXISTS "${rankings_dest}")
    file(READ "${rankings_dest}" rankings_backup)
    set(rankings_preserved TRUE)
endif()

file(REMOVE_RECURSE "${DEST_DIR}/assets")
file(COPY "${SOURCE_DIR}/assets" DESTINATION "${DEST_DIR}")

if(rankings_preserved)
    get_filename_component(rankings_parent "${rankings_dest}" DIRECTORY)
    file(MAKE_DIRECTORY "${rankings_parent}")
    file(WRITE "${rankings_dest}" "${rankings_backup}")
endif()
