module Top(mode, clk, rst, port, shift, latch, gnd, letter, start, set, speed_up, speed_down, record);

	input		[5:0]	mode;//64 mode 
	input				clk;
	input				rst;
	input 			letter;
	input 			start;
	input 			set;
	input				speed_up;
	input 			speed_down;
	output	[7:0] port;
	output			shift;
	output			latch;
	output	[2:0]	gnd;
	input			record;

	wire 	[511:0] 	data, letter_data, moder_data_out;
	wire 			 	ready;
	wire			 	clk_out;
	wire	  [8:0]  out;
	wire				d_start, d_set, d_spdup, d_spddn;
	wire stop;
	wire [3:0] speed;

	ledcube  m1(
      rst, 
      clk_out, 
      letter, 
      letter_data, 
      moder_data_out, 
      port, 
      latch, 
      shift, 
      gnd, 
      ready
   );
	
	mycontroller m2(
		.i_rst(rst),
		.i_play_stop_sig(d_start),
		.i_speed_up(d_spdup),
		.i_speed_down(d_spddn),
		.i_ready(ready),
		.o_stop_moder(stop),
		.o_speed_moder(speed)
	);
	MODER moder(
      .i_rst(rst),
      .i_ready(ready),
      .i_stop(stop),
      .i_mode(mode),
      .i_speed(speed),
      .i_random_layer(out),
      .o_data_moder(moder_data_out)
   );
	clk_divider m3(
      clk, 
      rst, 
      clk_out
   );
	lfsr m4(
      ready, 
      rst, 
      out
   );
	letter m5 (
      ready, 
      rst,
      mode, 
      d_set, 
      d_start, 
      letter_data
   );
	Debounce deb1(
		.i_in(start),
		.i_rst(rst),
		.i_clk(ready),
		.o_neg(d_start)
	);
	Debounce deb2(
		.i_in(set),
		.i_rst(rst),
		.i_clk(ready),
		.o_neg(d_set)
	);
	Debounce deb3(
		.i_in(speed_up),
		.i_rst(rst),
		.i_clk(ready),
		.o_neg(d_spdup)
	);
	Debounce deb4(
		.i_in(speed_down),
		.i_rst(rst),
		.i_clk(ready),
		.o_neg(d_spddn)
	);

endmodule
module mycontroller(
   input          i_rst,            // SW[16]
   input          i_play_stop_sig,  // key[0]
   input          i_speed_up,       // key[2]
   input          i_speed_down,     // key[3]
   input          i_ready,
   output   	  o_stop_moder,
   output	[3:0] o_speed_moder
);

   reg [511:0] eric_pattern_bits;


   // Declare controll FSM
   parameter IDLE = 3'd0, MODE = 3'd1, STOP = 3'd4;

   reg [2:0]       current_state_r, current_state_w;
   reg [3:0]       speed_r, speed_w;
   reg 			   stop_w, stop_r;
   assign o_stop_moder = stop_w;
   assign o_speed_moder = speed_r;
   

	
   always@(*) begin
      speed_w = speed_r;
      if(i_speed_up) begin
         if(speed_r < 4) speed_w = speed_r + 1;
      end
      if(i_speed_down) begin
         if((speed_r) > 0) speed_w = speed_r - 1;
      end
   end

   always@(*) begin
      current_state_w   = current_state_r;
	  stop_w            = stop_r;
      case(current_state_r) 
         MODE: begin
            if(i_play_stop_sig) begin
               current_state_w = STOP;
					stop_w = 1;
            end
         end
         STOP: begin
            if(i_play_stop_sig) begin
               current_state_w = MODE;
					stop_w = 1'd0;
            end
         end
         default: begin
            current_state_w = IDLE;
         end
      endcase
   end
	
   always@(posedge i_ready	or negedge i_rst) begin
      if(!i_rst) begin
         current_state_r   <= STOP;
         speed_r           <= 2;
			stop_r				<= 1;
      end else begin
			current_state_r	<= current_state_w;
         speed_r           <= speed_w;
			stop_r 				<= stop_w;
      end
   end
endmodule

