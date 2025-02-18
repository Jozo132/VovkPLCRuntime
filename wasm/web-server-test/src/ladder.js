// @ts-check
"use strict"

/**
 * @typedef { 'input' | 'output' | 'memory' } PLC_Symbol_Location
 * @typedef { 'bit' | 'byte' | 'int' | 'dint' | 'real' } PLC_Symbol_Type
 * @typedef {{ name: string, location: PLC_Symbol_Location, type: PLC_Symbol_Type, address: number, initial_value: number, comment: string }} PLC_Symbol
 * @typedef { 'contact' | 'coil' } PLC_Ladder_Block_Type
 * @typedef { 'normal' | 'rising' | 'falling' | 'change' } PLC_Trigger_Type
 * @typedef {{ id: number, x: number, y: number, type: PLC_Ladder_Block_Type, inverted: boolean, trigger: PLC_Trigger_Type, symbol: string, state: { active: boolean, powered: boolean, evaluated: boolean } }} PLC_LadderBlock
 * @typedef {{ id: number, from: { id: number, offset: number }, to: { id: number, offset: number }, powered: boolean, evaluated: boolean }} PLC_LadderConnection 
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
                            blocks: [
                                { id: 0, x: 0, y: 1, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button1', state: { active: false, powered: false, evaluated: false } },
                                { id: 1, x: 3, y: 1, type: 'coil', inverted: false, trigger: 'normal', symbol: 'light1', state: { active: false, powered: false, evaluated: false } },
                                { id: 2, x: 4, y: 3, type: 'coil', inverted: false, trigger: 'normal', symbol: 'light2', state: { active: false, powered: false, evaluated: false } },
                                { id: 3, x: 0, y: 3, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button2', state: { active: false, powered: false, evaluated: false } },
                                { id: 4, x: 1, y: 2, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button3', state: { active: false, powered: false, evaluated: false } },
                            ],
                            connections: [
                                { id: 0, from: { id: 0, offset: 0 }, to: { id: 4, offset: 0 }, powered: false, evaluated: false },
                                { id: 0, from: { id: 3, offset: 0 }, to: { id: 4, offset: 0 }, powered: false, evaluated: false },
                                { id: 0, from: { id: 4, offset: 0 }, to: { id: 1, offset: 0 }, powered: false, evaluated: false },
                                { id: 1, from: { id: 1, offset: 0 }, to: { id: 2, offset: 0 }, powered: false, evaluated: false },
                            ]
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

/** @type {(ctx: CanvasRenderingContext2D , block: PLC_LadderBlock) => void} */
const draw_contact = (ctx, block) => {
    // input state: the left side of the contact (green when true)
    // output state: the right side of the contact (green when true)
    // value: the inner state of the contact (green when true)
    const { x, y, inverted, trigger, state } = block
    const symbol = plc_structure.symbols.find(symbol => symbol.name === block.symbol)
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

    ctx.strokeStyle = '#0f0'
    ctx.lineWidth = 8
    ctx.beginPath()
    if (state.powered) {
        ctx.moveTo(x0, y_mid)
        ctx.lineTo(cl, y_mid)
    }
    if (state.powered && value) {
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

    ctx.strokeStyle = '#000'
    ctx.lineWidth = 2

    // Draw horizontal line from the left side of the contact
    ctx.beginPath()
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
}


/** @type {(ctx: CanvasRenderingContext2D , block: PLC_LadderBlock) => void} */
const draw_coil = (ctx, block) => {
    // input state: the left side of the contact (green when true)
    // output state: the right side of the contact is equal to the input state
    // value: the inner state of the contact (green when true)
    const { x, y, type, inverted, trigger, state } = block
    const symbol = plc_structure.symbols.find(symbol => symbol.name === block.symbol)
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
    ctx.stroke()

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
    ctx.stroke()


    const short_location = symbol.location === 'input' ? 'I' : symbol.location === 'output' ? 'Q' : 'M'

    // Draw the symbol name
    ctx.fillStyle = '#421'
    ctx.font = '16px Arial'
    ctx.textAlign = 'center'
    ctx.textBaseline = 'middle'
    ctx.fillText(symbol.name, x_mid, ct - 13)
    ctx.fillText(`${short_location}: ${symbol.address.toFixed(1)}`, x_mid, cb + 13)
}


// Draw links between blocks

/** @typedef {{ from: PLC_LadderBlock , to: PLC_LadderBlock, powered: boolean }} LadderLink */
/** @type {(ctx: CanvasRenderingContext2D , link: LadderLink) => void} */
const draw_connection = (ctx, link) => {
    // Draw a connection line from the output of the from block to the input of the to block
    // If the target block is bellow the source block, draw the vertical line first
    // If the target block is above the source block, draw the horizontal line first
    // If the target block is to the right, just draw a straight line

    const { from, to, powered } = link
    const x0 = from.x * ladder_block_width + ladder_block_width
    const y0 = from.y * ladder_block_height + ladder_block_height / 2
    const x1 = to.x * ladder_block_width
    const y1 = to.y * ladder_block_height + ladder_block_height / 2


    if (powered) {
        ctx.strokeStyle = '#0f0'
        ctx.lineWidth = 8
        ctx.beginPath()
        ctx.moveTo(x0, y0)
        if (from.y < to.y) {
            ctx.lineTo(x0, y1)
        } else if (from.y > to.y) {
            ctx.lineTo(x1, y0)
        }
        ctx.lineTo(x1, y1)
        ctx.stroke()
    }

    ctx.strokeStyle = '#000'
    ctx.lineWidth = 2
    ctx.beginPath()
    ctx.moveTo(x0, y0)
    if (from.y < to.y) {
        ctx.lineTo(x0, y1)
    } else if (from.y > to.y) {
        ctx.lineTo(x1, y0)
    }
    ctx.lineTo(x1, y1)
    ctx.stroke()
}



/** @type {(ladder: PLC_Ladder) => void} */
const evaluate_ladder = (ladder) => {
    const { blocks, connections } = ladder
    // Reset the state of all blocks and connections
    blocks.forEach(block => {
        const symbol = plc_structure.symbols.find(symbol => symbol.name === block.symbol)
        if (!symbol) throw new Error(`Symbol not found: ${block.symbol}`)
        let active = !!get_symbol_value(symbol)
        if (block.inverted) active = !active
        block.state.active = active // The actual state of the block
        block.state.powered = block.x === 0 // All blocks on x = 0 are powered
        block.state.evaluated = false
    })
    connections.forEach(connection => {
        connection.powered = false
        connection.evaluated = false
    })
    const evaluate_block = (block) => {
        if (block.state.evaluated) return
        block.state.powered = block.x === 0
        if (block.state.powered) {
            block.state.evaluated = true
        }
        connections.forEach(connection => {
            if (connection.to.id === block.id) {
                if (connection.powered) {
                    block.state.powered = true
                    block.state.evaluated = true
                }
            }
            if (connection.from.id === block.id) {
                connection.powered = block.state.powered && (block.state.active || block.type === 'coil')
                if (connection.powered) evaluate_block(blocks.find(block => block.id === connection.to.id))
            }
        })
    }
    const evaluate_connection = (connection) => {
        if (connection.evaluated) return
        const from = blocks.find(block => block.id === connection.from.id)
        const to = blocks.find(block => block.id === connection.to.id)
        if (!from || !to) throw new Error(`Connection block not found`)
        connection.powered = from.state.powered && (from.state.active || from.type === 'coil')
        if (connection.powered) {
            connection.evaluated = true
            evaluate_block(to)
        }
    }
    blocks.filter(block => block.x === 0).forEach(block => evaluate_block(block))
    connections.filter(connection => connection.from.id === 0).forEach(connection => evaluate_connection(connection))
}



const manual_plc_cycle = () => {
    const button1 = getMemoryBit(10, 0)
    const button2 = getMemoryBit(10, 1)
    const button3 = getMemoryBit(10, 2)
    const on = (button1 || button2) && button3
    setMemoryBit(20, 0, on)
    setMemoryBit(20, 1, on)
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
                            if (block.type === 'contact') draw_contact(ctx, block)
                            if (block.type === 'coil') draw_coil(ctx, block)
                        })
                        /** @type { LadderLink[] } */
                        const links = []
                        ladder.connections.forEach(connection => {
                            const from = ladder.blocks.find(block => block.id === connection.from.id)
                            const to = ladder.blocks.find(block => block.id === connection.to.id)
                            if (!from || !to) throw new Error(`Connection block not found`)
                            links.push({ from, to, powered: connection.powered })
                        })
                        links.forEach(link => {
                            draw_connection(ctx, link)
                        })
                    })
                }
            })
        }
    })

}

draw()
setInterval(draw, 20)