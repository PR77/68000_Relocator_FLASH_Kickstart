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
    input [6:0] ADDRESS_LOW,
    
    // Data bus
    inout [15:12] DATA,
    
    // Flash control
    output [1:0] FLASH_WR,
    output [1:0] FLASH_RD,
    output FLASH_A19,
    
    // Configuration and control
    output BLOCK
    );
    
// --- CONTROL GLOBALS

reg [19:0] eClockCounter = 20'b00000000000000000000;
reg programmingSession = 1'b0;
    
// --- AUTOCONFIG

reg configured = 1'b0;
reg shutup = 1'b0;
reg [3:0] autoConfigData = 4'b0000;
reg [7:0] autoConfigBaseFlash = 8'b00000000;

wire DS = (LDS & UDS);

wire AUTOCONFIG_RANGE = ({ADDRESS_HIGH[23:16]} == {8'hE8}) && ~CPU_AS && ~&shutup && ~&configured && (programmingSession == 1'b1);
wire AUTOCONFIG_READ = (AUTOCONFIG_RANGE && (RW == 1'b1));
wire AUTOCONFIG_WRITE = (AUTOCONFIG_RANGE && (RW == 1'b0));

// AUTOCONFIG cycle.
always @(negedge DS or negedge RESET) begin
    
    if (RESET == 1'b0) begin
        configured <= 1'b0; 
        shutup <= 1'b0;     
        autoConfigBaseFlash[7:0] <= 8'h0;
    end else begin

       if (AUTOCONFIG_WRITE == 1'b1) begin
            // AutoConfig Write sequence. Here is where we receive from the OS the base address for the RAM.
            case (ADDRESS_LOW[6:0])
                8'h24: begin
                // Written second
                    autoConfigBaseFlash[7:4] <= DATA[15:12];  // Flash
                    configured <= 1'b1;
                end

                8'h25: begin
                // Written first
                    autoConfigBaseFlash[3:0] <= DATA[15:12]; // Flash
                end

                8'h26: begin
                // Written last
                    shutup <= 1'b1;  // Flash
                end
                
            endcase
        end

        if (AUTOCONFIG_READ == 1'b1) begin
            // AutoConfig Read sequence. Here is where we publish the RAM and I/O port size and hardware attributes.
           case (ADDRESS_LOW[6:0])
                8'h00: begin
                    autoConfigData <= 4'hC;     // (00) Flash
                end
                
                8'h01: begin
                    autoConfigData <= 4'h4;     // (02) Flash
                end
                
                8'h02: autoConfigData <= 4'h9;  // (04)  
                
                8'h03: begin
                    autoConfigData <= 4'hB;     // (06) Flash
                end

                8'h04: autoConfigData <= 4'h7;  // (08/0A)
                8'h05: autoConfigData <= 4'hF;
                
                8'h06: autoConfigData <= 4'hF;  // (0C/0E)
                8'h07: autoConfigData <= 4'hF;
                
                8'h08: autoConfigData <= 4'hF;  // (10/12)
                8'h09: autoConfigData <= 4'h8;
                8'h0A: autoConfigData <= 4'h4;  // (14/16)
                8'h0B: autoConfigData <= 4'h6;                
                
                8'h0C: autoConfigData <= 4'hA;  // (18/1A)
                8'h0D: autoConfigData <= 4'hF;
                8'h0E: autoConfigData <= 4'hB;  // (1C/1E)
                8'h0F: autoConfigData <= 4'hE;
                8'h10: autoConfigData <= 4'hA;  // (20/22)
                8'h11: autoConfigData <= 4'hA;
                8'h12: autoConfigData <= 4'hB;  // (24/26)
                8'h13: autoConfigData <= 4'h3;

                default: 
                    autoConfigData <= 4'hF;

            endcase
        end
     end
end

// Output specific AUTOCONFIG data.
assign DATA[15:12] = (AUTOCONFIG_READ == 1'b1 && ~&shutup) ? autoConfigData : 4'bZZZZ;

// --- KICKSTART AND FLASH RANGE Control

// KICKSTART_RANGE is active throughout 0xF80000 to 0xFFFFFF for 256K and 512K KS support.
wire KICKSTART_RANGE = ({ADDRESS_HIGH[23:19]} == {5'h1F}) && ~CPU_AS & ~DS;

wire FLASH_RANGE = ({ADDRESS_HIGH[23:20]} == {autoConfigBaseFlash[7:4]}) && ~CPU_AS && ~DS && configured;

// --- FLASH Control

assign FLASH_RD[1:0] = (programmingSession == 1'b0 && RW == 1'b0 && KICKSTART_RANGE) ? {UDS, LDS} : 
                       (programmingSession == 1'b1 && RW == 1'b0 && FLASH_RANGE) ? {UDS, LDS} : {1'b1, 1'b1};
                       
assign FLASH_WR[1:0] = (programmingSession == 1'b1 && RW == 1'b1 && FLASH_RANGE) ? {UDS, LDS} : {1'b1, 1'b1};

// --- /AS Override Control

// While in programming session, this means using the ROM KS then pass /AS directly to
// the Amiga Mothboard to allow GARY to perform the address decoding. However when not
// in programming session, this means using the FLASH KS, then block /AS.

assign MB_AS = (programmingSession == 1'b0 && KICKSTART_RANGE) ? 1'b1 : 
               (AUTOCONFIG_RANGE) ? 1'b1 : CPU_AS;

/// --- /DTACK Override Control

// When accessing the FLASH KS, /AS is not asserted on the Amiga Motherboard, therefore
// GARY will not decode and assert /DTACK. As a result, we need to locally create this
// signal. Conventional timing is used. I.e., /DTACK asserted in rising edge of S4.

reg INTERNAL_CYCLE_DTACK = 1'b1;

// Everything is in the 7MHz clock domain so this should keep things simple.

always @(posedge MB_CLK or posedge CPU_AS) begin
    
    if (CPU_AS == 1'b1) begin
        INTERNAL_CYCLE_DTACK <= 1'b0;
    end else begin
        
        // Rising edge of S4.    
        INTERNAL_CYCLE_DTACK <= 1'b1;
    end
end

assign MB_DTACK = (INTERNAL_CYCLE_DTACK && ((programmingSession == 1'b0) && FLASH_RANGE) || AUTOCONFIG_RANGE) ? 1'b0 : 1'bZ;

// --- Reset Duration Detection

// Count the number of E clock cycles while /RESET is low. Every new /RESET negative edge
// resets the clock cycle counter.

reg START_COUNTING = 1'b0;

always @(posedge E_CLK) begin
    
    if (RESET == 1'b1) begin
        START_COUNTING <= 1'b0;
    end
    
    if (RESET == 1'b0 && START_COUNTING == 1'b0 && programmingSession == 1'b0) begin
        eClockCounter <= 20'b00000000000000000000;
        programmingSession <= 1'b0;
        START_COUNTING <= 1'b1;
    end else begin
    
        if (START_COUNTING == 1'b1) begin
            eClockCounter <= eClockCounter + 1;
        end
        
        //TODO: due to /RESET glitches after a long reset, need to time the time after reset before allowing a session change back to programming
        
        if (programmingSession == 1'b0 && &eClockCounter) begin
            programmingSession <= 1'b1;
        end
    end

    /*
    if (RESET == 1'b1) begin
        START_COUNTING <= 1'b0;
    end
    
    if (RESET == 1'b0 && START_COUNTING == 1'b0) begin
        eClockCounter <= 20'b00000000000000000000;
        programmingSession <= 1'b0;
        START_COUNTING <= 1'b1;
    end else begin
    
        if (START_COUNTING == 1'b1) begin
            eClockCounter <= eClockCounter + 1;
        end
        
        //TODO: due to /RESET glitches after a long reset, need to time the time after reset before allowing a session change back to programming
        
        if (programmingSession == 1'b0 && &eClockCounter) begin
            programmingSession <= 1'b1;
        end
    end
    */
end

assign BLOCK = programmingSession;
assign FLASH_A19 = START_COUNTING;

endmodule
