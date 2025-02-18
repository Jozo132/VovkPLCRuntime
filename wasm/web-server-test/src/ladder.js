// @ts-check
"use strict"

/**
 * @typedef { 'input' | 'output' | 'memory' } PLC_Symbol_Location
 * @typedef { 'bit' | 'byte' | 'int' | 'dint' | 'real' } PLC_Symbol_Type
 * @typedef {{ name: string, location: PLC_Symbol_Location, type: PLC_Symbol_Type, address: number, initial_value: number, comment: string }} PLC_Symbol
 * @typedef { 'contact' | 'coil' | 'coil_set' | 'coil_rset' } PLC_Ladder_Block_Type
 * @typedef { 'normal' | 'rising' | 'falling' | 'change' } PLC_Trigger_Type
 * @typedef {{ id: number, x: number, y: number, type: PLC_Ladder_Block_Type, inverted: boolean, trigger: PLC_Trigger_Type, symbol: string, state?: { active: boolean, powered: boolean, terminated_input: boolean, terminated_output: boolean, evaluated: boolean, symbol: PLC_Symbol } }} PLC_LadderBlock
 * @typedef {{ id: number, from: { id: number, offset?: number }, to: { id: number, offset?: number }, state?: { powered: boolean, evaluated: boolean } }} PLC_LadderConnection 
 * @typedef {{ type: 'ladder', comment: string, blocks: PLC_LadderBlock[], connections: PLC_LadderConnection[] }} PLC_Ladder
 * @typedef {{ type: 'program', name: string, comment: string, blocks: PLC_Ladder[] }} PLC_Program
 * @typedef {{ type: 'folder', name: string, comment: string, children: (PLC_Folder | PLC_Program)[] }} PLC_Folder
 * 
 * @typedef {{ 
 *     offsets: {
 *         input: number,
 *         output: number,
 *         memory: number,
 *     }
 *     memory: number[],
 *     symbols: PLC_Symbol[],
 *     structure: (PLC_Folder | PLC_Program)[]
 * }} PLC_Structure
 */


let _blk = 0
let _con = 0

