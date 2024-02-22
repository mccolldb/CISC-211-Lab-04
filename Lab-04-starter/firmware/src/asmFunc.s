/*** asmFunc.s   ***/
/* Tell the assembler to allow both 16b and 32b extended Thumb instructions */
.syntax unified

#include <xc.h>

/* Tell the assembler that what follows is in data memory    */
.data
.align
 
/* define and initialize global variables that C can access */

.global balance,transaction,eat_out,stay_in,eat_ice_cream,we_have_a_problem
.type balance,%gnu_unique_object
.type transaction,%gnu_unique_object
.type eat_out,%gnu_unique_object
.type stay_in,%gnu_unique_object
.type eat_ice_cream,%gnu_unique_object
.type we_have_a_problem,%gnu_unique_object

/* NOTE! These are only initialized ONCE, right before the program runs.
 * If you want these to be 0 every time asmFunc gets called, you must set
 * them to 0 at the start of your code!
 */
balance:           .word     0  /* input/output value */
transaction:       .word     0  /* output value */
eat_out:           .word     0  /* output value */
stay_in:           .word     0  /* output value */
eat_ice_cream:     .word     0  /* output value */
we_have_a_problem: .word     0  /* output value */

 /* Tell the assembler that what follows is in instruction memory    */
.text
.align


    
/********************************************************************
function name: asmFunc
function description:
     output = asmFunc ()
     
where:
     output: the integer value returned to the C function
     
     function description: The C call ..........
     
     notes:
        None
          
********************************************************************/    
.global asmFunc
.type asmFunc,%function
asmFunc:   

    /* save the caller's registers, as required by the ARM calling convention */
    push {r4-r11,LR}
 
    
    /*** STUDENTS: Place your code BELOW this line!!! **************/
    /* incoming R0 contains transaction amount */
    /* clear all flags */
    mov r2,0
    LDR r1,=eat_out
    STR r2,[r1]
    LDR r1,=stay_in
    STR r2,[r1]
    LDR r1,=eat_ice_cream
    STR r2,[r1]
    LDR r1,=we_have_a_problem
    STR r2,[r1]
    
    LDR r1,=transaction /* r1 has address of transaction amount */
    STR r0,[r1]	        /* store transaction amount */
    CMP r0,#1000
    BGT problem
    CMP r0,#-1000
    BLT problem
    
    LDR r1,=balance  /* r1 has address of balance */
    LDR r2,[r1]      /* r2 has incoming balance */
    ADD r2, r2, r0   /* update balance value */
    BVS problem      /* check for overflow */
    STR r2, [r1]     /* store updated balance using r1 address */
    
    CMP r2,#0
    BGT set_eat_out
    BLT set_stay_in
    LDR r1,=eat_ice_cream
    B set_return
set_eat_out:
    LDR r1,=eat_out
    B set_return
set_stay_in:
    LDR r1,=stay_in
set_return:
    MOV r0,1
    STR r0,[r1]  /* store flag */
    LDR r0,=balance
    LDR r0,[r0]  /* r0 has outgoing balance */
    b done
    
problem:
    mov r0,0	    /* zero transaction amount */
    LDR r1,=transaction /* r1 has address of transaction amount */
    STR r0,[r1]	    /* store transaction amount in transaction */
    
    mov r0,1
    LDR r1,=we_have_a_problem
    STR r0,[r1]    /* store 1 in we_have_a_problem */
    LDR r0,=balance
    LDR r0,[r0]  /* r0 has outgoing balance */
    b done
    /*** STUDENTS: Place your code ABOVE this line!!! **************/

done:    
    /* restore the caller's registers, as required by the 
     * ARM calling convention 
     */
    pop {r4-r11,LR}

    mov pc, lr	 /* asmFunc return to caller */
   

/**********************************************************************/   
.end  /* The assembler will not process anything after this directive!!! */
           




