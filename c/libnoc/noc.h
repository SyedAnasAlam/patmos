/*
   Copyright 2014 Technical University of Denmark, DTU Compute. 
   All rights reserved.
   
   This file is part of the time-predictable VLIW processor Patmos.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

      1. Redistributions of source code must retain the above copyright notice,
         this list of conditions and the following disclaimer.

      2. Redistributions in binary form must reproduce the above copyright
         notice, this list of conditions and the following disclaimer in the
         documentation and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER ``AS IS'' AND ANY EXPRESS
   OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN
   NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
   DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

   The views and conclusions contained in the software and documentation are
   those of the authors and should not be interpreted as representing official
   policies, either expressed or implied, of the copyright holder.
 */

/** \addtogroup libnoc
 *  @{
 */

/**
 * \file noc.h Definitions for libnoc.
 * 
 * \author Wolfgang Puffitsch <wpuffitsch@gmail.com>
 * \author Rasmus Bo Soerensen <rasmus@rbscloud.dk>
 *
 * \brief Low-level NoC communication library for the T-CREST platform.
 */

#ifndef _NOC_H_
#define _NOC_H_

#include <machine/patmos.h>
#include <machine/spm.h>
#include "coreset.h"

//#define DEBUG

#ifdef DEBUG

/// \brief Print message if (a) DEBUG is defined and (b) executing on master core.
#define DEBUGGER(...)                           \
  do {                                          \
    if (get_cpuid() == NOC_MASTER) {            \
      printf(__VA_ARGS__);                      \
    }                                           \
  } while(0)

/// \brief Abort if (a) DEBUG is defined and (b) condition X evaluates to true.
#define DEBUG_CORECHECK(X)                      \
  do {                                          \
    if(X) {                                     \
      abort();                                  \
    }                                           \
  } while(0)

#else
/// \brief Print message if (a) DEBUG is defined and (b) executing on master core.
#define DEBUGGER(...)
/// \brief Abort if (a) DEBUG is defined and (b) condition is true.
#define DEBUG_CORECHECK(x)
#endif

///////////////////////////////////////////////////////////////////////////////
// Definitions of NoC packet types
///////////////////////////////////////////////////////////////////////////////
#define DATA_PKT_TYPE 0
#define CONFIG_PKT_TYPE 1
#define IRQ_PKT_TYPE 3

#define NOC_PTR_WIDTH 14

///////////////////////////////////////////////////////////////////////////////
// Definitions used for initialization of network interface
///////////////////////////////////////////////////////////////////////////////

/// \brief The number of cores on the platform.
///
/// Typically generated by Poseidon.
extern const int NOC_CORES;

/// \brief The number of tables for Noc configuration.
///
/// Typically generated by Poseidon.
extern const int NOC_TABLES;

/// \brief The number of entries in the schedule table.
///
/// Typically generated by Poseidon.
extern const int NOC_SCHEDULE_ENTRIES;

/// \brief The array for initialization data.
///
/// Typically generated by Poseidon.
extern const int noc_init_array [];

/// \brief The master core, which governs booting and startup synchronization.
///
/// Typically defined by the application.
extern const int NOC_MASTER;

/// \brief Configure network interface according to initialization
/// information in #noc_init_array.
void noc_configure(void);

/// \brief Configure network-on-chip and synchronize all cores.
///
/// #noc_init is a static constructor and not intended to be called directly.
void noc_init(void) __attribute__((constructor(101),used));

#ifdef DOXYGEN
/// \brief Define this before including noc.h to force the use
/// of #noc_init as constructor. NOC_INIT does not need to be defined
/// if any functions from libnoc are used.
#define NOC_INIT
#endif

#ifdef NOC_INIT
// Pull in initializer, even if nothing else from the library is used
/// \cond PRIVATE
static const void * volatile __noc_include __attribute__((used)) = &noc_init;
/// \endcond
#endif

///////////////////////////////////////////////////////////////////////////////
// Exception handler functions
///////////////////////////////////////////////////////////////////////////////

// \brief Exception handler that is called when an interrupt from a remote
// is received
void __remote_irq_handler(void);

// \brief Exception handler that is called when the last packet of a NoC
// transfer is received
void __data_recv_handler(void);

// \brief Exception handler that is called when the application traps to call
// a function that directly access the NI hardware
void __noc_trap_handler(void);

///////////////////////////////////////////////////////////////////////////////
// Functions for transmitting data
///////////////////////////////////////////////////////////////////////////////

/// \brief Start a NoC data dma transfer.
///
/// The addresses and the size are in double-words and relative to the
/// communication SPM base #NOC_SPM_BASE.
/// \param dma_id The core id of the receiver.
/// \param write_ptr The address in the receiver's communication SPM,
/// in double-words, relative to #NOC_SPM_BASE.
/// \param read_ptr The address in the sender's communication SPM, in
/// double-words, relative to #NOC_SPM_BASE.
/// \param size The size of data to be transferred, in double-words.
/// \retval 1 Sending was successful.
/// \retval 0 Otherwise.
int k_noc_dma(unsigned dma_id, unsigned short write_ptr,
            unsigned short read_ptr, unsigned short size);
int noc_dma(unsigned dma_id, unsigned short write_ptr,
            unsigned short read_ptr, unsigned short size);

