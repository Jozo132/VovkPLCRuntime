// @ts-check
"use strict"

global.a = 1
global.b = 2
global.c = 3
global.d = 4
global.e = 5
global.f = 6
global.g = 7
global.h = 8
global.i = 9
global.j = 10
global.k = 11
global.l = 12
global.x = 0
global.y = 0

const reset = () => {
    global.a = 1
    global.b = 2
    global.c = 3
    global.d = 4
    global.e = 5
    global.f = 6
    global.g = 7
    global.h = 8
    global.i = 9
    global.j = 10
    global.k = 11
    global.l = 12
    global.x = 0
    global.y = 0
}

const sample_formulas = [
    `1+2*3`,
    `1 + 2*3`,
    `1+2 *3`,
    `1 + 2 * 3`,
    `  1 + 2 * 3    `,
    `(1 + 2 * 3)`,
    `1 + (2 * 3)`,
    `(1 + (2 * 3))`,
    `(1+2)*3`,
    `((1+2)*3)`,
    `1+2*3+4`,
    `(("x"-3)/4)*("y"+2)`,
    `"test1"+"test2"`,
    `1 + max(2,3)`,
    `1 + max(2,3) + 4`,
    `1 + max(2,3) + 4 + max(5,6)`,
    `round(1 + sqrt(pow(2,3) + 4))`,

    `round(-10.5+5.4)`,

    // Hard examples
    `10 * (1 - "a" * ("b" - --"c" * ("c" + "d" * ("d"-"e" *("e"-"f")))) / "d")`,

    `1 * (1 + 1 * (1 - 1 * (1 + 1 * (1 - 1 * (1 + 1)))))`,

    `pow(pow(2,2),pow(2,2))`,
    `map("x", 0, 100, -600, 600)`,
    `map("x"-2, 0, 100, -600, 600)`,

    `(0-"x") * (0-"y")`,
    `-"x"*-"y"`,
]


const OPERATORS = ["+", "-", "*", "/", "**", "%", "&", "|", "^", "<<", ">>", "&&", "||", "==", "!=", "<", ">", "<=", ">=", "=", "+=", "-=", "*=", "/="]
const OPERATOR_GROUPS = {
    "seperator": [",", ";", "\n"],
    "assignment": ["=", "+=", "-=", "*=", "/="],
    "logical_or": ["||"],
    "logical_and": ["&&"],
    "bitwise_or": ["|"],
    "bitwise_xor": ["^"],
    "bitwise_and": ["&"],
    "equality": ["==", "!="],
    "relational": ["<", ">", "<=", ">="],
    "shift": ["<<", ">>"],
    "additive": ["+", "-"],
    "multiplicative": ["*", "/", "%"],
    "exponentiation": ["**"],
    "unary": ["+", "-", "~", "!", "++", "--"],
    "member": [".", "->"],
    "call": ["function_call"],
}
const OPERATOR_GROUP_NAMES = Object.keys(OPERATOR_GROUPS)

const getPrecedence = t => {
    if (!OPERATORS.includes(t)) return -1
    for (let i = 0; i < OPERATOR_GROUP_NAMES.length; i++) {
        const group_name = OPERATOR_GROUP_NAMES[i]
        const group = OPERATOR_GROUPS[group_name]
        if (group.includes(t)) return i
    }
    return -1
}

const isOperator = t => {
    return OPERATORS.includes(t)
}

const isModifier = t => {
    return ['++', '--'].includes(t)
}

