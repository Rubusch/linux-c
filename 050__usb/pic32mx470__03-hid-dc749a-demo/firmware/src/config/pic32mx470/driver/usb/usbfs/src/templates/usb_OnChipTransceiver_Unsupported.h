/*******************************************************************************
  USB Peripheral Library Template Implementation

  File Name:
    usb_OnChipTransceiver_Unsupported.h

  Summary:
    USB PLIB Template Implementation

  Description:
    This header file contains template implementations
    For Feature : OnChipTransceiver
    and its Variant : Unsupported
    For following APIs :
        PLIB_USB_TransceiverEnable
        PLIB_USB_TransceiverDisable
        PLIB_USB_ExistsOnChipTransceiver

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

#ifndef USB_ONCHIPTRANSCEIVER_UNSUPPORTED_H
#define USB_ONCHIPTRANSCEIVER_UNSUPPORTED_H

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
/* Function :  USB_TransceiverEnable_Unsupported

  Summary:
    Implements Unsupported variant of PLIB_USB_TransceiverEnable 

  Description:
    This template implements the Unsupported variant of the PLIB_USB_TransceiverEnable function.
*/

PLIB_TEMPLATE void USB_TransceiverEnable_Unsupported( USB_MODULE_ID index )
{
    PLIB_ASSERT(false, "The device selected does not implement PLIB_USB_TransceiverEnable");
}


//******************************************************************************
/* Function :  USB_TransceiverDisable_Unsupported

  Summary:
    Implements Unsupported variant of PLIB_USB_TransceiverDisable 

  Description:
    This template implements the Unsupported variant of the PLIB_USB_TransceiverDisable function.
*/

PLIB_TEMPLATE void USB_TransceiverDisable_Unsupported( USB_MODULE_ID index )
{
    PLIB_ASSERT(false, "The device selected does not implement PLIB_USB_TransceiverDisable");
}


//******************************************************************************
/* Function :  USB_ExistsOnChipTransceiver_Unsupported

  Summary:
    Implements Unsupported variant of PLIB_USB_ExistsOnChipTransceiver

  Description:
    This template implements the Unsupported variant of the PLIB_USB_ExistsOnChipTransceiver function.
*/

PLIB_TEMPLATE bool USB_ExistsOnChipTransceiver_Unsupported( USB_MODULE_ID index )
{
    return false;
}


#endif /*USB_ONCHIPTRANSCEIVER_UNSUPPORTED_H*/

/******************************************************************************
 End of File
*/

