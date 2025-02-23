// @ts-check
"use strict"

/**
 * @typedef { 'locked' | 'unlocked' | 'editing' | 'live'  | 'editing_live' } PLC_ContextState
 * @typedef { 'control' | 'input' | 'output' | 'memory' } PLC_Symbol_Location
 * @typedef { 'bit' | 'byte' | 'int' | 'dint' | 'real' } PLC_Symbol_Type
 * @typedef {{ 
 *      name: string, 
 *      location: PLC_Symbol_Location, 
 *      type: PLC_Symbol_Type, 
 *      address: number, 
 *      initial_value: number, 
 *      comment: string }} PLC_Symbol
 * @typedef { 'contact' | 'coil' | 'coil_set' | 'coil_rset' } PLC_Ladder_Block_Type
 * @typedef { 'normal' | 'rising' | 'falling' | 'change' } PLC_Trigger_Type
 * @typedef {{ 
 *      id: string, 
 *      x: number, 
 *      y: number, 
 *      type: PLC_Ladder_Block_Type, 
 *      inverted: boolean, 
 *      trigger: PLC_Trigger_Type, 
 *      symbol: string, 
 *      state?: { active: boolean, powered: boolean, terminated_input: boolean, terminated_output: boolean, evaluated: boolean, symbol: PLC_Symbol } 
 * }} PLC_LadderBlock
 * @typedef {{ 
 *      id?: string, 
 *      from: { id: string, offset?: number }, 
 *      to: { id: string, offset?: number }, 
 *      state?: { powered: boolean, evaluated: boolean } 
 * }} PLC_LadderConnection 
 * @typedef {{ 
 *      id?: string, 
 *      type: 'ladder', 
 *      name: string, 
 *      comment: string, 
 *      blocks: PLC_LadderBlock[], 
 *      connections: PLC_LadderConnection[], 
 *      div?: Element, 
 *      ctx?: CanvasRenderingContext2D, 
 *      mode?: PLC_ContextState
 * }} PLC_Ladder
 * @typedef { PLC_Ladder } PLC_ProgramBlock
 * @typedef {{ id?: string, type: 'program', name: string, comment: string, blocks: PLC_ProgramBlock[], div?: Element }} PLC_Program
 * @typedef {{ id?: string, type: 'folder', name: string, comment: string, children: (PLC_Folder | PLC_Program)[], nav?: Element }} PLC_Folder
 * 
 * @typedef {{ 
 *     offsets: {
 *         control: { offset: number, size: number }
 *         input: { offset: number, size: number }
 *         output: { offset: number, size: number }
 *         memory: { offset: number, size: number }
 *         system: { offset: number, size: number }
 *     }
 *     symbols: PLC_Symbol[]
 *     project: (PLC_Folder | PLC_Program)[]
 * }} PLC_Project
 * 
 * @typedef {{
 *     id: number
 *     where: 'project' | 'symbol' | 'program' | 'program_block' 
 *     what: 'insert' | 'delete' | 'paste' | 'cut' | 'move' | 'modify'
 *     target: string
 *     index: number
 *     before: any // What to do to undo this action
 *     after: any // What to do to redo this action
 * }} TimelineEntry
 * 
**/


// Import style from path './VovkPLCEditor.css'
const style = document.createElement('link')
style.rel = 'stylesheet'
style.href = './VovkPLCEditor.css'
const styleLoadPromise = new Promise((resolve, reject) => {
    style.onload = () => resolve(1);
    style.onerror = () => reject(new Error('Failed to load stylesheet'));
});
document.head.appendChild(style)
await styleLoadPromise


/** @typedef */
const memory_locations = [
    { short: 'C', name: 'control', label: 'Control' },
    { short: 'I', name: 'input', label: 'Input' },
    { short: 'Q', name: 'output', label: 'Output' },
    { short: 'M', name: 'memory', label: 'Memory' },
]

