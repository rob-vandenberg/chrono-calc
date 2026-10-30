# --- Version ------------------------------------------------------------
# tests/cli_check.cmake 0.0.4

# --- Version history ----------------------------------------------------
# v0.0.4: Initial: runs chrono-calc-cli and checks its exit code, its output and its error text

# cmake -DCLI=<path> -DARGUMENTS=<a;b> -DEXIT=<code> -DOUTPUT=<regex> [-DERROR=<regex>] -P cli_check.cmake
execute_process(COMMAND ${CLI} ${ARGUMENTS} RESULT_VARIABLE exit_code OUTPUT_VARIABLE output ERROR_VARIABLE error)
if(NOT exit_code STREQUAL EXIT)
    message(FATAL_ERROR "exit code ${exit_code}, expected ${EXIT}\noutput:\n${output}\nerror:\n${error}")
endif()
if(NOT output MATCHES "${OUTPUT}")
    message(FATAL_ERROR "output does not match '${OUTPUT}':\n${output}")
endif()
if(DEFINED ERROR AND NOT error MATCHES "${ERROR}")
    message(FATAL_ERROR "error text does not match '${ERROR}':\n${error}")
endif()
