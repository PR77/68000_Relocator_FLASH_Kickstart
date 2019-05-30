`timescale 1ns / 1ps
/*
    This file is part of FLASH_KICKSTART originally designed by
    Paul Raspa 2017-2018.

    FLASH_KICKSTART is free software: you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation, either version 3 of the
    License, or (at your option) any later version.

    FLASH_KICKSTART is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FLASH_KICKSTART. If not, see <http://www.gnu.org/licenses/>.

    Revision 0.0 - 08.09.2018:
    Initial revision.
*/

 module FLASH_KICKSTART(

    input RESET,
    input MB_CLK,
    
    input CPU_AS,
    output MB_AS,
       
    output MB_DTACK,
    
    input E_CLK,    
        
    input RW,
    input LDS,
    input UDS,
        
    // Address bus
    input [23:16] ADDRESS_HIGH,
    input [7:1] ADDRESS_LOW,
    
    // Data bus
    inout [15:12] DATA,
    
    // Flash control
    output [1:0] FLASH_WR,
    output [1:0] FLASH_RD,
    output FLASH_A19,
    
    // Configuration and control
    input BLOCK
    );
    
// --- Control Global Registers

reg [19:0] eClockCounter = 20'b00000000000000000000;
reg programmingSession = 1'b0;
    
// --- AUTOCONFIG

reg configured = 1'b0;
reg shutup = 1'b0;
reg allConfigured = 1'b0;
reg [3:0] autoConfigData = 4'b0000;
reg [7:0] autoConfigBaseFlash = 8'b00000000;

wire DS = (LDS & UDS);

wire AUTOCONFIG_RANGE = ({ADDRESS_HIGH[23:16]} == {8'hE8}) && ~CPU_AS && ~&allConfigured && (programmingSession == 1'b1);

// Create allConfigured array based on "configured" and "shutup" status'.
always @(posedge CPU_AS or negedge RESET) begin

    if (RESET == 1'b0) begin
        allConfigured <= 1'b0;
    
    end else begin
    
        allConfigured <= (configured | shutup);
    end
end

// AUTOCONFIG cycle.
always @(negedge DS or negedge RESET) begin
    
    if (RESET == 1'b0) begin
        configured <= 1'b0; 
        shutup <= 1'b0;
        autoConfigData[3:0] <= 4'b1111;
        autoConfigBaseFlash[7:0] <= 8'h0;
    end else begin

       if (AUTOCONFIG_RANGE == 1'b1 && RW == 1'b0) begin
            // AutoConfig Write sequence. Here is where we receive from the OS the base address for the RAM.
            case (ADDRESS_LOW[7:1])
                'h24: begin
                // Written second
                    autoConfigBaseFlash[7:4] <= DATA[15:12];  // Flash
                    configured <= 1'b1;
                end

                'h25: begin
                // Written first
                    autoConfigBaseFlash[3:0] <= DATA[15:12]; // Flash
                end

                'h26: begin
                // Written asynchronously if the KS decides to not configure a specific device
                    shutup <= 1'b1;                         // Flash
                end
                
            endcase
        end

       // AutoConfig Read sequence. Here is where we publish the RAM and I/O port size and hardware attributes.
       case (ADDRESS_LOW[7:1])
            'h00: begin
                autoConfigData <= 4'hC;     // (00) Flash
            end
            
            'h01: begin
                if (BLOCK == 1'b1) begin
                    autoConfigData <= 4'h4;     // (02) Flash 512K
                end else begin
                    autoConfigData <= 4'h5;     // (02) Flash 1M
                end
            end
            
            'h02: autoConfigData <= 4'h9;   // (04)  
            
            'h03: begin
                autoConfigData <= 4'h7;     // (06) Flash
            end

            'h04: autoConfigData <= 4'h7;   // (08/0A)
            'h05: autoConfigData <= 4'hF;
            
            'h06: autoConfigData <= 4'hF;   // (0C/0E)
            'h07: autoConfigData <= 4'hF;
            
            'h08: autoConfigData <= 4'hF;   // (10/12)
            'h09: autoConfigData <= 4'h8;
            'h0A: autoConfigData <= 4'h4;   // (14/16)
            'h0B: autoConfigData <= 4'h6;                
            
            'h0C: autoConfigData <= 4'hA;   // (18/1A)
            'h0D: autoConfigData <= 4'hF;
            'h0E: autoConfigData <= 4'hB;   // (1C/1E)
            'h0F: autoConfigData <= 4'hE;
            'h10: autoConfigData <= 4'hA;   // (20/22)
            'h11: autoConfigData <= 4'hA;
            'h12: autoConfigData <= 4'hB;   // (24/26)
            'h13: autoConfigData <= 4'h3;

            default: 
                autoConfigData <= 4'hF;

        endcase
     end
end

// Output specific AUTOCONFIG data.
assign DATA[15:12] = (AUTOCONFIG_RANGE == 1'b1 && RW == 1'b1 && ~DS && ~&allConfigured) ? {autoConfigData[3:0]} : 4'bZZZZ;

// --- Custom Chip Access Detection

reg customChipsAccessed = 1'b0;
reg internalOverlayLatch = 1'b0;

wire CUSTOM_CHIP_RANGE = ({ADDRESS_HIGH[23:16]} == {8'hDF}) && ~CPU_AS && (programmingSession == 1'b0);

// Custom chip write access detection.
always @(negedge DS or negedge RESET) begin
    
    if (RESET == 1'b0) begin
        customChipsAccessed <= 1'b0; 
    end else begin

        if (CUSTOM_CHIP_RANGE == 1'b1 && RW == 1'b0) begin
       
            customChipsAccessed <= 1'b1;
        end
    end
end

// Custom chip write access latched when CPU_AS = INACTIVE.
always @(posedge CPU_AS or negedge RESET) begin

    if (RESET == 1'b0) begin
        internalOverlayLatch <= 1'b0; 
    end else begin

        internalOverlayLatch <= customChipsAccessed;
    end

end

// --- Kickstart and Flash control

// KICKSTART_RANGE is active throughout 0xF80000 to 0xFFFFFF for 256K and 512K KS support.
wire KICKSTART_RANGE = ({ADDRESS_HIGH[23:19]} == {5'h1F}) && ~CPU_AS;

// After /RESET, CPU PC = 0, therefore we need to intercept Address Selections in this range
// until the /OVL (Overlay) is applied by GARY. I simulate this also with "internalOverlayLatch"
// by detecting a write to the Customer Chips. KICKSTART_RESET_RANGE is active during these
// bus cycles (first 4 during the SP and PC fetch).
wire KICKSTART_RESET_RANGE = ({ADDRESS_HIGH[23:16]} == {8'h00}) && ~CPU_AS && ~internalOverlayLatch;

// FLASH_RANGE is active when programming session is active and memory is in Zorro II space.
wire FLASH_RANGE = ({ADDRESS_HIGH[23:20]} == {autoConfigBaseFlash[7:4]}) && ~CPU_AS && configured;

// --- FLASH Control

assign FLASH_RD[1:0] = (programmingSession == 1'b0 && RW == 1'b1 && (KICKSTART_RANGE || KICKSTART_RESET_RANGE)) ? {UDS, LDS} : 
                       (programmingSession == 1'b1 && RW == 1'b1 && FLASH_RANGE) ? {UDS, LDS} : {1'b1, 1'b1};
                       
assign FLASH_WR[1:0] = (programmingSession == 1'b1 && RW == 1'b0 && FLASH_RANGE) ? {UDS, LDS} : {1'b1, 1'b1};

// --- /AS Override Control

// While in programming session, this means using the ROM KS then pass /AS directly to
// the Amiga Mothboard to allow GARY to perform the address decoding. However when not
// in programming session, this means using the FLASH KS, then block /AS.

assign MB_AS = (programmingSession == 1'b0 && (KICKSTART_RANGE || KICKSTART_RESET_RANGE)) ? 1'b1 :
               (programmingSession == 1'b1 && AUTOCONFIG_RANGE) ? 1'b1 : CPU_AS;

/// --- /DTACK Override Control

// When accessing the FLASH KS, /AS is not asserted on the Amiga Motherboard, therefore
// GARY will not decode and assert /DTACK. As a result, we need to locally create this
// signal. Conventional timing is used. I.e., /DTACK asserted in rising edge of S4.
// A slow /DTACK is used for AUTOCONFIG in the event of a cascaded chain request; if used
// with an accelerator.

reg INTERNAL_CYCLE_DTACK_NORMAL = 1'b0;
reg INTERNAL_CYCLE_DTACK_SLOW = 1'b0;

// Everything is in the 7MHz clock domain so this should keep things simple.
always @(posedge MB_CLK or posedge CPU_AS) begin
    
    if (CPU_AS == 1'b1) begin
        INTERNAL_CYCLE_DTACK_NORMAL <= 1'b0;
        INTERNAL_CYCLE_DTACK_SLOW <= 1'b0;
    end else begin
        
        // Falling edge of S4.    
        INTERNAL_CYCLE_DTACK_NORMAL <= 1'b1;
        INTERNAL_CYCLE_DTACK_SLOW <= INTERNAL_CYCLE_DTACK_NORMAL;
    end
end

assign MB_DTACK = (INTERNAL_CYCLE_DTACK_NORMAL && ((programmingSession == 1'b0) && (KICKSTART_RANGE || KICKSTART_RESET_RANGE))) ? 1'b0 :
                  (INTERNAL_CYCLE_DTACK_SLOW && ((programmingSession == 1'b1) && AUTOCONFIG_RANGE)) ? 1'b0 : 1'bZ;
                  
// --- Reset Duration Detection

reg [1:0] stSessionChange = 2'b00;

// Active /RESET without interruption for longer than 1 second will toggle between
// Programming Session using the ROM Kickstart and Flash Kickstart. Default is always 
// Flash Kickstart after POR.
always @(posedge E_CLK) begin
    
    if (RESET == 1'b1) begin
        stSessionChange <= 2'b00;
    end
    
    case (stSessionChange)
    
        2'b00: begin
            if (RESET == 1'b0) begin
                eClockCounter <= 20'b00000000000000000000;
                if (programmingSession == 1'b0)
                    stSessionChange <= 2'b01;
                else
                    stSessionChange <= 2'b10;
            end
        end
        
        2'b01: begin
            if (RESET == 1'b0) begin
                eClockCounter <= eClockCounter + 1;

                if (&eClockCounter) begin
                    eClockCounter <= 20'b00000000000000000000;
                    programmingSession <= 1'b1;
                end
            end
        end

        2'b10: begin
            if (RESET == 1'b0) begin
                eClockCounter <= eClockCounter + 1;

                if (&eClockCounter) begin
                    eClockCounter <= 20'b00000000000000000000;
                    programmingSession <= 1'b0;
                end
            end
        end

        default: 
            stSessionChange <= 2'b00;        
    
    endcase
end

endmodule
