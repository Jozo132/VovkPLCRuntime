// @ts-check
"use strict"

/**
 * @typedef { 'locked' | 'unlocked' | 'editing' | 'live'  | 'editing_live' } PLC_ContextState
 * @typedef { 'input' | 'output' | 'memory' } PLC_Symbol_Location
 * @typedef { 'bit' | 'byte' | 'int' | 'dint' | 'real' } PLC_Symbol_Type
 * @typedef {{ name: string, location: PLC_Symbol_Location, type: PLC_Symbol_Type, address: number, initial_value: number, comment: string }} PLC_Symbol
 * @typedef { 'contact' | 'coil' | 'coil_set' | 'coil_rset' } PLC_Ladder_Block_Type
 * @typedef { 'normal' | 'rising' | 'falling' | 'change' } PLC_Trigger_Type
 * @typedef {{ id: string, x: number, y: number, type: PLC_Ladder_Block_Type, inverted: boolean, trigger: PLC_Trigger_Type, symbol: string, state?: { active: boolean, powered: boolean, terminated_input: boolean, terminated_output: boolean, evaluated: boolean, symbol: PLC_Symbol } }} PLC_LadderBlock
 * @typedef {{ id: string, from: { id: string, offset?: number }, to: { id: string, offset?: number }, state?: { powered: boolean, evaluated: boolean } }} PLC_LadderConnection 
 * @typedef {{ type: 'ladder', comment: string, blocks: PLC_LadderBlock[], connections: PLC_LadderConnection[], canvas?: HTMLCanvasElement, ctx?: CanvasRenderingContext2D, mode?: PLC_ContextState }} PLC_Ladder
 * @typedef { PLC_Ladder } PLC_ProgramBlock
 * @typedef {{ id: string, type: 'program', name: string, comment: string, blocks: PLC_ProgramBlock[], div?: HTMLDivElement }} PLC_Program
 * @typedef {{ id: string, type: 'folder', name: string, comment: string, children: (PLC_Folder | PLC_Program)[], nav?: HTMLDivElement }} PLC_Folder
 * 
 * @typedef {{ 
 *     offsets: {
 *         input: { offset: number, size: number }
 *         output: { offset: number, size: number }
 *         memory: { offset: number, size: number }
 *         system: { offset: number, size: number }
 *     }
 *     memory: number[]
 *     symbols: PLC_Symbol[]
 *     project: (PLC_Folder | PLC_Program)[]
 * }} PLC_Project
 * 
**/

class Menu {
    /**
     *  @typedef {{ type: 'item', name: string, label: string }} MenuItem 
     *  @typedef {{ type: 'separator', name?: undefined, label?: undefined }} MenuSeparator
     *  @typedef { MenuItem | MenuSeparator } MenuElement
     *  @typedef { (event: MouseEvent) => MenuElement[] | undefined } MenuOnOpen 
     *  @typedef { (selected: string) => void } MenuOnClose  
     * 
    */
    /** @type { { target: HTMLElement, onOpen: MenuOnOpen, onClose: MenuOnClose }[] } */ #listeners = []
    /** @type { MenuElement[] } */ #items = [{ type: 'item', name: 'test', label: '' }]
    /** @type { MenuOnOpen } */ #onOpen = (event) => undefined
    /** @type { MenuOnClose } */ #onClose = (selected) => undefined
    position = { x: 0, y: 0, width: 0, height: 0 }
    open = false