/** @type {PLC_Structure} */
const plc_structure = {
    offsets: {
        input: 10,
        output: 20,
        memory: 30,
    },
    memory: [
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, //  0 to  9
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 10 to 19
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // 20 to 29        
    ],
    symbols: [
        { name: 'button1', location: 'input', type: 'bit', address: 0.0, initial_value: 0, comment: 'Test input' },
        { name: 'button2', location: 'input', type: 'bit', address: 0.1, initial_value: 0, comment: 'Test input' },
        { name: 'button3', location: 'input', type: 'bit', address: 0.2, initial_value: 0, comment: 'Test input' },
        { name: 'button4', location: 'input', type: 'bit', address: 0.3, initial_value: 0, comment: 'Test input' },
        { name: 'light1', location: 'output', type: 'bit', address: 0.0, initial_value: 0, comment: 'Test output' },
        { name: 'light2', location: 'output', type: 'bit', address: 0.1, initial_value: 0, comment: 'Test output' },
    ],
    structure: [
        {
            type: 'folder',
            name: 'Programs',
            comment: '',
            children: [
                {
                    type: 'program',
                    name: 'Main',
                    comment: '',
                    blocks: [
                        {
                            type: 'ladder',
                            comment: '',
                            // Test
                            blocks: [
                                { id: 0, x: 0, y: 1, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button1' },
                                { id: 3, x: 0, y: 0, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button2' },
                                { id: 4, x: 1, y: 0, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button3' },
                                { id: 5, x: 0, y: 2, type: 'contact', inverted: false, trigger: 'normal', symbol: 'light1' },
                                { id: 6, x: 2, y: 0, type: 'contact', inverted: true, trigger: 'normal', symbol: 'button4' },
                                { id: 1, x: 4, y: 0, type: 'coil', inverted: false, trigger: 'normal', symbol: 'light1' },
                                { id: 2, x: 4, y: 1, type: 'coil_set', inverted: false, trigger: 'normal', symbol: 'light2' },


                                { id: 989, x: 1, y: 5, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button1' },
                                { id: 990, x: 3, y: 5, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button1' },
                                { id: 991, x: 4, y: 5, type: 'coil', inverted: false, trigger: 'normal', symbol: 'button1' },
                                { id: 992, x: 3, y: 6, type: 'coil', inverted: false, trigger: 'normal', symbol: 'button1' },
                                { id: 993, x: 4, y: 6, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button1' },
                                { id: 994, x: 1, y: 6, type: 'coil', inverted: false, trigger: 'normal', symbol: 'button1' },
                            ],
                            connections: [
                                { id: _con++, from: { id: 0 }, to: { id: 4 } },
                                { id: _con++, from: { id: 3 }, to: { id: 4 } },
                                { id: _con++, from: { id: 4 }, to: { id: 6 } },
                                { id: _con++, from: { id: 6 }, to: { id: 2 } },
                                { id: _con++, from: { id: 5 }, to: { id: 6 } },
                                { id: _con++, from: { id: 6 }, to: { id: 1 } },

                                { id: _con++, from: { id: 990 }, to: { id: 991 } },
                                { id: _con++, from: { id: 992 }, to: { id: 993 } },
                            ]
                            // // Toggle switch
                            // blocks: [
                            //     { id: 0, x: 0, y: 0, type: 'contact', inverted: false, trigger: 'rising', symbol: 'button1' },
                            //     { id: 1, x: 1, y: 0, type: 'contact', inverted: true, trigger: 'normal', symbol: 'light1' },
                            //     { id: 2, x: 2, y: 0, type: 'coil_set', inverted: false, trigger: 'normal', symbol: 'light1' },
                            //     { id: 3, x: 1, y: 1, type: 'contact', inverted: false, trigger: 'normal', symbol: 'light1' },
                            //     { id: 4, x: 2, y: 1, type: 'coil_rset', inverted: false, trigger: 'normal', symbol: 'light1' },
                            // ],
                            // connections: [
                            //     { id: _con++, from: { id: 0 }, to: { id: 1 } },
                            //     { id: _con++, from: { id: 1 }, to: { id: 2 } },
                            //     { id: _con++, from: { id: 0 }, to: { id: 3 } },
                            //     { id: _con++, from: { id: 3 }, to: { id: 4 } },
                            // ]
                        }
                    ] // blocks
                }
            ] // children
        }
    ] // structure
}



/** @type { HTMLCanvasElement } */// @ts-ignore
const ladder_canvas = document.getElementById('ladder-canvas')
if (!ladder_canvas) throw new Error('Canvas not found')
const ctx = ladder_canvas.getContext('2d')
if (!ctx) throw new Error('Context not found')

// Canvas fill background
const background_color = '#aaa'
ctx.fillStyle = background_color
ctx.fillRect(0, 0, ladder_canvas.width, ladder_canvas.height)


const ladder_block_width = 120
const ladder_block_height = 80

/** @type {(symbol: string | PLC_Symbol | undefined) => number | boolean} */
const get_symbol_value = (symbol) => {
    if (typeof symbol === 'string') {
        symbol = plc_structure.symbols.find(s => s.name === symbol)
    }
    if (!symbol) throw new Error(`Symbol not found: ${symbol}`)
    const location = plc_structure.offsets[symbol.location]
    const address = Math.floor(symbol.address)
    const bit = Math.min((symbol.address % 1) * 10, 7)
    const address_value = plc_structure.memory[address + location]
    if (symbol.type === 'bit') {
        return (address_value >> bit) & 1
    }
    return address_value
}

/** @type {(block: PLC_LadderBlock) => PLC_LadderBlock } */
const getBlockState = (block) => {
    if (!block.state) {
        const symbol = plc_structure.symbols.find(symbol => symbol.name === block.symbol)
        if (!symbol) throw new Error(`Symbol not found: ${block.symbol}`)
        block.state = { active: false, powered: false, evaluated: false, symbol, terminated_input: false, terminated_output: false }
    }
    return block
}

/** @type {(style: 'symbol' | 'highlight', ctx: CanvasRenderingContext2D , block: PLC_LadderBlock) => void} */
const draw_contact = (style, ctx, block) => {
    // input state: the left side of the contact (green when true)
    // output state: the right side of the contact (green when true)
    // value: the inner state of the contact (green when true)
    block = getBlockState(block)
    if (!block.state) throw new Error(`Block state not found: ${block.symbol}`)
    const { x, y, type, inverted, trigger, state } = block
    const symbol = state.symbol
    if (!symbol) throw new Error(`Symbol not found: ${block.symbol}`)
    let value = get_symbol_value(symbol)
    if (inverted) value = !value

    const x0 = x * ladder_block_width
    const y0 = y * ladder_block_height
    const x1 = x0 + ladder_block_width
    const y1 = y0 + ladder_block_height

    const x_mid = x0 + ladder_block_width / 2
    const y_mid = y0 + ladder_block_height / 2

    const cl = x0 + ladder_block_width * 2 / 5
    const cr = x0 + ladder_block_width * 3 / 5
    const ct = y0 + ladder_block_height * 1 / 3
    const cb = y0 + ladder_block_height * 2 / 3

    // console.log(`Drawing contact: ${block.symbol} at ${x}, ${y}`)

    //        Symbol Name
    //   --------|   |--------
    //      Location Address


    // Draw thick green line for the contact to symbolize the input state if true for the left and the right side of the contact
    if (style === 'highlight') {
        ctx.strokeStyle = '#0f0'
        ctx.lineWidth = 8
        ctx.beginPath()
        if (state.powered) {
            if (state.terminated_input) {
                ctx.moveTo(x0, y_mid - 12)
                ctx.lineTo(x0, y_mid + 12)
            }
            ctx.moveTo(x0, y_mid)
            ctx.lineTo(cl, y_mid)
        }

        const momentary = type === 'contact' && block.trigger !== 'normal'
        if (state.powered && value && !momentary) {
            ctx.moveTo(cr, y_mid)
            ctx.lineTo(x1, y_mid)
        }
        if (value) {
            ctx.fillStyle = ctx.strokeStyle
            // Draw box inside the contact
            ctx.roundRect(cl, ct, cr - cl, cb - ct, 2)
            ctx.fillRect(cl - 1, ct - 1, cr - cl + 2, cb - ct + 2)
        }
        ctx.stroke()
        return
    }

    if (style === 'symbol') {
        ctx.strokeStyle = '#000'
        ctx.lineWidth = 2

        // Draw horizontal line from the left side of the contact
        ctx.beginPath()
        if (state.terminated_input) {
            ctx.moveTo(x0, y_mid - 8)
            ctx.lineTo(x0, y_mid + 8)
        }
        ctx.moveTo(x0, y_mid)
        ctx.lineTo(cl, y_mid)
        ctx.moveTo(cr, y_mid)
        ctx.lineTo(x1, y_mid)

        // Draw vertical line for the contact
        if (inverted) {
            ctx.moveTo(cl, ct)
            ctx.lineTo(cl, cb)
            ctx.lineTo(cr, ct)
            ctx.lineTo(cr, cb)
        } else {
            ctx.moveTo(cl, ct)
            ctx.lineTo(cl, cb)
            ctx.moveTo(cr, ct)
            ctx.lineTo(cr, cb)
        }
        if (trigger !== 'normal') {
            ctx.moveTo(x_mid, ct)
            ctx.lineTo(x_mid, cb)
            if (trigger === 'rising' || trigger === 'change') {
                ctx.moveTo(x_mid - 4, ct + 4)
                ctx.lineTo(x_mid, ct)
                ctx.lineTo(x_mid + 4, ct + 4)
            }
            if (trigger === 'falling' || trigger === 'change') {
                ctx.moveTo(x_mid - 4, cb - 4)
                ctx.lineTo(x_mid, cb)
                ctx.lineTo(x_mid + 4, cb - 4)
            }
        }
        ctx.stroke()

        const short_location = symbol.location === 'input' ? 'I' : symbol.location === 'output' ? 'Q' : 'M'

        // Draw the symbol name
        ctx.fillStyle = '#421'
        ctx.font = '16px Arial'
        ctx.textAlign = 'center'
        ctx.textBaseline = 'middle'
        ctx.fillText(symbol.name, x_mid, ct - 13)
        ctx.fillText(`${short_location}: ${symbol.address.toFixed(1)}`, x_mid, cb + 13)
        return
    }

    throw new Error(`Invalid style: ${style}`)
}


/** @type {(style: 'symbol' | 'highlight', ctx: CanvasRenderingContext2D, block: PLC_LadderBlock) => void} */
const draw_coil = (style, ctx, block) => {
    // input state: the left side of the contact (green when true)
    // output state: the right side of the contact is equal to the input state
    // value: the inner state of the contact (green when true)
    block = getBlockState(block)
    if (!block.state) throw new Error(`Block state not found: ${block.symbol}`)
    const { x, y, type, inverted, trigger, state } = block
    const symbol = state.symbol
    if (!symbol) throw new Error(`Symbol not found: ${block.symbol}`)
    let value = get_symbol_value(symbol)
    if (inverted) value = !value

    const x0 = x * ladder_block_width
    const y0 = y * ladder_block_height
    const x1 = x0 + ladder_block_width
    const y1 = y0 + ladder_block_height

    const x_mid = x0 + ladder_block_width / 2
    const y_mid = y0 + ladder_block_height / 2

    const cl = x0 + ladder_block_width * 2 / 5
    const cr = x0 + ladder_block_width * 3 / 5
    const ct = y0 + ladder_block_height * 1 / 3
    const cb = y0 + ladder_block_height * 2 / 3

    // console.log(`Drawing coil: ${block.symbol} at ${x}, ${y}`)

    //        Symbol Name
    //   ---------O--------
    //      Location Address

    // The coil is a circle 

    // Draw thick green line for the contact to symbolize the input state if true for the left and the right side of the contact
    if (style === 'highlight') {
        ctx.strokeStyle = '#0f0'
        ctx.lineWidth = 8
        ctx.beginPath()
        if (state.powered) {
            ctx.moveTo(x0, y_mid)
            ctx.lineTo(cl, y_mid)
            ctx.moveTo(cr, y_mid)
            ctx.lineTo(x1, y_mid)
        }
        if (value) {
            ctx.fillStyle = ctx.strokeStyle
            // Draw circle inside the coil
            ctx.arc(x_mid, y_mid, (cr - cl) / 2, 0, Math.PI * 2)
            ctx.fill()
        }
        if (state.terminated_output) {
            ctx.moveTo(x1, y_mid - 12)
            ctx.lineTo(x1, y_mid + 12)
        }
        if (state.powered) {
            ctx.stroke()
        }
        return
    }

    if (style === 'symbol') {
        ctx.strokeStyle = '#000'
        ctx.lineWidth = 2

        // Draw horizontal line from the left side of the contact
        ctx.beginPath()

        ctx.moveTo(x0, y_mid)
        ctx.lineTo(cl, y_mid)
        ctx.moveTo(cr, y_mid)
        ctx.lineTo(x1, y_mid)

        // Draw circle for the coil
        ctx.arc(x_mid, y_mid, (cr - cl) / 2, 0, Math.PI * 2)


        if (state.terminated_output) {
            ctx.moveTo(x1, y_mid - 8)
            ctx.lineTo(x1, y_mid + 8)
        }

        ctx.stroke()


        const short_location = symbol.location === 'input' ? 'I' : symbol.location === 'output' ? 'Q' : 'M'

        // Draw the symbol name
        ctx.fillStyle = '#421'
        ctx.font = '16px Arial'
        ctx.textAlign = 'center'
        ctx.textBaseline = 'middle'
        ctx.fillText(symbol.name, x_mid, ct - 13)
        ctx.fillText(`${short_location}: ${symbol.address.toFixed(1)}`, x_mid, cb + 13)

        ctx.font = '18px Arial Black'
        if (type === 'coil_set') ctx.fillText('S', x_mid, y_mid + 1)
        if (type === 'coil_rset') ctx.fillText('R', x_mid, y_mid + 1)
        return
    }

    throw new Error(`Invalid style: ${style}`)
}


// Draw links between blocks

/** @typedef {{ from: PLC_LadderBlock , to: PLC_LadderBlock, powered: boolean }} LadderLink */
/** @type {(style: 'symbol' | 'highlight', ctx: CanvasRenderingContext2D , link: LadderLink) => void} */
const draw_connection = (style, ctx, link) => {
    // Draw a connection line from the output of the from block to the input of the to block
    // If the target block is bellow the source block, draw the vertical line first
    // If the target block is above the source block, draw the horizontal line first
    // If the target block is to the right, just draw a straight line

    const { from, to, powered } = link
    const x0 = from.x * ladder_block_width + ladder_block_width
    const y0 = from.y * ladder_block_height + ladder_block_height / 2
    const x1 = to.x * ladder_block_width
    const y1 = to.y * ladder_block_height + ladder_block_height / 2

    const x_direction = x0 < x1 ? 1 : x0 > x1 ? -1 : 0
    const y_direction = y0 < y1 ? 1 : y0 > y1 ? -1 : 0
    if (x_direction === 0 && y_direction === 0) return // elements are touching, no need to draw a connection
    if (style === 'highlight') {
        if (powered) {
            ctx.strokeStyle = '#0f0'
            ctx.lineWidth = 8
            ctx.beginPath()
            if (x_direction === 0) {
                if (y_direction > 0) ctx.moveTo(x0, y0 - 4)
                if (y_direction < 0) ctx.moveTo(x0, y0 + 4)
            } else {
                ctx.moveTo(x0, y0)
            }
            if (x_direction > 0) ctx.lineTo(x1, y0)
            if (y_direction > 0) ctx.lineTo(x1, y1 + 4)
            if (y_direction < 0) ctx.lineTo(x1, y1 - 4)
            ctx.stroke()
        }
        return
    }

    if (style === 'symbol') {
        ctx.strokeStyle = '#000'
        ctx.lineWidth = 2
        ctx.beginPath()
        ctx.moveTo(x0, y0)
        // if (y0 < y1) ctx.lineTo(x0, y1)
        // if (y0 > y1)
        ctx.lineTo(x1, y0)
        ctx.lineTo(x1, y1)
        ctx.stroke()
        return
    }

    throw new Error(`Invalid style: ${style}`)
}



/** @type {(ladder: PLC_Ladder) => void} */
const evaluate_ladder = (ladder) => {
    const { blocks, connections } = ladder
    // Reset the state of all blocks and connections
    const blockHasInputConnection = (block) => connections.some(connection => connection.to.id === block.id)
    blocks.forEach(block => {
        block = getBlockState(block)
        if (!block.state) throw new Error(`Block state not found: ${block.symbol}`)
        const { state, inverted } = block
        const symbol = state.symbol
        if (!symbol) throw new Error(`Symbol not found: ${block.symbol}`)
        let active = !!get_symbol_value(symbol)
        if (inverted) active = !active
        state.active = active // The actual state of the block
        // All blocks that have no input are powered by the power rail
        state.powered = !blockHasInputConnection(block)
        state.evaluated = false
    })
    connections.forEach(con => {
        con.state = con.state || {
            powered: false,
            evaluated: false
        }
        con.state.powered = false
        con.state.evaluated = false
    })



    const starting_blocks = blocks.filter(block => !blockHasInputConnection(block))
    starting_blocks.forEach(block => {
        if (!block.state) throw new Error(`Block state not found: ${block.symbol}`)
        block.state.terminated_input = true
    })
    const ending_blocks = blocks.filter(block => !connections.some(con => con.from.id === block.id))
    ending_blocks.forEach(block => {
        if (!block.state) throw new Error(`Block state not found: ${block.symbol}`)
        block.state.terminated_output = true
    })

    /** @type {(block: PLC_LadderBlock, first: boolean) => void} */
    const evaluate_powered_block = (block, first) => {
        if (!block.state) throw new Error(`Block state not found: ${block.symbol}`)
        if (block.state.evaluated) return
        const { state } = block
        const isContact = block.type === 'contact'
        const isCoil = ['coil', 'coil_set', 'coil_rset'].includes(block.type)
        const pass_through = isCoil && !first
        state.powered = isContact || (pass_through && !first)
        if (isCoil && first) return
        const momentary = block.trigger !== 'normal'
        if ((!momentary && state.active) || pass_through) {
            state.evaluated = true
            const outgoing_connections = connections.filter(con => con.from.id === block.id)
            outgoing_connections.forEach(con => {
                if (!con.state) throw new Error(`Connection state not found: ${con.from.id} -> ${con.to.id}`)
                const to_block = blocks.find(block => block.id === con.to.id)
                if (!to_block) throw new Error(`Block not found: ${con.to.id}`)
                con.state.powered = true
                con.state.evaluated = true
                evaluate_powered_block(to_block, false)
            })
        }
    }

    starting_blocks.forEach(block => {
        evaluate_powered_block(block, true)
    })


}





/** @type { (offset: number, bit: number) => boolean } */
const getMemoryBit = (offset, bit) => !!((plc_structure.memory[offset] >> bit) & 1)
/** @type { (offset: number, bit: number, value: boolean) => void } */
const setMemoryBit = (offset, bit, value) => {
    const byte = plc_structure.memory[offset]
    plc_structure.memory[offset] = byte & ~(1 << bit) | (+value << bit)
}

const toggle_button = (bit) => {
    let state = getMemoryBit(10, bit)
    state = !state
    setMemoryBit(10, bit, state)
}

const toggle_light = (bit) => {
    let state = getMemoryBit(20, bit)
    state = !state
    setMemoryBit(20, bit, state)
}



Object.assign(window, { plc_structure, ladder_canvas, ctx, ladder_block_width, ladder_block_height, draw_contact, getMemoryBit, setMemoryBit, toggle_button, toggle_light })


const draw = () => {
    // Canvas fill background
    ctx.fillStyle = background_color
    ctx.fillRect(0, 0, ladder_canvas.width, ladder_canvas.height)

    // Draw grid
    ctx.strokeStyle = '#337'
    ctx.lineWidth = 0.5
    ctx.setLineDash([5, 5])
    ctx.lineDashOffset = 2.5
    ctx.beginPath()
    for (let x = 1; x < 10; x++) {
        ctx.moveTo(x * ladder_block_width, 0)
        ctx.lineTo(x * ladder_block_width, ladder_canvas.height)
    }
    for (let y = 1; y < 10; y++) {
        ctx.moveTo(0, y * ladder_block_height)
        ctx.lineTo(ladder_canvas.width, y * ladder_block_height)
    }
    ctx.stroke()
    ctx.setLineDash([])

    manual_plc_cycle()

    // Draw the ladder blocks and connections
    plc_structure.structure.forEach(folder => {
        if (folder.type === 'folder') {
            folder.children.forEach(child => {
                if (child.type === 'program') {
                    child.blocks.forEach(ladder => {
                        evaluate_ladder(ladder)
                        // Draw the ladder
                        ladder.blocks.forEach(block => {
                            if (block.type === 'contact') draw_contact('highlight', ctx, block)
                            if (['coil', 'coil_set', 'coil_rset'].includes(block.type)) draw_coil('highlight', ctx, block)
                        })
                        /** @type { LadderLink[] } */
                        const links = []
                        ladder.connections.forEach(con => {
                            const from = ladder.blocks.find(block => block.id === con.from.id)
                            const to = ladder.blocks.find(block => block.id === con.to.id)
                            if (!from || !to) throw new Error(`Connection block not found`)
                            links.push({ from, to, powered: !!con.state?.powered })
                        })
                        links.forEach(link => {
                            draw_connection('highlight', ctx, link)
                        })


                        ladder.blocks.forEach(block => {
                            if (block.type === 'contact') draw_contact('symbol', ctx, block)
                            if (['coil', 'coil_set', 'coil_rset'].includes(block.type)) draw_coil('symbol', ctx, block)
                        })
                        links.forEach(link => {
                            draw_connection('symbol', ctx, link)
                        })
                    })
                }
            })
        }
    })

}


let button1_state = false
const manual_plc_cycle = () => {
    const light1 = getMemoryBit(20, 0)
    const light2 = getMemoryBit(20, 1)
    const button1 = getMemoryBit(10, 0)
    const button2 = getMemoryBit(10, 1)
    const button3 = getMemoryBit(10, 2)
    const button4 = getMemoryBit(10, 3)

    // Test
    const on = (((button1 || button2) && button3) || light1) && !button4
    setMemoryBit(20, 0, on)
    if (on) {
        setMemoryBit(20, 1, on)
    }

    // // Toggle switch
    // const button1_rising = button1 && !button1_state
    // button1_state = button1
    // if (button1_rising) {
    //     const light1_state = !light1
    //     setMemoryBit(20, 0, light1_state)
    // }


}


draw()
setInterval(draw, 20)