//lab_g cam_to_hdmi
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xparameters.h" //
#include "xgpio.h"  //axi gpio pl
#include "xgpiops.h"  //gpio ps
#include "xiicps.h" //iic ps
#include "MIPI_CSI_2_RX.h"
#include "MIPI_D_PHY_RX.h"
#include "xvtc.h"
#include "xaxivdma.h"
#include "xil_cache.h"
#define LED_CHANNEL 1  //lab-c
#define DMA_DEVICE_ID		XPAR_AXIVDMA_0_DEVICE_ID
#define XVTC_DEVICE_ID			XPAR_VTC_0_DEVICE_ID
#define VRAM_ADR 0x30000000
XGpio GpioOutput; /* The driver instance for GPIO Device configured as O/P */ //lab-c
XGpioPs GpioOutputps; /* The driver instance for GPIO Device configured as O/P */ //lab-c
XIicPs Iic;			/* Instance of the IIC Device */
XVtc	VtcInst;
XAxiVdma AxiVdma;


int main()
{
	typedef struct { uint16_t addr; uint8_t data; } adr_data_t;
	int Status; //lab-c
	int i ; //lab-c
	int lp;
	int cnt ; //lab-c
	//gpiops
	XGpioPs_Config *Config_gpiops;
	//iic
	XIicPs_Config *Config_iicps;
	//iic buff
	u8 iic_buffer[4];
	//vtc
	XVtc_Config *Config_vtc;
	//vdma
	XAxiVdma_Config *Config_Vdma;
	//
	adr_data_t const id_rd_cmd[2] ={{0x300a,0},{0x300b,0}};
	adr_data_t const init_cmd [2] ={{0x3103,0x11},{0x3008,0x82}};
	adr_data_t const cfg_init_[] =
    	{
    		//[7]=0 Software reset; [6]=1 Software power down; Default=0x02
    		{0x3008, 0x42},
    		//[1]=1 System input clock from PLL; Default read = 0x11
    		{0x3103, 0x03},
    		//[3:0]=0000 MD2P,MD2N,MCP,MCN input; Default=0x00
    		{0x3017, 0x00},
    		//[7:2]=000000 MD1P,MD1N, D3:0 input; Default=0x00
    		{0x3018, 0x00},
    		//[6:4]=001 PLL charge pump, [3:0]=1000 MIPI 8-bit mode
    		{0x3034, 0x18},

    		//              +----------------+        +------------------+         +---------------------+        +---------------------+
    		//XVCLK         | PRE_DIV0       |        | Mult (4+252)     |         | Sys divider (0=16)  |        | MIPI divider (0=16) |
    		//+-------+-----> 3037[3:0]=0001 +--------> 3036[7:0]=0x38   +---------> 3035[7:4]=0001      +--------> 3035[3:0]=0001      |
    		//12MHz   |     | / 1            | 12MHz  | * 56             | 672MHz  | / 1                 | 672MHz | / 1                 |
    		//        |     +----------------+        +------------------+         +----------+----------+        +----------+----------+
    		//        |                                                                       |                              |
    		//        |                                                                       |                      MIPISCLK|672MHz
    		//        |                                                                       |                              |
    		//        |     +----------------+        +------------------+         +----------v----------+        +----------v----------+
    		//        |     | PRE_DIVSP      |        | R_DIV_SP         |         | PLL R divider       |        | MIPI PHY            | MIPI_CLK
    		//        +-----> 303d[5:4]=01   +--------> 303d[2]=0 (+1)   |         | 3037[4]=1 (+1)      |        |                     +------->
    		//              | / 1.5          |  8MHz  | / 1              |         | / 2                 |        | / 2                 | 336MHz
    		//              +----------------+        +---------+--------+         +----------+----------+        +---------------------+
    		//                                                  |                             |
    		//                                                  |                             |
    		//                                                  |                             |
    		//              +----------------+        +---------v--------+         +----------v----------+        +---------------------+
    		//              | SP divider     |        | Mult             |         | BIT div (MIPI 8/10) |        | SCLK divider        | SCLK
    		//              | 303c[3:0]=0x1  +<-------+ 303b[4:0]=0x19   |         | 3034[3:0]=0x8)      +----+---> 3108[1:0]=01 (2^)   +------->
    		//              | / 1            | 200MHz | * 25             |         | / 2                 |    |   | / 2                 | 84MHz
    		//              +--------+-------+        +------------------+         +----------+----------+    |   +---------------------+
    		//                       |                                                        |               |
    		//                       |                                                        |               |
    		//                       |                                                        |               |
    		//              +--------v-------+                                     +----------v----------+    |   +---------------------+
    		//              | R_SELD5 div    | ADCCLK                              | PCLK div            |    |   | SCLK2x divider      |
    		//              | 303d[1:0]=001  +------->                             | 3108[5:4]=00 (2^)   |    +---> 3108[3:2]=00 (2^)   +------->
    		//              | / 1            | 200MHz                              | / 1                 |        | / 1                 | 168MHz
    		//              +----------------+                                     +----------+----------+        +---------------------+
    		//                                                                                |
    		//                                                                                |
    		//                                                                                |
    		//                                                                     +----------v----------+        +---------------------+
    		//                                                                     | P divider (* #lanes)| PCLK   | Scale divider       |
    		//                                                                     | 3035[3:0]=0001      +--------> 3824[4:0]           |
    		//                                                                     | / 1                 | 168MHz | / 2                 |
    		//                                                                     +---------------------+        +---------------------+

    		//PLL1 configuration
    		//[7:4]=0001 System clock divider /1, [3:0]=0001 Scale divider for MIPI /1
    		{0x3035, 0x11},
    		//[7:0]=56 PLL multiplier
    		{0x3036, 0x38},
    		//[4]=1 PLL root divider /2, [3:0]=1 PLL pre-divider /1
    		{0x3037, 0x11},
    		//[5:4]=00 PCLK root divider /1, [3:2]=00 SCLK2x root divider /1, [1:0]=01 SCLK root divider /2
    		{0x3108, 0x01},
    		//PLL2 configuration
    		//[5:4]=01 PRE_DIV_SP /1.5, [2]=1 R_DIV_SP /1, [1:0]=00 DIV12_SP /1
    		{0x303D, 0x10},
    		//[4:0]=11001 PLL2 multiplier DIV_CNT5B = 25
    		{0x303B, 0x19},

    		{0x3630, 0x2e},
    		{0x3631, 0x0e},
    		{0x3632, 0xe2},
    		{0x3633, 0x23},
    		{0x3621, 0xe0},
    		{0x3704, 0xa0},
    		{0x3703, 0x5a},
    		{0x3715, 0x78},
    		{0x3717, 0x01},
    		{0x370b, 0x60},
    		{0x3705, 0x1a},
    		{0x3905, 0x02},
    		{0x3906, 0x10},
    		{0x3901, 0x0a},
    		{0x3731, 0x02},
    		//VCM debug mode
    		{0x3600, 0x37},
    		{0x3601, 0x33},
    		//System control register changing not recommended
    		{0x302d, 0x60},
    		//??
    		{0x3620, 0x52},
    		{0x371b, 0x20},
    		//?? DVP
    		{0x471c, 0x50},

    		{0x3a13, 0x43},
    		{0x3a18, 0x00},
    		{0x3a19, 0xf8},
    		{0x3635, 0x13},
    		{0x3636, 0x06},
    		{0x3634, 0x44},
    		{0x3622, 0x01},
    		{0x3c01, 0x34},
    		{0x3c04, 0x28},
    		{0x3c05, 0x98},
    		{0x3c06, 0x00},
    		{0x3c07, 0x08},
    		{0x3c08, 0x00},
    		{0x3c09, 0x1c},
    		{0x3c0a, 0x9c},
    		{0x3c0b, 0x40},

    		//[7]=1 color bar enable, [3:2]=00 eight color bar
    		{0x503d, 0x00},
    		//[2]=1 ISP vflip, [1]=1 sensor vflip
    		{0x3820, 0x46},

    		//[7:5]=010 Two lane mode, [4]=0 MIPI HS TX no power down, [3]=0 MIPI LP RX no power down, [2]=1 MIPI enable, [1:0]=10 Debug mode; Default=0x58
    		{0x300e, 0x45},
    		//[5]=0 Clock free running, [4]=1 Send line short packet, [3]=0 Use lane1 as default, [2]=1 MIPI bus LP11 when no packet; Default=0x04
    		{0x4800, 0x14},
    		{0x302e, 0x08},
    		//[7:4]=0x3 YUV422, [3:0]=0x0 YUYV
    		//{0x4300, 0x30},
    		//[7:4]=0x6 RGB565, [3:0]=0x0 {b[4:0],g[5:3],g[2:0],r[4:0]}
    		{0x4300, 0x6f},
    		{0x501f, 0x01},

    		{0x4713, 0x03},
    		{0x4407, 0x04},
    		{0x440e, 0x00},
    		{0x460b, 0x35},
    		//[1]=0 DVP PCLK divider manual control by 0x3824[4:0]
    		{0x460c, 0x20},
    		//[4:0]=1 SCALE_DIV=INT(3824[4:0]/2)
    		{0x3824, 0x01},

    		//MIPI timing
    		//		{0x4805, 0x10}, //LPX global timing select=auto
    		//		{0x4818, 0x00}, //hs_prepare + hs_zero_min ns
    		//		{0x4819, 0x96},
    		//		{0x482A, 0x00}, //hs_prepare + hs_zero_min UI
    		//
    		//		{0x4824, 0x00}, //lpx_p_min ns
    		//		{0x4825, 0x32},
    		//		{0x4830, 0x00}, //lpx_p_min UI
    		//
    		//		{0x4826, 0x00}, //hs_prepare_min ns
    		//		{0x4827, 0x32},
    		//		{0x4831, 0x00}, //hs_prepare_min UI

    		//[7]=1 LENC correction enabled, [5]=1 RAW gamma enabled, [2]=1 Black pixel cancellation enabled, [1]=1 White pixel cancellation enabled, [0]=1 Color interpolation enabled
    		{0x5000, 0x07},
    		//[7]=0 Special digital effects, [5]=0 scaling, [2]=0 UV average disabled, [1]=1 Color matrix enabled, [0]=1 Auto white balance enabled
    		{0x5001, 0x03}
    	};

	//720 60fp
    adr_data_t const cfg_720_60fp_[] =
	{//1280 x 720 binned, RAW10, MIPISCLK=280M, SCLK=56Mz, PCLK=56M
		//PLL1 configuration
		//[7:4]=0010 System clock divider /2, [3:0]=0001 Scale divider for MIPI /1
		{0x3035, 0x21},
		//[7:0]=70 PLL multiplier
		{0x3036, 0x46},
		//[4]=0 PLL root divider /1, [3:0]=5 PLL pre-divider /1.5
		{0x3037, 0x05},
		//[5:4]=01 PCLK root divider /2, [3:2]=00 SCLK2x root divider /1, [1:0]=01 SCLK root divider /2
		{0x3108, 0x11},

		//[6:4]=001 PLL charge pump, [3:0]=1010 MIPI 10-bit mode
		{0x3034, 0x1A},

		//[3:0]=0 X address start high byte
		{0x3800, (0 >> 8) & 0x0F},
		//[7:0]=0 X address start low byte
		{0x3801, 0 & 0xFF},
		//[2:0]=0 Y address start high byte
		{0x3802, (8 >> 8) & 0x07},
		//[7:0]=0 Y address start low byte
		{0x3803, 8 & 0xFF},

		//[3:0] X address end high byte
		{0x3804, (2619 >> 8) & 0x0F},
		//[7:0] X address end low byte
		{0x3805, 2619 & 0xFF},
		//[2:0] Y address end high byte
		{0x3806, (1947 >> 8) & 0x07},
		//[7:0] Y address end low byte
		{0x3807, 1947 & 0xFF},

		//[3:0]=0 timing hoffset high byte
		{0x3810, (0 >> 8) & 0x0F},
		//[7:0]=0 timing hoffset low byte
		{0x3811, 0 & 0xFF},
		//[2:0]=0 timing voffset high byte
		{0x3812, (0 >> 8) & 0x07},
		//[7:0]=0 timing voffset low byte
		{0x3813, 0 & 0xFF},

		//[3:0] Output horizontal width high byte
		{0x3808, (1280 >> 8) & 0x0F},
		//[7:0] Output horizontal width low byte
		{0x3809, 1280 & 0xFF},
		//[2:0] Output vertical height high byte
		{0x380a, (720 >> 8) & 0x7F},
		//[7:0] Output vertical height low byte
		{0x380b, 720 & 0xFF},

		//HTS line exposure time in # of pixels
		{0x380c, (1896 >> 8) & 0x1F},
		{0x380d, 1896 & 0xFF},
		//VTS frame exposure time in # lines
		{0x380e, (984 >> 8) & 0xFF},
		{0x380f, 984 & 0xFF},

		//[7:4]=0x3 horizontal odd subsample increment, [3:0]=0x1 horizontal even subsample increment
		{0x3814, 0x31},
		//[7:4]=0x3 vertical odd subsample increment, [3:0]=0x1 vertical even subsample increment
		{0x3815, 0x31},

		//[2]=0 ISP mirror, [1]=0 sensor mirror, [0]=1 horizontal binning
		{0x3821, 0x01},

		//little MIPI shit: global timing unit, period of PCLK in ns * 2(depends on # of lanes)
		{0x4837, 36}, // 1/56M*2

		//Undocumented anti-green settings
		{0x3618, 0x00}, // Removes vertical lines appearing under bright light
		{0x3612, 0x59},
		{0x3708, 0x64},
		{0x3709, 0x52},
		{0x370c, 0x03},

		//[7:4]=0x0 Formatter RAW, [3:0]=0x0 BGBG/GRGR
		{0x4300, 0x00},
		//[2:0]=0x3 Format select ISP RAW (DPC)
		{0x501f, 0x03}
	};

    //AWD

    adr_data_t const cfg_simple_awb_[] =
	{
		// Disable Advanced AWB
		{0x518d ,0x00},
		{0x518f ,0x20},
		{0x518e ,0x00},
		{0x5190 ,0x20},
		{0x518b ,0x00},
		{0x518c ,0x00},
		{0x5187 ,0x10},
		{0x5188 ,0x10},
		{0x5189 ,0x40},
		{0x518a ,0x40},
		{0x5186 ,0x10},
		{0x5181 ,0x58},
		{0x5184 ,0x25},
		{0x5182 ,0x11},

		// Enable simple AWB
		{0x3406 ,0x00},
		{0x5183 ,0x80},
		{0x5191 ,0xff},
		{0x5192 ,0x00},
		{0x5001 ,0x03}
	};




    init_platform();

    print("Hello World\n\r");
    Xil_DCacheDisable();//キャッシュ無効
	for(i=0;i<1000000;i++);

    //AXI GPIO config
	Status = XGpio_Initialize(&GpioOutput, XPAR_AXI_GPIO_0_DEVICE_ID);
	if (Status != XST_SUCCESS)  {
		  return XST_FAILURE;
	}
	/* Set the direction for all signals to be outputs */
	XGpio_SetDataDirection(&GpioOutput, LED_CHANNEL, 0x0);
	/* Set the GPIO outputs to low */
	XGpio_DiscreteWrite(&GpioOutput, LED_CHANNEL, 0x0);

	//gpio ps conig
	Config_gpiops = XGpioPs_LookupConfig(XPAR_PS7_GPIO_0_DEVICE_ID);
	if (NULL == Config_gpiops) {
		return XST_FAILURE;
	}

	Status = XGpioPs_CfgInitialize(&GpioOutputps, Config_gpiops, Config_gpiops->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//IIC PS config
	Config_iicps = XIicPs_LookupConfig(XPAR_PS7_I2C_0_DEVICE_ID);
	if (NULL == Config_iicps) {
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic, Config_iicps, Config_iicps->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	//iicps reset

	XIicPs_WriteReg(Iic.Config.BaseAddress, XIICPS_CR_OFFSET,
		  XIICPS_CR_RESET_VALUE);
	XIicPs_WriteReg(Iic.Config.BaseAddress,
		  XIICPS_TIME_OUT_OFFSET, XIICPS_TO_RESET_VALUE);
	XIicPs_WriteReg(Iic.Config.BaseAddress, XIICPS_IDR_OFFSET,
		  XIICPS_IXR_ALL_INTR_MASK);
	//scl clock_rate
	XIicPs_SetSClk(&Iic, 100000);
	//vtc_confg
	Config_vtc = XVtc_LookupConfig(XVTC_DEVICE_ID);
	Status = XVtc_CfgInitialize(&VtcInst, Config_vtc, Config_vtc->BaseAddress);
	if (Status != (XST_SUCCESS)) {
		return (XST_FAILURE);
	}
	XVtc_Enable(&VtcInst); //vtc start
	//vdma config
	Config_Vdma = XAxiVdma_LookupConfig(DMA_DEVICE_ID);
	if (!Config_Vdma){
		return XST_FAILURE;
	}
	Status = XAxiVdma_CfgInitialize(&AxiVdma, Config_Vdma, Config_Vdma->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//gpiops ENIO
	XGpioPs_SetDirectionPin(&GpioOutputps,54, 1);
	XGpioPs_SetOutputEnablePin(&GpioOutputps,54 , 1);
	XGpioPs_WritePin(&GpioOutputps,54 , 1);
	XGpioPs_WritePin(&GpioOutputps,54 , 0); //cam reset
	XGpioPs_WritePin(&GpioOutputps,54 , 1); //

	//gpiops MIO7 -> led
	XGpioPs_SetDirectionPin(&GpioOutputps,7, 1);
	XGpioPs_SetOutputEnablePin(&GpioOutputps,7 , 1);
	XGpioPs_WritePin(&GpioOutputps,7 , 1); //led on
	XGpioPs_WritePin(&GpioOutputps,7 , 0); //led off
	XGpioPs_WritePin(&GpioOutputps,7 , 1); //led on

	//iicps for cam  cam_dev_adr = 0x78 >>1 .0x3c
	iic_buffer[0] = (u8)(id_rd_cmd[0].addr>>8);
	iic_buffer[1] = (u8)id_rd_cmd[0].addr;
	XIicPs_MasterSendPolled(&Iic, iic_buffer, 2, 0x3c); //write reg_adr(2byts)
	for(i=0;i<1000000;i++);
	XIicPs_MasterRecvPolled(&Iic, iic_buffer, 1, 0x3c); //read 1byte
	printf("id_h=%x\n",iic_buffer[0]);
	for(i=0;i<1000000;i++);
 	iic_buffer[0] = (u8)(id_rd_cmd[1].addr>>8);
	iic_buffer[1] = (u8)id_rd_cmd[1].addr;
	XIicPs_MasterSendPolled(&Iic, iic_buffer, 2, 0x3c); //write reg_adr(2byts)
	for(i=0;i<1000000;i++);
	XIicPs_MasterRecvPolled(&Iic, iic_buffer, 1, 0x3c); //read 1byte
	printf("id_l=%x\n",iic_buffer[0]);
	for(i=0;i<1000000;i++);
	//[1]=0 System input clock from pad; Default read = 0x11
 	iic_buffer[0] = (u8)(init_cmd[0].addr>>8);
 	iic_buffer[1] = (u8)init_cmd[0].addr;
 	iic_buffer[2] = (u8)init_cmd[0].data;
 	printf("ic_buff[0]=%x\n",iic_buffer[0]);
 	printf("ic_buff[1]=%x\n",iic_buffer[1]);
 	printf("ic_buff[2]=%x\n",iic_buffer[2]);
 	XIicPs_MasterSendPolled(&Iic, iic_buffer, 3, 0x3c);
	for(i=0;i<10000;i++);
	//[7]=1 Software reset; [6]=0 Software power down; Default=0x02
	iic_buffer[0] = (u8)(init_cmd[1].addr)>>8;
	iic_buffer[1] = (u8)init_cmd[1].addr;
	iic_buffer[2] = (u8)init_cmd[1].data;
  	XIicPs_MasterSendPolled(&Iic, iic_buffer, 3, 0x3c);
	for(i=0;i<10000;i++);
	//
	MIPI_CSI_2_RX_mWriteReg(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, (CR_RESET_MASK & ~CR_ENABLE_MASK));
	MIPI_D_PHY_RX_mWriteReg(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, (CR_RESET_MASK & ~CR_ENABLE_MASK));
  	//init
	for (lp=0;lp<sizeof(cfg_init_)/sizeof(cfg_init_[0]); ++lp)
	{
		//writeReg(OV5640_cfg::cfg_init_[i].addr, OV5640_cfg::cfg_init_[i].data);
	    iic_buffer[0] = (u8)(cfg_init_[lp].addr>>8);
	    iic_buffer[1] = (u8)cfg_init_[lp].addr;
	    iic_buffer[2] = (u8)cfg_init_[lp].data;
	    printf("ic_buffe=%2x %2x %2x\n",iic_buffer[0],iic_buffer[1],iic_buffer[2]);
	    XIicPs_MasterSendPolled(&Iic, iic_buffer, 3, 0x3c);
		for(i=0;i<10000;i++);
	}
	//MIPI_CSI EN
	MIPI_CSI_2_RX_mWriteReg(XPAR_MIPI_CSI_2_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, (CR_ENABLE_MASK));
 	//MIPI_D EN
	MIPI_D_PHY_RX_mWriteReg(XPAR_MIPI_D_PHY_RX_0_S_AXI_LITE_BASEADDR, CR_OFFSET, (CR_ENABLE_MASK));
	//720 60fp
	for (lp=0;lp<sizeof(cfg_720_60fp_)/sizeof(cfg_720_60fp_[0]); ++lp)
	{
		//writeReg(OV5640_cfg::cfg_init_[i].addr, OV5640_cfg::cfg_init_[i].data);
	    iic_buffer[0] = (u8)(cfg_720_60fp_[lp].addr>>8);
	    iic_buffer[1] = (u8)cfg_720_60fp_[lp].addr;
	    iic_buffer[2] = (u8)cfg_720_60fp_[lp].data;
	    printf("ic_buffe=%2x %2x %2x\n",iic_buffer[0],iic_buffer[1],iic_buffer[2]);
	    XIicPs_MasterSendPolled(&Iic, iic_buffer, 3, 0x3c);
		for(i=0;i<10000;i++);
	}
	//writeReg(0x3008, 0x02);
	iic_buffer[0] = (u8)0x30;
	iic_buffer[1] = (u8)0x08;
	iic_buffer[2] = (u8)0x02;
	XIicPs_MasterSendPolled(&Iic, iic_buffer, 3, 0x3c);
	for(i=0;i<1000000;i++);
	//
	//AWD
	for (lp=0;lp<sizeof(cfg_720_60fp_)/sizeof(cfg_simple_awb_[0]); ++lp)
	{
		//writeReg(OV5640_cfg::cfg_init_[i].addr, OV5640_cfg::cfg_init_[i].data);
	    iic_buffer[0] = (u8)(cfg_simple_awb_[lp].addr>>8);
	    iic_buffer[1] = (u8)cfg_simple_awb_[lp].addr;
	    iic_buffer[2] = (u8)cfg_simple_awb_[lp].data;
	    printf("ic_buffe=%2x %2x %2x\n",iic_buffer[0],iic_buffer[1],iic_buffer[2]);
	    XIicPs_MasterSendPolled(&Iic, iic_buffer, 3, 0x3c);
		for(i=0;i<10000;i++);
	}
	//writeReg(0x3008, 0x02);
	iic_buffer[0] = (u8)0x30;
	iic_buffer[1] = (u8)0x08;
	iic_buffer[2] = (u8)0x02;
	XIicPs_MasterSendPolled(&Iic, iic_buffer, 3, 0x3c);
	for(i=0;i<1000000;i++);
	//HDMI OUT
	//dma設定 read
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x0, 0x4); //reset
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x0, 0x8); //gen-lock
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x5C, VRAM_ADR);//start adr
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x54, 1280*3);//h size
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x58, 0x01001000);//
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x0, 0x83);//enablr
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x50, 720);//v size,start dma

	//dma設定 write
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x30, 0x4); //reset
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x30, 0x8); //gen-lock
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0xaC, VRAM_ADR);//start adr
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0xa4, 1280*3);//h size
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0xa8, 0x01001000);//
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0x30, 0x83);//enablr
	XAxiVdma_WriteReg(XPAR_AXI_VDMA_0_BASEADDR, 0xa0, 720);//v size,start dma


	while(1){
		for(cnt=0;cnt<64;cnt++){
			XGpio_DiscreteWrite(&GpioOutput, LED_CHANNEL, cnt);
			for(i=0;i<10000000;i++);
		}
	}

	cleanup_platform();
	return 0;
}
