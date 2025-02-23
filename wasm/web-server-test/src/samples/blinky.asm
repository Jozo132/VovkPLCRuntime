    
    u8.readBit     2.4    // Read bit 2.4 which is 1s pulse
    jump_if_not    end    // Jump to the label 'end' if the bit is OFF
    u8.writeBitInv 32.0   // Invert bit 32.0
end:                      // Label to jump to