# set(${NAME} webserver)

# add_library(webserver INTERFACE)


target_sources(${NAME} PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}/web_server.cpp
)