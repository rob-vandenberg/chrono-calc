# --- Version ------------------------------------------------------------
# tests/CMakeLists.txt 0.8.13

# --- Version history ----------------------------------------------------
# v0.8.13: test_reference_unitconversion, test_reference_datecalculation and test_reference_worksheets (the Windows 7
#          Unit conversion, Date calculation and Worksheets results); the CLI shows the panels
# v0.5.12: test_reference_history (the Windows 7 History, Digit grouping, Copy and Paste results); the CLI pastes
# v0.4.11: test_reference_statistics (the Windows 7 Statistics results); the CLI shows the Statistics list
# v0.3.10: test_keyboardlayouts (which layout a key comes from)
# v0.3.8: test_reference_programmer (the Windows 7 Programmer results)
# v0.2.7: test_reference_scientific (the Windows 7 Scientific results)
# v0.1.6: test_standardview (the buttons and menus of the Standard view)
# v0.1.5: test_reference_standard (the Windows 7 results)
# v0.0.4: test_calculator (the engine wrapper, commands and keys); chrono-calc-cli checked on exit code, output and error
# v0.0.3: Microsoft's engine tests in engine/; the layer check also reads third_party/
# v0.0.2: Initial: the version, main window and layer tests; 20 seconds at most each, no screen needed

# One Qt Test program per file; each test runs without a screen and stops after 20 seconds (AGENTS.md 4.5.3)
function(chrono_calc_test name)
    add_executable(${name} ${name}.cpp)
    set_target_properties(${name} PROPERTIES AUTOMOC ON)
    target_link_libraries(${name} PRIVATE Qt6::Test ${ARGN})
    chrono_calc_strict(${name})
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES TIMEOUT 20 ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

chrono_calc_test(test_version chrono_calc_core)
target_compile_definitions(test_version PRIVATE CHRONO_CALC_PROJECT_VERSION="${PROJECT_VERSION}")

chrono_calc_test(test_mainwindow chrono_calc_ui)
chrono_calc_test(test_keyboardlayouts chrono_calc_ui)

chrono_calc_test(test_calculator chrono_calc_core)
chrono_calc_test(test_reference_standard chrono_calc_core)
chrono_calc_test(test_standardview chrono_calc_core)
chrono_calc_test(test_reference_scientific chrono_calc_core)
chrono_calc_test(test_reference_programmer chrono_calc_core)
chrono_calc_test(test_reference_statistics chrono_calc_core)
chrono_calc_test(test_reference_history chrono_calc_core)
chrono_calc_test(test_reference_unitconversion chrono_calc_core)
chrono_calc_test(test_reference_datecalculation chrono_calc_core)
chrono_calc_test(test_reference_worksheets chrono_calc_core)

# The command-line driver, checked on exit code, output and error text (cli_check.cmake)
function(chrono_calc_cli_test name exit_code output)
    add_test(NAME ${name} COMMAND ${CMAKE_COMMAND} -DCLI=$<TARGET_FILE:chrono-calc-cli> "-DARGUMENTS=${ARGN}"
        -DEXIT=${exit_code} "-DOUTPUT=${output}" ${CHRONO_CALC_CLI_ERROR} -P ${CMAKE_CURRENT_SOURCE_DIR}/cli_check.cmake)
    set_tests_properties(${name} PROPERTIES TIMEOUT 20)
endfunction()

# 12 + 3 x 2 = 30 from left to right, then a command id, then the memory
chrono_calc_cli_test(cli_runs_keys_and_commands 0 "display: 0\nexpression: \nerror: no\nmemory: yes\n" 12+3*2= memory.store clear)
chrono_calc_cli_test(cli_shows_the_result 0 "^display: 30\n" 12+3*2=)
# The Statistics view: the list, its count and the selected item; item 1 selected, then Delete
chrono_calc_cli_test(cli_shows_the_statistics_list 0 "count: 2\nvalues: 5 \\| 4\nselected: 1\n$" view.statistics 5 statistics.add 3 statistics.add 4 statistics.add statistics.select.1 statistics.delete)
# A pasted text and History: the entries, the selected one, and Copy history
chrono_calc_cli_test(cli_pastes_and_keeps_the_history 0 "display: 12\nexpression: \nerror: no\nmemory: no\nhistory: 2 \\* 3 \\| 4 \\* 3\nselected entry: 1\ncopied: 2 \\* 3\\\\n4 \\* 3\\\\n\n$" "edit.paste=2*3=4=" history.copy)
# The Unit conversion panel: its type, From and To (5 Inch is 12.7 Centimeters)
chrono_calc_cli_test(cli_shows_the_unit_conversion 0 "unit: Length\nfrom: 5 Inch\nto: 12.7 Centimeters\n$" view.unit_conversion unit.type.3 unit.from.6 unit.to.1 unit.input=5)
# The Date calculation panel: the difference between two dates, and a date with years, months and days subtracted
chrono_calc_cli_test(cli_shows_the_date_difference 0 "dates: 2026-01-15 to 2027-03-20\ndifference: 1 year, 2 months, 5 days\ndays: 429 days\n$" view.date_calculation date.difference_from=2026-01-15 date.difference_to=2027-03-20 date.calculate)
chrono_calc_cli_test(cli_shows_the_shifted_date 0 "date: 2026-10-30 subtract 1 2 3\nresult: 2025-08-27\n$" view.date_calculation date.mode.1 date.shift_from=2026-10-30 date.subtract date.years=1 date.months=2 date.days=3 date.calculate)
# A Worksheet: Mortgage's monthly payment, and a box left empty
chrono_calc_cli_test(cli_shows_the_worksheet 0 "worksheet: Mortgage\ncalculate: Monthly payment\nPurchase price: 250000\nDown payment: 50000\nTerm \\(years\\): 30\nInterest rate \\(%\\): 4.5\nresult: 1013.370619651761\n$" view.mortgage worksheet.value.0=250000 worksheet.value.1=50000 worksheet.value.2=30 worksheet.value.3=4.5 worksheet.calculate)
chrono_calc_cli_test(cli_shows_the_worksheet_message 0 "result: \nmessage: This value cannot be blank. \\(Fuel used \\(gallons\\)\\)\n$" view.fuel_economy_mpg worksheet.value.0=100 worksheet.calculate)
set(CHRONO_CALC_CLI_ERROR "-DERROR=cannot read 'x' in 2x3")
chrono_calc_cli_test(cli_refuses_an_unknown_argument 2 "^$" 2x3)
unset(CHRONO_CALC_CLI_ERROR)

chrono_calc_test(test_layers)
target_compile_definitions(test_layers PRIVATE CHRONO_CALC_SOURCE_DIR="${PROJECT_SOURCE_DIR}/src"
    CHRONO_CALC_THIRD_PARTY_DIR="${PROJECT_SOURCE_DIR}/third_party")

add_subdirectory(engine)
