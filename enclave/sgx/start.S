// Copyright (c) Open Enclave SDK contributors.
// Licensed under the MIT License.

//==============================================================================
//
// _start(): 
//
//     The default entry point for the enclave image.
//
//==============================================================================

.globl _start
.type _start, @function
_start:
.cfi_startproc
    jmp oe_enter
.cfi_endproc

.size _start, .-_start

//==============================================================================
//
// oe_call_link_enclave()
//
//     This function is placed in the same compilation unit as the entry point
//     so that it will be included in the image. It forces a linkage reference 
//     to oe_link_enclave(), which forces various symbols to be included in the
//     enclave image. All these symbols will be available to subsequently
//     linked libraries.
//
//==============================================================================

.globl oe_call_link_enclave
.type oe_call_link_enclave, @function
oe_call_link_enclave:
.cfi_startproc
    call oe_link_enclave
.cfi_endproc

.size oe_call_link_enclave, .-oe_call_link_enclave