/** @type { PLC_Symbol[] } */
const system_symbols = [
    { name: 'P_100ms', location: 'control', type: 'bit', address: 2.0, initial_value: 0, comment: '100ms pulse' },
    { name: 'P_200ms', location: 'control', type: 'bit', address: 2.1, initial_value: 0, comment: '200ms pulse' },
    { name: 'P_300ms', location: 'control', type: 'bit', address: 2.2, initial_value: 0, comment: '300ms pulse' },
    { name: 'P_500ms', location: 'control', type: 'bit', address: 2.3, initial_value: 0, comment: '500ms pulse' },
    { name: 'P_1s', location: 'control', type: 'bit', address: 2.4, initial_value: 0, comment: '1 second pulse' },
    { name: 'P_2s', location: 'control', type: 'bit', address: 2.5, initial_value: 0, comment: '2 second pulse' },
    { name: 'P_5s', location: 'control', type: 'bit', address: 2.6, initial_value: 0, comment: '5 second pulse' },
    { name: 'P_10s', location: 'control', type: 'bit', address: 2.7, initial_value: 0, comment: '10 second pulse' },
    { name: 'P_30s', location: 'control', type: 'bit', address: 3.0, initial_value: 0, comment: '30 second pulse' },
    { name: 'P_1min', location: 'control', type: 'bit', address: 3.1, initial_value: 0, comment: '1 minute pulse' },
    { name: 'P_2min', location: 'control', type: 'bit', address: 3.2, initial_value: 0, comment: '2 minute pulse' },
    { name: 'P_5min', location: 'control', type: 'bit', address: 3.3, initial_value: 0, comment: '5 minute pulse' },
    { name: 'P_10min', location: 'control', type: 'bit', address: 3.4, initial_value: 0, comment: '10 minute pulse' },
    { name: 'P_15min', location: 'control', type: 'bit', address: 3.5, initial_value: 0, comment: '15 minute pulse' },
    { name: 'P_30min', location: 'control', type: 'bit', address: 3.6, initial_value: 0, comment: '30 minute pulse' },
    { name: 'P_1hr', location: 'control', type: 'bit', address: 3.7, initial_value: 0, comment: '1 hour pulse' },
    { name: 'P_2hr', location: 'control', type: 'bit', address: 4.0, initial_value: 0, comment: '2 hour pulse' },
    { name: 'P_3hr', location: 'control', type: 'bit', address: 4.1, initial_value: 0, comment: '3 hour pulse' },
    { name: 'P_4hr', location: 'control', type: 'bit', address: 4.2, initial_value: 0, comment: '4 hour pulse' },
    { name: 'P_5hr', location: 'control', type: 'bit', address: 4.3, initial_value: 0, comment: '5 hour pulse' },
    { name: 'P_6hr', location: 'control', type: 'bit', address: 4.4, initial_value: 0, comment: '6 hour pulse' },
    { name: 'P_12hr', location: 'control', type: 'bit', address: 4.5, initial_value: 0, comment: '12 hour pulse' },
    { name: 'P_1day', location: 'control', type: 'bit', address: 4.6, initial_value: 0, comment: '1 day pulse' },

    { name: 'S_100ms', location: 'control', type: 'bit', address: 5.0, initial_value: 0, comment: '100ms square wave' },
    { name: 'S_200ms', location: 'control', type: 'bit', address: 5.1, initial_value: 0, comment: '200ms square wave' },
    { name: 'S_300ms', location: 'control', type: 'bit', address: 5.2, initial_value: 0, comment: '300ms square wave' },
    { name: 'S_500ms', location: 'control', type: 'bit', address: 5.3, initial_value: 0, comment: '500ms square wave' },
    { name: 'S_1s', location: 'control', type: 'bit', address: 5.4, initial_value: 0, comment: '1 second square wave' },
    { name: 'S_2s', location: 'control', type: 'bit', address: 5.5, initial_value: 0, comment: '2 second square wave' },
    { name: 'S_5s', location: 'control', type: 'bit', address: 5.6, initial_value: 0, comment: '5 second square wave' },
    { name: 'S_10s', location: 'control', type: 'bit', address: 5.7, initial_value: 0, comment: '10 second square wave' },
    { name: 'S_30s', location: 'control', type: 'bit', address: 6.0, initial_value: 0, comment: '30 second square wave' },
    { name: 'S_1min', location: 'control', type: 'bit', address: 6.1, initial_value: 0, comment: '1 minute square wave' },
    { name: 'S_2min', location: 'control', type: 'bit', address: 6.2, initial_value: 0, comment: '2 minute square wave' },
    { name: 'S_5min', location: 'control', type: 'bit', address: 6.3, initial_value: 0, comment: '5 minute square wave' },
    { name: 'S_10min', location: 'control', type: 'bit', address: 6.4, initial_value: 0, comment: '10 minute square wave' },
    { name: 'S_15min', location: 'control', type: 'bit', address: 6.5, initial_value: 0, comment: '15 minute square wave' },
    { name: 'S_30min', location: 'control', type: 'bit', address: 6.6, initial_value: 0, comment: '30 minute square wave' },
    { name: 'S_1hr', location: 'control', type: 'bit', address: 6.7, initial_value: 0, comment: '1 hour square wave' },

    { name: 'elapsed_days', location: 'control', type: 'byte', address: 8.0, initial_value: 0, comment: 'Elapsed days' },
    { name: 'elapsed_hours', location: 'control', type: 'byte', address: 9.0, initial_value: 0, comment: 'Elapsed hours' },
    { name: 'elapsed_minutes', location: 'control', type: 'byte', address: 10.0, initial_value: 0, comment: 'Elapsed minutes' },
    { name: 'elapsed_seconds', location: 'control', type: 'byte', address: 11.0, initial_value: 0, comment: 'Elapsed seconds' },

    { name: 'system_uptime', location: 'control', type: 'dint', address: 12.0, initial_value: 0, comment: 'System uptime in seconds' },
]



