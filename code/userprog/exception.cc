// exception.cc
//      Entry point into the Nachos kernel from user programs.
//      There are two kinds of things that can cause control to
//      transfer back to here from user code:
//
//      syscall -- The user code explicitly requests to call a procedure
//      in the Nachos kernel.  Right now, the only function we support is
//      "Halt".
//
//      exceptions -- The user code does something that the CPU can't handle.
//      For instance, accessing memory that doesn't exist, arithmetic errors,
//      etc.
//
//      Interrupts (which can also cause control to transfer from user
//      code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "synchconsole.h"
//TODO uncomment  when do_UserThreadCreate is done
//#include "userthread.h"

extern SynchConsole *synchconsole;

//----------------------------------------------------------------------
// UpdatePC : Increments the Program Counter register in order to resume
// the user program immediately after the "syscall" instruction.
//----------------------------------------------------------------------
static void
UpdatePC ()
{
    int pc = machine->ReadRegister (PCReg);
    machine->WriteRegister (PrevPCReg, pc);
    pc = machine->ReadRegister (NextPCReg);
    machine->WriteRegister (PCReg, pc);
    pc += 4;
    machine->WriteRegister (NextPCReg, pc);
}
//----------------------------------------------------------------------
// Here are all the syscall functions we call with the sycall type switch 
//----------------------------------------------------------------------
void do_Halt()
{
	DEBUG ('m', "Shutdown, initiated by user program.\n");
	interrupt->Halt ();
}
//----------------------//
void do_Exit()
{
	DEBUG('m', "Exit program, return code exit(%d)\n", machine->ReadRegister(4));
	// Stop current thread
	AddrSpace::nbProcess --;
	if ( AddrSpace::nbProcess == 0 )
	{
		interrupt->Halt ();
	}
	else
	{
		currentThread->Finish();
	}
}
//----------------------//
void do_Putchar()
{
    char c = (char)machine->ReadRegister(4);//order of the bit endian
    DEBUG('a', "Putchar \n", machine->ReadRegister(4));
    synchconsole->SynchPutChar(c);
}
//----------------------//
void do_Getchar()
{
    int c = synchconsole->SynchGetChar();//change to int to make it work to EOF
    machine->WriteRegister(2,c);
    DEBUG('a', "Getchar %c\n",c);
}
//----------------------//
void do_Putstring()
{
    int from = machine->ReadRegister(4);
    char* c = new char[MAX_STRING_SIZE + 1];
    int really_write = copyStringFromMachine(from,c,MAX_STRING_SIZE);
    c[really_write] = '\0';
    DEBUG('a', "Putstring %s\n", c);
    synchconsole->SynchPutString(c);
    delete [] c;
}
//----------------------//
void do_Getstring()
{
    int n = (int)machine->ReadRegister(5);
    char* buffer = new char[n];
    if (synchconsole->SynchGetString(buffer, n) == NULL)
    {
        machine->WriteRegister(2, (int)NULL);
    }
    else
    {
        //copy buffer to string
        copyStringToMachine(machine->ReadRegister(4), buffer, n);
        machine->WriteRegister(2, machine->ReadRegister(4));
        DEBUG('a', "GetString %s\n", buffer);
    }

    delete buffer;
}
//----------------------//
void do_Putint()
{
    int num = machine->ReadRegister(4);
    synchconsole->SynchPutInt(num);
    DEBUG('a', "PutInt %d\n", num);
}
//----------------------//
void do_Getint()
{
    int p =  machine->ReadRegister(4);
    int num;

    //Try to write at @p before consume input
    if(!machine->WriteMem(p, sizeof(int), (int)0))
    {
        //-2 convention for non valid adress memory
        machine->WriteRegister(2,-2);
        DEBUG('a', "GetInt : bad adress %d\n", p);
        return;
    }
    int error_value = synchconsole->SynchGetInt(&num);
    machine->WriteRegister(2,error_value);
    machine->WriteMem(p, sizeof(int), num);
    DEBUG('a', "GetInt %d\n", error_value);
}
//----------------------//
void do_UserThreadCreate()
{
	int fn = machine->ReadRegister(4);
	int arg = machine->ReadRegister(5);
	DEBUG('t', "Create user thread on function at address %i and arg at address %i\n", fn, arg);
	//TODO uncomment  when do_UserThreadCreate is done
	/*
	if(do_UserThreadCreate(fn, arg)) 
	{
		//creation failed
		DEBUG('t', "Syscall failed to create a new user thread\n");
		machine->WriteRegister(2,-1);
	}
	else
	{
		//creation succeed
		machine->WriteRegister(2,0);
	}
	*/
}
//----------------------//
void do_UserThreadJoin()
{
	//TODO
	//must be synchronous to use the children variable
	//a thread should have a variable to count his children
	//this thread go in sleep mode if children!=0
	//else nothing
}
//----------------------//
void do_UserThreadExit()
{
	//TODO
	//must be synchronous to modifiy the children number and the state of the parent
	//decrease the children number of the parent
	//if children==0 set the parent to readyToRun, the scheduler can now restart this thread
}
//----------------------------------------------------------------------
// ExceptionHandler
//      Entry point into the Nachos kernel.  Called when a user program
//      is executing, and either does a syscall, or generates an addressing
//      or arithmetic exception.
//
//      For system calls, the following is the calling convention:
//
//      system call code -- r2
//              arg1 -- r4
//              arg2 -- r5
//              arg3 -- r6
//              arg4 -- r7
//
//      The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//      "which" is the kind of exception.  The list of possible exceptions
//      are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler (ExceptionType which)
{
    int type = machine->ReadRegister (2);

    switch (which)
    {
        /**
         * Handling syscall exceptions
         **/
        case SyscallException:
        {
            switch (type)
            {
                case SC_Halt:
                {
					do_Halt();
                    break;
                }
                case SC_Exit:
                {
					do_Exit();
                    break;
                }
                case SC_PutChar:
                {
                    do_Putchar();
                    break;
                }
                case SC_GetChar:
                {
					do_Getchar();
                    break;
                }
                case SC_PutString:
                {
					do_Putstring();
                    break;
                }
                case SC_GetString:
                {
					do_Getstring();
                    break;
                }
                case SC_PutInt:
                {
					do_Putint();
                    break;
                }
                case SC_GetInt:
                {
					do_Getint();
                    break;
                }
                case SC_ThreadCreate:
                {
					do_UserThreadCreate();
					break;
                }
                case SC_ThreadJoin:
                {
					do_UserThreadJoin();
					break;
                }
                case SC_ThreadExit:
                {
					do_UserThreadExit();
					break;
                }
                default:
                {
                    printf ("Unexpected syscall type %d\n", type);
                    ASSERT (FALSE);
                }
            }
            break;
        }
        /**
         * TODO : handle other type of exception
         **/
        default:
        {
            printf ("Unexpected user mode exception %d %d\n", which, type);
            ASSERT (FALSE);
        }
    }

    // LB: Do not forget to increment the pc before returning!
    UpdatePC ();
    // End of addition
}
