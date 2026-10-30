# --- Version ------------------------------------------------------------
# tests/engine/CMakeLists.txt 0.0.3

# --- Version history ----------------------------------------------------
# v0.0.3: Initial: Microsoft's engine tests, ported to Qt Test (D-014)

# The engine's English texts, as the original tests see them
add_library(chrono_calc_engine_test_texts STATIC engine_resource_provider.cpp engine_resource_provider.h)
target_link_libraries(chrono_calc_engine_test_texts PUBLIC chrono_calc_engine)
chrono_calc_strict(chrono_calc_engine_test_texts)

chrono_calc_test(test_engine_rational chrono_calc_engine)
chrono_calc_test(test_engine_calcinput chrono_calc_engine)
chrono_calc_test(test_engine_calcengine chrono_calc_engine_test_texts)
chrono_calc_test(test_engine_calculatormanager chrono_calc_engine_test_texts)
