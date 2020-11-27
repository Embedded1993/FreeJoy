/**
  ******************************************************************************
  * @file    usb_endp.c
  * @author  MCD Application Team
  * @version V4.1.0
  * @date    26-May-2017
  * @brief   Endpoint routines
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/

#include "usb_hw.h"
#include "usb_lib.h"
#include "usb_istr.h"
#include "usb_pwr.h"

#include "config.h"
#include "crc16.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile extern uint8_t bootloader;
volatile extern int32_t joy_ticks;

__IO uint8_t PrevXferComplete = 1;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : EP1_OUT_Callback.
* Description    : EP1 OUT Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_OUT_Callback(void)
{
	static  dev_config_t tmp_dev_config;
	
	uint8_t config_in_cnt;
	uint8_t config_out_cnt;
	uint8_t tmp_buf[64];
	uint8_t hid_buf[64];
	uint8_t i;
	uint8_t pos = 2;
	uint8_t repotId;

	// 2 second delay for joy report
	joy_ticks = GetTick() + 2000;
	
	/* Read received data (2 bytes) */  
  USB_SIL_Read(EP1_OUT, hid_buf);
	
	repotId = hid_buf[0];
	
	switch (repotId)
	{
		case REPORT_ID_CONFIG_IN:
		{
			config_in_cnt = hid_buf[1];			// requested config packet number
			
			if ((config_in_cnt > 0) & (config_in_cnt <= 16))
			{		
				
				uint8_t pos = 2;
				uint8_t i;
				
				DevConfigGet(&tmp_dev_config);
				
				memset(tmp_buf, 0, sizeof(tmp_buf));			
				tmp_buf[0] = REPORT_ID_CONFIG_IN;					
				tmp_buf[1] = config_in_cnt;
				
				switch(config_in_cnt)
				{
					case 1:	
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.firmware_version), sizeof(tmp_dev_config.firmware_version));
						pos += sizeof(tmp_dev_config.firmware_version);
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.device_name), sizeof(tmp_dev_config.device_name));
						pos += sizeof(tmp_dev_config.device_name);
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.button_debounce_ms), 4);
						pos += 4;							
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.pins), sizeof(tmp_dev_config.pins));
						pos += sizeof(tmp_dev_config.pins);
					
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
					
					case 2:
						i = 0;
						while(sizeof(tmp_buf) - pos > sizeof(axis_config_t))
						{
							memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.axis_config[i++]), sizeof(axis_config_t));
							pos += sizeof(axis_config_t);
						}
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
					
					case 3:
						i = 2;
						while(sizeof(tmp_buf) - pos > sizeof(axis_config_t))
						{
							memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.axis_config[i++]), sizeof(axis_config_t));
							pos += sizeof(axis_config_t);
						}
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
					
					case 4:
						i = 4;
						while(sizeof(tmp_buf) - pos > sizeof(axis_config_t))
						{
							memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.axis_config[i++]), sizeof(axis_config_t));
							pos += sizeof(axis_config_t);
						}
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;

					case 5:

						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.axis_config[6]), sizeof(axis_config_t));
						pos += sizeof(axis_config_t);
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.axis_config[7]), sizeof(axis_config_t));
						pos += sizeof(axis_config_t);
						
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
					
					case 6:
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.buttons[0]), 60);				
						memcpy(&tmp_buf[64-sizeof(tmp_dev_config.button_timer1_ms)], (uint8_t *) &(tmp_dev_config.button_timer1_ms), sizeof(tmp_dev_config.button_timer1_ms));
						
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
					
					case 7:
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.buttons[20]), 60);			
						memcpy(&tmp_buf[64-sizeof(tmp_dev_config.button_timer2_ms)], (uint8_t *) &(tmp_dev_config.button_timer2_ms), sizeof(tmp_dev_config.button_timer2_ms));
						
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
					
					case 8:
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.buttons[40]), 60);			
						memcpy(&tmp_buf[64-sizeof(tmp_dev_config.button_timer3_ms)], (uint8_t *) &(tmp_dev_config.button_timer3_ms), sizeof(tmp_dev_config.button_timer3_ms));
						
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
					
					case 9:
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.buttons[60]), 60);		
						memcpy(&tmp_buf[64-sizeof(tmp_dev_config.a2b_debounce_ms)], (uint8_t *) &(tmp_dev_config.a2b_debounce_ms), sizeof(tmp_dev_config.a2b_debounce_ms));
					
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
					
					case 10:
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.buttons[80]), 60);				
					
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
					
					case 11:
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.buttons[100]), 60);	

						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;	
					
					case 12:
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.buttons[120]), 8*sizeof(button_t));
						pos += 8*sizeof(button_t);
					
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.axes_to_buttons[0]), sizeof(axis_to_buttons_t));	
						pos += sizeof(axis_to_buttons_t);
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.axes_to_buttons[1]), sizeof(axis_to_buttons_t));	
						pos += sizeof(axis_to_buttons_t);
					
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
					
					case 13:
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.axes_to_buttons[2]), sizeof(axis_to_buttons_t));	
						pos += sizeof(axis_to_buttons_t);		
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.axes_to_buttons[3]), sizeof(axis_to_buttons_t));	
						pos += sizeof(axis_to_buttons_t);
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.axes_to_buttons[4]), sizeof(axis_to_buttons_t));	
						pos += sizeof(axis_to_buttons_t);					
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.axes_to_buttons[5]), sizeof(axis_to_buttons_t));	
						pos += sizeof(axis_to_buttons_t);
					
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
					
					case 14:
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.axes_to_buttons[6]), sizeof(axis_to_buttons_t));	
						pos += sizeof(axis_to_buttons_t);	
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.axes_to_buttons[7]), sizeof(axis_to_buttons_t));	
						pos += sizeof(axis_to_buttons_t);	
					
						for (i=0; i<4; i++)
						{
							memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.shift_registers[i]), sizeof(shift_reg_config_t));
							pos += sizeof(shift_reg_config_t);
						}
						
						for (i=0; i<5; i++)
						{
							memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.shift_config[i]), sizeof(shift_modificator_t));
							pos += sizeof(shift_modificator_t);
						}
						
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.vid), sizeof(tmp_dev_config.vid));
						pos += sizeof(tmp_dev_config.vid);
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.pid), sizeof(tmp_dev_config.pid));
						pos += sizeof(tmp_dev_config.pid);
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.is_dynamic_config), sizeof(tmp_dev_config.is_dynamic_config));
						pos += sizeof(tmp_dev_config.is_dynamic_config);
						
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
						
					case 15:
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.led_pwm_config), sizeof(tmp_dev_config.led_pwm_config));
						pos += sizeof(tmp_dev_config.led_pwm_config);
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.leds[0]), MAX_LEDS_NUM*sizeof(led_config_t));
						pos += MAX_LEDS_NUM*sizeof(led_config_t);
					
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
					
					case 16:												
						memcpy(&tmp_buf[pos], (uint8_t *) &(tmp_dev_config.encoders[0]), MAX_ENCODERS_NUM*sizeof(encoder_t));
						pos += MAX_ENCODERS_NUM*sizeof(encoder_t);
					
						USB_CUSTOM_HID_SendReport((uint8_t *)&(tmp_buf), 64);
						break;
						
					default:
						break;
											
				}
					
				
			}
		}
		break;
		
		case REPORT_ID_CONFIG_OUT:
		{
			switch (hid_buf[1])
			{
				case 1:
					{
						memcpy((uint8_t *) &(tmp_dev_config.firmware_version), &hid_buf[pos], sizeof(tmp_dev_config.firmware_version));
						pos += sizeof(tmp_dev_config.firmware_version);
						memcpy((uint8_t *) &(tmp_dev_config.device_name), &hid_buf[pos], sizeof(tmp_dev_config.device_name));
						pos += sizeof(tmp_dev_config.device_name);
						memcpy((uint8_t *) &(tmp_dev_config.button_debounce_ms), &hid_buf[pos], 4);
						pos += 4;					
						memcpy((uint8_t *) &(tmp_dev_config.pins), &hid_buf[pos], sizeof(tmp_dev_config.pins));
						pos += sizeof(tmp_dev_config.pins);
					}
				break;
				
				case 2:
				{
					i = 0;
					while(64 - pos > sizeof(axis_config_t))
					{
						memcpy((uint8_t *) &(tmp_dev_config.axis_config[i++]), &hid_buf[pos], sizeof(axis_config_t));
						pos += sizeof(axis_config_t);
					}
				}
				break;
				
				case 3:
				{
					i = 2;
					while(64 - pos > sizeof(axis_config_t))
					{
						memcpy((uint8_t *) &(tmp_dev_config.axis_config[i++]), &hid_buf[pos], sizeof(axis_config_t));
						pos += sizeof(axis_config_t);
					}
				}
				break;
				
				case 4:
				{
					i = 4;
					while(64 - pos > sizeof(axis_config_t))
					{
						memcpy((uint8_t *) &(tmp_dev_config.axis_config[i++]), &hid_buf[pos], sizeof(axis_config_t));
						pos += sizeof(axis_config_t);
					}
				}
				break;

				case 5:
				{
					memcpy((uint8_t *) &(tmp_dev_config.axis_config[6]), &hid_buf[pos], sizeof(axis_config_t));
					pos += sizeof(axis_config_t);
					memcpy((uint8_t *) &(tmp_dev_config.axis_config[7]), &hid_buf[pos], sizeof(axis_config_t));
					pos += sizeof(axis_config_t);
					
				}
				break;
				
				case 6:
				{
					memcpy((uint8_t *) &(tmp_dev_config.buttons[0]), &hid_buf[pos], 60);
					memcpy((uint8_t *) &(tmp_dev_config.button_timer1_ms), &hid_buf[64-sizeof(tmp_dev_config.button_timer1_ms)], sizeof(tmp_dev_config.button_timer1_ms));
				}
				break;
				
				case 7:
				{
					memcpy((uint8_t *) &(tmp_dev_config.buttons[20]), &hid_buf[pos], 60);
					memcpy((uint8_t *) &(tmp_dev_config.button_timer2_ms), &hid_buf[64-sizeof(tmp_dev_config.button_timer2_ms)], sizeof(tmp_dev_config.button_timer2_ms));
				}
				break;
				
				case 8:
				{
					memcpy((uint8_t *) &(tmp_dev_config.buttons[40]), &hid_buf[pos], 60);
					memcpy((uint8_t *) &(tmp_dev_config.button_timer3_ms), &hid_buf[64-sizeof(tmp_dev_config.button_timer3_ms)], sizeof(tmp_dev_config.button_timer3_ms));
				}
				break;
				
				case 9:
				{
					memcpy((uint8_t *) &(tmp_dev_config.buttons[60]), &hid_buf[pos], 60);
					memcpy((uint8_t *) &(tmp_dev_config.a2b_debounce_ms), &hid_buf[64-sizeof(tmp_dev_config.a2b_debounce_ms)], sizeof(tmp_dev_config.a2b_debounce_ms));
				}
				break;
				
				case 10:
				{
					memcpy((uint8_t *) &(tmp_dev_config.buttons[80]), &hid_buf[pos], 60);
				}
				break;
				
				case 11:
				{
					memcpy((uint8_t *) &(tmp_dev_config.buttons[100]), &hid_buf[pos], 60);
				}
				break;
				
				case 12:
				{
					memcpy((uint8_t *) &(tmp_dev_config.buttons[120]), &hid_buf[pos], 8*sizeof(button_t));
					pos += 8*sizeof(button_t);
					
					memcpy((uint8_t *) &(tmp_dev_config.axes_to_buttons[0]), &hid_buf[pos], sizeof(axis_to_buttons_t));
					pos += sizeof(axis_to_buttons_t);
					memcpy((uint8_t *) &(tmp_dev_config.axes_to_buttons[1]), &hid_buf[pos], sizeof(axis_to_buttons_t));
					pos += sizeof(axis_to_buttons_t);
					memcpy((uint8_t *) &(tmp_dev_config.axes_to_buttons[2]), &hid_buf[pos], sizeof(axis_to_buttons_t));
					pos += sizeof(axis_to_buttons_t);
				}
				break;
				
				case 13:
				{
					memcpy((uint8_t *) &(tmp_dev_config.axes_to_buttons[2]), &hid_buf[pos], sizeof(axis_to_buttons_t));
					pos += sizeof(axis_to_buttons_t);
					memcpy((uint8_t *) &(tmp_dev_config.axes_to_buttons[3]), &hid_buf[pos], sizeof(axis_to_buttons_t));
					pos += sizeof(axis_to_buttons_t);
					memcpy((uint8_t *) &(tmp_dev_config.axes_to_buttons[4]), &hid_buf[pos], sizeof(axis_to_buttons_t));
					pos += sizeof(axis_to_buttons_t);
					memcpy((uint8_t *) &(tmp_dev_config.axes_to_buttons[5]), &hid_buf[pos], sizeof(axis_to_buttons_t));
					pos += sizeof(axis_to_buttons_t);
					break;
				}
				case 14:
				{
					memcpy((uint8_t *) &(tmp_dev_config.axes_to_buttons[6]), &hid_buf[pos], sizeof(axis_to_buttons_t));
					pos += sizeof(axis_to_buttons_t);
					memcpy((uint8_t *) &(tmp_dev_config.axes_to_buttons[7]), &hid_buf[pos], sizeof(axis_to_buttons_t));
					pos += sizeof(axis_to_buttons_t);
					
					for (i=0; i<4; i++)
					{
						memcpy((uint8_t *) &(tmp_dev_config.shift_registers[i]), &hid_buf[pos], sizeof(shift_reg_config_t));
						pos += sizeof(shift_reg_config_t);
					}
					
					
					memcpy((uint8_t *) &(tmp_dev_config.shift_config[0]), &hid_buf[pos], sizeof(shift_modificator_t));
					pos += sizeof(shift_modificator_t);
					memcpy((uint8_t *) &(tmp_dev_config.shift_config[1]), &hid_buf[pos], sizeof(shift_modificator_t));
					pos += sizeof(shift_modificator_t);
					memcpy((uint8_t *) &(tmp_dev_config.shift_config[2]), &hid_buf[pos], sizeof(shift_modificator_t));
					pos += sizeof(shift_modificator_t);
					memcpy((uint8_t *) &(tmp_dev_config.shift_config[3]), &hid_buf[pos], sizeof(shift_modificator_t));
					pos += sizeof(shift_modificator_t);
					memcpy((uint8_t *) &(tmp_dev_config.shift_config[4]), &hid_buf[pos], sizeof(shift_modificator_t));
					pos += sizeof(shift_modificator_t);
					
					
					
					memcpy((uint8_t *) &(tmp_dev_config.vid), &hid_buf[pos], sizeof(tmp_dev_config.vid));
					pos += sizeof(tmp_dev_config.vid);
					memcpy((uint8_t *) &(tmp_dev_config.pid), &hid_buf[pos], sizeof(tmp_dev_config.pid));
					pos += sizeof(tmp_dev_config.pid);
					memcpy((uint8_t *) &(tmp_dev_config.is_dynamic_config), &hid_buf[pos], sizeof(tmp_dev_config.is_dynamic_config));
					pos += sizeof(tmp_dev_config.is_dynamic_config);
					
				}					
					break;
				
				case 15:
				{
					memcpy((uint8_t *) &(tmp_dev_config.led_pwm_config), &hid_buf[pos], sizeof(tmp_dev_config.led_pwm_config));
					pos += sizeof(tmp_dev_config.led_pwm_config);
					memcpy((uint8_t *) &(tmp_dev_config.leds[0]), &hid_buf[pos], MAX_LEDS_NUM*sizeof(led_config_t));
					pos += MAX_LEDS_NUM*sizeof(led_config_t);
				}
				break;
				
				case 16:
				{
					memcpy((uint8_t *) &(tmp_dev_config.encoders[0]), &hid_buf[pos], MAX_ENCODERS_NUM*sizeof(encoder_t));
					pos += MAX_ENCODERS_NUM*sizeof(encoder_t);
				}
				break;
				
				default:
					break;
			}
			if (hid_buf[1] < 16)		// request new packet
			{
				config_out_cnt = hid_buf[1] + 1;
				
				uint8_t tmp_buf[2];
				tmp_buf[0] = REPORT_ID_CONFIG_OUT;
				tmp_buf[1] = config_out_cnt;
				
				USB_CUSTOM_HID_SendReport(tmp_buf,2);
			}
			else // last packet received
			{
				// Check if config version matches
				if ((tmp_dev_config.firmware_version &0xFFF0) != (FIRMWARE_VERSION & 0xFFF0))
				{
					// Report error
					uint8_t tmp_buf[2];
					tmp_buf[0] = REPORT_ID_CONFIG_OUT;
					tmp_buf[1] = 0xFE;
					USB_CUSTOM_HID_SendReport(tmp_buf,2);
					
					// blink LED if firmware version doesnt match
					GPIO_InitTypeDef GPIO_InitStructure;
					GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
					GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
					GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
					GPIO_Init(GPIOC, &GPIO_InitStructure);
					
					GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
					GPIO_Init(GPIOB, &GPIO_InitStructure);
					
					for (uint8_t i=0; i<6; i++) 
					{
						
						GPIOB->ODR ^= GPIO_Pin_12;
						GPIOC->ODR ^=	GPIO_Pin_13;
						Delay_us(200000);
					}
				}
				else
				{
					tmp_dev_config.firmware_version = FIRMWARE_VERSION;
					DevConfigSet(&tmp_dev_config);
					NVIC_SystemReset();
				}		
			}
		}
		break;
			
		case REPORT_ID_FIRMWARE:
		{
			const char tmp_str[] = "bootloader run";

			if (strcmp(tmp_str, (const char *) &hid_buf[1]) == 0)
			{
				bootloader = 1;
			}
		}
		break;
		
		default:
			break;
	}

	memset(hid_buf, 0 ,64);
  SetEPRxStatus(ENDP1, EP_RX_VALID);
 
}

/*******************************************************************************
* Function Name  : EP1_IN_Callback.
* Description    : EP1 IN Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback(void)
{
  PrevXferComplete = 1;
}

void USB_CUSTOM_HID_SendReport(uint8_t * data, uint8_t length)
{
	if ((PrevXferComplete) && (bDeviceState == CONFIGURED))
	{
			USB_SIL_Write(EP1_IN, data, length);
			SetEPTxValid(ENDP1);
			PrevXferComplete = 0;
	}
}
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