    #drawList() {
        this.menu.innerHTML = ''
        this.#items.forEach(item => {
            if (item.type === 'separator') {
                const hr = document.createElement('hr')
                this.menu.appendChild(hr)
            }
            if (item.type === 'item') {
                const div = document.createElement('div')
                div.classList.add('item')
                div.innerText = item.label
                div.addEventListener('click', () => {
                    this.#onClose(item.name)
                    this.menu.classList.add('hidden')
                })
                this.menu.appendChild(div)
            }
        })
    }

    #clearList() {
        this.#items = []
        this.menu.innerHTML = ''
    }

    constructor() {
        const menu = document.createElement('div')
        menu.classList.add('menu')
        menu.classList.add('items-with-context-menu')
        document.body.appendChild(menu)
        this.menu = menu
        this.#drawList()
        this.menu.classList.add('hidden')

        const handle_click_event = (event) => {
            // Get mouse X and Y from the event
            const mouseX = event.clientX
            const mouseY = event.clientY
            const lmb = event.button === 0
            const rmb = event.button === 2
            if (this.open && lmb) {
                this.menu.classList.add('hidden')
                this.open = false
            } else if (rmb) {
                // Check if the click was on one of the targets
                const listener = this.#listeners.find(listener => listener.target === event.target)
                if (listener) {
                    const items = listener.onOpen(event)
                    this.#onClose = listener.onClose
                    if (items) {
                        this.#items = items
                        this.#drawList()
                        const menu_width = menu.clientWidth
                        const menu_height = menu.clientHeight
                        const window_width = window.innerWidth
                        const window_height = window.innerHeight
                        const offset = 5
                        const x = (mouseX + menu_width + offset > window_width) ? (mouseX - menu_width - offset) : (mouseX + offset)
                        const y = (mouseY + menu_height + offset > window_height) ? (mouseY - menu_height - offset) : (mouseY + offset)
                        menu.style.left = x + 'px'
                        menu.style.top = y + 'px'
                        menu.classList.remove('hidden')
                        this.position = { x, y, width: menu_width, height: menu_height }
                        event.preventDefault()
                        this.open = true
                        return false
                    }
                } else {
                    console.log(`No listener found for target:`, event.target)
                }
            }
        }

        window.addEventListener(`contextmenu`, handle_click_event)
        window.addEventListener('click', handle_click_event)
    }

    /** @type { (listener: { target: HTMLElement, onOpen: MenuOnOpen, onClose: MenuOnClose }) => void } */
    addListener(listener) { this.#listeners.push(listener) }
    /** @type { (target: HTMLElement) => void } */
    removeListener(target) { this.#listeners = this.#listeners.filter(listener => listener.target !== target) }
    removeAllListeners() { this.#listeners = [] }
}


/** @typedef */
const default_properties = {
    ladder_block_width: 120,
    ladder_block_height: 80,
    ladder_blocks_per_row: 7,
    style: {
        background_color_alt: '#333',
        background_color: '#aaa',
        color: '#000',
        highlight_color: '#0f0',
        grid_color: '#337',
        select_color: '#0af',
        hover_color: '#0cf',
        font: '16px Arial',
    }
}

export const generateID = () => {
    const characters = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
    const letters = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
    let id = letters.charAt(Math.floor(Math.random() * letters.length))
    for (let i = 0; i < 7; i++) {
        const index = Math.floor(Math.random() * characters.length)
        id += characters[index]
    }
    return id
}



/** @type {(editor: VovkPLCEditor, symbol: string | PLC_Symbol | undefined) => number | boolean} */
const get_symbol_value = (editor, symbol) => {
    if (typeof symbol === 'string') {
        symbol = editor.project.symbols.find(s => s.name === symbol)
    }
    if (!symbol) throw new Error(`Symbol not found: ${symbol}`)
    const location = editor ? editor.project.offsets[symbol.location] : { offset: 0, size: 9999 }
    if (!location) throw new Error(`Location not found: ${symbol.location}`)
    const address = Math.floor(symbol.address)
    const bit = Math.min((symbol.address % 1) * 10, 7)
    const offset = location.offset + address
    if (address >= location.size) throw new Error(`Symbol '${symbol.name}': ${address} is out of bounds for ${symbol.location} that has a size of ${location.size}`)
    const address_value = editor ? editor.project.memory[offset] : 0
    if (symbol.type === 'bit') {
        return (address_value >> bit) & 1
    }
    return address_value
}

/** @type {(editor: VovkPLCEditor, block: PLC_LadderBlock) => PLC_LadderBlock } */
const getBlockState = (editor, block) => {
    if (!block.state) {
        const symbol = editor.project.symbols.find(symbol => symbol.name === block.symbol)
        if (!symbol) throw new Error(`Symbol not found: ${block.symbol}`)
        block.state = { active: false, powered: false, evaluated: false, symbol, terminated_input: false, terminated_output: false }
    }
    return block
}

/** @type {(editor: VovkPLCEditor, style: 'symbol' | 'highlight', ctx: CanvasRenderingContext2D , block: PLC_LadderBlock) => void} */
const draw_contact = (editor, style, ctx, block) => {
    // input state: the left side of the contact (green when true)
    // output state: the right side of the contact (green when true)
    // value: the inner state of the contact (green when true)
    const { ladder_block_width, ladder_block_height } = editor.properties
    block = getBlockState(editor, block)
    if (!block.state) throw new Error(`Block state not found: ${block.symbol}`)
    const { x, y, type, inverted, trigger, state } = block
    const symbol = state.symbol
    if (!symbol) throw new Error(`Symbol not found: ${block.symbol}`)
    let value = get_symbol_value(editor, symbol)
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


/** @type {(editor: VovkPLCEditor, style: 'symbol' | 'highlight', ctx: CanvasRenderingContext2D, block: PLC_LadderBlock) => void} */
const draw_coil = (editor, style, ctx, block) => {
    // input state: the left side of the contact (green when true)
    // output state: the right side of the contact is equal to the input state
    // value: the inner state of the contact (green when true)
    const { ladder_block_width, ladder_block_height } = editor.properties
    block = getBlockState(editor, block)
    if (!block.state) throw new Error(`Block state not found: ${block.symbol}`)
    const { x, y, type, inverted, trigger, state } = block
    const symbol = state.symbol
    if (!symbol) throw new Error(`Symbol not found: ${block.symbol}`)
    let value = get_symbol_value(editor, symbol)
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
/** @type {(editor: VovkPLCEditor, style: 'symbol' | 'highlight', ctx: CanvasRenderingContext2D , link: LadderLink) => void} */
const draw_connection = (editor, style, ctx, link) => {
    // Draw a connection line from the output of the from block to the input of the to block
    // If the target block is bellow the source block, draw the vertical line first
    // If the target block is above the source block, draw the horizontal line first
    // If the target block is to the right, just draw a straight line
    const { ladder_block_width, ladder_block_height } = editor.properties
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

/** @type {(editor: VovkPLCEditor, ladder: PLC_Ladder) => void} */
const evaluate_ladder = (editor, ladder) => {
    const { blocks, connections } = ladder
    // Reset the state of all blocks and connections
    const blockHasInputConnection = (block) => connections.some(connection => connection.to.id === block.id)
    blocks.forEach(block => {
        block = getBlockState(editor, block)
        if (!block.state) throw new Error(`Block state not found: ${block.symbol}`)
        const { state, inverted } = block
        const symbol = state.symbol
        if (!symbol) throw new Error(`Symbol not found: ${block.symbol}`)
        let active = !!get_symbol_value(editor, symbol)
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

/** @type {(editor: VovkPLCEditor, program: PLC_Program, ladder: PLC_Ladder) => void} */
const draw_ladder = (editor, program, ladder) => {
    // Generate the canvas and context if not already present
    const { ladder_block_width, ladder_block_height, ladder_blocks_per_row, style } = editor.properties
    const { background_color, grid_color } = style
    const canvas = ladder.canvas || document.createElement('canvas')
    if (!canvas) throw new Error('Canvas not found')
    if (!ladder.canvas) {
        ladder.canvas = canvas
        editor.menu.addListener({
            target: canvas,
            onOpen: (event) => {
                console.log(`Ladder context menu open`)
                return [
                    { type: 'item', name: 'edit', label: 'Edit' },
                    { type: 'item', name: 'delete', label: 'Delete' },
                    { type: 'separator' },
                    { type: 'item', name: 'copy', label: 'Copy' },
                    { type: 'item', name: 'paste', label: 'Paste' },
                ]
            },
            onClose: (selected) => {
                console.log(`Ladder selected: ${selected}`)
            }
        })
        program.div?.appendChild(canvas)
    }
    const ctx = ladder.ctx || canvas.getContext('2d')
    if (!ctx) throw new Error('Context not found')
    if (!ladder.ctx) ladder.ctx = ctx

    const max_x = ladder.blocks.reduce((max, block) => Math.max(max, block.x), 0)
    const max_y = ladder.blocks.reduce((max, block) => Math.max(max, block.y), 0)
    canvas.width = Math.max(max_x + 1, ladder_blocks_per_row) * ladder_block_width
    canvas.height = (max_y + 1) * ladder_block_height

    // Canvas fill background
    ctx.fillStyle = background_color
    ctx.fillRect(0, 0, canvas.width, canvas.height)

    // Draw grid
    ctx.strokeStyle = grid_color
    ctx.lineWidth = 0.5
    ctx.setLineDash([5, 5])
    ctx.lineDashOffset = 2.5
    ctx.beginPath()
    for (let x = 1; x < 10; x++) {
        ctx.moveTo(x * ladder_block_width, 0)
        ctx.lineTo(x * ladder_block_width, canvas.height)
    }
    for (let y = 1; y < 10; y++) {
        ctx.moveTo(0, y * ladder_block_height)
        ctx.lineTo(canvas.width, y * ladder_block_height)
    }
    ctx.stroke()
    ctx.setLineDash([])

    // manual_plc_cycle()

    // Draw the ladder blocks and connections
    evaluate_ladder(editor, ladder)
    // Draw the ladder
    ladder.blocks.forEach(block => {
        if (block.type === 'contact') draw_contact(editor, 'highlight', ctx, block)
        if (['coil', 'coil_set', 'coil_rset'].includes(block.type)) draw_coil(editor, 'highlight', ctx, block)
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
        draw_connection(editor, 'highlight', ctx, link)
    })


    ladder.blocks.forEach(block => {
        if (block.type === 'contact') draw_contact(editor, 'symbol', ctx, block)
        if (['coil', 'coil_set', 'coil_rset'].includes(block.type)) draw_coil(editor, 'symbol', ctx, block)
    })
    links.forEach(link => {
        draw_connection(editor, 'symbol', ctx, link)
    })

    // plc_project.project.forEach(folder => {
    //     if (folder.type === 'folder') {
    //         folder.children.forEach(child => {
    //             if (child.type === 'program') {
    //                 child.blocks.forEach(ladder => {
    //                 })
    //             }
    //         })
    //     }
    // })

}

/** @type { (editor: VovkPLCEditor, offset: number, bit: number) => boolean } */
const getMemoryBit = (editor, offset, bit) => !!((editor.project.memory[offset] >> bit) & 1)
/** @type { (editor: VovkPLCEditor, offset: number, bit: number, value: boolean) => void } */
const setMemoryBit = (editor, offset, bit, value) => {
    const byte = editor.project.memory[offset]
    editor.project.memory[offset] = byte & ~(1 << bit) | (+value << bit)
}



/** @type {(editor: VovkPLCEditor, program: PLC_Program, program_block: PLC_ProgramBlock) => void} */
const draw_program_block = (editor, program, program_block) => {
    if (program_block.type === 'ladder') return draw_ladder(editor, program, program_block)
    throw new Error(`Invalid program block type: ${program_block.type}`)
}



/** @type {(editor: VovkPLCEditor, program: PLC_Program) => void} */
const draw_program = (editor, program) => {
    const div = program.div || document.createElement('div')
    if (!div) throw new Error('Div not found')
    if (!program.div) {
        div.classList.add('program')
        program.div = div
        editor.container.appendChild(div)
        editor.menu.addListener({
            target: div,
            onOpen: (event) => {
                console.log(`Program context menu open`)
                return [
                    { type: 'item', name: 'edit', label: 'Edit' },
                    { type: 'item', name: 'delete', label: 'Delete' },
                    { type: 'separator' },
                    { type: 'item', name: 'copy', label: 'Copy' },
                    { type: 'item', name: 'paste', label: 'Paste' },
                ]
            },
            onClose: (selected) => {
                console.log(`Program selected: ${selected}`)
            }
        })
    }
    program.blocks.forEach(block => draw_program_block(editor, program, block))
}

/** @type { (offset: number) => { address: number, bit: number } } */
const offsetToAddressAndBit = (offset) => {
    const address = Math.floor(offset)
    const bit = Math.min((offset % 1) * 10, 7)
    return { address, bit }
}

export class VovkPLCEditor {
    /** @type { HTMLElement } */ container
    /** @type { PLC_Project } */ project

    active_tab = ''

    menu = new Menu
    properties = default_properties

    /** @param { HTMLElement | string | null } container */
    constructor(container) {
        if (typeof container === 'string') container = document.getElementById(container)
        if (!container) throw new Error('Container not found')
        this.container = container
        container.classList.add('plc-editor')
        this.menu.addListener({
            target: container,
            onOpen: (event) => {
                console.log(`PLCEditor context menu open`)
                return [
                    { type: 'item', name: 'edit', label: 'Edit' },
                    { type: 'item', name: 'delete', label: 'Delete' },
                    { type: 'separator' },
                    { type: 'item', name: 'copy', label: 'Copy' },
                    { type: 'item', name: 'paste', label: 'Paste' },
                ]
            },
            onClose: (selected) => {
                console.log(`PLCEditor selected: ${selected}`)
            }
        })

        // ladder_canvas.width = PLCEditor_element.clientWidth * 0.996
        // ladder_canvas.height = PLCEditor_element.clientHeight * 0.996

        // // On window resize
        // window.onresize = () => {
        //     ladder_canvas.width = PLCEditor_element.clientWidth * 0.996
        //     ladder_canvas.height = PLCEditor_element.clientHeight * 0.996
        // }
    }

    /** @param { PLC_Project } project */
    open(project) {
        this.project = project

    }

    /** @param { number } offset */
    getMemoryBit = (offset) => {
        const { address, bit } = offsetToAddressAndBit(offset)
        return getMemoryBit(this, address, bit)
    }
    /** @param { number } offset * @param { boolean } value */
    setMemoryBit = (offset, value) => {
        const { address, bit } = offsetToAddressAndBit(offset)
        return setMemoryBit(this, address, bit, value)
    }

    /** @param { number } offset */
    getInputBit = (offset) => {
        const { address, bit } = offsetToAddressAndBit(offset)
        const offset_address = this.project.offsets.input.offset + address
        return getMemoryBit(this, offset_address, bit)
    }
    /** @param { number } offset * @param { boolean } value */
    setInputBit = (offset, value) => {
        const { address, bit } = offsetToAddressAndBit(offset)
        const offset_address = this.project.offsets.input.offset + address
        setMemoryBit(this, offset_address, bit, value)
    }

    /** @param { number } offset */
    getOutputBit = (offset) => {
        const { address, bit } = offsetToAddressAndBit(offset)
        const offset_address = this.project.offsets.output.offset + address
        return getMemoryBit(this, offset_address, bit)
    }
    /** @param { number } offset * @param { boolean } value */
    setOutputBit = (offset, value) => {
        const { address, bit } = offsetToAddressAndBit(offset)
        const offset_address = this.project.offsets.output.offset + address
        setMemoryBit(this, offset_address, bit, value)
    }



    draw() {
        /** @type { (folder: PLC_Folder) => void } */
        const drawFolder = (folder) => {
            /** @type { (program: PLC_Program) => void } */
            folder.children.forEach(child => {
                child.id = child.id || generateID()
                if (child.type === 'folder') drawFolder(child)
                if (child.type === 'program') drawProgram(child)
            })
        }
        const drawProgram = (program) => {
            if (program.id === this.active_tab) draw_program(this, program)
        }
        this.project.project.forEach(child => {
            if (child.type === 'folder') drawFolder(child)
            if (child.type === 'program') drawProgram(child)
        })
    }
}

export default {
    VovkPLCEditor,
    generateID,
}