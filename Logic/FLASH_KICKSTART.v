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
*/

 module FLASH_KICKSTART(
    input CLK,
    input E_CLK,

    input RESET_n,
    input CPU_AS_n,
    input LDS_n,
    input UDS_n,
    input RW,

    output MB_AS_n,
    output DTACK_n,

    input [23:16] ADDRESS_HIGH,
    input [7:1] ADDRESS_LOW,
    inout [15:12] DATA,

    output [1:0] FLASH_WR_n,
    output [1:0] FLASH_RD_n,
    output FLASH_A19,

    input SIZE_512K
    );

    assign FLASH_A19 = 1'b0;

    reg useMotherboardKickstart = 1'b0;
    reg [19:0] switchCounter = 20'd0;
    reg hasSwitched = 1'b0;

    always @(posedge E_CLK or posedge RESET_n)
    begin
        if (RESET_n)
        begin
            switchCounter <= 20'd0;
            hasSwitched <= 1'b0;
        end
        else
        begin
            switchCounter <= switchCounter + 20'd1;
            if (!hasSwitched && (&switchCounter))
            begin
                hasSwitched <= 1'b1;
                useMotherboardKickstart <= !useMotherboardKickstart;
            end
        end
    end

    reg driveMbAsReq = 1'b0;
    reg driveMbAsAck = 1'b0;
    assign MB_AS_n = driveMbAsReq ^ driveMbAsAck ? 1'b0 : 1'b1;

    reg driveDtackReq = 1'b0;
    reg driveDtackAck = 1'b0;
    assign DTACK_n = driveDtackReq ^ driveDtackAck ? 1'b0 : 1'bZ;

    reg [3:0] dataOut = 4'd0;
    reg driveDataReq = 1'b0;
    reg driveDataAck = 1'b0;
    assign DATA = driveDataReq ^ driveDataAck ? dataOut : 4'bZZZZ;

    reg [1:0] driveFlashWrReq = 2'b00;
    reg [1:0] driveFlashWrAck = 2'b00;
    assign FLASH_WR_n = ~(driveFlashWrReq ^ driveFlashWrAck);

    reg [1:0] driveFlashRdReq = 2'b00;
    reg [1:0] driveFlashRdAck = 2'b00;
    assign FLASH_RD_n = ~(driveFlashRdReq ^ driveFlashRdAck);

    reg autoConfigComplete = 1'b0;
    reg [3:0] flashBase = 4'h0;
    reg flashBaseValid = 1'b0;
    reg overlay_n = 1'b0;

    wire ciaRange               = ADDRESS_HIGH[23:16] == 8'hBF;
    wire autoConfigRange        = ADDRESS_HIGH[23:16] == 8'hE8;
    wire kickstartRange         = ADDRESS_HIGH[23:19] == 5'h1F;
    wire kickstartOverlayRange  = ADDRESS_HIGH[23:16] == 8'h00;
    wire flashRange             = ADDRESS_HIGH[23:20] == flashBase && flashBaseValid;

    wire relocatorKickstartAccess   = !useMotherboardKickstart && (kickstartRange || (!overlay_n && kickstartOverlayRange));
    wire autoConfigAccess           = useMotherboardKickstart && autoConfigRange && !autoConfigComplete;
    wire flashAccess                = useMotherboardKickstart && flashRange;

    wire relocatorAccess            = relocatorKickstartAccess || autoConfigAccess;

    wire addressStrobe = !CPU_AS_n && RESET_n;

    always @(posedge addressStrobe)
    begin
        if (!relocatorAccess)
            driveMbAsReq <= !driveMbAsAck;
    end

    always @(negedge addressStrobe)
    begin
        driveMbAsAck <= driveMbAsReq;
    end

    wire DS_n = UDS_n && LDS_n;
    wire access = !CPU_AS_n && !DS_n && RESET_n;

    always @(posedge access or negedge RESET_n)
    begin
        if (!RESET_n)
            overlay_n <= 1'b0;
        else if (ciaRange)
            overlay_n <= 1'b1;
    end

    always @(posedge access or negedge RESET_n)
    begin
        if (!RESET_n)
        begin
            flashBase <= 4'h0;
            flashBaseValid <= 1'b0;
            autoConfigComplete <= 1'b0;
        end
        else if (autoConfigAccess && !RW)
        begin
            if (ADDRESS_LOW == 7'h24)
            begin
                flashBase <= DATA;
                flashBaseValid <= 1'b1;
                autoConfigComplete <= 1'b1;
            end
            else if (ADDRESS_LOW == 7'h26)
                autoConfigComplete <= 1'b1;
        end
    end

    always @(posedge access)
    begin
        if (relocatorAccess)
            driveDtackReq <= !driveDtackAck;
    end

    always @(posedge access)
    begin
        if ((relocatorKickstartAccess || flashAccess) && RW)
            driveFlashRdReq <= driveFlashRdAck ^ {!UDS_n, !LDS_n};
    end

    always @(posedge access)
    begin
        if (flashAccess && !RW)
            driveFlashWrReq <= driveFlashWrAck ^ {!UDS_n, !LDS_n};
    end

    always @(posedge access)
    begin
        if (autoConfigAccess && RW)
        begin
            driveDataReq <= !driveDataAck;

            if (ADDRESS_LOW[7:6] == 2'd0)
                case (ADDRESS_LOW[5:1])
                    5'h00: dataOut <= 4'hC;  // (00)
                    5'h01: dataOut <= SIZE_512K ? 4'h4 : 4'h5; // (02) 512K / 1M
                    5'h02: dataOut <= 4'h9;  // (04)
                    5'h03: dataOut <= 4'h7;  // (06)
                    5'h04: dataOut <= 4'h7;  // (08/0A)
                    5'h05: dataOut <= 4'hF;
                    5'h06: dataOut <= 4'hF;  // (0C/0E)
                    5'h07: dataOut <= 4'hF;
                    5'h08: dataOut <= 4'hF;  // (10/12)
                    5'h09: dataOut <= 4'h8;
                    5'h0A: dataOut <= 4'h4;  // (14/16)
                    5'h0B: dataOut <= 4'h6;
                    5'h0C: dataOut <= 4'hA;  // (18/1A)
                    5'h0D: dataOut <= 4'hF;
                    5'h0E: dataOut <= 4'hB;  // (1C/1E)
                    5'h0F: dataOut <= 4'hE;
                    5'h10: dataOut <= 4'hA;  // (20/22)
                    5'h11: dataOut <= 4'hA;
                    5'h12: dataOut <= 4'hB;  // (24/26)
                    5'h13: dataOut <= 4'h3;
                    default: dataOut <= 4'hF;
                endcase
            else
                dataOut <= 4'hF;
        end
    end

    always @(negedge access)
    begin
        driveDtackAck <= driveDtackReq;
        driveDataAck <= driveDataReq;
        driveFlashWrAck <= driveFlashWrReq;
        driveFlashRdAck <= driveFlashRdReq;
    end
endmodule
