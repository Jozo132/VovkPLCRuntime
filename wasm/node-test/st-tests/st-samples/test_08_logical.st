(* Test 8: Logical Operators *)

VAR
    result : BOOL;
    a : BOOL := TRUE;
    b : BOOL := FALSE;
END_VAR

result := a AND b;
result := a OR b;
result := a XOR b;
result := NOT a;