const isNumeric = t => {
    // Check each character is a digit AND check there is at most one decimal point
    t = t || ""
    let is_hex = false
    let is_binary = false
    let is_octal = false
    if (t.startsWith('0x')) is_hex = true
    if (t.startsWith('0b')) is_binary = true
    if (t.startsWith('0o')) is_octal = true
    if (is_hex) {
        t = t.substr(2)
        for (let i = 0; i < t.length; i++) {
            const c = t[i]
            const is_underscore = c === "_"
            const is_num = c >= "0" && c <= "9"
            const is_hex = c >= "a" && c <= "f"
            const is_hex_upper = c >= "A" && c <= "F"
            if (!is_underscore && !is_num && !is_hex && !is_hex_upper) return false
        }
        return true
    }
    if (is_binary) {
        t = t.substr(2)
        for (let i = 0; i < t.length; i++) {
            const c = t[i]
            const is_underscore = c === "_"
            const is_num = c >= "0" && c <= "1"
            if (!is_underscore && !is_num) return false
        }
        return true
    }
    if (is_octal) {
        t = t.substr(2)
        for (let i = 0; i < t.length; i++) {
            const c = t[i]
            const is_underscore = c === "_"
            const is_num = c >= "0" && c <= "7"
            if (!is_underscore && !is_num) return false
        }
        return true
    }    
    let digits = 0
    let decimal_point_count = 0
    for (let i = 0; i < t.length; i++) {
        const c = t[i]
        const is_num = c >= "0" && c <= "9"
        const is_underscore = c === "_"
        if (c === ".") {
            decimal_point_count++
        } else if (!is_underscore && !is_num && c !== '-' && c !== '+') {
            return false
        }

        if (is_num) digits++
    }
    return digits > 0 && decimal_point_count <= 1
}

const isVariable = t => {
    // Check if the first and last characters are double quotes and the rest are letters or digits
    t = t || ""
    if (t.startsWith('--') || t.startsWith('++')) t = t.substr(2)
    if (t.startsWith('-') || t.startsWith('+')) t = t.substr(1)
    if (t.endsWith('--') || t.endsWith('++')) t = t.substr(0, t.length - 2)

    if (t.length <= 2) return false
    if (t[0] !== '"' || t[t.length - 1] !== '"') return false
    for (let i = 1; i < t.length - 1; i++) {
        const c = t[i]
        const is_lowercase_letter = c >= "a" && c <= "z"
        const is_uppercase_letter = c >= "A" && c <= "Z"
        const is_digit = c >= "0" && c <= "9"
        if (i === 1 && !is_lowercase_letter && !is_uppercase_letter) return false
        if (!is_lowercase_letter && !is_uppercase_letter && !is_digit) return false
    }
    return true
}

const isBracket = t => {
    return t === "(" || t === ")"
}

const isSeperator = t => {
    return t === "," || t === ";" || t === "\n"
}

const getType = t => {
    if (isOperator(t)) return "operator"
    if (isModifier(t)) return "modifier"
    if (isNumeric(t)) return "number"
    if (isVariable(t)) return "variable"
    if (isBracket(t)) return "bracket"
    if (isSeperator(t)) return "seperator"
    return "unknown"
}