/** @type { (html_code: string) => Element[] }  */
export const ElementSynthesis = (html_code) => {
    if (typeof html_code !== 'string') throw new Error(`Invalid HTML code: ${html_code}`)
    html_code = html_code.split('\n').map(line => line.trim()).filter(Boolean).join('')
    if (!html_code) return []
    const parser = new DOMParser()
    const doc = parser.parseFromString(html_code, 'text/html')
    return Array.from(doc.body.children)
}


class Menu {
    /**
     *  @typedef {{ type: 'item', name: string, label: string }} MenuItem 
     *  @typedef {{ type: 'separator', name?: undefined, label?: undefined }} MenuSeparator
     *  @typedef { MenuItem | MenuSeparator } MenuElement
     *  @typedef { (event: MouseEvent) => MenuElement[] | undefined } MenuOnOpen 
     *  @typedef { (selected: string) => void } MenuOnClose  
     *  @typedef { { target: HTMLElement | Element, onOpen: MenuOnOpen, onClose: MenuOnClose } } MenuListener  
     * 
    */
    /** @type { MenuListener[] } */ #listeners = []
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

        const findListener = (event) => {
            let target = event.target
            let found = false
            let listener = this.#listeners.find(listener => listener.target === target)
            found = !!listener

            while (target && !found) {
                target = target.parentElement
                listener = this.#listeners.find(listener => listener.target === target)
                found = !!listener
            }
            return found ? listener : null
        }

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
                const listener = findListener(event)
                if (listener) {
                    // console.log(`Listeners:`, this.#listeners)
                    // console.log(`Found listener for target:`, listener)
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

    /** @type { (listeners: MenuListener | MenuListener[]) => void } */
    addListener(listeners) {
        if (Array.isArray(listeners)) this.#listeners.push(...listeners)
        else this.#listeners.push(listeners)
    }
    /** @type { (target: HTMLElement | Element) => void } */
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
        background_color: '#444',
        color: '#000',
        highlight_color: '#4D4',
        grid_color: '#FFF4',
        select_color: '#0af',
        hover_color: '#0cf',
        font: '16px Arial',
        font_color: '#DDD',
        line_width: 3,
        highlight_width: 8,
    }
    // style: {
    //     background_color_alt: '#333',
    //     background_color: '#aaa',
    //     color: '#000',
    //     highlight_color: '#0f0',
    //     grid_color: '#337',
    //     select_color: '#0af',
    //     hover_color: '#0cf',
    //     font: '16px Arial',
    //     font_color: '#421',
    // }
}


const middle_mouse_drag = () => {
    let isDragging = false;
    let startX, startY, scrollLeft, scrollTop;
    let target = null;
    let prev_pointer = ''
    document.addEventListener("mousedown", function (e) {
        if (e.button !== 1) return; // Only trigger on middle mouse button
        // @ts-ignore
        target = e.target.closest(".plc-editor-body");
        if (!target) return

        e.preventDefault();
        isDragging = true;
        startX = e.pageX;
        startY = e.pageY;
        scrollLeft = target.scrollLeft;
        scrollTop = target.scrollTop;

        prev_pointer = target.style.cursor || ''
        target.style.cursor = "grabbing";
    });

    document.addEventListener("mousemove", function (e) {
        if (!isDragging) return;
        if (!target) return;
        e.preventDefault();
        const diff_x = e.pageX - startX;
        const diff_y = e.pageY - startY;
        // console.log('panning', { diff_x, diff_y })
        target.scrollLeft = scrollLeft - diff_x;
        target.scrollTop = scrollTop - diff_y;
    });

    document.addEventListener("mouseup", function (e) {
        if (!target) return;
        if (e.button !== 1) return;
        isDragging = false;
        target.style.cursor = prev_pointer
        target = null;
    });
}
// Enable middle-mouse drag support
middle_mouse_drag()


