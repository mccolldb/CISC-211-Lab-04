/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project. It is intended to
    be used as the starting point for CISC-211 Curiosity Nano Board
    programming projects. After initializing the hardware, it will
    go into a 0.5s loop that calls an assembly function specified in a separate
    .s file. It will print the iteration number and the result of the assembly 
    function call to the serial port.
    As an added bonus, it will toggle the LED on each iteration
    to provide feedback that the code is actually running.
  
    NOTE: PC serial port MUST be set to 115200 rate.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/


// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stdio.h>
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include <malloc.h>
#include <inttypes.h>   // required to print out pointers using PRIXPTR macro
#include "definitions.h"                // SYS function prototypes

void cnano_setup();
void cnano_printf(const char *__restrict format, ...);
#define PERIOD_4S                               4096

// the following array defines pairs of {balance, transaction} values
// tc stands for test case
static int32_t tc[][2] = {
    {         5,          7},  // normal case, no errs
    {       100,        -50},  // normal case, result is +
    {        75,        -99},  // normal case, result is -
    {         0,        -42},  // test with a 0 input
    {       -44,          0},  // test with a 0 input
    {         0,          0},
    {       125,       1000},  // valid transaction amount
    {       126,       1001},  // invalid transaction amount
    {       127,      -1000},  // valid transaction amount
    {       128,      -1001},  // invalid transaction amount
    {0x7FFFFFFF,        120},  // overflow
    {0x80000001,       -100}   // underlow
};

static char * pass = "PASS";
static char * fail = "FAIL";

// VB COMMENT:
// The ARM calling convention permits the use of up to 4 registers, r0-r3
// to pass data into a function. Only one value can be returned to the 
// C caller. The assembly language routine stores the return value
// in r0. The C compiler will automatically use it as the function's return
// value.
//
// Function signature
// for this lab, the function takes one arg (amount), and returns the balance
extern int32_t asmFunc(int32_t);

extern int32_t balance;
extern int32_t transaction;
extern int32_t eat_out;
extern int32_t stay_in;
extern int32_t eat_ice_cream;
extern int32_t we_have_a_problem;


// print the mem addresses of the global vars at startup
// this is to help the students debug their code
static void printGlobalAddresses(void)
{
    // build the string to be sent out over the serial lines
    cnano_printf(
            "========= GLOBAL VARIABLES MEMORY ADDRESS LIST\r\n"
            "global variable \"balance\" stored at address:           0x%" PRIXPTR "\r\n"
            "global variable \"transaction\" stored at address:       0x%" PRIXPTR "\r\n"
            "global variable \"eat_out\" stored at address:           0x%" PRIXPTR "\r\n"
            "global variable \"stay_in\" stored at address:           0x%" PRIXPTR "\r\n"
            "global variable \"eat_ice_cream\" stored at address:     0x%" PRIXPTR "\r\n"
            "global variable \"we_have_a_problem\" stored at address: 0x%" PRIXPTR "\r\n"
            "========= END -- GLOBAL VARIABLES MEMORY ADDRESS LIST\r\n"
            "\r\n",
            (uintptr_t)(&balance), 
            (uintptr_t)(&transaction), 
            (uintptr_t)(&eat_out), 
            (uintptr_t)(&stay_in), 
            (uintptr_t)(&eat_ice_cream), 
            (uintptr_t)(&we_have_a_problem)
            ); 
}


