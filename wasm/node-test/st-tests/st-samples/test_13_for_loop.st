(* Test 13: Simple FOR Loop *)

VAR
    i : INT;
    sum : INT := 0;
END_VAR

FOR i := 1 TO 10 DO
    sum := sum + i;
END_FOR;
