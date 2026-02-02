(* Test 20: Complex Nested Expression *)

VAR
    result : REAL;
    a : REAL := 1.0;
    b : REAL := 2.0;
    c : REAL := 3.0;
END_VAR

result := (a + b) * (c - a) / (b + c);
