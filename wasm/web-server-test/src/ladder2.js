// @ts-check
"use strict"

/** @typedef { import('./VovkPLCEditor.js').PLC_Project } PLC_Project */
import { VovkPLCEditor, ElementSynthesis } from './VovkPLCEditor.js'

/** @type {PLC_Project} */
const plc_project = {
    offsets: {
        control: { offset: 0, size: 10 },
        input: { offset: 10, size: 10 },
        output: { offset: 20, size: 10 },
        memory: { offset: 30, size: 10 },
        system: { offset: 40, size: 10 },
    },
    symbols: [
        { name: 'button1', location: 'input', type: 'bit', address: 0.0, initial_value: 0, comment: 'Test input' },
        { name: 'button2', location: 'input', type: 'bit', address: 0.1, initial_value: 0, comment: 'Test input' },
        { name: 'button3', location: 'input', type: 'bit', address: 0.2, initial_value: 0, comment: 'Test input' },
        { name: 'button4', location: 'input', type: 'bit', address: 0.3, initial_value: 0, comment: 'Test input' },
        { name: 'light1', location: 'output', type: 'bit', address: 0.0, initial_value: 0, comment: 'Test output' },
        { name: 'light2', location: 'output', type: 'bit', address: 0.1, initial_value: 0, comment: 'Test output' },
    ],
    project: [
        {
            type: 'folder',
            name: 'Programs',
            comment: 'This is the main folder where all programs are stored',
            children: [
                {
                    type: 'program',
                    name: 'Main',
                    comment: 'This is the main program',
                    blocks: [
                        {
                            type: 'ladder',
                            name: 'Test 1',
                            comment: 'Trying to get this to work',
                            // Test
                            blocks: [
                                { id: `0`, x: 0, y: 0, type: 'contact', inverted: false, trigger: 'normal', symbol: 'P_200ms' },
                                { id: `3`, x: 0, y: 1, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button2' },
                                { id: `4`, x: 1, y: 0, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button3' },
                                { id: `5`, x: 0, y: 2, type: 'contact', inverted: false, trigger: 'normal', symbol: 'light1' },
                                { id: `6`, x: 2, y: 0, type: 'contact', inverted: true, trigger: 'normal', symbol: 'button4' },
                                { id: `1`, x: 4, y: 0, type: 'coil', inverted: false, trigger: 'normal', symbol: 'light1' },
                                { id: `2`, x: 4, y: 1, type: 'coil_set', inverted: false, trigger: 'normal', symbol: 'light2' },


                                { id: `989`, x: 1, y: 5, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button1' },
                                { id: `990`, x: 3, y: 5, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button1' },
                                { id: `991`, x: 4, y: 5, type: 'coil', inverted: false, trigger: 'normal', symbol: 'button1' },
                                { id: `992`, x: 3, y: 6, type: 'coil', inverted: false, trigger: 'normal', symbol: 'button1' },
                                { id: `993`, x: 4, y: 6, type: 'contact', inverted: false, trigger: 'normal', symbol: 'button1' },
                                { id: `994`, x: 1, y: 6, type: 'coil', inverted: false, trigger: 'normal', symbol: 'button1' },
                            ],
                            connections: [
                                { from: { id: `0` }, to: { id: `4` } },
                                { from: { id: `3` }, to: { id: `4` } },
                                { from: { id: `4` }, to: { id: `6` } },
                                { from: { id: `6` }, to: { id: `2` } },
                                { from: { id: `5` }, to: { id: `6` } },
                                { from: { id: `6` }, to: { id: `1` } },

                                { from: { id: `990` }, to: { id: `991` } },
                                { from: { id: `992` }, to: { id: `993` } },
                            ]
                        }, {
                            type: 'ladder',
                            name: 'Test 2',
                            comment: '',
                            // Toggle switch
                            blocks: [
                                { id: `0`, x: 0, y: 0, type: 'contact', inverted: false, trigger: 'rising', symbol: 'button1' },
                                { id: `1`, x: 1, y: 0, type: 'contact', inverted: true, trigger: 'normal', symbol: 'light1' },
                                { id: `2`, x: 2, y: 0, type: 'coil_set', inverted: false, trigger: 'normal', symbol: 'light1' },
                                { id: `3`, x: 1, y: 1, type: 'contact', inverted: false, trigger: 'normal', symbol: 'light1' },
                                { id: `4`, x: 2, y: 1, type: 'coil_rset', inverted: false, trigger: 'normal', symbol: 'light1' },
                            ],
                            connections: [
                                { id: `1`, from: { id: `0` }, to: { id: `1` } },
                                { id: `2`, from: { id: `1` }, to: { id: `2` } },
                                { id: `3`, from: { id: `0` }, to: { id: `3` } },
                                { id: `4`, from: { id: `3` }, to: { id: `4` } },
                            ]
                        }
                    ] // blocks
                }
            ] // children
        }
    ] // project
}



const PLCEditor = new VovkPLCEditor({ workspace: 'PLCEditor', debug_css: true })

PLCEditor.open(plc_project)


const toggle_input = (offset) => {
    let state = PLCEditor.getInputBit(offset)
    state = !state
    PLCEditor.setInputBit(offset, state)
}

const toggle_output = (offset) => {
    let state = PLCEditor.getOutputBit(offset)
    state = !state
    PLCEditor.setOutputBit(offset, state)
}


let button1_state = false
const draw = () => {
    const light1 = PLCEditor.getOutputBit(0.0)
    const light2 = PLCEditor.getOutputBit(0.1)

    const button1 = PLCEditor.getInputBit(0.0)
    const button2 = PLCEditor.getInputBit(0.1)
    const button3 = PLCEditor.getInputBit(0.2)
    const button4 = PLCEditor.getInputBit(0.3)

    // Test
    const on = (((button1 || button2) && button3) || light1) && !button4
    PLCEditor.setOutputBit(20.0, on)
    if (on) {
        PLCEditor.setOutputBit(20.1, on)
    }

    PLCEditor.draw()

    // // Toggle switch
    // const button1_rising = button1 && !button1_state
    // button1_state = button1
    // if (button1_rising) {
    //     const light1_state = !light1
    //     setMemoryBit(20.0, light1_state)
    // }

}


draw()
setInterval(draw, 20)


Object.assign(window, { PLCEditor, ElementSynthesis, draw, plc_project, toggle_input, toggle_output })