const tokenize = code => {
    const tokens = []
    let t0 = ""
    let t1 = ""

    /** @type { (char?: string) => void } */
    const push = c => {
        if (c) t1 += c
        if (t1 !== "") {
            tokens.push(t1)
            t0 = t1
            t1 = ""
        }
    }

    let c0 = ""
    code = code.split('\n').map(line => line.trim()).filter(Boolean).join('; ')
    for (let i = 0; i < code.length; i++) {
        const c = code[i]
        const isSeperator = OPERATOR_GROUPS.seperator.includes(c)
        if (c === " ") {
            push()
        } else if (c === '=') {
            const join = ['=', '+', '-', '*', '/'].includes(c0)
            if (join) {
                tokens[tokens.length - 1] += c
            } else {
                push()
                push(c)
            }
        } else if (c === "(" || c === ")" || c == "," || c === ";") {
            push()
            push(c)
        } else if (c === "+" || c === "-" || c === "*" || c === "/") {
            if (c0 === c)
                tokens[tokens.length - 1] += c
            else {
                push()
                push(c)
            }
        } else if (isSeperator) {
            push()
            push(c)
        } else {
            t1 += c
        }
        c0 = c
    }

    push()

    let i = 1
    while (tokens.length > 1 && i < tokens.length) {
        const t00 = i > 1 ? tokens[i - 2] : ""
        const t0 = tokens[i - 1]
        const t1 = tokens[i]

        const t00_type = getType(t00)
        const t0_type = getType(t0)
        const t1_type = getType(t1)

        if (!["number", "variable"].includes(t00_type) && (t0_type === "modifier" || t0 === '-') && ["number", "variable"].includes(t1_type)) {
            // Merge the operator and the number/variable
            tokens[i - 1] += tokens[i]
            tokens.splice(i, 1)
            continue
        }
        if (["number", "variable"].includes(t0_type) && t1_type === "modifier") {
            // Merge the number/variable and the operator
            tokens[i] = tokens[i - 1] + tokens[i]
            tokens.splice(i - 1, 1)
            continue
        }

        i++
    }

    let previous = ''
    tokens.forEach((t, i) => {
        if (previous) {
            const type_prev = getType(previous)
            const type = getType(t)
            if (type === 'unknown') throw new Error(`Unknown token: ${t}`)
            if (type === 'variable' && type_prev === 'variable') throw new Error(`Missing operator between: ${previous} and ${t}`)
            if (type === 'variable' && type_prev === 'number') throw new Error(`Missing operator between: ${previous} and ${t}`)
            if (type === 'number' && type_prev === 'variable') throw new Error(`Missing operator between: ${previous} and ${t}`)
            if (type === 'number' && type_prev === 'number') throw new Error(`Missing operator between: ${previous} and ${t}`)
            if (t === '(' && type_prev === 'variable') throw new Error(`Missing operator between: ${previous} and ${t}`)
            if (t === '(' && type_prev === 'number') throw new Error(`Missing operator between: ${previous} and ${t}`)
            if (t === '(' && previous === ')') throw new Error(`Missing operator between: ${previous} and ${t}`)
            if (t === ')' && type_prev === 'operator') throw new Error(`Missing operand between: ${previous} and ${t}`)
            if (t === ')' && type_prev === 'modifier') throw new Error(`Missing operand between: ${previous} and ${t}`)

        }
        previous = t
    })

    return tokens
}


// console.log(tokenize(sample_formulas[18]))
// console.log(tokenize(sample_formulas[19]))
// console.log(tokenize(sample_formulas[20]))
// console.log(tokenize(sample_formulas[21]))
// console.log(tokenize(sample_formulas[22]))
// console.log(tokenize(sample_formulas[23]))
// console.log(tokenize(sample_formulas[24]))


/** @typedef { string } Token */

// Reverse Polish Notation (RPN) Shunting Yard Algorithm
// Input: ["1", "+", "2", "*", "3"]
// Output: ["1", "2", "3", "*", "+"]

/** @type { (infix: Token[]) => Token[] } */
const encode = infix => {
    const operator_stack = []
    let depth = 0
    const postfix = []
    for (let i = 0; i < infix.length; i++) {
        const t = infix[i]
        const type = getType(t)
        if (type === "number" || type === "variable") {
            postfix.push(t)
        } else if (type === "operator") {
            while (operator_stack.length > 0 && getPrecedence(operator_stack[operator_stack.length - 1]) >= getPrecedence(t)) {
                postfix.push(operator_stack.pop() + '')
            }
            operator_stack.push(t)
        } else if (t === "(") {
            operator_stack.push(t)
            depth++
        } else if (t === ")") {
            while (operator_stack[operator_stack.length - 1] !== "(") {
                postfix.push(operator_stack.pop() + '')
            }
            operator_stack.pop()
            depth--
        } else if (type === 'seperator') {
            while (operator_stack[operator_stack.length - 1]) {
                postfix.push(operator_stack.pop() + '')
            }
        } else {
            throw new Error(`Unknown token type: ${type} -> "${t}"`)
        }
    }

    while (operator_stack[operator_stack.length - 1]) {
        postfix.push(operator_stack.pop() + '')
    }

    return postfix
}