// Resizable borders
const handle_resizer_drag = (event) => { // @ts-ignore
    if (!event || !event.target || !event.target.classList.contains("resizer")) return;

    const resizer = event.target; // @ts-ignore
    // const target = resizer.parentElement;
    // Find the closest parent with the class 'resizable'
    const target = resizer.closest('.resizable')
    if (!target) return console.error("No resizable parent found for resizer: ", resizer)
    event.preventDefault();
    const startX = event.clientX || event.touches[0].clientX;
    const startY = event.clientY || event.touches[0].clientY;
    const startWidth = target.offsetWidth;
    const startHeight = target.offsetHeight;

    const previous_pointer = target.style.cursor || '' // @ts-ignore
    target.style.cursor = resizer.classList.contains("right") || resizer.classList.contains("left") ? "col-resize" : "row-resize";

    const constrain = value => {
        return Math.min(Math.max(value, 200), 1000)
    }

    function onMouseMove(event) {
        const endX = event.clientX || event.touches[0].clientX
        const endY = event.clientY || event.touches[0].clientY// @ts-ignore
        if (resizer.classList.contains("right")) {
            const newWidth = constrain(startWidth + (endX - startX));
            target.style.width = newWidth + "px"; // @ts-ignore
        } else if (resizer.classList.contains("left")) {
            const newWidth = constrain(startWidth - (endX - startX));
            target.style.width = newWidth + "px"; // @ts-ignore
        } else if (resizer.classList.contains("bottom")) {
            const newHeight = constrain(startHeight + (endY - startY));
            target.style.height = newHeight + "px"; // @ts-ignore
        } else if (resizer.classList.contains("top")) {
            const newHeight = constrain(startHeight - (endY - startY));
            target.style.height = newHeight + "px";
        } else { // @ts-ignore
            console.error("Invalid resizer class: ", resizer.classList)
        }
    }

    function onMouseUp() {
        target.style.cursor = previous_pointer
        document.removeEventListener("mousemove", onMouseMove);
        document.removeEventListener("mouseup", onMouseUp);
        // Handle touch
        document.removeEventListener("touchmove", onMouseMove);
        document.removeEventListener("touchend", onMouseUp);
        document.removeEventListener("touchcancel", onMouseUp);
    }

    document.addEventListener("mousemove", onMouseMove);
    document.addEventListener("mouseup", onMouseUp);
    // Handle touch
    document.addEventListener("touchmove", onMouseMove, { passive: false });
    document.addEventListener("touchend", onMouseUp);
    document.addEventListener("touchcancel", onMouseUp);
}

document.addEventListener("mousedown", handle_resizer_drag);
document.addEventListener("touchstart", handle_resizer_drag, { passive: false });

/** @type {(editor: VovkPLCEditor, symbol: string | PLC_Symbol | undefined) => number | boolean} */
const get_symbol_value = (editor, symbol) => {
    if (typeof symbol === 'string') {
        symbol = system_symbols.find(s => s.name === symbol) || editor.project.symbols.find(s => s.name === symbol)
    }
    if (!symbol) throw new Error(`Symbol not found: ${symbol}`)
    const location = editor ? editor.project.offsets[symbol.location] : { offset: 0, size: 9999 }
    if (!location) throw new Error(`Location not found: ${symbol.location}`)
    const address = Math.floor(symbol.address)
    const bit = Math.min((symbol.address % 1) * 10, 7)
    const offset = location.offset + address
    if (address >= location.size) throw new Error(`Symbol '${symbol.name}': ${address} is out of bounds for ${symbol.location} that has a size of ${location.size}`)
    const address_value = editor ? editor.memory[offset] : 0
    if (symbol.type === 'bit') {
        return (address_value >> bit) & 1
    }
    return address_value
}

/** @type {(editor: VovkPLCEditor, block: PLC_LadderBlock) => PLC_LadderBlock } */
const getBlockState = (editor, block) => {
    if (!block.state) {
        const symbol = system_symbols.find(symbol => symbol.name === block.symbol) || editor.project.symbols.find(symbol => symbol.name === block.symbol)
        if (!symbol) throw new Error(`Symbol not found: ${block.symbol}`)
        block.state = { active: false, powered: false, evaluated: false, symbol, terminated_input: false, terminated_output: false }
    }
    return block
}

