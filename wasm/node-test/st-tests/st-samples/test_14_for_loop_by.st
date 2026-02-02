(* Test 14: FOR Loop with BY Step *)

VAR
    i : INT;
    count : INT := 0;
END_VAR

FOR i := 0 TO 100 BY 10 DO
    count := count + 1;
END_FOR;
