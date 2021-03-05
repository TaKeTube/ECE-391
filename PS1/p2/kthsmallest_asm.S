/*
 * tab:2
 *
 * kthsmallest_asm.S - Implementation of Assembly QuickSelect
 *
 * Permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without fee, and without written agreement is
 * hereby granted, provided that the above copyright notice and the following
 * two paragraphs appear in all copies of this software.
 *
 * IN NO EVENT SHALL THE AUTHOR OR THE UNIVERSITY OF ILLINOIS BE LIABLE TO
 * ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
 * DAMAGES ARISING OUT  OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION,
 * EVEN IF THE AUTHOR AND/OR THE UNIVERSITY OF ILLINOIS HAS BEEN ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * THE AUTHOR AND THE UNIVERSITY OF ILLINOIS SPECIFICALLY DISCLAIM ANY
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE
 * PROVIDED HEREUNDER IS ON AN "AS IS" BASIS, AND NEITHER THE AUTHOR NOR
 * THE UNIVERSITY OF ILLINOIS HAS ANY OBLIGATION TO PROVIDE MAINTENANCE,
 * SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS."
 *
 * Author:        Harsh Agrawal
 * Version:       1
 * Creation Date: Tue Jan 26 2021
 * Filename:      kthsmallest_asm.S
 * History:
 *    HA    1    Tue Jan 26 2021
 *        First written.
 */

.text
.global kthSmallest_asm

# kthsmallest ASM (Assembly)
# Finds the kth smallest element in an array
# Declaration is in kthsmallest.h
#
# Registers:
#   eax - Return Value
#   ecx - right, j, i (for index of array)
#   edx - pivot, arr, pointer
kthSmallest_asm:
   pushl %ebp
   movl %esp, %ebp

   #--- YOUR CODE HERE ---
   # allocate 3 local variables
   subl $12, %esp
   # initialize local variables
   movl 16(%ebp), %ecx           # take right
   movl 8(%ebp), %edx            # take &arr
   movl (%edx, %ecx, 4), %edx    # take arr[right]
   movl %edx, -4(%ebp)           # set pivot
   movl 12(%ebp), %ecx           # take left
   movl %ecx, -8(%ebp)           # set i
   movl %ecx, -12(%ebp)          # set j

compare_loop:
   movl 16(%ebp), %ecx     # take right
   cmpl -12(%ebp), %ecx    # compare j and right
   jle break_loop          # j >= right

   movl -12(%ebp), %ecx          # take j
   movl 8(%ebp), %edx            # take &arr
   movl (%edx, %ecx, 4), %edx    # take arr[j]
   cmpl %edx, -4(%ebp)           # complare arr[j] and pivot
   jl repeat # arr[j] > pivot

   movl -12(%ebp), %ecx          # take j
   movl 8(%ebp), %edx            # take &arr
   leal (%edx, %ecx, 4), %edx    # set pointer &arr[j]
   pushl %edx # push &arr[j]
   movl -8(%ebp), %ecx           # take i
   movl 8(%ebp), %edx            # take &arr
   leal (%edx, %ecx, 4), %edx    # set pointer &arr[i]
   pushl %edx # push &arr[i]
   call swap
   addl $8, %esp

   incl -8(%ebp) # i++

repeat:
   incl -12(%ebp) # j++
   jmp compare_loop

break_loop:
   movl 16(%ebp), %ecx           # take right
   movl 8(%ebp), %edx            # take &arr
   leal (%edx, %ecx, 4), %edx    # set pointer &arr[right]
   pushl %edx                    # push &arr[right]
   movl -8(%ebp), %ecx           # take i
   movl 8(%ebp), %edx            # take &arr
   leal (%edx, %ecx, 4), %edx    # set pointer &arr[i]
   pushl %edx                    # push &arr[i]
   call swap
   addl $8, %esp

   movl -8(%ebp), %ecx # take i
   cmpl 20(%ebp), %ecx # compare k,i
   je equal # k == i
   jg great # k < i
   jmp less # else

equal:
   movl 20(%ebp), %ecx        # take k
   movl 8(%ebp), %edx         # take &arr
   movl (%edx, %ecx, 4), %eax # return arr[k]
   jmp finish

great:
   pushl 20(%ebp) # push k
   movl -8(%ebp), %ecx # take i
   decl %ecx # i--
   pushl %ecx # push i-1
   pushl 12(%ebp) # push left
   pushl 8(%ebp) # push arr
   call kthSmallest_asm
   addl $16, %esp
   jmp finish

less:
   pushl 20(%ebp) # push k
   pushl 16(%ebp) # push right
   movl -8(%ebp), %ecx # take i
   incl %ecx # i++
   pushl %ecx # push i+1
   pushl 8(%ebp) # push arr
   call kthSmallest_asm
   addl $16, %esp
   jmp finish

finish:
   movl %ebp, %esp
   leave
   ret
   #----------------------
