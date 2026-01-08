#define __SIMULATOR__
#define __RUNTIME_DEBUG__
#define __RUNTIME_FULL_UNIT_TEST___

#include "common.h"

#include "../../src/VovkPLCRuntime.h"

void custom_test();

WASM_EXPORT void doSomething() {
    custom_test();
}

const char* buggyAssembly = R"(
################# Linter Test Assembly

# Error 1: Typo in instruction
u8.const 10
u8.typomistake 

# Error 2: Missing operand
u8.const 

# Error 3: Jump to undefined label
jmp undefined_label

)";

void custom_test() {
    Serial.println(F("Running Linter Test with 3 expected errors..."));

    // 1. Reset linter
    defaultLinter.clearArray();
    
    // 2. Set assembly
    defaultLinter.set_assembly_string((char*) buggyAssembly);
    
    // 3. Run compilation (linting)
    // We expect compileAssembly to return false because there are errors.
    bool success = defaultLinter.compileAssembly(false, true);
    
    if (success) {
        Serial.println(F("Compilation UNEXPECTEDLY SUCCEEDED!"));
    } else {
        Serial.println(F("Compilation failed (as expected)."));
    }

    // 4. Check errors
    int found_errors = defaultLinter.problem_count;
    Serial.print(F("Linter reported ")); 
    Serial.print(found_errors); 
    Serial.println(F(" errors."));

    if (found_errors == 3) {
        Serial.println(F("SUCCESS: All 3 errors were found."));
    } else {
        Serial.print(F("FAILURE: Expected 3 errors, but found "));
        Serial.println(found_errors);
        
        Serial.println(F("Errors found:"));
        for (int i = 0; i < found_errors; i++) {
            char token_text[64] = {0};
            // Safely copy token text
            int j = 0;
            for (j = 0; j < defaultLinter.problems[i].length && j < 63; j++) {
                token_text[j] = defaultLinter.problems[i].token_text[j];
            }
            token_text[j] = '\0';
            Serial.print(F("  ")); Serial.print(i + 1); Serial.print(F(" ["));
            Serial.print((defaultLinter.problems[i].type == LINT_ERROR) ? F("ERROR") : F("WARNING")); Serial.print(F(" @ "));
            Serial.print(defaultLinter.problems[i].line); Serial.print(F(":")); Serial.print(defaultLinter.problems[i].column); Serial.print(F(":"));
            Serial.print(defaultLinter.problems[i].length); Serial.print(F("] "));
            Serial.print(defaultLinter.problems[i].message); Serial.print(F(" -> "));
            Serial.println(token_text);
        }
        
        if (found_errors < 3) {
            Serial.println(F("Note: The linter might be stopping at the first error."));
        }
    }
}
