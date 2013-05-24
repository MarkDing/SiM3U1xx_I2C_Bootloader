//------------------------------------------------------------------------------
// Copyright (c) 2013 by Silicon Laboratories.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the Silicon Laboratories End User
// License Agreement which accompanies this distribution, and is available at
// http://developer.silabs.com/legal/version/v10/License_Agreement_v10.htm
// Original content and implementation provided by Silicon Laboratories.
//------------------------------------------------------------------------------

/*
 *  filemgr.c
 *
 *    Revision: 1.0 (10 MAY 2013)
 *      Author: fbar / cmenke
 */

#include "global.h"

#include <sim3u1xx.h>
#include <SI32_CRC_A_Type.h>

//------------------------------------------------------------------------------
// Exported Function Prototypes
//------------------------------------------------------------------------------
void FILEMGR_Init(void);
uint32_t FILEMGR_Validate_Dnload(uint8_t* buffer, uint32_t length);
uint32_t FILEMGR_Initialize_Dnload(void);
uint32_t FILEMGR_Continue_Dnload(uint8_t* buffer, uint32_t length);
uint32_t FILEMGR_Finish_Dnload(void);
uint32_t FILEMGR_Start_Upload(uint8_t* buffer, uint32_t length);
uint32_t FILEMGR_Continue_Upload(uint8_t* buffer, uint32_t length);

//------------------------------------------------------------------------------
// Static Variables
//------------------------------------------------------------------------------
#define IMAGE_SIZE 5*1024

uint32_t Current_Address;
uint8_t FlashImage[IMAGE_SIZE];

DFU_File_Type Dfu_File;


//------------------------------------------------------------------------------
// FILEMGR_Init
//------------------------------------------------------------------------------
void FILEMGR_Init(void)
{
   FLCTL_Init();
   Current_Address = 0xFFFFFFFF;
}

//------------------------------------------------------------------------------
// FILEMGR_Validate_Dnload
//------------------------------------------------------------------------------
// Validates Block Zero to determine if the download operation will proceed
// or will be rejected.  Returns a DFU Status.
//
uint32_t FILEMGR_Validate_Dnload(uint8_t* buffer, uint32_t length)
{

   // Verify that Block0 is equal to the block size
   if(length != FILEMGR_Get_Block_Size())
   {
      return errFILE;
   }

   // Make a copy of the Block0 structure into DFU File
   uint8_t *pDfu_File = (uint8_t*) &Dfu_File;
   uint8_t *pBuffer =(uint8_t*) buffer;
   for(int i = 0; i < SIZEOF_DFU_File_Type; i++)
   {
      *pDfu_File++ = *pBuffer++;
   }


   // Validate DFU File Revision (Must be equal to 1.0 - 0x0100)
   uint16_t dfu_file_revision = *((uint16_t*)&Dfu_File.flash.bDfuFileRevision_Minor);
   if(dfu_file_revision != 0x0100)
   {
      return errFILE;
   }

   // Verify the App Size
   if(Dfu_File.flash.wAppSize > (flash_size - USER_APP_START_ADDR - SIZEOF_DFU_Flash_Type))
   {
      return errFILE;
   }

   // Verify the Target Family
   if(Dfu_File.flash.sTargetFamily[3] != '3') { return errFILE; }
   if(Dfu_File.flash.sTargetFamily[4] != 'U') { return errFILE; }
   if(Dfu_File.flash.sTargetFamily[5] != '1') { return errFILE; }


   // Verify the Signature
   if((Dfu_File.flash.wSignature & 0xFF0000FF) != 0xA500005A)
   {
      return errFILE;
   }

   // Verify the App Start Address
   if(Dfu_File.wAppStartAddress != USER_APP_START_ADDR)
   {
      return errFILE;
   }

   // Verify the block size
   if(Dfu_File.wBlockSize != FILEMGR_Get_Block_Size())
   {
      return errFILE;
   }

   // Verification Passed: Set Flash Keys
   FLCTL_SetFlashKeys(Dfu_File.wFlashKey_A, Dfu_File.wFlashKey_B);

   return OK;
}

//------------------------------------------------------------------------------
// FILEMGR_Initialize_Dnload
//------------------------------------------------------------------------------
uint32_t FILEMGR_Initialize_Dnload(void)
{
   uint32_t *pWord;

   // Erase the Lock Byte
   FLCTL_PageErase(FLCTL_Get_Lock_Word_Address(), DFU_DNLOAD);

   // Erase the Signature
   FLCTL_PageErase(flash_size-16, DFU_DNLOAD);


   // Verify Lock Byte was Erased
   pWord = (uint32_t*) FLCTL_Get_Lock_Word_Address();
   if(*pWord != 0xFFFFFFFF)
   {
      FLCTL_DestroyFlashKeys();
      return errCHECK_ERASED;
   }

   // Verify Signature was Erased
   pWord = (uint32_t*) (flash_size-16);
   if(*pWord != 0xFFFFFFFF)
   {
      FLCTL_DestroyFlashKeys();
      return errCHECK_ERASED;
   }

   // Device image has been destroyed
   // Set the address to begin programming at the application start
   Current_Address = USER_APP_START_ADDR;

   return OK;

}