const parseValue = (x) => {
    const type = getType(x)
    let incr_left = false
    let incr_right = false
    let decr_left = false
    let decr_right = false
    if (typeof x === 'string') {
        incr_left = x.startsWith('++')
        incr_right = x.endsWith('++')
        decr_left = x.startsWith('--')
        decr_right = x.endsWith('--')
        if (incr_left || incr_right || decr_left || decr_right) x = x.replace('++', '').replace('--', '')
    }
    if (type === "number") return parseFloat(x)
    if (type === "variable") {
        x = x.replace(/"/g, '')
        global[x] = +global[x] || 0
        let value = +global[x]
        if (incr_right) global[x]++
        if (decr_right) global[x]--
        if (incr_left) { global[x]++; value += 1 }
        if (decr_left) { global[x]--; value -= 1 }
        return value
    }
    throw new Error(`Unknown value type: ${type}`)
}

const evaluateOperator = (operator, x, y) => {
    const x_name = typeof x === 'string' ? x.replace(/"/g, '').replace(/-/g, '').replace(/\+/g, '') : ''
    const y_name = typeof y === 'string' ? y.replace(/"/g, '').replace(/-/g, '').replace(/\+/g, '') : ''
    const a = parseValue(x)
    const b = parseValue(y)
    if (operator === "+") return a + b
    if (operator === "-") return a - b
    if (operator === "*") return a * b
    if (operator === "/") return a / b
    if (operator === "**") return a ** b
    if (operator === "%") return a % b
    if (operator === "&") return a & b
    if (operator === "|") return a | b
    if (operator === "^") return a ^ b
    if (operator === "<<") return a << b
    if (operator === ">>") return a >> b
    if (operator === "&&") return a && b
    if (operator === "||") return a || b
    if (operator === "==") return a == b
    if (operator === "!=") return a != b
    if (operator === "<") return a < b
    if (operator === ">") return a > b
    if (operator === "<=") return a <= b
    if (operator === ">=") return a >= b
    if (OPERATOR_GROUPS.assignment.includes(operator)) {
        if (!x || getType(x) !== "variable") throw new Error(`Left side of assignment must be a variable: ${x}`)
        if (operator === "=") global[x_name] = b
        if (operator === "+=") global[x_name] += b
        if (operator === "-=") global[x_name] -= b
        if (operator === "*=") global[x_name] *= b
        if (operator === "/=") global[x_name] /= b
        return global[x_name]
    }

    throw new Error(`Unknown operator: ${operator}`)
}

const evaluate = postfix => {
    const stack = []
    for (let i = 0; i < postfix.length; i++) {
        const t = postfix[i]
        const type = getType(t)
        if (type === "number") {
            stack.push(parseFloat(t))
        } else if (type === "variable") {
            stack.push(t)
        } else if (type === "operator") {
            const b = stack.pop()
            const a = stack.pop()
            const result = evaluateOperator(t, a, b)
            stack.push(result)
        }
    }
    return stack[stack.length - 1]
}


const ITOP = code => {
    console.log("infix:", code)
    const tokens = tokenize(code)
    // console.log("tokens:", tokens.join(' '))
    const rpn = encode(tokens)
    const output = rpn.join(' ')
    console.log(`postfix: "${output}"`)
    reset()
    const result = evaluate(rpn)
    console.log("\nparsed RPN result:", result)
    console.log(`Final value: "x" = ${global.x}`)
    reset()
    const expected_result = eval(code.replace(/"/g, ''))
    console.log("\nreal eval result:", expected_result)
    console.log(`Final value: "x" = ${global.x}`)
    return rpn
}


const rpn_to_vplc = (rpn) => {
    const output = []
    const variables = []
    const stack = []
    for (let i = 0; i < rpn.length; i++) {
        const is_last = i === rpn.length - 1
        const token = rpn[i]
        const type = getType(token)
        if (type === 'number') {
            output.push(`i32.const ${token} // 1 - push the number "${token}" onto the stack`)
            stack.push(token)
        } else if (type === 'variable') {
            const name = token.replace(/"/g, '').replace(/-/g, '').replace(/\+/g, '')
            if (!variables.find(v => v.name === name)) {
                const index = variables.length * 4
                const create = `const ${name} = ${index} // 2 - memory index for variable "${name}"`
                variables.push({ name, create })
            }
            output.push(`i32.load ${name} // 3 - load the value of the variable "${name}"`)
            stack.push(token)
            const prefix_modifier = token.startsWith('--') || token.startsWith('++')
            const postfix_modifier = token.endsWith('--') || token.endsWith('++')
            if (prefix_modifier) {
                output.push(`i32.const 1 // 4 - push the number "1" onto the stack - handling ++${name} or --${name}`)
                if (token.startsWith('++')) output.push(`i32.add // 5 - add the number "1" to the variable "${name}"`)
                else output.push(`i32.sub // 6 - subtract the number "1" from the variable "${name}"`)
                output.push(`i32.copy // 7 - keep the updated value for return`)
                output.push(`i32.store ${name} // 8 - store the updated value of the variable "${name}"`)
            }
            if (postfix_modifier) {
                output.push(`i32.copy // 9 - keep the original value for return - handling ${name}++ or ${name}--`)
                output.push(`i32.const 1 // 10 - push the number "1" onto the stack`)
                if (token.endsWith('++')) output.push(`i32.add // 11 - add the number "1" to the variable "${name}"`)
                else output.push(`i32.sub // 12 - subtract the number "1" from the variable "${name}"`)
                output.push(`i32.store ${name} // 13 - store the updated value of the variable "${name}"`)
            }
        } else if (type === 'operator') {

            const token_b = stack.pop() // Remove the last value from the stack
            const token_a = stack.pop() // Remove the second last value from the stack

            const token_a_type = getType(token_a)
            const token_b_type = getType(token_b)

            const isAssignment = OPERATOR_GROUPS.assignment.includes(token)
            if (isAssignment) {
                const oneToOne = token_a && token_b && token_a_type === 'variable' && (token_b_type === 'variable' || token_b_type === 'number')
                if (oneToOne) {
                    output.pop() // Remove the last value from the stack
                    output.pop() // Remove the second last value from the stack
                    const name = token_a.replace(/"/g, '').replace(/-/g, '').replace(/\+/g, '')
                    const name_b = token_b_type === 'variable' ? token_b.replace(/"/g, '').replace(/-/g, '').replace(/\+/g, '') : null
                    if (token === '=') {
                        if (token_b_type === 'variable') output.push(`i32.load ${name_b} // 14 - load the value of the variable "${name_b}"`)
                        else output.push(`i32.const ${token_b} // 15 - push the number "${token_b}" onto the stack`)
                        if (is_last) output.push(`i32.copy // 16 - keep the last value for return`)
                        output.push(`i32.store ${name} // 17 - store the value of the variable "${name}"`)
                    } else {
                        output.push(`i32.load ${name} // 18 - load the value of the variable "${name}"`)
                        if (token_b_type === 'variable') output.push(`i32.load ${name_b} // 19 - load the value of the variable "${name_b}"`)
                        else output.push(`i32.const ${token_b} // 20 - push the number "${token_b}" onto the stack`)
                        switch (token) {
                            case '+=': output.push(`i32.add // 21`); break
                            case '-=': output.push(`i32.sub // 22`); break
                            case '*=': output.push(`i32.mul // 23`); break
                            case '/=': output.push(`i32.div // 24`); break
                            default: throw new Error(`Unknown assignment operator: ${token}`)
                        }
                        if (is_last) output.push(`i32.copy // 25 - keep the last value for return`)
                        output.push(`i32.store ${name} // 26 - store the value of the variable "${name}"`)
                    }
                } else {
                    if (token_a_type !== 'variable') throw new Error(`Left side of assignment must be a variable: ${token_a}`)
                    const name = token_a.replace(/"/g, '').replace(/-/g, '').replace(/\+/g, '')
                    if (token === '=') {
                        output.push(`i32.store ${name} // 27 - store the value of the variable "${name}"`)
                        output.push(`i32.drop // 28 - drop original value of the variable "${name}" since we stored it directly to memory`)
                        if (is_last) output.push(`i32.load ${name} // 29 - load the value of the variable "${name}"`)
                    } else {
                        output.push(`i32.swap // 30 - swap the last two values on the stack`)
                        output.push(`i32.drop // 31 - drop original value of the variable "${name}" since we stored it directly to memory`)
                        output.push(`i32.load ${name} // 32 - load the value of the variable "${name}"`)
                        switch (token) {
                            case '+=':
                                output.push(`i32.add // 33`);
                                break
                            case '-=':
                                output.push(`i32.swap // 34 - swap the last two values on the stack`);
                                output.push(`i32.sub // 35`);
                                break
                            case '*=':
                                output.push(`i32.mul // 36`);
                                break
                            case '/=':
                                output.push(`i32.swap // 37 - swap the last two values on the stack`);
                                output.push(`i32.div // 38`);
                                break
                            default: throw new Error(`Unknown assignment operator: ${token}`)
                        }
                        if (is_last) output.push(`i32.copy // 39 - keep the last value for return`)
                        output.push(`i32.store ${name} // 40 - store the value of the variable "${name}"`)
                    }
                }

            } else {
                switch (token) {
                    case '+': output.push(`i32.add // 41`); break
                    case '-': output.push(`i32.sub // 42`); break
                    case '*': output.push(`i32.mul // 43`); break
                    case '/': output.push(`i32.div // 44`); break
                    case '%': output.push(`i32.mod // 45`); break
                    case '<': output.push(`i32.cmp_lt // 46`); break
                    case '>': output.push(`i32.cmp_gt // 47`); break
                    case '==': output.push(`i32.cmp_eq // 48`); break
                    case '!=': output.push(`i32.cmp_neq // 49`); break
                    case '>=': output.push(`i32.cmp_gte // 50`); break
                    case '<=': output.push(`i32.cmp_lte // 51`); break
                    default: throw new Error(`Unknown operator: ${token}`)
                }
            }

            stack.push(undefined) // Push a null value to the stack to keep the stack size the same
        } else {
            throw new Error(`Unknown token type: ${type} -> "${token}"`)
        }
    }
    while (variables.length > 0) {
        const variable = variables.pop()
        output.unshift(variable?.create)
    }
    output.forEach((line, i) => {
        output[i] = line.split('//')[0]
    })
    return output
}

// const temp_code = `1 + (2 * 3 + -4 * (++"a" + --"b" * (3 + 1)) + "c"++ - "a"++ - "a"++) % 13`
// const temp_code = `1 + 2 * 3 + -4 * ("a" + "b" * (3 + 1)) + "c"`
// const temp_code = `1 + 2 * 3 + 4 * (5 + 6 * 7) + 8`
// const temp_code = '10 * (1 - "a" * ("b" + "c" * ("c" + "d" * ("d"-"e" *("e"-"f")))) / "d")'
// const temp_code = '5 + 4 * 8 / 3 + 1'
// const temp_code = '(2 + (9 - 3)) * 3'
const temp_code = `
   "a" = 1
   "b" = 2
   "c" = 3
   "x" = 1
   "x" += ("a" + "b") * "c"
`

const rpn = ITOP(temp_code)
const vplc = rpn_to_vplc(rpn)
console.log(`\nPLC ASM:`)
console.log(vplc.map(x => '  ' + x).join('\n'))



