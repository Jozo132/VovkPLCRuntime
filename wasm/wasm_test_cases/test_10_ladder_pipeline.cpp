// test_10_ladder_pipeline.cpp - Memory leak test for Ladder → Network IR → STL → PLCASM pipeline
// Tests the full compilation pipeline for memory leaks and correctness

#define __SIMULATOR__
#define __RUNTIME_DEBUG__
#define __RUNTIME_FULL_UNIT_TEST___

#include "common.h"

#include "../../src/VovkPLCRuntime.h"

void test_ladder_pipeline();

WASM_EXPORT void doSomething() {
    test_ladder_pipeline();
}

// Test cases - Ladder JSON inputs
const char* ladder_simple = R"({"rungs":[{"elements":[{"type":"contact","address":"X0"},{"type":"coil","address":"Y0"}]}]})";

const char* ladder_series = R"({"rungs":[{"elements":[{"type":"contact","address":"X0"},{"type":"contact","address":"X1"},{"type":"coil","address":"Y0"}]}]})";

const char* ladder_multiple_coils = R"({"rungs":[{"elements":[{"type":"contact","address":"X0"},{"type":"coil","address":"Y0"},{"type":"coil","address":"Y1"},{"type":"coil_set","address":"M0.0"}]}]})";

const char* ladder_passthrough_inc = R"({"rungs":[{"elements":[{"type":"contact","address":"X0"},{"type":"inc","address":"MW0","passThrough":true},{"type":"inc","address":"MW2","passThrough":true},{"type":"coil","address":"Y0","passThrough":true}]}]})";

const char* ladder_with_tap = R"({"rungs":[{"elements":[{"type":"contact","address":"X0"},{"type":"inc","address":"MW0"},{"type":"tap"},{"type":"contact","address":"X1"},{"type":"coil","address":"Y0"}]}]})";

const char* ladder_timer = R"({"rungs":[{"elements":[{"type":"contact","address":"X0"},{"type":"timer_ton","address":"T0","preset":"T#1s"},{"type":"coil","address":"Y0"}]}]})";

const char* ladder_counter = R"({"rungs":[{"elements":[{"type":"contact","address":"X0","trigger":"rising"},{"type":"counter_up","address":"C0","preset":10},{"type":"coil","address":"Y0"}]}]})";

const char* ladder_math = R"({"rungs":[{"elements":[{"type":"contact","address":"X0"},{"type":"math_add","in1":"MW0","in2":"#10","out":"MW0","passThrough":true},{"type":"coil","address":"Y0"}]}]})";

const char* ladder_or_branches = R"({"rungs":[{"comment":"OR logic","elements":[{"type":"contact","address":"X0"},{"type":"coil","address":"Y0"}],"branches":[{"elements":[{"type":"contact","address":"X1"}]}]}]})";

const char* ladder_complex = R"({"rungs":[{"elements":[{"type":"contact","address":"X0"},{"type":"inc","address":"MW14","passThrough":true},{"type":"inc","address":"MW14","passThrough":true},{"type":"inc","address":"MW22","passThrough":true}]}]})";


// Helper to load ladder JSON into the compiler
void load_ladder_json(const char* json) {
    int len = 0;
    while (json[len]) len++;
    
    for (int i = 0; i < len; i++) {
        streamIn(json[i]);
    }
    streamIn(0); // Null terminator
    
    ladder_load_from_stream();
}

// Test function that runs multiple iterations to detect memory leaks
void test_ladder_pipeline() {
    Serial.println("=== Ladder Pipeline Memory Leak Test ===\n");
    
    const char* test_cases[] = {
        ladder_simple,
        ladder_series,
        ladder_multiple_coils,
        ladder_passthrough_inc,
        ladder_with_tap,
        ladder_timer,
        ladder_counter,
        ladder_math,
        ladder_or_branches,
        ladder_complex
    };
    
    const char* test_names[] = {
        "Simple contact->coil",
        "Series contacts",
        "Multiple coils",
        "Pass-through inc",
        "TAP instruction",
        "Timer TON",
        "Counter CTU",
        "Math ADD",
        "OR branches",
        "Complex pass-through"
    };
    
    const int num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    const int iterations = 100; // Run each test multiple times to detect leaks
    
    int total_passed = 0;
    int total_failed = 0;
    
    for (int t = 0; t < num_tests; t++) {
        Serial.println();
        Serial.print("==== Test ");
        Serial.print(t + 1);
        Serial.print(": ");
        Serial.print(test_names[t]);
        Serial.println(" ====");
        
        bool test_passed = true;
        
        for (int iter = 0; iter < iterations; iter++) {
            // Reset NIR store before each compilation
            nir_reset();
            
            // Load and compile ladder JSON
            load_ladder_json(test_cases[t]);
            
            bool success = ladder_compile();
            
            if (!success) {
                if (iter == 0) { // Only report first failure
                    Serial.print("FAILED (compilation error): ");
                    Serial.println(ladder_get_error());
                }
                test_passed = false;
                break;
            }
            
            // Get the STL output
            const char* stl_output = ladder_get_output();
            int stl_len = ladder_get_output_length();
            
            if (stl_len == 0) {
                if (iter == 0) {
                    Serial.println("FAILED (empty STL output)");
                }
                test_passed = false;
                break;
            }
            
            // Show STL on first iteration
            if (iter == 0) {
                Serial.println("Generated STL:");
                Serial.println(stl_output);
            }
            
            // Now compile STL to PLCASM using the full pipeline
            // Load STL into stream for STL compiler
            for (int i = 0; i < stl_len; i++) {
                streamIn(stl_output[i]);
            }
            streamIn(0);
            
            stl_load_from_stream();
            bool stl_success = stl_compile();
            
            if (!stl_success) {
                if (iter == 0) {
                    Serial.print("FAILED (STL compilation error): ");
                    Serial.println(stl_get_error());
                }
                test_passed = false;
                break;
            }
            
            // Get PLCASM output
            const char* plcasm_output = stl_get_output();
            int plcasm_len = stl_get_output_length();
            
            if (plcasm_len == 0) {
                if (iter == 0) {
                    Serial.println("FAILED (empty PLCASM output)");
                }
                test_passed = false;
                break;
            }
            
            // Show PLCASM on first iteration
            if (iter == 0) {
                Serial.println("Generated PLCASM:");
                Serial.println(plcasm_output);
            }
            
            // Compile PLCASM to bytecode using global API
            set_assembly_string((char*)plcasm_output);
            bool asm_error = compileAssembly(false);
            
            if (asm_error) {
                if (iter == 0) {
                    Serial.println("FAILED (PLCASM compilation error)");
                }
                test_passed = false;
                break;
            }
            
            // Show bytecode and step-by-step execution on first iteration
            if (iter == 0) {
                logBytecode();
                loadCompiledProgram();
                // Set X0.0 = 1 so conditional operations execute
                runtime.setInputBit(0, 0, true);
                Serial.println("Executing bytecode step-by-step:");
                UnitTest::fullProgramDebug(runtime);
            }
        }
        
        if (test_passed) {
            Serial.print("Result: PASSED (");
            Serial.print(iterations);
            Serial.println(" iterations)");
            total_passed++;
        } else {
            Serial.println("Result: FAILED");
            total_failed++;
        }
    }
    
    Serial.println("\n=== Summary ===");
    Serial.print("Passed: ");
    Serial.print(total_passed);
    Serial.print(" / ");
    Serial.println(num_tests);
    
    if (total_failed > 0) {
        Serial.print("Failed: ");
        Serial.println(total_failed);
    }
    
    Serial.println("\n=== Pipeline Test Complete ===");
}
