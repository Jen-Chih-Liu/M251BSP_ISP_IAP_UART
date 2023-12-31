/**************************************************************************//**
 * @file     crypto.c
 * @version  V1.10
 * @brief  Cryptographic Accelerator driver source file
 *
 * SPDX-License-Identifier: Apache-2.0
 * @copyright (C) 2019 Nuvoton Technology Corp. All rights reserved.
*****************************************************************************/

#include <stdio.h>
#include <string.h>
#include "NuMicro.h"

#define ENABLE_DEBUG    0

#if ENABLE_DEBUG
    #define CRPT_DBGMSG   printf
#else
    #define CRPT_DBGMSG(...)   do { } while (0)       /* disable debug */
#endif

#if defined(__ICCARM__)
    #pragma diag_suppress=Pm073, Pm143        /* Misra C rule 14.7 */
#endif


/** @addtogroup Standard_Driver Standard Driver
  @{
*/

/** @addtogroup CRYPTO_Driver CRYPTO Driver
  @{
*/


/** @addtogroup CRYPTO_EXPORTED_FUNCTIONS CRYPTO Exported Functions
  @{
*/

/* // @cond HIDDEN_SYMBOLS */

static uint32_t g_AES_au32CTL[1];

/* // @endcond HIDDEN_SYMBOLS */

/**
  * @brief  Open AES encrypt/decrypt function.
  * @param[in]  crpt   Reference to Crypto module.
  * @param[in]  u32Channel   AES channel. Must be 0.
  * @param[in]  u32EncDec    1: AES encode;  0: AES decode
  * @param[in]  u32OpMode    AES operation mode, including:
  *         - \ref AES_MODE_ECB
  *         - \ref AES_MODE_CBC
  *         - \ref AES_MODE_CFB
  *         - \ref AES_MODE_OFB
  *         - \ref AES_MODE_CTR
  *         - \ref AES_MODE_CBC_CS1
  *         - \ref AES_MODE_CBC_CS2
  *         - \ref AES_MODE_CBC_CS3
  * @param[in]  u32KeySize is AES key size, including:
  *         - \ref AES_KEY_SIZE_128
  *         - \ref AES_KEY_SIZE_192
  *         - \ref AES_KEY_SIZE_256
  * @param[in]  u32SwapType is AES input/output data swap control, including:
  *         - \ref AES_NO_SWAP
  *         - \ref AES_OUT_SWAP
  *         - \ref AES_IN_SWAP
  *         - \ref AES_IN_OUT_SWAP
  */
void AES_Open(CRPT_T *crpt, uint32_t u32Channel, uint32_t u32EncDec,
              uint32_t u32OpMode, uint32_t u32KeySize, uint32_t u32SwapType)
{
    crpt->AES_CTL = (u32EncDec << CRPT_AES_CTL_ENCRYPTO_Pos) |
                    (u32OpMode << CRPT_AES_CTL_OPMODE_Pos) |
                    (u32KeySize << CRPT_AES_CTL_KEYSZ_Pos) |
                    (u32SwapType << CRPT_AES_CTL_OUTSWAP_Pos);
    g_AES_au32CTL[u32Channel] = crpt->AES_CTL;
}

/**
  * @brief  Start AES encrypt/decrypt
  * @param[in]  crpt   Reference to Crypto module.
  * @param[in]  u32Channel  AES channel. Must be 0.
  * @param[in]  u32DMAMode  AES DMA control, including:
  *         - \ref CRYPTO_DMA_FIRST   Do first encrypt/decrypt in DMA cascade.
  *         - \ref CRYPTO_DMA_ONE_SHOT   Do one shot encrypt/decrypt with DMA.
  *         - \ref CRYPTO_DMA_CONTINUE   Do continuous encrypt/decrypt in DMA cascade.
  *         - \ref CRYPTO_DMA_LAST       Do last encrypt/decrypt in DMA cascade.
  */
void AES_Start(CRPT_T *crpt, uint32_t u32Channel, uint32_t u32DMAMode)
{
    crpt->AES_CTL = g_AES_au32CTL[u32Channel];
    crpt->AES_CTL |= CRPT_AES_CTL_START_Msk | (u32DMAMode << CRPT_AES_CTL_DMALAST_Pos);
}

/**
  * @brief  Set AES keys
  * @param[in]  crpt   Reference to Crypto module.
  * @param[in]  u32Channel  AES channel. Must be 0.
  * @param[in]  au32Keys    An word array contains AES keys.
  * @param[in]  u32KeySize is AES key size, including:
  *         - \ref AES_KEY_SIZE_128
  *         - \ref AES_KEY_SIZE_192
  *         - \ref AES_KEY_SIZE_256
  */
void AES_SetKey(CRPT_T *crpt, uint32_t u32Channel, uint32_t au32Keys[], uint32_t u32KeySize)
{
    uint32_t  u32Idx, u32Wcnt, u32KeyRegAddr;

    u32KeyRegAddr = (uint32_t)&crpt->AES_KEY[0] + (u32Channel * 0x3CUL);
    u32Wcnt = 4UL + u32KeySize * 2UL;

    for (u32Idx = 0U; u32Idx < u32Wcnt; u32Idx++)
    {
        outpw(u32KeyRegAddr, au32Keys[u32Idx]);
        u32KeyRegAddr += 4UL;
    }
}

/**
  * @brief  Set AES initial vectors
  * @param[in]  crpt   Reference to Crypto module.
  * @param[in]  u32Channel  AES channel. Must be 0.
  * @param[in]  au32IV      A four entry word array contains AES initial vectors.
  */
void AES_SetInitVect(CRPT_T *crpt, uint32_t u32Channel, uint32_t au32IV[])
{
    uint32_t  u32Idx, u32KeyRegAddr;

    u32KeyRegAddr = (uint32_t)&crpt->AES_IV[0] + (u32Channel * 0x3CUL);

    for (u32Idx = 0U; u32Idx < 4U; u32Idx++)
    {
        outpw(u32KeyRegAddr, au32IV[u32Idx]);
        u32KeyRegAddr += 4UL;
    }
}

/**
  * @brief  Set AES DMA transfer configuration.
  * @param[in]  crpt   Reference to Crypto module.
  * @param[in]  u32Channel   AES channel. Must be 0.
  * @param[in]  u32SrcAddr   AES DMA source address
  * @param[in]  u32DstAddr   AES DMA destination address
  * @param[in]  u32TransCnt  AES DMA transfer byte count
  */
void AES_SetDMATransfer(CRPT_T *crpt, uint32_t u32Channel, uint32_t u32SrcAddr,
                        uint32_t u32DstAddr, uint32_t u32TransCnt)
{
    uint32_t  u32RegAddr;

    u32RegAddr = (uint32_t)&crpt->AES_SADDR + (u32Channel * 0x3CUL);
    outpw(u32RegAddr, u32SrcAddr);

    u32RegAddr = (uint32_t)&crpt->AES_DADDR + (u32Channel * 0x3CUL);
    outpw(u32RegAddr, u32DstAddr);

    u32RegAddr = (uint32_t)&crpt->AES_CNT + (u32Channel * 0x3CUL);
    outpw(u32RegAddr, u32TransCnt);
}

/** @} end of group CRYPTO_EXPORTED_FUNCTIONS */

/** @} end of group CRYPTO_Driver */

/** @} end of group Standard_Driver */

/*** (C) COPYRIGHT 2019 Nuvoton Technology Corp. ***/
