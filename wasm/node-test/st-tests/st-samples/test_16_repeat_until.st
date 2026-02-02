(* Test 16: REPEAT-UNTIL Loop *)

VAR
    count : INT := 0;
END_VAR

REPEAT
    count := count + 1;
UNTIL count >= 10
END_REPEAT;
