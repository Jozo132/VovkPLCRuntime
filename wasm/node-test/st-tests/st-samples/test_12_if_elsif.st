(* Test 12: IF-ELSIF-ELSE-END_IF *)

VAR
    value : INT;
    result : INT;
END_VAR

IF value = 1 THEN
    result := 10;
ELSIF value = 2 THEN
    result := 20;
ELSIF value = 3 THEN
    result := 30;
ELSE
    result := 0;
END_IF;