//------------------------------------------------------------------------------
// FILEMGR_Continue_Dnload
//------------------------------------------------------------------------------
uint32_t FILEMGR_Continue_Dnload(uint8_t* buffer, uint32_t length)
{
   uint8_t *pA, *pB;
   uint32_t num_bytes;

   // Verify the block size
   if(length != FILEMGR_Get_Block_Size())
   {
      FLCTL_DestroyFlashKeys();
      return errFILE;
   }

   if(Current_Address >= ((uint32_t)USER_APP_START_ADDR + Dfu_File.flash.wAppSize) ||
      Current_Address >= flash_size)
   {
      FLCTL_DestroyFlashKeys();
      return errADDRESS;
   }

   // Remove padding by adjusting the length field
   if((Current_Address + length) > (USER_APP_START_ADDR + Dfu_File.flash.wAppSize))
   {
      num_bytes = ((USER_APP_START_ADDR + Dfu_File.flash.wAppSize) - Current_Address);

   } else
   {
      num_bytes = length;
   }

   // Erase the page at the current address
   FLCTL_PageErase(Current_Address, DFU_DNLOAD);


   // Verify page was erased
   for(uint32_t i = Current_Address; i < (Current_Address + FILEMGR_Get_Block_Size()); i += 4)
   {
     if(*((uint32_t*)i) != 0xFFFFFFFF)
     {
        FLCTL_DestroyFlashKeys();
        return errCHECK_ERASED;
     }
   }

   // Write the block to Flash
   FLCTL_Write(buffer, Current_Address, num_bytes, DFU_DNLOAD);

   // Verify the block in Flash
   pA = (uint8_t*) Current_Address;
   pB = (uint8_t*) buffer;

   for(int i = 0; i < num_bytes; i++)
   {
      if(pA[i] != pB[i])
      {
         FLCTL_DestroyFlashKeys();
         return errVERIFY;
      }
   }

   // Block Write Verified, Increment Current Address to next Block
   Current_Address += length;

   return OK;
}
//------------------------------------------------------------------------------
// FILEMGR_Finish_Dnload
//------------------------------------------------------------------------------
uint32_t FILEMGR_Finish_Dnload(void)
{
   uint8_t *pA, *pB;
   uint32_t lock_word_flash;
   uint32_t signature;
   uint32_t crc;

   // Perform CRC on entire image
   uint32_t app_length = Dfu_File.flash.wAppSize;

   // Initialize Hardware CRC Engine
   DEVICE_InitializeCRC32();

   if(app_length <= (flash_size - USER_APP_START_ADDR - SIZEOF_DFU_Flash_Type))
   {
      for(int i = 0; i < app_length; i++)
      {
         DEVICE_UpdateCRC32 (*((uint8_t *) (USER_APP_START_ADDR + i)));
      }

      crc = DEVICE_ReadCRC32Result();

      if(crc != Dfu_File.flash.wCrc)
      {
         FLCTL_DestroyFlashKeys();
         return errVERIFY;
      }

   } else
   {
      FLCTL_DestroyFlashKeys();
      return errVERIFY;
   }


   // Write the DFU Information Structure to Flash
   FLCTL_Write((uint8_t*)&Dfu_File.flash.bDfuFileRevision_Minor,  // ram buffer
               flash_size-SIZEOF_DFU_Flash_Type,                  // flash address
               SIZEOF_DFU_Flash_Type - 8,                         // length
               DFU_DNLOAD                                         // operation
              );

   // Write the Lock Word
   FLCTL_Write((uint8_t*)&Dfu_File.flash.wLock,                   // ram buffer
               FLCTL_Get_Lock_Word_Address(),                     // flash address
               4,                                                 // length
               DFU_DNLOAD                                         // operation
              );

   // Write the Signature
   FLCTL_Write((uint8_t*)&Dfu_File.flash.wSignature,              // ram buffer
               flash_size-8,                                      // flash address
               4,                                                 // length
               DFU_DNLOAD                                         // operation                                                  // length
              );


   // Verify the DFU Information Structure was Written
   pA = (uint8_t*)&Dfu_File.flash.bDfuFileRevision_Minor;
   pB = (uint8_t*) flash_size-SIZEOF_DFU_Flash_Type;

   for(int i = 0; i < (SIZEOF_DFU_Flash_Type - 8); i++)
   {
      if(pA[i] != pB[i])
      {
         FLCTL_DestroyFlashKeys();
         return errVERIFY;
      }
   }

   // Verify the Lock Word
   lock_word_flash = *((uint32_t*)FLCTL_Get_Lock_Word_Address());
   if(lock_word_flash != Dfu_File.flash.wLock)
   {
      FLCTL_DestroyFlashKeys();
      return errVERIFY;
   }

   // Verify the Signature
   signature = *((uint32_t*)(flash_size-8));
   if(signature != Dfu_File.flash.wSignature)
   {
      FLCTL_DestroyFlashKeys();
      return errVERIFY;
   }

   // Download Complete
   Current_Address = 0xFFFFFFFF;
   FLCTL_DestroyFlashKeys();
   return OK;
}

