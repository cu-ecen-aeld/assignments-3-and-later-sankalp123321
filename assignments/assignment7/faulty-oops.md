# Error because of faulty kernel module

## Steps to reproduce the error

Run <code> echo “hello_world” > /dev/faulty</code> from the qemu terminal instance.

## Kernel Output   

Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000  
Mem abort info:  
  ESR = 0x96000046  
  EC = 0x25: DABT (current EL), IL = 32 bits  
  SET = 0, FnV = 0  
  EA = 0, S1PTW = 0  
Data abort info:  
  ISV = 0, ISS = 0x00000046  
  CM = 0, WnR = 1  
user pgtable: 4k pages, 39-bit VAs, pgdp=0000000042069000  
[0000000000000000] pgd=000000004207f003, p4d=000000004207f003, pud=000000004207f003, pmd=0000000000000000  
Internal error: Oops: 96000046 [#1] SMP  
Modules linked in: hello(O) faulty(O) scull(O)  
CPU: 0 PID: 152 Comm: sh Tainted: G           O      5.10.7 #1  
Hardware name: linux,dummy-virt (DT)  
pstate: 80000005 (Nzcv daif -PAN -UAO -TCO BTYPE=--)  
pc : faulty_write+0x10/0x20 [faulty]  
lr : vfs_write+0xc0/0x290  
sp : ffffffc010c4bdb0  
x29: ffffffc010c4bdb0 x28: ffffff8002088c80   
x27: 0000000000000000 x26: 0000000000000000   
x25: 0000000000000000 x24: 0000000000000000   
x23: 0000000000000000 x22: ffffffc010c4be30   
x21: 00000000004c9940 x20: ffffff8001fd4300   
x19: 0000000000000012 x18: 0000000000000000   
x17: 0000000000000000 x16: 0000000000000000   
x15: 0000000000000000 x14: 0000000000000000   
x13: 0000000000000000 x12: 0000000000000000   
x11: 0000000000000000 x10: 0000000000000000   
x9 : 0000000000000000 x8 : 0000000000000000   
x7 : 0000000000000000 x6 : 0000000000000000   
x5 : ffffff800222dce8 x4 : ffffffc008677000   
x3 : ffffffc010c4be30 x2 : 0000000000000012   
x1 : 0000000000000000 x0 : 0000000000000000   
Call trace:  
 faulty_write+0x10/0x20 [faulty]  
 ksys_write+0x6c/0x100  
 __arm64_sys_write+0x1c/0x30  
  el0_svc_common.constprop.0+0x9c/0x1c0  
 do_el0_svc+0x70/0x90  
 el0_svc+0x14/0x20  
 el0_sync_handler+0xb0/0xc0  
 el0_sync+0x174/0x180  
Code: d2800001 d2800000 d503233f d50323bf (b900003f)    
---[ end trace d3a7c1643fab6a14 ]---     

## Analysis

The Kernel dump tells us a lot of things about where and why the error occoured.  
The kerel dump points to various aspects of thr memory. A lot of this is platform dpenedent and this makes sens because the app runs on a aprticular architecture.   
ISA might not be same around different architectures. In the kernel dump above we can see that the we get an error saying <code>"Unable to handle kernel NULL pointer dereference at virtual address 0000000000000000" </code>    
This says that the dump was because of dereferencing a null pointer at address 0.  
The dump says <code>Internal error: Oops: 96000046 [#1] SMP</code> #1 means <code>Internal error: Oops: 96000046 [#1] SMP</code>.  
It points out to CPU 0 with PID 152 which crashed.   
The dump points to all the intermediate registers of the architecture and what thier values were at the time of the crash. The PC points to <code>faulty_write+0x10/0x20 [faulty]</code> which means that the error occoured on faulty_write+0x10 address which is the address which caused the null pointer dereference on address 0. The pgd p4d pmd are the page tables.    

The call stack shows how each function is executed and what address relative to the function name is instrunction being executed, i.e. the line at which the fault occoured. The the flow of a call stack is reverse, the faulty_write is the latest function which is executed. And faulty_write+0x10 instruction causes the crash and kernel dump.
