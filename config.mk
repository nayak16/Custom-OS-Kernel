###########################################################################
#
#    #####          #######         #######         ######            ###
#   #     #            #            #     #         #     #           ###
#   #                  #            #     #         #     #           ###
#    #####             #            #     #         ######             #
#         #            #            #     #         #
#   #     #            #            #     #         #                 ###
#    #####             #            #######         #                 ###
#
#
# Please read the directions in README and in this config.mk carefully.
# Do -N-O-T- just dump things randomly in here until your kernel builds.
# If you do that, you run an excellent chance of turning in something
# which can't be graded.  If you think the build infrastructure is
# somehow restricting you from doing something you need to do, contact
# the course staff--don't just hit it with a hammer and move on.
#
# [Once you've read this message, please edit it out of your config.mk]
# [Once you've read this message, please edit it out of your config.mk]
# [Once you've read this message, please edit it out of your config.mk]
###########################################################################

###########################################################################
# This is the include file for the make file.
# You should have to edit only this file to get things to build.
###########################################################################

###########################################################################
# Tab stops
###########################################################################
# If you use tabstops set to something other than the international
# standard of eight characters, this is your opportunity to inform
# our print scripts.
TABSTOP = 8

###########################################################################
# The method for acquiring project updates.
###########################################################################
# This should be "afs" for any Andrew machine, "web" for non-andrew machines
# and "offline" for machines with no network access.
#
# "offline" is strongly not recommended as you may miss important project
# updates.
#
UPDATE_METHOD = afs

###########################################################################
# WARNING: When we test your code, the two TESTS variables below will be
# blanked.  Your kernel MUST BOOT AND RUN if 410TESTS and STUDENTTESTS
# are blank.  It would be wise for you to test that this works.
###########################################################################

###########################################################################
# Test programs provided by course staff you wish to run
###########################################################################
# A list of the test programs you want compiled in from the 410user/progs
# directory.
#
410TESTS = getpid_test1 ck1 loader_test1 knife new_pages remove_pages_test1 remove_pages_test2 exec_basic exec_basic_helper exec_nonexist readline_basic sleep_test1 fork_wait fork_wait_bomb fork_exit_bomb actual_wait mem_permissions make_crash make_crash_helper wait_getpid wild_test1 print_basic cho cho2 yield_desc_mkrun swexn_basic_test swexn_cookie_monster swexn_dispatch swexn_regs stack_test1 swexn_uninstall_test swexn_stands_for_swextensible cat minclone_mem

###########################################################################
# Test programs you have written which you wish to run
###########################################################################
# A list of the test programs you want compiled in from the user/progs
# directory.
#
STUDENTTESTS = test_deschedule test_readline test_sleep test_thr_create test_cyclone test_agility_drill test_paraguay test_startle

###########################################################################
# Data files provided by course staff to build into the RAM disk
###########################################################################
# A list of the data files you want built in from the 410user/files
# directory.
#
410FILES =

###########################################################################
# Data files you have created which you wish to build into the RAM disk
###########################################################################
# A list of the data files you want built in from the user/files
# directory.
#
STUDENTFILES = dog.txt shrek.txt donkey.txt

###########################################################################
# Object files for your thread library
###########################################################################
THREAD_OBJS = malloc.o panic.o mutex.o cond.o asm_helpers.o thread.o ll.o sem.o rwlock.o
# Thread Group Library Support.
#
# Since libthrgrp.a depends on your thread library, the "buildable blank
# P3" we give you can't build libthrgrp.a.  Once you install your thread
# library and fix THREAD_OBJS above, uncomment this line to enable building
# libthrgrp.a:
410USER_LIBS_EARLY += libthrgrp.a

###########################################################################
# Object files for your syscall wrappers
###########################################################################
SYSCALL_OBJS = syscall_fork.o syscall_exec.o syscall_set_status.o syscall_vanish.o syscall_wait.o syscall_task_vanish.o syscall_gettid.o syscall_yield.o syscall_deschedule.o syscall_make_runnable.o syscall_get_ticks.o syscall_sleep.o syscall_swexn.o syscall_new_pages.o syscall_remove_pages.o syscall_getchar.o syscall_readline.o syscall_print.o syscall_set_term_color.o syscall_set_cursor_pos.o syscall_get_cursor_pos.o syscall_readfile.o syscall_halt.o syscall_misbehave.o

###########################################################################
# Object files for your automatic stack handling
###########################################################################
AUTOSTACK_OBJS = autostack.o

###########################################################################
# Parts of your kernel
###########################################################################
#
# Kernel object files you want included from 410kern/
#
410KERNEL_OBJS = load_helper.o
#
# Kernel object files you provide in from kern/
#
KERNEL_OBJS = console.o kernel.o loader/loader.o malloc_wrappers.o install_handlers.o debug.o asm_helpers.o keyboard.o \
handlers/syscall_handler_wrappers.o handlers/exception_handler_wrappers.o handlers/thr_mgmt_handlers.o handlers/life_cycle_handlers.o handlers/exception_handlers.o handlers/console_io_handlers.o handlers/misc_handlers.o handlers/peripheral_handler_wrappers.o handlers/peripheral_handlers.o handlers/mem_mgmt_handlers.o \
virtual_mem_mgmt/page_directory.o virtual_mem_mgmt/frame_manager.o virtual_mem_mgmt/mem_section.o virtual_mem_mgmt/virtual_mem_mgmt.o\
data_structures/ll.o data_structures/queue.o data_structures/circ_buffer.o data_structures/ht.o data_structures/stack.o data_structures/cleanup.o \
special_register_cntrl/spec_reg_wrappers.o special_register_cntrl/asm_functions.o \
scheduler/pcb.o scheduler/scheduler.o scheduler/tcb.o scheduler/tcb_pool.o scheduler/thr_helpers.o scheduler/thr_helpers_wrappers.o \
dispatcher/dispatcher.o dispatcher/asm_helpers.o \
locks/mutex.o locks/sem.o\

###########################################################################
# WARNING: Do not put **test** programs into the REQPROGS variables.  Your
#          kernel will probably not build in the test harness and you will
#          lose points.
###########################################################################

###########################################################################
# Mandatory programs whose source is provided by course staff
###########################################################################
# A list of the programs in 410user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# The shell is a really good thing to keep here.  Don't delete idle
# or init unless you are writing your own, and don't do that unless
# you have a really good reason to do so.
#
410REQPROGS = idle init shell

###########################################################################
# Mandatory programs whose source is provided by you
###########################################################################
# A list of the programs in user/progs which are provided in source
# form and NECESSARY FOR THE KERNEL TO RUN.
#
# Leave this blank unless you are writing custom init/idle/shell programs
# (not generally recommended).  If you use STUDENTREQPROGS so you can
# temporarily run a special debugging version of init/idle/shell, you
# need to be very sure you blank STUDENTREQPROGS before turning your
# kernel in, or else your tweaked version will run and the test harness
# won't.
#
STUDENTREQPROGS =