/// \brief Start a NoC configure transfer.
///
/// The addresses and the size are in double-words and relative to the
/// communication SPM base #NOC_SPM_BASE.
/// \param dma_id The core id of the receiver.
/// \param write_ptr The address in the receiver's communication SPM,
/// in double-words, relative to #NOC_SPM_BASE.
/// \param read_ptr The address in the sender's communication SPM, in
/// double-words, relative to #NOC_SPM_BASE.
/// \param size The size of data to be transferred, in double-words.
/// \retval 1 Sending was successful.
/// \retval 0 Otherwise.
int noc_conf(unsigned dma_id, unsigned short write_ptr,
            unsigned short read_ptr, unsigned short size);

/// \brief Start a NoC interrupt.
///
/// The addresses and the size are in double-words and relative to the
/// communication SPM base #NOC_SPM_BASE.
/// \param dma_id The core id of the receiver.
/// \param write_ptr The address in the receiver's communication SPM,
/// in double-words, relative to #NOC_SPM_BASE.
/// \param read_ptr The address in the sender's communication SPM, in
/// double-words, relative to #NOC_SPM_BASE.
/// \retval 1 Sending was successful.
/// \retval 0 Otherwise.
int noc_irq(unsigned dma_id, unsigned short write_ptr,
            unsigned short read_ptr);

/// \brief Check if a NoC transfer has finished.
///
/// \param dma_id The core id of the receiver.
/// \retval 1 The transfer has finished.
/// \retval 0 Otherwise.
int k_noc_done(unsigned dma_id);
int noc_done(unsigned dma_id);

/// \brief Attempt to transfer data via the NoC (non-blocking).
///
/// The addresses and the size are absolute and in bytes.
/// \param dma_id The core id of the receiver.
/// \param dst A pointer to the destination of the transfer.
/// \param src A pointer to the source of the transfer.
/// \param size The size of data to be transferred, in bytes.
/// \retval 1 Sending was successful.
/// \retval 0 Otherwise.
int noc_nbsend(unsigned dma_id, volatile void _SPM *dst,
               volatile void _SPM *src, size_t size);

/// \brief Transfer data via the NoC (blocking).
///
/// The addresses and the size are absolute and in bytes.
/// \param dma_id The core id of the receiver.
/// \param dst A pointer to the destination of the transfer.
/// \param src A pointer to the source of the transfer.
/// \param size The size of data to be transferred, in bytes.
void noc_send(unsigned dma_id, volatile void _SPM *dst,
              volatile void _SPM *src, size_t size);

/// \brief Multi-cast transfer of data via the NoC (blocking).
///
/// The addresses and the size are absolute and in bytes.
/// \param cnt The number of receivers.
/// \param dma_id An array with the core ids of the receivers.
/// \param dst An array with pointers to the destinations of the transfer.
/// \param src A pointer to the source of the transfer.
/// \param size The size of data to be transferred, in bytes.
void noc_multisend(unsigned cnt, unsigned dma_id [], volatile void _SPM *dst [],
                   volatile void _SPM *src, size_t size);

/// \brief Multi-cast transfer of data like #noc_multisend(), but with coreset
/// and a single destination address.
///
/// The addresses and the size are absolute and in bytes.
/// \param receivers The set of receivers.
/// \param dst An array with pointers to the destinations of the transfer.
/// \param offset Common offset for the destination addresses.
/// \param src A pointer to the source of the transfer.
/// \param size The size of data to be transferred, in bytes.
void noc_multisend_cs(coreset_t *receivers, volatile void _SPM *dst[],
                      unsigned offset, volatile void _SPM *src, size_t size);

/// \brief Wait until all transfers to a set of receivers have finished.
///
/// \param receivers The set of receivers.
void noc_wait_dma(coreset_t receivers);

///////////////////////////////////////////////////////////////////////////////
// Definitions for setting up a transfer
///////////////////////////////////////////////////////////////////////////////

/// The flag to mark a DMA entry as valid
#define NOC_ACTIVE_BIT 0x80000000

///////////////////////////////////////////////////////////////////////////////
// Definitions for address mapping
///////////////////////////////////////////////////////////////////////////////
#define OFFSET_WIDTH  (11+2)
#define BANK(ID)      (ID<<OFFSET_WIDTH)

#define DMA_BANK      BANK(0)
#define SCHED_BANK    BANK(1)
#define TDM_BANK      BANK(2)
#define MC_BANK       BANK(3)
#define IRQ_BANK      BANK(4)

/// The base address for DMA entries
#define NOC_DMA_BASE    ((volatile int _IODEV *)(0xE0000000+DMA_BANK))
/// The base address for DMA routing information
#define NOC_SCHED_BASE  ((volatile int _IODEV *)(0xE0000000+SCHED_BANK))
/// The base address for the slot table
#define NOC_TDM_BASE    ((volatile int _IODEV *)(0xE0000000+TDM_BANK))
/// The base address for the slot table
#define NOC_MC_BASE     ((volatile int _IODEV *)(0xE0000000+MC_BANK))
/// The base address for the slot table
#define NOC_IRQ_BASE    ((volatile int _IODEV *)(0xE0000000+IRQ_BANK))
/// The base address of the communication SPM
#define NOC_SPM_BASE    ((volatile int _SPM   *)0xE8000000)

#endif /* _NOC_H_ */
/** @}*/
