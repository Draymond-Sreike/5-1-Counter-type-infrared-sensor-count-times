#include "stm32f10x.h"                  // Device header

uint8_t count;

void countSensor_Init()
{
	/*****GPIOB的中断选择配置*******/
	//以下代码配置B14作为中断信号的输入口，即使得与B14连接的硬件作为中断源
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIOB_InitStructure;
	GPIOB_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIOB_InitStructure.GPIO_Pin = GPIO_Pin_14;
	GPIOB_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIOB_InitStructure);
	
	/*****AFIO的中断选择配置*******/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	//这个函数虽然写着EXTL，但实际上是配置AFIO，然后去跟EXTL产生联系
	//这个函数完成了中断源的选取，即中断输入口的选择，选择了GPIOB14口作为中断源输入
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
	
	/*****EXTI的中断选择配置*******/
	EXTI_InitTypeDef EXTI_InitStructure;
	///因为选择了B14作为中断源，所以相应的EXTI_Line选择14
	EXTI_InitStructure.EXTI_Line = EXTI_Line14;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	//事件响应/中断响应，选择中断响应
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	//选择中断为下降沿触发
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_Init(&EXTI_InitStructure);
	
	/*****NVIC的中断选择配置*******/
	//设置抢占和响应的优先级分组为组2
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitTypeDef NVIC_InitStructure;
	//因为EXTI是14线过来的，所以选择EXTI15_10_IRQn
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	//优先级组2中抢占和响应优先级的数值可选均为0~3，因为无中断冲突，选择任意数字均可
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	
	NVIC_Init(&NVIC_InitStructure);
	//至此完成IO口中断源与CPU的联系
}

uint8_t get_Count()
{
	return count;
}
/*****中断函数*******/
//中断函数不需要被调用，也无需在.h文件中声明，其会自动执行
void EXTI15_10_IRQHandler()
{
	if(EXTI_GetITStatus(EXTI_Line14) == SET)//判断是否为指定的中断标志位被中断标志,若其位指定中断，则执行相应功能
	{
		/*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14) == 0)
		{
			count ++;
		}
		
		//清0中断标志位，若不清零，中断标志位一直存在会导致其一直申请中断
		//最终导致主程序无法执行，且下一次中断也无法被察觉	
		EXTI_ClearITPendingBit(EXTI_Line14);
	}
}
