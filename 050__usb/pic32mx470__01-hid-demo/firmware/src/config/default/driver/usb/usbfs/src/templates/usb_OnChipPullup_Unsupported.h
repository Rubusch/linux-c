/*******************************************************************************
  USB Peripheral Library Template Implementation

  File Name:
    usb_OnChipPullup_Unsupported.h

  Summary:
    USB PLIB Template Implementation

  Description:
    This header file contains template implementations
    For Feature : OnChipPullup
    and its Variant : Unsupported
    For following APIs :
        PLIB_USB_OnChipPullUpEnable
        PLIB_USB_OnChipPullUpDisable
        PLIB_USB_ExistsOnChipPullup

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

#ifndef USB_ONCHIPPULLUP_UNSUPPORTED_H
#define USB_ONCHIPPULLUP_UNSUPPORTED_H

//******************************************************************************
/* Function :  USB_OnChipPullUpEnable_Unsupported

  Summary:
    Implements Unsupported variant of PLIB_USB_OnChipPullUpEnable 

  Description:
    This template implements the Unsupported variant of the PLIB_USB_OnChipPullUpEnable function.
*/

PLIB_TEMPLATE void USB_OnChipPullUpEnable_Unsupported( USB_MODULE_ID index )
{
    PLIB_ASSERT(false, "The device selected does not implement PLIB_USB_OnChipPullUpEnable");
}


//******************************************************************************
/* Function :  USB_OnChipPullUpDisable_Unsupported

  Summary:
    Implements Unsupported variant of PLIB_USB_OnChipPullUpDisable 

  Description:
    This template implements the Unsupported variant of the PLIB_USB_OnChipPullUpDisable function.
*/

PLIB_TEMPLATE void USB_OnChipPullUpDisable_Unsupported( USB_MODULE_ID index )
{
    PLIB_ASSERT(false, "The device selected does not implement PLIB_USB_OnChipPullUpDisable");
}


//******************************************************************************
/* Function :  USB_ExistsOnChipPullup_Unsupported

  Summary:
    Implements Unsupported variant of PLIB_USB_ExistsOnChipPullup

  Description:
    This template implements the Unsupported variant of the PLIB_USB_ExistsOnChipPullup function.
*/

PLIB_TEMPLATE bool USB_ExistsOnChipPullup_Unsupported( USB_MODULE_ID index )
{
    return false;
}


#endif /*USB_ONCHIPPULLUP_UNSUPPORTED_H*/

/******************************************************************************
 End of File
*/