/** @type {(editor: VovkPLCEditor, like: 'symbol' | 'highlight', ctx: CanvasRenderingContext2D , block: PLC_LadderBlock) => void} */
const draw_contact = (editor, like, ctx, block) => {
    // input state: the left side of the contact (green when true)
    // output state: the right side of the contact (green when true)
    // value: the inner state of the contact (green when true)
    const { ladder_block_width, ladder_block_height, style } = editor.properties
    const { line_width, highlight_width, color, highlight_color, font, font_color } = style
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
    if (like === 'highlight') {
        ctx.strokeStyle = highlight_color
        ctx.lineWidth = highlight_width
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

    if (like === 'symbol') {
        ctx.strokeStyle = color
        ctx.lineWidth = line_width

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

        const short_location = memory_locations.find(loc => loc.name === symbol.location)?.short || '?'

        // Draw the symbol name
        ctx.fillStyle = font_color
        ctx.font = font
        ctx.textAlign = 'center'
        ctx.textBaseline = 'middle'
        ctx.fillText(symbol.name, x_mid, ct - 13)
        ctx.fillText(`${short_location}: ${symbol.address.toFixed(1)}`, x_mid, cb + 13)
        return
    }

    throw new Error(`Invalid style: ${style}`)
}


/** @type {(editor: VovkPLCEditor, like: 'symbol' | 'highlight', ctx: CanvasRenderingContext2D, block: PLC_LadderBlock) => void} */
const draw_coil = (editor, like, ctx, block) => {
    // input state: the left side of the contact (green when true)
    // output state: the right side of the contact is equal to the input state
    // value: the inner state of the contact (green when true)
    const { ladder_block_width, ladder_block_height, style } = editor.properties
    const { line_width, highlight_width, color, highlight_color, font, font_color } = style
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
    if (like === 'highlight') {
        ctx.strokeStyle = highlight_color
        ctx.lineWidth = highlight_width
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

    if (like === 'symbol') {
        ctx.strokeStyle = color
        ctx.lineWidth = line_width

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


        const short_location = memory_locations.find(loc => loc.name === symbol.location)?.short || '?'

        // Draw the symbol name
        ctx.fillStyle = font_color
        ctx.font = font
        ctx.textAlign = 'center'
        ctx.textBaseline = 'middle'
        ctx.fillText(symbol.name, x_mid, ct - 13)
        ctx.fillText(`${short_location}: ${symbol.address.toFixed(1)}`, x_mid, cb + 13)

        ctx.fillStyle = color
        ctx.font = '18px Arial Black'
        if (type === 'coil_set') ctx.fillText('S', x_mid, y_mid + 1)
        if (type === 'coil_rset') ctx.fillText('R', x_mid, y_mid + 1)
        return
    }

    throw new Error(`Invalid style: ${style}`)
}


// Draw links between blocks

/** @typedef {{ from: PLC_LadderBlock , to: PLC_LadderBlock, powered: boolean }} LadderLink */
/** @type {(editor: VovkPLCEditor, like: 'symbol' | 'highlight', ctx: CanvasRenderingContext2D , link: LadderLink) => void} */
const draw_connection = (editor, like, ctx, link) => {
    // Draw a connection line from the output of the from block to the input of the to block
    // If the target block is bellow the source block, draw the vertical line first
    // If the target block is above the source block, draw the horizontal line first
    // If the target block is to the right, just draw a straight line
    const { ladder_block_width, ladder_block_height, style } = editor.properties
    const { line_width, highlight_width, highlight_color, color } = style
    const { from, to, powered } = link
    const x0 = from.x * ladder_block_width + ladder_block_width
    const y0 = from.y * ladder_block_height + ladder_block_height / 2
    const x1 = to.x * ladder_block_width
    const y1 = to.y * ladder_block_height + ladder_block_height / 2

    const x_direction = x0 < x1 ? 1 : x0 > x1 ? -1 : 0
    const y_direction = y0 < y1 ? 1 : y0 > y1 ? -1 : 0
    if (x_direction === 0 && y_direction === 0) return // elements are touching, no need to draw a connection
    if (like === 'highlight') {
        if (powered) {
            ctx.strokeStyle = highlight_color
            ctx.lineWidth = highlight_width
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

    if (like === 'symbol') {
        ctx.strokeStyle = color
        ctx.lineWidth = line_width
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
    ladder.id = editor.generateID(ladder.id)
    const { id, name, comment } = ladder
    const scale = 1.5
    // Generate the canvas and context if not already present
    let ladder_block_height = editor.properties.ladder_block_height
    let ladder_block_width = editor.properties.ladder_block_width
    const { ladder_blocks_per_row, style } = editor.properties
    const { background_color, grid_color, color } = style
    const div_exists = !!ladder.div
    const div = ladder.div || ElementSynthesis(`<div class="plc-program-block"></div>`)[0]
    if (!div) throw new Error('Div not found')
    const initialize = div && !div_exists
    if (initialize) {
        const children = Array.from(div.children)
        for (const child of children) div.removeChild(child)
        const [header, canvas] = ElementSynthesis(`
            <div class="plc-program-block-header"></div>
            <div class="plc-program-block-container">
                <canvas class="plc-program-block" id="${id}"></canvas>
            </div>
        `)
        div.appendChild(header)
        div.appendChild(canvas)
        ladder.div = div
    }
    /** @type { [HTMLElement, HTMLCanvasElement] } *///@ts-ignore
    const [header, container] = Array.from(ladder.div?.children || [])

    if (!header) throw new Error('Header not found')
    if (!container) throw new Error('Container not found')
    const canvas = container.querySelector('canvas')
    if (!canvas) throw new Error('Canvas not found')

    if (initialize) {
        if (!ladder.div) throw new Error('Div not found')
        canvas.setAttribute('id', id)
        const header_children = ElementSynthesis(`
            <div class="plc-program-block-header-content">
                <div class="plc-program-block-header-title">
                    <h3 style="margin-top: 3px; margin-bottom: 1px">Ladder: ${name}</h3>
                    <p class="plc-comment-simple" style="text-overflow: ellipsis;">${comment}</p>
                </div>
                <div class="plc-program-block-header-buttons">
                    <!--div class="menu-button delete">x</div-->
                    <!--div class="menu-button edit">/</div-->
                    <div class="menu-button minimize">-</div>
                </div>
            </div>
            <p class="plc-comment-detailed">${comment}</p>
        `)
        for (const child of header_children) header.appendChild(child)
        const minimize_button = header.querySelector('.minimize')
        if (!minimize_button) throw new Error('Minimize button not found')
        minimize_button.addEventListener('click', () => {
            // div.classList.toggle('minimized')
            const is_minimized = div.classList.contains('minimized')
            div.classList.toggle('minimized') // @ts-ignore
            minimize_button.innerText = is_minimized ? '-' : '+'
        })
        editor.workspace_context_menu.addListener({
            target: canvas,
            onOpen: (event) => {
                console.log(`Ladder canvas "#${id}" context menu open`)
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
        program.div?.appendChild(ladder.div)
    }
    const ctx = ladder.ctx || canvas.getContext('2d')
    if (!ctx) throw new Error('Context not found')
    if (!ladder.ctx) ladder.ctx = ctx

    const max_x = ladder.blocks.reduce((max, block) => Math.max(max, block.x), 0)
    const max_y = ladder.blocks.reduce((max, block) => Math.max(max, block.y), 0)
    const width = Math.max(max_x + 1, ladder_blocks_per_row) * ladder_block_width
    const height = (max_y + 1) * ladder_block_height
    canvas.style.width = (width / scale) + 'px';
    canvas.style.height = (height / scale) + 'px';
    canvas.width = width
    canvas.height = height

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
        con.id = editor.generateID(con.id)
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
}

/** @type { (editor: VovkPLCEditor, offset: number, bit: number) => boolean } */
const getMemoryBit = (editor, offset, bit) => !!((editor.memory[offset] >> bit) & 1)
/** @type { (editor: VovkPLCEditor, offset: number, bit: number, value: boolean) => void } */
const setMemoryBit = (editor, offset, bit, value) => {
    if (editor.runtime_ready) {
        let byte = editor.memory[offset]
        byte = byte & ~(1 << bit) | (+value << bit)
        editor.runtime.writeMemoryArea(offset, [byte])
    } else {
        const byte = editor.memory[offset]
        editor.memory[offset] = byte & ~(1 << bit) | (+value << bit)
    }
}



/** @type {(editor: VovkPLCEditor, program: PLC_Program, program_block: PLC_ProgramBlock) => void} */
const draw_program_block = (editor, program, program_block) => {
    if (program_block.type === 'ladder') return draw_ladder(editor, program, program_block)
    throw new Error(`Invalid program block type: ${program_block.type}`)
}



/** @type {(editor: VovkPLCEditor, body: Element, program: PLC_Program) => void} */
const draw_program = (editor, body, program) => {
    program.id = editor.generateID(program.id)
    const { id } = program
    const exists = !!program.div
    program.div = program.div || body
    if (!exists) {
        program.div.setAttribute('id', id)
        program.div = program.div
        editor.workspace_context_menu.addListener({
            target: program.div,
            onOpen: (event) => {
                console.log(`Program "#${id}" context menu open`)
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
    program.blocks.forEach(block => {
        draw_program_block(editor, program, block)
    })
}

/** @type { (offset: number) => { address: number, bit: number } } */
const offsetToAddressAndBit = (offset) => {
    const address = Math.floor(offset)
    const bit = Math.min((offset % 1) * 10, 7)
    return { address, bit }
}

/** @type { (list: string[], id: string) => void */
const addIdToList = (list, id) => {
    if (!list.includes(id)) list.push(id)
}



const generateID = () => {
    const characters = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789'
    const letters = 'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'
    let id = letters.charAt(Math.floor(Math.random() * letters.length))
    for (let i = 0; i < 7; i++) {
        const index = Math.floor(Math.random() * characters.length)
        id += characters[index]
    }
    return id
}


/** @type { (editor: VovkPLCEditor, folder: PLC_Folder) => PLC_Program | null } */
const searchForProgramInFolder = (editor, folder) => { // @ts-ignore
    return folder.children.find(child => {
        if (child.type === 'folder') return searchForProgramInFolder(editor, child)
        if (child.type === 'program') return searchForProgram(editor, child) // @ts-ignore
        throw new Error(`Invalid child type: ${child.type}`)
    }) || null
}
/** @type { (editor: VovkPLCEditor, program: PLC_Program) => PLC_Program | null } */
const searchForProgram = (editor, program) => {
    if (!program.id) throw new Error('Program ID not found')
    if (!editor.active_tab) editor.active_tab = program.id // TODO: remove this and actually open the selected program from navigation
    if (program.id === editor.active_tab) return program
    return null
}


import PLCRuntimeWasm from "../../simulator.js"

/** @type { (editor: VovkPLCEditor, id: string) => PLC_Program | null } */
const findProgram = (editor, id) => {
    if (!editor) throw new Error('Editor not found')
    if (!editor.project) return null
    if (!editor.project.project) return null
    const project = editor.project.project
    for (let i = 0; i < project.length; i++) {
        const folder = project[i]
        let program
        if (folder.type === 'folder') program = searchForProgramInFolder(editor, folder)
        if (folder.type === 'program') program = searchForProgram(editor, folder)
        if (id && program && program.id === id) return program
        if (program) return program
    }
    return null
}

class PLCEditor {
    id
    name = ''
    comment = ''
    div
    header
    body
    /** @type { HTMLCanvasElement } */ canvas
    editor
    /** @type { PLC_Program | null } */ program = null
    /** @param { VovkPLCEditor } editor */
    constructor(editor) {
        if (!editor) throw new Error('Editor not found')
        this.editor = editor
        const div = editor.workspace.querySelector('.plc-editor')
        if (!div) throw new Error('Editor not found')
        this.div = div
        this.id = editor.generateID(div.getAttribute('id'))
        div.setAttribute('id', this.id)
        const content = ElementSynthesis(/*HTML*/`
            <div class="plc-editor-top">
                <div class="plc-editor-header"></div>
            </div>
            <div class="plc-editor-body"></div>
        `)
        const header = content[0].querySelector('.plc-editor-header')
        const body = content[1]
        if (!header) throw new Error('Header not found')
        if (!body) throw new Error('Body not found')
        this.header = header
        this.body = body
        content.forEach(c => div.appendChild(c))
        this.editor.workspace_context_menu.addListener({
            target: this.div,
            onOpen: (event) => {
                console.log(`VovkPLC Editor "#${this.id}" context menu open`)
                return [
                    { type: 'item', name: 'edit', label: 'Edit' },
                    { type: 'item', name: 'delete', label: 'Delete' },
                    { type: 'separator' },
                    { type: 'item', name: 'copy', label: 'Copy' },
                    { type: 'item', name: 'paste', label: 'Paste' },
                ]
            },
            onClose: (selected) => {
                console.log(`Editor selected: ${selected}`)
            }
        })
        this.reloadProgram()
    }

    appendChild(child) {
        this.body.appendChild(child)
    }

    reloadProgram() {
        this.program = findProgram(this.editor, this.id)
    }

    draw() {
        this.program = this.program || findProgram(this.editor, this.id)
        if (!this.program) throw new Error('Program not found')

        const { id, name, comment } = this.program
        if (this.name !== name || this.comment !== comment) {
            this.name = name
            this.comment = comment
            this.header.innerHTML = `
                <h2 style="margin-top: 0px; margin-bottom: 3px;">Program: ${name || ''}</h2>
                <p>${comment || ''}</p>
            `
        }
        draw_program(this.editor, this.body, this.program)
    }
}


export class VovkPLCEditor {
    /** @type { PLC_Project } */ project
    /** @type { HTMLElement } */ workspace


    workspace_context_menu = new Menu

    /** @type { number[] } */ memory = new Array(100).fill(0)
    runtime = new PLCRuntimeWasm()
    runtime_ready = false

    active_tab = ''

    /** @type { TimelineEntry[] } */ timeline = []
    /** @type { String[] } */ reserved_ids = []

    properties = default_properties

    /** 
     * @param {{ 
     *      workspace?: HTMLElement | string | null
     *      debug_css?: boolean
     * }} options 
    */
    constructor({ workspace, debug_css }) {
        this.runtime.initialize('/simulator.wasm').then(() => {
            console.log('PLC Runtime initialized')
            this.runtime_ready = true
        })
        workspace = workspace || this.workspace
        if (typeof workspace === 'string') workspace = document.getElementById(workspace)
        if (workspace !== null) {
            const container_id = workspace.getAttribute('id')
            if (container_id) addIdToList(this.reserved_ids, container_id)
        }
        if (!workspace) throw new Error('Container not found')
        const id = workspace.getAttribute('id')
        if (!id) throw new Error('Container ID not found')
        this.workspace = workspace
        this.workspace.classList.add('plc-workspace')
        if (debug_css) this.workspace.classList.add('debug')

        /** @type { Element[] } */
        const workspace_body = ElementSynthesis(/*HTML*/`
            <!--div class="plc-workspace-header">
                <h2>Header</h2>
            </div-->
            <div class="plc-workspace-body">
                <div class="plc-navigation no-select resizable" style="width: 200px">
                    <div class="plc-navigation-container">
                        <h3>Navigation</h3>
                    </div>
                    <div class="resizer right"></div>
                    <div class="plc-navigation-bar">
                        <div class="menu-button">-</div>
                        <span class="thick text-rotate" style="margin: auto auto; margin-top: 5px; font-size: 0.6em;">Navigation</span>
                    </div>
                </div>
                <div class="plc-window">
                    <div class="plc-window-bar">
                        <div class="plc-tab active">Main</div>
                        <div class="plc-tab">Test</div>
                    </div>
                    <div class="plc-window-frame">
                        <div class="plc-editor"></div>
                    </div>
                </div>
                <div class="plc-tools no-select resizable minimized" style="width: 200px">
                    <div class="plc-tools-bar">
                        <div class="menu-button">+</div>
                        <span class="thick text-rotate" style="margin: auto auto; margin-top: 5px; font-size: 0.6em;">Tools</span>
                    </div>
                    <div class="resizer left"></div>
                    <div class="plc-tools-container">
                        <h3>Tools</h3>
                    </div>
                </div>
            </div>
            <!--div class="plc-workspace-footer">
                <h3>Footer</h3>
            </div-->
        `)
        workspace_body.forEach(element => workspace.appendChild(element))
        const [header, body, footer] = workspace_body

        const navigation_minimize_button = workspace.querySelector('.plc-navigation-bar .menu-button')
        const tools_minimize_button = workspace.querySelector('.plc-tools-bar .menu-button')
        if (!navigation_minimize_button) throw new Error('Navigation minimize button not found')
        if (!tools_minimize_button) throw new Error('Tools minimize button not found')
        navigation_minimize_button.addEventListener('click', () => {
            const navigation = workspace.querySelector('.plc-navigation')
            if (!navigation) throw new Error('Navigation not found')
            const [container] = Array.from(navigation.children)
            if (!container) throw new Error('Navigation container not found')
            const bar = navigation.querySelector('.plc-navigation-bar .menu-button')
            if (!bar) throw new Error('Navigation bar not found')
            // If the navigation doesn't have class 'minimized', we add it to the navigation and change the button text to '+'
            // Otherwise we remove the class and change the button text to '-'
            const is_minimized = navigation.classList.contains('minimized')
            if (is_minimized) {
                navigation.classList.remove('minimized')
                bar.innerHTML = '-'
            } else {
                navigation.classList.add('minimized')
                bar.innerHTML = '+'
            }
        })
        tools_minimize_button.addEventListener('click', () => {
            const tools = workspace.querySelector('.plc-tools')
            if (!tools) throw new Error('Tools not found')
            const [container] = Array.from(tools.children)
            if (!container) throw new Error('Tools container not found')
            const bar = tools.querySelector('.plc-tools-bar .menu-button')
            if (!bar) throw new Error('Tools bar not found')
            // If the tools doesn't have class 'minimized', we add it to the tools and change the button text to '+'
            // Otherwise we remove the class and change the button text to '-'
            const is_minimized = tools.classList.contains('minimized')
            if (is_minimized) {
                tools.classList.remove('minimized')
                bar.innerHTML = '-'
            } else {
                tools.classList.add('minimized')
                bar.innerHTML = '+'
            }
        })

        this.editor = new PLCEditor(this)
        this.workspace_context_menu.addListener({
            target: workspace,
            onOpen: (event) => {
                console.log(`VovkPLC Workspace "#${id}" context menu open`)
                return [
                    { type: 'item', name: 'edit', label: 'Edit' },
                    { type: 'item', name: 'delete', label: 'Delete' },
                    { type: 'separator' },
                    { type: 'item', name: 'copy', label: 'Copy' },
                    { type: 'item', name: 'paste', label: 'Paste' },
                ]
            },
            onClose: (selected) => {
                console.log(`Workspace selected: ${selected}`)
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

        /** @type { (folder: PLC_Folder) => void } */
        const reserveFolder = (folder) => {
            folder.id = this.generateID(folder.id)
            /** @type { (program: PLC_Program) => void } */
            folder.children.forEach(child => {
                if (child.type === 'folder') return reserveFolder(child)
                if (child.type === 'program') return reserveProgram(child) // @ts-ignore
                throw new Error(`Invalid child type: ${child.type}`)
            })
        }
        /** @param { PLC_Program } program */
        const reserveProgram = (program) => {
            program.id = this.generateID(program.id)
            if (!this.active_tab) this.active_tab = program.id
            if (program.id === this.active_tab) {
                program.blocks.forEach(block => {
                    block.id = this.generateID(block.id)
                    block.blocks.forEach(ladder => {
                        ladder.id = this.generateID(ladder.id)
                    })
                    block.connections.forEach(con => {
                        con.id = this.generateID(con.id)
                    })
                })
            }
        }
        this.project.project.forEach(child => {
            if (child.type === 'folder') return reserveFolder(child)
            if (child.type === 'program') return reserveProgram(child) // @ts-ignore
            throw new Error(`Invalid child type: ${child.type}`)
        })
        this.draw()
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

    /** @param { string | undefined | null } [existing] */
    generateID = (existing) => {
        if (existing) { this.reserveID(existing); return existing }
        while (true) { const id = generateID(); if (this.reserveID(id)) return id }
    }

    /** @param { string | undefined } id */
    reserveID = (id) => {
        if (!id) return false
        const exists = this.reserved_ids.some(reserved => reserved === id)
        if (!exists) {
            this.reserved_ids.push(id)
            return true
        }
        return false
    }

    draw() {
        if (this.runtime_ready && this.runtime.wasm_exports) {
            this.runtime.wasm_exports.run()
            const u8array = this.runtime.readMemoryArea(0, 50)
            this.memory = [...u8array]
        }
        this.editor.draw()
    }
}

export default {
    ElementSynthesis,
    VovkPLCEditor
}