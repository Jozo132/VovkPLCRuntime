// @ts-check
"use strict"

const ptr_list = ['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', "k", "l", "x", "y"]

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
    let decimal_point_count = 0
    for (let i = 0; i < t.length; i++) {
        const c = t[i]
        const is_num = c >= "0" && c <= "9"
        if (c === ".") {
            decimal_point_count++
        } else if (!is_num && c !== '-' && c !== '+') {
            return false
        }
    }
    return decimal_point_count <= 1
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

const isAssignment = t => {
    return OPERATOR_GROUPS.assignment.includes(t)
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


/** @type { (code: string) => Token[] } */
const tokenize = code => {
    const tokens = []
    let t1 = ""

    /** @type { (char?: string) => void } */
    const push = c => {
        if (c) t1 += c
        if (t1 !== "") {
            tokens.push(t1)
            t1 = ""
        }
    }

    let c0 = ""
    code = code.split('\r\n').join('\n').split('\n').map(line => line.trim()).filter(Boolean).join('\n')
    for (let i = 0; i < code.length; i++) {
        const c = code[i]
        const isSeperator = OPERATOR_GROUPS.seperator.includes(c)
        const isBracket = c === "(" || c === ")"
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
        } else if (isBracket) {
            push()
            push(c)
        } else if (isSeperator) {
            push()
            push(c === '\n' ? ';' : c)
        } else if (c === "+" || c === "-" || c === "*" || c === "/") {
            if (c0 === c)
                tokens[tokens.length - 1] += c
            else {
                push()
                push(c)
            }
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
            if (isAssignment(t)) {
                const top = postfix.pop()
                postfix.push('ptr')
                postfix.push(top)
            }
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
            postfix.push(t)
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

const solveOperator = (operator, x, y) => {
    const b = parseValue(y)
    if (OPERATOR_GROUPS.assignment.includes(operator)) {
        const x_name = ptr_list[+x]
        if (typeof x_name !== 'string') throw new Error(`Unknown pointer: ${x}`)
        if (operator === "=") global[x_name] = b
        if (operator === "+=") global[x_name] += b
        if (operator === "-=") global[x_name] -= b
        if (operator === "*=") global[x_name] *= b
        if (operator === "/=") global[x_name] /= b
        return global[x_name]
    }

    const a = parseValue(x)
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

    throw new Error(`Unknown operator: ${operator}`)
}

const solveRPN = postfix => {
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
            const result = solveOperator(t, a, b)
            stack.push(result)
        } else if (t === ';' && i < postfix.length - 1) {
            const x = stack.pop()
            parseValue(x) // This will handle any modifiers
        } else if (t === 'ptr') {
            i++
            const name = postfix[i].replace(/"/g, '').replace(/-/g, '').replace(/\+/g, '')
            const index = ptr_list.indexOf(name)
            if (index === -1) throw new Error(`Unknown pointer: ${name}`)
            stack.push(`${index}`)
        } else {
            throw new Error(`Unknown token type: ${type} -> "${t}"`)
        }
    }
    // return stack[stack.length - 1]
    return stack
}


const rpn_to_vplc = (rpn) => {
    const output = []
    const variables = []
    const temp_stack = []
    let next_is_ptr = false
    for (let i = 0; i < rpn.length; i++) {
        const is_last = i === rpn.length - 1
        const token = rpn[i]
        const type = getType(token)
        if (type === 'number') {
            output.push(`i32.const ${token} // Constant numeric value`)
            temp_stack.push(token)
        } else if (type === 'variable') {
            const name = token.replace(/"/g, '').replace(/-/g, '').replace(/\+/g, '')
            if (!variables.find(v => v.name === name)) {
                const index = variables.length * 4
                const create = `const ${name} = ${index}`
                variables.push({ name, create })
            }
            temp_stack.push(token)
            const prefix_modifier = token.startsWith('--') || token.startsWith('++')
            const postfix_modifier = token.endsWith('--') || token.endsWith('++')
            const pre_op = prefix_modifier ? token.startsWith('++') ? 'add' : 'sub' : null
            const post_op = postfix_modifier ? token.endsWith('++') ? 'add' : 'sub' : null
            if (pre_op) {
                output.push(`i32.const ${name} // Handle unary operation: ${token}`)
                output.push(`ptr.const ${name}`)
                output.push(`i32.const 1`)
                output.push(`i32.${pre_op}`)
                if (next_is_ptr) {
                    output.push(`i32.move`)
                    output.push(`i32.const ${name}`)
                    next_is_ptr = false
                } else {
                    output.push(`i32.move_copy`)
                }
            }
            if (postfix_modifier) {
                if (prefix_modifier) throw new Error(`Cannot have both unary prefix and unary postfix operations: ${token}`)
                output.push(`ptr.const ${name} // Handle unary operation: ${token}`)
                output.push(`i32.load`)
                output.push(`i32.copy`)
                output.push(`i32.const ${name}`)
                output.push(`i32.swap`)
                output.push(`i32.const 1`)
                output.push(`i32.${post_op}`)
                output.push(`i32.move`)
                if (next_is_ptr) {
                    output.push(`i32.drop`)
                    output.push(`ptr.const ${name}`)
                    next_is_ptr = false
                }
            }
            if (!pre_op && !post_op) {
                if (next_is_ptr) {
                    output.push(`ptr.const ${name} // Load address of "${name}" (pointer)`)
                    next_is_ptr = false
                } else {
                    output.push(`ptr.const ${name} // Load value of "${name}"`)
                    output.push(`i32.load`)
                }
            }
        } else if (type === 'operator') {

            const token_b = temp_stack.pop() // Remove the last value from the stack
            const token_a = temp_stack.pop() // Remove the second last value from the stack

            const token_a_type = getType(token_a)
            const token_b_type = getType(token_b)

            const isAssignment = OPERATOR_GROUPS.assignment.includes(token)
            if (isAssignment) {
                if (token_a_type !== 'variable') throw new Error(`Left side of assignment must be a variable: ${token_a}`)
                const name = token_a.replace(/"/g, '').replace(/-/g, '').replace(/\+/g, '')
                if (token === '=') {
                    output.push(`i32.move_copy // Save value to "${name}"`)
                } else {
                    output.push(`ptr.const ${name} // Load value of "${name}"`)
                    output.push(`i32.load`)
                    switch (token) {
                        case '+=':
                            output.push(`i32.add`);
                            break
                        case '-=':
                            output.push(`i32.swap`);
                            output.push(`i32.sub`);
                            break
                        case '*=':
                            output.push(`i32.mul`);
                            break
                        case '/=':
                            output.push(`i32.swap`);
                            output.push(`i32.div`);
                            break
                        default: throw new Error(`Unknown assignment operator: ${token}`)
                    }
                    output.push(`i32.move_copy // Save value to "${name}"`)
                }

            } else {
                switch (token) {
                    case '+': output.push(`i32.add`); break
                    case '-': output.push(`i32.sub`); break
                    case '*': output.push(`i32.mul`); break
                    case '/': output.push(`i32.div`); break
                    case '%': output.push(`i32.mod`); break
                    case '<': output.push(`i32.cmp_lt`); break
                    case '>': output.push(`i32.cmp_gt`); break
                    case '==': output.push(`i32.cmp_eq`); break
                    case '!=': output.push(`i32.cmp_neq`); break
                    case '>=': output.push(`i32.cmp_gte`); break
                    case '<=': output.push(`i32.cmp_lte`); break
                    default: throw new Error(`Unknown operator: ${token}`)
                }
            }

            temp_stack.push(undefined) // Push a null value to the stack to keep the stack size the same
        } else if (type === 'seperator') {
            const last = output[output.length - 1]
            if (last) {
                const [code, comment] = last.split('//').map(x => x.trim())
                if (code === 'i32.move_copy') {
                    output[output.length - 1] = `i32.move${comment ? ` // ${comment.replace('Save', 'Move')}` : ''}`
                }
            }
            output.push(`// ------------------`)
        } else if (token === 'ptr') {
            next_is_ptr = true
        } else {
            throw new Error(`Unknown token type: ${type} -> "${token}"`)
        }
    }
    output.unshift(`// Bytecode:`)
    if (variables.length > 0) {
        while (variables.length > 0) {
            const variable = variables.pop()
            output.unshift(variable?.create)
        }
        output.unshift(`// Variable offsets:`)
    }
    return output
}



const ITOP = code => {
    console.log(`infix: ${code}`)
    const tokens = tokenize(code)
    // console.log(`tokens: ${tokens.join(' ')}`)
    const rpn = encode(tokens)
    const output = rpn.join(' ')
    console.log(`postfix: ${output}`)
    reset()
    const result = solveRPN(rpn)
    console.log("\nparsed RPN result:", result)
    console.log(`Final value: "x" = ${global.x}`)
    reset()
    const expected_result = eval(code.replace(/"/g, ''))
    console.log("\nreal eval result:", expected_result)
    console.log(`Final value: "x" = ${global.x}`)


    const vplc = rpn_to_vplc(rpn)
    vplc.forEach((line, i) => {
        const [code, comment] = line.split('//').map(x => x.trim())
        if (!code) return
        vplc[i] = `${code.padEnd(16, ' ')} ${comment ? `// ${comment}` : ''}`
        // vplc[i] = code
    })
    console.log(`\nPLC ASM:`)
    console.log(vplc.map(x => '  ' + x).join('\n'))

    return rpn
}


// const temp_code = `1 + (2 * 3 + -4 * (++"a" + --"b" * (3 + 1)) + "c"++ - "a"++ - "a"++) % 13`
// const temp_code = `1 + 2 * 3 + -4 * ("a" + "b" * (3 + 1)) + "c"`
// const temp_code = `1 + 2 * 3 + 4 * (5 + 6 * 7) + 8`
// const temp_code = '10 * (1 - "a" * ("b" + "c" * ("c" + "d" * ("d"-"e" *("e"-"f")))) / "d")'
// const temp_code = '5 + 4 * 8 / 3 + 1'
// const temp_code = '(2 + (9 - 3)) * 3'
// const temp_code = '"x" += ("a" + "b") * "c"'
const temp_code = `
   "a" = 1
   "b" = 2
   "c" = 3
   "x" = 1
   "x"++
   ++"x"
   "x" += ("a" + "b") * "c"
`

const rpn = ITOP(temp_code)



