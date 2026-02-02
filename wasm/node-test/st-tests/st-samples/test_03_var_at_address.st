(* Test 3: Variable Declaration with AT address *)

VAR
    input1 AT %IX0.0 : BOOL;
    output1 AT %QX0.0 : BOOL;
    memory1 AT %MW10 : INT;
END_VAR