module MODER(
   input          i_rst,
   input          i_ready,
   input		  i_stop,
   input [5:0]    i_mode,
   input [3:0]    i_speed,
   input [8:0]    i_random_layer,
   output [511:0] o_data_moder
);

   // Declare mode variable
   //   pattern
   integer i,j;
   parameter OFF        = 6'd0;
   parameter ON         = 6'd1;
   parameter NTUEE      = 6'd4;
   parameter LULU       = 6'd3;
   parameter RAIN       = 6'd2;
   parameter ARROW      = 6'd6;
   parameter DOG        = 6'd5;
   parameter DYM_CUBE   = 6'd12;
   parameter MAN        = 6'd8;
   parameter TEST       = 6'd9;
   //   effect
   parameter BLING      = 6'd10;
   parameter SATURATE   = 6'd11;
   parameter CIRCLE     = 6'd7;

   parameter CUSTOM     = 6'd16;
   // Declare default speed counter
   parameter [7:0] VERY_FAST     =  8'd2;
   parameter [7:0] FAST          =  8'd4;
   parameter [7:0] GENERAL       =  8'd8;
   parameter [7:0] SLOW          =  8'd12;
   parameter [7:0] VERY_SLOW     =  8'd16;


   reg [511:0]     buffer_r, buffer_w, sat_buffer_w, sat_buffer_r;
   reg [6:0]       global_counter_r, global_counter_w;
   reg [8:0]       test_counter_r, test_counter_w;

   // for man & cube
   //   Man parts
   parameter BODY= 2'b00, HEAD = 2'b01, HAND = 2'b10, LEG = 2'b11; 
   //   Motion states
   parameter NORMAL = 3'b001, FORWARD = 3'b010, BACKWARD = 3'b000;
   reg [3:0] cube_counter_r, cube_counter_w;
   reg [2:0] perspect_counter_r, perspect_counter_w;
   reg [2:0] motion_counter_r, motion_counter_w;
   reg shrink_r, shrink_w;

   
   assign o_data_moder = buffer_r;
   task assign_value_perspect; //can set led[x,y,z]=value
      input [2:0] x, y, z; 
      input value;
      input [2:0] perspect;
      begin 
         case(perspect)
            3'b000:buffer_w[64*z + 8*y+ x] = value;
            3'b001:buffer_w[64*(7-z)+ 8*y+ x] = value;
            3'b011:buffer_w[64*(7-z)+ 8*(7-y)+ x] = value;
            3'b111:buffer_w[64*(7-z)+ 8*(7-y)+ (7-x)] = value;
            3'b101:buffer_w[64*(7-z)+ 8*y+ (7-x)] = value;
            3'b110:buffer_w[64*z+ 8*(7-y)+ (7-x)] = value;
            3'b100:buffer_w[64*z+ 8*y+ (7-x)] = value;
            3'b010:buffer_w[64*z+ 8*(7-y)+ x] = value;
         endcase
      end
   endtask  

   task assign_value; //can set led[x,y,z]=value
      input    [2:0] x, y, z; 
      input          value;
      begin 
         buffer_w[64*z + 8*y + x] = value;
      end
   endtask 
   function ask; //can know the value led[x,y,z]
      input    [2:0] x, y, z; 
      begin 
         ask = (buffer_r[64*z + 8*y + x] == 1);
      end
   endfunction 
   function [7:0] current_speed;
      input    [7:0] speed_base;
      begin 
			case(speed_base)
				GENERAL: begin
					if(i_speed == 2) begin
						current_speed = GENERAL;
					end else if(i_speed == 3) begin
						current_speed = GENERAL >> 1;
					end else if(i_speed == 4) begin
						current_speed = GENERAL >> 2;
					end else if((i_speed) == 1) begin
						current_speed = GENERAL << 1;
					end else if((i_speed) == 0) begin
						current_speed = GENERAL << 2;
					end else begin
						current_speed = GENERAL;
					end
				end
				FAST: begin
					if(i_speed == 2) begin
						current_speed = FAST;
					end else if(i_speed == 3) begin
						current_speed = FAST >> 1;
					end else if(i_speed == 4) begin
						current_speed = FAST >> 2;
					end else if((i_speed) == 1) begin
						current_speed = FAST << 1;
					end else if((i_speed) == 0) begin
						current_speed = FAST << 2;
					end else begin
						current_speed = FAST;
					end
				end
				VERY_FAST: begin
					if(i_speed == 2) begin
						current_speed = VERY_FAST;
					end else if(i_speed == 3) begin
						current_speed = VERY_FAST >> 1;
					end else if(i_speed == 4) begin
						current_speed = VERY_FAST >> 2;
					end else if((i_speed) == 1) begin
						current_speed = VERY_FAST << 1;
					end else if((i_speed) == 0) begin
						current_speed = VERY_FAST << 2;
					end else begin
						current_speed = VERY_FAST;
					end
				end
				SLOW: begin
					if(i_speed == 2) begin
						current_speed = SLOW;
					end else if(i_speed == 3) begin
						current_speed = SLOW >> 1;
					end else if(i_speed == 4) begin
						current_speed = SLOW >> 2;
					end else if((i_speed) == 1) begin
						current_speed = SLOW << 1;
					end else if((i_speed) == 0) begin
						current_speed = SLOW << 2;
					end else begin
						current_speed = SLOW;
					end
				end
				VERY_SLOW: begin
					if(i_speed == 2) begin
						current_speed = VERY_SLOW;
					end else if(i_speed == 3) begin
						current_speed = VERY_SLOW >> 1;
					end else if(i_speed == 4) begin
						current_speed = VERY_SLOW >> 2;
					end else if((i_speed) == 1) begin
						current_speed = VERY_SLOW << 1;
					end else if((i_speed) == 0) begin
						current_speed = VERY_SLOW << 2;
					end else begin
						current_speed = VERY_SLOW;
					end
				end
				default: begin
					current_speed = speed_base;
				end
			endcase
      end
   endfunction
   task cube_assign;
      input [3:0] size;
      input [2:0] perspect;
      begin
         case(size)
            4'b0: begin
               for(i = 0; i < 1; i=i+1)begin
                  assign_value_perspect(0, i, 0, 1, perspect);
                  assign_value_perspect(i, 0, 0, 1, perspect);
                  assign_value_perspect(0, 0, i, 1, perspect);
                  assign_value_perspect(0, 0, i, 1, perspect);
                  assign_value_perspect(0, i, 0, 1, perspect);
                  assign_value_perspect(0, 0, i, 1, perspect);
                  assign_value_perspect(i, 0, 0, 1, perspect);
                  assign_value_perspect(i, 0, 0, 1, perspect);
                  assign_value_perspect(0, i, 0, 1, perspect);
                  assign_value_perspect(i, 0, 0, 1, perspect);
                  assign_value_perspect(0, i, 0, 1, perspect);
                  assign_value_perspect(0, 0, i, 1, perspect);
               end
            end
            4'd1: begin
               for(i = 0; i < 2; i=i+1)begin
                  assign_value_perspect(0, i, 0, 1, perspect);
                  assign_value_perspect(i, 0, 0, 1, perspect);
                  assign_value_perspect(0, 0, i, 1, perspect);
                  assign_value_perspect(1, 0, i, 1, perspect);
                  assign_value_perspect(1, i, 0, 1, perspect);
                  assign_value_perspect(0, 1, i, 1, perspect);
                  assign_value_perspect(i, 1, 0, 1, perspect);
                  assign_value_perspect(i, 0, 1, 1, perspect);
                  assign_value_perspect(0, i, 1, 1, perspect);
                  assign_value_perspect(i, 1, 1, 1, perspect);
                  assign_value_perspect(1, i, 1, 1, perspect);
                  assign_value_perspect(1, 1, i, 1, perspect);
               end
            end
            4'd2: begin
               for(i = 0; i < 3; i=i+1)begin
                  assign_value_perspect(0, i, 0, 1, perspect);
                  assign_value_perspect(i, 0, 0, 1, perspect);
                  assign_value_perspect(0, 0, i, 1, perspect);
                  assign_value_perspect(2, 0, i, 1, perspect);
                  assign_value_perspect(2, i, 0, 1, perspect);
                  assign_value_perspect(0, 2, i, 1, perspect);
                  assign_value_perspect(i, 2, 0, 1, perspect);
                  assign_value_perspect(i, 0, 2, 1, perspect);
                  assign_value_perspect(0, i, 2, 1, perspect);
                  assign_value_perspect(i, 2, 2, 1, perspect);
                  assign_value_perspect(2, i, 2, 1, perspect);
                  assign_value_perspect(2, 2, i, 1, perspect);
               end
            end
            4'd3: begin
               for(i = 0; i < 4; i=i+1)begin
                  assign_value_perspect(0, i, 0, 1, perspect);
                  assign_value_perspect(i, 0, 0, 1, perspect);
                  assign_value_perspect(0, 0, i, 1, perspect);
                  assign_value_perspect(3, 0, i, 1, perspect);
                  assign_value_perspect(3, i, 0, 1, perspect);
                  assign_value_perspect(0, 3, i, 1, perspect);
                  assign_value_perspect(i, 3, 0, 1, perspect);
                  assign_value_perspect(i, 0, 3, 1, perspect);
                  assign_value_perspect(0, i, 3, 1, perspect);
                  assign_value_perspect(i, 3, 3, 1, perspect);
                  assign_value_perspect(3, i, 3, 1, perspect);
                  assign_value_perspect(3, 3, i, 1, perspect);
               end
            end
            4'd4: begin
               for(i = 0; i < 5; i=i+1)begin
                  assign_value_perspect(0, i, 0, 1, perspect);
                  assign_value_perspect(i, 0, 0, 1, perspect);
                  assign_value_perspect(0, 0, i, 1, perspect);
                  assign_value_perspect(4, 0, i, 1, perspect);
                  assign_value_perspect(4, i, 0, 1, perspect);
                  assign_value_perspect(0, 4, i, 1, perspect);
                  assign_value_perspect(i, 4, 0, 1, perspect);
                  assign_value_perspect(i, 0, 4, 1, perspect);
                  assign_value_perspect(0, i, 4, 1, perspect);
                  assign_value_perspect(i, 4, 4, 1, perspect);
                  assign_value_perspect(4, i, 4, 1, perspect);
                  assign_value_perspect(4, 4, i, 1, perspect);
               end
            end
            4'd5: begin
               for(i = 0; i < 6; i=i+1)begin
                  assign_value_perspect(0, i, 0, 1, perspect);
                  assign_value_perspect(i, 0, 0, 1, perspect);
                  assign_value_perspect(0, 0, i, 1, perspect);
                  assign_value_perspect(5, 0, i, 1, perspect);
                  assign_value_perspect(5, i, 0, 1, perspect);
                  assign_value_perspect(0, 5, i, 1, perspect);
                  assign_value_perspect(i, 5, 0, 1, perspect);
                  assign_value_perspect(i, 0, 5, 1, perspect);
                  assign_value_perspect(0, i, 5, 1, perspect);
                  assign_value_perspect(i, 5, size, 1, perspect);
                  assign_value_perspect(5, i, size, 1, perspect);
                  assign_value_perspect(5, size, i, 1, perspect);
               end
            end
            4'd6: begin
               for(i = 0; i < 7; i=i+1)begin
                  assign_value_perspect(0, i, 0, 1, perspect);
                  assign_value_perspect(i, 0, 0, 1, perspect);
                  assign_value_perspect(0, 0, i, 1, perspect);
                  assign_value_perspect(6, 0, i, 1, perspect);
                  assign_value_perspect(6, i, 0, 1, perspect);
                  assign_value_perspect(0, 6, i, 1, perspect);
                  assign_value_perspect(i, 6, 0, 1, perspect);
                  assign_value_perspect(i, 0, 6, 1, perspect);
                  assign_value_perspect(0, i, 6, 1, perspect);
                  assign_value_perspect(i, 6, 6, 1, perspect);
                  assign_value_perspect(6, i, 6, 1, perspect);
                  assign_value_perspect(6, 6, i, 1, perspect);
               end
            end
            4'd7: begin
               for(i = 0; i < 8; i=i+1)begin
                  assign_value_perspect(0, i, 0, 1, perspect);
                  assign_value_perspect(i, 0, 0, 1, perspect);
                  assign_value_perspect(0, 0, i, 1, perspect);
                  assign_value_perspect(7, 0, i, 1, perspect);
                  assign_value_perspect(7, i, 0, 1, perspect);
                  assign_value_perspect(0, 7, i, 1, perspect);
                  assign_value_perspect(i, 7, 0, 1, perspect);
                  assign_value_perspect(i, 0, 7, 1, perspect);
                  assign_value_perspect(0, i, 7, 1, perspect);
                  assign_value_perspect(i, 7, 7, 1, perspect);
                  assign_value_perspect(7, i, 7, 1, perspect);
                  assign_value_perspect(7, 7, i, 1, perspect);
               end
            end
            default: begin
               buffer_w = 512'b0;
            end
         endcase
      end
   endtask
   task ntuee_assign;
      begin
         assign_value_perspect(0,6,2,1,2);
         assign_value_perspect(0,6,6,1,2);
         assign_value_perspect(0,7,2,1,2);
         assign_value_perspect(0,7,4,1,2);
         assign_value_perspect(0,7,6,1,2);
         //x=0
         assign_value_perspect(7,0,2,1,2);
         assign_value_perspect(7,0,3,1,2);
         assign_value_perspect(7,0,4,1,2);
         assign_value_perspect(7,0,5,1,2);
         assign_value_perspect(7,0,6,1,2);
         assign_value_perspect(7,1,6,1,2);
         assign_value_perspect(7,2,6,1,2);
         assign_value_perspect(7,4,3,1,2);
         assign_value_perspect(7,4,4,1,2);
         assign_value_perspect(7,4,5,1,2);
         assign_value_perspect(7,4,6,1,2);
         assign_value_perspect(7,5,2,1,2);
         assign_value_perspect(7,6,2,1,2);
         assign_value_perspect(7,7,3,1,2);
         assign_value_perspect(7,7,4,1,2);
         assign_value_perspect(7,7,5,1,2);
         assign_value_perspect(7,7,6,1,2);
         //x=7
         assign_value_perspect(0,0,2,1,2);
         assign_value_perspect(0,0,3,1,2);
         assign_value_perspect(0,0,4,1,2);
         assign_value_perspect(0,0,5,1,2);
         assign_value_perspect(0,0,6,1,2);
         assign_value_perspect(1,0,4,1,2);
         assign_value_perspect(1,0,5,1,2);
         assign_value_perspect(2,0,2,1,2);
         assign_value_perspect(2,0,3,1,2);
         assign_value_perspect(3,0,2,1,2);
         assign_value_perspect(3,0,3,1,2);
         assign_value_perspect(3,0,4,1,2);
         assign_value_perspect(3,0,5,1,2);
         assign_value_perspect(3,0,6,1,2);
         assign_value_perspect(5,0,6,1,2);
         assign_value_perspect(6,0,6,1,2);
         //y=0
         assign_value_perspect(1,7,2,1,2);
         assign_value_perspect(1,7,3,1,2);
         assign_value_perspect(1,7,4,1,2);
         assign_value_perspect(1,7,5,1,2);
         assign_value_perspect(1,7,6,1,2);
         assign_value_perspect(2,7,2,1,2);
         assign_value_perspect(2,7,3,1,2);
         assign_value_perspect(3,7,2,1,2);
         assign_value_perspect(3,7,6,1,2);
         assign_value_perspect(4,7,2,1,2);
         assign_value_perspect(4,7,4,1,2);
         assign_value_perspect(4,7,6,1,2);
         assign_value_perspect(5,7,2,1,2);
         assign_value_perspect(5,7,3,1,2);
         assign_value_perspect(5,7,4,1,2);
         assign_value_perspect(5,7,5,1,2);
         assign_value_perspect(5,7,6,1,2);
         //y=7
      end
   endtask
   task arrow_assign;
      begin
         assign_value(0,0,3,1);
         assign_value(0,0,4,1);
         assign_value(1,0,3,1);
         assign_value(1,0,4,1);
         assign_value(2,0,3,1);
         assign_value(2,0,4,1);
         assign_value(3,0,3,1);
         assign_value(3,0,4,1);
         assign_value(4,0,3,1);
         assign_value(4,0,4,1);
         assign_value(5,0,1,1);
         assign_value(5,0,2,1);
         assign_value(5,0,3,1);
         assign_value(5,0,4,1);
         assign_value(5,0,5,1);
         assign_value(5,0,6,1);
         assign_value(6,0,2,1);
         assign_value(6,0,3,1);
         assign_value(6,0,4,1);
         assign_value(6,0,5,1);
         assign_value(7,0,3,1);
         assign_value(7,0,4,1);
      end
   endtask
   task dog_assign;
      begin
         for(i=2; i<6; i=i+1)begin
            assign_value(i,0,4,1);
            assign_value(i,1,4,1);
            assign_value(i,1,5,1);
            assign_value(i,1,6,1);
            assign_value(i,1,7,1);
            assign_value(i,2,0,1);
            assign_value(i,2,1,1);
            assign_value(i,2,2,1);
            assign_value(i,2,3,1);
            assign_value(i,2,4,1);
            assign_value(i,2,5,1);
            assign_value(i,2,6,1);
            assign_value(i,2,7,1);
            assign_value(i,3,2,1);
            assign_value(i,3,3,1);
            assign_value(i,4,2,1);
            assign_value(i,4,3,1);
            assign_value(i,5,2,1);
            assign_value(i,5,3,1);
            assign_value(i,6,0,1);
            assign_value(i,6,1,1);
            assign_value(i,6,2,1);
            assign_value(i,6,3,1);
            assign_value(i,7,3,1);
            assign_value(i,7,4,1);
            assign_value(i,7,5,1);
         end
      end
   endtask
   
   task man_assign;
      input [1:0] man_parts;
      input [2:0] motion_state;
      begin
         case(man_parts)
            BODY: begin
               // left body
               assign_value(2+2, 3, 2, 1);
               assign_value(2+2, 3, 3, 1);
               assign_value(2+2, 3, 4, 1);
               assign_value(2+2, 3, 5, 1);
               assign_value(3+2, 3, 2, 1);
               assign_value(3+2, 3, 3, 1);
               assign_value(3+2, 3, 4, 1);
               assign_value(3+2, 3, 5, 1);
               // right body
               assign_value(2+2, 4, 2, 1);
               assign_value(2+2, 4, 3, 1);
               assign_value(2+2, 4, 4, 1);
               assign_value(2+2, 4, 5, 1);
               assign_value(3+2, 4, 2, 1);
               assign_value(3+2, 4, 3, 1);
               assign_value(3+2, 4, 4, 1);
               assign_value(3+2, 4, 5, 1);
            end
            HEAD: begin
               // ha+2t
               assign_value(0+2, 3, 7, 1);
               assign_value(0+2, 4, 7, 1);
               // left brain
               assign_value(1+2, 3, 6, 1);
               assign_value(1+2, 3, 7, 1);
               assign_value(2+2, 3, 6, 1);
               assign_value(2+2, 3, 7, 1);
               assign_value(3+2, 3, 6, 1);
               assign_value(3+2, 3, 7, 1);
               // right brain
               assign_value(1+2, 4, 6, 1);
               assign_value(1+2, 4, 7, 1);
               assign_value(2+2, 4, 6, 1);
               assign_value(2+2, 4, 7, 1);
               assign_value(3+2, 4, 6, 1);
               assign_value(3+2, 4, 7, 1);
            end
            HAND: begin
               case(motion_state) 
                  NORMAL: begin
                     // left hand
                     assign_value(2+2, 1, 3, 1);
                     assign_value(2+2, 1, 4, 1);
                     assign_value(3+2, 1, 3, 1);
                     assign_value(3+2, 1, 4, 1);
                     // left shoulder
                     assign_value(2+2, 2, 5, 1);
                     assign_value(3+2, 2, 5, 1);
                     // right hand
                     assign_value(2+2, 6, 3, 1);
                     assign_value(2+2, 6, 4, 1);
                     assign_value(3+2, 6, 3, 1);
                     assign_value(3+2, 6, 4, 1);
                     // right shoulder
                     assign_value(2+2, 5, 5, 1);
                     assign_value(3+2, 5, 5, 1);
                  end
                  FORWARD: begin
                     // left hand
                     assign_value(2+2, 1, 5, 1);
                     assign_value(2+2, 1, 4, 1);
                     assign_value(3+2, 1, 4, 1);
                     assign_value(3+2, 1, 3, 1);
                     assign_value(4+2, 1, 3, 1);
                     // left shoulder
                     assign_value(2+2, 2, 5, 1);
                     assign_value(3+2, 2, 5, 1);
                     // right hand
                     assign_value(3+2, 6, 5, 1);
                     assign_value(2+2, 6, 4, 1);
                     assign_value(3+2, 6, 4, 1);
                     assign_value(2+2, 6, 3, 1);
                     assign_value(1+2, 6, 3, 1);
                     // right shoulder
                     assign_value(2+2, 5, 5, 1);
                     assign_value(3+2, 5, 5, 1);
                  end
                  BACKWARD: begin
                     // left hand
                     assign_value(3+2, 1, 5, 1);
                     assign_value(2+2, 1, 4, 1);
                     assign_value(3+2, 1, 4, 1);
                     assign_value(1+2, 1, 3, 1);
                     assign_value(2+2, 1, 3, 1);
                     // left shoulder
                     assign_value(2+2, 2, 5, 1);
                     assign_value(3+2, 2, 5, 1);
                     // right hand
                     assign_value(2+2, 6, 5, 1);
                     assign_value(2+2, 6, 4, 1);
                     assign_value(3+2, 6, 4, 1);
                     assign_value(3+2, 6, 3, 1);
                     assign_value(4+2, 6, 3, 1);
                     // right shoulder
                     assign_value(2+2, 5, 5, 1);
                     assign_value(3+2, 5, 5, 1);
                  end
                  default: begin
                     assign_value(0,0,0,0);
                  end
               endcase
            end
            LEG: begin
               case(motion_state)
                  NORMAL: begin
                     // left leg
                     assign_value(1+2, 2, 0, 1);
                     assign_value(2+2, 2, 0, 1);
                     assign_value(3+2, 2, 0, 1);
                     assign_value(2+2, 2, 1, 1);
                     assign_value(3+2, 2, 1, 1);
                     // right leg
                     assign_value(1+2, 5, 0, 1);
                     assign_value(2+2, 5, 0, 1);
                     assign_value(3+2, 5, 0, 1);
                     assign_value(2+2, 5, 1, 1);
                     assign_value(3+2, 5, 1, 1);
                  end
                  FORWARD: begin
                     // left leg
                     assign_value(1+2, 2, 0, 1);
                     assign_value(2+2, 2, 0, 1);
                     assign_value(0+2, 2, 1, 1);
                     assign_value(1+2, 2, 1, 1);
                     assign_value(2+2, 2, 1, 1);
                     assign_value(3+2, 2, 2, 1);
                     // right leg
                     assign_value(3+2, 5, 0, 1);
                     assign_value(2+2, 5, 1, 1);
                     assign_value(3+2, 5, 1, 1);
                     assign_value(4+2, 5, 1, 1);
                     assign_value(2+2, 5, 2, 1);
                  end
                  BACKWARD: begin
                     // left leg
                     assign_value(3+2, 2, 0, 1);
                     assign_value(2+2, 2, 1, 1);
                     assign_value(3+2, 2, 1, 1);
                     assign_value(4+2, 2, 1, 1);
                     assign_value(2+2, 2, 2, 1);
                     // right leg
                     assign_value(1+2, 5, 0, 1);
                     assign_value(2+2, 5, 0, 1);
                     assign_value(0+2, 5, 1, 1);
                     assign_value(1+2, 5, 1, 1);
                     assign_value(2+2, 5, 1, 1);
                     assign_value(3+2, 5, 2, 1);
                  end
               default: begin
                  buffer_w = 512'b0;
               end
               endcase
            end
         endcase

      end
   endtask
   always@(*) begin
      cube_counter_w = cube_counter_r;
      perspect_counter_w = perspect_counter_r;
      motion_counter_w = motion_counter_r;
      shrink_w = shrink_r;
		global_counter_w = global_counter_r;
		test_counter_w = test_counter_r;
		sat_buffer_w = sat_buffer_r;
      //cube_assign(-1, 0);
      if(i_stop) begin
         buffer_w = buffer_r;
      end else begin
         buffer_w = 512'd0;
         case(i_mode) 
            OFF: buffer_w = {512{1'b0}};
            ON: begin
					buffer_w = {512{1'b1}};
					sat_buffer_w = buffer_r;
				end
            NTUEE: begin
					sat_buffer_w = buffer_r;
               buffer_w = 0;
               ntuee_assign;
            end
            LULU: begin
               buffer_w = buffer_r;
            end
            RAIN: begin
               if(global_counter_r % current_speed(GENERAL) ==0) begin
                  for(i=0; i<448; i=i+1)
                     buffer_w[i] = buffer_r[i+64];
                  for(i=448; i<512; i=i+1) 
                     buffer_w[i] = (i[5:0] == i_random_layer[5:0]) ? 1 : 0;
               end
               else buffer_w = buffer_r;
            end
            ARROW: begin
               buffer_w = {512{1'b0}};
					sat_buffer_w = buffer_r;
               arrow_assign;
            end
            DOG: begin
               buffer_w = {512{1'b0}};
					sat_buffer_w = buffer_r;
               dog_assign;
            end
            DYM_CUBE: begin
               if(global_counter_r%current_speed(FAST) == 0) begin
                  buffer_w = 0;
                  cube_assign(cube_counter_r, perspect_counter_r);
                  if(shrink_r) begin
                     cube_counter_w = cube_counter_r - 1;
                  end else begin
                     cube_counter_w = cube_counter_r + 1;
                  end
               end else begin
                  buffer_w = buffer_r;
                  if(cube_counter_r==0) begin
                     cube_counter_w = cube_counter_r;
                     shrink_w = 0;
                  end else if(cube_counter_r == 8) begin
                     cube_counter_w = cube_counter_r - 1;
                     perspect_counter_w = perspect_counter_r + 1;
                     shrink_w = 1;
                  end
               end
            end
            MAN: begin
					sat_buffer_w = buffer_r;
               if(global_counter_r%current_speed(GENERAL) == 0) begin
                  buffer_w = 0;
                  man_assign(BODY, 0);
                  man_assign(HEAD, 0);
                  man_assign(HAND, motion_counter_r);
                  man_assign(LEG, motion_counter_r);
                  if(shrink_r) begin
                     motion_counter_w = motion_counter_r- 1;
                  end else begin
                     motion_counter_w = motion_counter_r + 1;
                  end
               end else begin
                  if(motion_counter_r == 3) begin
                     motion_counter_w = motion_counter_r - 2;
                     shrink_w = 1;
                  end else if(motion_counter_r == 0) begin
                     motion_counter_w = motion_counter_r;
                     shrink_w = 0;
                  end
                  buffer_w = buffer_r;
               end
            end
            TEST: begin
               buffer_w = buffer_r;
               buffer_w[test_counter_r] = 1;
            end
            SATURATE: begin
               for(i=0; i<512; i=i+1) 
                  buffer_w[i] = (buffer_r[i] || (sat_buffer_r[i] &&(i == i_random_layer))) ? 1 : 0;
            end
            CIRCLE: begin
               if(global_counter_r % current_speed(FAST) == 0)begin
                  for(i=0; i<7; i=i+1)
                     for(j=0; j<8; j=j+1)
                        assign_value(i+1,0,j,ask(i,0,j));
                  for(i=0; i<7; i=i+1)
                     for(j=0; j<8; j=j+1)
                        assign_value(7,i+1,j,ask(7,i,j));
                  for(i=1; i<8; i=i+1)
                     for(j=0; j<8; j=j+1)
                        assign_value(i-1,7,j,ask(i,7,j));
                  for(i=1; i<8; i=i+1)
                     for(j=0; j<8; j=j+1)
                        assign_value(0,i-1,j,ask(0,i,j));
               end else begin
                  buffer_w = buffer_r;
               end
            end
            default: begin
               buffer_w = buffer_r;
            end
         endcase
      end
   end


   always@(posedge i_ready or negedge i_rst) begin
      if(!i_rst) begin
         buffer_r <= 0;
         global_counter_r <= 0;
         test_counter_r <= 0;
         cube_counter_r <= 0;
         perspect_counter_r <= 0;
         shrink_r <= 0;
         motion_counter_r <= 0;
			sat_buffer_r <= 0;
      end else begin
         buffer_r <= buffer_w;
         global_counter_r <= (i_stop)? global_counter_w:global_counter_w+1;
         test_counter_r <= (i_stop)? test_counter_w:test_counter_w + 1;
         cube_counter_r <= cube_counter_w;
         perspect_counter_r <= perspect_counter_w;
         shrink_r <= shrink_w;
         motion_counter_r <= motion_counter_w;
			sat_buffer_r <= sat_buffer_w;
      end
   end
endmodule

module ledcube(rst, clk, letter, letter_data, moder_data_out, port, latch, shift, gnd, ready);

	input 		     rst;
	input 		     clk;
	input			 letter;
	input  [511:0]   letter_data;
	input  [511:0]   moder_data_out;
	output	 [7:0]	 port;
	output			 latch;
	output			 shift;
	output	 [2:0]	 gnd;
	output			 ready;

	reg		 [7:0] 	 counter;
	reg 	 [7:0]	 port;
	reg				 latch;
	reg				 shift;
	reg		 [2:0]	 gnd;
	reg 			 ready;
	wire    [511:0]  data;
	assign data = (letter) ? letter_data : moder_data_out;

	always@(posedge clk or negedge rst)begin

		if(!rst) begin
			counter <= 8'd0;
			port    <= 8'd0;
			latch   <= 0;
			shift   <= 0;
			gnd     <= 3'd0;
			ready   <= 0;
		end
		else begin
			counter <= (counter == 8'd136) ? 8'd0 :    
					   counter + 1;
			case(counter)
				8'd0:begin
					port <= data[7:0];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd1:begin
					port <= data[7:0];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd2:begin
					port <= data[15:8];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd3:begin
					port <= data[15:8];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd4:begin
					port <= data[23:16];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd5:begin
					port <= data[23:16];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd6:begin
					port <= data[31:24];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd7:begin
					port <= data[31:24];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd8:begin
					port <= data[39:32];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd9:begin
					port <= data[39:32];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd10:begin
					port <= data[47:40];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd11:begin
					port <= data[47:40];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd12:begin
					port <= data[55:48];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd13:begin
					port <= data[55:48];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd14:begin
					port <= data[63:56];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd15:begin
					port <= data[63:56];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd16:begin
					port <= data[63:56];
					latch <= 1;
					shift <= 0;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd17:begin
					port <= data[71:64];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd18:begin
					port <= data[71:64];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd19:begin
					port <= data[79:72];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd20:begin
					port <= data[79:72];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd21:begin
					port <= data[87:80];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd22:begin
					port <= data[87:80];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd23:begin
					port <= data[95:88];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd24:begin
					port <= data[95:88];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd25:begin
					port <= data[103:96];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd26:begin
					port <= data[103:96];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd27:begin
					port <= data[111:104];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd28:begin
					port <= data[111:104];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd29:begin
					port <= data[119:112];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd30:begin
					port <= data[119:112];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd31:begin
					port <= data[127:120];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd32:begin
					port <= data[127:120];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd0;
					ready <= 0;
				end
				8'd33:begin
					port <= data[127:120];
					latch <= 1;
					shift <= 0;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd34:begin
					port <= data[135:128];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd35:begin
					port <= data[135:128];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd36:begin
					port <= data[143:136];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd37:begin
					port <= data[143:136];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd38:begin
					port <= data[151:144];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd39:begin
					port <= data[151:144];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd40:begin
					port <= data[159:152];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd41:begin
					port <= data[159:152];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd42:begin
					port <= data[167:160];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd43:begin
					port <= data[167:160];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd44:begin
					port <= data[175:168];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd45:begin
					port <= data[175:168];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd46:begin
					port <= data[183:176];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd47:begin
					port <= data[183:176];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd48:begin
					port <= data[191:184];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd49:begin
					port <= data[191:184];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd1;
					ready <= 0;
				end
				8'd50:begin
					port <= data[191:184];
					latch <= 1;
					shift <= 0;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd51:begin
					port <= data[199:192];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd52:begin
					port <= data[199:192];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd53:begin
					port <= data[207:200];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd54:begin
					port <= data[207:200];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd55:begin
					port <= data[215:208];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd56:begin
					port <= data[215:208];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd57:begin
					port <= data[223:216];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd58:begin
					port <= data[223:216];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd59:begin
					port <= data[231:224];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd60:begin
					port <= data[231:224];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd61:begin
					port <= data[239:232];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd62:begin
					port <= data[239:232];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd63:begin
					port <= data[247:240];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd64:begin
					port <= data[247:240];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd65:begin
					port <= data[255:248];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd66:begin
					port <= data[255:248];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd2;
					ready <= 0;
				end
				8'd67:begin
					port <= data[255:248];
					latch <= 1;
					shift <= 0;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd68:begin
					port <= data[263:256];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd69:begin
					port <= data[263:256];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd70:begin
					port <= data[271:264];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd71:begin
					port <= data[271:264];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd72:begin
					port <= data[279:272];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd73:begin
					port <= data[279:272];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd74:begin
					port <= data[287:280];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd75:begin
					port <= data[287:280];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd76:begin
					port <= data[295:288];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd77:begin
					port <= data[295:288];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd78:begin
					port <= data[303:296];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd79:begin
					port <= data[303:296];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd80:begin
					port <= data[311:304];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd81:begin
					port <= data[311:304];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd82:begin
					port <= data[319:312];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd83:begin
					port <= data[319:312];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd3;
					ready <= 0;
				end
				8'd84:begin
					port <= data[319:312];
					latch <= 1;
					shift <= 0;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd85:begin
					port <= data[327:320];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd86:begin
					port <= data[327:320];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd87:begin
					port <= data[335:328];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd88:begin
					port <= data[335:328];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd89:begin
					port <= data[343:336];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd90:begin
					port <= data[343:336];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd91:begin
					port <= data[351:344];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd92:begin
					port <= data[351:344];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd93:begin
					port <= data[359:352];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd94:begin
					port <= data[359:352];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd95:begin
					port <= data[367:360];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd96:begin
					port <= data[367:360];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd97:begin
					port <= data[375:368];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd98:begin
					port <= data[375:368];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd99:begin
					port <= data[383:376];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd100:begin
					port <= data[383:376];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd4;
					ready <= 0;
				end
				8'd101:begin
					port <= data[383:376];
					latch <= 1;
					shift <= 0;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd102:begin
					port <= data[391:384];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd103:begin
					port <= data[391:384];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd104:begin
					port <= data[399:392];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd105:begin
					port <= data[399:392];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd106:begin
					port <= data[407:400];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd107:begin
					port <= data[407:400];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd108:begin
					port <= data[415:408];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd109:begin
					port <= data[415:408];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd110:begin
					port <= data[423:416];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd111:begin
					port <= data[423:416];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd112:begin
					port <= data[431:424];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd113:begin
					port <= data[431:424];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd114:begin
					port <= data[439:432];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd115:begin
					port <= data[439:432];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd116:begin
					port <= data[447:440];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd117:begin
					port <= data[447:440];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd5;
					ready <= 0;
				end
				8'd118:begin
					port <= data[447:440];
					latch <= 1;
					shift <= 0;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd119:begin
					port <= data[455:448];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd120:begin
					port <= data[455:448];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd121:begin
					port <= data[463:456];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd122:begin
					port <= data[463:456];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd123:begin
					port <= data[471:464];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd124:begin
					port <= data[471:464];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd125:begin
					port <= data[479:472];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd126:begin
					port <= data[479:472];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd127:begin
					port <= data[487:480];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd128:begin
					port <= data[487:480];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd129:begin
					port <= data[495:488];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd130:begin
					port <= data[495:488];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd131:begin
					port <= data[503:496];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd132:begin
					port <= data[503:496];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd133:begin
					port <= data[511:504];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd134:begin
					port <= data[511:504];
					latch <= 0;
					shift <= 1;
					gnd <= 3'd6;
					ready <= 0;
				end
				8'd135:begin
					port <= data[511:504];
					latch <= 1;
					shift <= 0;
					gnd <= 3'd7;
					ready <= 0;
				end
				8'd136:begin
					port <= data[511:504];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd7;
					ready <= 1;
				end
				default:begin
					port <= data[7:0];
					latch <= 0;
					shift <= 0;
					gnd <= 3'd7;
					ready <= 0;
				end
			endcase
		end
	end

endmodule

module clk_divider(clk,rst,clk_out);
	input 			clk;
	input			rst;
	output 			clk_out;

	reg 	[27:0] 	counter; 	
	reg 			clk_out;

always @(posedge clk or negedge rst)begin

	if(!rst) begin
		counter <= 28'd0;	
		clk_out <= 1'b0;
	end
	// originally 25000000
	// When it starts to look static: 150000
	else if(counter == 28'd3500) begin
		counter <= 28'd0;
		clk_out <= ~clk_out;
	end
	else begin
		counter <= counter + 1;
		clk_out <= clk_out;
	end
end

endmodule

module lfsr(i_Clk, rst, o_LFSR_Data);
  input            i_Clk;
  input            rst;
  output     [8:0] o_LFSR_Data;
  reg        [8:0] r_LFSR;
  reg              r_XNOR;

  always @(posedge i_Clk or negedge rst)begin
      if(!rst) r_LFSR <= 9'd1;
      else r_LFSR <= {r_LFSR[7:0], r_XNOR};
  end

  always @(*)begin
      r_XNOR = r_LFSR[8] ^~ r_LFSR[4];
  end
 
 
  assign o_LFSR_Data = r_LFSR[8:0];

endmodule

module Debounce(
	input i_in,
	input i_rst,
	input i_clk,
	output o_neg
);
	parameter CNT_N = 7;
	localparam CNT_BIT = 3;

	reg o_debounced_r, o_debounced_w;
	reg [CNT_BIT-1:0] counter_r, counter_w;
	reg neg_r, neg_w;

	assign o_debounced = o_debounced_r;
	assign o_neg = neg_r;

	always@(*)begin
		if (i_in != o_debounced_r) begin
			counter_w = counter_r - 1;
		end else begin
			counter_w = CNT_N;
		end
		if (counter_r == 0) begin
			o_debounced_w = ~o_debounced_r;
		end else begin
			o_debounced_w = o_debounced_r;
		end
		neg_w =  o_debounced_r & ~o_debounced_w;
	end

	always@(posedge i_clk) begin
		if (!i_rst) begin
			o_debounced_r <= 0;
			counter_r <= 0;
			neg_r <= 0;
		end else begin
			o_debounced_r <= o_debounced_w;
			counter_r <= counter_w;
			neg_r <= neg_w;
		end
	end
endmodule

module letter(ready,rst,letter, set, start, data);

   input                rst;
   input                ready;
   input        [5:0]   letter;
   input                set;
   input                start;
   output     [511:0]   data;
   //mode
   parameter A       = 6'd37;//set all led off
   parameter B       = 6'd1;//set all led on and off
   parameter C       = 6'd2;//random light all ledcube
   parameter D       = 6'd3;//transfer some letter pattern from back to front
   parameter E       = 6'd4;//random light top led and then rain 
   parameter F       = 6'd5;//NTUEE around cube
   parameter G       = 6'd6;//arrow around cube
   parameter H       = 6'd7;//circulate with higher speed
   parameter I       = 6'd8;//sequential light all led cube
   parameter J       = 6'd9;//set all led on
   parameter K       = 6'd10;//dog in cube
   parameter L       = 6'd11;//set all led on and off
   parameter M       = 6'd12;//random light all ledcube
   parameter N       = 6'd13;//transfer some letter pattern from back to front
   parameter O       = 6'd14;//random light top led and then rain 
   parameter P       = 6'd15;//NTUEE around cube
   parameter Q       = 6'd16;//arrow around cube
   parameter R       = 6'd17;//circulate with higher speed
   parameter S       = 6'd18;//sequential light all led cube
   parameter T       = 6'd19;//set all led on
   parameter U       = 6'd20;//dog in cube
   parameter V       = 6'd21;//set all led on and off
   parameter W       = 6'd22;//random light all ledcube
   parameter X       = 6'd23;//transfer some letter pattern from back to front
   parameter Y       = 6'd24;//random light top led and then rain 
   parameter Z       = 6'd25;//NTUEE around cube
   parameter Love    = 6'd26;
   parameter n_0     = 6'd27;//NTUEE around cube
   parameter n_1     = 6'd28;
   parameter n_2     = 6'd29;
   parameter n_3     = 6'd30;
   parameter n_4     = 6'd31;
   parameter n_5     = 6'd32;
   parameter n_6     = 6'd33;
   parameter n_7     = 6'd34;
   parameter n_8     = 6'd35;
   parameter n_9     = 6'd36;
   parameter idle    = 0;
   parameter setting = 2'd1;
   parameter save    = 2'd2;
   parameter print   = 2'd3;


   reg   [1:0]    state;
   reg   [1:0]    state_w;
   reg   [2:0]    counter;
   reg   [2:0]    counter_w;
   //counter count from 0 to 7 recursively
   reg   [2:0]    counter3;
   reg   [2:0]    counter3_w;
   //counter3 count from 7 to 0 when counter==7 8 cycles
   reg   [2:0]    counter2;
   reg   [2:0]    counter2_w;
   //counter2 count the 10 words
   reg   [5:0]    sentence_w[7:0];
   reg   [5:0]    sentence[7:0] ; 
   reg   [511:0]  buffer_r, buffer_w;
   integer        i,j,k;
   reg   [5:0]    display;
   reg   [5:0]    display_w;
   assign   data =   buffer_r;
task assign_value; //can set led[x,y,z]=value
   input    [2:0] x, y, z; 
   input         value;

   begin 
	  buffer_w[64*z + 8*y + x] = value;
   end
endtask 


always @(*)begin
	state_w  = state;
	display_w = display;
	for(i=0; i < 8; i = i+1) sentence_w[i] = sentence[i];
	//sentence_w[0]=6'd0;
	counter3_w = (counter  ==   7) ? counter3 - 1 : counter3;
	counter_w  = (counter  ==   7) ? 3'd0         : counter  + 1;
	counter2_w = counter2;
   case(state)
		 setting:begin
			if(set)begin
				sentence_w[7]=letter;
				for(i=0;i<7;i=i+1)   sentence_w[i]=sentence[i+1];
				display_w      = letter;
				counter3_w     = 7; 
				counter2_w     = 0;
			end else if(start)begin
			   j=0;
			   for(i=0;i<8;i=i+1)begin
			    	if(sentence[i]!=6'd0)begin
			    		sentence_w[j]=sentence[i];
			    		j=j+1;
					end
				end
				
				for(i=0;i<8;i=i+1)begin
					sentence_w[i]=(i >= j)?6'd0:sentence_w[i];
				end
			   
			   display_w      = sentence_w[0];
			   state_w        = print;
			   counter2_w     = 0;
			   counter3_w     = 7;
			   counter_w      = 0;
				  
			end else  begin
			   display_w      = letter;
			   state_w        =  setting;
			   counter3_w     =  7; 
			   counter2_w     =  0;
			end
		 end
		 print:begin
			   display_w   = sentence[counter2_w];
			   counter2_w  = (counter3 == 0 && counter == 7) ? counter2+1 :counter2;
			   state_w     = (display_w == 6'd0)?setting
			   									:(counter2 == 7 &&counter3 == 0 && counter == 7) ? setting    :print;
		 end
		 default:begin
			   counter2_w = 0;
			   counter3_w = 7; 
		 end
   endcase
end
always@(*)begin
	buffer_w = {512{1'b0}};
   case(display)

	  A:begin
		 assign_value(counter3[2:0],1,0,1);
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],1,3,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],1,4,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],3,2,1);
		 assign_value(counter3[2:0],4,2,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],5,7,1);
	  end
	  B:begin
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],1,3,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
	  end
	  C:begin
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 
	  end
	  D:begin
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],1,3,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],1,4,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
	  end
	  E:begin
		 assign_value(counter3[2:0],1,0,1);
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
		 
	  end
	  F:begin
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
	  end
	  G:begin
		 //letter L
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],1,3,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
	  end
	  H:begin
		 assign_value(counter3[2:0],1,0,1);
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],1,3,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],1,4,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
	  end
	  I:begin
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,0,1);
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],3,2,1);
		 assign_value(counter3[2:0],4,2,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],3,5,1);
		 assign_value(counter3[2:0],4,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
	  end
	  J:begin
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],1,3,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],1,4,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
	  end
	  K:begin
		 assign_value(counter3[2:0],1,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],3,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],3,5,1);
		 assign_value(counter3[2:0],4,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
	  end
	  L:begin
		 assign_value(counter3[2:0],1,0,1);
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
	  end
	  M:begin
		 assign_value(counter3[2:0],1,0,1);
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],1,3,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],1,4,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],3,5,1);
		 assign_value(counter3[2:0],4,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],6,7,1);
	  end
	  N:begin
		 assign_value(counter3[2:0],1,0,1);
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],3,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],1,3,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],1,4,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],4,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
	  end 
	  O:begin
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],1,3,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],1,4,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
	  end
	  P:begin
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],1,4,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
	  end
	  R:begin
		 assign_value(counter3[2:0],1,0,1);
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],3,2,1);
		 assign_value(counter3[2:0],4,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],1,4,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
	  end
	  S:begin
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],1,3,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],4,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
	  end
	  T:begin
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],3,2,1);
		 assign_value(counter3[2:0],4,2,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],3,5,1);
		 assign_value(counter3[2:0],4,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
	  end 
	  U:begin
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],1,3,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],1,4,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
   
	  end
	  V:begin
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],1,3,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],6,3,1);
		 assign_value(counter3[2:0],1,4,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
   
	  end
	  W:begin
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],0,2,1);
		 assign_value(counter3[2:0],3,2,1);
		 assign_value(counter3[2:0],4,2,1);
		 assign_value(counter3[2:0],7,2,1);
		 assign_value(counter3[2:0],0,3,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],7,3,1);
		 assign_value(counter3[2:0],0,4,1);
		 assign_value(counter3[2:0],7,4,1);
		 assign_value(counter3[2:0],0,5,1);
		 assign_value(counter3[2:0],7,5,1);
		 assign_value(counter3[2:0],0,6,1);
		 assign_value(counter3[2:0],7,6,1);
		 assign_value(counter3[2:0],0,7,1);
		 assign_value(counter3[2:0],7,7,1);
	  end
	  X:begin
		 assign_value(counter3[2:0],1,0,1);
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],3,2,1);
		 assign_value(counter3[2:0],4,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);

	  end
	  Y:begin
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],3,2,1);
		 assign_value(counter3[2:0],4,2,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],5,3,1);
		 assign_value(counter3[2:0],1,4,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],5,4,1);
		 assign_value(counter3[2:0],6,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],5,5,1);
		 assign_value(counter3[2:0],6,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);
	  end
	  Z:begin
		 assign_value(counter3[2:0],1,0,1);
		 assign_value(counter3[2:0],2,0,1);
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],5,0,1);
		 assign_value(counter3[2:0],6,0,1);
		 assign_value(counter3[2:0],1,1,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],6,1,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],4,2,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],2,4,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],1,5,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],1,6,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],6,6,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,7,1);

	  end
	  Love:begin
		 //letter love
		 assign_value(counter3[2:0],0,3,1);
		 assign_value(counter3[2:0],0,4,1);
		 assign_value(counter3[2:0],0,5,1);
		 assign_value(counter3[2:0],0,6,1);
		 assign_value(counter3[2:0],1,2,1);
		 assign_value(counter3[2:0],1,7,1);
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],2,7,1);
		 assign_value(counter3[2:0],3,0,1);
		 assign_value(counter3[2:0],3,5,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,0,1);
		 assign_value(counter3[2:0],4,5,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],5,7,1);
		 assign_value(counter3[2:0],6,2,1);
		 assign_value(counter3[2:0],6,7,1);
		 assign_value(counter3[2:0],7,3,1);
		 assign_value(counter3[2:0],7,4,1);
		 assign_value(counter3[2:0],7,5,1);
		 assign_value(counter3[2:0],7,6,1);
	  end
	  n_1:begin
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],3,2,1);
		 assign_value(counter3[2:0],3,3,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],3,5,1);
		 assign_value(counter3[2:0],3,6,1);
		 assign_value(counter3[2:0],4,6,1);
		 assign_value(counter3[2:0],3,7,1);
	  end
	  n_2:begin
		 assign_value(counter3[2:0],2,1,1);
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],5,1,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],4,3,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
	  end
	  n_3:begin
		 assign_value(counter3[2:0],3,1,1);
		 assign_value(counter3[2:0],4,1,1);
		 assign_value(counter3[2:0],2,2,1);
		 assign_value(counter3[2:0],5,2,1);
		 assign_value(counter3[2:0],2,3,1);
		 assign_value(counter3[2:0],3,4,1);
		 assign_value(counter3[2:0],4,4,1);
		 assign_value(counter3[2:0],2,5,1);
		 assign_value(counter3[2:0],2,6,1);
		 assign_value(counter3[2:0],5,6,1);
		 assign_value(counter3[2:0],3,7,1);
		 assign_value(counter3[2:0],4,7,1);
	  end
	  n_4:begin
        assign_value(counter3[2:0],3,1,1);
        assign_value(counter3[2:0],3,2,1);
        assign_value(counter3[2:0],3,3,1);
        assign_value(counter3[2:0],2,4,1);
        assign_value(counter3[2:0],3,4,1);
        assign_value(counter3[2:0],4,4,1);
        assign_value(counter3[2:0],5,4,1);
        assign_value(counter3[2:0],6,4,1);
        assign_value(counter3[2:0],3,5,1);
        assign_value(counter3[2:0],6,5,1);
        assign_value(counter3[2:0],3,6,1);
        assign_value(counter3[2:0],6,6,1);
        assign_value(counter3[2:0],3,7,1);
        assign_value(counter3[2:0],6,7,1);
	  end
	  n_5:begin
        assign_value(counter3[2:0],3,1,1);
        assign_value(counter3[2:0],4,1,1);
        assign_value(counter3[2:0],5,1,1);
        assign_value(counter3[2:0],2,2,1);
        assign_value(counter3[2:0],2,3,1);
        assign_value(counter3[2:0],2,4,1);
        assign_value(counter3[2:0],4,4,1);
        assign_value(counter3[2:0],5,4,1);
        assign_value(counter3[2:0],3,4,1);
        assign_value(counter3[2:0],5,5,1);
        assign_value(counter3[2:0],5,6,1);
        assign_value(counter3[2:0],2,7,1);
        assign_value(counter3[2:0],4,7,1);
        assign_value(counter3[2:0],5,7,1);
        assign_value(counter3[2:0],3,7,1);
	  end
	  n_6:begin
        assign_value(counter3[2:0],3,1,1);
        assign_value(counter3[2:0],4,1,1);
        assign_value(counter3[2:0],2,2,1);
        assign_value(counter3[2:0],5,2,1);
        assign_value(counter3[2:0],2,3,1);
        assign_value(counter3[2:0],5,3,1);
        assign_value(counter3[2:0],3,4,1);
        assign_value(counter3[2:0],4,4,1);
        assign_value(counter3[2:0],5,4,1);
        assign_value(counter3[2:0],5,5,1);
        assign_value(counter3[2:0],5,6,1);
        assign_value(counter3[2:0],2,6,1);
        assign_value(counter3[2:0],4,7,1);
        assign_value(counter3[2:0],3,7,1);
	  end
	  n_7:begin
        assign_value(counter3[2:0],5,1,1);
        assign_value(counter3[2:0],5,2,1);
        assign_value(counter3[2:0],4,3,1);
        assign_value(counter3[2:0],3,4,1);
        assign_value(counter3[2:0],2,5,1);
        assign_value(counter3[2:0],2,6,1);
        assign_value(counter3[2:0],2,7,1);
        assign_value(counter3[2:0],4,7,1);
        assign_value(counter3[2:0],5,7,1);
        assign_value(counter3[2:0],3,7,1);
        assign_value(counter3[2:0],6,7,1);
	  end
	  n_8:begin
        assign_value(counter3[2:0],3,1,1);
        assign_value(counter3[2:0],4,1,1);
        assign_value(counter3[2:0],2,2,1);
        assign_value(counter3[2:0],5,2,1);
        assign_value(counter3[2:0],2,3,1);
        assign_value(counter3[2:0],5,3,1);
        assign_value(counter3[2:0],4,4,1);
        assign_value(counter3[2:0],3,4,1);
        assign_value(counter3[2:0],2,5,1);
        assign_value(counter3[2:0],5,5,1);
        assign_value(counter3[2:0],2,6,1);
        assign_value(counter3[2:0],5,6,1);
        assign_value(counter3[2:0],3,7,1);
        assign_value(counter3[2:0],4,7,1);
	  end
	  n_9:begin
        assign_value(counter3[2:0],3,1,1);
        assign_value(counter3[2:0],4,1,1);
        assign_value(counter3[2:0],2,2,1);
        assign_value(counter3[2:0],5,2,1);
        assign_value(counter3[2:0],2,3,1);
        assign_value(counter3[2:0],2,4,1);
        assign_value(counter3[2:0],4,4,1);
        assign_value(counter3[2:0],3,4,1);
        assign_value(counter3[2:0],2,5,1);
        assign_value(counter3[2:0],5,5,1);
        assign_value(counter3[2:0],2,6,1);
        assign_value(counter3[2:0],5,6,1);
        assign_value(counter3[2:0],3,7,1);
        assign_value(counter3[2:0],4,7,1);
	  end
	  n_0:begin
        assign_value(counter3[2:0],3,1,1);
        assign_value(counter3[2:0],4,1,1);
        assign_value(counter3[2:0],2,2,1);
        assign_value(counter3[2:0],5,2,1);
        assign_value(counter3[2:0],2,3,1);
        assign_value(counter3[2:0],2,4,1);
        assign_value(counter3[2:0],4,4,1);
        assign_value(counter3[2:0],5,4,1);
        assign_value(counter3[2:0],2,5,1);
        assign_value(counter3[2:0],3,5,1);
        assign_value(counter3[2:0],5,5,1);
        assign_value(counter3[2:0],2,6,1);
        assign_value(counter3[2:0],5,6,1);
        assign_value(counter3[2:0],3,7,1);
        assign_value(counter3[2:0],4,7,1);
	  end
	  default:begin
		 buffer_w = 512'd0;
	  end
   endcase
end

always @(posedge ready or negedge rst)begin
   if(!rst)begin
	  state <= setting;
	  display <= 6'd0;
	  for(i = 0; i < 8; i = i + 1)
		 sentence[i] <= 6'd0;
	  buffer_r  <= 512'd0;
	  counter   <= 3'd0;
	  counter2  <= 2'd0;
	  counter3  <= 3'd7;
   end
   else begin
	  state <= state_w;
	  display <= display_w;
	  for(i = 0; i < 8; i = i + 1)
		 sentence[i] <= sentence_w[i];
	  buffer_r    <= buffer_w;
	  counter     <= counter_w;
	  counter2    <= counter2_w;
	  counter3    <= counter3_w;
   end
end

endmodule