//------------------------------------------------------------------------------
// FILEMGR_Start_Upload
//------------------------------------------------------------------------------
uint32_t FILEMGR_Start_Upload(uint8_t* buffer, uint32_t length)
{
   int i;
   uint8_t *pA, *pB, *pC;

   volatile uint32_t lock_word;

   // Fill the buffer with everything minus the lock byte
   // Also make a copy in Dfu_File
   pA = (uint8_t*) buffer;
   pB = (uint8_t*) flash_size-SIZEOF_DFU_Flash_Type;
   pC = (uint8_t*) &Dfu_File;
   for(i = 0; i < SIZEOF_DFU_Flash_Type-4; i++)
   {
      pA[i] = pB[i];
      pC[i] = pB[i];
   }

   // Fill the buffer with the lock word
   pB = (uint8_t*) FLCTL_Get_Lock_Word_Address();
   for(int k = 0; k < 4; k++)
   {
      pA[i] = pB[k];
      pC[i] = pB[k];

      i++;
   }

   // Populate the DFU File Structure with remaining fields then copy to buffer
   Dfu_File.wAppStartAddress = USER_APP_START_ADDR;
   Dfu_File.wBlockSize = FILEMGR_Get_Block_Size();
   pB = (uint8_t*) &Dfu_File.wAppStartAddress;
   for(int k = 0; k < 8; k++)
   {
      pA[i] = pB[k];
      i++;
   }

   // Insert the Flash Keys into the Output Buffer
   Dfu_File.wFlashKey_A = 0xA5;
   Dfu_File.wFlashKey_B = 0xF1;

   uint32_t *p32 = (uint32_t*) &pA[i];
   *p32 = (FLCTL_Get_FlashKey_A() ^ DFU_UPLOAD);
   i += 4;

   p32 = (uint32_t*) &pA[i];
   *p32 = (FLCTL_Get_FlashKey_B() ^ DFU_UPLOAD);
   i += 4;

   // Pad the remaining length with zeros
   while(i < length)
   {
      pA[i++] = 0x00;
   }

   // Set future upload packets to begin uploading image
   Current_Address = USER_APP_START_ADDR;

   return length;
}
//------------------------------------------------------------------------------
// FILEMGR_Continue_Upload
//------------------------------------------------------------------------------
uint32_t FILEMGR_Continue_Upload(uint8_t* buffer, uint32_t length)
{

   uint32_t num_bytes;
   uint32_t i;

   // Invalid application image
   if((Dfu_File.flash.wSignature & 0xFF0000FF) != 0xA500005A)
   {
     return 0;
   }

   if(Dfu_File.flash.wAppSize > (flash_size - USER_APP_START_ADDR - SIZEOF_DFU_Flash_Type))
   {
      return 0;
   }

   if(Current_Address > (USER_APP_START_ADDR + Dfu_File.flash.wAppSize))
   {
      return 0;
   }

   // Determine the number of unpadded bytes
   if((Current_Address + length) > (USER_APP_START_ADDR + Dfu_File.flash.wAppSize))
   {
      num_bytes = ((USER_APP_START_ADDR + Dfu_File.flash.wAppSize) - Current_Address);

      if(num_bytes == 0)
      {
         return 0;
      }

   } else
   {
      num_bytes = length;
   }


   // Copy num_bytes into buffer
   for( i = 0; i < num_bytes; i++)
   {
      buffer[i] = ((uint8_t*) Current_Address)[i];
   }

   // Pad the remaining length with zeros
   while(i < length)
   {
      buffer[i++] = 0xFF;
   }

   // Increment the current address
   Current_Address += num_bytes;

   return length;
}
