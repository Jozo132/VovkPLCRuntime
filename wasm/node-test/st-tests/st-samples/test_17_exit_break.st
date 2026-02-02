(* Test 17: EXIT Statement *)

VAR
    i : INT;
END_VAR

FOR i := 1 TO 100 DO
    IF i = 50 THEN
        EXIT;
    END_IF;
END_FOR;
