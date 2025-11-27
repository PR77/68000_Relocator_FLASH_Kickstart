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

    Revision 1.0 - 12.06.2019:
    Thanks to https://github.com/niklasekstrom for refactoring and addressing
    the Auto Config (R) glitch when used with a cascaded chain.
    
    Revision 2.0 - 22.08.2020:
    Thanks to https://github.com/niklasekstrom for again refactoring and addressing
    the issue identified by "The Q!". 
    
    Revision 2.1 - 27.11.2025:
    Minor formatting improvements and added some necessary comments to the verilog.
    Qualified DTACK_n with LDS_n and UDS_n signals.
    
*/

 module FLASH_KICKSTART(
    input           CLK,
    input           E_CLK,

    input           RESET_n,
    input           CPU_AS_n,
    input           LDS_n,
    input           UDS_n,
    input           RW,

    output          MB_AS_n,
    output          DTACK_n,

    input [23:16]   ADDRESS_HIGH,
    input [7:1]     ADDRESS_LOW,
    inout [15:12]   DATA,

    output [1:0]    FLASH_WR_n,
    output [1:0]    FLASH_RD_n,
    output          FLASH_A19,

    input           SIZE_512K
);

// --- Configuration constants for my hardware - these should probably be centerally documented.
localparam [15:0] manufacturerId = 16'd1977;
localparam [7:0] productIdRelocator = 8'd104;
localparam [7:0] productIdFastRAM = 8'd103;
localparam [7:0] productIdSPI = 8'd102;
localparam [7:0] productIdIOPort = 8'd101;
localparam [31:0] serialNumber = 32'd27112025;

