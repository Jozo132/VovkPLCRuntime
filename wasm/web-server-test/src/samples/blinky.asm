
    u8.readBit     1.4    // Read bit 1.4 which is 1s pulse
    jump_if_not    end    // Jump to the label 'end' if the bit is OFF
    u8.writeBitInv 20.0   // Invert bit 20.0
end:                      // Label to jump to