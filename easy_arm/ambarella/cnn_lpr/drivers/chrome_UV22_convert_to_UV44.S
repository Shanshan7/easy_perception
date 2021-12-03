/*******************************************************************************
 * chrome_UV22_convert_to_UV44.S
 *
 * History:
 *    2019/06/04 - [Ming Wen] created file
 *
 * Copyright (c) 2019 Ambarella International LP
 *
 * This file and its contents ( "Software" ) are protected by intellectual
 * property rights including, without limitation, U.S. and/or foreign
 * copyrights. This Software is also the confidential and proprietary
 * information of Ambarella International LP and its licensors. You may not use, reproduce,
 * disclose, distribute, modify, or otherwise prepare derivative works of this
 * Software or any portion thereof except pursuant to a signed license agreement
 * or nondisclosure agreement with Ambarella International LP or its authorized affiliates.
 * In the absence of such an agreement, you agree to promptly notify and return
 * this Software to Ambarella International LP.
 *
 * This file includes sample code and is only for internal testing and evaluation.  If you
 * distribute this sample code (whether in source, object, or binary code form), it will be
 * without any warranty or indemnity protection from Ambarella International LP or its affiliates.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 * MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL AMBARELLA INTERNATIONAL LP OR ITS AFFILIATES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; COMPUTER FAILURE OR MALFUNCTION; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
******************************************************************************/

.text

/* ARM */
#define IN	X0
#define U_OUT	X1
#define V_OUT	X2
#define ROW	X3
#define COL	X4
#define PITCH	X5
#define CURRENT_ROW	X6
#define CURRENT_COL	X7
#define SPACING	X8

/* NEON */
#define U_DATA	V0
#define V_DATA	V1


/********************************************************************
**
** function: chrome_UV22_convert_to_UV44
**
********************************************************************/

	.align 2
	.global chrome_UV22_convert_to_UV44
	.type	chrome_UV22_convert_to_UV44, %function
chrome_UV22_convert_to_UV44:
	stp		X4, X5, [SP,#-16]!
	stp		X6, X7, [SP,#-16]!
	stp		X8, X9, [SP,#-16]!
	stp		X10, X11, [SP,#-16]!

	mov		X9, X0
	ldp		IN, U_OUT, [X9], #16
	ldp		V_OUT, ROW, [X9], #16
	ldp		COL, PITCH, [X9], #16

	mov		CURRENT_ROW, #0
	sub		SPACING, PITCH, COL

LOOP_ROW:
	cmp		CURRENT_ROW, ROW
	beq		END
	mov		CURRENT_COL, #0

LOOP_COL:
	cmp		CURRENT_COL, COL
	add		X10, IN, SPACING
	add		X11, CURRENT_ROW, #1
	csel	IN, X10, IN, GE
	csel	CURRENT_ROW, X11, CURRENT_ROW, GE
	bge		LOOP_ROW
	ld2 	{U_DATA.B, V_DATA.B}[0] , [IN], #2
	st1 	{U_DATA.B}[0], [U_OUT], #1
	st1 	{U_DATA.B}[0], [U_OUT], #1
	st1 	{V_DATA.B}[0], [V_OUT], #1
	st1 	{V_DATA.B}[0], [V_OUT], #1
	add 	CURRENT_COL, CURRENT_COL, #2

	b		LOOP_COL

END:
	ldp		x10 ,x11 , [sp], #16
	ldp		x8 ,x9 , [sp], #16
	ldp		x6 ,x7 , [sp], #16
	ldp		x4 ,x5 , [sp], #16

	ret		X30

