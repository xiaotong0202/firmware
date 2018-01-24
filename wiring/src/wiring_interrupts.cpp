/**
 ******************************************************************************
  Copyright (c) 2013-2014 IntoRobot Team.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation, either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, see <http://www.gnu.org/licenses/>.
  ******************************************************************************
*/

#include "wiring_interrupts.h"

static wiring_interrupt_handler_t* handlers[TOTAL_PINS];

wiring_interrupt_handler_t* allocate_handler(uint16_t pin, wiring_interrupt_handler_t& fn)
{
    delete handlers[pin];
    return handlers[pin] = new wiring_interrupt_handler_t(fn);
}

void call_wiring_interrupt_handler(void* data)
{
    wiring_interrupt_handler_t* handler = (wiring_interrupt_handler_t*)data;
    (*handler)();
}

void call_raw_interrupt_handler(void* data)
{
    raw_interrupt_handler_t handler = raw_interrupt_handler_t(data);
    handler();
}

/*******************************************************************************
 * Function Name  : attachInterrupt
 * Description    : Arduino compatible function to attach hardware interrupts to
						        the Core pins
 * Input          : pin number, user function name and interrupt mode
 * Output         : None.
 * Return         : true if function handler was allocated, false otherwise.
 *******************************************************************************/

HAL_InterruptExtraConfiguration* configure_interrupt(HAL_InterruptExtraConfiguration& extra, int8_t priority, uint8_t subpriority)
{
	extra.size = sizeof(extra);
    if (priority >= 0) {
      extra.IRQChannelPreemptionPriority = (uint8_t)priority;
      extra.IRQChannelSubPriority = subpriority;
      return &extra;
    }
	return nullptr;
}

bool attachInterrupt(uint16_t pin, wiring_interrupt_handler_t fn, InterruptMode mode, int8_t priority, uint8_t subpriority)
{
    HAL_Interrupts_Detach(pin);
    wiring_interrupt_handler_t* handler = allocate_handler(pin, fn);
    if (handler) {
        HAL_InterruptExtraConfiguration extra = {0};
        HAL_Interrupts_Attach(pin, call_wiring_interrupt_handler, handler, mode, configure_interrupt(extra, priority, subpriority));
    }
    return handler!=NULL;
}

bool attachInterrupt(uint16_t pin, raw_interrupt_handler_t handler, InterruptMode mode, int8_t priority, uint8_t subpriority)
{
    HAL_Interrupts_Detach(pin);
    HAL_InterruptExtraConfiguration extra = {0};
    HAL_Interrupts_Attach(pin, call_raw_interrupt_handler, (void*)handler, mode, configure_interrupt(extra, priority, subpriority));
    return true;
}

/*******************************************************************************
 * Function Name  : detachInterrupt
 * Description    : Arduino compatible function to detach hardware interrupts that
						        were asssigned previously using attachInterrupt
 * Input          : pin number to which the interrupt was attached
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void detachInterrupt(uint16_t pin)
{
    HAL_Interrupts_Detach(pin);
    delete handlers[pin];
    handlers[pin] = NULL;
}

/*******************************************************************************
 * Function Name  : noInterrupts
 * Description    : Disable all external interrupts
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void noInterrupts(void)
{
    //Only disable the interrupts that are exposed to the user
    HAL_Interrupts_Disable_All();
}


/*******************************************************************************
 * Function Name  : interrupts
 * Description    : Enable all external interrupts
 * Output         : None.
 * Return         : None.
 *******************************************************************************/
void interrupts(void)
{
    //Only enable the interrupts that are exposed to the user
    HAL_Interrupts_Enable_All();
}

/*
 * System Interrupts
 */
bool attachSystemInterrupt(hal_irq_t irq, wiring_interrupt_handler_t handler)
{
    HAL_InterruptCallback callback;
    callback.handler = call_wiring_interrupt_handler;
    wiring_interrupt_handler_t& h = handler;
    callback.data = new wiring_interrupt_handler_t(h);
    return HAL_Set_System_Interrupt_Handler(irq, &callback, NULL, NULL);
}

/**
 * Removes all registered handlers from the given system interrupt.
 * @param irq   The interrupt from which all handlers are removed.
 * @return {@code true} if handlers were removed.
 */
bool detachSystemInterrupt(hal_irq_t irq)
{
    return HAL_Set_System_Interrupt_Handler(irq, NULL, NULL, NULL);
}

// irq operation
void disableIRQ(void)
{
    HAL_disable_irq();
}
void enableIRQ(void)
{
    HAL_enable_irq(0);
}

