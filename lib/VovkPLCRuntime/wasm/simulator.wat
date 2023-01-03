(module
  (type (;0;) (func (param i32 i32 i32 i32)))
  (type (;1;) (func (param i32)))
  (type (;2;) (func (param i32) (result i32)))
  (type (;3;) (func (param i32 i32 i32) (result i32)))
  (type (;4;) (func))
  (type (;5;) (func (param i32 i32 i32 i32 i32) (result i32)))
  (type (;6;) (func (param i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32) (result i32)))
  (type (;7;) (func (param i32 i32 i32 i32 f64 i32 i32 i32) (result i32)))
  (type (;8;) (func (param i32 i32)))
  (type (;9;) (func (param i32) (result i64)))
  (type (;10;) (func (param i32 i64) (result i32)))
  (type (;11;) (func (param i32 i32 i32 i32) (result i32)))
  (type (;12;) (func (param i64)))
  (type (;13;) (func (param i32 i32) (result i32)))
  (type (;14;) (func (param i32 f32) (result i32)))
  (type (;15;) (func (param i32 f64) (result i32)))
  (type (;16;) (func (result i32)))
  (type (;17;) (func (param i32 i32 i32 i32 i32 i32 i32 i32) (result i32)))
  (import "env" "memory" (memory (;0;) 2))
  (import "env" "__print" (func $__print (type 1)))
  (import "env" "_Znam" (func $operator_new___unsigned_long_ (type 2)))
  (import "env" "_ZdaPv" (func $operator_delete___void*_ (type 1)))
  (import "env" "memset" (func $memset (type 3)))
  (import "env" "_Znwm" (func $operator_new_unsigned_long_ (type 2)))
  (func $__wasm_call_ctors (type 4)
    call $_GLOBAL__sub_I_simulator.cpp)
  (func $_vsnprintf_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__char_const*__void*_ (type 5) (param i32 i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32 i64 i32 i64 i64 i64 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 5
    global.set $__stack_pointer
    local.get 0
    i32.const 1
    local.get 1
    select
    local.set 6
    i32.const 0
    local.set 0
    block  ;; label = @1
      loop  ;; label = @2
        local.get 3
        i32.load8_u
        local.tee 7
        i32.eqz
        br_if 1 (;@1;)
        block  ;; label = @3
          local.get 7
          i32.const 37
          i32.eq
          br_if 0 (;@3;)
          local.get 7
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          local.get 1
          local.get 0
          local.get 2
          local.get 6
          call_indirect (type 0)
          local.get 3
          i32.const 1
          i32.add
          local.set 3
          local.get 0
          i32.const 1
          i32.add
          local.set 0
          br 1 (;@2;)
        end
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        i32.const 0
        local.set 8
        loop  ;; label = @3
          i32.const 1
          local.set 7
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        local.get 3
                        i32.load8_s
                        i32.const -32
                        i32.add
                        br_table 3 (;@7;) 6 (;@4;) 6 (;@4;) 4 (;@6;) 6 (;@4;) 6 (;@4;) 6 (;@4;) 6 (;@4;) 6 (;@4;) 6 (;@4;) 6 (;@4;) 2 (;@8;) 6 (;@4;) 1 (;@9;) 6 (;@4;) 6 (;@4;) 0 (;@10;) 6 (;@4;)
                      end
                      local.get 3
                      i32.const 1
                      i32.add
                      local.set 3
                      local.get 8
                      i32.const 1
                      i32.or
                      local.set 8
                      br 4 (;@5;)
                    end
                    local.get 3
                    i32.const 1
                    i32.add
                    local.set 3
                    local.get 8
                    i32.const 2
                    i32.or
                    local.set 8
                    br 3 (;@5;)
                  end
                  local.get 3
                  i32.const 1
                  i32.add
                  local.set 3
                  local.get 8
                  i32.const 4
                  i32.or
                  local.set 8
                  br 2 (;@5;)
                end
                local.get 3
                i32.const 1
                i32.add
                local.set 3
                local.get 8
                i32.const 8
                i32.or
                local.set 8
                br 1 (;@5;)
              end
              local.get 3
              i32.const 1
              i32.add
              local.set 3
              local.get 8
              i32.const 16
              i32.or
              local.set 8
            end
            i32.const 0
            local.set 7
          end
          local.get 7
          i32.eqz
          br_if 0 (;@3;)
        end
        block  ;; label = @3
          block  ;; label = @4
            local.get 3
            i32.load8_u
            local.tee 9
            i32.const -48
            i32.add
            i32.const 255
            i32.and
            i32.const 9
            i32.gt_u
            br_if 0 (;@4;)
            i32.const 0
            local.set 7
            loop  ;; label = @5
              local.get 7
              i32.const 10
              i32.mul
              local.get 3
              i32.load8_s
              i32.add
              i32.const -48
              i32.add
              local.set 7
              local.get 3
              i32.load8_u offset=1
              local.set 9
              local.get 3
              i32.const 1
              i32.add
              local.tee 10
              local.set 3
              local.get 9
              i32.const -48
              i32.add
              i32.const 255
              i32.and
              i32.const 10
              i32.lt_u
              br_if 0 (;@5;)
            end
            local.get 10
            local.set 3
            br 1 (;@3;)
          end
          i32.const 0
          local.set 7
          local.get 9
          i32.const 255
          i32.and
          i32.const 42
          i32.ne
          br_if 0 (;@3;)
          local.get 8
          i32.const 2
          i32.or
          local.get 8
          local.get 4
          i32.load
          local.tee 7
          i32.const 0
          i32.lt_s
          select
          local.set 8
          local.get 7
          local.get 7
          i32.const 31
          i32.shr_s
          local.tee 9
          i32.xor
          local.get 9
          i32.sub
          local.set 7
          local.get 3
          i32.const 1
          i32.add
          local.set 3
          local.get 4
          i32.const 4
          i32.add
          local.set 4
        end
        i32.const 0
        local.set 10
        block  ;; label = @3
          local.get 3
          i32.load8_u
          i32.const 46
          i32.ne
          br_if 0 (;@3;)
          local.get 3
          i32.const 1
          i32.add
          local.set 9
          local.get 8
          i32.const 1024
          i32.or
          local.set 8
          block  ;; label = @4
            local.get 3
            i32.load8_u offset=1
            local.tee 11
            i32.const -48
            i32.add
            i32.const 255
            i32.and
            i32.const 9
            i32.gt_u
            br_if 0 (;@4;)
            i32.const 0
            local.set 10
            loop  ;; label = @5
              local.get 10
              i32.const 10
              i32.mul
              local.get 9
              i32.load8_s
              i32.add
              i32.const -48
              i32.add
              local.set 10
              local.get 9
              i32.load8_u offset=1
              local.set 11
              local.get 9
              i32.const 1
              i32.add
              local.tee 3
              local.set 9
              local.get 11
              i32.const -48
              i32.add
              i32.const 255
              i32.and
              i32.const 10
              i32.lt_u
              br_if 0 (;@5;)
              br 2 (;@3;)
            end
          end
          block  ;; label = @4
            local.get 11
            i32.const 255
            i32.and
            i32.const 42
            i32.eq
            br_if 0 (;@4;)
            local.get 9
            local.set 3
            br 1 (;@3;)
          end
          local.get 4
          i32.load
          local.tee 9
          i32.const 0
          local.get 9
          i32.const 0
          i32.gt_s
          select
          local.set 10
          local.get 3
          i32.const 2
          i32.add
          local.set 3
          local.get 4
          i32.const 4
          i32.add
          local.set 4
        end
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  local.get 3
                  i32.load8_s
                  i32.const -104
                  i32.add
                  i32.const 31
                  i32.rotl
                  br_table 1 (;@6;) 2 (;@5;) 0 (;@7;) 4 (;@3;) 4 (;@3;) 4 (;@3;) 3 (;@4;) 4 (;@3;) 4 (;@3;) 3 (;@4;) 4 (;@3;)
                end
                local.get 3
                i32.load8_u offset=1
                i32.const 108
                i32.ne
                br_if 2 (;@4;)
                local.get 3
                i32.const 2
                i32.add
                local.set 3
                local.get 8
                i32.const 768
                i32.or
                local.set 8
                br 3 (;@3;)
              end
              block  ;; label = @6
                local.get 3
                i32.load8_u offset=1
                i32.const 104
                i32.eq
                br_if 0 (;@6;)
                local.get 3
                i32.const 1
                i32.add
                local.set 3
                local.get 8
                i32.const 128
                i32.or
                local.set 8
                br 3 (;@3;)
              end
              local.get 3
              i32.const 2
              i32.add
              local.set 3
              local.get 8
              i32.const 192
              i32.or
              local.set 8
              br 2 (;@3;)
            end
            local.get 3
            i32.const 1
            i32.add
            local.set 3
            local.get 8
            i32.const 512
            i32.or
            local.set 8
            br 1 (;@3;)
          end
          local.get 3
          i32.const 1
          i32.add
          local.set 3
          local.get 8
          i32.const 256
          i32.or
          local.set 8
        end
        local.get 3
        i32.load8_s
        local.tee 11
        i32.const 255
        i32.and
        local.set 12
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          block  ;; label = @12
                            local.get 11
                            i32.const -37
                            i32.add
                            br_table 6 (;@6;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 2 (;@10;) 1 (;@11;) 2 (;@10;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 0 (;@12;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 0 (;@12;) 3 (;@9;) 0 (;@12;) 2 (;@10;) 1 (;@11;) 2 (;@10;) 7 (;@5;) 0 (;@12;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 7 (;@5;) 0 (;@12;) 5 (;@7;) 7 (;@5;) 7 (;@5;) 4 (;@8;) 7 (;@5;) 0 (;@12;) 7 (;@5;) 7 (;@5;) 0 (;@12;) 7 (;@5;)
                          end
                          i32.const 16
                          local.set 9
                          block  ;; label = @12
                            block  ;; label = @13
                              block  ;; label = @14
                                block  ;; label = @15
                                  local.get 12
                                  i32.const -88
                                  i32.add
                                  br_table 3 (;@12;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 1 (;@14;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 0 (;@15;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 2 (;@13;) 3 (;@12;) 2 (;@13;)
                                end
                                i32.const 8
                                local.set 9
                                br 2 (;@12;)
                              end
                              i32.const 2
                              local.set 9
                              br 1 (;@12;)
                            end
                            local.get 8
                            i32.const -17
                            i32.and
                            local.set 8
                            i32.const 10
                            local.set 9
                          end
                          local.get 8
                          i32.const 32
                          i32.or
                          local.get 8
                          local.get 11
                          i32.const 88
                          i32.eq
                          select
                          local.set 8
                          block  ;; label = @12
                            block  ;; label = @13
                              local.get 12
                              i32.const -100
                              i32.add
                              local.tee 11
                              br_table 1 (;@12;) 0 (;@13;) 0 (;@13;) 0 (;@13;) 0 (;@13;) 1 (;@12;) 0 (;@13;)
                            end
                            local.get 8
                            i32.const -13
                            i32.and
                            local.set 8
                          end
                          local.get 8
                          i32.const -2
                          i32.and
                          local.get 8
                          local.get 8
                          i32.const 1024
                          i32.and
                          select
                          local.set 8
                          block  ;; label = @12
                            block  ;; label = @13
                              local.get 11
                              br_table 0 (;@13;) 1 (;@12;) 1 (;@12;) 1 (;@12;) 1 (;@12;) 0 (;@13;) 1 (;@12;)
                            end
                            block  ;; label = @13
                              local.get 8
                              i32.const 512
                              i32.and
                              i32.eqz
                              br_if 0 (;@13;)
                              local.get 8
                              local.get 8
                              i32.const -17
                              i32.and
                              local.get 4
                              i32.const 7
                              i32.add
                              i32.const -8
                              i32.and
                              local.tee 13
                              i64.load
                              local.tee 14
                              i64.const 0
                              i64.ne
                              local.tee 11
                              select
                              local.set 15
                              block  ;; label = @14
                                block  ;; label = @15
                                  local.get 11
                                  br_if 0 (;@15;)
                                  i32.const 0
                                  local.set 11
                                  local.get 15
                                  i32.const 1024
                                  i32.and
                                  br_if 1 (;@14;)
                                end
                                local.get 9
                                i64.extend_i32_u
                                local.set 16
                                local.get 14
                                local.get 14
                                i64.const 63
                                i64.shr_s
                                local.tee 17
                                i64.xor
                                local.get 17
                                i64.sub
                                local.set 17
                                local.get 15
                                i32.const 32
                                i32.and
                                i32.const 97
                                i32.xor
                                i32.const 246
                                i32.add
                                local.set 12
                                i32.const 0
                                local.set 8
                                loop  ;; label = @15
                                  local.get 5
                                  local.get 8
                                  i32.add
                                  i32.const 48
                                  local.get 12
                                  local.get 17
                                  local.get 17
                                  local.get 16
                                  i64.div_u
                                  local.tee 18
                                  local.get 16
                                  i64.mul
                                  i64.sub
                                  i32.wrap_i64
                                  local.tee 11
                                  i32.const 10
                                  i32.lt_u
                                  select
                                  local.get 11
                                  i32.add
                                  i32.store8
                                  local.get 8
                                  i32.const 1
                                  i32.add
                                  local.set 11
                                  local.get 17
                                  local.get 16
                                  i64.lt_u
                                  br_if 1 (;@14;)
                                  local.get 8
                                  i32.const 31
                                  i32.lt_u
                                  local.set 4
                                  local.get 11
                                  local.set 8
                                  local.get 18
                                  local.set 17
                                  local.get 4
                                  br_if 0 (;@15;)
                                end
                              end
                              local.get 13
                              i32.const 8
                              i32.add
                              local.set 4
                              local.get 6
                              local.get 1
                              local.get 0
                              local.get 2
                              local.get 5
                              local.get 11
                              local.get 14
                              i64.const 63
                              i64.shr_u
                              i32.wrap_i64
                              local.get 9
                              local.get 10
                              local.get 7
                              local.get 15
                              call $_ntoa_format_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__char*__unsigned_long__bool__unsigned_int__unsigned_int__unsigned_int__unsigned_int_
                              local.set 0
                              local.get 3
                              i32.const 1
                              i32.add
                              local.set 3
                              br 11 (;@2;)
                            end
                            block  ;; label = @13
                              local.get 8
                              i32.const 256
                              i32.and
                              i32.eqz
                              br_if 0 (;@13;)
                              local.get 8
                              local.get 8
                              i32.const -17
                              i32.and
                              local.get 4
                              i32.load
                              local.tee 19
                              select
                              local.set 20
                              block  ;; label = @14
                                block  ;; label = @15
                                  local.get 19
                                  br_if 0 (;@15;)
                                  i32.const 0
                                  local.set 12
                                  local.get 20
                                  i32.const 1024
                                  i32.and
                                  br_if 1 (;@14;)
                                end
                                local.get 19
                                local.get 19
                                i32.const 31
                                i32.shr_s
                                local.tee 8
                                i32.xor
                                local.get 8
                                i32.sub
                                local.set 8
                                local.get 20
                                i32.const 32
                                i32.and
                                i32.const 97
                                i32.xor
                                i32.const 246
                                i32.add
                                local.set 21
                                i32.const 0
                                local.set 11
                                loop  ;; label = @15
                                  local.get 5
                                  local.get 11
                                  i32.add
                                  i32.const 48
                                  local.get 21
                                  local.get 8
                                  local.get 8
                                  local.get 9
                                  i32.div_u
                                  local.tee 15
                                  local.get 9
                                  i32.mul
                                  i32.sub
                                  local.tee 12
                                  i32.const 10
                                  i32.lt_u
                                  select
                                  local.get 12
                                  i32.add
                                  i32.store8
                                  local.get 11
                                  i32.const 1
                                  i32.add
                                  local.set 12
                                  local.get 8
                                  local.get 9
                                  i32.lt_u
                                  br_if 1 (;@14;)
                                  local.get 11
                                  i32.const 31
                                  i32.lt_u
                                  local.set 13
                                  local.get 12
                                  local.set 11
                                  local.get 15
                                  local.set 8
                                  local.get 13
                                  br_if 0 (;@15;)
                                end
                              end
                              local.get 4
                              i32.const 4
                              i32.add
                              local.set 4
                              local.get 6
                              local.get 1
                              local.get 0
                              local.get 2
                              local.get 5
                              local.get 12
                              local.get 19
                              i32.const 31
                              i32.shr_u
                              local.get 9
                              local.get 10
                              local.get 7
                              local.get 20
                              call $_ntoa_format_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__char*__unsigned_long__bool__unsigned_int__unsigned_int__unsigned_int__unsigned_int_
                              local.set 0
                              local.get 3
                              i32.const 1
                              i32.add
                              local.set 3
                              br 11 (;@2;)
                            end
                            block  ;; label = @13
                              block  ;; label = @14
                                local.get 8
                                i32.const 64
                                i32.and
                                i32.eqz
                                br_if 0 (;@14;)
                                local.get 4
                                i32.load8_s
                                local.set 19
                                br 1 (;@13;)
                              end
                              local.get 4
                              i32.load
                              local.set 19
                              local.get 8
                              i32.const 128
                              i32.and
                              i32.eqz
                              br_if 0 (;@13;)
                              local.get 19
                              i32.const 16
                              i32.shl
                              i32.const 16
                              i32.shr_s
                              local.set 19
                            end
                            local.get 8
                            local.get 8
                            i32.const -17
                            i32.and
                            local.get 19
                            select
                            local.set 20
                            block  ;; label = @13
                              block  ;; label = @14
                                local.get 19
                                br_if 0 (;@14;)
                                i32.const 0
                                local.set 12
                                local.get 20
                                i32.const 1024
                                i32.and
                                br_if 1 (;@13;)
                              end
                              local.get 19
                              local.get 19
                              i32.const 31
                              i32.shr_s
                              local.tee 8
                              i32.xor
                              local.get 8
                              i32.sub
                              local.set 8
                              local.get 20
                              i32.const 32
                              i32.and
                              i32.const 97
                              i32.xor
                              i32.const 246
                              i32.add
                              local.set 21
                              i32.const 0
                              local.set 11
                              loop  ;; label = @14
                                local.get 5
                                local.get 11
                                i32.add
                                i32.const 48
                                local.get 21
                                local.get 8
                                local.get 8
                                local.get 9
                                i32.div_u
                                local.tee 15
                                local.get 9
                                i32.mul
                                i32.sub
                                local.tee 12
                                i32.const 10
                                i32.lt_u
                                select
                                local.get 12
                                i32.add
                                i32.store8
                                local.get 11
                                i32.const 1
                                i32.add
                                local.set 12
                                local.get 8
                                local.get 9
                                i32.lt_u
                                br_if 1 (;@13;)
                                local.get 11
                                i32.const 31
                                i32.lt_u
                                local.set 13
                                local.get 12
                                local.set 11
                                local.get 15
                                local.set 8
                                local.get 13
                                br_if 0 (;@14;)
                              end
                            end
                            local.get 4
                            i32.const 4
                            i32.add
                            local.set 4
                            local.get 6
                            local.get 1
                            local.get 0
                            local.get 2
                            local.get 5
                            local.get 12
                            local.get 19
                            i32.const 31
                            i32.shr_u
                            local.get 9
                            local.get 10
                            local.get 7
                            local.get 20
                            call $_ntoa_format_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__char*__unsigned_long__bool__unsigned_int__unsigned_int__unsigned_int__unsigned_int_
                            local.set 0
                            local.get 3
                            i32.const 1
                            i32.add
                            local.set 3
                            br 10 (;@2;)
                          end
                          block  ;; label = @12
                            local.get 8
                            i32.const 512
                            i32.and
                            i32.eqz
                            br_if 0 (;@12;)
                            local.get 8
                            local.get 8
                            i32.const -17
                            i32.and
                            local.get 4
                            i32.const 7
                            i32.add
                            i32.const -8
                            i32.and
                            local.tee 13
                            i64.load
                            local.tee 17
                            i64.const 0
                            i64.ne
                            local.tee 11
                            select
                            local.set 15
                            block  ;; label = @13
                              block  ;; label = @14
                                local.get 11
                                br_if 0 (;@14;)
                                i32.const 0
                                local.set 11
                                local.get 15
                                i32.const 1024
                                i32.and
                                br_if 1 (;@13;)
                              end
                              local.get 9
                              i64.extend_i32_u
                              local.set 16
                              local.get 15
                              i32.const 32
                              i32.and
                              i32.const 97
                              i32.xor
                              i32.const 246
                              i32.add
                              local.set 12
                              i32.const 0
                              local.set 8
                              loop  ;; label = @14
                                local.get 5
                                local.get 8
                                i32.add
                                i32.const 48
                                local.get 12
                                local.get 17
                                local.get 17
                                local.get 16
                                i64.div_u
                                local.tee 18
                                local.get 16
                                i64.mul
                                i64.sub
                                i32.wrap_i64
                                local.tee 11
                                i32.const 10
                                i32.lt_u
                                select
                                local.get 11
                                i32.add
                                i32.store8
                                local.get 8
                                i32.const 1
                                i32.add
                                local.set 11
                                local.get 17
                                local.get 16
                                i64.lt_u
                                br_if 1 (;@13;)
                                local.get 8
                                i32.const 31
                                i32.lt_u
                                local.set 4
                                local.get 11
                                local.set 8
                                local.get 18
                                local.set 17
                                local.get 4
                                br_if 0 (;@14;)
                              end
                            end
                            local.get 13
                            i32.const 8
                            i32.add
                            local.set 4
                            local.get 6
                            local.get 1
                            local.get 0
                            local.get 2
                            local.get 5
                            local.get 11
                            i32.const 0
                            local.get 9
                            local.get 10
                            local.get 7
                            local.get 15
                            call $_ntoa_format_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__char*__unsigned_long__bool__unsigned_int__unsigned_int__unsigned_int__unsigned_int_
                            local.set 0
                            local.get 3
                            i32.const 1
                            i32.add
                            local.set 3
                            br 10 (;@2;)
                          end
                          block  ;; label = @12
                            local.get 8
                            i32.const 256
                            i32.and
                            i32.eqz
                            br_if 0 (;@12;)
                            local.get 8
                            local.get 8
                            i32.const -17
                            i32.and
                            local.get 4
                            i32.load
                            local.tee 11
                            select
                            local.set 19
                            block  ;; label = @13
                              block  ;; label = @14
                                local.get 11
                                br_if 0 (;@14;)
                                i32.const 0
                                local.set 12
                                local.get 19
                                i32.const 1024
                                i32.and
                                br_if 1 (;@13;)
                              end
                              local.get 19
                              i32.const 32
                              i32.and
                              i32.const 97
                              i32.xor
                              i32.const 246
                              i32.add
                              local.set 21
                              i32.const 0
                              local.set 8
                              loop  ;; label = @14
                                local.get 5
                                local.get 8
                                i32.add
                                i32.const 48
                                local.get 21
                                local.get 11
                                local.get 11
                                local.get 9
                                i32.div_u
                                local.tee 15
                                local.get 9
                                i32.mul
                                i32.sub
                                local.tee 12
                                i32.const 10
                                i32.lt_u
                                select
                                local.get 12
                                i32.add
                                i32.store8
                                local.get 8
                                i32.const 1
                                i32.add
                                local.set 12
                                local.get 11
                                local.get 9
                                i32.lt_u
                                br_if 1 (;@13;)
                                local.get 8
                                i32.const 31
                                i32.lt_u
                                local.set 13
                                local.get 12
                                local.set 8
                                local.get 15
                                local.set 11
                                local.get 13
                                br_if 0 (;@14;)
                              end
                            end
                            local.get 4
                            i32.const 4
                            i32.add
                            local.set 4
                            local.get 6
                            local.get 1
                            local.get 0
                            local.get 2
                            local.get 5
                            local.get 12
                            i32.const 0
                            local.get 9
                            local.get 10
                            local.get 7
                            local.get 19
                            call $_ntoa_format_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__char*__unsigned_long__bool__unsigned_int__unsigned_int__unsigned_int__unsigned_int_
                            local.set 0
                            local.get 3
                            i32.const 1
                            i32.add
                            local.set 3
                            br 10 (;@2;)
                          end
                          block  ;; label = @12
                            block  ;; label = @13
                              local.get 8
                              i32.const 64
                              i32.and
                              i32.eqz
                              br_if 0 (;@13;)
                              local.get 4
                              i32.load8_u
                              local.set 11
                              br 1 (;@12;)
                            end
                            local.get 4
                            i32.load
                            local.tee 11
                            i32.const 65535
                            i32.and
                            local.get 11
                            local.get 8
                            i32.const 128
                            i32.and
                            select
                            local.set 11
                          end
                          local.get 8
                          local.get 8
                          i32.const -17
                          i32.and
                          local.get 11
                          select
                          local.set 19
                          block  ;; label = @12
                            block  ;; label = @13
                              local.get 11
                              br_if 0 (;@13;)
                              i32.const 0
                              local.set 12
                              local.get 19
                              i32.const 1024
                              i32.and
                              br_if 1 (;@12;)
                            end
                            local.get 19
                            i32.const 32
                            i32.and
                            i32.const 97
                            i32.xor
                            i32.const 246
                            i32.add
                            local.set 21
                            i32.const 0
                            local.set 8
                            loop  ;; label = @13
                              local.get 5
                              local.get 8
                              i32.add
                              i32.const 48
                              local.get 21
                              local.get 11
                              local.get 11
                              local.get 9
                              i32.div_u
                              local.tee 15
                              local.get 9
                              i32.mul
                              i32.sub
                              local.tee 12
                              i32.const 10
                              i32.lt_u
                              select
                              local.get 12
                              i32.add
                              i32.store8
                              local.get 8
                              i32.const 1
                              i32.add
                              local.set 12
                              local.get 11
                              local.get 9
                              i32.lt_u
                              br_if 1 (;@12;)
                              local.get 8
                              i32.const 31
                              i32.lt_u
                              local.set 13
                              local.get 12
                              local.set 8
                              local.get 15
                              local.set 11
                              local.get 13
                              br_if 0 (;@13;)
                            end
                          end
                          local.get 4
                          i32.const 4
                          i32.add
                          local.set 4
                          local.get 6
                          local.get 1
                          local.get 0
                          local.get 2
                          local.get 5
                          local.get 12
                          i32.const 0
                          local.get 9
                          local.get 10
                          local.get 7
                          local.get 19
                          call $_ntoa_format_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__char*__unsigned_long__bool__unsigned_int__unsigned_int__unsigned_int__unsigned_int_
                          local.set 0
                          local.get 3
                          i32.const 1
                          i32.add
                          local.set 3
                          br 9 (;@2;)
                        end
                        local.get 4
                        i32.const 7
                        i32.add
                        i32.const -8
                        i32.and
                        local.tee 9
                        i32.const 8
                        i32.add
                        local.set 4
                        local.get 6
                        local.get 1
                        local.get 0
                        local.get 2
                        local.get 9
                        f64.load
                        local.get 10
                        local.get 7
                        local.get 8
                        i32.const 32
                        i32.or
                        local.get 8
                        local.get 11
                        i32.const 70
                        i32.eq
                        select
                        call $_ftoa_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__double__unsigned_int__unsigned_int__unsigned_int_
                        local.set 0
                        local.get 3
                        i32.const 1
                        i32.add
                        local.set 3
                        br 8 (;@2;)
                      end
                      block  ;; label = @10
                        local.get 12
                        i32.const 32
                        i32.or
                        i32.const 103
                        i32.ne
                        br_if 0 (;@10;)
                        local.get 8
                        i32.const 2048
                        i32.or
                        local.set 8
                      end
                      block  ;; label = @10
                        block  ;; label = @11
                          local.get 12
                          i32.const -69
                          i32.add
                          br_table 0 (;@11;) 1 (;@10;) 0 (;@11;) 1 (;@10;)
                        end
                        local.get 8
                        i32.const 32
                        i32.or
                        local.set 8
                      end
                      local.get 4
                      i32.const 7
                      i32.add
                      i32.const -8
                      i32.and
                      local.tee 9
                      i32.const 8
                      i32.add
                      local.set 4
                      local.get 6
                      local.get 1
                      local.get 0
                      local.get 2
                      local.get 9
                      f64.load
                      local.get 10
                      local.get 7
                      local.get 8
                      call $_etoa_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__double__unsigned_int__unsigned_int__unsigned_int_
                      local.set 0
                      local.get 3
                      i32.const 1
                      i32.add
                      local.set 3
                      br 7 (;@2;)
                    end
                    i32.const 1
                    local.set 10
                    block  ;; label = @9
                      local.get 8
                      i32.const 2
                      i32.and
                      local.tee 11
                      br_if 0 (;@9;)
                      i32.const 2
                      local.set 10
                      local.get 7
                      i32.const 2
                      i32.lt_u
                      br_if 0 (;@9;)
                      local.get 7
                      i32.const -1
                      i32.add
                      local.set 9
                      local.get 7
                      i32.const 1
                      i32.add
                      local.set 10
                      i32.const 0
                      local.set 8
                      loop  ;; label = @10
                        i32.const 32
                        local.get 1
                        local.get 0
                        local.get 8
                        i32.add
                        local.get 2
                        local.get 6
                        call_indirect (type 0)
                        local.get 9
                        local.get 8
                        i32.const 1
                        i32.add
                        local.tee 8
                        i32.ne
                        br_if 0 (;@10;)
                      end
                      local.get 0
                      local.get 8
                      i32.add
                      local.set 0
                    end
                    local.get 4
                    i32.load8_s
                    local.get 1
                    local.get 0
                    local.get 2
                    local.get 6
                    call_indirect (type 0)
                    local.get 0
                    i32.const 1
                    i32.add
                    local.set 0
                    local.get 4
                    i32.const 4
                    i32.add
                    local.set 4
                    local.get 11
                    i32.eqz
                    br_if 5 (;@3;)
                    local.get 10
                    local.get 7
                    i32.ge_u
                    br_if 5 (;@3;)
                    local.get 7
                    local.get 10
                    i32.sub
                    local.set 7
                    loop  ;; label = @9
                      i32.const 32
                      local.get 1
                      local.get 0
                      local.get 2
                      local.get 6
                      call_indirect (type 0)
                      local.get 0
                      i32.const 1
                      i32.add
                      local.set 0
                      local.get 7
                      i32.const -1
                      i32.add
                      local.tee 7
                      br_if 0 (;@9;)
                      br 6 (;@3;)
                    end
                  end
                  local.get 4
                  i32.load
                  local.tee 13
                  local.set 12
                  block  ;; label = @8
                    local.get 13
                    i32.load8_u
                    i32.eqz
                    br_if 0 (;@8;)
                    local.get 10
                    i32.const -1
                    local.get 10
                    select
                    i32.const -1
                    i32.add
                    local.set 15
                    local.get 13
                    local.set 9
                    loop  ;; label = @9
                      local.get 15
                      local.set 11
                      local.get 9
                      i32.const 1
                      i32.add
                      local.set 12
                      local.get 9
                      i32.load8_u offset=1
                      i32.eqz
                      br_if 1 (;@8;)
                      local.get 11
                      i32.const -1
                      i32.add
                      local.set 15
                      local.get 12
                      local.set 9
                      local.get 11
                      br_if 0 (;@9;)
                    end
                  end
                  local.get 12
                  local.get 13
                  i32.sub
                  local.tee 9
                  local.get 10
                  local.get 9
                  local.get 10
                  i32.lt_u
                  select
                  local.get 9
                  local.get 8
                  i32.const 1024
                  i32.and
                  local.tee 11
                  select
                  local.set 12
                  block  ;; label = @8
                    local.get 8
                    i32.const 2
                    i32.and
                    local.tee 15
                    br_if 0 (;@8;)
                    block  ;; label = @9
                      local.get 12
                      local.get 7
                      i32.lt_u
                      br_if 0 (;@9;)
                      local.get 12
                      i32.const 1
                      i32.add
                      local.set 12
                      br 1 (;@8;)
                    end
                    local.get 7
                    local.get 12
                    i32.sub
                    local.set 9
                    local.get 7
                    i32.const 1
                    i32.add
                    local.set 12
                    i32.const 0
                    local.set 8
                    loop  ;; label = @9
                      i32.const 32
                      local.get 1
                      local.get 0
                      local.get 8
                      i32.add
                      local.get 2
                      local.get 6
                      call_indirect (type 0)
                      local.get 9
                      local.get 8
                      i32.const 1
                      i32.add
                      local.tee 8
                      i32.ne
                      br_if 0 (;@9;)
                    end
                    local.get 0
                    local.get 8
                    i32.add
                    local.set 0
                  end
                  block  ;; label = @8
                    local.get 13
                    i32.load8_u
                    local.tee 8
                    i32.eqz
                    br_if 0 (;@8;)
                    local.get 13
                    i32.const 1
                    i32.add
                    local.set 9
                    loop  ;; label = @9
                      block  ;; label = @10
                        local.get 11
                        i32.eqz
                        br_if 0 (;@10;)
                        local.get 10
                        i32.eqz
                        br_if 2 (;@8;)
                        local.get 10
                        i32.const -1
                        i32.add
                        local.set 10
                      end
                      local.get 8
                      i32.const 24
                      i32.shl
                      i32.const 24
                      i32.shr_s
                      local.get 1
                      local.get 0
                      local.get 2
                      local.get 6
                      call_indirect (type 0)
                      local.get 0
                      i32.const 1
                      i32.add
                      local.set 0
                      local.get 9
                      i32.load8_u
                      local.set 8
                      local.get 9
                      i32.const 1
                      i32.add
                      local.set 9
                      local.get 8
                      br_if 0 (;@9;)
                    end
                  end
                  local.get 4
                  i32.const 4
                  i32.add
                  local.set 4
                  local.get 15
                  i32.eqz
                  br_if 4 (;@3;)
                  local.get 12
                  local.get 7
                  i32.ge_u
                  br_if 4 (;@3;)
                  local.get 7
                  local.get 12
                  i32.sub
                  local.set 7
                  loop  ;; label = @8
                    i32.const 32
                    local.get 1
                    local.get 0
                    local.get 2
                    local.get 6
                    call_indirect (type 0)
                    local.get 0
                    i32.const 1
                    i32.add
                    local.set 0
                    local.get 7
                    i32.const -1
                    i32.add
                    local.tee 7
                    br_if 0 (;@8;)
                    br 5 (;@3;)
                  end
                end
                local.get 8
                i32.const 33
                i32.or
                local.tee 7
                local.get 7
                i32.const -17
                i32.and
                local.get 4
                i32.load
                local.tee 7
                select
                local.set 12
                block  ;; label = @7
                  block  ;; label = @8
                    local.get 7
                    br_if 0 (;@8;)
                    i32.const 0
                    local.set 9
                    local.get 12
                    i32.const 1024
                    i32.and
                    br_if 1 (;@7;)
                  end
                  i32.const 0
                  local.set 8
                  loop  ;; label = @8
                    local.get 5
                    local.get 8
                    i32.add
                    i32.const 48
                    i32.const 55
                    local.get 7
                    i32.const 14
                    i32.and
                    i32.const 10
                    i32.lt_u
                    select
                    local.get 7
                    i32.const 15
                    i32.and
                    i32.add
                    i32.store8
                    local.get 8
                    i32.const 1
                    i32.add
                    local.set 9
                    local.get 7
                    i32.const 16
                    i32.lt_u
                    br_if 1 (;@7;)
                    local.get 7
                    i32.const 4
                    i32.shr_u
                    local.set 7
                    local.get 8
                    i32.const 31
                    i32.lt_u
                    local.set 11
                    local.get 9
                    local.set 8
                    local.get 11
                    br_if 0 (;@8;)
                  end
                end
                local.get 4
                i32.const 4
                i32.add
                local.set 4
                local.get 6
                local.get 1
                local.get 0
                local.get 2
                local.get 5
                local.get 9
                i32.const 0
                i32.const 16
                local.get 10
                i32.const 8
                local.get 12
                call $_ntoa_format_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__char*__unsigned_long__bool__unsigned_int__unsigned_int__unsigned_int__unsigned_int_
                local.set 0
                local.get 3
                i32.const 1
                i32.add
                local.set 3
                br 4 (;@2;)
              end
              i32.const 37
              local.get 1
              local.get 0
              local.get 2
              local.get 6
              call_indirect (type 0)
              br 1 (;@4;)
            end
            local.get 11
            local.get 1
            local.get 0
            local.get 2
            local.get 6
            call_indirect (type 0)
          end
          local.get 0
          i32.const 1
          i32.add
          local.set 0
        end
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        br 0 (;@2;)
      end
    end
    i32.const 0
    local.get 1
    local.get 0
    local.get 2
    i32.const -1
    i32.add
    local.get 0
    local.get 2
    i32.lt_u
    select
    local.get 2
    local.get 6
    call_indirect (type 0)
    local.get 5
    i32.const 32
    i32.add
    global.set $__stack_pointer
    local.get 0)
  (func $_out_null_char__void*__unsigned_long__unsigned_long_ (type 0) (param i32 i32 i32 i32))
  (func $_ntoa_format_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__char*__unsigned_long__bool__unsigned_int__unsigned_int__unsigned_int__unsigned_int_ (type 6) (param i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32) (result i32)
    (local i32 i32 i32)
    block  ;; label = @1
      local.get 10
      i32.const 2
      i32.and
      local.tee 11
      br_if 0 (;@1;)
      block  ;; label = @2
        block  ;; label = @3
          local.get 9
          br_if 0 (;@3;)
          i32.const 0
          local.set 9
          br 1 (;@2;)
        end
        local.get 10
        i32.const 1
        i32.and
        i32.eqz
        br_if 0 (;@2;)
        local.get 9
        local.get 10
        i32.const 12
        i32.and
        i32.const 0
        i32.ne
        local.get 6
        i32.or
        i32.sub
        local.set 9
      end
      block  ;; label = @2
        local.get 5
        local.get 8
        i32.ge_u
        br_if 0 (;@2;)
        local.get 5
        i32.const 31
        i32.gt_u
        br_if 0 (;@2;)
        local.get 4
        local.get 5
        i32.add
        i32.const 48
        local.get 5
        i32.const -1
        i32.xor
        local.get 8
        i32.add
        local.tee 12
        i32.const 31
        local.get 5
        i32.sub
        local.tee 13
        local.get 12
        local.get 13
        i32.lt_u
        select
        i32.const 1
        i32.add
        local.tee 12
        call $memset
        drop
        local.get 12
        local.get 5
        i32.add
        local.set 5
      end
      local.get 10
      i32.const 1
      i32.and
      i32.eqz
      br_if 0 (;@1;)
      local.get 5
      local.get 9
      i32.ge_u
      br_if 0 (;@1;)
      local.get 5
      i32.const 31
      i32.gt_u
      br_if 0 (;@1;)
      loop  ;; label = @2
        local.get 4
        local.get 5
        i32.add
        i32.const 48
        i32.store8
        local.get 5
        i32.const 1
        i32.add
        local.tee 5
        local.get 9
        i32.ge_u
        br_if 1 (;@1;)
        local.get 5
        i32.const 32
        i32.lt_u
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      local.get 10
      i32.const 16
      i32.and
      i32.eqz
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 10
        i32.const 1024
        i32.and
        br_if 0 (;@2;)
        local.get 5
        i32.eqz
        br_if 0 (;@2;)
        block  ;; label = @3
          local.get 5
          local.get 8
          i32.eq
          br_if 0 (;@3;)
          local.get 5
          local.get 9
          i32.ne
          br_if 1 (;@2;)
        end
        local.get 5
        i32.const -2
        i32.add
        local.get 5
        i32.const -1
        i32.add
        local.tee 8
        local.get 8
        select
        local.get 8
        local.get 7
        i32.const 16
        i32.eq
        select
        local.set 5
      end
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            local.get 7
            i32.const 16
            i32.ne
            br_if 0 (;@4;)
            block  ;; label = @5
              local.get 10
              i32.const 32
              i32.and
              local.tee 12
              br_if 0 (;@5;)
              i32.const 120
              local.set 8
              local.get 5
              i32.const 32
              i32.lt_u
              br_if 2 (;@3;)
            end
            local.get 12
            i32.eqz
            br_if 0 (;@4;)
            i32.const 88
            local.set 8
            local.get 5
            i32.const 32
            i32.lt_u
            br_if 1 (;@3;)
          end
          local.get 7
          i32.const 2
          i32.ne
          br_if 1 (;@2;)
          i32.const 98
          local.set 8
          local.get 5
          i32.const 31
          i32.gt_u
          br_if 1 (;@2;)
        end
        local.get 4
        local.get 5
        i32.add
        local.get 8
        i32.store8
        local.get 5
        i32.const 1
        i32.add
        local.set 5
      end
      local.get 5
      i32.const 31
      i32.gt_u
      br_if 0 (;@1;)
      local.get 4
      local.get 5
      i32.add
      i32.const 48
      i32.store8
      local.get 5
      i32.const 1
      i32.add
      local.set 5
    end
    block  ;; label = @1
      local.get 5
      i32.const 31
      i32.gt_u
      br_if 0 (;@1;)
      i32.const 45
      local.set 8
      block  ;; label = @2
        local.get 6
        br_if 0 (;@2;)
        i32.const 43
        local.set 8
        local.get 10
        i32.const 4
        i32.and
        br_if 0 (;@2;)
        i32.const 32
        local.set 8
        local.get 10
        i32.const 8
        i32.and
        i32.eqz
        br_if 1 (;@1;)
      end
      local.get 4
      local.get 5
      i32.add
      local.get 8
      i32.store8
      local.get 5
      i32.const 1
      i32.add
      local.set 5
    end
    local.get 2
    local.set 8
    block  ;; label = @1
      local.get 10
      i32.const 3
      i32.and
      br_if 0 (;@1;)
      local.get 2
      local.set 8
      local.get 5
      local.get 9
      i32.ge_u
      br_if 0 (;@1;)
      local.get 9
      local.get 5
      i32.sub
      local.set 10
      local.get 2
      local.set 8
      loop  ;; label = @2
        i32.const 32
        local.get 1
        local.get 8
        local.get 3
        local.get 0
        call_indirect (type 0)
        local.get 8
        i32.const 1
        i32.add
        local.set 8
        local.get 10
        i32.const -1
        i32.add
        local.tee 10
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      local.get 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 4
      i32.const -1
      i32.add
      local.set 10
      loop  ;; label = @2
        local.get 10
        local.get 5
        i32.add
        i32.load8_s
        local.get 1
        local.get 8
        local.get 3
        local.get 0
        call_indirect (type 0)
        local.get 8
        i32.const 1
        i32.add
        local.set 8
        local.get 5
        i32.const -1
        i32.add
        local.tee 5
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      local.get 11
      i32.eqz
      br_if 0 (;@1;)
      local.get 8
      local.get 2
      i32.sub
      local.get 9
      i32.ge_u
      br_if 0 (;@1;)
      i32.const 0
      local.get 2
      i32.sub
      local.set 5
      loop  ;; label = @2
        i32.const 32
        local.get 1
        local.get 8
        local.get 3
        local.get 0
        call_indirect (type 0)
        local.get 5
        local.get 8
        i32.const 1
        i32.add
        local.tee 8
        i32.add
        local.get 9
        i32.lt_u
        br_if 0 (;@2;)
      end
    end
    local.get 8)
  (func $_ftoa_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__double__unsigned_int__unsigned_int__unsigned_int_ (type 7) (param i32 i32 i32 i32 f64 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32 f64 f64 f64 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 8
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 4
        local.get 4
        f64.eq
        br_if 0 (;@2;)
        local.get 0
        local.get 1
        local.get 2
        local.get 3
        i32.const 1303
        i32.const 3
        local.get 6
        local.get 7
        call $_out_rev_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__char_const*__unsigned_long__unsigned_int__unsigned_int_
        local.set 5
        br 1 (;@1;)
      end
      block  ;; label = @2
        local.get 4
        f64.const -0x1.fffffffffffffp+1023 (;=-1.79769e+308;)
        f64.lt
        i32.eqz
        br_if 0 (;@2;)
        i32.const 3
        local.set 9
        local.get 2
        local.set 5
        block  ;; label = @3
          local.get 6
          i32.const 5
          i32.lt_u
          br_if 0 (;@3;)
          local.get 2
          local.set 5
          local.get 7
          i32.const 3
          i32.and
          br_if 0 (;@3;)
          local.get 6
          i32.const -4
          i32.add
          local.set 10
          local.get 2
          local.set 5
          loop  ;; label = @4
            i32.const 32
            local.get 1
            local.get 5
            local.get 3
            local.get 0
            call_indirect (type 0)
            local.get 5
            i32.const 1
            i32.add
            local.set 5
            local.get 10
            i32.const -1
            i32.add
            local.tee 10
            br_if 0 (;@4;)
          end
        end
        local.get 7
        i32.const 2
        i32.and
        local.set 10
        loop  ;; label = @3
          local.get 9
          i32.const 2999
          i32.add
          i32.load8_s
          local.get 1
          local.get 5
          local.get 3
          local.get 0
          call_indirect (type 0)
          local.get 5
          i32.const 1
          i32.add
          local.set 5
          local.get 9
          i32.const -1
          i32.add
          local.tee 9
          i32.const -1
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 10
        i32.eqz
        br_if 1 (;@1;)
        local.get 5
        local.get 2
        i32.sub
        local.get 6
        i32.ge_u
        br_if 1 (;@1;)
        i32.const 0
        local.get 2
        i32.sub
        local.set 9
        loop  ;; label = @3
          i32.const 32
          local.get 1
          local.get 5
          local.get 3
          local.get 0
          call_indirect (type 0)
          local.get 9
          local.get 5
          i32.const 1
          i32.add
          local.tee 5
          i32.add
          local.get 6
          i32.lt_u
          br_if 0 (;@3;)
          br 2 (;@1;)
        end
      end
      block  ;; label = @2
        local.get 4
        f64.const 0x1.fffffffffffffp+1023 (;=1.79769e+308;)
        f64.gt
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4
        i32.const 3
        local.get 7
        i32.const 4
        i32.and
        local.tee 5
        select
        local.set 9
        i32.const 3063
        i32.const 1344
        local.get 5
        select
        local.set 11
        local.get 2
        local.set 5
        block  ;; label = @3
          local.get 7
          i32.const 3
          i32.and
          br_if 0 (;@3;)
          local.get 2
          local.set 5
          local.get 9
          local.get 6
          i32.ge_u
          br_if 0 (;@3;)
          local.get 6
          local.get 9
          i32.sub
          local.set 10
          local.get 2
          local.set 5
          loop  ;; label = @4
            i32.const 32
            local.get 1
            local.get 5
            local.get 3
            local.get 0
            call_indirect (type 0)
            local.get 5
            i32.const 1
            i32.add
            local.set 5
            local.get 10
            i32.const -1
            i32.add
            local.tee 10
            br_if 0 (;@4;)
          end
        end
        local.get 7
        i32.const 2
        i32.and
        local.set 12
        local.get 11
        i32.const -1
        i32.add
        local.set 10
        loop  ;; label = @3
          local.get 10
          local.get 9
          i32.add
          i32.load8_s
          local.get 1
          local.get 5
          local.get 3
          local.get 0
          call_indirect (type 0)
          local.get 5
          i32.const 1
          i32.add
          local.set 5
          local.get 9
          i32.const -1
          i32.add
          local.tee 9
          br_if 0 (;@3;)
        end
        local.get 12
        i32.eqz
        br_if 1 (;@1;)
        local.get 5
        local.get 2
        i32.sub
        local.get 6
        i32.ge_u
        br_if 1 (;@1;)
        i32.const 0
        local.get 2
        i32.sub
        local.set 9
        loop  ;; label = @3
          i32.const 32
          local.get 1
          local.get 5
          local.get 3
          local.get 0
          call_indirect (type 0)
          local.get 9
          local.get 5
          i32.const 1
          i32.add
          local.tee 5
          i32.add
          local.get 6
          i32.lt_u
          br_if 0 (;@3;)
          br 2 (;@1;)
        end
      end
      block  ;; label = @2
        block  ;; label = @3
          local.get 4
          f64.const 0x1.dcd65p+29 (;=1e+09;)
          f64.gt
          br_if 0 (;@3;)
          local.get 4
          f64.const -0x1.dcd65p+29 (;=-1e+09;)
          f64.lt
          i32.eqz
          br_if 1 (;@2;)
        end
        local.get 0
        local.get 1
        local.get 2
        local.get 3
        local.get 4
        local.get 5
        local.get 6
        local.get 7
        call $_etoa_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__double__unsigned_int__unsigned_int__unsigned_int_
        local.set 5
        br 1 (;@1;)
      end
      f64.const 0x0p+0 (;=0;)
      local.get 4
      f64.sub
      local.get 4
      local.get 4
      f64.const 0x0p+0 (;=0;)
      f64.lt
      select
      local.set 13
      block  ;; label = @2
        block  ;; label = @3
          local.get 5
          i32.const 6
          local.get 7
          i32.const 1024
          i32.and
          select
          local.tee 12
          i32.const 10
          i32.ge_u
          br_if 0 (;@3;)
          i32.const 1
          local.set 9
          i32.const 0
          local.set 5
          br 1 (;@2;)
        end
        local.get 8
        i32.const 48
        local.get 12
        i32.const -10
        i32.add
        local.tee 5
        i32.const 31
        local.get 5
        i32.const 31
        i32.lt_u
        select
        i32.const 1
        i32.add
        local.tee 5
        call $memset
        drop
        i32.const 0
        local.set 9
        loop  ;; label = @3
          local.get 5
          local.get 9
          i32.const 1
          i32.add
          local.tee 9
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 12
        local.get 9
        i32.sub
        local.set 12
        local.get 9
        i32.const -1
        i32.add
        i32.const 31
        i32.lt_u
        local.set 9
      end
      block  ;; label = @2
        block  ;; label = @3
          local.get 13
          f64.abs
          f64.const 0x1p+31 (;=2.14748e+09;)
          f64.lt
          i32.eqz
          br_if 0 (;@3;)
          local.get 13
          i32.trunc_f64_s
          local.set 10
          br 1 (;@2;)
        end
        i32.const -2147483648
        local.set 10
      end
      block  ;; label = @2
        block  ;; label = @3
          local.get 13
          local.get 10
          f64.convert_i32_s
          f64.sub
          local.get 12
          i32.const 3
          i32.shl
          i32.const 4304
          i32.add
          f64.load
          local.tee 14
          f64.mul
          local.tee 15
          f64.const 0x1p+32 (;=4.29497e+09;)
          f64.lt
          local.get 15
          f64.const 0x0p+0 (;=0;)
          f64.ge
          i32.and
          i32.eqz
          br_if 0 (;@3;)
          local.get 15
          i32.trunc_f64_u
          local.set 11
          br 1 (;@2;)
        end
        i32.const 0
        local.set 11
      end
      block  ;; label = @2
        block  ;; label = @3
          local.get 15
          local.get 11
          f64.convert_i32_u
          f64.sub
          local.tee 15
          f64.const 0x1p-1 (;=0.5;)
          f64.gt
          i32.eqz
          br_if 0 (;@3;)
          local.get 14
          local.get 11
          i32.const 1
          i32.add
          local.tee 11
          f64.convert_i32_u
          f64.le
          i32.eqz
          br_if 1 (;@2;)
          local.get 10
          i32.const 1
          i32.add
          local.set 10
          i32.const 0
          local.set 11
          br 1 (;@2;)
        end
        local.get 15
        f64.const 0x1p-1 (;=0.5;)
        f64.lt
        br_if 0 (;@2;)
        local.get 11
        i32.eqz
        local.get 11
        i32.or
        i32.const 1
        i32.and
        local.get 11
        i32.add
        local.set 11
      end
      block  ;; label = @2
        block  ;; label = @3
          local.get 12
          br_if 0 (;@3;)
          local.get 10
          local.get 13
          local.get 10
          f64.convert_i32_s
          f64.sub
          f64.const 0x1p-1 (;=0.5;)
          f64.lt
          i32.const 1
          i32.xor
          local.get 10
          i32.and
          i32.add
          local.set 10
          br 1 (;@2;)
        end
        block  ;; label = @3
          block  ;; label = @4
            local.get 9
            br_if 0 (;@4;)
            local.get 5
            local.set 9
            br 1 (;@3;)
          end
          loop  ;; label = @4
            local.get 8
            local.get 5
            i32.add
            local.get 11
            local.get 11
            i32.const 10
            i32.div_u
            local.tee 16
            i32.const 10
            i32.mul
            i32.sub
            i32.const 48
            i32.or
            i32.store8
            local.get 5
            i32.const 1
            i32.add
            local.set 9
            local.get 12
            i32.const -1
            i32.add
            local.set 12
            local.get 11
            i32.const 10
            i32.lt_u
            br_if 1 (;@3;)
            local.get 5
            i32.const 31
            i32.lt_u
            local.set 17
            local.get 9
            local.set 5
            local.get 16
            local.set 11
            local.get 17
            br_if 0 (;@4;)
          end
        end
        local.get 9
        i32.const 32
        i32.lt_u
        local.set 16
        block  ;; label = @3
          local.get 9
          i32.const 31
          i32.gt_u
          br_if 0 (;@3;)
          local.get 12
          i32.eqz
          br_if 0 (;@3;)
          local.get 12
          i32.const -1
          i32.add
          local.set 11
          loop  ;; label = @4
            local.get 11
            local.set 5
            local.get 8
            local.get 9
            i32.add
            i32.const 48
            i32.store8
            local.get 9
            i32.const 1
            i32.add
            local.tee 9
            i32.const 32
            i32.lt_u
            local.set 16
            local.get 9
            i32.const 31
            i32.gt_u
            br_if 1 (;@3;)
            local.get 5
            i32.const -1
            i32.add
            local.set 11
            local.get 5
            br_if 0 (;@4;)
          end
        end
        block  ;; label = @3
          local.get 16
          br_if 0 (;@3;)
          local.get 9
          local.set 5
          br 1 (;@2;)
        end
        local.get 8
        local.get 9
        i32.add
        i32.const 46
        i32.store8
        local.get 9
        i32.const 1
        i32.add
        local.set 5
      end
      block  ;; label = @2
        block  ;; label = @3
          local.get 5
          i32.const 31
          i32.le_u
          br_if 0 (;@3;)
          local.get 5
          local.set 9
          br 1 (;@2;)
        end
        loop  ;; label = @3
          local.get 8
          local.get 5
          i32.add
          local.get 10
          local.get 10
          i32.const 10
          i32.div_s
          local.tee 11
          i32.const 10
          i32.mul
          i32.sub
          i32.const 48
          i32.add
          i32.store8
          local.get 5
          i32.const 1
          i32.add
          local.set 9
          local.get 10
          i32.const -10
          i32.add
          i32.const -20
          i32.gt_u
          br_if 1 (;@2;)
          local.get 5
          i32.const 31
          i32.lt_u
          local.set 12
          local.get 9
          local.set 5
          local.get 11
          local.set 10
          local.get 12
          br_if 0 (;@3;)
        end
      end
      block  ;; label = @2
        local.get 7
        i32.const 3
        i32.and
        i32.const 1
        i32.ne
        br_if 0 (;@2;)
        block  ;; label = @3
          block  ;; label = @4
            local.get 6
            br_if 0 (;@4;)
            i32.const 0
            local.set 6
            br 1 (;@3;)
          end
          local.get 6
          local.get 4
          f64.const 0x0p+0 (;=0;)
          f64.lt
          local.get 7
          i32.const 12
          i32.and
          i32.const 0
          i32.ne
          i32.or
          i32.sub
          local.set 6
        end
        local.get 9
        local.get 6
        i32.ge_u
        br_if 0 (;@2;)
        local.get 9
        i32.const 31
        i32.gt_u
        br_if 0 (;@2;)
        local.get 8
        local.get 9
        i32.add
        i32.const 48
        local.get 6
        local.get 9
        i32.const -1
        i32.xor
        i32.add
        local.tee 5
        i32.const 31
        local.get 9
        i32.sub
        local.tee 10
        local.get 5
        local.get 10
        i32.lt_u
        select
        i32.const 1
        i32.add
        local.tee 5
        call $memset
        drop
        local.get 5
        local.get 9
        i32.add
        local.set 9
      end
      block  ;; label = @2
        local.get 9
        i32.const 31
        i32.gt_u
        br_if 0 (;@2;)
        i32.const 45
        local.set 5
        block  ;; label = @3
          local.get 4
          f64.const 0x0p+0 (;=0;)
          f64.lt
          br_if 0 (;@3;)
          i32.const 43
          local.set 5
          local.get 7
          i32.const 4
          i32.and
          br_if 0 (;@3;)
          i32.const 32
          local.set 5
          local.get 7
          i32.const 8
          i32.and
          i32.eqz
          br_if 1 (;@2;)
        end
        local.get 8
        local.get 9
        i32.add
        local.get 5
        i32.store8
        local.get 9
        i32.const 1
        i32.add
        local.set 9
      end
      local.get 2
      local.set 5
      block  ;; label = @2
        local.get 7
        i32.const 3
        i32.and
        br_if 0 (;@2;)
        local.get 2
        local.set 5
        local.get 9
        local.get 6
        i32.ge_u
        br_if 0 (;@2;)
        local.get 6
        local.get 9
        i32.sub
        local.set 10
        local.get 2
        local.set 5
        loop  ;; label = @3
          i32.const 32
          local.get 1
          local.get 5
          local.get 3
          local.get 0
          call_indirect (type 0)
          local.get 5
          i32.const 1
          i32.add
          local.set 5
          local.get 10
          i32.const -1
          i32.add
          local.tee 10
          br_if 0 (;@3;)
        end
      end
      local.get 7
      i32.const 2
      i32.and
      local.set 11
      block  ;; label = @2
        local.get 9
        i32.eqz
        br_if 0 (;@2;)
        local.get 8
        i32.const -1
        i32.add
        local.set 10
        loop  ;; label = @3
          local.get 10
          local.get 9
          i32.add
          i32.load8_s
          local.get 1
          local.get 5
          local.get 3
          local.get 0
          call_indirect (type 0)
          local.get 5
          i32.const 1
          i32.add
          local.set 5
          local.get 9
          i32.const -1
          i32.add
          local.tee 9
          br_if 0 (;@3;)
        end
      end
      local.get 11
      i32.eqz
      br_if 0 (;@1;)
      local.get 5
      local.get 2
      i32.sub
      local.get 6
      i32.ge_u
      br_if 0 (;@1;)
      i32.const 0
      local.get 2
      i32.sub
      local.set 9
      loop  ;; label = @2
        i32.const 32
        local.get 1
        local.get 5
        local.get 3
        local.get 0
        call_indirect (type 0)
        local.get 9
        local.get 5
        i32.const 1
        i32.add
        local.tee 5
        i32.add
        local.get 6
        i32.lt_u
        br_if 0 (;@2;)
      end
    end
    local.get 8
    i32.const 32
    i32.add
    global.set $__stack_pointer
    local.get 5)
  (func $_etoa_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__double__unsigned_int__unsigned_int__unsigned_int_ (type 7) (param i32 i32 i32 i32 f64 i32 i32 i32) (result i32)
    (local i32 i32 f64 i64 f64 i32 f64 i32 i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 8
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        block  ;; label = @3
          local.get 4
          f64.const -0x1.fffffffffffffp+1023 (;=-1.79769e+308;)
          f64.lt
          br_if 0 (;@3;)
          local.get 4
          local.get 4
          f64.ne
          br_if 0 (;@3;)
          local.get 4
          f64.const 0x1.fffffffffffffp+1023 (;=1.79769e+308;)
          f64.gt
          i32.eqz
          br_if 1 (;@2;)
        end
        local.get 0
        local.get 1
        local.get 2
        local.get 3
        local.get 4
        local.get 5
        local.get 6
        local.get 7
        call $_ftoa_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__double__unsigned_int__unsigned_int__unsigned_int_
        local.set 9
        br 1 (;@1;)
      end
      block  ;; label = @2
        block  ;; label = @3
          local.get 4
          f64.neg
          local.get 4
          local.get 4
          f64.const 0x0p+0 (;=0;)
          f64.lt
          select
          local.tee 10
          i64.reinterpret_f64
          local.tee 11
          i64.const 4503599627370495
          i64.and
          i64.const 4607182418800017408
          i64.or
          f64.reinterpret_i64
          f64.const -0x1.8p+0 (;=-1.5;)
          f64.add
          f64.const 0x1.287a7636f4361p-2 (;=0.28953;)
          f64.mul
          local.get 11
          i64.const 52
          i64.shr_u
          i32.wrap_i64
          i32.const 2047
          i32.and
          i32.const -1023
          i32.add
          f64.convert_i32_s
          f64.const 0x1.34413509f79fbp-2 (;=0.30103;)
          f64.mul
          f64.const 0x1.68a288b60c8b3p-3 (;=0.176091;)
          f64.add
          f64.add
          local.tee 12
          f64.abs
          f64.const 0x1p+31 (;=2.14748e+09;)
          f64.lt
          i32.eqz
          br_if 0 (;@3;)
          local.get 12
          i32.trunc_f64_s
          local.set 13
          br 1 (;@2;)
        end
        i32.const -2147483648
        local.set 13
      end
      local.get 13
      f64.convert_i32_s
      local.tee 12
      f64.const 0x1.26bb1bbb55516p+1 (;=2.30259;)
      f64.mul
      local.set 14
      block  ;; label = @2
        block  ;; label = @3
          local.get 12
          f64.const 0x1.a934f0979a371p+1 (;=3.32193;)
          f64.mul
          f64.const 0x1p-1 (;=0.5;)
          f64.add
          local.tee 12
          f64.abs
          f64.const 0x1p+31 (;=2.14748e+09;)
          f64.lt
          i32.eqz
          br_if 0 (;@3;)
          local.get 12
          i32.trunc_f64_s
          local.set 15
          br 1 (;@2;)
        end
        i32.const -2147483648
        local.set 15
      end
      local.get 7
      i32.const 1024
      i32.and
      local.set 9
      block  ;; label = @2
        local.get 10
        local.get 14
        local.get 15
        f64.convert_i32_s
        f64.const -0x1.62e42fefa39efp-1 (;=-0.693147;)
        f64.mul
        f64.add
        local.tee 12
        local.get 12
        f64.add
        f64.const 0x1p+1 (;=2;)
        local.get 12
        f64.sub
        local.get 12
        local.get 12
        f64.mul
        local.tee 12
        local.get 12
        local.get 12
        f64.const 0x1.cp+3 (;=14;)
        f64.div
        f64.const 0x1.4p+3 (;=10;)
        f64.add
        f64.div
        f64.const 0x1.8p+2 (;=6;)
        f64.add
        f64.div
        f64.add
        f64.div
        f64.const 0x1p+0 (;=1;)
        f64.add
        local.get 15
        i32.const 1023
        i32.add
        i64.extend_i32_u
        i64.const 52
        i64.shl
        f64.reinterpret_i64
        f64.mul
        local.tee 12
        f64.lt
        i32.eqz
        br_if 0 (;@2;)
        local.get 12
        f64.const 0x1.4p+3 (;=10;)
        f64.div
        local.set 12
        local.get 13
        i32.const -1
        i32.add
        local.set 13
      end
      local.get 5
      i32.const 6
      local.get 9
      select
      local.set 15
      i32.const 4
      i32.const 5
      local.get 13
      i32.const 99
      i32.add
      i32.const 199
      i32.lt_u
      select
      local.set 16
      block  ;; label = @2
        local.get 7
        i32.const 2048
        i32.and
        i32.eqz
        br_if 0 (;@2;)
        block  ;; label = @3
          local.get 10
          f64.const 0x1.a36e2eb1c432dp-14 (;=0.0001;)
          f64.ge
          i32.eqz
          br_if 0 (;@3;)
          local.get 10
          f64.const 0x1.e848p+19 (;=1e+06;)
          f64.lt
          i32.eqz
          br_if 0 (;@3;)
          local.get 15
          local.get 13
          i32.gt_s
          local.set 5
          local.get 13
          i32.const -1
          i32.xor
          local.set 9
          i32.const 0
          local.set 13
          local.get 15
          local.get 9
          i32.add
          i32.const 0
          local.get 5
          select
          local.set 15
          local.get 7
          i32.const 1024
          i32.or
          local.set 7
          i32.const 0
          local.set 16
          br 1 (;@2;)
        end
        block  ;; label = @3
          local.get 15
          br_if 0 (;@3;)
          i32.const 0
          local.set 15
          br 1 (;@2;)
        end
        local.get 15
        local.get 9
        i32.const 0
        i32.ne
        i32.sub
        local.set 15
      end
      i32.const 0
      local.set 5
      local.get 0
      local.get 1
      local.get 2
      local.get 3
      local.get 10
      local.get 12
      f64.const 0x1p+0 (;=1;)
      local.get 13
      select
      f64.div
      local.tee 12
      f64.neg
      local.get 12
      local.get 4
      f64.const 0x0p+0 (;=0;)
      f64.lt
      select
      local.get 15
      i32.const 0
      i32.const 0
      local.get 6
      local.get 16
      i32.sub
      local.tee 9
      local.get 9
      local.get 6
      i32.gt_u
      select
      local.tee 9
      local.get 16
      select
      local.get 9
      local.get 7
      i32.const 2
      i32.and
      local.tee 17
      i32.const 1
      i32.shr_u
      select
      local.get 7
      i32.const -2049
      i32.and
      call $_ftoa_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__double__unsigned_int__unsigned_int__unsigned_int_
      local.set 9
      local.get 16
      i32.eqz
      br_if 0 (;@1;)
      i32.const 69
      i32.const 101
      local.get 7
      i32.const 32
      i32.and
      select
      local.get 1
      local.get 9
      local.get 3
      local.get 0
      call_indirect (type 0)
      local.get 13
      local.get 13
      i32.const 31
      i32.shr_s
      local.tee 7
      i32.xor
      local.get 7
      i32.sub
      local.set 15
      i32.const -1
      local.set 18
      block  ;; label = @2
        loop  ;; label = @3
          local.get 8
          local.get 5
          i32.add
          local.get 15
          local.get 15
          i32.const 10
          i32.div_u
          local.tee 19
          i32.const 10
          i32.mul
          i32.sub
          i32.const 48
          i32.or
          i32.store8
          local.get 18
          i32.const 1
          i32.add
          local.set 18
          local.get 5
          i32.const 1
          i32.add
          local.set 7
          local.get 15
          i32.const 10
          i32.lt_u
          br_if 1 (;@2;)
          local.get 5
          i32.const 31
          i32.lt_u
          local.set 20
          local.get 7
          local.set 5
          local.get 19
          local.set 15
          local.get 20
          br_if 0 (;@3;)
        end
      end
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.const 31
      i32.lt_u
      local.set 19
      block  ;; label = @2
        block  ;; label = @3
          local.get 16
          i32.const -2
          i32.add
          local.tee 20
          local.get 7
          i32.gt_u
          br_if 0 (;@3;)
          local.get 7
          local.set 5
          br 1 (;@2;)
        end
        block  ;; label = @3
          local.get 5
          i32.const 30
          i32.le_u
          br_if 0 (;@3;)
          local.get 7
          local.set 5
          br 1 (;@2;)
        end
        local.get 8
        local.get 7
        i32.add
        i32.const 48
        local.get 18
        i32.const 30
        local.get 18
        i32.const 30
        i32.gt_u
        select
        local.get 7
        i32.sub
        i32.const 1
        i32.add
        local.tee 5
        local.get 16
        local.get 7
        i32.sub
        i32.const -3
        i32.add
        local.tee 15
        local.get 5
        local.get 15
        i32.lt_u
        select
        i32.const 1
        i32.add
        call $memset
        drop
        loop  ;; label = @3
          local.get 7
          i32.const 31
          i32.lt_u
          local.set 19
          local.get 20
          local.get 7
          i32.const 1
          i32.add
          local.tee 5
          i32.le_u
          br_if 1 (;@2;)
          local.get 7
          i32.const 31
          i32.lt_u
          local.set 15
          local.get 5
          local.set 7
          local.get 15
          br_if 0 (;@3;)
        end
      end
      block  ;; label = @2
        local.get 19
        i32.eqz
        br_if 0 (;@2;)
        local.get 8
        local.get 5
        i32.add
        i32.const 45
        i32.const 43
        local.get 13
        i32.const 0
        i32.lt_s
        select
        i32.store8
        local.get 5
        i32.const 1
        i32.add
        local.set 5
      end
      local.get 9
      i32.const 1
      i32.add
      local.set 9
      block  ;; label = @2
        local.get 5
        i32.eqz
        br_if 0 (;@2;)
        local.get 8
        i32.const -1
        i32.add
        local.set 7
        loop  ;; label = @3
          local.get 7
          local.get 5
          i32.add
          i32.load8_s
          local.get 1
          local.get 9
          local.get 3
          local.get 0
          call_indirect (type 0)
          local.get 9
          i32.const 1
          i32.add
          local.set 9
          local.get 5
          i32.const -1
          i32.add
          local.tee 5
          br_if 0 (;@3;)
        end
      end
      local.get 17
      i32.eqz
      br_if 0 (;@1;)
      local.get 9
      local.get 2
      i32.sub
      local.get 6
      i32.ge_u
      br_if 0 (;@1;)
      i32.const 0
      local.get 2
      i32.sub
      local.set 7
      loop  ;; label = @2
        i32.const 32
        local.get 1
        local.get 9
        local.get 3
        local.get 0
        call_indirect (type 0)
        local.get 7
        local.get 9
        i32.const 1
        i32.add
        local.tee 9
        i32.add
        local.get 6
        i32.lt_u
        br_if 0 (;@2;)
      end
    end
    local.get 8
    i32.const 32
    i32.add
    global.set $__stack_pointer
    local.get 9)
  (func $sprintf_ (type 3) (param i32 i32 i32) (result i32)
    (local i32)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 3
    global.set $__stack_pointer
    local.get 3
    local.get 2
    i32.store offset=12
    i32.const 2
    local.get 0
    i32.const -1
    local.get 1
    local.get 2
    call $_vsnprintf_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__char_const*__void*_
    local.set 2
    local.get 3
    i32.const 16
    i32.add
    global.set $__stack_pointer
    local.get 2)
  (func $_out_buffer_char__void*__unsigned_long__unsigned_long_ (type 0) (param i32 i32 i32 i32)
    block  ;; label = @1
      local.get 2
      local.get 3
      i32.ge_u
      br_if 0 (;@1;)
      local.get 1
      local.get 2
      i32.add
      local.get 0
      i32.store8
    end)
  (func $logRuntimeErrorList__ (type 4)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 0
    global.set $__stack_pointer
    i32.const 82
    local.set 1
    i32.const 1
    local.set 2
    loop  ;; label = @1
      local.get 1
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 2
      i32.const 3997
      i32.add
      i32.load8_u
      local.set 1
      local.get 2
      i32.const 1
      i32.add
      local.tee 3
      local.set 2
      local.get 3
      i32.const 27
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 0
    local.set 4
    loop  ;; label = @1
      i32.const 1
      local.set 2
      i32.const 32
      local.set 1
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 2
        i32.const 3950
        i32.add
        i32.load8_u
        local.set 1
        local.get 2
        i32.const 1
        i32.add
        local.tee 3
        local.set 2
        local.get 3
        i32.const 5
        i32.ne
        br_if 0 (;@2;)
      end
      block  ;; label = @2
        local.get 4
        i32.const 9
        i32.gt_u
        br_if 0 (;@2;)
        i32.const 32
        call $__print
      end
      local.get 0
      local.get 4
      i32.store
      i32.const 4400
      i32.const 1466
      local.get 0
      call $sprintf_
      drop
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 2
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 1
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 1
          i32.load8_u
          local.set 2
          local.get 1
          i32.const 1
          i32.add
          local.set 1
          local.get 2
          br_if 0 (;@3;)
        end
      end
      i32.const 58
      local.set 1
      i32.const 1
      local.set 2
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 2
        i32.const 3915
        i32.add
        i32.load8_u
        local.set 1
        local.get 2
        i32.const 1
        i32.add
        local.tee 3
        local.set 2
        local.get 3
        i32.const 3
        i32.ne
        br_if 0 (;@2;)
      end
      block  ;; label = @2
        local.get 4
        i32.const 2
        i32.shl
        i32.const 4032
        i32.add
        i32.load
        local.tee 1
        i32.load8_u
        local.tee 2
        i32.eqz
        br_if 0 (;@2;)
        local.get 1
        i32.const 1
        i32.add
        local.set 1
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 1
          i32.load8_u
          local.set 2
          local.get 1
          i32.const 1
          i32.add
          local.set 1
          local.get 2
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
      local.get 4
      i32.const 1
      i32.add
      local.tee 4
      i32.const 23
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    local.get 0
    i32.const 16
    i32.add
    global.set $__stack_pointer)
  (func $OPCODE_EXISTS_PLCRuntimeInstructionSet_ (type 2) (param i32) (result i32)
    (local i32)
    i32.const 1
    local.set 1
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        br_table 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 1 (;@1;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 0 (;@2;) 1 (;@1;) 0 (;@2;)
      end
      i32.const 0
      local.set 1
    end
    local.get 1)
  (func $OPCODE_NAME_PLCRuntimeInstructionSet_ (type 2) (param i32) (result i32)
    (local i32)
    i32.const 1808
    local.set 1
    block  ;; label = @1
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          block  ;; label = @12
                            block  ;; label = @13
                              block  ;; label = @14
                                block  ;; label = @15
                                  block  ;; label = @16
                                    block  ;; label = @17
                                      block  ;; label = @18
                                        block  ;; label = @19
                                          block  ;; label = @20
                                            block  ;; label = @21
                                              block  ;; label = @22
                                                block  ;; label = @23
                                                  block  ;; label = @24
                                                    block  ;; label = @25
                                                      block  ;; label = @26
                                                        block  ;; label = @27
                                                          block  ;; label = @28
                                                            block  ;; label = @29
                                                              block  ;; label = @30
                                                                block  ;; label = @31
                                                                  block  ;; label = @32
                                                                    block  ;; label = @33
                                                                      block  ;; label = @34
                                                                        block  ;; label = @35
                                                                          block  ;; label = @36
                                                                            block  ;; label = @37
                                                                              block  ;; label = @38
                                                                                block  ;; label = @39
                                                                                  block  ;; label = @40
                                                                                    block  ;; label = @41
                                                                                      block  ;; label = @42
                                                                                        block  ;; label = @43
                                                                                          block  ;; label = @44
                                                                                            block  ;; label = @45
                                                                                              block  ;; label = @46
                                                                                                block  ;; label = @47
                                                                                                  block  ;; label = @48
                                                                                                    block  ;; label = @49
                                                                                                      block  ;; label = @50
                                                                                                        block  ;; label = @51
                                                                                                          block  ;; label = @52
                                                                                                            block  ;; label = @53
                                                                                                              block  ;; label = @54
                                                                                                                block  ;; label = @55
                                                                                                                  block  ;; label = @56
                                                                                                                    block  ;; label = @57
                                                                                                                      block  ;; label = @58
                                                                                                                        block  ;; label = @59
                                                                                                                          block  ;; label = @60
                                                                                                                            block  ;; label = @61
                                                                                                                              block  ;; label = @62
                                                                                                                                local.get 0
                                                                                                                                br_table 61 (;@1;) 0 (;@62;) 1 (;@61;) 3 (;@59;) 5 (;@57;) 7 (;@55;) 2 (;@60;) 4 (;@58;) 6 (;@56;) 8 (;@54;) 9 (;@53;) 10 (;@52;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 11 (;@51;) 12 (;@50;) 13 (;@49;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 14 (;@48;) 15 (;@47;) 16 (;@46;) 17 (;@45;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 18 (;@44;) 19 (;@43;) 20 (;@42;) 21 (;@41;) 22 (;@40;) 23 (;@39;) 24 (;@38;) 25 (;@37;) 26 (;@36;) 27 (;@35;) 28 (;@34;) 29 (;@33;) 30 (;@32;) 31 (;@31;) 32 (;@30;) 33 (;@29;) 34 (;@28;) 35 (;@27;) 36 (;@26;) 37 (;@25;) 38 (;@24;) 39 (;@23;) 40 (;@22;) 41 (;@21;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 42 (;@20;) 43 (;@19;) 44 (;@18;) 45 (;@17;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 46 (;@16;) 47 (;@15;) 48 (;@14;) 49 (;@13;) 50 (;@12;) 51 (;@11;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 52 (;@10;) 53 (;@9;) 54 (;@8;) 55 (;@7;) 56 (;@6;) 57 (;@5;) 58 (;@4;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 60 (;@2;) 59 (;@3;) 60 (;@2;)
                                                                                                                              end
                                                                                                                              i32.const 1307
                                                                                                                              return
                                                                                                                            end
                                                                                                                            i32.const 1193
                                                                                                                            return
                                                                                                                          end
                                                                                                                          i32.const 1206
                                                                                                                          return
                                                                                                                        end
                                                                                                                        i32.const 1218
                                                                                                                        return
                                                                                                                      end
                                                                                                                      i32.const 1232
                                                                                                                      return
                                                                                                                    end
                                                                                                                    i32.const 1272
                                                                                                                    return
                                                                                                                  end
                                                                                                                  i32.const 1286
                                                                                                                  return
                                                                                                                end
                                                                                                                i32.const 1245
                                                                                                                return
                                                                                                              end
                                                                                                              i32.const 1259
                                                                                                              return
                                                                                                            end
                                                                                                            i32.const 1182
                                                                                                            return
                                                                                                          end
                                                                                                          i32.const 1447
                                                                                                          return
                                                                                                        end
                                                                                                        i32.const 1576
                                                                                                        return
                                                                                                      end
                                                                                                      i32.const 1584
                                                                                                      return
                                                                                                    end
                                                                                                    i32.const 1662
                                                                                                    return
                                                                                                  end
                                                                                                  i32.const 2104
                                                                                                  return
                                                                                                end
                                                                                                i32.const 2108
                                                                                                return
                                                                                              end
                                                                                              i32.const 1898
                                                                                              return
                                                                                            end
                                                                                            i32.const 1580
                                                                                            return
                                                                                          end
                                                                                          i32.const 2325
                                                                                          return
                                                                                        end
                                                                                        i32.const 2411
                                                                                        return
                                                                                      end
                                                                                      i32.const 2715
                                                                                      return
                                                                                    end
                                                                                    i32.const 2502
                                                                                    return
                                                                                  end
                                                                                  i32.const 2306
                                                                                  return
                                                                                end
                                                                                i32.const 2390
                                                                                return
                                                                              end
                                                                              i32.const 2694
                                                                              return
                                                                            end
                                                                            i32.const 2481
                                                                            return
                                                                          end
                                                                          i32.const 2315
                                                                          return
                                                                        end
                                                                        i32.const 2400
                                                                        return
                                                                      end
                                                                      i32.const 2704
                                                                      return
                                                                    end
                                                                    i32.const 2491
                                                                    return
                                                                  end
                                                                  i32.const 2270
                                                                  return
                                                                end
                                                                i32.const 2351
                                                                return
                                                              end
                                                              i32.const 2655
                                                              return
                                                            end
                                                            i32.const 2442
                                                            return
                                                          end
                                                          i32.const 2293
                                                          return
                                                        end
                                                        i32.const 2376
                                                        return
                                                      end
                                                      i32.const 2680
                                                      return
                                                    end
                                                    i32.const 2467
                                                    return
                                                  end
                                                  i32.const 2280
                                                  return
                                                end
                                                i32.const 2362
                                                return
                                              end
                                              i32.const 2666
                                              return
                                            end
                                            i32.const 2453
                                            return
                                          end
                                          i32.const 2057
                                          return
                                        end
                                        i32.const 1738
                                        return
                                      end
                                      i32.const 1747
                                      return
                                    end
                                    i32.const 1629
                                    return
                                  end
                                  i32.const 1793
                                  return
                                end
                                i32.const 1800
                                return
                              end
                              i32.const 1651
                              return
                            end
                            i32.const 1639
                            return
                          end
                          i32.const 1981
                          return
                        end
                        i32.const 1973
                        return
                      end
                      i32.const 1812
                      return
                    end
                    i32.const 1919
                    return
                  end
                  i32.const 1606
                  return
                end
                i32.const 1902
                return
              end
              i32.const 1926
              return
            end
            i32.const 1617
            return
          end
          i32.const 1658
          return
        end
        i32.const 1646
        return
      end
      i32.const 2042
      local.set 1
    end
    local.get 1)
  (func $OPCODE_SIZE_PLCRuntimeInstructionSet_ (type 2) (param i32) (result i32)
    (local i32)
    i32.const 1
    local.set 1
    block  ;; label = @1
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          block  ;; label = @12
                            local.get 0
                            br_table 11 (;@1;) 0 (;@12;) 0 (;@12;) 1 (;@11;) 2 (;@10;) 3 (;@9;) 0 (;@12;) 1 (;@11;) 2 (;@10;) 3 (;@9;) 4 (;@8;) 5 (;@7;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 6 (;@6;) 7 (;@5;) 7 (;@5;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 8 (;@4;) 8 (;@4;) 8 (;@4;) 8 (;@4;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 11 (;@1;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 8 (;@4;) 8 (;@4;) 8 (;@4;) 8 (;@4;) 8 (;@4;) 8 (;@4;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 9 (;@3;) 9 (;@3;) 9 (;@3;) 9 (;@3;) 9 (;@3;) 9 (;@3;) 11 (;@1;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 10 (;@2;) 11 (;@1;) 10 (;@2;)
                          end
                          i32.const 2
                          return
                        end
                        i32.const 3
                        return
                      end
                      i32.const 5
                      return
                    end
                    i32.const 9
                    return
                  end
                  i32.const 5
                  return
                end
                i32.const 9
                return
              end
              i32.const 8
              return
            end
            i32.const 4
            return
          end
          i32.const 2
          return
        end
        i32.const 3
        return
      end
      i32.const 0
      local.set 1
    end
    local.get 1)
  (func $logRuntimeInstructionSet__ (type 4)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 0
    global.set $__stack_pointer
    i32.const 66
    local.set 1
    i32.const 1
    local.set 2
    loop  ;; label = @1
      local.get 1
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 2
      i32.const 2132
      i32.add
      i32.load8_u
      local.set 1
      local.get 2
      i32.const 1
      i32.add
      local.tee 3
      local.set 2
      local.get 3
      i32.const 26
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 1
    local.set 1
    i32.const 0
    local.set 4
    loop  ;; label = @1
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            local.get 4
            call $OPCODE_EXISTS_PLCRuntimeInstructionSet_
            local.tee 5
            i32.eqz
            br_if 0 (;@4;)
            local.get 1
            local.get 4
            i32.const 1
            i32.gt_u
            i32.and
            i32.eqz
            br_if 1 (;@3;)
            br 2 (;@2;)
          end
          local.get 4
          i32.const 1
          i32.gt_u
          br_if 1 (;@2;)
        end
        i32.const 10
        call $__print
      end
      block  ;; label = @2
        local.get 5
        i32.eqz
        br_if 0 (;@2;)
        i32.const 32
        local.set 1
        i32.const 1
        local.set 2
        loop  ;; label = @3
          local.get 1
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 2
          i32.const 1099
          i32.add
          i32.load8_u
          local.set 1
          local.get 2
          i32.const 1
          i32.add
          local.tee 3
          local.set 2
          local.get 3
          i32.const 7
          i32.ne
          br_if 0 (;@3;)
        end
        i32.const 6
        local.set 6
        block  ;; label = @3
          local.get 4
          i32.const 15
          i32.gt_u
          br_if 0 (;@3;)
          i32.const 48
          call $__print
          i32.const 7
          local.set 6
        end
        local.get 0
        local.get 4
        i32.store offset=16
        i32.const 4400
        i32.const 1106
        local.get 0
        i32.const 16
        i32.add
        call $sprintf_
        drop
        i32.const 0
        local.set 7
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 1
          i32.eqz
          br_if 0 (;@3;)
          i32.const 0
          local.set 2
          loop  ;; label = @4
            local.get 1
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 2
            i32.const 4401
            i32.add
            local.set 1
            local.get 2
            i32.const 1
            i32.add
            local.tee 7
            local.set 2
            local.get 1
            i32.load8_u
            local.tee 1
            br_if 0 (;@4;)
          end
        end
        i32.const 58
        local.set 1
        i32.const 1
        local.set 2
        loop  ;; label = @3
          local.get 1
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 2
          i32.const 3946
          i32.add
          i32.load8_u
          local.set 1
          local.get 2
          i32.const 1
          i32.add
          local.tee 3
          local.set 2
          local.get 3
          i32.const 4
          i32.ne
          br_if 0 (;@3;)
        end
        block  ;; label = @3
          block  ;; label = @4
            local.get 4
            call $OPCODE_NAME_PLCRuntimeInstructionSet_
            local.tee 2
            i32.load8_u
            local.tee 1
            br_if 0 (;@4;)
            i32.const 0
            local.set 3
            br 1 (;@3;)
          end
          local.get 2
          i32.const 1
          i32.add
          local.set 8
          i32.const 0
          local.set 2
          loop  ;; label = @4
            local.get 1
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 8
            local.get 2
            i32.add
            local.set 1
            local.get 2
            i32.const 1
            i32.add
            local.tee 3
            local.set 2
            local.get 1
            i32.load8_u
            local.tee 1
            br_if 0 (;@4;)
          end
        end
        block  ;; label = @3
          local.get 4
          call $OPCODE_SIZE_PLCRuntimeInstructionSet_
          local.tee 8
          i32.eqz
          br_if 0 (;@3;)
          block  ;; label = @4
            local.get 7
            local.get 6
            i32.add
            i32.const 3
            i32.add
            local.get 3
            i32.add
            i32.const 25
            i32.gt_s
            br_if 0 (;@4;)
            local.get 7
            local.get 3
            i32.add
            local.get 6
            i32.add
            i32.const 2
            i32.add
            local.set 2
            loop  ;; label = @5
              i32.const 32
              call $__print
              local.get 2
              i32.const 1
              i32.add
              local.tee 2
              i32.const 25
              i32.lt_s
              br_if 0 (;@5;)
            end
          end
          i32.const 40
          call $__print
          local.get 0
          local.get 8
          i32.store
          i32.const 4400
          i32.const 1466
          local.get 0
          call $sprintf_
          drop
          block  ;; label = @4
            i32.const 0
            i32.load8_u offset=4400
            local.tee 2
            i32.eqz
            br_if 0 (;@4;)
            i32.const 4401
            local.set 1
            loop  ;; label = @5
              local.get 2
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 1
              i32.load8_u
              local.set 2
              local.get 1
              i32.const 1
              i32.add
              local.set 1
              local.get 2
              br_if 0 (;@5;)
            end
          end
          i32.const 32
          local.set 1
          i32.const 1
          local.set 2
          loop  ;; label = @4
            local.get 1
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 2
            i32.const 1382
            i32.add
            i32.load8_u
            local.set 1
            local.get 2
            i32.const 1
            i32.add
            local.tee 3
            local.set 2
            local.get 3
            i32.const 6
            i32.ne
            br_if 0 (;@4;)
          end
          block  ;; label = @4
            local.get 8
            i32.const 2
            i32.lt_u
            br_if 0 (;@4;)
            i32.const 115
            call $__print
          end
          i32.const 41
          call $__print
        end
        i32.const 10
        call $__print
      end
      local.get 4
      i32.const 255
      i32.ne
      local.set 2
      local.get 5
      local.set 1
      local.get 4
      i32.const 1
      i32.add
      local.set 4
      local.get 2
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    local.get 0
    i32.const 32
    i32.add
    global.set $__stack_pointer)
  (func $Stack<unsigned_char>::push_unsigned_char_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 48
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load offset=8
        local.tee 3
        local.get 0
        i32.load offset=4
        i32.lt_s
        br_if 0 (;@2;)
        i32.const 83
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3256
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 16
          i32.ne
          br_if 0 (;@3;)
        end
        i32.const 10
        call $__print
        i32.const 84
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3526
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 16
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 1
        i32.store offset=32
        i32.const 4400
        i32.const 1106
        local.get 2
        i32.const 32
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 4
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 4
            i32.load8_u
            local.set 3
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3310
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 20
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load offset=8
        i32.store offset=16
        i32.const 4400
        i32.const 1466
        local.get 2
        i32.const 16
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 4
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 4
            i32.load8_u
            local.set 3
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        i32.const 77
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3556
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 18
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load offset=4
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 2
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 4
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 4
            i32.load8_u
            local.set 3
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 3
      i32.add
      local.get 1
      i32.store8
      local.get 0
      local.get 0
      i32.load offset=8
      i32.const 1
      i32.add
      i32.store offset=8
    end
    local.get 2
    i32.const 48
    i32.add
    global.set $__stack_pointer)
  (func $PLCMethods::ADD_uint16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      local.get 4
      i32.const 255
      i32.and
      i32.add
      local.get 3
      i32.const 8
      i32.shl
      i32.add
      local.tee 1
      i32.const 65280
      i32.and
      i32.const 8
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::SUB_uint16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 4
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      i32.const 0
      local.get 1
      i32.const 8
      i32.shl
      i32.sub
      local.get 2
      i32.const 255
      i32.and
      i32.sub
      local.get 3
      i32.const 255
      i32.and
      i32.add
      local.get 4
      i32.const 8
      i32.shl
      i32.add
      local.tee 1
      i32.const 65280
      i32.and
      i32.const 8
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::MUL_uint16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.mul
      local.tee 1
      i32.const 65280
      i32.and
      i32.const 8
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::DIV_uint16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      i32.div_u
      local.tee 1
      i32.const 8
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::ADD_uint32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.add
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::SUB_uint32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.sub
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::MUL_uint32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.mul
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::DIV_uint32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.div_u
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $RuntimeStack::pop_uint64_t__ (type 9) (param i32) (result i64)
    (local i64 i64 i32 i64 i64 i64 i64 i64 i64)
    i64.const 0
    local.set 1
    i64.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i64.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i64.load8_u
      i64.const 8
      i64.shl
      local.set 1
    end
    i64.const 0
    local.set 4
    i64.const 0
    local.set 5
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i64.load8_u
      i64.const 16
      i64.shl
      local.set 5
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i64.load8_u
      i64.const 24
      i64.shl
      local.set 4
    end
    i64.const 0
    local.set 6
    i64.const 0
    local.set 7
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i64.load8_u
      i64.const 32
      i64.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i64.load8_u
      i64.const 40
      i64.shl
      local.set 6
    end
    i64.const 0
    local.set 8
    i64.const 0
    local.set 9
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i64.load8_u
      i64.const 48
      i64.shl
      local.set 9
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i64.load8_u
      i64.const 56
      i64.shl
      local.set 8
    end
    local.get 1
    local.get 2
    i64.or
    local.get 5
    i64.or
    local.get 4
    i64.or
    local.get 7
    i64.or
    local.get 6
    i64.or
    local.get 9
    i64.or
    local.get 8
    i64.or)
  (func $RuntimeStack::push_uint64_t_unsigned_long_long_ (type 10) (param i32 i64) (result i32)
    (local i32)
    i32.const 4
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 8
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      i64.const 56
      i64.shr_u
      i32.wrap_i64
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i64.const 48
      i64.shr_u
      i32.wrap_i64
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i64.const 40
      i64.shr_u
      i32.wrap_i64
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i64.const 32
      i64.shr_u
      i32.wrap_i64
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.wrap_i64
      local.tee 2
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 2
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 2
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 2
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      i32.const 0
      local.set 2
    end
    local.get 2)
  (func $PLCMethods::ADD_int16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      local.get 4
      i32.const 255
      i32.and
      i32.add
      local.get 3
      i32.const 8
      i32.shl
      i32.add
      local.tee 1
      i32.const 65280
      i32.and
      i32.const 8
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::SUB_int16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 4
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      i32.const 0
      local.get 1
      i32.const 8
      i32.shl
      i32.sub
      local.get 2
      i32.const 255
      i32.and
      i32.sub
      local.get 3
      i32.const 255
      i32.and
      i32.add
      local.get 4
      i32.const 8
      i32.shl
      i32.add
      local.tee 1
      i32.const 65280
      i32.and
      i32.const 8
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::MUL_int16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.mul
      local.tee 1
      i32.const 65280
      i32.and
      i32.const 8
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::DIV_int16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 16
      i32.shl
      i32.const 16
      i32.shr_s
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 16
      i32.shl
      i32.const 16
      i32.shr_s
      i32.div_s
      local.tee 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::ADD_int32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.add
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::SUB_int32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.sub
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::MUL_int32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.mul
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::DIV_int32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.div_s
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::ADD_float_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      f32.reinterpret_i32
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      f32.reinterpret_i32
      f32.add
      i32.reinterpret_f32
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::SUB_float_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      f32.reinterpret_i32
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      f32.reinterpret_i32
      f32.sub
      i32.reinterpret_f32
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::MUL_float_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      f32.reinterpret_i32
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      f32.reinterpret_i32
      f32.mul
      i32.reinterpret_f32
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::DIV_float_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      f32.reinterpret_i32
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      f32.reinterpret_i32
      f32.div
      i32.reinterpret_f32
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::PUSH_bool_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32)
    i32.const 14
    local.set 4
    i32.const 0
    local.set 5
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 6
      local.get 2
      i32.ge_u
      local.tee 2
      br_if 0 (;@1;)
      local.get 1
      local.get 6
      i32.add
      i32.load8_u
      local.set 5
      local.get 3
      local.get 6
      i32.const 1
      i32.add
      i32.store16
      i32.const 0
      local.set 4
      local.get 5
      i32.const 0
      i32.ne
      local.set 5
    end
    i32.const 0
    local.get 4
    i32.store offset=4724
    block  ;; label = @1
      local.get 2
      br_if 0 (;@1;)
      i32.const 4
      local.set 4
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      call $Stack<unsigned_char>::push_unsigned_char_
      i32.const 0
      local.set 4
    end
    local.get 4)
  (func $PLCMethods::PUSH_uint8_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32)
    i32.const 14
    local.set 4
    i32.const 0
    local.set 5
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 6
      local.get 2
      i32.ge_u
      local.tee 2
      br_if 0 (;@1;)
      local.get 1
      local.get 6
      i32.add
      i32.load8_u
      local.set 5
      local.get 3
      local.get 6
      i32.const 1
      i32.add
      i32.store16
      i32.const 0
      local.set 4
    end
    i32.const 0
    local.get 4
    i32.store offset=4724
    block  ;; label = @1
      local.get 2
      br_if 0 (;@1;)
      i32.const 4
      local.set 4
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      i32.const 0
      local.set 4
    end
    local.get 4)
  (func $PLCMethods::PUSH_uint16_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 14
    local.set 4
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 7
      i32.const 2
      i32.add
      local.tee 8
      local.get 2
      i32.gt_u
      local.tee 2
      br_if 0 (;@1;)
      local.get 1
      local.get 7
      i32.add
      local.tee 4
      i32.load8_u
      local.set 6
      local.get 4
      i32.const 1
      i32.add
      i32.load8_u
      local.set 5
      local.get 3
      local.get 8
      i32.store16
      i32.const 0
      local.set 4
    end
    i32.const 0
    local.get 4
    i32.store offset=4724
    block  ;; label = @1
      local.get 2
      br_if 0 (;@1;)
      i32.const 4
      local.set 4
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 6
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 5
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      i32.const 0
      local.set 4
    end
    local.get 4)
  (func $PLCMethods::PUSH_uint32_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32)
    i32.const 14
    local.set 4
    i32.const 0
    local.set 5
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 6
      i32.const 4
      i32.add
      local.tee 7
      local.get 2
      i32.gt_u
      local.tee 2
      br_if 0 (;@1;)
      local.get 1
      local.get 6
      i32.add
      i32.load align=1
      local.set 4
      local.get 3
      local.get 7
      i32.store16
      local.get 4
      i32.const 24
      i32.shl
      local.get 4
      i32.const 8
      i32.shl
      i32.const 16711680
      i32.and
      i32.or
      local.get 4
      i32.const 8
      i32.shr_u
      i32.const 65280
      i32.and
      local.get 4
      i32.const 24
      i32.shr_u
      i32.or
      i32.or
      local.set 5
      i32.const 0
      local.set 4
    end
    i32.const 0
    local.get 4
    i32.store offset=4724
    block  ;; label = @1
      local.get 2
      br_if 0 (;@1;)
      i32.const 4
      local.set 4
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 5
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 5
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 5
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      i32.const 0
      local.set 4
    end
    local.get 4)
  (func $PLCMethods::PUSH_uint64_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i64)
    block  ;; label = @1
      block  ;; label = @2
        local.get 3
        i32.load16_u
        local.tee 4
        i32.const 8
        i32.add
        local.tee 5
        local.get 2
        i32.gt_u
        local.tee 2
        i32.eqz
        br_if 0 (;@2;)
        i32.const 14
        local.set 3
        i64.const 0
        local.set 6
        br 1 (;@1;)
      end
      local.get 1
      local.get 4
      i32.add
      i64.load align=1
      local.set 6
      local.get 3
      local.get 5
      i32.store16
      local.get 6
      i64.const 56
      i64.shl
      local.get 6
      i64.const 40
      i64.shl
      i64.const 71776119061217280
      i64.and
      i64.or
      local.get 6
      i64.const 24
      i64.shl
      i64.const 280375465082880
      i64.and
      local.get 6
      i64.const 8
      i64.shl
      i64.const 1095216660480
      i64.and
      i64.or
      i64.or
      local.get 6
      i64.const 8
      i64.shr_u
      i64.const 4278190080
      i64.and
      local.get 6
      i64.const 24
      i64.shr_u
      i64.const 16711680
      i64.and
      i64.or
      local.get 6
      i64.const 40
      i64.shr_u
      i64.const 65280
      i64.and
      local.get 6
      i64.const 56
      i64.shr_u
      i64.or
      i64.or
      i64.or
      local.set 6
      i32.const 0
      local.set 3
    end
    i32.const 0
    local.get 3
    i32.store offset=4724
    block  ;; label = @1
      local.get 2
      br_if 0 (;@1;)
      local.get 0
      local.get 6
      call $RuntimeStack::push_uint64_t_unsigned_long_long_
      local.set 3
    end
    local.get 3)
  (func $PLCMethods::PUSH_int8_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32)
    i32.const 14
    local.set 4
    i32.const 0
    local.set 5
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 6
      local.get 2
      i32.ge_u
      local.tee 2
      br_if 0 (;@1;)
      local.get 1
      local.get 6
      i32.add
      i32.load8_u
      local.set 5
      local.get 3
      local.get 6
      i32.const 1
      i32.add
      i32.store16
      i32.const 0
      local.set 4
    end
    i32.const 0
    local.get 4
    i32.store offset=4724
    block  ;; label = @1
      local.get 2
      br_if 0 (;@1;)
      i32.const 4
      local.set 4
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      i32.const 0
      local.set 4
    end
    local.get 4)
  (func $PLCMethods::PUSH_int16_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 14
    local.set 4
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 7
      i32.const 2
      i32.add
      local.tee 8
      local.get 2
      i32.gt_u
      local.tee 2
      br_if 0 (;@1;)
      local.get 1
      local.get 7
      i32.add
      local.tee 4
      i32.load8_u
      local.set 6
      local.get 4
      i32.const 1
      i32.add
      i32.load8_u
      local.set 5
      local.get 3
      local.get 8
      i32.store16
      i32.const 0
      local.set 4
    end
    i32.const 0
    local.get 4
    i32.store offset=4724
    block  ;; label = @1
      local.get 2
      br_if 0 (;@1;)
      i32.const 4
      local.set 4
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 6
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 5
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      i32.const 0
      local.set 4
    end
    local.get 4)
  (func $PLCMethods::PUSH_int32_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32)
    i32.const 14
    local.set 4
    i32.const 0
    local.set 5
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 6
      i32.const 4
      i32.add
      local.tee 7
      local.get 2
      i32.gt_u
      local.tee 2
      br_if 0 (;@1;)
      local.get 1
      local.get 6
      i32.add
      i32.load align=1
      local.set 4
      local.get 3
      local.get 7
      i32.store16
      local.get 4
      i32.const 24
      i32.shl
      local.get 4
      i32.const 8
      i32.shl
      i32.const 16711680
      i32.and
      i32.or
      local.get 4
      i32.const 8
      i32.shr_u
      i32.const 65280
      i32.and
      local.get 4
      i32.const 24
      i32.shr_u
      i32.or
      i32.or
      local.set 5
      i32.const 0
      local.set 4
    end
    i32.const 0
    local.get 4
    i32.store offset=4724
    block  ;; label = @1
      local.get 2
      br_if 0 (;@1;)
      i32.const 4
      local.set 4
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 5
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 5
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 5
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      i32.const 0
      local.set 4
    end
    local.get 4)
  (func $PLCMethods::PUSH_int64_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i64)
    block  ;; label = @1
      block  ;; label = @2
        local.get 3
        i32.load16_u
        local.tee 4
        i32.const 8
        i32.add
        local.tee 5
        local.get 2
        i32.gt_u
        local.tee 2
        i32.eqz
        br_if 0 (;@2;)
        i32.const 14
        local.set 3
        i64.const 0
        local.set 6
        br 1 (;@1;)
      end
      local.get 1
      local.get 4
      i32.add
      i64.load align=1
      local.set 6
      local.get 3
      local.get 5
      i32.store16
      local.get 6
      i64.const 56
      i64.shl
      local.get 6
      i64.const 40
      i64.shl
      i64.const 71776119061217280
      i64.and
      i64.or
      local.get 6
      i64.const 24
      i64.shl
      i64.const 280375465082880
      i64.and
      local.get 6
      i64.const 8
      i64.shl
      i64.const 1095216660480
      i64.and
      i64.or
      i64.or
      local.get 6
      i64.const 8
      i64.shr_u
      i64.const 4278190080
      i64.and
      local.get 6
      i64.const 24
      i64.shr_u
      i64.const 16711680
      i64.and
      i64.or
      local.get 6
      i64.const 40
      i64.shr_u
      i64.const 65280
      i64.and
      local.get 6
      i64.const 56
      i64.shr_u
      i64.or
      i64.or
      i64.or
      local.set 6
      i32.const 0
      local.set 3
    end
    i32.const 0
    local.get 3
    i32.store offset=4724
    block  ;; label = @1
      local.get 2
      br_if 0 (;@1;)
      local.get 0
      local.get 6
      call $RuntimeStack::push_uint64_t_unsigned_long_long_
      local.set 3
    end
    local.get 3)
  (func $PLCMethods::PUSH_float_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32)
    i32.const 14
    local.set 4
    i32.const 0
    local.set 5
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 6
      i32.const 4
      i32.add
      local.tee 7
      local.get 2
      i32.gt_u
      local.tee 2
      br_if 0 (;@1;)
      local.get 1
      local.get 6
      i32.add
      i32.load align=1
      local.set 4
      local.get 3
      local.get 7
      i32.store16
      local.get 4
      i32.const 24
      i32.shl
      local.get 4
      i32.const 8
      i32.shl
      i32.const 16711680
      i32.and
      i32.or
      local.get 4
      i32.const 8
      i32.shr_u
      i32.const 65280
      i32.and
      local.get 4
      i32.const 24
      i32.shr_u
      i32.or
      i32.or
      local.set 5
      i32.const 0
      local.set 4
    end
    i32.const 0
    local.get 4
    i32.store offset=4724
    block  ;; label = @1
      local.get 2
      br_if 0 (;@1;)
      i32.const 4
      local.set 4
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 5
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 5
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 5
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      i32.const 0
      local.set 4
    end
    local.get 4)
  (func $PLCMethods::PUSH_double_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i64)
    block  ;; label = @1
      block  ;; label = @2
        local.get 3
        i32.load16_u
        local.tee 4
        i32.const 8
        i32.add
        local.tee 5
        local.get 2
        i32.gt_u
        local.tee 2
        i32.eqz
        br_if 0 (;@2;)
        i32.const 14
        local.set 3
        i64.const 0
        local.set 6
        br 1 (;@1;)
      end
      local.get 1
      local.get 4
      i32.add
      i64.load align=1
      local.set 6
      local.get 3
      local.get 5
      i32.store16
      local.get 6
      i64.const 56
      i64.shl
      local.get 6
      i64.const 40
      i64.shl
      i64.const 71776119061217280
      i64.and
      i64.or
      local.get 6
      i64.const 24
      i64.shl
      i64.const 280375465082880
      i64.and
      local.get 6
      i64.const 8
      i64.shl
      i64.const 1095216660480
      i64.and
      i64.or
      i64.or
      local.get 6
      i64.const 8
      i64.shr_u
      i64.const 4278190080
      i64.and
      local.get 6
      i64.const 24
      i64.shr_u
      i64.const 16711680
      i64.and
      i64.or
      local.get 6
      i64.const 40
      i64.shr_u
      i64.const 65280
      i64.and
      local.get 6
      i64.const 56
      i64.shr_u
      i64.or
      i64.or
      i64.or
      local.set 6
      i32.const 0
      local.set 3
    end
    i32.const 0
    local.get 3
    i32.store offset=4724
    block  ;; label = @1
      local.get 2
      br_if 0 (;@1;)
      local.get 0
      local.get 6
      call $RuntimeStack::push_uint64_t_unsigned_long_long_
      local.set 3
    end
    local.get 3)
  (func $PLCMethods::PUT_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 4
    global.set $__stack_pointer
    i32.const 15
    local.set 5
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 6
      i32.const 3
      i32.add
      local.get 2
      i32.gt_u
      br_if 0 (;@1;)
      i32.const 14
      local.set 5
      block  ;; label = @2
        local.get 6
        local.get 2
        i32.ge_u
        local.tee 7
        br_if 0 (;@2;)
        local.get 3
        local.get 6
        i32.const 1
        i32.add
        i32.store16
        i32.const 0
        local.set 5
      end
      i32.const 0
      local.set 6
      i32.const 0
      local.get 5
      i32.store offset=4724
      local.get 7
      br_if 0 (;@1;)
      i32.const 14
      local.set 5
      block  ;; label = @2
        local.get 3
        i32.load16_u
        local.tee 7
        i32.const 2
        i32.add
        local.tee 8
        local.get 2
        i32.gt_u
        local.tee 2
        br_if 0 (;@2;)
        local.get 1
        local.get 7
        i32.add
        local.tee 5
        i32.const 1
        i32.add
        i32.load8_u
        local.set 6
        local.get 5
        i32.load8_u
        local.set 5
        local.get 3
        local.get 8
        i32.store16
        local.get 6
        local.get 5
        i32.const 8
        i32.shl
        i32.or
        local.set 6
        i32.const 0
        local.set 5
      end
      i32.const 0
      local.set 3
      i32.const 0
      local.get 5
      i32.store offset=4724
      local.get 2
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 0
        i32.load offset=8
        local.tee 5
        i32.eqz
        br_if 0 (;@2;)
        local.get 0
        local.get 5
        i32.const -1
        i32.add
        local.tee 5
        i32.store offset=8
        local.get 0
        i32.load
        local.get 5
        i32.add
        i32.load8_u
        local.set 3
      end
      block  ;; label = @2
        local.get 0
        i32.const 36
        i32.add
        i32.load
        local.get 6
        i32.gt_s
        br_if 0 (;@2;)
        i32.const 80
        local.set 2
        i32.const 1
        local.set 5
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 5
          i32.const 3354
          i32.add
          i32.load8_u
          local.set 2
          local.get 5
          i32.const 1
          i32.add
          local.tee 3
          local.set 5
          local.get 3
          i32.const 46
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 0
        i32.load16_u offset=36
        i32.store offset=16
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 16
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 5
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 2
          loop  ;; label = @4
            local.get 5
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 2
            i32.load8_u
            local.set 5
            local.get 2
            i32.const 1
            i32.add
            local.set 2
            local.get 5
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        local.set 2
        i32.const 1
        local.set 5
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 5
          i32.const 3330
          i32.add
          i32.load8_u
          local.set 2
          local.get 5
          i32.const 1
          i32.add
          local.tee 3
          local.set 5
          local.get 3
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 4
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 5
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 2
          loop  ;; label = @4
            local.get 5
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 2
            i32.load8_u
            local.set 5
            local.get 2
            i32.const 1
            i32.add
            local.set 2
            local.get 5
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        i32.const 22
        local.set 5
        br 1 (;@1;)
      end
      local.get 0
      i32.load offset=28
      local.get 6
      i32.add
      local.get 3
      i32.store8
      i32.const 0
      local.set 5
    end
    local.get 4
    i32.const 32
    i32.add
    global.set $__stack_pointer
    local.get 5)
  (func $PLCMethods::GET_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 4
    global.set $__stack_pointer
    i32.const 15
    local.set 5
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 6
      i32.const 3
      i32.add
      local.get 2
      i32.gt_u
      br_if 0 (;@1;)
      i32.const 14
      local.set 5
      block  ;; label = @2
        local.get 6
        local.get 2
        i32.ge_u
        local.tee 7
        br_if 0 (;@2;)
        local.get 3
        local.get 6
        i32.const 1
        i32.add
        i32.store16
        i32.const 0
        local.set 5
      end
      i32.const 0
      local.set 6
      i32.const 0
      local.get 5
      i32.store offset=4724
      local.get 7
      br_if 0 (;@1;)
      i32.const 14
      local.set 5
      block  ;; label = @2
        local.get 3
        i32.load16_u
        local.tee 7
        i32.const 2
        i32.add
        local.tee 8
        local.get 2
        i32.gt_u
        local.tee 2
        br_if 0 (;@2;)
        local.get 1
        local.get 7
        i32.add
        local.tee 5
        i32.const 1
        i32.add
        i32.load8_u
        local.set 6
        local.get 5
        i32.load8_u
        local.set 5
        local.get 3
        local.get 8
        i32.store16
        local.get 6
        local.get 5
        i32.const 8
        i32.shl
        i32.or
        local.set 6
        i32.const 0
        local.set 5
      end
      i32.const 0
      local.set 3
      i32.const 0
      local.get 5
      i32.store offset=4724
      local.get 2
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 0
        i32.const 36
        i32.add
        i32.load
        local.tee 5
        local.get 6
        i32.le_s
        br_if 0 (;@2;)
        local.get 0
        i32.load offset=28
        local.get 6
        i32.add
        i32.load8_u
        local.set 3
      end
      block  ;; label = @2
        local.get 5
        local.get 6
        i32.gt_s
        br_if 0 (;@2;)
        i32.const 71
        local.set 2
        i32.const 1
        local.set 5
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 5
          i32.const 3400
          i32.add
          i32.load8_u
          local.set 2
          local.get 5
          i32.const 1
          i32.add
          local.tee 3
          local.set 5
          local.get 3
          i32.const 46
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 0
        i32.load16_u offset=36
        i32.store offset=16
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 16
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 5
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 2
          loop  ;; label = @4
            local.get 5
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 2
            i32.load8_u
            local.set 5
            local.get 2
            i32.const 1
            i32.add
            local.set 2
            local.get 5
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        local.set 2
        i32.const 1
        local.set 5
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 5
          i32.const 3330
          i32.add
          i32.load8_u
          local.set 2
          local.get 5
          i32.const 1
          i32.add
          local.tee 3
          local.set 5
          local.get 3
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 4
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 5
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 2
          loop  ;; label = @4
            local.get 5
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 2
            i32.load8_u
            local.set 5
            local.get 2
            i32.const 1
            i32.add
            local.set 2
            local.get 5
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        i32.const 22
        local.set 5
        br 1 (;@1;)
      end
      i32.const 4
      local.set 5
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      i32.const 0
      local.set 5
    end
    local.get 4
    i32.const 32
    i32.add
    global.set $__stack_pointer
    local.get 5)
  (func $PLCMethods::BW_AND_X8_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      local.get 2
      i32.and
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::BW_AND_X16_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.and
      local.tee 1
      i32.const 65280
      i32.and
      i32.const 8
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::BW_AND_X32_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.and
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::BW_AND_X64_RuntimeStack*_ (type 2) (param i32) (result i32)
    local.get 0
    local.get 0
    call $RuntimeStack::pop_uint64_t__
    local.get 0
    call $RuntimeStack::pop_uint64_t__
    i64.and
    call $RuntimeStack::push_uint64_t_unsigned_long_long_
    drop
    i32.const 0)
  (func $PLCMethods::BW_OR_X8_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      local.get 2
      i32.or
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::BW_OR_X16_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.or
      local.tee 1
      i32.const 65280
      i32.and
      i32.const 8
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::BW_OR_X32_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      local.get 7
      i32.or
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      local.tee 3
      local.get 8
      i32.or
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 3
      local.get 5
      i32.or
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 3
      local.get 6
      i32.or
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::BW_OR_X64_RuntimeStack*_ (type 2) (param i32) (result i32)
    local.get 0
    local.get 0
    call $RuntimeStack::pop_uint64_t__
    local.get 0
    call $RuntimeStack::pop_uint64_t__
    i64.or
    call $RuntimeStack::push_uint64_t_unsigned_long_long_
    drop
    i32.const 0)
  (func $PLCMethods::BW_XOR_X8_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      local.get 2
      i32.xor
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::BW_XOR_X16_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.xor
      local.tee 1
      i32.const 65280
      i32.and
      i32.const 8
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::BW_XOR_X32_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.xor
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::BW_XOR_X64_RuntimeStack*_ (type 2) (param i32) (result i32)
    local.get 0
    local.get 0
    call $RuntimeStack::pop_uint64_t__
    local.get 0
    call $RuntimeStack::pop_uint64_t__
    i64.xor
    call $RuntimeStack::push_uint64_t_unsigned_long_long_
    drop
    i32.const 0)
  (func $PLCMethods::BW_NOT_X8_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32)
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load offset=8
        local.tee 1
        br_if 0 (;@2;)
        i32.const 255
        local.set 1
        br 1 (;@1;)
      end
      local.get 0
      local.get 1
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const -1
      i32.xor
      local.set 1
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::BW_NOT_X16_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 2
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const -1
      i32.xor
      local.tee 1
      i32.const 65280
      i32.and
      i32.const 8
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::BW_NOT_X32_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=8
      i32.const 4
      i32.add
      local.get 0
      i32.load16_u offset=24
      i32.gt_u
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.const -1
      i32.xor
      local.tee 1
      i32.const 24
      i32.shr_u
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 16
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 8
      i32.shr_u
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
      local.get 0
      local.get 1
      i32.const 255
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::BW_NOT_X64_RuntimeStack*_ (type 2) (param i32) (result i32)
    local.get 0
    local.get 0
    call $RuntimeStack::pop_uint64_t__
    i64.const -1
    i64.xor
    call $RuntimeStack::push_uint64_t_unsigned_long_long_
    drop
    i32.const 0)
  (func $PLCMethods::LOGIC_AND_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      i32.const 0
      i32.ne
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 0
      i32.ne
      local.set 1
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 2
      local.get 1
      i32.and
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::LOGIC_OR_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      local.get 2
      i32.or
      i32.const 255
      i32.and
      i32.const 0
      i32.ne
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::LOGIC_XOR_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      i32.const 0
      i32.ne
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 0
      i32.ne
      local.set 1
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 2
      local.get 1
      i32.xor
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::LOGIC_NOT_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32)
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load offset=8
        local.tee 1
        br_if 0 (;@2;)
        i32.const 1
        local.set 1
        br 1 (;@1;)
      end
      local.get 0
      local.get 1
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.eqz
      local.set 1
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 1
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_EQ_uint16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      i32.eq
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_EQ_uint32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.eq
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_EQ_int16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      i32.eq
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_EQ_int32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.eq
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_EQ_float_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      f32.reinterpret_i32
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      f32.reinterpret_i32
      f32.eq
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_NEQ_uint16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      i32.ne
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_NEQ_uint32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.ne
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_NEQ_int16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      i32.ne
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_NEQ_int32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.ne
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_NEQ_float_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      f32.reinterpret_i32
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      f32.reinterpret_i32
      f32.ne
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_GT_uint16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      i32.gt_u
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_GT_uint32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.gt_u
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_GT_int16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 16
      i32.shl
      i32.const 16
      i32.shr_s
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 16
      i32.shl
      i32.const 16
      i32.shr_s
      i32.gt_s
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_GT_int32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.gt_s
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_GT_float_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      f32.reinterpret_i32
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      f32.reinterpret_i32
      f32.gt
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_LT_uint16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      i32.lt_u
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_LT_uint32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.lt_u
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_LT_int16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 16
      i32.shl
      i32.const 16
      i32.shr_s
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 16
      i32.shl
      i32.const 16
      i32.shr_s
      i32.lt_s
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_LT_int32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.lt_s
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_LT_float_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      f32.reinterpret_i32
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      f32.reinterpret_i32
      f32.lt
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_GTE_uint16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      i32.ge_u
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_GTE_uint32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.ge_u
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_GTE_int16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 16
      i32.shl
      i32.const 16
      i32.shr_s
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 16
      i32.shl
      i32.const 16
      i32.shr_s
      i32.ge_s
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_GTE_int32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.ge_s
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_GTE_float_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      f32.reinterpret_i32
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      f32.reinterpret_i32
      f32.ge
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_LTE_uint16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 65535
      i32.and
      i32.le_u
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_LTE_uint32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.le_u
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_LTE_int16_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const 8
      i32.shl
      local.get 4
      i32.const 255
      i32.and
      i32.or
      i32.const 16
      i32.shl
      i32.const 16
      i32.shr_s
      local.get 1
      i32.const 8
      i32.shl
      local.get 2
      i32.const 255
      i32.and
      i32.or
      i32.const 16
      i32.shl
      i32.const 16
      i32.shr_s
      i32.le_s
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_LTE_int32_t_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      i32.le_s
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $PLCMethods::CMP_LTE_float_RuntimeStack*_ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32)
    i32.const 0
    local.set 1
    i32.const 0
    local.set 2
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 2
      i32.store offset=8
      local.get 0
      i32.load
      local.get 2
      i32.add
      i32.load8_u
      local.set 2
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 3
      i32.const -1
      i32.add
      local.tee 1
      i32.store offset=8
      local.get 0
      i32.load
      local.get 1
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 1
    end
    i32.const 0
    local.set 3
    i32.const 0
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 4
      i32.store offset=8
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 4
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 3
    end
    i32.const 0
    local.set 5
    i32.const 0
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 6
      i32.store offset=8
      local.get 0
      i32.load
      local.get 6
      i32.add
      i32.load8_u
      local.set 6
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 0
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 8
      i32.shl
      local.set 5
    end
    i32.const 0
    local.set 7
    i32.const 0
    local.set 8
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 8
      i32.store offset=8
      local.get 0
      i32.load
      local.get 8
      i32.add
      i32.load8_u
      i32.const 16
      i32.shl
      local.set 8
    end
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      local.tee 9
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      local.get 9
      i32.const -1
      i32.add
      local.tee 7
      i32.store offset=8
      local.get 0
      i32.load
      local.get 7
      i32.add
      i32.load8_u
      i32.const 24
      i32.shl
      local.set 7
    end
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 5
      local.get 6
      i32.or
      local.get 8
      i32.or
      local.get 7
      i32.or
      f32.reinterpret_i32
      local.get 1
      local.get 2
      i32.or
      local.get 4
      i32.or
      local.get 3
      i32.or
      f32.reinterpret_i32
      f32.le
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    i32.const 0)
  (func $printOpcodeAt_unsigned_char_const*__unsigned_short__unsigned_short_ (type 3) (param i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 3
    global.set $__stack_pointer
    i32.const 13
    local.set 4
    block  ;; label = @1
      local.get 2
      local.get 1
      i32.ge_u
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 0
        local.get 2
        i32.add
        i32.load8_u
        local.tee 5
        call $OPCODE_EXISTS_PLCRuntimeInstructionSet_
        br_if 0 (;@2;)
        i32.const 79
        local.set 1
        i32.const 1
        local.set 4
        loop  ;; label = @3
          local.get 1
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 4
          i32.const 1076
          i32.add
          i32.load8_u
          local.set 1
          local.get 4
          i32.const 1
          i32.add
          local.tee 6
          local.set 4
          local.get 6
          i32.const 11
          i32.ne
          br_if 0 (;@3;)
        end
        block  ;; label = @3
          local.get 5
          i32.const 15
          i32.gt_u
          br_if 0 (;@3;)
          i32.const 48
          call $__print
        end
        local.get 3
        local.get 5
        i32.store offset=16
        i32.const 4400
        i32.const 1106
        local.get 3
        i32.const 16
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 4
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 1
          loop  ;; label = @4
            local.get 4
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 1
            i32.load8_u
            local.set 4
            local.get 1
            i32.const 1
            i32.add
            local.set 1
            local.get 4
            br_if 0 (;@4;)
          end
        end
        i32.const 93
        local.set 1
        i32.const 1
        local.set 4
        loop  ;; label = @3
          local.get 1
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 4
          i32.const 2112
          i32.add
          i32.load8_u
          local.set 1
          local.get 4
          i32.const 1
          i32.add
          local.tee 6
          local.set 4
          local.get 6
          i32.const 12
          i32.ne
          br_if 0 (;@3;)
        end
        i32.const 2
        local.set 4
        br 1 (;@1;)
      end
      i32.const 79
      local.set 1
      i32.const 1
      local.set 4
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.const 3240
        i32.add
        i32.load8_u
        local.set 1
        local.get 4
        i32.const 1
        i32.add
        local.tee 6
        local.set 4
        local.get 6
        i32.const 9
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 5
      call $OPCODE_SIZE_PLCRuntimeInstructionSet_
      local.set 7
      block  ;; label = @2
        local.get 5
        call $OPCODE_NAME_PLCRuntimeInstructionSet_
        local.tee 1
        i32.load8_u
        local.tee 4
        i32.eqz
        br_if 0 (;@2;)
        local.get 1
        i32.const 1
        i32.add
        local.set 1
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 1
          i32.load8_u
          local.set 4
          local.get 1
          i32.const 1
          i32.add
          local.set 1
          local.get 4
          br_if 0 (;@3;)
        end
      end
      i32.const 41
      local.set 1
      i32.const 1
      local.set 4
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.const 1509
        i32.add
        i32.load8_u
        local.set 1
        local.get 4
        i32.const 1
        i32.add
        local.tee 6
        local.set 4
        local.get 6
        i32.const 4
        i32.ne
        br_if 0 (;@2;)
      end
      block  ;; label = @2
        local.get 7
        i32.eqz
        br_if 0 (;@2;)
        local.get 7
        i32.const -1
        i32.add
        local.set 5
        i32.const 0
        local.set 6
        loop  ;; label = @3
          block  ;; label = @4
            local.get 0
            local.get 6
            local.get 2
            i32.add
            i32.add
            i32.load8_u
            local.tee 4
            i32.const 15
            i32.gt_u
            br_if 0 (;@4;)
            i32.const 48
            call $__print
          end
          local.get 3
          local.get 4
          i32.store
          i32.const 4400
          i32.const 1106
          local.get 3
          call $sprintf_
          drop
          block  ;; label = @4
            i32.const 0
            i32.load8_u offset=4400
            local.tee 4
            i32.eqz
            br_if 0 (;@4;)
            i32.const 4401
            local.set 1
            loop  ;; label = @5
              local.get 4
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 1
              i32.load8_u
              local.set 4
              local.get 1
              i32.const 1
              i32.add
              local.set 1
              local.get 4
              br_if 0 (;@5;)
            end
          end
          block  ;; label = @4
            local.get 5
            local.get 6
            i32.le_s
            br_if 0 (;@4;)
            i32.const 32
            call $__print
          end
          local.get 6
          i32.const 1
          i32.add
          local.tee 6
          local.get 7
          i32.ne
          br_if 0 (;@3;)
        end
      end
      i32.const 93
      call $__print
      i32.const 0
      local.set 4
    end
    local.get 3
    i32.const 32
    i32.add
    global.set $__stack_pointer
    local.get 4)
  (func $PLCMethods::handle_JMP_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 64
    i32.sub
    local.tee 4
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        block  ;; label = @3
          local.get 3
          i32.load16_u
          local.tee 5
          i32.const 2
          i32.add
          local.get 2
          i32.le_u
          br_if 0 (;@3;)
          i32.const 80
          local.set 6
          i32.const 1
          local.set 7
          loop  ;; label = @4
            local.get 6
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 7
            i32.const 3754
            i32.add
            i32.load8_u
            local.set 6
            local.get 7
            i32.const 1
            i32.add
            local.tee 8
            local.set 7
            local.get 8
            i32.const 39
            i32.ne
            br_if 0 (;@4;)
          end
          local.get 4
          local.get 3
          i32.load16_u
          i32.store offset=16
          i32.const 4400
          i32.const 1466
          local.get 4
          i32.const 16
          i32.add
          call $sprintf_
          drop
          block  ;; label = @4
            i32.const 0
            i32.load8_u offset=4400
            local.tee 7
            i32.eqz
            br_if 0 (;@4;)
            i32.const 4401
            local.set 6
            loop  ;; label = @5
              local.get 7
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 6
              i32.load8_u
              local.set 7
              local.get 6
              i32.const 1
              i32.add
              local.set 6
              local.get 7
              br_if 0 (;@5;)
            end
          end
          i32.const 32
          local.set 6
          i32.const 1
          local.set 7
          loop  ;; label = @4
            local.get 6
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 7
            i32.const 3552
            i32.add
            i32.load8_u
            local.set 6
            local.get 7
            i32.const 1
            i32.add
            local.tee 8
            local.set 7
            local.get 8
            i32.const 4
            i32.ne
            br_if 0 (;@4;)
          end
          local.get 4
          local.get 2
          i32.store
          i32.const 4400
          i32.const 1466
          local.get 4
          call $sprintf_
          drop
          i32.const 0
          i32.load8_u offset=4400
          local.tee 7
          i32.eqz
          br_if 1 (;@2;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 7
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 7
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 7
            br_if 0 (;@4;)
            br 2 (;@2;)
          end
        end
        local.get 3
        local.get 1
        local.get 5
        i32.add
        i32.load16_u align=1
        local.tee 7
        i32.const 8
        i32.shl
        local.get 7
        i32.const 8
        i32.shr_u
        i32.or
        local.tee 6
        i32.store16
        i32.const 0
        local.set 7
        local.get 6
        i32.const 65535
        i32.and
        local.get 2
        i32.lt_u
        br_if 1 (;@1;)
        i32.const 80
        local.set 6
        i32.const 1
        local.set 7
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 7
          i32.const 3754
          i32.add
          i32.load8_u
          local.set 6
          local.get 7
          i32.const 1
          i32.add
          local.tee 8
          local.set 7
          local.get 8
          i32.const 39
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 3
        i32.load16_u
        i32.store offset=48
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 48
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 7
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 7
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 7
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 7
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        local.set 6
        i32.const 1
        local.set 7
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 7
          i32.const 3552
          i32.add
          i32.load8_u
          local.set 6
          local.get 7
          i32.const 1
          i32.add
          local.tee 8
          local.set 7
          local.get 8
          i32.const 4
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 2
        i32.store offset=32
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 32
        i32.add
        call $sprintf_
        drop
        i32.const 0
        i32.load8_u offset=4400
        local.tee 7
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 6
        loop  ;; label = @3
          local.get 7
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 6
          i32.load8_u
          local.set 7
          local.get 6
          i32.const 1
          i32.add
          local.set 6
          local.get 7
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
      local.get 1
      local.get 2
      local.get 5
      call $printOpcodeAt_unsigned_char_const*__unsigned_short__unsigned_short_
      drop
      i32.const 15
      local.set 7
    end
    local.get 4
    i32.const 64
    i32.add
    global.set $__stack_pointer
    local.get 7)
  (func $PLCMethods::handle_JMP_IF_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 64
    i32.sub
    local.tee 4
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 3
        i32.load16_u
        local.tee 5
        i32.const 2
        i32.add
        local.get 2
        i32.le_u
        br_if 0 (;@2;)
        i32.const 80
        local.set 6
        i32.const 1
        local.set 0
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 0
          i32.const 3754
          i32.add
          i32.load8_u
          local.set 6
          local.get 0
          i32.const 1
          i32.add
          local.tee 7
          local.set 0
          local.get 7
          i32.const 39
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 3
        i32.load16_u
        i32.store offset=16
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 16
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 0
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        local.set 6
        i32.const 1
        local.set 0
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 0
          i32.const 3552
          i32.add
          i32.load8_u
          local.set 6
          local.get 0
          i32.const 1
          i32.add
          local.tee 7
          local.set 0
          local.get 7
          i32.const 4
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 2
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 4
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 0
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 1
        local.get 2
        local.get 5
        call $printOpcodeAt_unsigned_char_const*__unsigned_short__unsigned_short_
        drop
        i32.const 15
        local.set 0
        br 1 (;@1;)
      end
      block  ;; label = @2
        local.get 0
        i32.load offset=8
        local.tee 6
        i32.eqz
        br_if 0 (;@2;)
        local.get 0
        local.get 6
        i32.const -1
        i32.add
        local.tee 6
        i32.store offset=8
        local.get 0
        i32.load
        local.get 6
        i32.add
        i32.load8_u
        i32.eqz
        br_if 0 (;@2;)
        local.get 3
        local.get 1
        local.get 5
        i32.add
        i32.load16_u align=1
        local.tee 0
        i32.const 8
        i32.shl
        local.get 0
        i32.const 8
        i32.shr_u
        i32.or
        local.tee 6
        i32.store16
        i32.const 0
        local.set 0
        local.get 6
        i32.const 65535
        i32.and
        local.get 2
        i32.lt_u
        br_if 1 (;@1;)
        i32.const 80
        local.set 6
        i32.const 1
        local.set 0
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 0
          i32.const 3754
          i32.add
          i32.load8_u
          local.set 6
          local.get 0
          i32.const 1
          i32.add
          local.tee 7
          local.set 0
          local.get 7
          i32.const 39
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 3
        i32.load16_u
        i32.store offset=48
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 48
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 0
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        local.set 6
        i32.const 1
        local.set 0
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 0
          i32.const 3552
          i32.add
          i32.load8_u
          local.set 6
          local.get 0
          i32.const 1
          i32.add
          local.tee 7
          local.set 0
          local.get 7
          i32.const 4
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 2
        i32.store offset=32
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 32
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 0
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 1
        local.get 2
        local.get 5
        call $printOpcodeAt_unsigned_char_const*__unsigned_short__unsigned_short_
        drop
        i32.const 15
        local.set 0
        br 1 (;@1;)
      end
      local.get 3
      local.get 5
      i32.const 2
      i32.add
      i32.store16
      i32.const 0
      local.set 0
    end
    local.get 4
    i32.const 64
    i32.add
    global.set $__stack_pointer
    local.get 0)
  (func $PLCMethods::handle_JMP_IF_NOT_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 64
    i32.sub
    local.tee 4
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 3
        i32.load16_u
        local.tee 5
        i32.const 2
        i32.add
        local.get 2
        i32.le_u
        br_if 0 (;@2;)
        i32.const 80
        local.set 6
        i32.const 1
        local.set 0
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 0
          i32.const 3754
          i32.add
          i32.load8_u
          local.set 6
          local.get 0
          i32.const 1
          i32.add
          local.tee 7
          local.set 0
          local.get 7
          i32.const 39
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 3
        i32.load16_u
        i32.store offset=16
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 16
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 0
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        local.set 6
        i32.const 1
        local.set 0
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 0
          i32.const 3552
          i32.add
          i32.load8_u
          local.set 6
          local.get 0
          i32.const 1
          i32.add
          local.tee 7
          local.set 0
          local.get 7
          i32.const 4
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 2
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 4
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 0
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 1
        local.get 2
        local.get 5
        call $printOpcodeAt_unsigned_char_const*__unsigned_short__unsigned_short_
        drop
        i32.const 15
        local.set 0
        br 1 (;@1;)
      end
      block  ;; label = @2
        block  ;; label = @3
          local.get 0
          i32.load offset=8
          local.tee 6
          i32.eqz
          br_if 0 (;@3;)
          local.get 0
          local.get 6
          i32.const -1
          i32.add
          local.tee 6
          i32.store offset=8
          local.get 0
          i32.load
          local.get 6
          i32.add
          i32.load8_u
          br_if 1 (;@2;)
        end
        local.get 3
        local.get 1
        local.get 5
        i32.add
        i32.load16_u align=1
        local.tee 0
        i32.const 8
        i32.shl
        local.get 0
        i32.const 8
        i32.shr_u
        i32.or
        local.tee 6
        i32.store16
        i32.const 0
        local.set 0
        local.get 6
        i32.const 65535
        i32.and
        local.get 2
        i32.lt_u
        br_if 1 (;@1;)
        i32.const 80
        local.set 6
        i32.const 1
        local.set 0
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 0
          i32.const 3754
          i32.add
          i32.load8_u
          local.set 6
          local.get 0
          i32.const 1
          i32.add
          local.tee 7
          local.set 0
          local.get 7
          i32.const 39
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 3
        i32.load16_u
        i32.store offset=48
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 48
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 0
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        local.set 6
        i32.const 1
        local.set 0
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 0
          i32.const 3552
          i32.add
          i32.load8_u
          local.set 6
          local.get 0
          i32.const 1
          i32.add
          local.tee 7
          local.set 0
          local.get 7
          i32.const 4
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 2
        i32.store offset=32
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 32
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 0
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 1
        local.get 2
        local.get 5
        call $printOpcodeAt_unsigned_char_const*__unsigned_short__unsigned_short_
        drop
        i32.const 15
        local.set 0
        br 1 (;@1;)
      end
      local.get 3
      local.get 5
      i32.const 2
      i32.add
      i32.store16
      i32.const 0
      local.set 0
    end
    local.get 4
    i32.const 64
    i32.add
    global.set $__stack_pointer
    local.get 0)
  (func $PLCMethods::handle_CALL_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 96
    i32.sub
    local.tee 4
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        block  ;; label = @3
          local.get 3
          i32.load16_u
          local.tee 5
          i32.const 2
          i32.add
          local.get 2
          i32.le_u
          br_if 0 (;@3;)
          i32.const 80
          local.set 0
          i32.const 1
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.const 3754
            i32.add
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.tee 7
            local.set 6
            local.get 7
            i32.const 39
            i32.ne
            br_if 0 (;@4;)
          end
          local.get 4
          local.get 3
          i32.load16_u
          i32.store offset=16
          i32.const 4400
          i32.const 1466
          local.get 4
          i32.const 16
          i32.add
          call $sprintf_
          drop
          block  ;; label = @4
            i32.const 0
            i32.load8_u offset=4400
            local.tee 6
            i32.eqz
            br_if 0 (;@4;)
            i32.const 4401
            local.set 0
            loop  ;; label = @5
              local.get 6
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 0
              i32.load8_u
              local.set 6
              local.get 0
              i32.const 1
              i32.add
              local.set 0
              local.get 6
              br_if 0 (;@5;)
            end
          end
          i32.const 32
          local.set 0
          i32.const 1
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.const 3552
            i32.add
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.tee 7
            local.set 6
            local.get 7
            i32.const 4
            i32.ne
            br_if 0 (;@4;)
          end
          local.get 4
          local.get 2
          i32.store
          i32.const 4400
          i32.const 1466
          local.get 4
          call $sprintf_
          drop
          i32.const 0
          i32.load8_u offset=4400
          local.tee 6
          i32.eqz
          br_if 1 (;@2;)
          i32.const 4401
          local.set 0
          loop  ;; label = @4
            local.get 6
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 0
            i32.load8_u
            local.set 6
            local.get 0
            i32.const 1
            i32.add
            local.set 0
            local.get 6
            br_if 0 (;@4;)
            br 2 (;@2;)
          end
        end
        local.get 3
        local.get 1
        local.get 5
        i32.add
        i32.load16_u align=1
        local.tee 6
        i32.const 8
        i32.shl
        local.get 6
        i32.const 8
        i32.shr_u
        i32.or
        local.tee 7
        i32.store16
        block  ;; label = @3
          local.get 7
          i32.const 65535
          i32.and
          local.get 2
          i32.lt_u
          br_if 0 (;@3;)
          i32.const 80
          local.set 0
          i32.const 1
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.const 3754
            i32.add
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.tee 7
            local.set 6
            local.get 7
            i32.const 39
            i32.ne
            br_if 0 (;@4;)
          end
          local.get 4
          local.get 3
          i32.load16_u
          i32.store offset=80
          i32.const 4400
          i32.const 1466
          local.get 4
          i32.const 80
          i32.add
          call $sprintf_
          drop
          block  ;; label = @4
            i32.const 0
            i32.load8_u offset=4400
            local.tee 6
            i32.eqz
            br_if 0 (;@4;)
            i32.const 4401
            local.set 0
            loop  ;; label = @5
              local.get 6
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 0
              i32.load8_u
              local.set 6
              local.get 0
              i32.const 1
              i32.add
              local.set 0
              local.get 6
              br_if 0 (;@5;)
            end
          end
          i32.const 32
          local.set 0
          i32.const 1
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.const 3552
            i32.add
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.tee 7
            local.set 6
            local.get 7
            i32.const 4
            i32.ne
            br_if 0 (;@4;)
          end
          local.get 4
          local.get 2
          i32.store offset=64
          i32.const 4400
          i32.const 1466
          local.get 4
          i32.const 64
          i32.add
          call $sprintf_
          drop
          i32.const 0
          i32.load8_u offset=4400
          local.tee 6
          i32.eqz
          br_if 1 (;@2;)
          i32.const 4401
          local.set 0
          loop  ;; label = @4
            local.get 6
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 0
            i32.load8_u
            local.set 6
            local.get 0
            i32.const 1
            i32.add
            local.set 0
            local.get 6
            br_if 0 (;@4;)
            br 2 (;@2;)
          end
        end
        i32.const 4
        local.set 6
        block  ;; label = @3
          local.get 0
          i32.load16_u offset=26
          local.tee 8
          local.get 0
          i32.const 20
          i32.add
          i32.load
          local.tee 9
          i32.const 65535
          i32.and
          i32.le_u
          br_if 0 (;@3;)
          local.get 0
          i32.const 12
          i32.add
          local.get 7
          i32.const 65535
          i32.and
          call $Stack<unsigned_short>::push_unsigned_short_
          i32.const 0
          local.set 6
        end
        local.get 8
        local.get 9
        i32.const 65535
        i32.and
        i32.le_u
        br_if 1 (;@1;)
        i32.const 0
        local.set 6
        local.get 3
        i32.load16_u
        local.get 2
        i32.lt_u
        br_if 1 (;@1;)
        i32.const 80
        local.set 0
        i32.const 1
        local.set 6
        loop  ;; label = @3
          local.get 0
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 6
          i32.const 3754
          i32.add
          i32.load8_u
          local.set 0
          local.get 6
          i32.const 1
          i32.add
          local.tee 7
          local.set 6
          local.get 7
          i32.const 39
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 3
        i32.load16_u
        i32.store offset=48
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 48
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 6
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 0
          loop  ;; label = @4
            local.get 6
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 0
            i32.load8_u
            local.set 6
            local.get 0
            i32.const 1
            i32.add
            local.set 0
            local.get 6
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        local.set 0
        i32.const 1
        local.set 6
        loop  ;; label = @3
          local.get 0
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 6
          i32.const 3552
          i32.add
          i32.load8_u
          local.set 0
          local.get 6
          i32.const 1
          i32.add
          local.tee 7
          local.set 6
          local.get 7
          i32.const 4
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 2
        i32.store offset=32
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 32
        i32.add
        call $sprintf_
        drop
        i32.const 0
        i32.load8_u offset=4400
        local.tee 6
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 0
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 0
          i32.load8_u
          local.set 6
          local.get 0
          i32.const 1
          i32.add
          local.set 0
          local.get 6
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
      local.get 1
      local.get 2
      local.get 5
      call $printOpcodeAt_unsigned_char_const*__unsigned_short__unsigned_short_
      drop
      i32.const 15
      local.set 6
    end
    local.get 4
    i32.const 96
    i32.add
    global.set $__stack_pointer
    local.get 6)
  (func $Stack<unsigned_short>::push_unsigned_short_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 48
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load offset=8
        local.tee 3
        local.get 0
        i32.load offset=4
        i32.lt_s
        br_if 0 (;@2;)
        i32.const 83
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3256
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 16
          i32.ne
          br_if 0 (;@3;)
        end
        i32.const 10
        call $__print
        i32.const 84
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3526
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 16
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 1
        i32.store offset=32
        i32.const 4400
        i32.const 1106
        local.get 2
        i32.const 32
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 4
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 4
            i32.load8_u
            local.set 3
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3310
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 20
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load offset=8
        i32.store offset=16
        i32.const 4400
        i32.const 1466
        local.get 2
        i32.const 16
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 4
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 4
            i32.load8_u
            local.set 3
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        i32.const 77
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3556
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 18
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load offset=4
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 2
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 4
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 4
            i32.load8_u
            local.set 3
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        br 1 (;@1;)
      end
      local.get 0
      local.get 3
      i32.const 1
      i32.add
      i32.store offset=8
      local.get 0
      i32.load
      local.get 3
      i32.const 1
      i32.shl
      i32.add
      local.get 1
      i32.store16
    end
    local.get 2
    i32.const 48
    i32.add
    global.set $__stack_pointer)
  (func $PLCMethods::handle_CALL_IF_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 96
    i32.sub
    local.tee 4
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 3
        i32.load16_u
        local.tee 5
        i32.const 2
        i32.add
        local.get 2
        i32.le_u
        br_if 0 (;@2;)
        i32.const 80
        local.set 6
        i32.const 1
        local.set 0
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 0
          i32.const 3754
          i32.add
          i32.load8_u
          local.set 6
          local.get 0
          i32.const 1
          i32.add
          local.tee 7
          local.set 0
          local.get 7
          i32.const 39
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 3
        i32.load16_u
        i32.store offset=16
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 16
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 0
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        local.set 6
        i32.const 1
        local.set 0
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 0
          i32.const 3552
          i32.add
          i32.load8_u
          local.set 6
          local.get 0
          i32.const 1
          i32.add
          local.tee 7
          local.set 0
          local.get 7
          i32.const 4
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 2
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 4
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 0
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 1
        local.get 2
        local.get 5
        call $printOpcodeAt_unsigned_char_const*__unsigned_short__unsigned_short_
        drop
        i32.const 15
        local.set 6
        br 1 (;@1;)
      end
      block  ;; label = @2
        block  ;; label = @3
          local.get 0
          i32.load offset=8
          local.tee 6
          i32.eqz
          br_if 0 (;@3;)
          local.get 0
          local.get 6
          i32.const -1
          i32.add
          local.tee 6
          i32.store offset=8
          local.get 0
          i32.load
          local.get 6
          i32.add
          i32.load8_u
          i32.eqz
          br_if 0 (;@3;)
          local.get 3
          local.get 1
          local.get 5
          i32.add
          i32.load16_u align=1
          local.tee 6
          i32.const 8
          i32.shl
          local.get 6
          i32.const 8
          i32.shr_u
          i32.or
          local.tee 7
          i32.store16
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  local.get 7
                  i32.const 65535
                  i32.and
                  local.tee 6
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 6
                  local.get 2
                  i32.lt_u
                  br_if 1 (;@6;)
                end
                i32.const 80
                local.set 6
                i32.const 1
                local.set 0
                loop  ;; label = @7
                  local.get 6
                  i32.const 24
                  i32.shl
                  i32.const 24
                  i32.shr_s
                  call $__print
                  local.get 0
                  i32.const 3754
                  i32.add
                  i32.load8_u
                  local.set 6
                  local.get 0
                  i32.const 1
                  i32.add
                  local.tee 7
                  local.set 0
                  local.get 7
                  i32.const 39
                  i32.ne
                  br_if 0 (;@7;)
                end
                local.get 4
                local.get 3
                i32.load16_u
                i32.store offset=80
                i32.const 4400
                i32.const 1466
                local.get 4
                i32.const 80
                i32.add
                call $sprintf_
                drop
                block  ;; label = @7
                  i32.const 0
                  i32.load8_u offset=4400
                  local.tee 0
                  i32.eqz
                  br_if 0 (;@7;)
                  i32.const 4401
                  local.set 6
                  loop  ;; label = @8
                    local.get 0
                    i32.const 24
                    i32.shl
                    i32.const 24
                    i32.shr_s
                    call $__print
                    local.get 6
                    i32.load8_u
                    local.set 0
                    local.get 6
                    i32.const 1
                    i32.add
                    local.set 6
                    local.get 0
                    br_if 0 (;@8;)
                  end
                end
                i32.const 32
                local.set 6
                i32.const 1
                local.set 0
                loop  ;; label = @7
                  local.get 6
                  i32.const 24
                  i32.shl
                  i32.const 24
                  i32.shr_s
                  call $__print
                  local.get 0
                  i32.const 3552
                  i32.add
                  i32.load8_u
                  local.set 6
                  local.get 0
                  i32.const 1
                  i32.add
                  local.tee 7
                  local.set 0
                  local.get 7
                  i32.const 4
                  i32.ne
                  br_if 0 (;@7;)
                end
                local.get 4
                local.get 2
                i32.store offset=64
                i32.const 4400
                i32.const 1466
                local.get 4
                i32.const 64
                i32.add
                call $sprintf_
                drop
                i32.const 0
                i32.load8_u offset=4400
                local.tee 0
                i32.eqz
                br_if 1 (;@5;)
                i32.const 4401
                local.set 6
                loop  ;; label = @7
                  local.get 0
                  i32.const 24
                  i32.shl
                  i32.const 24
                  i32.shr_s
                  call $__print
                  local.get 6
                  i32.load8_u
                  local.set 0
                  local.get 6
                  i32.const 1
                  i32.add
                  local.set 6
                  local.get 0
                  br_if 0 (;@7;)
                  br 2 (;@5;)
                end
              end
              i32.const 4
              local.set 6
              block  ;; label = @6
                local.get 0
                i32.load16_u offset=26
                local.get 0
                i32.const 20
                i32.add
                i32.load
                i32.const 65535
                i32.and
                i32.le_u
                local.tee 8
                br_if 0 (;@6;)
                local.get 0
                i32.const 12
                i32.add
                local.get 7
                i32.const 65535
                i32.and
                call $Stack<unsigned_short>::push_unsigned_short_
                i32.const 0
                local.set 6
              end
              i32.const 0
              local.set 0
              local.get 8
              br_if 1 (;@4;)
              block  ;; label = @6
                local.get 3
                i32.load16_u
                local.get 2
                i32.ge_u
                br_if 0 (;@6;)
                i32.const 1
                br_if 4 (;@2;)
                br 5 (;@1;)
              end
              i32.const 80
              local.set 6
              i32.const 1
              local.set 0
              loop  ;; label = @6
                local.get 6
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 0
                i32.const 3754
                i32.add
                i32.load8_u
                local.set 6
                local.get 0
                i32.const 1
                i32.add
                local.tee 7
                local.set 0
                local.get 7
                i32.const 39
                i32.ne
                br_if 0 (;@6;)
              end
              local.get 4
              local.get 3
              i32.load16_u
              i32.store offset=48
              i32.const 4400
              i32.const 1466
              local.get 4
              i32.const 48
              i32.add
              call $sprintf_
              drop
              block  ;; label = @6
                i32.const 0
                i32.load8_u offset=4400
                local.tee 0
                i32.eqz
                br_if 0 (;@6;)
                i32.const 4401
                local.set 6
                loop  ;; label = @7
                  local.get 0
                  i32.const 24
                  i32.shl
                  i32.const 24
                  i32.shr_s
                  call $__print
                  local.get 6
                  i32.load8_u
                  local.set 0
                  local.get 6
                  i32.const 1
                  i32.add
                  local.set 6
                  local.get 0
                  br_if 0 (;@7;)
                end
              end
              i32.const 32
              local.set 6
              i32.const 1
              local.set 0
              loop  ;; label = @6
                local.get 6
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 0
                i32.const 3552
                i32.add
                i32.load8_u
                local.set 6
                local.get 0
                i32.const 1
                i32.add
                local.tee 7
                local.set 0
                local.get 7
                i32.const 4
                i32.ne
                br_if 0 (;@6;)
              end
              local.get 4
              local.get 2
              i32.store offset=32
              i32.const 4400
              i32.const 1466
              local.get 4
              i32.const 32
              i32.add
              call $sprintf_
              drop
              i32.const 0
              i32.load8_u offset=4400
              local.tee 0
              i32.eqz
              br_if 0 (;@5;)
              i32.const 4401
              local.set 6
              loop  ;; label = @6
                local.get 0
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 6
                i32.load8_u
                local.set 0
                local.get 6
                i32.const 1
                i32.add
                local.set 6
                local.get 0
                br_if 0 (;@6;)
              end
            end
            i32.const 10
            call $__print
            local.get 1
            local.get 2
            local.get 5
            call $printOpcodeAt_unsigned_char_const*__unsigned_short__unsigned_short_
            drop
            i32.const 15
            local.set 6
            i32.const 0
            local.set 0
          end
          local.get 0
          br_if 1 (;@2;)
          br 2 (;@1;)
        end
        local.get 3
        local.get 5
        i32.const 2
        i32.add
        i32.store16
      end
      i32.const 0
      local.set 6
    end
    local.get 4
    i32.const 96
    i32.add
    global.set $__stack_pointer
    local.get 6)
  (func $PLCMethods::handle_CALL_IF_NOT_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 96
    i32.sub
    local.tee 4
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 3
        i32.load16_u
        local.tee 5
        i32.const 2
        i32.add
        local.get 2
        i32.le_u
        br_if 0 (;@2;)
        i32.const 80
        local.set 6
        i32.const 1
        local.set 0
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 0
          i32.const 3754
          i32.add
          i32.load8_u
          local.set 6
          local.get 0
          i32.const 1
          i32.add
          local.tee 7
          local.set 0
          local.get 7
          i32.const 39
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 3
        i32.load16_u
        i32.store offset=16
        i32.const 4400
        i32.const 1466
        local.get 4
        i32.const 16
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 0
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        local.set 6
        i32.const 1
        local.set 0
        loop  ;; label = @3
          local.get 6
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 0
          i32.const 3552
          i32.add
          i32.load8_u
          local.set 6
          local.get 0
          i32.const 1
          i32.add
          local.tee 7
          local.set 0
          local.get 7
          i32.const 4
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 2
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 4
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 6
          loop  ;; label = @4
            local.get 0
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 6
            i32.load8_u
            local.set 0
            local.get 6
            i32.const 1
            i32.add
            local.set 6
            local.get 0
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 1
        local.get 2
        local.get 5
        call $printOpcodeAt_unsigned_char_const*__unsigned_short__unsigned_short_
        drop
        i32.const 15
        local.set 6
        br 1 (;@1;)
      end
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            local.get 0
            i32.load offset=8
            local.tee 6
            i32.eqz
            br_if 0 (;@4;)
            local.get 0
            local.get 6
            i32.const -1
            i32.add
            local.tee 6
            i32.store offset=8
            local.get 0
            i32.load
            local.get 6
            i32.add
            i32.load8_u
            br_if 1 (;@3;)
          end
          local.get 3
          local.get 1
          local.get 5
          i32.add
          i32.load16_u align=1
          local.tee 6
          i32.const 8
          i32.shl
          local.get 6
          i32.const 8
          i32.shr_u
          i32.or
          local.tee 7
          i32.store16
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  local.get 7
                  i32.const 65535
                  i32.and
                  local.tee 6
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 6
                  local.get 2
                  i32.lt_u
                  br_if 1 (;@6;)
                end
                i32.const 80
                local.set 6
                i32.const 1
                local.set 0
                loop  ;; label = @7
                  local.get 6
                  i32.const 24
                  i32.shl
                  i32.const 24
                  i32.shr_s
                  call $__print
                  local.get 0
                  i32.const 3754
                  i32.add
                  i32.load8_u
                  local.set 6
                  local.get 0
                  i32.const 1
                  i32.add
                  local.tee 7
                  local.set 0
                  local.get 7
                  i32.const 39
                  i32.ne
                  br_if 0 (;@7;)
                end
                local.get 4
                local.get 3
                i32.load16_u
                i32.store offset=80
                i32.const 4400
                i32.const 1466
                local.get 4
                i32.const 80
                i32.add
                call $sprintf_
                drop
                block  ;; label = @7
                  i32.const 0
                  i32.load8_u offset=4400
                  local.tee 0
                  i32.eqz
                  br_if 0 (;@7;)
                  i32.const 4401
                  local.set 6
                  loop  ;; label = @8
                    local.get 0
                    i32.const 24
                    i32.shl
                    i32.const 24
                    i32.shr_s
                    call $__print
                    local.get 6
                    i32.load8_u
                    local.set 0
                    local.get 6
                    i32.const 1
                    i32.add
                    local.set 6
                    local.get 0
                    br_if 0 (;@8;)
                  end
                end
                i32.const 32
                local.set 6
                i32.const 1
                local.set 0
                loop  ;; label = @7
                  local.get 6
                  i32.const 24
                  i32.shl
                  i32.const 24
                  i32.shr_s
                  call $__print
                  local.get 0
                  i32.const 3552
                  i32.add
                  i32.load8_u
                  local.set 6
                  local.get 0
                  i32.const 1
                  i32.add
                  local.tee 7
                  local.set 0
                  local.get 7
                  i32.const 4
                  i32.ne
                  br_if 0 (;@7;)
                end
                local.get 4
                local.get 2
                i32.store offset=64
                i32.const 4400
                i32.const 1466
                local.get 4
                i32.const 64
                i32.add
                call $sprintf_
                drop
                i32.const 0
                i32.load8_u offset=4400
                local.tee 0
                i32.eqz
                br_if 1 (;@5;)
                i32.const 4401
                local.set 6
                loop  ;; label = @7
                  local.get 0
                  i32.const 24
                  i32.shl
                  i32.const 24
                  i32.shr_s
                  call $__print
                  local.get 6
                  i32.load8_u
                  local.set 0
                  local.get 6
                  i32.const 1
                  i32.add
                  local.set 6
                  local.get 0
                  br_if 0 (;@7;)
                  br 2 (;@5;)
                end
              end
              i32.const 4
              local.set 6
              block  ;; label = @6
                local.get 0
                i32.load16_u offset=26
                local.get 0
                i32.const 20
                i32.add
                i32.load
                i32.const 65535
                i32.and
                i32.le_u
                local.tee 8
                br_if 0 (;@6;)
                local.get 0
                i32.const 12
                i32.add
                local.get 7
                i32.const 65535
                i32.and
                call $Stack<unsigned_short>::push_unsigned_short_
                i32.const 0
                local.set 6
              end
              i32.const 0
              local.set 0
              local.get 8
              br_if 1 (;@4;)
              block  ;; label = @6
                local.get 3
                i32.load16_u
                local.get 2
                i32.ge_u
                br_if 0 (;@6;)
                i32.const 1
                br_if 4 (;@2;)
                br 5 (;@1;)
              end
              i32.const 80
              local.set 6
              i32.const 1
              local.set 0
              loop  ;; label = @6
                local.get 6
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 0
                i32.const 3754
                i32.add
                i32.load8_u
                local.set 6
                local.get 0
                i32.const 1
                i32.add
                local.tee 7
                local.set 0
                local.get 7
                i32.const 39
                i32.ne
                br_if 0 (;@6;)
              end
              local.get 4
              local.get 3
              i32.load16_u
              i32.store offset=48
              i32.const 4400
              i32.const 1466
              local.get 4
              i32.const 48
              i32.add
              call $sprintf_
              drop
              block  ;; label = @6
                i32.const 0
                i32.load8_u offset=4400
                local.tee 0
                i32.eqz
                br_if 0 (;@6;)
                i32.const 4401
                local.set 6
                loop  ;; label = @7
                  local.get 0
                  i32.const 24
                  i32.shl
                  i32.const 24
                  i32.shr_s
                  call $__print
                  local.get 6
                  i32.load8_u
                  local.set 0
                  local.get 6
                  i32.const 1
                  i32.add
                  local.set 6
                  local.get 0
                  br_if 0 (;@7;)
                end
              end
              i32.const 32
              local.set 6
              i32.const 1
              local.set 0
              loop  ;; label = @6
                local.get 6
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 0
                i32.const 3552
                i32.add
                i32.load8_u
                local.set 6
                local.get 0
                i32.const 1
                i32.add
                local.tee 7
                local.set 0
                local.get 7
                i32.const 4
                i32.ne
                br_if 0 (;@6;)
              end
              local.get 4
              local.get 2
              i32.store offset=32
              i32.const 4400
              i32.const 1466
              local.get 4
              i32.const 32
              i32.add
              call $sprintf_
              drop
              i32.const 0
              i32.load8_u offset=4400
              local.tee 0
              i32.eqz
              br_if 0 (;@5;)
              i32.const 4401
              local.set 6
              loop  ;; label = @6
                local.get 0
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 6
                i32.load8_u
                local.set 0
                local.get 6
                i32.const 1
                i32.add
                local.set 6
                local.get 0
                br_if 0 (;@6;)
              end
            end
            i32.const 10
            call $__print
            local.get 1
            local.get 2
            local.get 5
            call $printOpcodeAt_unsigned_char_const*__unsigned_short__unsigned_short_
            drop
            i32.const 15
            local.set 6
            i32.const 0
            local.set 0
          end
          local.get 0
          br_if 1 (;@2;)
          br 2 (;@1;)
        end
        local.get 3
        local.get 5
        i32.const 2
        i32.add
        i32.store16
      end
      i32.const 0
      local.set 6
    end
    local.get 4
    i32.const 96
    i32.add
    global.set $__stack_pointer
    local.get 6)
  (func $PLCMethods::handle_RET_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 4
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        local.tee 5
        i32.load16_u
        br_if 0 (;@2;)
        i32.const 9
        local.set 0
        br 1 (;@1;)
      end
      local.get 3
      i32.load16_u
      local.set 6
      block  ;; label = @2
        block  ;; label = @3
          local.get 5
          i32.load
          local.tee 5
          i32.const 65535
          i32.and
          br_if 0 (;@3;)
          i32.const 5
          local.set 5
          br 1 (;@2;)
        end
        block  ;; label = @3
          local.get 5
          br_if 0 (;@3;)
          i32.const 0
          local.set 5
          br 1 (;@2;)
        end
        local.get 0
        local.get 5
        i32.const -1
        i32.add
        local.tee 5
        i32.store offset=20
        local.get 0
        i32.load offset=12
        local.get 5
        i32.const 1
        i32.shl
        i32.add
        i32.load16_u
        local.set 5
      end
      local.get 3
      local.get 5
      i32.store16
      i32.const 0
      local.set 0
      local.get 5
      i32.const 65535
      i32.and
      local.get 2
      i32.lt_u
      br_if 0 (;@1;)
      i32.const 80
      local.set 5
      i32.const 1
      local.set 0
      loop  ;; label = @2
        local.get 5
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 0
        i32.const 3754
        i32.add
        i32.load8_u
        local.set 5
        local.get 0
        i32.const 1
        i32.add
        local.tee 7
        local.set 0
        local.get 7
        i32.const 39
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 4
      local.get 3
      i32.load16_u
      i32.store offset=16
      i32.const 4400
      i32.const 1466
      local.get 4
      i32.const 16
      i32.add
      call $sprintf_
      drop
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 0
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 5
        loop  ;; label = @3
          local.get 0
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 5
          i32.load8_u
          local.set 0
          local.get 5
          i32.const 1
          i32.add
          local.set 5
          local.get 0
          br_if 0 (;@3;)
        end
      end
      i32.const 32
      local.set 5
      i32.const 1
      local.set 0
      loop  ;; label = @2
        local.get 5
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 0
        i32.const 3552
        i32.add
        i32.load8_u
        local.set 5
        local.get 0
        i32.const 1
        i32.add
        local.tee 7
        local.set 0
        local.get 7
        i32.const 4
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 4
      local.get 2
      i32.store
      i32.const 4400
      i32.const 1466
      local.get 4
      call $sprintf_
      drop
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 0
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 5
        loop  ;; label = @3
          local.get 0
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 5
          i32.load8_u
          local.set 0
          local.get 5
          i32.const 1
          i32.add
          local.set 5
          local.get 0
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
      local.get 1
      local.get 2
      local.get 6
      i32.const 65535
      i32.and
      call $printOpcodeAt_unsigned_char_const*__unsigned_short__unsigned_short_
      drop
      i32.const 15
      local.set 0
    end
    local.get 4
    i32.const 32
    i32.add
    global.set $__stack_pointer
    local.get 0)
  (func $PLCMethods::handle_ADD_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32)
    i32.const 15
    local.set 4
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 5
      local.get 2
      i32.ge_u
      br_if 0 (;@1;)
      local.get 3
      local.get 5
      i32.const 1
      i32.add
      i32.store16
      i32.const 3
      local.set 4
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          block  ;; label = @12
                            local.get 1
                            local.get 5
                            i32.add
                            i32.load8_u
                            i32.const -1
                            i32.add
                            br_table 0 (;@12;) 0 (;@12;) 1 (;@11;) 2 (;@10;) 3 (;@9;) 4 (;@8;) 5 (;@7;) 6 (;@6;) 7 (;@5;) 8 (;@4;) 9 (;@3;) 11 (;@1;)
                          end
                          i32.const 0
                          local.set 3
                          i32.const 0
                          local.set 2
                          block  ;; label = @12
                            local.get 0
                            i32.load offset=8
                            local.tee 4
                            i32.eqz
                            br_if 0 (;@12;)
                            local.get 0
                            local.get 4
                            i32.const -1
                            i32.add
                            local.tee 4
                            i32.store offset=8
                            local.get 0
                            i32.load
                            local.get 4
                            i32.add
                            i32.load8_u
                            local.set 2
                          end
                          block  ;; label = @12
                            local.get 0
                            i32.load offset=8
                            local.tee 4
                            i32.eqz
                            br_if 0 (;@12;)
                            local.get 0
                            local.get 4
                            i32.const -1
                            i32.add
                            local.tee 4
                            i32.store offset=8
                            local.get 0
                            i32.load
                            local.get 4
                            i32.add
                            i32.load8_u
                            local.set 3
                          end
                          i32.const 0
                          local.set 4
                          local.get 0
                          i32.load16_u offset=24
                          local.get 0
                          i32.load16_u offset=8
                          i32.le_u
                          br_if 10 (;@1;)
                          local.get 0
                          local.get 3
                          local.get 2
                          i32.add
                          i32.const 255
                          i32.and
                          call $Stack<unsigned_char>::push_unsigned_char_
                          i32.const 0
                          return
                        end
                        local.get 0
                        call $PLCMethods::ADD_uint16_t_RuntimeStack*_
                        drop
                        br 8 (;@2;)
                      end
                      local.get 0
                      call $PLCMethods::ADD_uint32_t_RuntimeStack*_
                      drop
                      br 7 (;@2;)
                    end
                    local.get 0
                    local.get 0
                    call $RuntimeStack::pop_uint64_t__
                    local.get 0
                    call $RuntimeStack::pop_uint64_t__
                    i64.add
                    call $RuntimeStack::push_uint64_t_unsigned_long_long_
                    drop
                    br 6 (;@2;)
                  end
                  i32.const 0
                  local.set 3
                  i32.const 0
                  local.set 2
                  block  ;; label = @8
                    local.get 0
                    i32.load offset=8
                    local.tee 4
                    i32.eqz
                    br_if 0 (;@8;)
                    local.get 0
                    local.get 4
                    i32.const -1
                    i32.add
                    local.tee 4
                    i32.store offset=8
                    local.get 0
                    i32.load
                    local.get 4
                    i32.add
                    i32.load8_u
                    local.set 2
                  end
                  block  ;; label = @8
                    local.get 0
                    i32.load offset=8
                    local.tee 4
                    i32.eqz
                    br_if 0 (;@8;)
                    local.get 0
                    local.get 4
                    i32.const -1
                    i32.add
                    local.tee 4
                    i32.store offset=8
                    local.get 0
                    i32.load
                    local.get 4
                    i32.add
                    i32.load8_u
                    local.set 3
                  end
                  i32.const 0
                  local.set 4
                  local.get 0
                  i32.load16_u offset=24
                  local.get 0
                  i32.load16_u offset=8
                  i32.le_u
                  br_if 6 (;@1;)
                  local.get 0
                  local.get 3
                  local.get 2
                  i32.add
                  i32.const 255
                  i32.and
                  call $Stack<unsigned_char>::push_unsigned_char_
                  i32.const 0
                  return
                end
                local.get 0
                call $PLCMethods::ADD_int16_t_RuntimeStack*_
                drop
                br 4 (;@2;)
              end
              local.get 0
              call $PLCMethods::ADD_int32_t_RuntimeStack*_
              drop
              br 3 (;@2;)
            end
            local.get 0
            local.get 0
            call $RuntimeStack::pop_uint64_t__
            local.get 0
            call $RuntimeStack::pop_uint64_t__
            i64.add
            call $RuntimeStack::push_uint64_t_unsigned_long_long_
            drop
            br 2 (;@2;)
          end
          local.get 0
          call $PLCMethods::ADD_float_RuntimeStack*_
          drop
          br 1 (;@2;)
        end
        local.get 0
        local.get 0
        call $RuntimeStack::pop_uint64_t__
        f64.reinterpret_i64
        local.get 0
        call $RuntimeStack::pop_uint64_t__
        f64.reinterpret_i64
        f64.add
        i64.reinterpret_f64
        call $RuntimeStack::push_uint64_t_unsigned_long_long_
        drop
      end
      i32.const 0
      local.set 4
    end
    local.get 4)
  (func $PLCMethods::handle_SUB_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i64)
    i32.const 15
    local.set 4
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 5
      local.get 2
      i32.ge_u
      br_if 0 (;@1;)
      local.get 3
      local.get 5
      i32.const 1
      i32.add
      i32.store16
      i32.const 3
      local.set 4
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          block  ;; label = @12
                            local.get 1
                            local.get 5
                            i32.add
                            i32.load8_u
                            i32.const -1
                            i32.add
                            br_table 0 (;@12;) 0 (;@12;) 1 (;@11;) 2 (;@10;) 3 (;@9;) 4 (;@8;) 5 (;@7;) 6 (;@6;) 7 (;@5;) 8 (;@4;) 9 (;@3;) 11 (;@1;)
                          end
                          i32.const 0
                          local.set 3
                          i32.const 0
                          local.set 2
                          block  ;; label = @12
                            local.get 0
                            i32.load offset=8
                            local.tee 4
                            i32.eqz
                            br_if 0 (;@12;)
                            local.get 0
                            local.get 4
                            i32.const -1
                            i32.add
                            local.tee 4
                            i32.store offset=8
                            local.get 0
                            i32.load
                            local.get 4
                            i32.add
                            i32.load8_u
                            local.set 2
                          end
                          block  ;; label = @12
                            local.get 0
                            i32.load offset=8
                            local.tee 4
                            i32.eqz
                            br_if 0 (;@12;)
                            local.get 0
                            local.get 4
                            i32.const -1
                            i32.add
                            local.tee 4
                            i32.store offset=8
                            local.get 0
                            i32.load
                            local.get 4
                            i32.add
                            i32.load8_u
                            local.set 3
                          end
                          i32.const 0
                          local.set 4
                          local.get 0
                          i32.load16_u offset=24
                          local.get 0
                          i32.load16_u offset=8
                          i32.le_u
                          br_if 10 (;@1;)
                          local.get 0
                          local.get 3
                          local.get 2
                          i32.sub
                          i32.const 255
                          i32.and
                          call $Stack<unsigned_char>::push_unsigned_char_
                          i32.const 0
                          return
                        end
                        local.get 0
                        call $PLCMethods::SUB_uint16_t_RuntimeStack*_
                        drop
                        br 8 (;@2;)
                      end
                      local.get 0
                      call $PLCMethods::SUB_uint32_t_RuntimeStack*_
                      drop
                      br 7 (;@2;)
                    end
                    local.get 0
                    call $RuntimeStack::pop_uint64_t__
                    local.set 6
                    local.get 0
                    local.get 0
                    call $RuntimeStack::pop_uint64_t__
                    local.get 6
                    i64.sub
                    call $RuntimeStack::push_uint64_t_unsigned_long_long_
                    drop
                    br 6 (;@2;)
                  end
                  i32.const 0
                  local.set 3
                  i32.const 0
                  local.set 2
                  block  ;; label = @8
                    local.get 0
                    i32.load offset=8
                    local.tee 4
                    i32.eqz
                    br_if 0 (;@8;)
                    local.get 0
                    local.get 4
                    i32.const -1
                    i32.add
                    local.tee 4
                    i32.store offset=8
                    local.get 0
                    i32.load
                    local.get 4
                    i32.add
                    i32.load8_u
                    local.set 2
                  end
                  block  ;; label = @8
                    local.get 0
                    i32.load offset=8
                    local.tee 4
                    i32.eqz
                    br_if 0 (;@8;)
                    local.get 0
                    local.get 4
                    i32.const -1
                    i32.add
                    local.tee 4
                    i32.store offset=8
                    local.get 0
                    i32.load
                    local.get 4
                    i32.add
                    i32.load8_u
                    local.set 3
                  end
                  i32.const 0
                  local.set 4
                  local.get 0
                  i32.load16_u offset=24
                  local.get 0
                  i32.load16_u offset=8
                  i32.le_u
                  br_if 6 (;@1;)
                  local.get 0
                  local.get 3
                  local.get 2
                  i32.sub
                  i32.const 255
                  i32.and
                  call $Stack<unsigned_char>::push_unsigned_char_
                  i32.const 0
                  return
                end
                local.get 0
                call $PLCMethods::SUB_int16_t_RuntimeStack*_
                drop
                br 4 (;@2;)
              end
              local.get 0
              call $PLCMethods::SUB_int32_t_RuntimeStack*_
              drop
              br 3 (;@2;)
            end
            local.get 0
            call $RuntimeStack::pop_uint64_t__
            local.set 6
            local.get 0
            local.get 0
            call $RuntimeStack::pop_uint64_t__
            local.get 6
            i64.sub
            call $RuntimeStack::push_uint64_t_unsigned_long_long_
            drop
            br 2 (;@2;)
          end
          local.get 0
          call $PLCMethods::SUB_float_RuntimeStack*_
          drop
          br 1 (;@2;)
        end
        local.get 0
        call $RuntimeStack::pop_uint64_t__
        local.set 6
        local.get 0
        local.get 0
        call $RuntimeStack::pop_uint64_t__
        f64.reinterpret_i64
        local.get 6
        f64.reinterpret_i64
        f64.sub
        i64.reinterpret_f64
        call $RuntimeStack::push_uint64_t_unsigned_long_long_
        drop
      end
      i32.const 0
      local.set 4
    end
    local.get 4)
  (func $PLCMethods::handle_MUL_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32)
    i32.const 15
    local.set 4
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 5
      local.get 2
      i32.ge_u
      br_if 0 (;@1;)
      local.get 3
      local.get 5
      i32.const 1
      i32.add
      i32.store16
      i32.const 3
      local.set 4
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          block  ;; label = @12
                            local.get 1
                            local.get 5
                            i32.add
                            i32.load8_u
                            i32.const -1
                            i32.add
                            br_table 0 (;@12;) 0 (;@12;) 1 (;@11;) 2 (;@10;) 3 (;@9;) 4 (;@8;) 5 (;@7;) 6 (;@6;) 7 (;@5;) 8 (;@4;) 9 (;@3;) 11 (;@1;)
                          end
                          i32.const 0
                          local.set 3
                          i32.const 0
                          local.set 2
                          block  ;; label = @12
                            local.get 0
                            i32.load offset=8
                            local.tee 4
                            i32.eqz
                            br_if 0 (;@12;)
                            local.get 0
                            local.get 4
                            i32.const -1
                            i32.add
                            local.tee 4
                            i32.store offset=8
                            local.get 0
                            i32.load
                            local.get 4
                            i32.add
                            i32.load8_u
                            local.set 2
                          end
                          block  ;; label = @12
                            local.get 0
                            i32.load offset=8
                            local.tee 4
                            i32.eqz
                            br_if 0 (;@12;)
                            local.get 0
                            local.get 4
                            i32.const -1
                            i32.add
                            local.tee 4
                            i32.store offset=8
                            local.get 0
                            i32.load
                            local.get 4
                            i32.add
                            i32.load8_u
                            local.set 3
                          end
                          i32.const 0
                          local.set 4
                          local.get 0
                          i32.load16_u offset=24
                          local.get 0
                          i32.load16_u offset=8
                          i32.le_u
                          br_if 10 (;@1;)
                          local.get 0
                          local.get 3
                          local.get 2
                          i32.mul
                          i32.const 255
                          i32.and
                          call $Stack<unsigned_char>::push_unsigned_char_
                          i32.const 0
                          return
                        end
                        local.get 0
                        call $PLCMethods::MUL_uint16_t_RuntimeStack*_
                        drop
                        br 8 (;@2;)
                      end
                      local.get 0
                      call $PLCMethods::MUL_uint32_t_RuntimeStack*_
                      drop
                      br 7 (;@2;)
                    end
                    local.get 0
                    local.get 0
                    call $RuntimeStack::pop_uint64_t__
                    local.get 0
                    call $RuntimeStack::pop_uint64_t__
                    i64.mul
                    call $RuntimeStack::push_uint64_t_unsigned_long_long_
                    drop
                    br 6 (;@2;)
                  end
                  i32.const 0
                  local.set 3
                  i32.const 0
                  local.set 2
                  block  ;; label = @8
                    local.get 0
                    i32.load offset=8
                    local.tee 4
                    i32.eqz
                    br_if 0 (;@8;)
                    local.get 0
                    local.get 4
                    i32.const -1
                    i32.add
                    local.tee 4
                    i32.store offset=8
                    local.get 0
                    i32.load
                    local.get 4
                    i32.add
                    i32.load8_u
                    local.set 2
                  end
                  block  ;; label = @8
                    local.get 0
                    i32.load offset=8
                    local.tee 4
                    i32.eqz
                    br_if 0 (;@8;)
                    local.get 0
                    local.get 4
                    i32.const -1
                    i32.add
                    local.tee 4
                    i32.store offset=8
                    local.get 0
                    i32.load
                    local.get 4
                    i32.add
                    i32.load8_u
                    local.set 3
                  end
                  i32.const 0
                  local.set 4
                  local.get 0
                  i32.load16_u offset=24
                  local.get 0
                  i32.load16_u offset=8
                  i32.le_u
                  br_if 6 (;@1;)
                  local.get 0
                  local.get 3
                  local.get 2
                  i32.mul
                  i32.const 255
                  i32.and
                  call $Stack<unsigned_char>::push_unsigned_char_
                  i32.const 0
                  return
                end
                local.get 0
                call $PLCMethods::MUL_int16_t_RuntimeStack*_
                drop
                br 4 (;@2;)
              end
              local.get 0
              call $PLCMethods::MUL_int32_t_RuntimeStack*_
              drop
              br 3 (;@2;)
            end
            local.get 0
            local.get 0
            call $RuntimeStack::pop_uint64_t__
            local.get 0
            call $RuntimeStack::pop_uint64_t__
            i64.mul
            call $RuntimeStack::push_uint64_t_unsigned_long_long_
            drop
            br 2 (;@2;)
          end
          local.get 0
          call $PLCMethods::MUL_float_RuntimeStack*_
          drop
          br 1 (;@2;)
        end
        local.get 0
        local.get 0
        call $RuntimeStack::pop_uint64_t__
        f64.reinterpret_i64
        local.get 0
        call $RuntimeStack::pop_uint64_t__
        f64.reinterpret_i64
        f64.mul
        i64.reinterpret_f64
        call $RuntimeStack::push_uint64_t_unsigned_long_long_
        drop
      end
      i32.const 0
      local.set 4
    end
    local.get 4)
  (func $PLCMethods::handle_DIV_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i64)
    i32.const 15
    local.set 4
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 5
      local.get 2
      i32.ge_u
      br_if 0 (;@1;)
      local.get 3
      local.get 5
      i32.const 1
      i32.add
      i32.store16
      i32.const 3
      local.set 4
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          block  ;; label = @12
                            local.get 1
                            local.get 5
                            i32.add
                            i32.load8_u
                            i32.const -1
                            i32.add
                            br_table 0 (;@12;) 0 (;@12;) 1 (;@11;) 2 (;@10;) 3 (;@9;) 4 (;@8;) 5 (;@7;) 6 (;@6;) 7 (;@5;) 8 (;@4;) 9 (;@3;) 11 (;@1;)
                          end
                          i32.const 0
                          local.set 3
                          i32.const 0
                          local.set 2
                          block  ;; label = @12
                            local.get 0
                            i32.load offset=8
                            local.tee 4
                            i32.eqz
                            br_if 0 (;@12;)
                            local.get 0
                            local.get 4
                            i32.const -1
                            i32.add
                            local.tee 4
                            i32.store offset=8
                            local.get 0
                            i32.load
                            local.get 4
                            i32.add
                            i32.load8_u
                            local.set 2
                          end
                          block  ;; label = @12
                            local.get 0
                            i32.load offset=8
                            local.tee 4
                            i32.eqz
                            br_if 0 (;@12;)
                            local.get 0
                            local.get 4
                            i32.const -1
                            i32.add
                            local.tee 4
                            i32.store offset=8
                            local.get 0
                            i32.load
                            local.get 4
                            i32.add
                            i32.load8_u
                            local.set 3
                          end
                          i32.const 0
                          local.set 4
                          local.get 0
                          i32.load16_u offset=24
                          local.get 0
                          i32.load16_u offset=8
                          i32.le_u
                          br_if 10 (;@1;)
                          local.get 0
                          local.get 3
                          i32.const 255
                          i32.and
                          local.get 2
                          i32.const 255
                          i32.and
                          i32.div_u
                          call $Stack<unsigned_char>::push_unsigned_char_
                          i32.const 0
                          return
                        end
                        local.get 0
                        call $PLCMethods::DIV_uint16_t_RuntimeStack*_
                        drop
                        br 8 (;@2;)
                      end
                      local.get 0
                      call $PLCMethods::DIV_uint32_t_RuntimeStack*_
                      drop
                      br 7 (;@2;)
                    end
                    local.get 0
                    call $RuntimeStack::pop_uint64_t__
                    local.set 6
                    local.get 0
                    local.get 0
                    call $RuntimeStack::pop_uint64_t__
                    local.get 6
                    i64.div_u
                    call $RuntimeStack::push_uint64_t_unsigned_long_long_
                    drop
                    br 6 (;@2;)
                  end
                  i32.const 0
                  local.set 3
                  i32.const 0
                  local.set 2
                  block  ;; label = @8
                    local.get 0
                    i32.load offset=8
                    local.tee 4
                    i32.eqz
                    br_if 0 (;@8;)
                    local.get 0
                    local.get 4
                    i32.const -1
                    i32.add
                    local.tee 4
                    i32.store offset=8
                    local.get 0
                    i32.load
                    local.get 4
                    i32.add
                    i32.load8_s
                    local.set 2
                  end
                  block  ;; label = @8
                    local.get 0
                    i32.load offset=8
                    local.tee 4
                    i32.eqz
                    br_if 0 (;@8;)
                    local.get 0
                    local.get 4
                    i32.const -1
                    i32.add
                    local.tee 4
                    i32.store offset=8
                    local.get 0
                    i32.load
                    local.get 4
                    i32.add
                    i32.load8_s
                    local.set 3
                  end
                  i32.const 0
                  local.set 4
                  local.get 0
                  i32.load16_u offset=24
                  local.get 0
                  i32.load16_u offset=8
                  i32.le_u
                  br_if 6 (;@1;)
                  local.get 0
                  local.get 3
                  local.get 2
                  i32.div_s
                  i32.const 255
                  i32.and
                  call $Stack<unsigned_char>::push_unsigned_char_
                  i32.const 0
                  return
                end
                local.get 0
                call $PLCMethods::DIV_int16_t_RuntimeStack*_
                drop
                br 4 (;@2;)
              end
              local.get 0
              call $PLCMethods::DIV_int32_t_RuntimeStack*_
              drop
              br 3 (;@2;)
            end
            local.get 0
            call $RuntimeStack::pop_uint64_t__
            local.set 6
            local.get 0
            local.get 0
            call $RuntimeStack::pop_uint64_t__
            local.get 6
            i64.div_s
            call $RuntimeStack::push_uint64_t_unsigned_long_long_
            drop
            br 2 (;@2;)
          end
          local.get 0
          call $PLCMethods::DIV_float_RuntimeStack*_
          drop
          br 1 (;@2;)
        end
        local.get 0
        call $RuntimeStack::pop_uint64_t__
        local.set 6
        local.get 0
        local.get 0
        call $RuntimeStack::pop_uint64_t__
        f64.reinterpret_i64
        local.get 6
        f64.reinterpret_i64
        f64.div
        i64.reinterpret_f64
        call $RuntimeStack::push_uint64_t_unsigned_long_long_
        drop
      end
      i32.const 0
      local.set 4
    end
    local.get 4)
  (func $PLCMethods::handle_CMP_EQ_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i64 i64)
    i32.const 15
    local.set 4
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 5
      local.get 2
      i32.ge_u
      br_if 0 (;@1;)
      local.get 3
      local.get 5
      i32.const 1
      i32.add
      i32.store16
      i32.const 3
      local.set 4
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          local.get 1
                          local.get 5
                          i32.add
                          i32.load8_u
                          i32.const -1
                          i32.add
                          br_table 0 (;@11;) 0 (;@11;) 1 (;@10;) 2 (;@9;) 3 (;@8;) 4 (;@7;) 5 (;@6;) 6 (;@5;) 7 (;@4;) 8 (;@3;) 9 (;@2;) 10 (;@1;)
                        end
                        i32.const 0
                        local.set 3
                        i32.const 0
                        local.set 2
                        block  ;; label = @11
                          local.get 0
                          i32.load offset=8
                          local.tee 4
                          i32.eqz
                          br_if 0 (;@11;)
                          local.get 0
                          local.get 4
                          i32.const -1
                          i32.add
                          local.tee 4
                          i32.store offset=8
                          local.get 0
                          i32.load
                          local.get 4
                          i32.add
                          i32.load8_u
                          local.set 2
                        end
                        block  ;; label = @11
                          local.get 0
                          i32.load offset=8
                          local.tee 4
                          i32.eqz
                          br_if 0 (;@11;)
                          local.get 0
                          local.get 4
                          i32.const -1
                          i32.add
                          local.tee 3
                          i32.store offset=8
                          local.get 0
                          i32.load
                          local.get 3
                          i32.add
                          i32.load8_u
                          local.set 3
                        end
                        i32.const 0
                        local.set 4
                        local.get 0
                        i32.load16_u offset=24
                        local.get 0
                        i32.load16_u offset=8
                        i32.le_u
                        br_if 9 (;@1;)
                        local.get 0
                        local.get 3
                        i32.const 255
                        i32.and
                        local.get 2
                        i32.const 255
                        i32.and
                        i32.eq
                        call $Stack<unsigned_char>::push_unsigned_char_
                        i32.const 0
                        return
                      end
                      local.get 0
                      call $PLCMethods::CMP_EQ_uint16_t_RuntimeStack*_
                      drop
                      i32.const 0
                      return
                    end
                    local.get 0
                    call $PLCMethods::CMP_EQ_uint32_t_RuntimeStack*_
                    drop
                    i32.const 0
                    return
                  end
                  local.get 0
                  call $RuntimeStack::pop_uint64_t__
                  local.set 6
                  local.get 0
                  call $RuntimeStack::pop_uint64_t__
                  local.set 7
                  i32.const 0
                  local.set 4
                  local.get 0
                  i32.load16_u offset=24
                  local.get 0
                  i32.load16_u offset=8
                  i32.le_u
                  br_if 6 (;@1;)
                  local.get 0
                  local.get 7
                  local.get 6
                  i64.eq
                  call $Stack<unsigned_char>::push_unsigned_char_
                  i32.const 0
                  return
                end
                i32.const 0
                local.set 3
                i32.const 0
                local.set 2
                block  ;; label = @7
                  local.get 0
                  i32.load offset=8
                  local.tee 4
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 0
                  local.get 4
                  i32.const -1
                  i32.add
                  local.tee 4
                  i32.store offset=8
                  local.get 0
                  i32.load
                  local.get 4
                  i32.add
                  i32.load8_u
                  local.set 2
                end
                block  ;; label = @7
                  local.get 0
                  i32.load offset=8
                  local.tee 4
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 0
                  local.get 4
                  i32.const -1
                  i32.add
                  local.tee 3
                  i32.store offset=8
                  local.get 0
                  i32.load
                  local.get 3
                  i32.add
                  i32.load8_u
                  local.set 3
                end
                i32.const 0
                local.set 4
                local.get 0
                i32.load16_u offset=24
                local.get 0
                i32.load16_u offset=8
                i32.le_u
                br_if 5 (;@1;)
                local.get 0
                local.get 3
                i32.const 255
                i32.and
                local.get 2
                i32.const 255
                i32.and
                i32.eq
                call $Stack<unsigned_char>::push_unsigned_char_
                i32.const 0
                return
              end
              local.get 0
              call $PLCMethods::CMP_EQ_int16_t_RuntimeStack*_
              drop
              i32.const 0
              return
            end
            local.get 0
            call $PLCMethods::CMP_EQ_int32_t_RuntimeStack*_
            drop
            i32.const 0
            return
          end
          local.get 0
          call $RuntimeStack::pop_uint64_t__
          local.set 6
          local.get 0
          call $RuntimeStack::pop_uint64_t__
          local.set 7
          i32.const 0
          local.set 4
          local.get 0
          i32.load16_u offset=24
          local.get 0
          i32.load16_u offset=8
          i32.le_u
          br_if 2 (;@1;)
          local.get 0
          local.get 7
          local.get 6
          i64.eq
          call $Stack<unsigned_char>::push_unsigned_char_
          i32.const 0
          return
        end
        local.get 0
        call $PLCMethods::CMP_EQ_float_RuntimeStack*_
        drop
        i32.const 0
        return
      end
      local.get 0
      call $RuntimeStack::pop_uint64_t__
      local.set 6
      local.get 0
      call $RuntimeStack::pop_uint64_t__
      local.set 7
      i32.const 0
      local.set 4
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      f64.reinterpret_i64
      local.get 6
      f64.reinterpret_i64
      f64.eq
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    local.get 4)
  (func $PLCMethods::handle_CMP_NEQ_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i64 i64)
    i32.const 15
    local.set 4
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 5
      local.get 2
      i32.ge_u
      br_if 0 (;@1;)
      local.get 3
      local.get 5
      i32.const 1
      i32.add
      i32.store16
      i32.const 3
      local.set 4
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          local.get 1
                          local.get 5
                          i32.add
                          i32.load8_u
                          i32.const -1
                          i32.add
                          br_table 0 (;@11;) 0 (;@11;) 1 (;@10;) 2 (;@9;) 3 (;@8;) 4 (;@7;) 5 (;@6;) 6 (;@5;) 7 (;@4;) 8 (;@3;) 9 (;@2;) 10 (;@1;)
                        end
                        i32.const 0
                        local.set 3
                        i32.const 0
                        local.set 2
                        block  ;; label = @11
                          local.get 0
                          i32.load offset=8
                          local.tee 4
                          i32.eqz
                          br_if 0 (;@11;)
                          local.get 0
                          local.get 4
                          i32.const -1
                          i32.add
                          local.tee 4
                          i32.store offset=8
                          local.get 0
                          i32.load
                          local.get 4
                          i32.add
                          i32.load8_u
                          local.set 2
                        end
                        block  ;; label = @11
                          local.get 0
                          i32.load offset=8
                          local.tee 4
                          i32.eqz
                          br_if 0 (;@11;)
                          local.get 0
                          local.get 4
                          i32.const -1
                          i32.add
                          local.tee 3
                          i32.store offset=8
                          local.get 0
                          i32.load
                          local.get 3
                          i32.add
                          i32.load8_u
                          local.set 3
                        end
                        i32.const 0
                        local.set 4
                        local.get 0
                        i32.load16_u offset=24
                        local.get 0
                        i32.load16_u offset=8
                        i32.le_u
                        br_if 9 (;@1;)
                        local.get 0
                        local.get 3
                        i32.const 255
                        i32.and
                        local.get 2
                        i32.const 255
                        i32.and
                        i32.ne
                        call $Stack<unsigned_char>::push_unsigned_char_
                        i32.const 0
                        return
                      end
                      local.get 0
                      call $PLCMethods::CMP_NEQ_uint16_t_RuntimeStack*_
                      drop
                      i32.const 0
                      return
                    end
                    local.get 0
                    call $PLCMethods::CMP_NEQ_uint32_t_RuntimeStack*_
                    drop
                    i32.const 0
                    return
                  end
                  local.get 0
                  call $RuntimeStack::pop_uint64_t__
                  local.set 6
                  local.get 0
                  call $RuntimeStack::pop_uint64_t__
                  local.set 7
                  i32.const 0
                  local.set 4
                  local.get 0
                  i32.load16_u offset=24
                  local.get 0
                  i32.load16_u offset=8
                  i32.le_u
                  br_if 6 (;@1;)
                  local.get 0
                  local.get 7
                  local.get 6
                  i64.ne
                  call $Stack<unsigned_char>::push_unsigned_char_
                  i32.const 0
                  return
                end
                i32.const 0
                local.set 3
                i32.const 0
                local.set 2
                block  ;; label = @7
                  local.get 0
                  i32.load offset=8
                  local.tee 4
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 0
                  local.get 4
                  i32.const -1
                  i32.add
                  local.tee 4
                  i32.store offset=8
                  local.get 0
                  i32.load
                  local.get 4
                  i32.add
                  i32.load8_u
                  local.set 2
                end
                block  ;; label = @7
                  local.get 0
                  i32.load offset=8
                  local.tee 4
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 0
                  local.get 4
                  i32.const -1
                  i32.add
                  local.tee 3
                  i32.store offset=8
                  local.get 0
                  i32.load
                  local.get 3
                  i32.add
                  i32.load8_u
                  local.set 3
                end
                i32.const 0
                local.set 4
                local.get 0
                i32.load16_u offset=24
                local.get 0
                i32.load16_u offset=8
                i32.le_u
                br_if 5 (;@1;)
                local.get 0
                local.get 3
                i32.const 255
                i32.and
                local.get 2
                i32.const 255
                i32.and
                i32.ne
                call $Stack<unsigned_char>::push_unsigned_char_
                i32.const 0
                return
              end
              local.get 0
              call $PLCMethods::CMP_NEQ_int16_t_RuntimeStack*_
              drop
              i32.const 0
              return
            end
            local.get 0
            call $PLCMethods::CMP_NEQ_int32_t_RuntimeStack*_
            drop
            i32.const 0
            return
          end
          local.get 0
          call $RuntimeStack::pop_uint64_t__
          local.set 6
          local.get 0
          call $RuntimeStack::pop_uint64_t__
          local.set 7
          i32.const 0
          local.set 4
          local.get 0
          i32.load16_u offset=24
          local.get 0
          i32.load16_u offset=8
          i32.le_u
          br_if 2 (;@1;)
          local.get 0
          local.get 7
          local.get 6
          i64.ne
          call $Stack<unsigned_char>::push_unsigned_char_
          i32.const 0
          return
        end
        local.get 0
        call $PLCMethods::CMP_NEQ_float_RuntimeStack*_
        drop
        i32.const 0
        return
      end
      local.get 0
      call $RuntimeStack::pop_uint64_t__
      local.set 6
      local.get 0
      call $RuntimeStack::pop_uint64_t__
      local.set 7
      i32.const 0
      local.set 4
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      f64.reinterpret_i64
      local.get 6
      f64.reinterpret_i64
      f64.ne
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    local.get 4)
  (func $PLCMethods::handle_CMP_GT_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i64 i64)
    i32.const 15
    local.set 4
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 5
      local.get 2
      i32.ge_u
      br_if 0 (;@1;)
      local.get 3
      local.get 5
      i32.const 1
      i32.add
      i32.store16
      i32.const 3
      local.set 4
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          local.get 1
                          local.get 5
                          i32.add
                          i32.load8_u
                          i32.const -1
                          i32.add
                          br_table 0 (;@11;) 0 (;@11;) 1 (;@10;) 2 (;@9;) 3 (;@8;) 4 (;@7;) 5 (;@6;) 6 (;@5;) 7 (;@4;) 8 (;@3;) 9 (;@2;) 10 (;@1;)
                        end
                        i32.const 0
                        local.set 3
                        i32.const 0
                        local.set 2
                        block  ;; label = @11
                          local.get 0
                          i32.load offset=8
                          local.tee 4
                          i32.eqz
                          br_if 0 (;@11;)
                          local.get 0
                          local.get 4
                          i32.const -1
                          i32.add
                          local.tee 4
                          i32.store offset=8
                          local.get 0
                          i32.load
                          local.get 4
                          i32.add
                          i32.load8_u
                          local.set 2
                        end
                        block  ;; label = @11
                          local.get 0
                          i32.load offset=8
                          local.tee 4
                          i32.eqz
                          br_if 0 (;@11;)
                          local.get 0
                          local.get 4
                          i32.const -1
                          i32.add
                          local.tee 3
                          i32.store offset=8
                          local.get 0
                          i32.load
                          local.get 3
                          i32.add
                          i32.load8_u
                          local.set 3
                        end
                        i32.const 0
                        local.set 4
                        local.get 0
                        i32.load16_u offset=24
                        local.get 0
                        i32.load16_u offset=8
                        i32.le_u
                        br_if 9 (;@1;)
                        local.get 0
                        local.get 3
                        i32.const 255
                        i32.and
                        local.get 2
                        i32.const 255
                        i32.and
                        i32.gt_u
                        call $Stack<unsigned_char>::push_unsigned_char_
                        i32.const 0
                        return
                      end
                      local.get 0
                      call $PLCMethods::CMP_GT_uint16_t_RuntimeStack*_
                      drop
                      i32.const 0
                      return
                    end
                    local.get 0
                    call $PLCMethods::CMP_GT_uint32_t_RuntimeStack*_
                    drop
                    i32.const 0
                    return
                  end
                  local.get 0
                  call $RuntimeStack::pop_uint64_t__
                  local.set 6
                  local.get 0
                  call $RuntimeStack::pop_uint64_t__
                  local.set 7
                  i32.const 0
                  local.set 4
                  local.get 0
                  i32.load16_u offset=24
                  local.get 0
                  i32.load16_u offset=8
                  i32.le_u
                  br_if 6 (;@1;)
                  local.get 0
                  local.get 7
                  local.get 6
                  i64.gt_u
                  call $Stack<unsigned_char>::push_unsigned_char_
                  i32.const 0
                  return
                end
                i32.const 0
                local.set 3
                i32.const 0
                local.set 2
                block  ;; label = @7
                  local.get 0
                  i32.load offset=8
                  local.tee 4
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 0
                  local.get 4
                  i32.const -1
                  i32.add
                  local.tee 4
                  i32.store offset=8
                  local.get 0
                  i32.load
                  local.get 4
                  i32.add
                  i32.load8_u
                  local.set 2
                end
                block  ;; label = @7
                  local.get 0
                  i32.load offset=8
                  local.tee 4
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 0
                  local.get 4
                  i32.const -1
                  i32.add
                  local.tee 3
                  i32.store offset=8
                  local.get 0
                  i32.load
                  local.get 3
                  i32.add
                  i32.load8_u
                  local.set 3
                end
                i32.const 0
                local.set 4
                local.get 0
                i32.load16_u offset=24
                local.get 0
                i32.load16_u offset=8
                i32.le_u
                br_if 5 (;@1;)
                local.get 0
                local.get 3
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                local.get 2
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                i32.gt_s
                call $Stack<unsigned_char>::push_unsigned_char_
                i32.const 0
                return
              end
              local.get 0
              call $PLCMethods::CMP_GT_int16_t_RuntimeStack*_
              drop
              i32.const 0
              return
            end
            local.get 0
            call $PLCMethods::CMP_GT_int32_t_RuntimeStack*_
            drop
            i32.const 0
            return
          end
          local.get 0
          call $RuntimeStack::pop_uint64_t__
          local.set 6
          local.get 0
          call $RuntimeStack::pop_uint64_t__
          local.set 7
          i32.const 0
          local.set 4
          local.get 0
          i32.load16_u offset=24
          local.get 0
          i32.load16_u offset=8
          i32.le_u
          br_if 2 (;@1;)
          local.get 0
          local.get 7
          local.get 6
          i64.gt_s
          call $Stack<unsigned_char>::push_unsigned_char_
          i32.const 0
          return
        end
        local.get 0
        call $PLCMethods::CMP_GT_float_RuntimeStack*_
        drop
        i32.const 0
        return
      end
      local.get 0
      call $RuntimeStack::pop_uint64_t__
      local.set 6
      local.get 0
      call $RuntimeStack::pop_uint64_t__
      local.set 7
      i32.const 0
      local.set 4
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      f64.reinterpret_i64
      local.get 6
      f64.reinterpret_i64
      f64.gt
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    local.get 4)
  (func $PLCMethods::handle_CMP_GTE_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i64 i64)
    i32.const 15
    local.set 4
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 5
      local.get 2
      i32.ge_u
      br_if 0 (;@1;)
      local.get 3
      local.get 5
      i32.const 1
      i32.add
      i32.store16
      i32.const 3
      local.set 4
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          local.get 1
                          local.get 5
                          i32.add
                          i32.load8_u
                          i32.const -1
                          i32.add
                          br_table 0 (;@11;) 0 (;@11;) 1 (;@10;) 2 (;@9;) 3 (;@8;) 4 (;@7;) 5 (;@6;) 6 (;@5;) 7 (;@4;) 8 (;@3;) 9 (;@2;) 10 (;@1;)
                        end
                        i32.const 0
                        local.set 3
                        i32.const 0
                        local.set 2
                        block  ;; label = @11
                          local.get 0
                          i32.load offset=8
                          local.tee 4
                          i32.eqz
                          br_if 0 (;@11;)
                          local.get 0
                          local.get 4
                          i32.const -1
                          i32.add
                          local.tee 4
                          i32.store offset=8
                          local.get 0
                          i32.load
                          local.get 4
                          i32.add
                          i32.load8_u
                          local.set 2
                        end
                        block  ;; label = @11
                          local.get 0
                          i32.load offset=8
                          local.tee 4
                          i32.eqz
                          br_if 0 (;@11;)
                          local.get 0
                          local.get 4
                          i32.const -1
                          i32.add
                          local.tee 3
                          i32.store offset=8
                          local.get 0
                          i32.load
                          local.get 3
                          i32.add
                          i32.load8_u
                          local.set 3
                        end
                        i32.const 0
                        local.set 4
                        local.get 0
                        i32.load16_u offset=24
                        local.get 0
                        i32.load16_u offset=8
                        i32.le_u
                        br_if 9 (;@1;)
                        local.get 0
                        local.get 3
                        i32.const 255
                        i32.and
                        local.get 2
                        i32.const 255
                        i32.and
                        i32.ge_u
                        call $Stack<unsigned_char>::push_unsigned_char_
                        i32.const 0
                        return
                      end
                      local.get 0
                      call $PLCMethods::CMP_GTE_uint16_t_RuntimeStack*_
                      drop
                      i32.const 0
                      return
                    end
                    local.get 0
                    call $PLCMethods::CMP_GTE_uint32_t_RuntimeStack*_
                    drop
                    i32.const 0
                    return
                  end
                  local.get 0
                  call $RuntimeStack::pop_uint64_t__
                  local.set 6
                  local.get 0
                  call $RuntimeStack::pop_uint64_t__
                  local.set 7
                  i32.const 0
                  local.set 4
                  local.get 0
                  i32.load16_u offset=24
                  local.get 0
                  i32.load16_u offset=8
                  i32.le_u
                  br_if 6 (;@1;)
                  local.get 0
                  local.get 7
                  local.get 6
                  i64.ge_u
                  call $Stack<unsigned_char>::push_unsigned_char_
                  i32.const 0
                  return
                end
                i32.const 0
                local.set 3
                i32.const 0
                local.set 2
                block  ;; label = @7
                  local.get 0
                  i32.load offset=8
                  local.tee 4
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 0
                  local.get 4
                  i32.const -1
                  i32.add
                  local.tee 4
                  i32.store offset=8
                  local.get 0
                  i32.load
                  local.get 4
                  i32.add
                  i32.load8_u
                  local.set 2
                end
                block  ;; label = @7
                  local.get 0
                  i32.load offset=8
                  local.tee 4
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 0
                  local.get 4
                  i32.const -1
                  i32.add
                  local.tee 3
                  i32.store offset=8
                  local.get 0
                  i32.load
                  local.get 3
                  i32.add
                  i32.load8_u
                  local.set 3
                end
                i32.const 0
                local.set 4
                local.get 0
                i32.load16_u offset=24
                local.get 0
                i32.load16_u offset=8
                i32.le_u
                br_if 5 (;@1;)
                local.get 0
                local.get 3
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                local.get 2
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                i32.ge_s
                call $Stack<unsigned_char>::push_unsigned_char_
                i32.const 0
                return
              end
              local.get 0
              call $PLCMethods::CMP_GTE_int16_t_RuntimeStack*_
              drop
              i32.const 0
              return
            end
            local.get 0
            call $PLCMethods::CMP_GTE_int32_t_RuntimeStack*_
            drop
            i32.const 0
            return
          end
          local.get 0
          call $RuntimeStack::pop_uint64_t__
          local.set 6
          local.get 0
          call $RuntimeStack::pop_uint64_t__
          local.set 7
          i32.const 0
          local.set 4
          local.get 0
          i32.load16_u offset=24
          local.get 0
          i32.load16_u offset=8
          i32.le_u
          br_if 2 (;@1;)
          local.get 0
          local.get 7
          local.get 6
          i64.ge_s
          call $Stack<unsigned_char>::push_unsigned_char_
          i32.const 0
          return
        end
        local.get 0
        call $PLCMethods::CMP_GTE_float_RuntimeStack*_
        drop
        i32.const 0
        return
      end
      local.get 0
      call $RuntimeStack::pop_uint64_t__
      local.set 6
      local.get 0
      call $RuntimeStack::pop_uint64_t__
      local.set 7
      i32.const 0
      local.set 4
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      f64.reinterpret_i64
      local.get 6
      f64.reinterpret_i64
      f64.ge
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    local.get 4)
  (func $PLCMethods::handle_CMP_LT_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i64 i64)
    i32.const 15
    local.set 4
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 5
      local.get 2
      i32.ge_u
      br_if 0 (;@1;)
      local.get 3
      local.get 5
      i32.const 1
      i32.add
      i32.store16
      i32.const 3
      local.set 4
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          local.get 1
                          local.get 5
                          i32.add
                          i32.load8_u
                          i32.const -1
                          i32.add
                          br_table 0 (;@11;) 0 (;@11;) 1 (;@10;) 2 (;@9;) 3 (;@8;) 4 (;@7;) 5 (;@6;) 6 (;@5;) 7 (;@4;) 8 (;@3;) 9 (;@2;) 10 (;@1;)
                        end
                        i32.const 0
                        local.set 3
                        i32.const 0
                        local.set 2
                        block  ;; label = @11
                          local.get 0
                          i32.load offset=8
                          local.tee 4
                          i32.eqz
                          br_if 0 (;@11;)
                          local.get 0
                          local.get 4
                          i32.const -1
                          i32.add
                          local.tee 4
                          i32.store offset=8
                          local.get 0
                          i32.load
                          local.get 4
                          i32.add
                          i32.load8_u
                          local.set 2
                        end
                        block  ;; label = @11
                          local.get 0
                          i32.load offset=8
                          local.tee 4
                          i32.eqz
                          br_if 0 (;@11;)
                          local.get 0
                          local.get 4
                          i32.const -1
                          i32.add
                          local.tee 3
                          i32.store offset=8
                          local.get 0
                          i32.load
                          local.get 3
                          i32.add
                          i32.load8_u
                          local.set 3
                        end
                        i32.const 0
                        local.set 4
                        local.get 0
                        i32.load16_u offset=24
                        local.get 0
                        i32.load16_u offset=8
                        i32.le_u
                        br_if 9 (;@1;)
                        local.get 0
                        local.get 3
                        i32.const 255
                        i32.and
                        local.get 2
                        i32.const 255
                        i32.and
                        i32.lt_u
                        call $Stack<unsigned_char>::push_unsigned_char_
                        i32.const 0
                        return
                      end
                      local.get 0
                      call $PLCMethods::CMP_LT_uint16_t_RuntimeStack*_
                      drop
                      i32.const 0
                      return
                    end
                    local.get 0
                    call $PLCMethods::CMP_LT_uint32_t_RuntimeStack*_
                    drop
                    i32.const 0
                    return
                  end
                  local.get 0
                  call $RuntimeStack::pop_uint64_t__
                  local.set 6
                  local.get 0
                  call $RuntimeStack::pop_uint64_t__
                  local.set 7
                  i32.const 0
                  local.set 4
                  local.get 0
                  i32.load16_u offset=24
                  local.get 0
                  i32.load16_u offset=8
                  i32.le_u
                  br_if 6 (;@1;)
                  local.get 0
                  local.get 7
                  local.get 6
                  i64.lt_u
                  call $Stack<unsigned_char>::push_unsigned_char_
                  i32.const 0
                  return
                end
                i32.const 0
                local.set 3
                i32.const 0
                local.set 2
                block  ;; label = @7
                  local.get 0
                  i32.load offset=8
                  local.tee 4
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 0
                  local.get 4
                  i32.const -1
                  i32.add
                  local.tee 4
                  i32.store offset=8
                  local.get 0
                  i32.load
                  local.get 4
                  i32.add
                  i32.load8_u
                  local.set 2
                end
                block  ;; label = @7
                  local.get 0
                  i32.load offset=8
                  local.tee 4
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 0
                  local.get 4
                  i32.const -1
                  i32.add
                  local.tee 3
                  i32.store offset=8
                  local.get 0
                  i32.load
                  local.get 3
                  i32.add
                  i32.load8_u
                  local.set 3
                end
                i32.const 0
                local.set 4
                local.get 0
                i32.load16_u offset=24
                local.get 0
                i32.load16_u offset=8
                i32.le_u
                br_if 5 (;@1;)
                local.get 0
                local.get 3
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                local.get 2
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                i32.lt_s
                call $Stack<unsigned_char>::push_unsigned_char_
                i32.const 0
                return
              end
              local.get 0
              call $PLCMethods::CMP_LT_int16_t_RuntimeStack*_
              drop
              i32.const 0
              return
            end
            local.get 0
            call $PLCMethods::CMP_LT_int32_t_RuntimeStack*_
            drop
            i32.const 0
            return
          end
          local.get 0
          call $RuntimeStack::pop_uint64_t__
          local.set 6
          local.get 0
          call $RuntimeStack::pop_uint64_t__
          local.set 7
          i32.const 0
          local.set 4
          local.get 0
          i32.load16_u offset=24
          local.get 0
          i32.load16_u offset=8
          i32.le_u
          br_if 2 (;@1;)
          local.get 0
          local.get 7
          local.get 6
          i64.lt_s
          call $Stack<unsigned_char>::push_unsigned_char_
          i32.const 0
          return
        end
        local.get 0
        call $PLCMethods::CMP_LT_float_RuntimeStack*_
        drop
        i32.const 0
        return
      end
      local.get 0
      call $RuntimeStack::pop_uint64_t__
      local.set 6
      local.get 0
      call $RuntimeStack::pop_uint64_t__
      local.set 7
      i32.const 0
      local.set 4
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      f64.reinterpret_i64
      local.get 6
      f64.reinterpret_i64
      f64.lt
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    local.get 4)
  (func $PLCMethods::handle_CMP_LTE_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i64 i64)
    i32.const 15
    local.set 4
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 5
      local.get 2
      i32.ge_u
      br_if 0 (;@1;)
      local.get 3
      local.get 5
      i32.const 1
      i32.add
      i32.store16
      i32.const 3
      local.set 4
      block  ;; label = @2
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          local.get 1
                          local.get 5
                          i32.add
                          i32.load8_u
                          i32.const -1
                          i32.add
                          br_table 0 (;@11;) 0 (;@11;) 1 (;@10;) 2 (;@9;) 3 (;@8;) 4 (;@7;) 5 (;@6;) 6 (;@5;) 7 (;@4;) 8 (;@3;) 9 (;@2;) 10 (;@1;)
                        end
                        i32.const 0
                        local.set 3
                        i32.const 0
                        local.set 2
                        block  ;; label = @11
                          local.get 0
                          i32.load offset=8
                          local.tee 4
                          i32.eqz
                          br_if 0 (;@11;)
                          local.get 0
                          local.get 4
                          i32.const -1
                          i32.add
                          local.tee 4
                          i32.store offset=8
                          local.get 0
                          i32.load
                          local.get 4
                          i32.add
                          i32.load8_u
                          local.set 2
                        end
                        block  ;; label = @11
                          local.get 0
                          i32.load offset=8
                          local.tee 4
                          i32.eqz
                          br_if 0 (;@11;)
                          local.get 0
                          local.get 4
                          i32.const -1
                          i32.add
                          local.tee 3
                          i32.store offset=8
                          local.get 0
                          i32.load
                          local.get 3
                          i32.add
                          i32.load8_u
                          local.set 3
                        end
                        i32.const 0
                        local.set 4
                        local.get 0
                        i32.load16_u offset=24
                        local.get 0
                        i32.load16_u offset=8
                        i32.le_u
                        br_if 9 (;@1;)
                        local.get 0
                        local.get 3
                        i32.const 255
                        i32.and
                        local.get 2
                        i32.const 255
                        i32.and
                        i32.le_u
                        call $Stack<unsigned_char>::push_unsigned_char_
                        i32.const 0
                        return
                      end
                      local.get 0
                      call $PLCMethods::CMP_LTE_uint16_t_RuntimeStack*_
                      drop
                      i32.const 0
                      return
                    end
                    local.get 0
                    call $PLCMethods::CMP_LTE_uint32_t_RuntimeStack*_
                    drop
                    i32.const 0
                    return
                  end
                  local.get 0
                  call $RuntimeStack::pop_uint64_t__
                  local.set 6
                  local.get 0
                  call $RuntimeStack::pop_uint64_t__
                  local.set 7
                  i32.const 0
                  local.set 4
                  local.get 0
                  i32.load16_u offset=24
                  local.get 0
                  i32.load16_u offset=8
                  i32.le_u
                  br_if 6 (;@1;)
                  local.get 0
                  local.get 7
                  local.get 6
                  i64.le_u
                  call $Stack<unsigned_char>::push_unsigned_char_
                  i32.const 0
                  return
                end
                i32.const 0
                local.set 3
                i32.const 0
                local.set 2
                block  ;; label = @7
                  local.get 0
                  i32.load offset=8
                  local.tee 4
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 0
                  local.get 4
                  i32.const -1
                  i32.add
                  local.tee 4
                  i32.store offset=8
                  local.get 0
                  i32.load
                  local.get 4
                  i32.add
                  i32.load8_u
                  local.set 2
                end
                block  ;; label = @7
                  local.get 0
                  i32.load offset=8
                  local.tee 4
                  i32.eqz
                  br_if 0 (;@7;)
                  local.get 0
                  local.get 4
                  i32.const -1
                  i32.add
                  local.tee 3
                  i32.store offset=8
                  local.get 0
                  i32.load
                  local.get 3
                  i32.add
                  i32.load8_u
                  local.set 3
                end
                i32.const 0
                local.set 4
                local.get 0
                i32.load16_u offset=24
                local.get 0
                i32.load16_u offset=8
                i32.le_u
                br_if 5 (;@1;)
                local.get 0
                local.get 3
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                local.get 2
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                i32.le_s
                call $Stack<unsigned_char>::push_unsigned_char_
                i32.const 0
                return
              end
              local.get 0
              call $PLCMethods::CMP_LTE_int16_t_RuntimeStack*_
              drop
              i32.const 0
              return
            end
            local.get 0
            call $PLCMethods::CMP_LTE_int32_t_RuntimeStack*_
            drop
            i32.const 0
            return
          end
          local.get 0
          call $RuntimeStack::pop_uint64_t__
          local.set 6
          local.get 0
          call $RuntimeStack::pop_uint64_t__
          local.set 7
          i32.const 0
          local.set 4
          local.get 0
          i32.load16_u offset=24
          local.get 0
          i32.load16_u offset=8
          i32.le_u
          br_if 2 (;@1;)
          local.get 0
          local.get 7
          local.get 6
          i64.le_s
          call $Stack<unsigned_char>::push_unsigned_char_
          i32.const 0
          return
        end
        local.get 0
        call $PLCMethods::CMP_LTE_float_RuntimeStack*_
        drop
        i32.const 0
        return
      end
      local.get 0
      call $RuntimeStack::pop_uint64_t__
      local.set 6
      local.get 0
      call $RuntimeStack::pop_uint64_t__
      local.set 7
      i32.const 0
      local.set 4
      local.get 0
      i32.load16_u offset=24
      local.get 0
      i32.load16_u offset=8
      i32.le_u
      br_if 0 (;@1;)
      local.get 0
      local.get 7
      f64.reinterpret_i64
      local.get 6
      f64.reinterpret_i64
      f64.le
      call $Stack<unsigned_char>::push_unsigned_char_
    end
    local.get 4)
  (func $Stack<unsigned_char>::print__ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 64
    i32.sub
    local.tee 1
    global.set $__stack_pointer
    i32.const 83
    local.set 2
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 2
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3233
      i32.add
      i32.load8_u
      local.set 2
      local.get 3
      i32.const 1
      i32.add
      local.tee 4
      local.set 3
      local.get 4
      i32.const 7
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 6
    local.set 5
    block  ;; label = @1
      local.get 0
      i32.load offset=8
      i32.const 9
      i32.gt_s
      br_if 0 (;@1;)
      i32.const 32
      call $__print
      i32.const 7
      local.set 5
    end
    local.get 1
    local.get 0
    i32.load offset=8
    i32.store offset=48
    i32.const 4400
    i32.const 1466
    local.get 1
    i32.const 48
    i32.add
    call $sprintf_
    drop
    i32.const 0
    local.set 6
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 2
      i32.eqz
      br_if 0 (;@1;)
      i32.const 0
      local.set 3
      loop  ;; label = @2
        local.get 2
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 4401
        i32.add
        local.set 2
        local.get 3
        i32.const 1
        i32.add
        local.tee 6
        local.set 3
        local.get 2
        i32.load8_u
        local.tee 2
        br_if 0 (;@2;)
      end
    end
    i32.const 41
    local.set 2
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 2
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 1509
      i32.add
      i32.load8_u
      local.set 2
      local.get 3
      i32.const 1
      i32.add
      local.tee 4
      local.set 3
      local.get 4
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 6
    local.get 5
    i32.add
    local.set 3
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load offset=8
        local.tee 2
        i32.const 16
        i32.gt_s
        br_if 0 (;@2;)
        local.get 3
        i32.const 3
        i32.add
        local.set 6
        local.get 0
        i32.load offset=8
        i32.const 1
        i32.lt_s
        br_if 1 (;@1;)
        i32.const 0
        local.set 5
        i32.const 0
        local.set 7
        loop  ;; label = @3
          block  ;; label = @4
            local.get 0
            i32.load
            local.get 5
            i32.add
            i32.load8_u
            local.tee 3
            i32.const 15
            i32.gt_u
            br_if 0 (;@4;)
            i32.const 48
            call $__print
            local.get 6
            i32.const 1
            i32.add
            local.set 6
          end
          local.get 1
          local.get 3
          i32.store offset=32
          i32.const 4400
          i32.const 1106
          local.get 1
          i32.const 32
          i32.add
          call $sprintf_
          drop
          i32.const 0
          local.set 4
          block  ;; label = @4
            i32.const 0
            i32.load8_u offset=4400
            local.tee 2
            i32.eqz
            br_if 0 (;@4;)
            i32.const 0
            local.set 3
            loop  ;; label = @5
              local.get 2
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 3
              i32.const 4401
              i32.add
              local.set 2
              local.get 3
              i32.const 1
              i32.add
              local.tee 4
              local.set 3
              local.get 2
              i32.load8_u
              local.tee 2
              br_if 0 (;@5;)
            end
          end
          local.get 4
          local.get 6
          i32.add
          local.set 6
          block  ;; label = @4
            local.get 0
            i32.load offset=8
            i32.const -1
            i32.add
            local.get 5
            i32.le_s
            br_if 0 (;@4;)
            i32.const 32
            call $__print
            local.get 6
            i32.const 1
            i32.add
            local.set 6
          end
          local.get 0
          i32.load offset=8
          local.get 7
          i32.const 1
          i32.add
          local.tee 7
          i32.const 65535
          i32.and
          local.tee 5
          i32.gt_s
          br_if 0 (;@3;)
          br 2 (;@1;)
        end
      end
      i32.const 32
      call $__print
      block  ;; label = @2
        block  ;; label = @3
          local.get 2
          i32.const 65520
          i32.add
          i32.const 65535
          i32.and
          local.tee 7
          i32.const 9
          i32.le_u
          br_if 0 (;@3;)
          local.get 3
          i32.const 4
          i32.add
          local.set 5
          br 1 (;@2;)
        end
        i32.const 32
        call $__print
        local.get 3
        i32.const 5
        i32.add
        local.set 5
      end
      local.get 1
      local.get 7
      i32.store offset=16
      i32.const 4400
      i32.const 1466
      local.get 1
      i32.const 16
      i32.add
      call $sprintf_
      drop
      i32.const 0
      local.set 6
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 2
        i32.eqz
        br_if 0 (;@2;)
        i32.const 0
        local.set 3
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 4401
          i32.add
          local.set 2
          local.get 3
          i32.const 1
          i32.add
          local.tee 6
          local.set 3
          local.get 2
          i32.load8_u
          local.tee 2
          br_if 0 (;@3;)
        end
      end
      i32.const 32
      local.set 2
      i32.const 1
      local.set 3
      loop  ;; label = @2
        local.get 2
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 3918
        i32.add
        i32.load8_u
        local.set 2
        local.get 3
        i32.const 1
        i32.add
        local.tee 4
        local.set 3
        local.get 4
        i32.const 16
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 6
      local.get 5
      i32.add
      i32.const 15
      i32.add
      local.set 6
      i32.const 0
      local.set 5
      loop  ;; label = @2
        block  ;; label = @3
          local.get 0
          i32.load
          local.get 7
          local.get 5
          i32.add
          i32.add
          i32.load8_u
          local.tee 3
          i32.const 15
          i32.gt_u
          br_if 0 (;@3;)
          i32.const 48
          call $__print
          local.get 6
          i32.const 1
          i32.add
          local.set 6
        end
        local.get 1
        local.get 3
        i32.store
        i32.const 4400
        i32.const 1106
        local.get 1
        call $sprintf_
        drop
        i32.const 0
        local.set 4
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 2
          i32.eqz
          br_if 0 (;@3;)
          i32.const 0
          local.set 3
          loop  ;; label = @4
            local.get 2
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 3
            i32.const 4401
            i32.add
            local.set 2
            local.get 3
            i32.const 1
            i32.add
            local.tee 4
            local.set 3
            local.get 2
            i32.load8_u
            local.tee 2
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        call $__print
        local.get 6
        local.get 4
        i32.add
        i32.const 1
        i32.add
        local.set 6
        local.get 5
        i32.const 1
        i32.add
        local.tee 5
        i32.const 16
        i32.ne
        br_if 0 (;@2;)
      end
    end
    i32.const 93
    call $__print
    local.get 1
    i32.const 64
    i32.add
    global.set $__stack_pointer
    local.get 6
    i32.const 1
    i32.add)
  (func $VovkPLCRuntime::startup__ (type 1) (param i32)
    (local i32 i32)
    block  ;; label = @1
      local.get 0
      i32.load8_u
      br_if 0 (;@1;)
      local.get 0
      i32.const 1
      i32.store8
      i32.const 10
      call $__print
      i32.const 0
      local.set 0
      loop  ;; label = @2
        i32.const 58
        call $__print
        local.get 0
        i32.const 255
        i32.and
        local.set 1
        local.get 0
        i32.const 1
        i32.add
        local.set 0
        local.get 1
        i32.const 69
        i32.lt_u
        br_if 0 (;@2;)
      end
      i32.const 10
      call $__print
      i32.const 58
      local.set 1
      i32.const 1
      local.set 0
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 0
        i32.const 2184
        i32.add
        i32.load8_u
        local.set 1
        local.get 0
        i32.const 1
        i32.add
        local.tee 2
        local.set 0
        local.get 2
        i32.const 71
        i32.ne
        br_if 0 (;@2;)
      end
      i32.const 10
      call $__print
      i32.const 0
      local.set 0
      loop  ;; label = @2
        i32.const 58
        call $__print
        local.get 0
        i32.const 255
        i32.and
        local.set 1
        local.get 0
        i32.const 1
        i32.add
        local.set 0
        local.get 1
        i32.const 69
        i32.lt_u
        br_if 0 (;@2;)
      end
      i32.const 10
      call $__print
    end)
  (func $VovkPLCRuntime::step_unsigned_char*__unsigned_short__unsigned_short&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32)
    block  ;; label = @1
      local.get 2
      br_if 0 (;@1;)
      i32.const 17
      return
    end
    i32.const 14
    local.set 4
    block  ;; label = @1
      local.get 3
      i32.load16_u
      local.tee 5
      local.get 2
      i32.ge_u
      br_if 0 (;@1;)
      local.get 1
      local.get 5
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      local.get 5
      i32.const 1
      i32.add
      i32.store16
      block  ;; label = @2
        i32.const 0
        br_if 0 (;@2;)
        block  ;; label = @3
          block  ;; label = @4
            block  ;; label = @5
              block  ;; label = @6
                block  ;; label = @7
                  block  ;; label = @8
                    block  ;; label = @9
                      block  ;; label = @10
                        block  ;; label = @11
                          block  ;; label = @12
                            block  ;; label = @13
                              block  ;; label = @14
                                block  ;; label = @15
                                  block  ;; label = @16
                                    block  ;; label = @17
                                      block  ;; label = @18
                                        block  ;; label = @19
                                          block  ;; label = @20
                                            block  ;; label = @21
                                              block  ;; label = @22
                                                block  ;; label = @23
                                                  block  ;; label = @24
                                                    block  ;; label = @25
                                                      block  ;; label = @26
                                                        block  ;; label = @27
                                                          block  ;; label = @28
                                                            block  ;; label = @29
                                                              block  ;; label = @30
                                                                block  ;; label = @31
                                                                  block  ;; label = @32
                                                                    block  ;; label = @33
                                                                      block  ;; label = @34
                                                                        block  ;; label = @35
                                                                          block  ;; label = @36
                                                                            block  ;; label = @37
                                                                              block  ;; label = @38
                                                                                block  ;; label = @39
                                                                                  block  ;; label = @40
                                                                                    block  ;; label = @41
                                                                                      block  ;; label = @42
                                                                                        block  ;; label = @43
                                                                                          block  ;; label = @44
                                                                                            block  ;; label = @45
                                                                                              block  ;; label = @46
                                                                                                block  ;; label = @47
                                                                                                  block  ;; label = @48
                                                                                                    block  ;; label = @49
                                                                                                      block  ;; label = @50
                                                                                                        block  ;; label = @51
                                                                                                          block  ;; label = @52
                                                                                                            block  ;; label = @53
                                                                                                              local.get 4
                                                                                                              br_table 52 (;@1;) 13 (;@40;) 14 (;@39;) 16 (;@37;) 18 (;@35;) 20 (;@33;) 15 (;@38;) 17 (;@36;) 19 (;@34;) 21 (;@32;) 22 (;@31;) 23 (;@30;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 4 (;@49;) 5 (;@48;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 24 (;@29;) 25 (;@28;) 26 (;@27;) 27 (;@26;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 28 (;@25;) 29 (;@24;) 30 (;@23;) 31 (;@22;) 32 (;@21;) 33 (;@20;) 34 (;@19;) 35 (;@18;) 36 (;@17;) 37 (;@16;) 38 (;@15;) 39 (;@14;) 40 (;@13;) 41 (;@12;) 42 (;@11;) 43 (;@10;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 0 (;@53;) 1 (;@52;) 3 (;@50;) 2 (;@51;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 44 (;@9;) 45 (;@8;) 46 (;@7;) 48 (;@5;) 47 (;@6;) 49 (;@4;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 6 (;@47;) 7 (;@46;) 8 (;@45;) 9 (;@44;) 10 (;@43;) 11 (;@42;) 12 (;@41;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 51 (;@2;) 50 (;@3;) 52 (;@1;)
                                                                                                            end
                                                                                                            local.get 0
                                                                                                            i32.load offset=4
                                                                                                            call $PLCMethods::LOGIC_AND_RuntimeStack*_
                                                                                                            drop
                                                                                                            i32.const 0
                                                                                                            return
                                                                                                          end
                                                                                                          local.get 0
                                                                                                          i32.load offset=4
                                                                                                          call $PLCMethods::LOGIC_OR_RuntimeStack*_
                                                                                                          drop
                                                                                                          i32.const 0
                                                                                                          return
                                                                                                        end
                                                                                                        local.get 0
                                                                                                        i32.load offset=4
                                                                                                        call $PLCMethods::LOGIC_NOT_RuntimeStack*_
                                                                                                        drop
                                                                                                        i32.const 0
                                                                                                        return
                                                                                                      end
                                                                                                      local.get 0
                                                                                                      i32.load offset=4
                                                                                                      call $PLCMethods::LOGIC_XOR_RuntimeStack*_
                                                                                                      drop
                                                                                                      i32.const 0
                                                                                                      return
                                                                                                    end
                                                                                                    local.get 0
                                                                                                    i32.load offset=4
                                                                                                    local.get 1
                                                                                                    local.get 2
                                                                                                    local.get 3
                                                                                                    call $PLCMethods::PUT_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                                                    return
                                                                                                  end
                                                                                                  local.get 0
                                                                                                  i32.load offset=4
                                                                                                  local.get 1
                                                                                                  local.get 2
                                                                                                  local.get 3
                                                                                                  call $PLCMethods::GET_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                                                  return
                                                                                                end
                                                                                                local.get 2
                                                                                                local.get 1
                                                                                                local.get 2
                                                                                                local.get 3
                                                                                                call $PLCMethods::handle_JMP_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                                                return
                                                                                              end
                                                                                              local.get 0
                                                                                              i32.load offset=4
                                                                                              local.get 1
                                                                                              local.get 2
                                                                                              local.get 3
                                                                                              call $PLCMethods::handle_JMP_IF_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                                              return
                                                                                            end
                                                                                            local.get 0
                                                                                            i32.load offset=4
                                                                                            local.get 1
                                                                                            local.get 2
                                                                                            local.get 3
                                                                                            call $PLCMethods::handle_JMP_IF_NOT_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                                            return
                                                                                          end
                                                                                          local.get 0
                                                                                          i32.load offset=4
                                                                                          local.get 1
                                                                                          local.get 2
                                                                                          local.get 3
                                                                                          call $PLCMethods::handle_CALL_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                                          return
                                                                                        end
                                                                                        local.get 0
                                                                                        i32.load offset=4
                                                                                        local.get 1
                                                                                        local.get 2
                                                                                        local.get 3
                                                                                        call $PLCMethods::handle_CALL_IF_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                                        return
                                                                                      end
                                                                                      local.get 0
                                                                                      i32.load offset=4
                                                                                      local.get 1
                                                                                      local.get 2
                                                                                      local.get 3
                                                                                      call $PLCMethods::handle_CALL_IF_NOT_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                                      return
                                                                                    end
                                                                                    local.get 0
                                                                                    i32.load offset=4
                                                                                    local.get 1
                                                                                    local.get 2
                                                                                    local.get 3
                                                                                    call $PLCMethods::handle_RET_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                                    return
                                                                                  end
                                                                                  local.get 0
                                                                                  i32.load offset=4
                                                                                  local.get 1
                                                                                  local.get 2
                                                                                  local.get 3
                                                                                  call $PLCMethods::PUSH_bool_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                                  return
                                                                                end
                                                                                local.get 0
                                                                                i32.load offset=4
                                                                                local.get 1
                                                                                local.get 2
                                                                                local.get 3
                                                                                call $PLCMethods::PUSH_uint8_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                                return
                                                                              end
                                                                              local.get 0
                                                                              i32.load offset=4
                                                                              local.get 1
                                                                              local.get 2
                                                                              local.get 3
                                                                              call $PLCMethods::PUSH_int8_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                              return
                                                                            end
                                                                            local.get 0
                                                                            i32.load offset=4
                                                                            local.get 1
                                                                            local.get 2
                                                                            local.get 3
                                                                            call $PLCMethods::PUSH_uint16_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                            return
                                                                          end
                                                                          local.get 0
                                                                          i32.load offset=4
                                                                          local.get 1
                                                                          local.get 2
                                                                          local.get 3
                                                                          call $PLCMethods::PUSH_int16_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                          return
                                                                        end
                                                                        local.get 0
                                                                        i32.load offset=4
                                                                        local.get 1
                                                                        local.get 2
                                                                        local.get 3
                                                                        call $PLCMethods::PUSH_uint32_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                        return
                                                                      end
                                                                      local.get 0
                                                                      i32.load offset=4
                                                                      local.get 1
                                                                      local.get 2
                                                                      local.get 3
                                                                      call $PLCMethods::PUSH_int32_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                      return
                                                                    end
                                                                    local.get 0
                                                                    i32.load offset=4
                                                                    local.get 1
                                                                    local.get 2
                                                                    local.get 3
                                                                    call $PLCMethods::PUSH_uint64_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                    return
                                                                  end
                                                                  local.get 0
                                                                  i32.load offset=4
                                                                  local.get 1
                                                                  local.get 2
                                                                  local.get 3
                                                                  call $PLCMethods::PUSH_int64_t_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                  return
                                                                end
                                                                local.get 0
                                                                i32.load offset=4
                                                                local.get 1
                                                                local.get 2
                                                                local.get 3
                                                                call $PLCMethods::PUSH_float_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                                return
                                                              end
                                                              local.get 0
                                                              i32.load offset=4
                                                              local.get 1
                                                              local.get 2
                                                              local.get 3
                                                              call $PLCMethods::PUSH_double_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                              return
                                                            end
                                                            local.get 0
                                                            i32.load offset=4
                                                            local.get 1
                                                            local.get 2
                                                            local.get 3
                                                            call $PLCMethods::handle_ADD_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                            return
                                                          end
                                                          local.get 0
                                                          i32.load offset=4
                                                          local.get 1
                                                          local.get 2
                                                          local.get 3
                                                          call $PLCMethods::handle_SUB_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                          return
                                                        end
                                                        local.get 0
                                                        i32.load offset=4
                                                        local.get 1
                                                        local.get 2
                                                        local.get 3
                                                        call $PLCMethods::handle_MUL_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                        return
                                                      end
                                                      local.get 0
                                                      i32.load offset=4
                                                      local.get 1
                                                      local.get 2
                                                      local.get 3
                                                      call $PLCMethods::handle_DIV_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                                                      return
                                                    end
                                                    local.get 0
                                                    i32.load offset=4
                                                    call $PLCMethods::BW_AND_X8_RuntimeStack*_
                                                    drop
                                                    i32.const 0
                                                    return
                                                  end
                                                  local.get 0
                                                  i32.load offset=4
                                                  call $PLCMethods::BW_AND_X16_RuntimeStack*_
                                                  drop
                                                  i32.const 0
                                                  return
                                                end
                                                local.get 0
                                                i32.load offset=4
                                                call $PLCMethods::BW_AND_X32_RuntimeStack*_
                                                drop
                                                i32.const 0
                                                return
                                              end
                                              local.get 0
                                              i32.load offset=4
                                              call $PLCMethods::BW_AND_X64_RuntimeStack*_
                                              drop
                                              i32.const 0
                                              return
                                            end
                                            local.get 0
                                            i32.load offset=4
                                            call $PLCMethods::BW_OR_X8_RuntimeStack*_
                                            drop
                                            i32.const 0
                                            return
                                          end
                                          local.get 0
                                          i32.load offset=4
                                          call $PLCMethods::BW_OR_X16_RuntimeStack*_
                                          drop
                                          i32.const 0
                                          return
                                        end
                                        local.get 0
                                        i32.load offset=4
                                        call $PLCMethods::BW_OR_X32_RuntimeStack*_
                                        drop
                                        i32.const 0
                                        return
                                      end
                                      local.get 0
                                      i32.load offset=4
                                      call $PLCMethods::BW_OR_X64_RuntimeStack*_
                                      drop
                                      i32.const 0
                                      return
                                    end
                                    local.get 0
                                    i32.load offset=4
                                    call $PLCMethods::BW_XOR_X8_RuntimeStack*_
                                    drop
                                    i32.const 0
                                    return
                                  end
                                  local.get 0
                                  i32.load offset=4
                                  call $PLCMethods::BW_XOR_X16_RuntimeStack*_
                                  drop
                                  i32.const 0
                                  return
                                end
                                local.get 0
                                i32.load offset=4
                                call $PLCMethods::BW_XOR_X32_RuntimeStack*_
                                drop
                                i32.const 0
                                return
                              end
                              local.get 0
                              i32.load offset=4
                              call $PLCMethods::BW_XOR_X64_RuntimeStack*_
                              drop
                              i32.const 0
                              return
                            end
                            local.get 0
                            i32.load offset=4
                            call $PLCMethods::BW_NOT_X8_RuntimeStack*_
                            drop
                            i32.const 0
                            return
                          end
                          local.get 0
                          i32.load offset=4
                          call $PLCMethods::BW_NOT_X16_RuntimeStack*_
                          drop
                          i32.const 0
                          return
                        end
                        local.get 0
                        i32.load offset=4
                        call $PLCMethods::BW_NOT_X32_RuntimeStack*_
                        drop
                        i32.const 0
                        return
                      end
                      local.get 0
                      i32.load offset=4
                      call $PLCMethods::BW_NOT_X64_RuntimeStack*_
                      drop
                      i32.const 0
                      return
                    end
                    local.get 0
                    i32.load offset=4
                    local.get 1
                    local.get 2
                    local.get 3
                    call $PLCMethods::handle_CMP_EQ_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                    return
                  end
                  local.get 0
                  i32.load offset=4
                  local.get 1
                  local.get 2
                  local.get 3
                  call $PLCMethods::handle_CMP_NEQ_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                  return
                end
                local.get 0
                i32.load offset=4
                local.get 1
                local.get 2
                local.get 3
                call $PLCMethods::handle_CMP_GT_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
                return
              end
              local.get 0
              i32.load offset=4
              local.get 1
              local.get 2
              local.get 3
              call $PLCMethods::handle_CMP_GTE_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
              return
            end
            local.get 0
            i32.load offset=4
            local.get 1
            local.get 2
            local.get 3
            call $PLCMethods::handle_CMP_LT_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
            return
          end
          local.get 0
          i32.load offset=4
          local.get 1
          local.get 2
          local.get 3
          call $PLCMethods::handle_CMP_LTE_RuntimeStack*__unsigned_char*__unsigned_short__unsigned_short&_
          return
        end
        i32.const 16
        return
      end
      i32.const 1
      local.set 4
    end
    local.get 4)
  (func $print__uint64_t_unsigned_long_long_ (type 12) (param i64)
    (local i32 i32 f64 f64 f64 f64 f64 i32 i32 i64)
    global.get $__stack_pointer
    local.tee 1
    local.set 2
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        f64.convert_i64_u
        local.tee 3
        f64.const -0x1p+0 (;=-1;)
        f64.add
        local.get 3
        f64.const 0x1p+0 (;=1;)
        f64.add
        f64.div
        local.tee 3
        f64.const 0x0p+0 (;=0;)
        f64.ne
        br_if 0 (;@2;)
        local.get 3
        local.set 4
        br 1 (;@1;)
      end
      local.get 3
      local.get 3
      f64.mul
      local.set 5
      local.get 3
      local.set 6
      f64.const 0x1p+0 (;=1;)
      local.set 7
      loop  ;; label = @2
        local.get 3
        local.get 5
        local.get 6
        f64.mul
        local.tee 6
        local.get 7
        f64.const 0x1p+1 (;=2;)
        f64.add
        local.tee 7
        f64.div
        f64.add
        local.tee 4
        local.get 3
        f64.ne
        local.set 8
        local.get 4
        local.set 3
        local.get 8
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 4
        local.get 4
        f64.add
        f64.const 0x1.26bb1bbb55516p+1 (;=2.30259;)
        f64.div
        f64.const 0x1p+0 (;=1;)
        f64.add
        local.tee 3
        f64.const 0x1p+32 (;=4.29497e+09;)
        f64.lt
        local.get 3
        f64.const 0x0p+0 (;=0;)
        f64.ge
        i32.and
        i32.eqz
        br_if 0 (;@2;)
        local.get 3
        i32.trunc_f64_u
        local.set 8
        br 1 (;@1;)
      end
      i32.const 0
      local.set 8
    end
    local.get 1
    local.get 8
    i32.const 16
    i32.add
    i32.const 131056
    i32.and
    i32.sub
    local.tee 9
    global.set $__stack_pointer
    local.get 9
    local.get 8
    i32.add
    i32.const 0
    i32.store8
    block  ;; label = @1
      local.get 8
      i32.eqz
      br_if 0 (;@1;)
      local.get 9
      i32.const -1
      i32.add
      local.set 1
      loop  ;; label = @2
        local.get 1
        local.get 8
        i32.add
        local.get 0
        local.get 0
        i64.const 10
        i64.div_u
        local.tee 10
        i64.const 10
        i64.mul
        i64.sub
        i32.wrap_i64
        i32.const 48
        i32.or
        i32.store8
        local.get 10
        local.set 0
        local.get 8
        i32.const -1
        i32.add
        local.tee 8
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      local.get 9
      i32.load8_u
      local.tee 8
      i32.eqz
      br_if 0 (;@1;)
      local.get 9
      i32.const 1
      i32.add
      local.set 1
      loop  ;; label = @2
        local.get 8
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 1
        i32.load8_u
        local.set 8
        local.get 1
        i32.const 1
        i32.add
        local.set 1
        local.get 8
        br_if 0 (;@2;)
      end
    end
    local.get 2
    global.set $__stack_pointer)
  (func $UnitTest::UnitTest__ (type 2) (param i32) (result i32)
    (local i32)
    local.get 0
    i64.const 0
    i64.store align=4
    local.get 0
    i32.const 12
    i32.store offset=12
    local.get 0
    i32.const 8
    i32.add
    i32.const 0
    i32.store16
    local.get 0
    i32.const 64
    i32.store16 offset=4
    i32.const 64
    call $operator_new___unsigned_long_
    local.set 1
    local.get 0
    i32.const 12
    i32.store offset=12
    local.get 0
    i32.const 0
    i32.store offset=6 align=2
    local.get 0
    local.get 1
    i32.store
    local.get 0
    i32.const 16
    i32.add
    i32.const 32
    i32.const 16
    local.get 0
    call $VovkPLCRuntime::VovkPLCRuntime_unsigned_short__unsigned_short__RuntimeProgram&_
    drop
    local.get 0
    i32.const 64
    i32.store16 offset=4
    i32.const 64
    call $operator_new___unsigned_long_
    local.set 1
    local.get 0
    i32.const 12
    i32.store offset=12
    local.get 0
    i32.const 0
    i32.store offset=6 align=2
    local.get 0
    local.get 1
    i32.store
    block  ;; label = @1
      local.get 0
      i32.const 28
      i32.add
      i32.load
      local.tee 1
      i32.eqz
      br_if 0 (;@1;)
      local.get 1
      call $operator_delete___void*_
    end
    local.get 0
    i32.const 32
    i32.add
    i32.const 16
    i32.store
    i32.const 16
    call $operator_new___unsigned_long_
    local.set 1
    local.get 0
    i32.const 36
    i32.add
    i32.const 16
    i32.store
    local.get 0
    local.get 1
    i32.store offset=28
    local.get 0)
  (func $VovkPLCRuntime::VovkPLCRuntime_unsigned_short__unsigned_short__RuntimeProgram&_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32)
    local.get 0
    i64.const 0
    i64.store offset=12 align=4
    local.get 0
    i32.const 0
    i32.store16 offset=8
    local.get 0
    i32.const 0
    i32.store8
    local.get 0
    i32.const 20
    i32.add
    local.tee 4
    i32.const 0
    i32.store
    i32.const 0
    call $operator_new___unsigned_long_
    local.set 5
    local.get 4
    i64.const 0
    i64.store align=4
    local.get 0
    local.get 5
    i32.store offset=12
    i32.const 40
    call $operator_new_unsigned_long_
    local.get 1
    i32.const 10
    i32.const 4
    call $RuntimeStack::RuntimeStack_unsigned_short__unsigned_short__unsigned_short_
    local.set 4
    local.get 0
    local.get 3
    i32.store offset=24
    local.get 0
    local.get 1
    i32.store16 offset=8
    local.get 0
    local.get 4
    i32.store offset=4
    block  ;; label = @1
      local.get 2
      i32.eqz
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 0
        i32.const 12
        i32.add
        i32.load
        local.tee 1
        i32.eqz
        br_if 0 (;@2;)
        local.get 1
        call $operator_delete___void*_
      end
      local.get 0
      i32.const 16
      i32.add
      local.get 2
      i32.store
      local.get 2
      call $operator_new___unsigned_long_
      local.set 1
      local.get 0
      i32.const 20
      i32.add
      local.get 2
      i32.store
      local.get 0
      local.get 1
      i32.store offset=12
    end
    local.get 0)
  (func $RuntimeProgram::push_uint8_t_unsigned_char_ (type 13) (param i32 i32) (result i32)
    (local i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        local.tee 3
        i32.const 2
        i32.add
        local.get 0
        i32.load16_u offset=4
        i32.le_u
        br_if 0 (;@2;)
        local.get 0
        i32.const 14
        i32.store offset=12
        i32.const 80
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3890
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load16_u offset=6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 2
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 4
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 4
            i32.load8_u
            local.set 3
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 0
        i32.load offset=12
        local.set 5
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.const 2
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 1
      i32.add
      local.get 1
      i32.store8
      i32.const 80
      local.set 4
      i32.const 1
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 3654
        i32.add
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.tee 5
        local.set 3
        local.get 5
        i32.const 17
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      local.get 1
      i32.store offset=16
      i32.const 4400
      i32.const 1466
      local.get 2
      i32.const 16
      i32.add
      call $sprintf_
      drop
      i32.const 0
      local.set 5
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 3
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 4
        loop  ;; label = @3
          local.get 3
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 4
          i32.load8_u
          local.set 3
          local.get 4
          i32.const 1
          i32.add
          local.set 4
          local.get 3
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
      local.get 0
      i32.const 0
      i32.store offset=12
      local.get 0
      local.get 0
      i32.load16_u offset=6
      i32.const 2
      i32.add
      i32.store16 offset=6
    end
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer
    local.get 5)
  (func $RuntimeProgram::push_unsigned_char__unsigned_char_ (type 3) (param i32 i32 i32) (result i32)
    (local i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 48
    i32.sub
    local.tee 3
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        local.tee 4
        i32.const 2
        i32.add
        local.get 0
        i32.load16_u offset=4
        i32.le_u
        br_if 0 (;@2;)
        local.get 0
        i32.const 14
        i32.store offset=12
        i32.const 80
        local.set 5
        i32.const 1
        local.set 4
        loop  ;; label = @3
          local.get 5
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 4
          i32.const 3890
          i32.add
          i32.load8_u
          local.set 5
          local.get 4
          i32.const 1
          i32.add
          local.tee 6
          local.set 4
          local.get 6
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 3
        local.get 0
        i32.load16_u offset=6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 3
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 4
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 5
          loop  ;; label = @4
            local.get 4
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 5
            i32.load8_u
            local.set 4
            local.get 5
            i32.const 1
            i32.add
            local.set 5
            local.get 4
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 0
        i32.load offset=12
        local.set 6
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 4
      i32.add
      local.get 1
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 1
      i32.add
      local.get 2
      i32.store8
      i32.const 80
      local.set 5
      i32.const 1
      local.set 4
      loop  ;; label = @2
        local.get 5
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.const 3819
        i32.add
        i32.load8_u
        local.set 5
        local.get 4
        i32.const 1
        i32.add
        local.tee 6
        local.set 4
        local.get 6
        i32.const 21
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 3
      local.get 1
      i32.store offset=32
      i32.const 4400
      i32.const 1466
      local.get 3
      i32.const 32
      i32.add
      call $sprintf_
      drop
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 4
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 5
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 5
          i32.load8_u
          local.set 4
          local.get 5
          i32.const 1
          i32.add
          local.set 5
          local.get 4
          br_if 0 (;@3;)
        end
      end
      i32.const 32
      local.set 5
      i32.const 1
      local.set 4
      loop  ;; label = @2
        local.get 5
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.const 3866
        i32.add
        i32.load8_u
        local.set 5
        local.get 4
        i32.const 1
        i32.add
        local.tee 6
        local.set 4
        local.get 6
        i32.const 10
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 3
      local.get 2
      i32.store offset=16
      i32.const 4400
      i32.const 1466
      local.get 3
      i32.const 16
      i32.add
      call $sprintf_
      drop
      i32.const 0
      local.set 6
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 4
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 5
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 5
          i32.load8_u
          local.set 4
          local.get 5
          i32.const 1
          i32.add
          local.set 5
          local.get 4
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
      local.get 0
      i32.const 0
      i32.store offset=12
      local.get 0
      local.get 0
      i32.load16_u offset=6
      i32.const 2
      i32.add
      i32.store16 offset=6
    end
    local.get 3
    i32.const 48
    i32.add
    global.set $__stack_pointer
    local.get 6)
  (func $RuntimeProgram::push_uint16_t_unsigned_short_ (type 13) (param i32 i32) (result i32)
    (local i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        local.tee 3
        i32.const 3
        i32.add
        local.get 0
        i32.load16_u offset=4
        i32.le_u
        br_if 0 (;@2;)
        local.get 0
        i32.const 14
        i32.store offset=12
        i32.const 80
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3890
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load16_u offset=6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 2
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 4
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 4
            i32.load8_u
            local.set 3
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 0
        i32.load offset=12
        local.set 5
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.const 3
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 1
      i32.add
      local.get 1
      i32.const 8
      i32.shr_u
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 2
      i32.add
      local.get 1
      i32.store8
      i32.const 80
      local.set 4
      i32.const 1
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 3687
        i32.add
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.tee 5
        local.set 3
        local.get 5
        i32.const 18
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      local.get 1
      i32.store offset=16
      i32.const 4400
      i32.const 1466
      local.get 2
      i32.const 16
      i32.add
      call $sprintf_
      drop
      i32.const 0
      local.set 5
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 3
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 4
        loop  ;; label = @3
          local.get 3
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 4
          i32.load8_u
          local.set 3
          local.get 4
          i32.const 1
          i32.add
          local.set 4
          local.get 3
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
      local.get 0
      i32.const 0
      i32.store offset=12
      local.get 0
      local.get 0
      i32.load16_u offset=6
      i32.const 3
      i32.add
      i32.store16 offset=6
    end
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer
    local.get 5)
  (func $RuntimeProgram::push_uint32_t_unsigned_int_ (type 13) (param i32 i32) (result i32)
    (local i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        local.tee 3
        i32.const 5
        i32.add
        local.get 0
        i32.load16_u offset=4
        i32.le_u
        br_if 0 (;@2;)
        local.get 0
        i32.const 14
        i32.store offset=12
        i32.const 80
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3890
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load16_u offset=6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 2
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 4
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 4
            i32.load8_u
            local.set 3
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 0
        i32.load offset=12
        local.set 5
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.const 4
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 1
      i32.add
      local.get 1
      i32.const 24
      i32.shr_u
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 2
      i32.add
      local.get 1
      i32.const 16
      i32.shr_u
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 3
      i32.add
      local.get 1
      i32.const 8
      i32.shr_u
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 4
      i32.add
      local.get 1
      i32.store8
      i32.const 80
      local.set 4
      i32.const 1
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 3705
        i32.add
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.tee 5
        local.set 3
        local.get 5
        i32.const 18
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      local.get 1
      i32.store offset=16
      i32.const 4400
      i32.const 1109
      local.get 2
      i32.const 16
      i32.add
      call $sprintf_
      drop
      i32.const 0
      local.set 5
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 3
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 4
        loop  ;; label = @3
          local.get 3
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 4
          i32.load8_u
          local.set 3
          local.get 4
          i32.const 1
          i32.add
          local.set 4
          local.get 3
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
      local.get 0
      i32.const 0
      i32.store offset=12
      local.get 0
      local.get 0
      i32.load16_u offset=6
      i32.const 5
      i32.add
      i32.store16 offset=6
    end
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer
    local.get 5)
  (func $RuntimeProgram::push_uint64_t_unsigned_long_long_ (type 10) (param i32 i64) (result i32)
    (local i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        local.tee 3
        i32.const 9
        i32.add
        local.get 0
        i32.load16_u offset=4
        i32.le_u
        br_if 0 (;@2;)
        local.get 0
        i32.const 14
        i32.store offset=12
        i32.const 80
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3890
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load16_u offset=6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 2
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 4
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 4
            i32.load8_u
            local.set 3
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 0
        i32.load offset=12
        local.set 3
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.const 5
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 1
      i32.add
      local.get 1
      i64.const 56
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 2
      i32.add
      local.get 1
      i64.const 48
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 3
      i32.add
      local.get 1
      i64.const 40
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 4
      i32.add
      local.get 1
      i64.const 32
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 5
      i32.add
      local.get 1
      i64.const 24
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 6
      i32.add
      local.get 1
      i64.const 16
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 7
      i32.add
      local.get 1
      i64.const 8
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 8
      i32.add
      local.get 1
      i64.store8
      i32.const 0
      local.set 3
      local.get 0
      i32.const 0
      i32.store offset=12
      local.get 0
      local.get 0
      i32.load16_u offset=6
      i32.const 9
      i32.add
      i32.store16 offset=6
    end
    local.get 2
    i32.const 16
    i32.add
    global.set $__stack_pointer
    local.get 3)
  (func $RuntimeProgram::push_int8_t_signed_char_ (type 13) (param i32 i32) (result i32)
    (local i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        local.tee 3
        i32.const 2
        i32.add
        local.get 0
        i32.load16_u offset=4
        i32.le_u
        br_if 0 (;@2;)
        local.get 0
        i32.const 14
        i32.store offset=12
        i32.const 80
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3890
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load16_u offset=6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 2
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 4
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 4
            i32.load8_u
            local.set 3
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 0
        i32.load offset=12
        local.set 5
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.const 6
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 1
      i32.add
      local.get 1
      i32.store8
      i32.const 80
      local.set 4
      i32.const 1
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 3671
        i32.add
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.tee 5
        local.set 3
        local.get 5
        i32.const 16
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      local.get 1
      i32.store offset=16
      i32.const 4400
      i32.const 1466
      local.get 2
      i32.const 16
      i32.add
      call $sprintf_
      drop
      i32.const 0
      local.set 5
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 3
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 4
        loop  ;; label = @3
          local.get 3
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 4
          i32.load8_u
          local.set 3
          local.get 4
          i32.const 1
          i32.add
          local.set 4
          local.get 3
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
      local.get 0
      i32.const 0
      i32.store offset=12
      local.get 0
      local.get 0
      i32.load16_u offset=6
      i32.const 2
      i32.add
      i32.store16 offset=6
    end
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer
    local.get 5)
  (func $RuntimeProgram::push_float_float_ (type 14) (param i32 f32) (result i32)
    (local i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        local.tee 3
        i32.const 5
        i32.add
        local.get 0
        i32.load16_u offset=4
        i32.le_u
        br_if 0 (;@2;)
        local.get 0
        i32.const 14
        i32.store offset=12
        i32.const 80
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3890
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load16_u offset=6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 2
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 4
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 4
            i32.load8_u
            local.set 3
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 0
        i32.load offset=12
        local.set 3
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.const 10
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 1
      i32.add
      local.get 1
      i32.reinterpret_f32
      local.tee 3
      i32.const 24
      i32.shr_u
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 2
      i32.add
      local.get 3
      i32.const 16
      i32.shr_u
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 3
      i32.add
      local.get 3
      i32.const 8
      i32.shr_u
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 4
      i32.add
      local.get 3
      i32.store8
      i32.const 0
      local.set 3
      local.get 0
      i32.const 0
      i32.store offset=12
      local.get 0
      local.get 0
      i32.load16_u offset=6
      i32.const 5
      i32.add
      i32.store16 offset=6
    end
    local.get 2
    i32.const 16
    i32.add
    global.set $__stack_pointer
    local.get 3)
  (func $RuntimeProgram::push_double_double_ (type 15) (param i32 f64) (result i32)
    (local i32 i32 i32 i32 i64)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        local.tee 3
        i32.const 9
        i32.add
        local.get 0
        i32.load16_u offset=4
        i32.le_u
        br_if 0 (;@2;)
        local.get 0
        i32.const 14
        i32.store offset=12
        i32.const 80
        local.set 4
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 4
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3890
          i32.add
          i32.load8_u
          local.set 4
          local.get 3
          i32.const 1
          i32.add
          local.tee 5
          local.set 3
          local.get 5
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load16_u offset=6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 2
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 4
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 4
            i32.load8_u
            local.set 3
            local.get 4
            i32.const 1
            i32.add
            local.set 4
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 0
        i32.load offset=12
        local.set 3
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.const 11
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 1
      i32.add
      local.get 1
      i64.reinterpret_f64
      local.tee 6
      i64.const 56
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 2
      i32.add
      local.get 6
      i64.const 48
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 3
      i32.add
      local.get 6
      i64.const 40
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 4
      i32.add
      local.get 6
      i64.const 32
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 5
      i32.add
      local.get 6
      i64.const 24
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 6
      i32.add
      local.get 6
      i64.const 16
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 7
      i32.add
      local.get 6
      i64.const 8
      i64.shr_u
      i64.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 8
      i32.add
      local.get 6
      i64.store8
      i32.const 0
      local.set 3
      local.get 0
      i32.const 0
      i32.store offset=12
      local.get 0
      local.get 0
      i32.load16_u offset=6
      i32.const 9
      i32.add
      i32.store16 offset=6
    end
    local.get 2
    i32.const 16
    i32.add
    global.set $__stack_pointer
    local.get 3)
  (func $RuntimeProgram::push_unsigned_char_ (type 13) (param i32 i32) (result i32)
    (local i32 i32 i32)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        local.tee 3
        local.get 0
        i32.load16_u offset=4
        i32.lt_u
        br_if 0 (;@2;)
        local.get 0
        i32.const 14
        i32.store offset=12
        i32.const 80
        local.set 3
        i32.const 1
        local.set 1
        loop  ;; label = @3
          local.get 3
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 1
          i32.const 3890
          i32.add
          i32.load8_u
          local.set 3
          local.get 1
          i32.const 1
          i32.add
          local.tee 4
          local.set 1
          local.get 4
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load16_u offset=6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 2
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 1
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 3
          loop  ;; label = @4
            local.get 1
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 3
            i32.load8_u
            local.set 1
            local.get 3
            i32.const 1
            i32.add
            local.set 3
            local.get 1
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 0
        i32.load offset=12
        local.set 1
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 3
      i32.add
      local.get 1
      i32.store8
      i32.const 0
      local.set 1
      local.get 0
      i32.const 0
      i32.store offset=12
      local.get 0
      local.get 0
      i32.load16_u offset=6
      i32.const 1
      i32.add
      i32.store16 offset=6
    end
    local.get 2
    i32.const 16
    i32.add
    global.set $__stack_pointer
    local.get 1)
  (func $case_jump_build__ (type 4)
    (local i32 i32)
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 0
    i32.load16_u offset=4734
    local.set 0
    i32.const 4728
    i32.const 0
    call $RuntimeProgram::pushJMP_unsigned_short_
    drop
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 32
    i32.const 2
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 3
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 34
    i32.const 2
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 0
    i32.load16_u offset=4734
    local.set 1
    i32.const 4728
    i32.const 255
    call $RuntimeProgram::push_unsigned_char_
    drop
    block  ;; label = @1
      local.get 0
      i32.const 1
      i32.add
      i32.const 65535
      i32.and
      local.tee 0
      i32.const 2
      i32.add
      i32.const 0
      i32.load16_u offset=4734
      i32.gt_u
      br_if 0 (;@1;)
      i32.const 0
      i32.load offset=4728
      local.get 0
      i32.add
      local.get 1
      i32.const 8
      i32.shr_u
      i32.store8
      local.get 0
      i32.const 0
      i32.load offset=4728
      i32.add
      i32.const 1
      i32.add
      local.get 1
      i32.store8
    end)
  (func $RuntimeProgram::pushJMP_unsigned_short_ (type 13) (param i32 i32) (result i32)
    (local i32 i32 i32)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        local.tee 3
        i32.const 3
        i32.add
        local.get 0
        i32.load16_u offset=4
        i32.le_u
        br_if 0 (;@2;)
        local.get 0
        i32.const 14
        i32.store offset=12
        i32.const 80
        local.set 3
        i32.const 1
        local.set 1
        loop  ;; label = @3
          local.get 3
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 1
          i32.const 3890
          i32.add
          i32.load8_u
          local.set 3
          local.get 1
          i32.const 1
          i32.add
          local.tee 4
          local.set 1
          local.get 4
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load16_u offset=6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 2
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 1
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 3
          loop  ;; label = @4
            local.get 1
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 3
            i32.load8_u
            local.set 1
            local.get 3
            i32.const 1
            i32.add
            local.set 3
            local.get 1
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 0
        i32.load offset=12
        local.set 1
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.const 192
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 1
      i32.add
      local.get 1
      i32.const 8
      i32.shr_u
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 2
      i32.add
      local.get 1
      i32.store8
      i32.const 0
      local.set 1
      local.get 0
      i32.const 0
      i32.store offset=12
      local.get 0
      local.get 0
      i32.load16_u offset=6
      i32.const 3
      i32.add
      i32.store16 offset=6
    end
    local.get 2
    i32.const 16
    i32.add
    global.set $__stack_pointer
    local.get 1)
  (func $case_jump_if_build__ (type 4)
    (local i32 i32)
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 0
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 0
    i32.const 2
    call $RuntimeProgram::pushPUT_unsigned_short__PLCRuntimeInstructionSet_
    drop
    i32.const 4728
    i32.const 0
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 1
    i32.const 2
    call $RuntimeProgram::pushPUT_unsigned_short__PLCRuntimeInstructionSet_
    drop
    i32.const 0
    i32.load16_u offset=4734
    local.set 0
    i32.const 4728
    i32.const 1
    i32.const 2
    call $RuntimeProgram::pushGET_unsigned_short__PLCRuntimeInstructionSet_
    drop
    i32.const 4728
    i32.const 10
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 163
    i32.const 2
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 0
    i32.load16_u offset=4734
    local.set 1
    i32.const 4728
    i32.const 0
    call $RuntimeProgram::pushJMP_IF_NOT_unsigned_short_
    drop
    i32.const 4728
    i32.const 1
    i32.const 2
    call $RuntimeProgram::pushGET_unsigned_short__PLCRuntimeInstructionSet_
    drop
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 32
    i32.const 2
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 1
    i32.const 2
    call $RuntimeProgram::pushPUT_unsigned_short__PLCRuntimeInstructionSet_
    drop
    i32.const 4728
    i32.const 0
    i32.const 2
    call $RuntimeProgram::pushGET_unsigned_short__PLCRuntimeInstructionSet_
    drop
    i32.const 4728
    i32.const 10
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 32
    i32.const 2
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 0
    i32.const 2
    call $RuntimeProgram::pushPUT_unsigned_short__PLCRuntimeInstructionSet_
    drop
    i32.const 4728
    local.get 0
    call $RuntimeProgram::pushJMP_unsigned_short_
    drop
    i32.const 0
    i32.load16_u offset=4734
    local.set 0
    i32.const 4728
    i32.const 0
    i32.const 2
    call $RuntimeProgram::pushGET_unsigned_short__PLCRuntimeInstructionSet_
    drop
    i32.const 4728
    i32.const 255
    call $RuntimeProgram::push_unsigned_char_
    drop
    block  ;; label = @1
      local.get 1
      i32.const 1
      i32.add
      i32.const 65535
      i32.and
      local.tee 1
      i32.const 2
      i32.add
      i32.const 0
      i32.load16_u offset=4734
      i32.gt_u
      br_if 0 (;@1;)
      i32.const 0
      i32.load offset=4728
      local.get 1
      i32.add
      local.get 0
      i32.const 8
      i32.shr_u
      i32.store8
      local.get 1
      i32.const 0
      i32.load offset=4728
      i32.add
      i32.const 1
      i32.add
      local.get 0
      i32.store8
    end)
  (func $RuntimeProgram::pushPUT_unsigned_short__PLCRuntimeInstructionSet_ (type 3) (param i32 i32 i32) (result i32)
    (local i32 i32)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 3
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        local.tee 4
        i32.const 4
        i32.add
        local.get 0
        i32.load16_u offset=4
        i32.le_u
        br_if 0 (;@2;)
        local.get 0
        i32.const 14
        i32.store offset=12
        i32.const 80
        local.set 2
        i32.const 1
        local.set 1
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 1
          i32.const 3890
          i32.add
          i32.load8_u
          local.set 2
          local.get 1
          i32.const 1
          i32.add
          local.tee 4
          local.set 1
          local.get 4
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 3
        local.get 0
        i32.load16_u offset=6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 3
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 1
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 2
          loop  ;; label = @4
            local.get 1
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 2
            i32.load8_u
            local.set 1
            local.get 2
            i32.const 1
            i32.add
            local.set 2
            local.get 1
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 0
        i32.load offset=12
        local.set 1
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.const 17
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 1
      i32.add
      local.get 2
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 2
      i32.add
      local.get 1
      i32.const 8
      i32.shr_u
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 3
      i32.add
      local.get 1
      i32.store8
      i32.const 0
      local.set 1
      local.get 0
      i32.const 0
      i32.store offset=12
      local.get 0
      local.get 0
      i32.load16_u offset=6
      i32.const 4
      i32.add
      i32.store16 offset=6
    end
    local.get 3
    i32.const 16
    i32.add
    global.set $__stack_pointer
    local.get 1)
  (func $RuntimeProgram::pushGET_unsigned_short__PLCRuntimeInstructionSet_ (type 3) (param i32 i32 i32) (result i32)
    (local i32 i32)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 3
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        local.tee 4
        i32.const 4
        i32.add
        local.get 0
        i32.load16_u offset=4
        i32.le_u
        br_if 0 (;@2;)
        local.get 0
        i32.const 14
        i32.store offset=12
        i32.const 80
        local.set 2
        i32.const 1
        local.set 1
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 1
          i32.const 3890
          i32.add
          i32.load8_u
          local.set 2
          local.get 1
          i32.const 1
          i32.add
          local.tee 4
          local.set 1
          local.get 4
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 3
        local.get 0
        i32.load16_u offset=6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 3
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 1
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 2
          loop  ;; label = @4
            local.get 1
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 2
            i32.load8_u
            local.set 1
            local.get 2
            i32.const 1
            i32.add
            local.set 2
            local.get 1
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 0
        i32.load offset=12
        local.set 1
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 4
      i32.add
      i32.const 18
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 1
      i32.add
      local.get 2
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 2
      i32.add
      local.get 1
      i32.const 8
      i32.shr_u
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 3
      i32.add
      local.get 1
      i32.store8
      i32.const 0
      local.set 1
      local.get 0
      i32.const 0
      i32.store offset=12
      local.get 0
      local.get 0
      i32.load16_u offset=6
      i32.const 4
      i32.add
      i32.store16 offset=6
    end
    local.get 3
    i32.const 16
    i32.add
    global.set $__stack_pointer
    local.get 1)
  (func $RuntimeProgram::pushJMP_IF_NOT_unsigned_short_ (type 13) (param i32 i32) (result i32)
    (local i32 i32 i32)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        local.tee 3
        i32.const 3
        i32.add
        local.get 0
        i32.load16_u offset=4
        i32.le_u
        br_if 0 (;@2;)
        local.get 0
        i32.const 14
        i32.store offset=12
        i32.const 80
        local.set 3
        i32.const 1
        local.set 1
        loop  ;; label = @3
          local.get 3
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 1
          i32.const 3890
          i32.add
          i32.load8_u
          local.set 3
          local.get 1
          i32.const 1
          i32.add
          local.tee 4
          local.set 1
          local.get 4
          i32.const 24
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 2
        local.get 0
        i32.load16_u offset=6
        i32.store
        i32.const 4400
        i32.const 1466
        local.get 2
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 1
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 3
          loop  ;; label = @4
            local.get 1
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 3
            i32.load8_u
            local.set 1
            local.get 3
            i32.const 1
            i32.add
            local.set 3
            local.get 1
            br_if 0 (;@4;)
          end
        end
        i32.const 10
        call $__print
        local.get 0
        i32.load offset=12
        local.set 1
        br 1 (;@1;)
      end
      local.get 0
      i32.load
      local.get 3
      i32.add
      i32.const 194
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 1
      i32.add
      local.get 1
      i32.const 8
      i32.shr_u
      i32.store8
      local.get 0
      i32.load16_u offset=6
      local.get 0
      i32.load
      i32.add
      i32.const 2
      i32.add
      local.get 1
      i32.store8
      i32.const 0
      local.set 1
      local.get 0
      i32.const 0
      i32.store offset=12
      local.get 0
      local.get 0
      i32.load16_u offset=6
      i32.const 3
      i32.add
      i32.store16 offset=6
    end
    local.get 2
    i32.const 16
    i32.add
    global.set $__stack_pointer
    local.get 1)
  (func $runtime_unit_test__ (type 4)
    (local i32 i32 i32)
    i32.const 4744
    call $VovkPLCRuntime::startup__
    i32.const 0
    local.set 0
    loop  ;; label = @1
      i32.const 45
      call $__print
      local.get 0
      i32.const 255
      i32.and
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.set 0
      local.get 1
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 82
    local.set 1
    i32.const 1
    local.set 0
    loop  ;; label = @1
      local.get 1
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 0
      i32.const 1132
      i32.add
      i32.load8_u
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.tee 2
      local.set 0
      local.get 2
      i32.const 18
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 0
    local.set 0
    loop  ;; label = @1
      i32.const 45
      call $__print
      local.get 0
      i32.const 255
      i32.and
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.set 0
      local.get 1
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    call $logRuntimeErrorList__
    i32.const 0
    local.set 0
    loop  ;; label = @1
      i32.const 45
      call $__print
      local.get 0
      i32.const 255
      i32.and
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.set 0
      local.get 1
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    call $logRuntimeInstructionSet__
    i32.const 0
    local.set 0
    loop  ;; label = @1
      i32.const 45
      call $__print
      local.get 0
      i32.const 255
      i32.and
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.set 0
      local.get 1
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 0
    local.set 0
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 0
      i32.const 255
      i32.and
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.set 0
      local.get 1
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 69
    local.set 1
    i32.const 1
    local.set 0
    loop  ;; label = @1
      local.get 1
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 0
      i32.const 2932
      i32.add
      i32.load8_u
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.tee 2
      local.set 0
      local.get 2
      i32.const 32
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 0
    local.set 0
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 2
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 32
    i32.const 2
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 3
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 34
    i32.const 2
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4124
    call $void_UnitTest::run<unsigned_char>_TestCase<unsigned_char>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint16_t_unsigned_short_
    drop
    i32.const 4728
    i32.const 2
    call $RuntimeProgram::push_uint16_t_unsigned_short_
    drop
    i32.const 4728
    i32.const 32
    i32.const 3
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 3
    call $RuntimeProgram::push_uint16_t_unsigned_short_
    drop
    i32.const 4728
    i32.const 34
    i32.const 3
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4136
    call $void_UnitTest::run<unsigned_short>_TestCase<unsigned_short>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint32_t_unsigned_int_
    drop
    i32.const 4728
    i32.const 2
    call $RuntimeProgram::push_uint32_t_unsigned_int_
    drop
    i32.const 4728
    i32.const 32
    i32.const 4
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 3
    call $RuntimeProgram::push_uint32_t_unsigned_int_
    drop
    i32.const 4728
    i32.const 34
    i32.const 4
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4148
    call $void_UnitTest::run<unsigned_int>_TestCase<unsigned_int>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i64.const 1
    call $RuntimeProgram::push_uint64_t_unsigned_long_long_
    drop
    i32.const 4728
    i64.const 2
    call $RuntimeProgram::push_uint64_t_unsigned_long_long_
    drop
    i32.const 4728
    i32.const 32
    i32.const 5
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i64.const 3
    call $RuntimeProgram::push_uint64_t_unsigned_long_long_
    drop
    i32.const 4728
    i32.const 34
    i32.const 5
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4160
    call $void_UnitTest::run<unsigned_long_long>_TestCase<unsigned_long_long>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_int8_t_signed_char_
    drop
    i32.const 4728
    i32.const 2
    call $RuntimeProgram::push_int8_t_signed_char_
    drop
    i32.const 4728
    i32.const 33
    i32.const 6
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 3
    call $RuntimeProgram::push_int8_t_signed_char_
    drop
    i32.const 4728
    i32.const 34
    i32.const 6
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4176
    call $void_UnitTest::run<signed_char>_TestCase<signed_char>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    f32.const 0x1.99999ap-4 (;=0.1;)
    call $RuntimeProgram::push_float_float_
    drop
    i32.const 4728
    f32.const 0x1.99999ap-3 (;=0.2;)
    call $RuntimeProgram::push_float_float_
    drop
    i32.const 4728
    i32.const 32
    i32.const 10
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    f32.const -0x1p+0 (;=-1;)
    call $RuntimeProgram::push_float_float_
    drop
    i32.const 4728
    i32.const 34
    i32.const 10
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4188
    call $void_UnitTest::run<float>_TestCase<float>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    f64.const 0x1.999999999999ap-4 (;=0.1;)
    call $RuntimeProgram::push_double_double_
    drop
    i32.const 4728
    f64.const 0x1.999999999999ap-3 (;=0.2;)
    call $RuntimeProgram::push_double_double_
    drop
    i32.const 4728
    i32.const 32
    i32.const 11
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    f64.const -0x1p+0 (;=-1;)
    call $RuntimeProgram::push_double_double_
    drop
    i32.const 4728
    i32.const 34
    i32.const 11
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4200
    call $void_UnitTest::run<double>_TestCase<double>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 15
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 85
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 96
    call $RuntimeProgram::push_unsigned_char_
    drop
    i32.const 4728
    i32.const 4216
    call $void_UnitTest::run<unsigned_char>_TestCase<unsigned_char>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 255
    call $RuntimeProgram::push_uint16_t_unsigned_short_
    drop
    i32.const 4728
    i32.const 61455
    call $RuntimeProgram::push_uint16_t_unsigned_short_
    drop
    i32.const 4728
    i32.const 97
    call $RuntimeProgram::push_unsigned_char_
    drop
    i32.const 4728
    i32.const 4228
    call $void_UnitTest::run<unsigned_short>_TestCase<unsigned_short>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 128
    call $RuntimeProgram::push_unsigned_char_
    drop
    i32.const 4728
    i32.const 4240
    call $void_UnitTest::run<bool>_TestCase<bool>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 0
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 129
    call $RuntimeProgram::push_unsigned_char_
    drop
    i32.const 4728
    i32.const 4252
    call $void_UnitTest::run<bool>_TestCase<bool>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 160
    i32.const 1
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4264
    call $void_UnitTest::run<bool>_TestCase<bool>_const&_
    call $case_jump_build__
    i32.const 4728
    i32.const 4276
    call $void_UnitTest::run<unsigned_char>_TestCase<unsigned_char>_const&_
    call $case_jump_if_build__
    i32.const 4728
    i32.const 4288
    call $void_UnitTest::run<unsigned_char>_TestCase<unsigned_char>_const&_
    loop  ;; label = @1
      i32.const 45
      call $__print
      local.get 0
      i32.const 255
      i32.and
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.set 0
      local.get 1
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 0
    local.set 0
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 0
      i32.const 255
      i32.and
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.set 0
      local.get 1
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 82
    local.set 1
    i32.const 1
    local.set 0
    loop  ;; label = @1
      local.get 1
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 0
      i32.const 2902
      i32.add
      i32.load8_u
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.tee 2
      local.set 0
      local.get 2
      i32.const 30
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 0
    local.set 0
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 0
      i32.const 255
      i32.and
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.set 0
      local.get 1
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 0
    local.set 0
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 0
      i32.const 255
      i32.and
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.set 0
      local.get 1
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 82
    local.set 1
    i32.const 1
    local.set 0
    loop  ;; label = @1
      local.get 1
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 0
      i32.const 2124
      i32.add
      i32.load8_u
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.tee 2
      local.set 0
      local.get 2
      i32.const 8
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 0
    local.set 0
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 0
      i32.const 255
      i32.and
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.set 0
      local.get 1
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 0
    local.set 0
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 2
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 32
    i32.const 2
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 3
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 34
    i32.const 2
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4124
    call $void_UnitTest::review<unsigned_char>_TestCase<unsigned_char>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint16_t_unsigned_short_
    drop
    i32.const 4728
    i32.const 2
    call $RuntimeProgram::push_uint16_t_unsigned_short_
    drop
    i32.const 4728
    i32.const 32
    i32.const 3
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 3
    call $RuntimeProgram::push_uint16_t_unsigned_short_
    drop
    i32.const 4728
    i32.const 34
    i32.const 3
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4136
    call $void_UnitTest::review<unsigned_short>_TestCase<unsigned_short>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint32_t_unsigned_int_
    drop
    i32.const 4728
    i32.const 2
    call $RuntimeProgram::push_uint32_t_unsigned_int_
    drop
    i32.const 4728
    i32.const 32
    i32.const 4
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 3
    call $RuntimeProgram::push_uint32_t_unsigned_int_
    drop
    i32.const 4728
    i32.const 34
    i32.const 4
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4148
    call $void_UnitTest::review<unsigned_int>_TestCase<unsigned_int>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i64.const 1
    call $RuntimeProgram::push_uint64_t_unsigned_long_long_
    drop
    i32.const 4728
    i64.const 2
    call $RuntimeProgram::push_uint64_t_unsigned_long_long_
    drop
    i32.const 4728
    i32.const 32
    i32.const 5
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i64.const 3
    call $RuntimeProgram::push_uint64_t_unsigned_long_long_
    drop
    i32.const 4728
    i32.const 34
    i32.const 5
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4160
    call $void_UnitTest::review<unsigned_long_long>_TestCase<unsigned_long_long>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_int8_t_signed_char_
    drop
    i32.const 4728
    i32.const 2
    call $RuntimeProgram::push_int8_t_signed_char_
    drop
    i32.const 4728
    i32.const 33
    i32.const 6
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 3
    call $RuntimeProgram::push_int8_t_signed_char_
    drop
    i32.const 4728
    i32.const 34
    i32.const 6
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4176
    call $void_UnitTest::review<signed_char>_TestCase<signed_char>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    f32.const 0x1.99999ap-4 (;=0.1;)
    call $RuntimeProgram::push_float_float_
    drop
    i32.const 4728
    f32.const 0x1.99999ap-3 (;=0.2;)
    call $RuntimeProgram::push_float_float_
    drop
    i32.const 4728
    i32.const 32
    i32.const 10
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    f32.const -0x1p+0 (;=-1;)
    call $RuntimeProgram::push_float_float_
    drop
    i32.const 4728
    i32.const 34
    i32.const 10
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4188
    call $void_UnitTest::review<float>_TestCase<float>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    f64.const 0x1.999999999999ap-4 (;=0.1;)
    call $RuntimeProgram::push_double_double_
    drop
    i32.const 4728
    f64.const 0x1.999999999999ap-3 (;=0.2;)
    call $RuntimeProgram::push_double_double_
    drop
    i32.const 4728
    i32.const 32
    i32.const 11
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    f64.const -0x1p+0 (;=-1;)
    call $RuntimeProgram::push_double_double_
    drop
    i32.const 4728
    i32.const 34
    i32.const 11
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4200
    call $void_UnitTest::review<double>_TestCase<double>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 15
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 85
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 96
    call $RuntimeProgram::push_unsigned_char_
    drop
    i32.const 4728
    i32.const 4216
    call $void_UnitTest::review<unsigned_char>_TestCase<unsigned_char>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 255
    call $RuntimeProgram::push_uint16_t_unsigned_short_
    drop
    i32.const 4728
    i32.const 61455
    call $RuntimeProgram::push_uint16_t_unsigned_short_
    drop
    i32.const 4728
    i32.const 97
    call $RuntimeProgram::push_unsigned_char_
    drop
    i32.const 4728
    i32.const 4228
    call $void_UnitTest::review<unsigned_short>_TestCase<unsigned_short>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 128
    call $RuntimeProgram::push_unsigned_char_
    drop
    i32.const 4728
    i32.const 4240
    call $void_UnitTest::review<bool>_TestCase<bool>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 0
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 129
    call $RuntimeProgram::push_unsigned_char_
    drop
    i32.const 4728
    i32.const 4252
    call $void_UnitTest::review<bool>_TestCase<bool>_const&_
    i32.const 0
    i32.const 12
    i32.store offset=4740
    i32.const 0
    i32.const 0
    i32.store offset=4734 align=2
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 1
    call $RuntimeProgram::push_uint8_t_unsigned_char_
    drop
    i32.const 4728
    i32.const 160
    i32.const 1
    call $RuntimeProgram::push_unsigned_char__unsigned_char_
    drop
    i32.const 4728
    i32.const 4264
    call $void_UnitTest::review<bool>_TestCase<bool>_const&_
    call $case_jump_build__
    i32.const 4728
    i32.const 4276
    call $void_UnitTest::review<unsigned_char>_TestCase<unsigned_char>_const&_
    call $case_jump_if_build__
    i32.const 4728
    i32.const 4288
    call $void_UnitTest::review<unsigned_char>_TestCase<unsigned_char>_const&_
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 0
      i32.const 255
      i32.and
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.set 0
      local.get 1
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 82
    local.set 1
    i32.const 1
    local.set 0
    loop  ;; label = @1
      local.get 1
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 0
      i32.const 2865
      i32.add
      i32.load8_u
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.tee 2
      local.set 0
      local.get 2
      i32.const 37
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 0
    local.set 0
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 0
      i32.const 255
      i32.and
      local.set 1
      local.get 0
      i32.const 1
      i32.add
      local.set 0
      local.get 1
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print)
  (func $void_UnitTest::run<unsigned_char>_TestCase<unsigned_char>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 10
    call $__print
    i32.const 0
    local.set 3
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 3
      i32.const 255
      i32.and
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      local.get 4
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 82
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3590
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 15
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 1
      i32.load
      local.tee 4
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 4
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 3
    local.get 0
    call $UnitTest::fullProgramDebug_VovkPLCRuntime&__RuntimeProgram&_
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 3
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 3
        i32.load offset=8
        local.tee 5
        i32.const 65535
        i32.and
        br_if 0 (;@2;)
        i32.const 0
        local.set 0
        br 1 (;@1;)
      end
      local.get 3
      local.get 5
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 3
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      local.set 0
    end
    local.get 4
    local.get 1
    i32.load offset=4
    i32.eq
    local.get 1
    i32.load8_u offset=8
    local.get 0
    i32.const 255
    i32.and
    i32.eq
    i32.and
    local.set 6
    i32.const 80
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3605
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 17
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    local.get 0
    i32.const 255
    i32.and
    i32.store offset=16
    i32.const 4400
    i32.const 1466
    local.get 2
    i32.const 16
    i32.add
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 1459
    i32.const 3004
    local.get 6
    select
    local.set 0
    i32.const 10
    call $__print
    i32.const 69
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3622
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 18
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    local.get 1
    i32.load8_u offset=8
    i32.store
    i32.const 4400
    i32.const 1466
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 84
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3723
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 14
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 0
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 10
    call $__print
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::run<unsigned_short>_TestCase<unsigned_short>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 10
    call $__print
    i32.const 0
    local.set 3
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 3
      i32.const 255
      i32.and
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      local.get 4
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 82
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3590
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 15
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 1
      i32.load
      local.tee 4
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 4
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 3
    local.get 0
    call $UnitTest::fullProgramDebug_VovkPLCRuntime&__RuntimeProgram&_
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 3
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 5
        i32.load16_u offset=8
        i32.const 65534
        i32.and
        br_if 0 (;@2;)
        i32.const 0
        local.set 0
        br 1 (;@1;)
      end
      i32.const 0
      local.set 3
      loop  ;; label = @2
        i32.const 0
        local.set 4
        block  ;; label = @3
          local.get 5
          i32.load offset=8
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          local.get 5
          local.get 0
          i32.const -1
          i32.add
          local.tee 4
          i32.store offset=8
          local.get 5
          i32.load
          local.get 4
          i32.add
          i32.load8_u
          local.set 4
        end
        local.get 2
        i32.const 30
        i32.add
        local.get 3
        i32.add
        local.get 4
        i32.store8
        local.get 3
        i32.const 1
        i32.add
        local.tee 3
        i32.const 2
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      i32.load16_u offset=30
      local.set 0
    end
    local.get 6
    local.get 1
    i32.load offset=4
    i32.eq
    local.get 1
    i32.load16_u offset=8
    local.get 0
    i32.const 65535
    i32.and
    i32.eq
    i32.and
    local.set 6
    i32.const 80
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3605
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 17
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    local.get 0
    i32.const 65535
    i32.and
    i32.store offset=16
    i32.const 4400
    i32.const 1466
    local.get 2
    i32.const 16
    i32.add
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 1459
    i32.const 3004
    local.get 6
    select
    local.set 0
    i32.const 10
    call $__print
    i32.const 69
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3622
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 18
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    local.get 1
    i32.load16_u offset=8
    i32.store
    i32.const 4400
    i32.const 1466
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 84
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3723
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 14
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 0
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 10
    call $__print
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::run<unsigned_int>_TestCase<unsigned_int>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 10
    call $__print
    i32.const 0
    local.set 3
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 3
      i32.const 255
      i32.and
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      local.get 4
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 82
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3590
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 15
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 1
      i32.load
      local.tee 4
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 4
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 3
    local.get 0
    call $UnitTest::fullProgramDebug_VovkPLCRuntime&__RuntimeProgram&_
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 3
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 5
        i32.load16_u offset=8
        i32.const 65532
        i32.and
        br_if 0 (;@2;)
        i32.const 0
        local.set 0
        br 1 (;@1;)
      end
      i32.const 0
      local.set 3
      loop  ;; label = @2
        i32.const 0
        local.set 4
        block  ;; label = @3
          local.get 5
          i32.load offset=8
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          local.get 5
          local.get 0
          i32.const -1
          i32.add
          local.tee 4
          i32.store offset=8
          local.get 5
          i32.load
          local.get 4
          i32.add
          i32.load8_u
          local.set 4
        end
        local.get 2
        i32.const 28
        i32.add
        local.get 3
        i32.add
        local.get 4
        i32.store8
        local.get 3
        i32.const 1
        i32.add
        local.tee 3
        i32.const 4
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      i32.load offset=28
      local.set 0
    end
    local.get 6
    local.get 1
    i32.load offset=4
    i32.eq
    local.get 1
    i32.load offset=8
    local.get 0
    i32.eq
    i32.and
    local.set 6
    i32.const 80
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3605
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 17
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    local.get 0
    i32.store offset=16
    i32.const 4400
    i32.const 1109
    local.get 2
    i32.const 16
    i32.add
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 1459
    i32.const 3004
    local.get 6
    select
    local.set 0
    i32.const 10
    call $__print
    i32.const 69
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3622
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 18
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    local.get 1
    i32.load offset=8
    i32.store
    i32.const 4400
    i32.const 1109
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 84
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3723
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 14
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 0
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 10
    call $__print
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::run<unsigned_long_long>_TestCase<unsigned_long_long>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32 i64)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 10
    call $__print
    i32.const 0
    local.set 3
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 3
      i32.const 255
      i32.and
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      local.get 4
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 82
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3590
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 15
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 1
      i32.load
      local.tee 4
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 4
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 3
    local.get 0
    call $UnitTest::fullProgramDebug_VovkPLCRuntime&__RuntimeProgram&_
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 3
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 5
        i32.load16_u offset=8
        i32.const 65528
        i32.and
        br_if 0 (;@2;)
        i64.const 0
        local.set 7
        br 1 (;@1;)
      end
      i32.const 0
      local.set 3
      loop  ;; label = @2
        i32.const 0
        local.set 4
        block  ;; label = @3
          local.get 5
          i32.load offset=8
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          local.get 5
          local.get 0
          i32.const -1
          i32.add
          local.tee 4
          i32.store offset=8
          local.get 5
          i32.load
          local.get 4
          i32.add
          i32.load8_u
          local.set 4
        end
        local.get 2
        i32.const 8
        i32.add
        local.get 3
        i32.add
        local.get 4
        i32.store8
        local.get 3
        i32.const 1
        i32.add
        local.tee 3
        i32.const 8
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      i64.load offset=8
      local.set 7
    end
    local.get 6
    local.get 1
    i32.load offset=4
    i32.eq
    local.get 1
    i64.load offset=8
    local.get 7
    i64.eq
    i32.and
    local.set 0
    i32.const 80
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3605
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 17
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 7
    call $print__uint64_t_unsigned_long_long_
    i32.const 10
    call $__print
    i32.const 69
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3622
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 18
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 1
    i64.load offset=8
    call $print__uint64_t_unsigned_long_long_
    i32.const 10
    call $__print
    i32.const 84
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3723
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 14
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      i32.const 1459
      i32.const 3004
      local.get 0
      select
      local.tee 4
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 4
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 10
    call $__print
    local.get 2
    i32.const 16
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::run<signed_char>_TestCase<signed_char>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 10
    call $__print
    i32.const 0
    local.set 3
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 3
      i32.const 255
      i32.and
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      local.get 4
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 82
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3590
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 15
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 1
      i32.load
      local.tee 4
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 4
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 3
    local.get 0
    call $UnitTest::fullProgramDebug_VovkPLCRuntime&__RuntimeProgram&_
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 3
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 3
        i32.load offset=8
        local.tee 5
        i32.const 65535
        i32.and
        br_if 0 (;@2;)
        i32.const 0
        local.set 0
        br 1 (;@1;)
      end
      local.get 3
      local.get 5
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 3
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      local.set 0
    end
    local.get 4
    local.get 1
    i32.load offset=4
    i32.eq
    local.get 1
    i32.load8_u offset=8
    local.get 0
    i32.const 255
    i32.and
    i32.eq
    i32.and
    local.set 6
    i32.const 80
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3605
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 17
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    local.get 0
    i32.const 24
    i32.shl
    i32.const 24
    i32.shr_s
    i32.store offset=16
    i32.const 4400
    i32.const 1466
    local.get 2
    i32.const 16
    i32.add
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 1459
    i32.const 3004
    local.get 6
    select
    local.set 0
    i32.const 10
    call $__print
    i32.const 69
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3622
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 18
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    local.get 1
    i32.load8_s offset=8
    i32.store
    i32.const 4400
    i32.const 1466
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 84
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3723
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 14
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 0
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 10
    call $__print
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::run<float>_TestCase<float>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32 f32)
    global.get $__stack_pointer
    i32.const 48
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 10
    call $__print
    i32.const 0
    local.set 3
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 3
      i32.const 255
      i32.and
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      local.get 4
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 82
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3590
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 15
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 1
      i32.load
      local.tee 4
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 4
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 3
    local.get 0
    call $UnitTest::fullProgramDebug_VovkPLCRuntime&__RuntimeProgram&_
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 3
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 5
        i32.load16_u offset=8
        i32.const 65532
        i32.and
        br_if 0 (;@2;)
        f32.const 0x0p+0 (;=0;)
        local.set 7
        br 1 (;@1;)
      end
      i32.const 0
      local.set 3
      loop  ;; label = @2
        i32.const 0
        local.set 4
        block  ;; label = @3
          local.get 5
          i32.load offset=8
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          local.get 5
          local.get 0
          i32.const -1
          i32.add
          local.tee 4
          i32.store offset=8
          local.get 5
          i32.load
          local.get 4
          i32.add
          i32.load8_u
          local.set 4
        end
        local.get 2
        i32.const 44
        i32.add
        local.get 3
        i32.add
        local.get 4
        i32.store8
        local.get 3
        i32.const 1
        i32.add
        local.tee 3
        i32.const 4
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      f32.load offset=44
      local.set 7
    end
    local.get 6
    local.get 1
    i32.load offset=4
    i32.eq
    local.get 1
    f32.load offset=8
    local.get 7
    f32.eq
    i32.and
    local.set 0
    i32.const 80
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3605
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 17
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    i32.const 10
    i32.store offset=16
    local.get 2
    local.get 7
    f64.promote_f32
    f64.store offset=24
    i32.const 4400
    i32.const 1348
    local.get 2
    i32.const 16
    i32.add
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 1459
    i32.const 3004
    local.get 0
    select
    local.set 0
    i32.const 10
    call $__print
    i32.const 69
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3622
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 18
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 1
    f32.load offset=8
    local.set 7
    local.get 2
    i32.const 10
    i32.store
    local.get 2
    local.get 7
    f64.promote_f32
    f64.store offset=8
    i32.const 4400
    i32.const 1348
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 84
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3723
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 14
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 0
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 10
    call $__print
    local.get 2
    i32.const 48
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::run<double>_TestCase<double>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32 f64)
    global.get $__stack_pointer
    i32.const 48
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 10
    call $__print
    i32.const 0
    local.set 3
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 3
      i32.const 255
      i32.and
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      local.get 4
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 82
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3590
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 15
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 1
      i32.load
      local.tee 4
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 4
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 3
    local.get 0
    call $UnitTest::fullProgramDebug_VovkPLCRuntime&__RuntimeProgram&_
    local.set 6
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 3
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 5
        i32.load16_u offset=8
        i32.const 65528
        i32.and
        br_if 0 (;@2;)
        f64.const 0x0p+0 (;=0;)
        local.set 7
        br 1 (;@1;)
      end
      i32.const 0
      local.set 3
      loop  ;; label = @2
        i32.const 0
        local.set 4
        block  ;; label = @3
          local.get 5
          i32.load offset=8
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          local.get 5
          local.get 0
          i32.const -1
          i32.add
          local.tee 4
          i32.store offset=8
          local.get 5
          i32.load
          local.get 4
          i32.add
          i32.load8_u
          local.set 4
        end
        local.get 2
        i32.const 40
        i32.add
        local.get 3
        i32.add
        local.get 4
        i32.store8
        local.get 3
        i32.const 1
        i32.add
        local.tee 3
        i32.const 8
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      f64.load offset=40
      local.set 7
    end
    local.get 6
    local.get 1
    i32.load offset=4
    i32.eq
    local.get 1
    f64.load offset=8
    local.get 7
    f64.eq
    i32.and
    local.set 0
    i32.const 80
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3605
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 17
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    local.get 7
    f64.store offset=24
    local.get 2
    i32.const 10
    i32.store offset=16
    i32.const 4400
    i32.const 1348
    local.get 2
    i32.const 16
    i32.add
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 1459
    i32.const 3004
    local.get 0
    select
    local.set 0
    i32.const 10
    call $__print
    i32.const 69
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3622
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 18
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    local.get 1
    f64.load offset=8
    f64.store offset=8
    local.get 2
    i32.const 10
    i32.store
    i32.const 4400
    i32.const 1348
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 84
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3723
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 14
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 0
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 10
    call $__print
    local.get 2
    i32.const 48
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::run<bool>_TestCase<bool>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 10
    call $__print
    i32.const 0
    local.set 3
    loop  ;; label = @1
      i32.const 35
      call $__print
      local.get 3
      i32.const 255
      i32.and
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      local.get 4
      i32.const 69
      i32.lt_u
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 82
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3590
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 15
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 1
      i32.load
      local.tee 4
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 4
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 3
    local.get 0
    call $UnitTest::fullProgramDebug_VovkPLCRuntime&__RuntimeProgram&_
    local.set 4
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 3
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 3
        i32.load offset=8
        local.tee 5
        i32.const 65535
        i32.and
        br_if 0 (;@2;)
        i32.const 0
        local.set 0
        br 1 (;@1;)
      end
      local.get 3
      local.get 5
      i32.const -1
      i32.add
      local.tee 5
      i32.store offset=8
      local.get 3
      i32.load
      local.get 5
      i32.add
      i32.load8_u
      i32.const 0
      i32.ne
      local.set 0
    end
    local.get 4
    local.get 1
    i32.load offset=4
    i32.eq
    local.get 1
    i32.load8_u offset=8
    local.get 0
    i32.eq
    i32.and
    local.set 6
    i32.const 80
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3605
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 17
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    local.get 0
    i32.store offset=16
    i32.const 4400
    i32.const 1466
    local.get 2
    i32.const 16
    i32.add
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 1459
    i32.const 3004
    local.get 6
    select
    local.set 0
    i32.const 10
    call $__print
    i32.const 69
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3622
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 18
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    local.get 1
    i32.load8_u offset=8
    i32.store
    i32.const 4400
    i32.const 1466
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 84
    local.set 4
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 4
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 3723
      i32.add
      i32.load8_u
      local.set 4
      local.get 3
      i32.const 1
      i32.add
      local.tee 5
      local.set 3
      local.get 5
      i32.const 14
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      local.get 0
      i32.load8_u
      local.tee 3
      i32.eqz
      br_if 0 (;@1;)
      local.get 0
      i32.const 1
      i32.add
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 4
        i32.load8_u
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.set 4
        local.get 3
        br_if 0 (;@2;)
      end
    end
    i32.const 10
    call $__print
    i32.const 10
    call $__print
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::review<unsigned_char>_TestCase<unsigned_char>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 84
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3249
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 7
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 1
        i32.load
        local.tee 6
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
        i32.const 6
        local.set 5
        br 1 (;@1;)
      end
      i32.const 6
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 6
        local.get 4
        i32.add
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.tee 5
        local.set 4
        local.get 3
        i32.const -5
        i32.add
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
      end
    end
    i32.const 34
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 4
    call $VovkPLCRuntime::cleanRun__
    drop
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 4
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 4
        i32.load offset=8
        local.tee 3
        i32.const 65535
        i32.and
        br_if 0 (;@2;)
        i32.const 0
        local.set 3
        br 1 (;@1;)
      end
      local.get 4
      local.get 3
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 4
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    local.get 1
    i32.load8_u offset=8
    local.set 6
    block  ;; label = @1
      local.get 5
      i32.const 38
      i32.gt_s
      br_if 0 (;@1;)
      local.get 5
      i32.const -39
      i32.add
      local.set 4
      loop  ;; label = @2
        i32.const 32
        call $__print
        local.get 4
        i32.const 1
        i32.add
        local.tee 4
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      i32.const 1459
      i32.const 3299
      local.get 6
      i32.const 255
      i32.and
      local.get 3
      i32.const 255
      i32.and
      i32.eq
      select
      local.tee 3
      i32.load8_u
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3935
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    i64.const 0
    i64.store offset=8
    local.get 2
    i32.const 3
    i32.store
    i32.const 4400
    i32.const 1348
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 1299
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    local.get 2
    i32.const 16
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::review<unsigned_short>_TestCase<unsigned_short>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 84
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3249
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 7
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 1
        i32.load
        local.tee 5
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
        i32.const 6
        local.set 6
        br 1 (;@1;)
      end
      i32.const 6
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 5
        local.get 4
        i32.add
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.tee 6
        local.set 4
        local.get 3
        i32.const -5
        i32.add
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
      end
    end
    i32.const 34
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 4
    call $VovkPLCRuntime::cleanRun__
    drop
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 4
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 5
        i32.load16_u offset=8
        i32.const 65534
        i32.and
        br_if 0 (;@2;)
        i32.const 0
        local.set 3
        br 1 (;@1;)
      end
      i32.const 0
      local.set 4
      loop  ;; label = @2
        i32.const 0
        local.set 3
        block  ;; label = @3
          local.get 5
          i32.load offset=8
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          local.get 5
          local.get 0
          i32.const -1
          i32.add
          local.tee 3
          i32.store offset=8
          local.get 5
          i32.load
          local.get 3
          i32.add
          i32.load8_u
          local.set 3
        end
        local.get 2
        i32.const 30
        i32.add
        local.get 4
        i32.add
        local.get 3
        i32.store8
        local.get 4
        i32.const 1
        i32.add
        local.tee 4
        i32.const 2
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      i32.load16_u offset=30
      local.set 3
    end
    local.get 1
    i32.load16_u offset=8
    local.set 5
    block  ;; label = @1
      local.get 6
      i32.const 38
      i32.gt_s
      br_if 0 (;@1;)
      local.get 6
      i32.const -39
      i32.add
      local.set 4
      loop  ;; label = @2
        i32.const 32
        call $__print
        local.get 4
        i32.const 1
        i32.add
        local.tee 4
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      i32.const 1459
      i32.const 3299
      local.get 5
      i32.const 65535
      i32.and
      local.get 3
      i32.const 65535
      i32.and
      i32.eq
      select
      local.tee 3
      i32.load8_u
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3935
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    i64.const 0
    i64.store offset=8
    local.get 2
    i32.const 3
    i32.store
    i32.const 4400
    i32.const 1348
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 1299
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::review<unsigned_int>_TestCase<unsigned_int>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 84
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3249
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 7
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 1
        i32.load
        local.tee 5
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
        i32.const 6
        local.set 6
        br 1 (;@1;)
      end
      i32.const 6
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 5
        local.get 4
        i32.add
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.tee 6
        local.set 4
        local.get 3
        i32.const -5
        i32.add
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
      end
    end
    i32.const 34
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 4
    call $VovkPLCRuntime::cleanRun__
    drop
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 4
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 5
        i32.load16_u offset=8
        i32.const 65532
        i32.and
        br_if 0 (;@2;)
        i32.const 0
        local.set 3
        br 1 (;@1;)
      end
      i32.const 0
      local.set 4
      loop  ;; label = @2
        i32.const 0
        local.set 3
        block  ;; label = @3
          local.get 5
          i32.load offset=8
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          local.get 5
          local.get 0
          i32.const -1
          i32.add
          local.tee 3
          i32.store offset=8
          local.get 5
          i32.load
          local.get 3
          i32.add
          i32.load8_u
          local.set 3
        end
        local.get 2
        i32.const 28
        i32.add
        local.get 4
        i32.add
        local.get 3
        i32.store8
        local.get 4
        i32.const 1
        i32.add
        local.tee 4
        i32.const 4
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      i32.load offset=28
      local.set 3
    end
    local.get 1
    i32.load offset=8
    local.set 5
    block  ;; label = @1
      local.get 6
      i32.const 38
      i32.gt_s
      br_if 0 (;@1;)
      local.get 6
      i32.const -39
      i32.add
      local.set 4
      loop  ;; label = @2
        i32.const 32
        call $__print
        local.get 4
        i32.const 1
        i32.add
        local.tee 4
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      i32.const 1459
      i32.const 3299
      local.get 5
      local.get 3
      i32.eq
      select
      local.tee 3
      i32.load8_u
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3935
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    i64.const 0
    i64.store offset=8
    local.get 2
    i32.const 3
    i32.store
    i32.const 4400
    i32.const 1348
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 1299
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::review<unsigned_long_long>_TestCase<unsigned_long_long>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32 i64 i64)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 84
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3249
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 7
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 1
        i32.load
        local.tee 5
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
        i32.const 6
        local.set 6
        br 1 (;@1;)
      end
      i32.const 6
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 5
        local.get 4
        i32.add
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.tee 6
        local.set 4
        local.get 3
        i32.const -5
        i32.add
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
      end
    end
    i32.const 34
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 4
    call $VovkPLCRuntime::cleanRun__
    drop
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 4
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 5
        i32.load16_u offset=8
        i32.const 65528
        i32.and
        br_if 0 (;@2;)
        i64.const 0
        local.set 7
        br 1 (;@1;)
      end
      i32.const 0
      local.set 4
      loop  ;; label = @2
        i32.const 0
        local.set 3
        block  ;; label = @3
          local.get 5
          i32.load offset=8
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          local.get 5
          local.get 0
          i32.const -1
          i32.add
          local.tee 3
          i32.store offset=8
          local.get 5
          i32.load
          local.get 3
          i32.add
          i32.load8_u
          local.set 3
        end
        local.get 2
        i32.const 24
        i32.add
        local.get 4
        i32.add
        local.get 3
        i32.store8
        local.get 4
        i32.const 1
        i32.add
        local.tee 4
        i32.const 8
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      i64.load offset=24
      local.set 7
    end
    local.get 1
    i64.load offset=8
    local.set 8
    block  ;; label = @1
      local.get 6
      i32.const 38
      i32.gt_s
      br_if 0 (;@1;)
      local.get 6
      i32.const -39
      i32.add
      local.set 4
      loop  ;; label = @2
        i32.const 32
        call $__print
        local.get 4
        i32.const 1
        i32.add
        local.tee 4
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      i32.const 1459
      i32.const 3299
      local.get 8
      local.get 7
      i64.eq
      select
      local.tee 3
      i32.load8_u
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3935
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    i64.const 0
    i64.store offset=8
    local.get 2
    i32.const 3
    i32.store
    i32.const 4400
    i32.const 1348
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 1299
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::review<signed_char>_TestCase<signed_char>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 84
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3249
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 7
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 1
        i32.load
        local.tee 6
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
        i32.const 6
        local.set 5
        br 1 (;@1;)
      end
      i32.const 6
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 6
        local.get 4
        i32.add
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.tee 5
        local.set 4
        local.get 3
        i32.const -5
        i32.add
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
      end
    end
    i32.const 34
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 4
    call $VovkPLCRuntime::cleanRun__
    drop
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 4
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 4
        i32.load offset=8
        local.tee 3
        i32.const 65535
        i32.and
        br_if 0 (;@2;)
        i32.const 0
        local.set 3
        br 1 (;@1;)
      end
      local.get 4
      local.get 3
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 4
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      local.set 3
    end
    local.get 1
    i32.load8_u offset=8
    local.set 6
    block  ;; label = @1
      local.get 5
      i32.const 38
      i32.gt_s
      br_if 0 (;@1;)
      local.get 5
      i32.const -39
      i32.add
      local.set 4
      loop  ;; label = @2
        i32.const 32
        call $__print
        local.get 4
        i32.const 1
        i32.add
        local.tee 4
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      i32.const 1459
      i32.const 3299
      local.get 6
      i32.const 255
      i32.and
      local.get 3
      i32.const 255
      i32.and
      i32.eq
      select
      local.tee 3
      i32.load8_u
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3935
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    i64.const 0
    i64.store offset=8
    local.get 2
    i32.const 3
    i32.store
    i32.const 4400
    i32.const 1348
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 1299
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    local.get 2
    i32.const 16
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::review<float>_TestCase<float>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32 f32 f32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 84
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3249
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 7
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 1
        i32.load
        local.tee 5
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
        i32.const 6
        local.set 6
        br 1 (;@1;)
      end
      i32.const 6
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 5
        local.get 4
        i32.add
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.tee 6
        local.set 4
        local.get 3
        i32.const -5
        i32.add
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
      end
    end
    i32.const 34
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 4
    call $VovkPLCRuntime::cleanRun__
    drop
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 4
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 5
        i32.load16_u offset=8
        i32.const 65532
        i32.and
        br_if 0 (;@2;)
        f32.const 0x0p+0 (;=0;)
        local.set 7
        br 1 (;@1;)
      end
      i32.const 0
      local.set 4
      loop  ;; label = @2
        i32.const 0
        local.set 3
        block  ;; label = @3
          local.get 5
          i32.load offset=8
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          local.get 5
          local.get 0
          i32.const -1
          i32.add
          local.tee 3
          i32.store offset=8
          local.get 5
          i32.load
          local.get 3
          i32.add
          i32.load8_u
          local.set 3
        end
        local.get 2
        i32.const 28
        i32.add
        local.get 4
        i32.add
        local.get 3
        i32.store8
        local.get 4
        i32.const 1
        i32.add
        local.tee 4
        i32.const 4
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      f32.load offset=28
      local.set 7
    end
    local.get 1
    f32.load offset=8
    local.set 8
    block  ;; label = @1
      local.get 6
      i32.const 38
      i32.gt_s
      br_if 0 (;@1;)
      local.get 6
      i32.const -39
      i32.add
      local.set 4
      loop  ;; label = @2
        i32.const 32
        call $__print
        local.get 4
        i32.const 1
        i32.add
        local.tee 4
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      i32.const 1459
      i32.const 3299
      local.get 8
      local.get 7
      f32.eq
      select
      local.tee 3
      i32.load8_u
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3935
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    i64.const 0
    i64.store offset=8
    local.get 2
    i32.const 3
    i32.store
    i32.const 4400
    i32.const 1348
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 1299
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::review<double>_TestCase<double>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32 f64 f64)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 84
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3249
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 7
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 1
        i32.load
        local.tee 5
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
        i32.const 6
        local.set 6
        br 1 (;@1;)
      end
      i32.const 6
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 5
        local.get 4
        i32.add
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.tee 6
        local.set 4
        local.get 3
        i32.const -5
        i32.add
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
      end
    end
    i32.const 34
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 4
    call $VovkPLCRuntime::cleanRun__
    drop
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 4
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 5
        i32.load16_u offset=8
        i32.const 65528
        i32.and
        br_if 0 (;@2;)
        f64.const 0x0p+0 (;=0;)
        local.set 7
        br 1 (;@1;)
      end
      i32.const 0
      local.set 4
      loop  ;; label = @2
        i32.const 0
        local.set 3
        block  ;; label = @3
          local.get 5
          i32.load offset=8
          local.tee 0
          i32.eqz
          br_if 0 (;@3;)
          local.get 5
          local.get 0
          i32.const -1
          i32.add
          local.tee 3
          i32.store offset=8
          local.get 5
          i32.load
          local.get 3
          i32.add
          i32.load8_u
          local.set 3
        end
        local.get 2
        i32.const 24
        i32.add
        local.get 4
        i32.add
        local.get 3
        i32.store8
        local.get 4
        i32.const 1
        i32.add
        local.tee 4
        i32.const 8
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      f64.load offset=24
      local.set 7
    end
    local.get 1
    f64.load offset=8
    local.set 8
    block  ;; label = @1
      local.get 6
      i32.const 38
      i32.gt_s
      br_if 0 (;@1;)
      local.get 6
      i32.const -39
      i32.add
      local.set 4
      loop  ;; label = @2
        i32.const 32
        call $__print
        local.get 4
        i32.const 1
        i32.add
        local.tee 4
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      i32.const 1459
      i32.const 3299
      local.get 8
      local.get 7
      f64.eq
      select
      local.tee 3
      i32.load8_u
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3935
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    i64.const 0
    i64.store offset=8
    local.get 2
    i32.const 3
    i32.store
    i32.const 4400
    i32.const 1348
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 1299
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    local.get 2
    i32.const 32
    i32.add
    global.set $__stack_pointer)
  (func $void_UnitTest::review<bool>_TestCase<bool>_const&_ (type 8) (param i32 i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 84
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3249
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 7
      i32.ne
      br_if 0 (;@1;)
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 1
        i32.load
        local.tee 6
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
        i32.const 6
        local.set 5
        br 1 (;@1;)
      end
      i32.const 6
      local.set 4
      loop  ;; label = @2
        local.get 3
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 6
        local.get 4
        i32.add
        local.set 3
        local.get 4
        i32.const 1
        i32.add
        local.tee 5
        local.set 4
        local.get 3
        i32.const -5
        i32.add
        i32.load8_u
        local.tee 3
        br_if 0 (;@2;)
      end
    end
    i32.const 34
    call $__print
    local.get 0
    i32.const 16
    i32.add
    local.tee 4
    call $VovkPLCRuntime::cleanRun__
    drop
    block  ;; label = @1
      local.get 0
      i32.load8_u offset=16
      br_if 0 (;@1;)
      local.get 4
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.const 20
        i32.add
        i32.load
        local.tee 4
        i32.load offset=8
        local.tee 3
        i32.const 65535
        i32.and
        br_if 0 (;@2;)
        i32.const 0
        local.set 3
        br 1 (;@1;)
      end
      local.get 4
      local.get 3
      i32.const -1
      i32.add
      local.tee 3
      i32.store offset=8
      local.get 4
      i32.load
      local.get 3
      i32.add
      i32.load8_u
      i32.const 0
      i32.ne
      local.set 3
    end
    local.get 1
    i32.load8_u offset=8
    local.set 6
    block  ;; label = @1
      local.get 5
      i32.const 38
      i32.gt_s
      br_if 0 (;@1;)
      local.get 5
      i32.const -39
      i32.add
      local.set 4
      loop  ;; label = @2
        i32.const 32
        call $__print
        local.get 4
        i32.const 1
        i32.add
        local.tee 4
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      i32.const 1459
      i32.const 3299
      local.get 6
      i32.const 255
      i32.and
      local.get 3
      i32.eq
      select
      local.tee 3
      i32.load8_u
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      local.get 3
      i32.const 1
      i32.add
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 3935
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 2
    i64.const 0
    i64.store offset=8
    local.get 2
    i32.const 3
    i32.store
    i32.const 4400
    i32.const 1348
    local.get 2
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 4
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 3
      loop  ;; label = @2
        local.get 4
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.load8_u
        local.set 4
        local.get 3
        i32.const 1
        i32.add
        local.set 3
        local.get 4
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 3
    i32.const 1
    local.set 4
    loop  ;; label = @1
      local.get 3
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 4
      i32.const 1299
      i32.add
      i32.load8_u
      local.set 3
      local.get 4
      i32.const 1
      i32.add
      local.tee 5
      local.set 4
      local.get 5
      i32.const 4
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    local.get 2
    i32.const 16
    i32.add
    global.set $__stack_pointer)
  (func $UnitTest::fullProgramDebug_VovkPLCRuntime&__RuntimeProgram&_ (type 13) (param i32 i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 96
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    local.get 1
    i32.const 0
    i32.store16 offset=8
    block  ;; label = @1
      local.get 0
      i32.load offset=4
      local.tee 3
      i32.load offset=8
      i32.eqz
      br_if 0 (;@1;)
      local.get 3
      i32.const 0
      i32.store offset=8
    end
    block  ;; label = @1
      local.get 3
      i32.const 20
      i32.add
      i32.load
      i32.eqz
      br_if 0 (;@1;)
      local.get 3
      i32.const 0
      i32.store offset=20
    end
    local.get 1
    i32.const 8
    i32.add
    local.set 4
    local.get 1
    call $RuntimeProgram::print__
    drop
    i32.const 10
    call $__print
    local.get 1
    call $RuntimeProgram::explain__
    i32.const 83
    local.set 5
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 5
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 2964
      i32.add
      i32.load8_u
      local.set 5
      local.get 3
      i32.const 1
      i32.add
      local.tee 6
      local.set 3
      local.get 6
      i32.const 35
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    i32.const 0
    local.set 7
    block  ;; label = @1
      loop  ;; label = @2
        local.get 1
        i32.load16_u offset=8
        local.set 8
        local.get 0
        local.get 1
        i32.load
        local.get 1
        i32.load16_u offset=6
        local.get 4
        call $VovkPLCRuntime::step_unsigned_char*__unsigned_short__unsigned_short&_
        local.tee 9
        i32.const -17
        i32.and
        local.set 10
        i32.const 1
        local.set 3
        i32.const 32
        local.set 5
        loop  ;; label = @3
          local.get 5
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3504
          i32.add
          i32.load8_u
          local.set 5
          local.get 3
          i32.const 1
          i32.add
          local.tee 6
          local.set 3
          local.get 6
          i32.const 10
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 8
        i32.eqz
        local.set 5
        block  ;; label = @3
          local.get 8
          i32.eqz
          br_if 0 (;@3;)
          local.get 8
          local.set 3
          loop  ;; label = @4
            local.get 5
            i32.const 1
            i32.add
            local.set 5
            local.get 3
            i32.const 9
            i32.gt_s
            local.set 6
            local.get 3
            i32.const 10
            i32.div_s
            local.set 3
            local.get 6
            br_if 0 (;@4;)
          end
        end
        block  ;; label = @3
          local.get 5
          i32.const 3
          i32.gt_s
          br_if 0 (;@3;)
          local.get 5
          i32.const -4
          i32.add
          local.set 3
          loop  ;; label = @4
            i32.const 32
            call $__print
            local.get 3
            i32.const 1
            i32.add
            local.tee 3
            br_if 0 (;@4;)
          end
        end
        local.get 9
        i32.const 16
        i32.eq
        local.set 6
        local.get 2
        local.get 8
        i32.store offset=80
        i32.const 4400
        i32.const 1466
        local.get 2
        i32.const 80
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 5
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 5
            i32.load8_u
            local.set 3
            local.get 5
            i32.const 1
            i32.add
            local.set 5
            local.get 3
            br_if 0 (;@4;)
          end
        end
        local.get 6
        local.get 7
        i32.or
        local.set 7
        i32.const 32
        local.set 5
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 5
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 1510
          i32.add
          i32.load8_u
          local.set 5
          local.get 3
          i32.const 1
          i32.add
          local.tee 6
          local.set 3
          local.get 6
          i32.const 3
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 8
        i32.const 8
        i32.shr_u
        local.set 11
        block  ;; label = @3
          local.get 8
          i32.const 256
          i32.lt_u
          local.tee 5
          br_if 0 (;@3;)
          local.get 11
          local.set 3
          loop  ;; label = @4
            local.get 5
            i32.const 1
            i32.add
            local.set 5
            local.get 3
            i32.const 15
            i32.gt_s
            local.set 6
            local.get 3
            i32.const 16
            i32.div_s
            local.set 3
            local.get 6
            br_if 0 (;@4;)
          end
        end
        block  ;; label = @3
          local.get 5
          i32.const 1
          i32.gt_s
          br_if 0 (;@3;)
          local.get 5
          i32.const -2
          i32.add
          local.set 3
          loop  ;; label = @4
            i32.const 48
            call $__print
            local.get 3
            i32.const 1
            i32.add
            local.tee 3
            br_if 0 (;@4;)
          end
        end
        local.get 2
        local.get 11
        i32.store offset=64
        i32.const 4400
        i32.const 1106
        local.get 2
        i32.const 64
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 5
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 5
            i32.load8_u
            local.set 3
            local.get 5
            i32.const 1
            i32.add
            local.set 5
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 32
        call $__print
        local.get 8
        i32.const 255
        i32.and
        local.tee 11
        i32.eqz
        local.set 5
        block  ;; label = @3
          local.get 11
          i32.eqz
          br_if 0 (;@3;)
          local.get 11
          local.set 3
          loop  ;; label = @4
            local.get 5
            i32.const 1
            i32.add
            local.set 5
            local.get 3
            i32.const 15
            i32.gt_s
            local.set 6
            local.get 3
            i32.const 16
            i32.div_s
            local.set 3
            local.get 6
            br_if 0 (;@4;)
          end
        end
        block  ;; label = @3
          local.get 5
          i32.const 1
          i32.gt_s
          br_if 0 (;@3;)
          local.get 5
          i32.const -2
          i32.add
          local.set 3
          loop  ;; label = @4
            i32.const 48
            call $__print
            local.get 3
            i32.const 1
            i32.add
            local.tee 3
            br_if 0 (;@4;)
          end
        end
        local.get 2
        local.get 11
        i32.store offset=48
        i32.const 4400
        i32.const 1106
        local.get 2
        i32.const 48
        i32.add
        call $sprintf_
        drop
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 3
          i32.eqz
          br_if 0 (;@3;)
          i32.const 4401
          local.set 5
          loop  ;; label = @4
            local.get 3
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 5
            i32.load8_u
            local.set 3
            local.get 5
            i32.const 1
            i32.add
            local.set 5
            local.get 3
            br_if 0 (;@4;)
          end
        end
        i32.const 93
        local.set 5
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 5
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 3914
          i32.add
          i32.load8_u
          local.set 5
          local.get 3
          i32.const 1
          i32.add
          local.tee 6
          local.set 3
          local.get 6
          i32.const 4
          i32.ne
          br_if 0 (;@3;)
        end
        block  ;; label = @3
          block  ;; label = @4
            local.get 10
            br_if 0 (;@4;)
            i32.const 69
            local.set 5
            i32.const 1
            local.set 3
            loop  ;; label = @5
              local.get 5
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 3
              i32.const 1491
              i32.add
              i32.load8_u
              local.set 5
              local.get 3
              i32.const 1
              i32.add
              local.tee 6
              local.set 3
              local.get 6
              i32.const 14
              i32.ne
              br_if 0 (;@5;)
            end
            i32.const 32
            call $__print
            local.get 2
            i64.const 0
            i64.store offset=24
            local.get 2
            i32.const 3
            i32.store offset=16
            i32.const 4400
            i32.const 1348
            local.get 2
            i32.const 16
            i32.add
            call $sprintf_
            drop
            block  ;; label = @5
              i32.const 0
              i32.load8_u offset=4400
              local.tee 3
              i32.eqz
              br_if 0 (;@5;)
              i32.const 4401
              local.set 5
              loop  ;; label = @6
                local.get 3
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 5
                i32.load8_u
                local.set 3
                local.get 5
                i32.const 1
                i32.add
                local.set 5
                local.get 3
                br_if 0 (;@6;)
              end
            end
            i32.const 32
            local.set 5
            i32.const 1
            local.set 3
            loop  ;; label = @5
              local.get 5
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 3
              i32.const 3939
              i32.add
              i32.load8_u
              local.set 5
              local.get 3
              i32.const 1
              i32.add
              local.tee 6
              local.set 3
              local.get 6
              i32.const 7
              i32.ne
              br_if 0 (;@5;)
            end
            local.get 1
            local.get 8
            call $RuntimeProgram::printOpcodeAt_unsigned_short_
            local.set 3
            i32.const 32
            call $__print
            i32.const 32
            call $__print
            block  ;; label = @5
              local.get 3
              i32.const 58
              i32.gt_s
              br_if 0 (;@5;)
              local.get 3
              i32.const -59
              i32.add
              local.set 3
              loop  ;; label = @6
                i32.const 45
                call $__print
                local.get 3
                i32.const 1
                i32.add
                local.tee 3
                br_if 0 (;@6;)
              end
              i32.const 59
              local.set 3
            end
            block  ;; label = @5
              local.get 3
              i32.const 60
              i32.gt_s
              br_if 0 (;@5;)
              local.get 3
              i32.const -61
              i32.add
              local.set 3
              loop  ;; label = @6
                i32.const 32
                call $__print
                local.get 3
                i32.const 1
                i32.add
                local.tee 3
                br_if 0 (;@6;)
              end
            end
            local.get 0
            i32.load offset=4
            call $Stack<unsigned_char>::print__
            drop
            i32.const 10
            call $__print
            block  ;; label = @5
              local.get 10
              br_if 0 (;@5;)
              local.get 1
              i32.load16_u offset=8
              local.get 1
              i32.load16_u offset=6
              i32.ge_u
              local.get 7
              i32.or
              local.set 7
              br 2 (;@3;)
            end
            local.get 12
            local.set 9
            br 3 (;@1;)
          end
          i32.const 40
          call $operator_new___unsigned_long_
          local.set 11
          block  ;; label = @4
            block  ;; label = @5
              local.get 9
              i32.const 24
              i32.lt_u
              br_if 0 (;@5;)
              local.get 11
              i32.const 15
              i32.add
              i32.const 0
              i32.load offset=2038 align=1
              i32.store align=1
              local.get 11
              i32.const 8
              i32.add
              i32.const 0
              i64.load offset=2031 align=1
              i64.store align=1
              local.get 11
              i32.const 0
              i64.load offset=2023 align=1
              i64.store align=1
              br 1 (;@4;)
            end
            local.get 9
            i32.const 2
            i32.shl
            i32.const 4032
            i32.add
            i32.load
            local.set 6
            i32.const 0
            local.set 3
            loop  ;; label = @5
              local.get 11
              local.get 3
              i32.add
              local.get 6
              local.get 3
              i32.add
              i32.load8_u
              local.tee 5
              i32.store8
              local.get 5
              i32.eqz
              br_if 1 (;@4;)
              local.get 3
              i32.const 1
              i32.add
              local.tee 3
              i32.const 255
              i32.ne
              br_if 0 (;@5;)
            end
            local.get 11
            i32.const 0
            i32.store8 offset=255
          end
          i32.const 69
          local.set 5
          i32.const 1
          local.set 3
          loop  ;; label = @4
            local.get 5
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 3
            i32.const 3478
            i32.add
            i32.load8_u
            local.set 5
            local.get 3
            i32.const 1
            i32.add
            local.tee 6
            local.set 3
            local.get 6
            i32.const 26
            i32.ne
            br_if 0 (;@4;)
          end
          local.get 2
          local.get 8
          i32.store offset=32
          i32.const 4400
          i32.const 1466
          local.get 2
          i32.const 32
          i32.add
          call $sprintf_
          drop
          block  ;; label = @4
            i32.const 0
            i32.load8_u offset=4400
            local.tee 3
            i32.eqz
            br_if 0 (;@4;)
            i32.const 4401
            local.set 5
            loop  ;; label = @5
              local.get 3
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 5
              i32.load8_u
              local.set 3
              local.get 5
              i32.const 1
              i32.add
              local.set 5
              local.get 3
              br_if 0 (;@5;)
            end
          end
          i32.const 58
          local.set 5
          i32.const 1
          local.set 3
          loop  ;; label = @4
            local.get 5
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 3
            i32.const 3915
            i32.add
            i32.load8_u
            local.set 5
            local.get 3
            i32.const 1
            i32.add
            local.tee 6
            local.set 3
            local.get 6
            i32.const 3
            i32.ne
            br_if 0 (;@4;)
          end
          block  ;; label = @4
            local.get 11
            i32.load8_u
            local.tee 3
            i32.eqz
            br_if 0 (;@4;)
            local.get 11
            i32.const 1
            i32.add
            local.set 5
            loop  ;; label = @5
              local.get 3
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 5
              i32.load8_u
              local.set 3
              local.get 5
              i32.const 1
              i32.add
              local.set 5
              local.get 3
              br_if 0 (;@5;)
            end
          end
          i32.const 10
          call $__print
          local.get 9
          local.set 12
          local.get 10
          br_if 2 (;@1;)
        end
        local.get 7
        i32.const 1
        i32.and
        i32.eqz
        br_if 0 (;@2;)
      end
      block  ;; label = @2
        local.get 0
        i32.load8_u
        br_if 0 (;@2;)
        local.get 0
        call $VovkPLCRuntime::startup__
      end
      local.get 4
      i32.const 0
      i32.store16
      block  ;; label = @2
        local.get 0
        i32.load offset=4
        local.tee 3
        i32.load offset=8
        i32.eqz
        br_if 0 (;@2;)
        local.get 3
        i32.const 0
        i32.store offset=8
      end
      block  ;; label = @2
        local.get 3
        i32.const 20
        i32.add
        i32.load
        i32.eqz
        br_if 0 (;@2;)
        local.get 3
        i32.const 0
        i32.store offset=20
      end
      local.get 1
      i32.load16_u offset=6
      local.set 3
      local.get 1
      i32.load
      local.set 5
      block  ;; label = @2
        local.get 0
        i32.load8_u
        br_if 0 (;@2;)
        local.get 0
        call $VovkPLCRuntime::startup__
      end
      local.get 2
      i32.const 0
      i32.store16 offset=94
      block  ;; label = @2
        local.get 3
        i32.const 65535
        i32.and
        local.tee 3
        i32.eqz
        br_if 0 (;@2;)
        loop  ;; label = @3
          local.get 0
          local.get 5
          local.get 3
          local.get 2
          i32.const 94
          i32.add
          call $VovkPLCRuntime::step_unsigned_char*__unsigned_short__unsigned_short&_
          br_if 1 (;@2;)
          local.get 2
          i32.load16_u offset=94
          i32.const 65535
          i32.and
          local.get 3
          i32.lt_u
          br_if 0 (;@3;)
        end
      end
      i32.const 76
      local.set 5
      i32.const 1
      local.set 3
      loop  ;; label = @2
        local.get 5
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 3468
        i32.add
        i32.load8_u
        local.set 5
        local.get 3
        i32.const 1
        i32.add
        local.tee 6
        local.set 3
        local.get 6
        i32.const 10
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 0
      i32.load offset=4
      call $Stack<unsigned_char>::print__
      drop
      i32.const 10
      call $__print
      i32.const 84
      local.set 5
      i32.const 1
      local.set 3
      loop  ;; label = @2
        local.get 5
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 3840
        i32.add
        i32.load8_u
        local.set 5
        local.get 3
        i32.const 1
        i32.add
        local.tee 6
        local.set 3
        local.get 6
        i32.const 26
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 2
      i64.const 0
      i64.store offset=8
      local.get 2
      i32.const 3
      i32.store
      i32.const 4400
      i32.const 1348
      local.get 2
      call $sprintf_
      drop
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 3
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 5
        loop  ;; label = @3
          local.get 3
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 5
          i32.load8_u
          local.set 3
          local.get 5
          i32.const 1
          i32.add
          local.set 5
          local.get 3
          br_if 0 (;@3;)
        end
      end
      i32.const 32
      local.set 5
      i32.const 1
      local.set 3
      loop  ;; label = @2
        local.get 5
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 1299
        i32.add
        i32.load8_u
        local.set 5
        local.get 3
        i32.const 1
        i32.add
        local.tee 6
        local.set 3
        local.get 6
        i32.const 4
        i32.ne
        br_if 0 (;@2;)
      end
      i32.const 10
      call $__print
      local.get 9
      i32.eqz
      br_if 0 (;@1;)
      i32.const 68
      local.set 5
      i32.const 1
      local.set 3
      loop  ;; label = @2
        local.get 5
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 3793
        i32.add
        i32.load8_u
        local.set 5
        local.get 3
        i32.const 1
        i32.add
        local.tee 6
        local.set 3
        local.get 6
        i32.const 26
        i32.ne
        br_if 0 (;@2;)
      end
      i32.const 40
      call $operator_new___unsigned_long_
      local.set 6
      block  ;; label = @2
        block  ;; label = @3
          local.get 9
          i32.const 24
          i32.lt_u
          br_if 0 (;@3;)
          local.get 6
          i32.const 15
          i32.add
          i32.const 0
          i32.load offset=2038 align=1
          i32.store align=1
          local.get 6
          i32.const 8
          i32.add
          i32.const 0
          i64.load offset=2031 align=1
          i64.store align=1
          local.get 6
          i32.const 0
          i64.load offset=2023 align=1
          i64.store align=1
          br 1 (;@2;)
        end
        local.get 9
        i32.const 2
        i32.shl
        i32.const 4032
        i32.add
        i32.load
        local.set 8
        i32.const 0
        local.set 3
        loop  ;; label = @3
          local.get 6
          local.get 3
          i32.add
          local.get 8
          local.get 3
          i32.add
          i32.load8_u
          local.tee 5
          i32.store8
          local.get 5
          i32.eqz
          br_if 1 (;@2;)
          local.get 3
          i32.const 1
          i32.add
          local.tee 3
          i32.const 255
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 6
        i32.const 0
        i32.store8 offset=255
      end
      block  ;; label = @2
        local.get 6
        i32.load8_u
        local.tee 3
        i32.eqz
        br_if 0 (;@2;)
        local.get 6
        i32.const 1
        i32.add
        local.set 5
        loop  ;; label = @3
          local.get 3
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 5
          i32.load8_u
          local.set 3
          local.get 5
          i32.const 1
          i32.add
          local.set 5
          local.get 3
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
    end
    local.get 2
    i32.const 96
    i32.add
    global.set $__stack_pointer
    local.get 9)
  (func $VovkPLCRuntime::cleanRun__ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 16
    i32.sub
    local.tee 1
    global.set $__stack_pointer
    block  ;; label = @1
      local.get 0
      i32.load8_u
      br_if 0 (;@1;)
      local.get 0
      call $VovkPLCRuntime::startup__
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load offset=24
        local.tee 2
        br_if 0 (;@2;)
        i32.const 18
        local.set 3
        br 1 (;@1;)
      end
      local.get 2
      i32.const 0
      i32.store16 offset=8
      block  ;; label = @2
        local.get 0
        i32.load offset=4
        local.tee 3
        i32.load offset=8
        i32.eqz
        br_if 0 (;@2;)
        local.get 3
        i32.const 0
        i32.store offset=8
      end
      block  ;; label = @2
        local.get 3
        i32.const 20
        i32.add
        i32.load
        i32.eqz
        br_if 0 (;@2;)
        local.get 3
        i32.const 0
        i32.store offset=20
      end
      local.get 2
      i32.load16_u offset=6
      local.set 3
      local.get 2
      i32.load
      local.set 4
      block  ;; label = @2
        local.get 0
        i32.load8_u
        br_if 0 (;@2;)
        local.get 0
        call $VovkPLCRuntime::startup__
      end
      local.get 1
      i32.const 0
      i32.store16 offset=14
      local.get 3
      i32.const 65535
      i32.and
      local.set 5
      loop  ;; label = @2
        block  ;; label = @3
          local.get 1
          i32.load16_u offset=14
          local.get 5
          i32.lt_u
          br_if 0 (;@3;)
          i32.const 0
          local.set 3
          br 2 (;@1;)
        end
        i32.const 0
        local.get 0
        local.get 4
        local.get 5
        local.get 1
        i32.const 14
        i32.add
        call $VovkPLCRuntime::step_unsigned_char*__unsigned_short__unsigned_short&_
        local.tee 2
        local.get 2
        i32.const 16
        i32.eq
        select
        local.get 3
        local.get 2
        select
        local.set 3
        local.get 2
        i32.eqz
        br_if 0 (;@2;)
      end
    end
    local.get 1
    i32.const 16
    i32.add
    global.set $__stack_pointer
    local.get 3)
  (func $_start (type 4))
  (func $setup (type 4)
    call $runtime_unit_test__)
  (func $loop (type 4)
    (local i32 i32 i32 i32 i32 f32 i32)
    global.get $__stack_pointer
    i32.const 112
    i32.sub
    local.tee 0
    global.set $__stack_pointer
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4384
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4848
      call $VovkPLCRuntime::startup__
      i32.const 67
      local.set 1
      i32.const 1
      local.set 2
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 2
        i32.const 1112
        i32.add
        i32.load8_u
        local.set 1
        local.get 2
        i32.const 1
        i32.add
        local.tee 3
        local.set 2
        local.get 3
        i32.const 20
        i32.ne
        br_if 0 (;@2;)
      end
      i32.const 10
      call $__print
      i32.const 86
      local.set 1
      i32.const 1
      local.set 2
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 2
        i32.const 1024
        i32.add
        i32.load8_u
        local.set 1
        local.get 2
        i32.const 1
        i32.add
        local.tee 3
        local.set 2
        local.get 3
        i32.const 52
        i32.ne
        br_if 0 (;@2;)
      end
      i32.const 10
      call $__print
      i32.const 70
      local.set 1
      i32.const 1
      local.set 2
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 2
        i32.const 3148
        i32.add
        i32.load8_u
        local.set 1
        local.get 2
        i32.const 1
        i32.add
        local.tee 3
        local.set 2
        local.get 3
        i32.const 85
        i32.ne
        br_if 0 (;@2;)
      end
      i32.const 10
      call $__print
      i32.const 82
      local.set 1
      i32.const 1
      local.set 2
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 2
        i32.const 3068
        i32.add
        i32.load8_u
        local.set 1
        local.get 2
        i32.const 1
        i32.add
        local.tee 3
        local.set 2
        local.get 3
        i32.const 80
        i32.ne
        br_if 0 (;@2;)
      end
      i32.const 10
      call $__print
      i32.const 82
      local.set 1
      i32.const 1
      local.set 2
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 2
        i32.const 2422
        i32.add
        i32.load8_u
        local.set 1
        local.get 2
        i32.const 1
        i32.add
        local.tee 3
        local.set 2
        local.get 3
        i32.const 20
        i32.ne
        br_if 0 (;@2;)
      end
      i32.const 10
      call $__print
      i32.const 10
      call $__print
      i32.const 0
      i32.load offset=4872
      local.tee 2
      i32.const 12
      i32.store offset=12
      local.get 2
      i32.const 0
      i32.store offset=6 align=2
      local.get 2
      f32.const 0x1.4p+3 (;=10;)
      call $RuntimeProgram::push_float_float_
      drop
      i32.const 0
      i32.load offset=4872
      f32.const 0x1p+0 (;=1;)
      call $RuntimeProgram::push_float_float_
      drop
      i32.const 0
      i32.load offset=4872
      f32.const 0x1p+0 (;=1;)
      call $RuntimeProgram::push_float_float_
      drop
      i32.const 0
      i32.load offset=4872
      f32.const 0x1p+1 (;=2;)
      call $RuntimeProgram::push_float_float_
      drop
      i32.const 0
      i32.load offset=4872
      f32.const 0x1.8p+1 (;=3;)
      call $RuntimeProgram::push_float_float_
      drop
      i32.const 0
      i32.load offset=4872
      f32.const 0x1.8p+1 (;=3;)
      call $RuntimeProgram::push_float_float_
      drop
      i32.const 0
      i32.load offset=4872
      f32.const 0x1p+2 (;=4;)
      call $RuntimeProgram::push_float_float_
      drop
      i32.const 0
      i32.load offset=4872
      f32.const 0x1p+2 (;=4;)
      call $RuntimeProgram::push_float_float_
      drop
      i32.const 0
      i32.load offset=4872
      f32.const 0x1.4p+2 (;=5;)
      call $RuntimeProgram::push_float_float_
      drop
      i32.const 0
      i32.load offset=4872
      f32.const 0x1.4p+2 (;=5;)
      call $RuntimeProgram::push_float_float_
      drop
      i32.const 0
      i32.load offset=4872
      f32.const 0x1.8p+2 (;=6;)
      call $RuntimeProgram::push_float_float_
      drop
      i32.const 0
      i32.load offset=4872
      i32.const 33
      i32.const 10
      call $RuntimeProgram::push_unsigned_char__unsigned_char_
      drop
      i32.const 0
      i32.load offset=4872
      i32.const 34
      i32.const 10
      call $RuntimeProgram::push_unsigned_char__unsigned_char_
      drop
      i32.const 0
      i32.load offset=4872
      i32.const 33
      i32.const 10
      call $RuntimeProgram::push_unsigned_char__unsigned_char_
      drop
      i32.const 0
      i32.load offset=4872
      i32.const 34
      i32.const 10
      call $RuntimeProgram::push_unsigned_char__unsigned_char_
      drop
      i32.const 0
      i32.load offset=4872
      i32.const 32
      i32.const 10
      call $RuntimeProgram::push_unsigned_char__unsigned_char_
      drop
      i32.const 0
      i32.load offset=4872
      i32.const 34
      i32.const 10
      call $RuntimeProgram::push_unsigned_char__unsigned_char_
      drop
      i32.const 0
      i32.load offset=4872
      i32.const 32
      i32.const 10
      call $RuntimeProgram::push_unsigned_char__unsigned_char_
      drop
      i32.const 0
      i32.load offset=4872
      f32.const 0x1p+2 (;=4;)
      call $RuntimeProgram::push_float_float_
      drop
      i32.const 0
      i32.load offset=4872
      i32.const 35
      i32.const 10
      call $RuntimeProgram::push_unsigned_char__unsigned_char_
      drop
      i32.const 0
      i32.load offset=4872
      i32.const 34
      i32.const 10
      call $RuntimeProgram::push_unsigned_char__unsigned_char_
      drop
      i32.const 0
      i32.load offset=4872
      i32.const 33
      i32.const 10
      call $RuntimeProgram::push_unsigned_char__unsigned_char_
      drop
      i32.const 0
      i32.load offset=4872
      i32.const 34
      i32.const 10
      call $RuntimeProgram::push_unsigned_char__unsigned_char_
      drop
      block  ;; label = @2
        block  ;; label = @3
          i32.const 0
          i32.load offset=4872
          local.tee 2
          br_if 0 (;@3;)
          i32.const 78
          local.set 1
          i32.const 1
          local.set 2
          loop  ;; label = @4
            local.get 1
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 2
            i32.const 1414
            i32.add
            i32.load8_u
            local.set 1
            local.get 2
            i32.const 1
            i32.add
            local.tee 3
            local.set 2
            local.get 3
            i32.const 33
            i32.ne
            br_if 0 (;@4;)
          end
          i32.const 10
          call $__print
          i32.const 18
          local.set 4
          br 1 (;@2;)
        end
        i32.const 4848
        local.get 2
        call $UnitTest::fullProgramDebug_VovkPLCRuntime&__RuntimeProgram&_
        local.set 4
      end
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4848
        br_if 0 (;@2;)
        i32.const 4848
        call $VovkPLCRuntime::startup__
      end
      block  ;; label = @2
        block  ;; label = @3
          i32.const 0
          i32.load offset=4852
          local.tee 3
          i32.load16_u offset=8
          i32.const 65532
          i32.and
          br_if 0 (;@3;)
          f32.const 0x0p+0 (;=0;)
          local.set 5
          br 1 (;@2;)
        end
        i32.const 0
        local.set 2
        loop  ;; label = @3
          i32.const 0
          local.set 1
          block  ;; label = @4
            local.get 3
            i32.load offset=8
            local.tee 6
            i32.eqz
            br_if 0 (;@4;)
            local.get 3
            local.get 6
            i32.const -1
            i32.add
            local.tee 1
            i32.store offset=8
            local.get 3
            i32.load
            local.get 1
            i32.add
            i32.load8_u
            local.set 1
          end
          local.get 0
          i32.const 108
          i32.add
          local.get 2
          i32.add
          local.get 1
          i32.store8
          local.get 2
          i32.const 1
          i32.add
          local.tee 2
          i32.const 4
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 0
        f32.load offset=108
        local.set 5
      end
      i32.const 40
      call $operator_new___unsigned_long_
      local.set 6
      block  ;; label = @2
        block  ;; label = @3
          local.get 4
          i32.const 24
          i32.lt_u
          br_if 0 (;@3;)
          local.get 6
          i32.const 15
          i32.add
          i32.const 0
          i32.load offset=2038 align=1
          i32.store align=1
          local.get 6
          i32.const 8
          i32.add
          i32.const 0
          i64.load offset=2031 align=1
          i64.store align=1
          local.get 6
          i32.const 0
          i64.load offset=2023 align=1
          i64.store align=1
          br 1 (;@2;)
        end
        local.get 4
        i32.const 2
        i32.shl
        i32.const 4032
        i32.add
        i32.load
        local.set 3
        i32.const 0
        local.set 2
        loop  ;; label = @3
          local.get 6
          local.get 2
          i32.add
          local.get 3
          local.get 2
          i32.add
          i32.load8_u
          local.tee 1
          i32.store8
          local.get 1
          i32.eqz
          br_if 1 (;@2;)
          local.get 2
          i32.const 1
          i32.add
          local.tee 2
          i32.const 255
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 6
        i32.const 0
        i32.store8 offset=255
      end
      i32.const 82
      local.set 1
      i32.const 1
      local.set 2
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 2
        i32.const 3737
        i32.add
        i32.load8_u
        local.set 1
        local.get 2
        i32.const 1
        i32.add
        local.tee 3
        local.set 2
        local.get 3
        i32.const 17
        i32.ne
        br_if 0 (;@2;)
      end
      block  ;; label = @2
        local.get 6
        i32.load8_u
        local.tee 2
        i32.eqz
        br_if 0 (;@2;)
        local.get 6
        i32.const 1
        i32.add
        local.set 1
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 1
          i32.load8_u
          local.set 2
          local.get 1
          i32.const 1
          i32.add
          local.set 1
          local.get 2
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
      i32.const 82
      local.set 1
      i32.const 1
      local.set 2
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 2
        i32.const 3645
        i32.add
        i32.load8_u
        local.set 1
        local.get 2
        i32.const 1
        i32.add
        local.tee 3
        local.set 2
        local.get 3
        i32.const 9
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 0
      i32.const 10
      i32.store offset=80
      local.get 0
      local.get 5
      f64.promote_f32
      f64.store offset=88
      i32.const 4400
      i32.const 1348
      local.get 0
      i32.const 80
      i32.add
      call $sprintf_
      drop
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 2
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 1
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 1
          i32.load8_u
          local.set 2
          local.get 1
          i32.const 1
          i32.add
          local.set 1
          local.get 2
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
      i32.const 69
      local.set 1
      i32.const 1
      local.set 2
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 2
        i32.const 3622
        i32.add
        i32.load8_u
        local.set 1
        local.get 2
        i32.const 1
        i32.add
        local.tee 3
        local.set 2
        local.get 3
        i32.const 18
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 0
      i64.const -4579606667175395328
      i64.store offset=72
      local.get 0
      i32.const 10
      i32.store offset=64
      i32.const 4400
      i32.const 1348
      local.get 0
      i32.const 64
      i32.add
      call $sprintf_
      drop
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 2
        i32.eqz
        br_if 0 (;@2;)
        i32.const 4401
        local.set 1
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 1
          i32.load8_u
          local.set 2
          local.get 1
          i32.const 1
          i32.add
          local.set 1
          local.get 2
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
      i32.const 84
      local.set 1
      i32.const 1
      local.set 2
      loop  ;; label = @2
        local.get 1
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 2
        i32.const 3876
        i32.add
        i32.load8_u
        local.set 1
        local.get 2
        i32.const 1
        i32.add
        local.tee 3
        local.set 2
        local.get 3
        i32.const 14
        i32.ne
        br_if 0 (;@2;)
      end
      block  ;; label = @2
        i32.const 1704
        i32.const 3272
        local.get 5
        f32.const -0x1.1f8p+8 (;=-287.5;)
        f32.eq
        select
        local.tee 1
        i32.load8_u
        local.tee 2
        i32.eqz
        br_if 0 (;@2;)
        local.get 1
        i32.const 1
        i32.add
        local.set 1
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 1
          i32.load8_u
          local.set 2
          local.get 1
          i32.const 1
          i32.add
          local.set 1
          local.get 2
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
      i32.const 10
      call $__print
      i32.const 0
      i32.const 0
      i32.store8 offset=4384
    end
    i32.const 0
    local.set 4
    loop  ;; label = @1
      i32.const 4848
      call $VovkPLCRuntime::cleanRun__
      drop
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4848
        br_if 0 (;@2;)
        i32.const 4848
        call $VovkPLCRuntime::startup__
      end
      block  ;; label = @2
        block  ;; label = @3
          i32.const 0
          i32.load offset=4852
          local.tee 3
          i32.load16_u offset=8
          i32.const 65532
          i32.and
          br_if 0 (;@3;)
          f32.const 0x0p+0 (;=0;)
          local.set 5
          br 1 (;@2;)
        end
        i32.const 0
        local.set 2
        loop  ;; label = @3
          i32.const 0
          local.set 1
          block  ;; label = @4
            local.get 3
            i32.load offset=8
            local.tee 6
            i32.eqz
            br_if 0 (;@4;)
            local.get 3
            local.get 6
            i32.const -1
            i32.add
            local.tee 1
            i32.store offset=8
            local.get 3
            i32.load
            local.get 1
            i32.add
            i32.load8_u
            local.set 1
          end
          local.get 0
          i32.const 108
          i32.add
          local.get 2
          i32.add
          local.get 1
          i32.store8
          local.get 2
          i32.const 1
          i32.add
          local.tee 2
          i32.const 4
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 0
        f32.load offset=108
        local.set 5
      end
      local.get 4
      i32.const 1
      i32.add
      local.tee 4
      i32.const 10000
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 80
    local.set 1
    i32.const 1
    local.set 2
    loop  ;; label = @1
      local.get 1
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 2
      i32.const 3446
      i32.add
      i32.load8_u
      local.set 1
      local.get 2
      i32.const 1
      i32.add
      local.tee 3
      local.set 2
      local.get 3
      i32.const 22
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 0
    i32.const 10000
    i32.store offset=48
    i32.const 4400
    i32.const 1466
    local.get 0
    i32.const 48
    i32.add
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 2
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 1
      loop  ;; label = @2
        local.get 2
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 1
        i32.load8_u
        local.set 2
        local.get 1
        i32.const 1
        i32.add
        local.set 1
        local.get 2
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 1
    i32.const 1
    local.set 2
    loop  ;; label = @1
      local.get 1
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 2
      i32.const 3514
      i32.add
      i32.load8_u
      local.set 1
      local.get 2
      i32.const 1
      i32.add
      local.tee 3
      local.set 2
      local.get 3
      i32.const 12
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 0
    i64.const 0
    i64.store offset=40
    local.get 0
    i32.const 3
    i32.store offset=32
    i32.const 4400
    i32.const 1348
    local.get 0
    i32.const 32
    i32.add
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 2
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 1
      loop  ;; label = @2
        local.get 2
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 1
        i32.load8_u
        local.set 2
        local.get 1
        i32.const 1
        i32.add
        local.set 1
        local.get 2
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 1
    i32.const 1
    local.set 2
    loop  ;; label = @1
      local.get 1
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 2
      i32.const 3640
      i32.add
      i32.load8_u
      local.set 1
      local.get 2
      i32.const 1
      i32.add
      local.tee 3
      local.set 2
      local.get 3
      i32.const 14
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 0
    local.get 5
    f64.promote_f32
    f64.store offset=16
    i32.const 4400
    i32.const 1353
    local.get 0
    i32.const 16
    i32.add
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 2
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 1
      loop  ;; label = @2
        local.get 2
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 1
        i32.load8_u
        local.set 2
        local.get 1
        i32.const 1
        i32.add
        local.set 1
        local.get 2
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 1
    i32.const 1
    local.set 2
    loop  ;; label = @1
      local.get 1
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 2
      i32.const 3574
      i32.add
      i32.load8_u
      local.set 1
      local.get 2
      i32.const 1
      i32.add
      local.tee 3
      local.set 2
      local.get 3
      i32.const 16
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 0
    i32.const 9000
    i32.store
    i32.const 4400
    i32.const 1466
    local.get 0
    call $sprintf_
    drop
    block  ;; label = @1
      i32.const 0
      i32.load8_u offset=4400
      local.tee 2
      i32.eqz
      br_if 0 (;@1;)
      i32.const 4401
      local.set 1
      loop  ;; label = @2
        local.get 2
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 1
        i32.load8_u
        local.set 2
        local.get 1
        i32.const 1
        i32.add
        local.set 1
        local.get 2
        br_if 0 (;@2;)
      end
    end
    i32.const 32
    local.set 1
    i32.const 1
    local.set 2
    loop  ;; label = @1
      local.get 1
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 2
      i32.const 2824
      i32.add
      i32.load8_u
      local.set 1
      local.get 2
      i32.const 1
      i32.add
      local.tee 3
      local.set 2
      local.get 3
      i32.const 8
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    local.get 0
    i32.const 112
    i32.add
    global.set $__stack_pointer)
  (func $__original_main (type 16) (result i32)
    call $runtime_unit_test__
    loop (result i32)  ;; label = @1
      call $loop
      br 0 (;@1;)
    end)
  (func $_out_rev_void__*__char__void*__unsigned_long__unsigned_long___char*__unsigned_long__unsigned_long__char_const*__unsigned_long__unsigned_int__unsigned_int_ (type 17) (param i32 i32 i32 i32 i32 i32 i32 i32) (result i32)
    (local i32 i32)
    local.get 2
    local.set 8
    block  ;; label = @1
      local.get 7
      i32.const 3
      i32.and
      br_if 0 (;@1;)
      local.get 2
      local.set 8
      local.get 5
      local.get 6
      i32.ge_u
      br_if 0 (;@1;)
      local.get 6
      local.get 5
      i32.sub
      local.set 9
      local.get 2
      local.set 8
      loop  ;; label = @2
        i32.const 32
        local.get 1
        local.get 8
        local.get 3
        local.get 0
        call_indirect (type 0)
        local.get 8
        i32.const 1
        i32.add
        local.set 8
        local.get 9
        i32.const -1
        i32.add
        local.tee 9
        br_if 0 (;@2;)
      end
    end
    local.get 7
    i32.const 2
    i32.and
    local.set 7
    block  ;; label = @1
      local.get 5
      i32.eqz
      br_if 0 (;@1;)
      local.get 4
      i32.const -1
      i32.add
      local.set 9
      loop  ;; label = @2
        local.get 9
        local.get 5
        i32.add
        i32.load8_s
        local.get 1
        local.get 8
        local.get 3
        local.get 0
        call_indirect (type 0)
        local.get 8
        i32.const 1
        i32.add
        local.set 8
        local.get 5
        i32.const -1
        i32.add
        local.tee 5
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      local.get 7
      i32.eqz
      br_if 0 (;@1;)
      local.get 8
      local.get 2
      i32.sub
      local.get 6
      i32.ge_u
      br_if 0 (;@1;)
      i32.const 0
      local.get 2
      i32.sub
      local.set 5
      loop  ;; label = @2
        i32.const 32
        local.get 1
        local.get 8
        local.get 3
        local.get 0
        call_indirect (type 0)
        local.get 5
        local.get 8
        i32.const 1
        i32.add
        local.tee 8
        i32.add
        local.get 6
        i32.lt_u
        br_if 0 (;@2;)
      end
    end
    local.get 8)
  (func $RuntimeStack::RuntimeStack_unsigned_short__unsigned_short__unsigned_short_ (type 11) (param i32 i32 i32 i32) (result i32)
    (local i32 i32 i32 i32 i32)
    local.get 0
    i64.const 0
    i64.store align=4
    local.get 0
    i32.const 8
    i32.add
    local.tee 4
    i32.const 0
    i32.store
    i32.const 0
    call $operator_new___unsigned_long_
    local.set 5
    local.get 4
    i64.const 0
    i64.store align=4
    local.get 0
    local.get 5
    i32.store
    local.get 0
    i32.const 16
    i32.add
    i64.const 0
    i64.store align=4
    i32.const 0
    call $operator_new___unsigned_long_
    local.set 6
    local.get 0
    i32.const 20
    i32.add
    i64.const 0
    i64.store align=4
    local.get 0
    local.get 6
    i32.store offset=12
    local.get 0
    i32.const 28
    i32.add
    local.tee 6
    i64.const 0
    i64.store align=4
    local.get 0
    i32.const 36
    i32.add
    local.tee 7
    i32.const 0
    i32.store
    i32.const 0
    call $operator_new___unsigned_long_
    local.set 8
    local.get 7
    i32.const 0
    i32.store
    local.get 6
    local.get 8
    i32.store
    local.get 0
    local.get 2
    i32.store16 offset=26
    local.get 0
    local.get 1
    i32.store16 offset=24
    local.get 5
    call $operator_delete___void*_
    local.get 0
    local.get 1
    i32.store offset=4
    local.get 1
    call $operator_new___unsigned_long_
    local.set 5
    local.get 4
    local.get 1
    i32.store
    local.get 0
    local.get 5
    i32.store
    block  ;; label = @1
      local.get 6
      i32.load
      local.tee 1
      i32.eqz
      br_if 0 (;@1;)
      local.get 1
      call $operator_delete___void*_
    end
    local.get 0
    i32.const 32
    i32.add
    local.get 3
    i32.store
    local.get 3
    call $operator_new___unsigned_long_
    local.set 1
    local.get 0
    local.get 3
    i32.store offset=36
    local.get 0
    local.get 1
    i32.store offset=28
    local.get 0)
  (func $RuntimeProgram::print__ (type 2) (param i32) (result i32)
    (local i32 i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 32
    i32.sub
    local.tee 1
    global.set $__stack_pointer
    i32.const 80
    local.set 2
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 2
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 1474
      i32.add
      i32.load8_u
      local.set 2
      local.get 3
      i32.const 1
      i32.add
      local.tee 4
      local.set 3
      local.get 4
      i32.const 9
      i32.ne
      br_if 0 (;@1;)
    end
    local.get 1
    local.get 0
    i32.load16_u offset=6
    i32.store offset=16
    i32.const 4400
    i32.const 1466
    local.get 1
    i32.const 16
    i32.add
    call $sprintf_
    drop
    block  ;; label = @1
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 2
        br_if 0 (;@2;)
        i32.const 8
        local.set 5
        br 1 (;@1;)
      end
      i32.const 8
      local.set 3
      loop  ;; label = @2
        local.get 2
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 4393
        i32.add
        local.set 2
        local.get 3
        i32.const 1
        i32.add
        local.tee 5
        local.set 3
        local.get 2
        i32.load8_u
        local.tee 2
        br_if 0 (;@2;)
      end
    end
    block  ;; label = @1
      block  ;; label = @2
        local.get 0
        i32.load16_u offset=6
        i32.eqz
        br_if 0 (;@2;)
        i32.const 93
        local.set 2
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 1505
          i32.add
          i32.load8_u
          local.set 2
          local.get 3
          i32.const 1
          i32.add
          local.tee 4
          local.set 3
          local.get 4
          i32.const 4
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 5
        i32.const 3
        i32.add
        local.set 5
        block  ;; label = @3
          local.get 0
          i32.load16_u offset=6
          i32.eqz
          br_if 0 (;@3;)
          i32.const 0
          local.set 6
          loop  ;; label = @4
            block  ;; label = @5
              local.get 0
              i32.load
              local.get 6
              i32.add
              i32.load8_u
              local.tee 3
              i32.const 15
              i32.gt_u
              br_if 0 (;@5;)
              i32.const 48
              call $__print
              local.get 5
              i32.const 1
              i32.add
              local.set 5
            end
            local.get 1
            local.get 3
            i32.store
            i32.const 4400
            i32.const 1106
            local.get 1
            call $sprintf_
            drop
            i32.const 0
            local.set 4
            block  ;; label = @5
              i32.const 0
              i32.load8_u offset=4400
              local.tee 2
              i32.eqz
              br_if 0 (;@5;)
              i32.const 0
              local.set 3
              loop  ;; label = @6
                local.get 2
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 3
                i32.const 4401
                i32.add
                local.set 2
                local.get 3
                i32.const 1
                i32.add
                local.tee 4
                local.set 3
                local.get 2
                i32.load8_u
                local.tee 2
                br_if 0 (;@6;)
              end
            end
            local.get 4
            local.get 5
            i32.add
            local.set 5
            block  ;; label = @5
              local.get 0
              i32.load16_u offset=6
              i32.const -1
              i32.add
              local.get 6
              i32.le_s
              br_if 0 (;@5;)
              i32.const 32
              call $__print
              local.get 5
              i32.const 1
              i32.add
              local.set 5
            end
            local.get 6
            i32.const 1
            i32.add
            local.tee 6
            local.get 0
            i32.load16_u offset=6
            i32.lt_u
            br_if 0 (;@4;)
          end
        end
        i32.const 93
        call $__print
        local.get 5
        i32.const 1
        i32.add
        local.set 3
        br 1 (;@1;)
      end
      i32.const 93
      local.set 2
      i32.const 1
      local.set 3
      loop  ;; label = @2
        local.get 2
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 1469
        i32.add
        i32.load8_u
        local.set 2
        local.get 3
        i32.const 1
        i32.add
        local.tee 4
        local.set 3
        local.get 4
        i32.const 5
        i32.ne
        br_if 0 (;@2;)
      end
      local.get 5
      i32.const 4
      i32.add
      local.set 3
    end
    local.get 1
    i32.const 32
    i32.add
    global.set $__stack_pointer
    local.get 3)
  (func $RuntimeProgram::explain__ (type 1) (param i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 96
    i32.sub
    local.tee 1
    global.set $__stack_pointer
    i32.const 35
    local.set 2
    i32.const 1
    local.set 3
    loop  ;; label = @1
      local.get 2
      i32.const 24
      i32.shl
      i32.const 24
      i32.shr_s
      call $__print
      local.get 3
      i32.const 2158
      i32.add
      i32.load8_u
      local.set 2
      local.get 3
      i32.const 1
      i32.add
      local.tee 4
      local.set 3
      local.get 4
      i32.const 26
      i32.ne
      br_if 0 (;@1;)
    end
    i32.const 10
    call $__print
    block  ;; label = @1
      block  ;; label = @2
        block  ;; label = @3
          local.get 0
          i32.load16_u offset=6
          i32.eqz
          br_if 0 (;@3;)
          i32.const 0
          local.set 5
          i32.const 0
          local.set 6
          loop  ;; label = @4
            block  ;; label = @5
              local.get 5
              i32.const 1
              i32.and
              i32.eqz
              br_if 0 (;@5;)
              i32.const 35
              local.set 2
              i32.const 1
              local.set 3
              loop  ;; label = @6
                local.get 2
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 3
                i32.const 2832
                i32.add
                i32.load8_u
                local.set 2
                local.get 3
                i32.const 1
                i32.add
                local.tee 4
                local.set 3
                local.get 4
                i32.const 33
                i32.ne
                br_if 0 (;@6;)
                br 4 (;@2;)
              end
            end
            local.get 0
            i32.load
            local.get 6
            i32.const 65535
            i32.and
            local.tee 7
            i32.add
            i32.load8_u
            local.set 8
            i32.const 32
            local.set 2
            i32.const 1
            local.set 3
            loop  ;; label = @5
              local.get 2
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 3
              i32.const 3950
              i32.add
              i32.load8_u
              local.set 2
              local.get 3
              i32.const 1
              i32.add
              local.tee 4
              local.set 3
              local.get 4
              i32.const 5
              i32.ne
              br_if 0 (;@5;)
            end
            local.get 6
            i32.const 65535
            i32.and
            local.tee 9
            i32.eqz
            local.set 2
            block  ;; label = @5
              local.get 9
              i32.eqz
              br_if 0 (;@5;)
              local.get 7
              local.set 3
              loop  ;; label = @6
                local.get 2
                i32.const 1
                i32.add
                local.set 2
                local.get 3
                i32.const 9
                i32.gt_s
                local.set 4
                local.get 3
                i32.const 10
                i32.div_s
                local.set 3
                local.get 4
                br_if 0 (;@6;)
              end
            end
            block  ;; label = @5
              local.get 2
              i32.const 3
              i32.gt_s
              br_if 0 (;@5;)
              local.get 2
              i32.const -4
              i32.add
              local.set 3
              loop  ;; label = @6
                i32.const 32
                call $__print
                local.get 3
                i32.const 1
                i32.add
                local.tee 3
                br_if 0 (;@6;)
              end
            end
            local.get 1
            local.get 7
            i32.store offset=80
            i32.const 4400
            i32.const 1466
            local.get 1
            i32.const 80
            i32.add
            call $sprintf_
            drop
            block  ;; label = @5
              i32.const 0
              i32.load8_u offset=4400
              local.tee 3
              i32.eqz
              br_if 0 (;@5;)
              i32.const 4401
              local.set 2
              loop  ;; label = @6
                local.get 3
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 2
                i32.load8_u
                local.set 3
                local.get 2
                i32.const 1
                i32.add
                local.set 2
                local.get 3
                br_if 0 (;@6;)
              end
            end
            i32.const 32
            local.set 2
            i32.const 1
            local.set 3
            loop  ;; label = @5
              local.get 2
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 3
              i32.const 1510
              i32.add
              i32.load8_u
              local.set 2
              local.get 3
              i32.const 1
              i32.add
              local.tee 4
              local.set 3
              local.get 4
              i32.const 3
              i32.ne
              br_if 0 (;@5;)
            end
            local.get 9
            i32.const 8
            i32.shr_u
            local.set 10
            block  ;; label = @5
              local.get 9
              i32.const 256
              i32.lt_u
              local.tee 2
              br_if 0 (;@5;)
              local.get 10
              local.set 3
              loop  ;; label = @6
                local.get 2
                i32.const 1
                i32.add
                local.set 2
                local.get 3
                i32.const 15
                i32.gt_s
                local.set 4
                local.get 3
                i32.const 16
                i32.div_s
                local.set 3
                local.get 4
                br_if 0 (;@6;)
              end
            end
            block  ;; label = @5
              local.get 2
              i32.const 1
              i32.gt_s
              br_if 0 (;@5;)
              local.get 2
              i32.const -2
              i32.add
              local.set 3
              loop  ;; label = @6
                i32.const 48
                call $__print
                local.get 3
                i32.const 1
                i32.add
                local.tee 3
                br_if 0 (;@6;)
              end
            end
            local.get 1
            local.get 10
            i32.store offset=64
            i32.const 4400
            i32.const 1106
            local.get 1
            i32.const 64
            i32.add
            call $sprintf_
            drop
            block  ;; label = @5
              i32.const 0
              i32.load8_u offset=4400
              local.tee 3
              i32.eqz
              br_if 0 (;@5;)
              i32.const 4401
              local.set 2
              loop  ;; label = @6
                local.get 3
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 2
                i32.load8_u
                local.set 3
                local.get 2
                i32.const 1
                i32.add
                local.set 2
                local.get 3
                br_if 0 (;@6;)
              end
            end
            i32.const 32
            call $__print
            local.get 6
            i32.const 255
            i32.and
            local.tee 10
            i32.eqz
            local.set 2
            block  ;; label = @5
              local.get 10
              i32.eqz
              br_if 0 (;@5;)
              local.get 10
              local.set 3
              loop  ;; label = @6
                local.get 2
                i32.const 1
                i32.add
                local.set 2
                local.get 3
                i32.const 15
                i32.gt_s
                local.set 4
                local.get 3
                i32.const 16
                i32.div_s
                local.set 3
                local.get 4
                br_if 0 (;@6;)
              end
            end
            block  ;; label = @5
              local.get 2
              i32.const 1
              i32.gt_s
              br_if 0 (;@5;)
              local.get 2
              i32.const -2
              i32.add
              local.set 3
              loop  ;; label = @6
                i32.const 48
                call $__print
                local.get 3
                i32.const 1
                i32.add
                local.tee 3
                br_if 0 (;@6;)
              end
            end
            local.get 1
            local.get 10
            i32.store offset=48
            i32.const 4400
            i32.const 1106
            local.get 1
            i32.const 48
            i32.add
            call $sprintf_
            drop
            block  ;; label = @5
              i32.const 0
              i32.load8_u offset=4400
              local.tee 3
              i32.eqz
              br_if 0 (;@5;)
              i32.const 4401
              local.set 2
              loop  ;; label = @6
                local.get 3
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 2
                i32.load8_u
                local.set 3
                local.get 2
                i32.const 1
                i32.add
                local.set 2
                local.get 3
                br_if 0 (;@6;)
              end
            end
            i32.const 93
            local.set 2
            i32.const 1
            local.set 3
            loop  ;; label = @5
              local.get 2
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 3
              i32.const 3934
              i32.add
              i32.load8_u
              local.set 2
              local.get 3
              i32.const 1
              i32.add
              local.tee 4
              local.set 3
              local.get 4
              i32.const 5
              i32.ne
              br_if 0 (;@5;)
            end
            local.get 8
            i32.eqz
            local.set 2
            block  ;; label = @5
              local.get 8
              i32.eqz
              br_if 0 (;@5;)
              local.get 8
              local.set 3
              loop  ;; label = @6
                local.get 2
                i32.const 1
                i32.add
                local.set 2
                local.get 3
                i32.const 15
                i32.gt_s
                local.set 4
                local.get 3
                i32.const 16
                i32.div_s
                local.set 3
                local.get 4
                br_if 0 (;@6;)
              end
            end
            block  ;; label = @5
              local.get 2
              i32.const 1
              i32.gt_s
              br_if 0 (;@5;)
              local.get 2
              i32.const -2
              i32.add
              local.set 3
              loop  ;; label = @6
                i32.const 48
                call $__print
                local.get 3
                i32.const 1
                i32.add
                local.tee 3
                br_if 0 (;@6;)
              end
            end
            local.get 1
            local.get 8
            i32.store offset=32
            i32.const 4400
            i32.const 1106
            local.get 1
            i32.const 32
            i32.add
            call $sprintf_
            drop
            block  ;; label = @5
              i32.const 0
              i32.load8_u offset=4400
              local.tee 3
              i32.eqz
              br_if 0 (;@5;)
              i32.const 4401
              local.set 2
              loop  ;; label = @6
                local.get 3
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 2
                i32.load8_u
                local.set 3
                local.get 2
                i32.const 1
                i32.add
                local.set 2
                local.get 3
                br_if 0 (;@6;)
              end
            end
            i32.const 58
            local.set 2
            i32.const 1
            local.set 3
            loop  ;; label = @5
              local.get 2
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 3
              i32.const 3915
              i32.add
              i32.load8_u
              local.set 2
              local.get 3
              i32.const 1
              i32.add
              local.tee 4
              local.set 3
              local.get 4
              i32.const 3
              i32.ne
              br_if 0 (;@5;)
            end
            block  ;; label = @5
              block  ;; label = @6
                local.get 8
                call $OPCODE_EXISTS_PLCRuntimeInstructionSet_
                br_if 0 (;@6;)
                i32.const 73
                local.set 2
                i32.const 1
                local.set 3
                loop  ;; label = @7
                  local.get 2
                  i32.const 24
                  i32.shl
                  i32.const 24
                  i32.shr_s
                  call $__print
                  local.get 3
                  i32.const 3981
                  i32.add
                  i32.load8_u
                  local.set 2
                  local.get 3
                  i32.const 1
                  i32.add
                  local.tee 4
                  local.set 3
                  local.get 4
                  i32.const 16
                  i32.ne
                  br_if 0 (;@7;)
                end
                i32.const 10
                call $__print
                i32.const 0
                local.set 3
                br 1 (;@5;)
              end
              local.get 8
              call $OPCODE_SIZE_PLCRuntimeInstructionSet_
              local.set 10
              block  ;; label = @6
                block  ;; label = @7
                  local.get 8
                  call $OPCODE_NAME_PLCRuntimeInstructionSet_
                  local.tee 3
                  i32.load8_u
                  local.tee 2
                  br_if 0 (;@7;)
                  i32.const 0
                  local.set 4
                  br 1 (;@6;)
                end
                local.get 3
                i32.const 1
                i32.add
                local.set 8
                i32.const 0
                local.set 3
                loop  ;; label = @7
                  local.get 2
                  i32.const 24
                  i32.shl
                  i32.const 24
                  i32.shr_s
                  call $__print
                  local.get 8
                  local.get 3
                  i32.add
                  local.set 2
                  local.get 3
                  i32.const 1
                  i32.add
                  local.tee 4
                  local.set 3
                  local.get 2
                  i32.load8_u
                  local.tee 2
                  br_if 0 (;@7;)
                end
              end
              i32.const 32
              call $__print
              block  ;; label = @6
                local.get 4
                i32.const 11
                i32.gt_s
                br_if 0 (;@6;)
                local.get 4
                i32.const -12
                i32.add
                local.set 3
                loop  ;; label = @7
                  i32.const 45
                  call $__print
                  local.get 3
                  i32.const 1
                  i32.add
                  local.tee 3
                  br_if 0 (;@7;)
                end
              end
              i32.const 32
              call $__print
              i32.const 91
              local.set 2
              i32.const 1
              local.set 3
              loop  ;; label = @6
                local.get 2
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 3
                i32.const 3542
                i32.add
                i32.load8_u
                local.set 2
                local.get 3
                i32.const 1
                i32.add
                local.tee 4
                local.set 3
                local.get 4
                i32.const 7
                i32.ne
                br_if 0 (;@6;)
              end
              i32.const 32
              call $__print
              local.get 1
              local.get 10
              i32.store offset=16
              i32.const 4400
              i32.const 1466
              local.get 1
              i32.const 16
              i32.add
              call $sprintf_
              drop
              block  ;; label = @6
                i32.const 0
                i32.load8_u offset=4400
                local.tee 3
                i32.eqz
                br_if 0 (;@6;)
                i32.const 4401
                local.set 2
                loop  ;; label = @7
                  local.get 3
                  i32.const 24
                  i32.shl
                  i32.const 24
                  i32.shr_s
                  call $__print
                  local.get 2
                  i32.load8_u
                  local.set 3
                  local.get 2
                  i32.const 1
                  i32.add
                  local.set 2
                  local.get 3
                  br_if 0 (;@7;)
                end
              end
              i32.const 93
              local.set 2
              i32.const 1
              local.set 3
              loop  ;; label = @6
                local.get 2
                i32.const 24
                i32.shl
                i32.const 24
                i32.shr_s
                call $__print
                local.get 3
                i32.const 3549
                i32.add
                i32.load8_u
                local.set 2
                local.get 3
                i32.const 1
                i32.add
                local.tee 4
                local.set 3
                local.get 4
                i32.const 3
                i32.ne
                br_if 0 (;@6;)
              end
              block  ;; label = @6
                local.get 10
                i32.eqz
                br_if 0 (;@6;)
                block  ;; label = @7
                  block  ;; label = @8
                    local.get 9
                    local.get 0
                    i32.load16_u offset=6
                    i32.lt_u
                    br_if 0 (;@8;)
                    local.get 10
                    i32.const 0
                    i32.ne
                    local.set 8
                    br 1 (;@7;)
                  end
                  i32.const 0
                  local.set 9
                  local.get 7
                  local.set 3
                  loop  ;; label = @8
                    local.get 0
                    i32.load
                    local.get 3
                    i32.add
                    i32.load8_u
                    local.tee 8
                    i32.eqz
                    local.set 2
                    block  ;; label = @9
                      local.get 8
                      i32.eqz
                      br_if 0 (;@9;)
                      local.get 8
                      local.set 3
                      loop  ;; label = @10
                        local.get 2
                        i32.const 1
                        i32.add
                        local.set 2
                        local.get 3
                        i32.const 15
                        i32.gt_s
                        local.set 4
                        local.get 3
                        i32.const 16
                        i32.div_s
                        local.set 3
                        local.get 4
                        br_if 0 (;@10;)
                      end
                    end
                    block  ;; label = @9
                      local.get 2
                      i32.const 1
                      i32.gt_s
                      br_if 0 (;@9;)
                      local.get 2
                      i32.const -2
                      i32.add
                      local.set 3
                      loop  ;; label = @10
                        i32.const 48
                        call $__print
                        local.get 3
                        i32.const 1
                        i32.add
                        local.tee 3
                        br_if 0 (;@10;)
                      end
                    end
                    local.get 1
                    local.get 8
                    i32.store
                    i32.const 4400
                    i32.const 1106
                    local.get 1
                    call $sprintf_
                    drop
                    block  ;; label = @9
                      i32.const 0
                      i32.load8_u offset=4400
                      local.tee 3
                      i32.eqz
                      br_if 0 (;@9;)
                      i32.const 4401
                      local.set 2
                      loop  ;; label = @10
                        local.get 3
                        i32.const 24
                        i32.shl
                        i32.const 24
                        i32.shr_s
                        call $__print
                        local.get 2
                        i32.load8_u
                        local.set 3
                        local.get 2
                        i32.const 1
                        i32.add
                        local.set 2
                        local.get 3
                        br_if 0 (;@10;)
                      end
                    end
                    local.get 9
                    i32.const 1
                    i32.add
                    local.tee 9
                    i32.const 255
                    i32.and
                    local.tee 3
                    local.get 10
                    i32.lt_u
                    local.tee 8
                    i32.eqz
                    br_if 2 (;@6;)
                    local.get 3
                    local.get 7
                    i32.add
                    local.tee 3
                    local.get 0
                    i32.load16_u offset=6
                    i32.lt_u
                    br_if 0 (;@8;)
                  end
                end
                i32.const 32
                local.set 2
                i32.const 1
                local.set 3
                loop  ;; label = @7
                  local.get 2
                  i32.const 24
                  i32.shl
                  i32.const 24
                  i32.shr_s
                  call $__print
                  local.get 3
                  i32.const 3955
                  i32.add
                  i32.load8_u
                  local.set 2
                  local.get 3
                  i32.const 1
                  i32.add
                  local.tee 4
                  local.set 3
                  local.get 4
                  i32.const 26
                  i32.ne
                  br_if 0 (;@7;)
                end
                i32.const 10
                call $__print
                i32.const 0
                local.set 3
                local.get 8
                br_if 1 (;@5;)
              end
              i32.const 10
              call $__print
              local.get 6
              local.get 10
              i32.add
              local.tee 6
              i32.const 65535
              i32.and
              local.get 0
              i32.load16_u offset=6
              i32.ge_u
              local.get 5
              i32.or
              local.set 5
              i32.const 1
              local.set 3
            end
            local.get 3
            br_if 0 (;@4;)
            br 3 (;@1;)
          end
        end
        i32.const 80
        local.set 2
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 2
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 2806
          i32.add
          i32.load8_u
          local.set 2
          local.get 3
          i32.const 1
          i32.add
          local.tee 4
          local.set 3
          local.get 4
          i32.const 18
          i32.ne
          br_if 0 (;@3;)
        end
      end
      i32.const 10
      call $__print
    end
    local.get 1
    i32.const 96
    i32.add
    global.set $__stack_pointer)
  (func $RuntimeProgram::printOpcodeAt_unsigned_short_ (type 13) (param i32 i32) (result i32)
    (local i32 i32 i32 i32 i32 i32 i32 i32)
    global.get $__stack_pointer
    i32.const 48
    i32.sub
    local.tee 2
    global.set $__stack_pointer
    i32.const 0
    local.set 3
    block  ;; label = @1
      local.get 0
      i32.load16_u offset=6
      local.get 1
      i32.le_u
      br_if 0 (;@1;)
      block  ;; label = @2
        local.get 0
        i32.load
        local.get 1
        i32.add
        i32.load8_u
        local.tee 4
        call $OPCODE_EXISTS_PLCRuntimeInstructionSet_
        br_if 0 (;@2;)
        i32.const 79
        local.set 5
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 5
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 1087
          i32.add
          i32.load8_u
          local.set 5
          local.get 3
          i32.const 1
          i32.add
          local.tee 6
          local.set 3
          local.get 6
          i32.const 12
          i32.ne
          br_if 0 (;@3;)
        end
        i32.const 11
        local.set 7
        block  ;; label = @3
          local.get 4
          i32.const 15
          i32.gt_u
          br_if 0 (;@3;)
          i32.const 48
          call $__print
          i32.const 12
          local.set 7
        end
        local.get 2
        local.get 4
        i32.store offset=32
        i32.const 4400
        i32.const 1106
        local.get 2
        i32.const 32
        i32.add
        call $sprintf_
        drop
        i32.const 0
        local.set 4
        block  ;; label = @3
          i32.const 0
          i32.load8_u offset=4400
          local.tee 5
          i32.eqz
          br_if 0 (;@3;)
          i32.const 0
          local.set 3
          loop  ;; label = @4
            local.get 5
            i32.const 24
            i32.shl
            i32.const 24
            i32.shr_s
            call $__print
            local.get 3
            i32.const 4401
            i32.add
            local.set 5
            local.get 3
            i32.const 1
            i32.add
            local.tee 4
            local.set 3
            local.get 5
            i32.load8_u
            local.tee 5
            br_if 0 (;@4;)
          end
        end
        i32.const 93
        local.set 5
        i32.const 1
        local.set 3
        loop  ;; label = @3
          local.get 5
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 2112
          i32.add
          i32.load8_u
          local.set 5
          local.get 3
          i32.const 1
          i32.add
          local.tee 6
          local.set 3
          local.get 6
          i32.const 12
          i32.ne
          br_if 0 (;@3;)
        end
        local.get 4
        local.get 7
        i32.add
        i32.const 11
        i32.add
        local.set 3
        br 1 (;@1;)
      end
      i32.const 79
      local.set 5
      i32.const 1
      local.set 3
      loop  ;; label = @2
        local.get 5
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 1483
        i32.add
        i32.load8_u
        local.set 5
        local.get 3
        i32.const 1
        i32.add
        local.tee 6
        local.set 3
        local.get 6
        i32.const 8
        i32.ne
        br_if 0 (;@2;)
      end
      i32.const 9
      local.set 8
      local.get 4
      call $OPCODE_SIZE_PLCRuntimeInstructionSet_
      local.set 9
      block  ;; label = @2
        local.get 4
        i32.const 15
        i32.gt_u
        br_if 0 (;@2;)
        i32.const 48
        call $__print
        i32.const 10
        local.set 8
      end
      local.get 2
      local.get 4
      i32.store offset=16
      i32.const 4400
      i32.const 1106
      local.get 2
      i32.const 16
      i32.add
      call $sprintf_
      drop
      i32.const 0
      local.set 7
      block  ;; label = @2
        i32.const 0
        i32.load8_u offset=4400
        local.tee 5
        i32.eqz
        br_if 0 (;@2;)
        i32.const 0
        local.set 3
        loop  ;; label = @3
          local.get 5
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 3
          i32.const 4401
          i32.add
          local.set 5
          local.get 3
          i32.const 1
          i32.add
          local.tee 7
          local.set 3
          local.get 5
          i32.load8_u
          local.tee 5
          br_if 0 (;@3;)
        end
      end
      i32.const 93
      local.set 5
      i32.const 1
      local.set 3
      loop  ;; label = @2
        local.get 5
        i32.const 24
        i32.shl
        i32.const 24
        i32.shr_s
        call $__print
        local.get 3
        i32.const 3914
        i32.add
        i32.load8_u
        local.set 5
        local.get 3
        i32.const 1
        i32.add
        local.tee 6
        local.set 3
        local.get 6
        i32.const 4
        i32.ne
        br_if 0 (;@2;)
      end
      block  ;; label = @2
        block  ;; label = @3
          local.get 4
          call $OPCODE_NAME_PLCRuntimeInstructionSet_
          local.tee 3
          i32.load8_u
          local.tee 5
          br_if 0 (;@3;)
          i32.const 0
          local.set 6
          br 1 (;@2;)
        end
        local.get 3
        i32.const 1
        i32.add
        local.set 4
        i32.const 0
        local.set 3
        loop  ;; label = @3
          local.get 5
          i32.const 24
          i32.shl
          i32.const 24
          i32.shr_s
          call $__print
          local.get 4
          local.get 3
          i32.add
          local.set 5
          local.get 3
          i32.const 1
          i32.add
          local.tee 6
          local.set 3
          local.get 5
          i32.load8_u
          local.tee 5
          br_if 0 (;@3;)
        end
      end
      i32.const 32
      call $__print
      i32.const 32
      call $__print
      block  ;; label = @2
        local.get 8
        local.get 7
        i32.add
        local.get 6
        i32.add
        i32.const 3
        i32.add
        local.tee 4
        i32.const 24
        i32.gt_s
        br_if 0 (;@2;)
        local.get 7
        local.get 6
        i32.add
        local.get 8
        i32.add
        i32.const 2
        i32.add
        local.set 3
        loop  ;; label = @3
          i32.const 45
          call $__print
          local.get 3
          i32.const 1
          i32.add
          local.tee 3
          i32.const 24
          i32.lt_s
          br_if 0 (;@3;)
        end
        i32.const 25
        local.set 4
      end
      block  ;; label = @2
        local.get 4
        i32.const 26
        i32.gt_s
        br_if 0 (;@2;)
        local.get 4
        i32.const -27
        i32.add
        local.set 3
        loop  ;; label = @3
          i32.const 32
          call $__print
          local.get 3
          i32.const 1
          i32.add
          local.tee 3
          br_if 0 (;@3;)
        end
        i32.const 27
        local.set 4
      end
      i32.const 91
      call $__print
      block  ;; label = @2
        local.get 9
        i32.eqz
        br_if 0 (;@2;)
        local.get 9
        i32.const -1
        i32.add
        local.set 8
        i32.const 0
        local.set 7
        loop  ;; label = @3
          block  ;; label = @4
            local.get 0
            i32.load
            local.get 7
            local.get 1
            i32.add
            i32.add
            i32.load8_u
            local.tee 3
            i32.const 15
            i32.gt_u
            br_if 0 (;@4;)
            i32.const 48
            call $__print
            local.get 4
            i32.const 1
            i32.add
            local.set 4
          end
          local.get 2
          local.get 3
          i32.store
          i32.const 4400
          i32.const 1106
          local.get 2
          call $sprintf_
          drop
          i32.const 0
          local.set 6
          block  ;; label = @4
            i32.const 0
            i32.load8_u offset=4400
            local.tee 5
            i32.eqz
            br_if 0 (;@4;)
            i32.const 0
            local.set 3
            loop  ;; label = @5
              local.get 5
              i32.const 24
              i32.shl
              i32.const 24
              i32.shr_s
              call $__print
              local.get 3
              i32.const 4401
              i32.add
              local.set 5
              local.get 3
              i32.const 1
              i32.add
              local.tee 6
              local.set 3
              local.get 5
              i32.load8_u
              local.tee 5
              br_if 0 (;@5;)
            end
          end
          local.get 6
          local.get 4
          i32.add
          local.set 4
          block  ;; label = @4
            local.get 8
            local.get 7
            i32.le_s
            br_if 0 (;@4;)
            i32.const 32
            call $__print
            local.get 4
            i32.const 1
            i32.add
            local.set 4
          end
          local.get 7
          i32.const 1
          i32.add
          local.tee 7
          local.get 9
          i32.ne
          br_if 0 (;@3;)
        end
      end
      i32.const 93
      call $__print
      local.get 4
      i32.const 1
      i32.add
      local.set 3
    end
    local.get 2
    i32.const 48
    i32.add
    global.set $__stack_pointer
    local.get 3)
  (func $_GLOBAL__sub_I_simulator.cpp (type 4)
    (local i32)
    i32.const 0
    i32.const 0
    i32.store offset=4720
    i32.const 4728
    call $UnitTest::UnitTest__
    drop
    i32.const 0
    i64.const 0
    i64.store offset=4832 align=4
    i32.const 0
    i32.const 12
    i32.store offset=4844
    i32.const 0
    i32.const 86
    i32.store16 offset=4836
    i32.const 0
    i32.const 0
    i32.store16 offset=4840
    i32.const 86
    call $operator_new___unsigned_long_
    local.set 0
    i32.const 0
    i32.const 12
    i32.store offset=4844
    i32.const 0
    local.get 0
    i32.store offset=4832
    i32.const 0
    i64.const 0
    i64.store offset=4860 align=4
    i32.const 0
    i32.const 0
    i32.store offset=4838 align=2
    i32.const 0
    i32.const 0
    i32.store16 offset=4856
    i32.const 0
    i32.const 0
    i32.store8 offset=4848
    i32.const 0
    i32.const 0
    i32.store offset=4868
    i32.const 0
    call $operator_new___unsigned_long_
    local.set 0
    i32.const 0
    i64.const 0
    i64.store offset=4868 align=4
    i32.const 0
    local.get 0
    i32.store offset=4860
    i32.const 40
    call $operator_new_unsigned_long_
    i32.const 64
    i32.const 10
    i32.const 4
    call $RuntimeStack::RuntimeStack_unsigned_short__unsigned_short__unsigned_short_
    local.set 0
    i32.const 0
    i32.const 4832
    i32.store offset=4872
    i32.const 0
    i32.const 64
    i32.store16 offset=4856
    i32.const 0
    local.get 0
    i32.store offset=4852)
  (func $main (type 13) (param i32 i32) (result i32)
    call $__original_main)
  (func $_start.command_export (type 4)
    call $__wasm_call_ctors
    call $_start)
  (func $setup.command_export (type 4)
    call $__wasm_call_ctors
    call $setup)
  (func $loop.command_export (type 4)
    call $__wasm_call_ctors
    call $loop)
  (func $__original_main.command_export (type 16) (result i32)
    call $__wasm_call_ctors
    call $__original_main)
  (func $main.command_export (type 13) (param i32 i32) (result i32)
    call $__wasm_call_ctors
    local.get 0
    local.get 1
    call $main)
  (table (;0;) 3 3 funcref)
  (global $__stack_pointer (mut i32) (i32.const 70480))
  (export "_start" (func $_start.command_export))
  (export "setup" (func $setup.command_export))
  (export "loop" (func $loop.command_export))
  (export "__original_main" (func $__original_main.command_export))
  (export "main" (func $main.command_export))
  (elem (;0;) (i32.const 1) func $_out_null_char__void*__unsigned_long__unsigned_long_ $_out_buffer_char__void*__unsigned_long__unsigned_long_)
  (data $.rodata (i32.const 1024) "Variables  = { a: 1, b: 2, c: 3, d: 4, e: 5, f: 6 }\00Opcode [0x\00Opcode: [0x\00    0x\00%x\00%u\00Custom program test\00Runtime Unit Test\00jump => push 1 and jump to exit\00PUSH float\00PUSH uint8_t\00PUSH int8_t\00PUSH uint16_t\00PUSH int16_t\00PUSH uint64_t\00PUSH int64_t\00PUSH uint32_t\00PUSH int32_t\00 ms\00nan\00PUSH boolean\00jump_if => for loop sum\00fni\00%.*f\00%f\00logic_and => true && true\00 byte\00logic_or => true || false\00No program loaded in the runtime\00PUSH double\00Passed\00%d\00] []\00Program[\00Opcode[\00Executed in [\00] [\00) [\00INVALID_PROGRAM_INDEX\00CALL_STACK_OVERFLOW\00CALL_STACK_UNDERFLOW\00MOV\00DIV\00PUT\00EXECUTION_TIMEOUT\00JMP_IF_NOT\00CALL_IF_NOT\00LOGIC_NOT\00CMP_LT\00EXIT\00CMP_GT\00RET\00GET\00INVALID_MEMORY_ADDRESS\00STATUS_SUCCESS\00YES\00PROGRAM_POINTER_OUT_OF_BOUNDS\00LOGIC_OR\00LOGIC_XOR\00MEMORY_ACCESS_ERROR\00EXECUTION_ERROR\00CMP_EQ\00CMP_NEQ\00NOP\00JMP\00UNKNOWN_INSTRUCTION\00INVALID_INSTRUCTION\00INVALID_CHECKSUM\00EMPTY_PROGRAM\00NO_PROGRAM\00MUL\00CALL\00EMPTY_STACK\00JMP_IF\00CALL_IF\00INVALID_MEMORY_SIZE\00INVALID_STACK_SIZE\00CMP_LTE\00CMP_GTE\00UNDEFINED_STATE\00INVALID_DATA_TYPE\00UNKNOWN_ERROR_CODE\00UNKNOWN OPCODE\00LOGIC_AND\00PROGRAM_EXITED\00PROGRAM_SIZE_EXCEEDED\00ADD\00SUB\00] <Invalid>\00Report:\00Bytecode Instruction Set:\00#### Program Explanation:\00:: Using VovkPLCRuntime Library - Author J.Vovk <jozo132@gmail.com> ::\00bitwise_and_X8\00BW_NOT_X8\00BW_RSHIFT_X8\00BW_LSHIFT_X8\00BW_OR_X8\00BW_XOR_X8\00BW_AND_X8\00bitwise_and_X16\00BW_NOT_X16\00BW_RSHIFT_X16\00BW_LSHIFT_X16\00BW_OR_X16\00BW_XOR_X16\00BW_AND_X16\00Result     = -287.5\00BW_NOT_X64\00BW_RSHIFT_X64\00BW_LSHIFT_X64\00BW_OR_X64\00BW_XOR_X64\00BW_AND_X64\00demo_int8_t => (1 - 2) * 3\00demo_uint8_t => (1 + 2) * 3\00demo_uint16_t => (1 + 2) * 3\00demo_uint64_t => (1 + 2) * 3\00demo_uint32_t => (1 + 2) * 3\00BW_NOT_X32\00BW_RSHIFT_X32\00BW_LSHIFT_X32\00BW_OR_X32\00BW_XOR_X32\00BW_AND_X32\00demo_float => (0.1 + 0.2) * -1\00demo_double => (0.1 + 0.2) * -1\00cmp_eq => 1 == 1\00Program is empty.\00 bytes.\00#### End of program explanation.\00Runtime Unit Tests Report Completed.\00Runtime Unit Tests Completed.\00Executing Runtime Unit Tests...\00Starting detailed program debug...\00fni-\00--------------------> !!! FAILED !!! <--------------------\00fni+\00RPN        = 10|1|'a'|'b'|'c'|'c'|'d'|'d'|'e'|'e'|'f'|-|*|-|*|+|*|+|'d'|/|*|-|*\00Function   = 10 * (1 - 'a' * ('b' + 'c' * ('c' + 'd' * ('d'-'e' *('e'-'f')))) / 'd')\00Stack(\00Opcode (\00Test \22\00Stack overflow!\00NO - TEST DID NOT PASS !!!\00FAILED !!!\00 to stack at index \00 but target address is \00PUT: Memory access error. Max memory size is \00GET: Memory access error. Max memory size is \00Program executed for \00Leftover \00Error at program pointer \00    Step \00 cycles in \00Trying to push \00[size \00] \00 > \00MAX_STACK_SIZE = \00  Free memory: \00Running test: \00Program result: \00Expected result: \00 ms. Result: \00Pushed uint8_t: \00Pushed int8_t: \00Pushed uint16_t: \00Pushed uint32_t: \00Test status: \00Runtime status: \00Program pointer points out of bounds: \00Debug failed with error: \00Pushed instruction: \00Time to execute program: \00 - type: \00Test passed: \00Program size exceeded: \00]: \00 more bytes... \00] - \00 ms]  \00:  \00    \00 - OUT OF PROGRAM BOUNDS\0a\00INVALID OPCODE\0a\00RuntimeError Status List:\0a\00\00\00\00\00\00\00\00\00\99\06\00\00\18\07\00\00,\07\00\00\d5\07\00\00\04\06\00\00\18\06\00\00\a2\07\00\00s\07\00\00\ff\05\00\00\13\06\00\00\82\06\00\00\8e\07\00\00\c5\07\00\00\e9\05\00\00\22\08\00\00\ac\06\00\00\13\08\00\00Q\07\00\00_\07\00\00@\07\00\00\f1\06\00\004\06\00\00\dd\06\00\00\ec\09\00\00\00\00\00\00\09\00\00\00\08\0a\00\00\00\00\00\00\09\00\00\00B\0a\00\00\00\00\00\00\09\00\00\00%\0a\00\00\00\00\00\00\09\00\00\00\00\00\00\00\d1\09\00\00\00\00\00\00\fd\00\00\00\a6\0a\00\00\00\00\00\00\9a\99\99\be\c5\0a\00\00\00\00\00\00433333\d3\bf\cf\08\00\00\00\00\00\00\05\00\00\00\1f\09\00\00\00\00\00\00\0f\00\00\00L\05\00\00\00\00\00\00\01\00\00\00l\05\00\00\00\00\00\00\01\00\00\00\e5\0a\00\00\00\00\00\00\01\00\00\00~\04\00\00\10\00\00\00\01\00\00\00(\05\00\00\10\00\00\00d\00\00\00\00\00\00\00\00\00\00\00\00\00\f0?\00\00\00\00\00\00$@\00\00\00\00\00\00Y@\00\00\00\00\00@\8f@\00\00\00\00\00\88\c3@\00\00\00\00\00j\f8@\00\00\00\00\80\84.A\00\00\00\00\d0\12cA\00\00\00\00\84\d7\97A\00\00\00\00e\cd\cdA")
  (data $.data (i32.const 4384) "\01")
  (data $.bss (i32.const 4400) "\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00\00"))