// return failure count. A return value of 0 means everything passed.
static int testResult(int testNum, 
                      int32_t r0Balance, 
                      int32_t *passCount,
                      int32_t *failCount)
{
    // for this lab, each test case corresponds to a single pass or fail
    // But for future labs, one test case may have multiple pass/fail criteria
    // So I'm setting it up this way so it'll work for future labs, too --VB
    *failCount = 0;
    *passCount = 0;
    char *flagsCheck = "OOPS";
    char *memBalCheck = "OOPS";
    char *regBalCheck = "OOPS";
    char *transactionCheck = "OOPS";
    // static char *s2 = "OOPS";
    static bool firstTime = true;
    int32_t myInputBalance = tc[testNum][0];
    int32_t myInputAmount = tc[testNum][1];
    int32_t myCalculatedBalance = tc[testNum][0] + tc[testNum][1];
    int32_t correctBalance = 0;
    int32_t correctInMemTransaction = 0;

    // Check if test case input was out of range
    bool myOutOfRange = false;
    if ( (myInputAmount > 1000 ) || (myInputAmount < -1000))
    {
        myOutOfRange = true;
    }
    
    // check if test case input generated an overflow
    bool myOverflow = false;
    // detect two +'s overflowing to negative
    if ((myInputBalance > 0) && (myInputAmount > 0) && (myCalculatedBalance <= 0))
    {
        myOverflow = true;        
    }
    else if ((myInputBalance < 0) && (myInputAmount < 0) && (myCalculatedBalance >= 0))
    {
        myOverflow = true;
    }
    if ((myOverflow == true) || (myOutOfRange == true))
    {
        correctBalance = myInputBalance;
        correctInMemTransaction = 0;
    }
    else
    {
        correctBalance = myCalculatedBalance;
        correctInMemTransaction = myInputAmount;        
    }

    int32_t myProb = 0;
    int32_t myEatOut = 0;
    int32_t myStayIn = 0;
    int32_t myIceCream = 0;
    // handle the overflow and input-out-of-range-cases
    if (myOverflow == true || myOutOfRange == true)
    {
        // flags test: only we_have_a_problem should be set
        myProb = 1;
        if ((we_have_a_problem == 1) &&
                (eat_out == 0) &&
                (stay_in == 0) &&
                (eat_ice_cream == 0))
        {
            *passCount += 1;
            flagsCheck = pass;
        }
        else
        {
            *failCount += 1;
            flagsCheck = fail;
        }
    }
    else if (correctBalance > 0)  // no errs, and new balance is positive
    {
        myEatOut = 1;
        if ((we_have_a_problem == 0) &&
                (eat_out == 1) &&
                (stay_in == 0) &&
                (eat_ice_cream == 0))
        {
            *passCount += 1;
            flagsCheck = pass;
        }
        else
        {
            *failCount += 1;
            flagsCheck = fail;
        }       
    }
    else if (correctBalance < 0)  // no errs, and new balance is negative
    {
            myStayIn = 1;
            if ((we_have_a_problem == 0) &&
                (eat_out == 0) &&
                (stay_in == 1) &&
                (eat_ice_cream == 0))
        {
            *passCount += 1;
            flagsCheck = pass;
        }
        else
        {
            *failCount += 1;
            flagsCheck = fail;
        }        
    }
    else  // no errs, and new balance is 0
    {
        myIceCream = 1;
        if ((we_have_a_problem == 0) &&
                (eat_out == 0) &&
                (stay_in == 0) &&
                (eat_ice_cream == 1))
        {
            *passCount += 1;
            flagsCheck = pass;
        }
        else
        {
            *failCount += 1;
            flagsCheck = fail;
        }        
        
    }
        
    // balance checks
    if (correctBalance == r0Balance)
    {
        *passCount += 1;
        regBalCheck = pass;
    }
    else                 
    {
        *failCount += 1;
        regBalCheck = fail;
    }
    if (correctBalance == balance)
    {
        *passCount += 1;
        memBalCheck = pass;
    }
    else                 
    {
        *failCount += 1;
        memBalCheck = fail;
    }
    // transaction mem update check
    if (correctInMemTransaction == transaction)
    {
        *passCount += 1;
        transactionCheck = pass;
    }
    else                 
    {
        *failCount += 1;
        transactionCheck = fail;
    }

    
    /* only check the string the first time through */
    if (firstTime == true)
    {
        /* Do first time stuff here, if needed!!!  */        
        firstTime = false; // don't check the strings for subsequent test cases
    }
           
    // build the string to be sent out over the serial lines
    cnano_printf(
            "========= Test Number: %d =========\r\n"
            "test case INPUT: balance:     %11ld\r\n"
            "test case INPUT: transaction: %11ld\r\n"
            "flags pass/fail:       %s\r\n"
            "mem balance pass/fail: %s\r\n"
            "r0 balance pass/fail:  %s\r\n"
            "transaction pass/fail: %s\r\n"
            "debug values                  expected        actual\r\n"
            "we_have_a_problem:.........%11ld   %11ld\r\n"
            "eat_out:...................%11ld   %11ld\r\n"
            "stay_in:...................%11ld   %11ld\r\n"
            "eat_ice_cream:.............%11ld   %11ld\r\n"
            "balance stored in mem:     %11ld   %11ld\r\n"
            "balance returned in r0:    %11ld   %11ld\r\n"
            "transaction stored in mem: %11ld   %11ld\r\n"
            "\r\n",
            testNum,
            myInputBalance,
            myInputAmount,
            flagsCheck,
            memBalCheck,
            regBalCheck,
            transactionCheck,
            myProb, we_have_a_problem,
            myEatOut, eat_out,
            myStayIn, stay_in,
            myIceCream, eat_ice_cream,
            correctBalance, balance,
            correctBalance, r0Balance,
            correctInMemTransaction, transaction
            );

    return *failCount;    
}



// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************
int main ( void )
{
    cnano_setup();
     
    printGlobalAddresses();

    // initialize all the variables
    int32_t passCount = 0;
    int32_t failCount = 0;
    int32_t totalPassCount = 0;
    int32_t totalFailCount = 0;
    int32_t totalTests = 0;
    uint32_t numTestCases = sizeof(tc)/sizeof(tc[0]);
    
    // Loop forever
    while ( true )
    {
        // Do the tests
        for (int testCase = 0; testCase < numTestCases; ++testCase)
        {
            // Toggle the LED to show we're running a new test case
            LED0_Toggle();
           
            // set the balance global variable to the test value
            balance = tc[testCase][0];
            // pass in amount to assembly in r0
            int32_t amount = tc[testCase][1];

            // !!!! THIS IS WHERE YOUR ASSEMBLY LANGUAGE PROGRAM GETS CALLED!!!!
            // Call our assembly function defined in file asmFunc.s
            int32_t newBalance = asmFunc(amount);
            
            // test the result and see if it passed
            failCount = testResult(testCase,newBalance,
                                   &passCount,&failCount);
            totalPassCount += passCount;
            totalFailCount += failCount;
            totalTests = totalPassCount + totalFailCount;

            cnano_printf(
                    "========= In-progress test summary:\r\n"
                    "%ld of %ld tests passed so far...\r\n"
                    "\r\n",
                    totalPassCount, totalTests); 
        } // for each test case
        
        // When all test cases are complete, print the pass/fail statistics
        // Keep looping so that students can see code is still running.
        // We do this in case there are very few tests and they don't have the
        // terminal hooked up in time.
        uint32_t idleCount = 1;
        bool firstTime = true;
        while(true)      // post-test forever loop
        {
            uint32_t numPointsMax = 30;
            uint32_t pointsScored = numPointsMax * totalPassCount / totalTests;
            
            cnano_printf(
                    "========= ALL TESTS COMPLETE: Post-test Idle Cycle Number: %ld\r\n"
                    "Summary of tests: %ld of %ld tests passed\r\n"
                    "Final score for test cases: %ld of %ld points\r\n"
                    "\r\n",
                    idleCount, 
                    totalPassCount, totalTests,
                    pointsScored, numPointsMax); 

            LED0_Toggle();
            ++idleCount;

            // slow down the blink rate after the tests have been executed
            if (firstTime == true)
            {
                firstTime = false; // only execute this section once
                RTC_Timer32Compare0Set(PERIOD_4S); // set blink period to 4sec
                RTC_Timer32CounterSet(0); // reset timer to start at 0
            }
        } // end - post-test forever loop
        break;        // Should never get here...
    } // while ...
            
    /* Execution should not come here during normal operation */
    return ( EXIT_FAILURE );
}
/*******************************************************************************
 End of File
*/

