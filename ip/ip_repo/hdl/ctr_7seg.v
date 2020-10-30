`timescale 1ns / 1ps
//lab_d_vlog1
module ctrl_7seg(
    input clk,
    input rstn,
    input [7:0]data_in,
    output sel,
    output [7:0]seg_data
    );
    
  reg [15:0]count;
  reg [7:0]seg_data;
  wire [3:0]disp_data;

always@(posedge clk)
  if(rstn==1'b0)
    count <= 16'd0;
  else
    count <= count + 1;
    
assign sel = count[15];
assign disp_data = (sel==1'b1)?data_in[7:4]:data_in[3:0];
  
 always@(posedge clk)
  if(rstn==1'b0)
    seg_data <= 8'h00;
  else
    case(disp_data) 
      4'h0:seg_data<=8'b00111111;
      4'h1:seg_data<=8'b00000110;
      4'h2:seg_data<=8'b01011011;
      4'h3:seg_data<=8'b01001111;
      4'h4:seg_data<=8'b01100110;
      4'h5:seg_data<=8'b01101101;
      4'h6:seg_data<=8'b01111101;
      4'h7:seg_data<=8'b00000111;
      4'h8:seg_data<=8'b01111111;
      4'h9:seg_data<=8'b01100111;
      4'ha:seg_data<=8'b01110111;
      4'hb:seg_data<=8'b01111100;
      4'hc:seg_data<=8'b00111001;
      4'hd:seg_data<=8'b01011110;
      4'he:seg_data<=8'b01111001;
      4'hf:seg_data<=8'b01110001;
      default:seg_data<=8'b00000000;
    endcase
endmodule