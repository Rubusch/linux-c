/*******************************************************************************
  USB Peripheral Library Template Implementation

  File Name:
    usb_SpeedControl_Unsupported.h

  Summary:
    USB PLIB Template Implementation

  Description:
    This header file contains template implementations
    For Feature : SpeedControl
    and its Variant : Unsupported
    For following APIs :
        PLIB_USB_FullSpeedEnable
        PLIB_USB_FullSpeedDisable
        PLIB_USB_ExistsSpeedControl

*******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2012 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*******************************************************************************/

//DOM-IGNORE-END

#ifndef USB_SPEEDCONTROL_UNSUPPORTED_H
#define USB_SPEEDCONTROL_UNSUPPORTED_H

//******************************************************************************
/* Routines available for accessing VREGS, MASKS, POS, LEN are 

  VREGs: 
    None.

  MASKs: 
    None.

  POSs: 
    None.

  LENs: 
    None.

*/


//******************************************************************************
/* Function :  USB_FullSpeedEnable_Unsupported

  Summary:
    Implements Unsupported variant of PLIB_USB_FullSpeedEnable 

  Description:
    This template implements the Unsupported variant of the PLIB_USB_FullSpeedEnable function.
*/

PLIB_TEMPLATE void USB_FullSpeedEnable_Unsupported( USB_MODULE_ID index )
{
    PLIB_ASSERT(false, "The device selected does not implement PLIB_USB_FullSpeedEnable");
}


//******************************************************************************
/* Function :  USB_FullSpeedDisable_Unsupported

  Summary:
    Implements Unsupported variant of PLIB_USB_FullSpeedDisable 

  Description:
    This template implements the Unsupported variant of the PLIB_USB_FullSpeedDisable function.
*/

PLIB_TEMPLATE void USB_FullSpeedDisable_Unsupported( USB_MODULE_ID index )
{
    PLIB_ASSERT(false, "The device selected does not implement PLIB_USB_FullSpeedDisable");
}


//******************************************************************************
/* Function :  USB_ExistsSpeedControl_Unsupported

  Summary:
    Implements Unsupported variant of PLIB_USB_ExistsSpeedControl

  Description:
    This template implements the Unsupported variant of the PLIB_USB_ExistsSpeedControl function.
*/

PLIB_TEMPLATE bool USB_ExistsSpeedControl_Unsupported( USB_MODULE_ID index )
{
    return false;
}


#endif /*USB_SPEEDCONTROL_UNSUPPORTED_H*/

/******************************************************************************
 End of File
*/