// --- FLASH_A19 used for BANK selection. Can be updated to support 1Mb Kickstart Extensions.
assign FLASH_A19 = (SIZE_512K == 1'b1) ? 1'b0 : 1'b1;

// --- Base address of Relocated assigned during AUTOCONFIG (R) process. Also valid flag
// for assigned based address.
reg [3:0] r_flashBase = 4'h0;
reg r_flashBaseValid = 1'b0;

// --- Decode some basic memory regions.
wire ciaRange               = ADDRESS_HIGH[23:16] == 8'hBF;
wire autoConfigRange        = ADDRESS_HIGH[23:16] == 8'hE8;
wire kickstartRange         = ADDRESS_HIGH[23:19] == 5'h1F;
wire kickstartOverlayRange  = ADDRESS_HIGH[23:16] == 8'h00;
wire flashRange             = ADDRESS_HIGH[23:20] == r_flashBase && r_flashBaseValid;

// --- Counter to detect when KBRESET (actually RESET_n) is active for approx 1.5 seconds.
// This is done by counting E_CLK cycles when RESET_n is asserted. Luckily E_CLK doesn't
// stop when the 68000 is in RESET.
reg r_useMotherboardKickstart = 1'b0;
reg [19:0] r_switchCounter = 20'd0;
reg r_hasSwitched = 1'b0;

always @(posedge E_CLK or posedge RESET_n) begin
    
    if (RESET_n) begin
        r_switchCounter <= 20'd0;
        r_hasSwitched <= 1'b0;
    
    end else begin
    
        r_switchCounter <= r_switchCounter + 20'd1;
        
        if (!r_hasSwitched && (&r_switchCounter)) begin
            r_hasSwitched <= 1'b1;
            r_useMotherboardKickstart <= !r_useMotherboardKickstart;
        end
    end
end

// --- Detect when the first access to CIA has occurred. This clears the /OVR signal to GARY.
reg r_overlayDeactivated = 1'b0;

always @(posedge CPU_AS_n or negedge RESET_n) begin
    
    if (!RESET_n)
        r_overlayDeactivated <= 1'b0;
        
    else if (ciaRange)
        r_overlayDeactivated <= 1'b1;
end

// --- AUTOCONFIG (R) read and write handler - http://amigadev.elowar.com/read/ADCD_2.1/Hardware_Manual_guide/node02C8.html
reg r_autoConfigComplete = 1'b0;
reg [3:0] r_dataOut = 4'h0;
wire autoConfigAccess = r_useMotherboardKickstart && autoConfigRange && !r_autoConfigComplete;

always @(posedge CPU_AS_n or negedge RESET_n) begin
    
    // AUTOCONFIG (R) write handler. Here we get the base address of the
    // "Memory Expansion" which is actually the Flash Memory.
    
    if (!RESET_n) begin
        r_flashBase <= 4'h0;
        r_flashBaseValid <= 1'b0;
        r_autoConfigComplete <= 1'b0;
        
    end else begin
    
        if (autoConfigAccess && !RW) begin     
            if (ADDRESS_LOW == 7'h24) begin
                r_flashBase <= DATA;
                r_flashBaseValid <= 1'b1;
                r_autoConfigComplete <= 1'b1;
            end
            
            if (ADDRESS_LOW == 7'h26) begin
                r_autoConfigComplete <= 1'b1;
            end
        end
    end
end

always @(negedge CPU_AS_n) begin
    
    // AUTOCONFIG (R) read handler. Here we assemble the register assignments
    // specific to the Relocator hardware.
    
    if (ADDRESS_LOW[7:6] == 2'd0) begin
        case (ADDRESS_LOW[5:1])
            5'h00: r_dataOut <= 4'hC;                       // (00)
            5'h01: r_dataOut <= 4'h4;                       // (02) 512K
            5'h02: r_dataOut <= ~productIdRelocator[7:4];   // (04)
            5'h03: r_dataOut <= ~productIdRelocator[3:0];   // (06)
            5'h04: r_dataOut <= 4'h7;                       // (08/0A)
            5'h05: r_dataOut <= 4'hF;
            5'h06: r_dataOut <= 4'hF;                       // (0C/0E)
            5'h07: r_dataOut <= 4'hF;
            5'h08: r_dataOut <= ~manufacturerId[15:12];     // (10/12)
            5'h09: r_dataOut <= ~manufacturerId[11:8];
            5'h0A: r_dataOut <= ~manufacturerId[7:4];       // (14/16)
            5'h0B: r_dataOut <= ~manufacturerId[3:0];
            5'h0C: r_dataOut <= ~serialNumber[31:28];       // (18/1A)
            5'h0D: r_dataOut <= ~serialNumber[27:24];
            5'h0E: r_dataOut <= ~serialNumber[23:20];       // (1C/1E)
            5'h0F: r_dataOut <= ~serialNumber[19:16];
            5'h10: r_dataOut <= ~serialNumber[15:12];       // (20/22)
            5'h11: r_dataOut <= ~serialNumber[11:8];
            5'h12: r_dataOut <= ~serialNumber[7:4];         // (24/26)
            5'h13: r_dataOut <= ~serialNumber[3:0];
            default: r_dataOut <= 4'hF;
        endcase
     
     end else r_dataOut <= 4'hF;
end

assign DATA = !CPU_AS_n && autoConfigAccess && RW ? r_dataOut : 4'bZZZZ;

// --- Logic wiring...
wire relocatorKickstartAccess = !r_useMotherboardKickstart && (kickstartRange || (!r_overlayDeactivated && kickstartOverlayRange));
wire flashAccess = r_useMotherboardKickstart && flashRange;
wire relocatorAccess = relocatorKickstartAccess || autoConfigAccess || flashAccess;

assign DTACK_n = relocatorAccess && !CPU_AS_n && (!UDS_n || !LDS_n) ? 1'b0 : 1'bZ;
assign MB_AS_n = !relocatorAccess ? CPU_AS_n : 1'bZ;

assign FLASH_RD_n = !CPU_AS_n && (relocatorKickstartAccess || flashAccess) && RW ? {UDS_n, LDS_n} : 2'b11;
assign FLASH_WR_n = !CPU_AS_n && flashAccess && !RW ? {UDS_n, LDS_n} : 2'b11;

endmodule
