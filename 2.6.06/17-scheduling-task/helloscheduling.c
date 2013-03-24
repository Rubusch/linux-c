// helloscheduling.c
/*
  counts the timerinterrupts and shows how often the timer 
  already was called at doing an "cat" on the /proc interface

  "void" declaration for empty function parameters necessary
  to avoid warnings!

  "static" declaration for variables and functions necessary 
  to avoid namespace conflicts with other functions by the same 
  name (in same "common" namespace!).

  init_module(void) and cleanup_module(void) implementations 
  must not have "static" return types!

  C90 conformity: declarations of variables have to be made at 
  begin of each block (a function body is a block!)

  declaration follows the C90 standard
//*/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/interrupt.h>


// some work_queue related functions are just available to GPL licensed Modules
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("counts the timerinterrupts and shows how often the timer already was called at doing an \"cat\" on the /proc interface");


/*
  defines / macros
//*/


#define PROC_ENTRY_FILENAME "Lothars_sched_stat"
#define WORK_QUEUE_NAME "WQsched.c"
#define READ_BUF_SIZ 80


/*
  prototypes
//*/


int init_module(void);
void cleanup_module(void);

static void intrpt_routine(void*);
static void intrpt_routine(void*);
ssize_t procfile_read(char*, char**, off_t, int, int*, void*);


/*
  globals
//*/


// proc file pointer
struct proc_dir_entry* Proc_file;

// number of times the timer interrupt has been called so far
static int Timer_intrpt = 0;

// set "die" to 1, for shutdown
static int Die = 0;

// work queue structure for this task, from workqueue.h
static struct workqueue_struct *Work_queue;

static struct work_struct Task;

static DECLARE_WORK(Task, intrpt_routine, NULL);


/*
  functions
//*/


/*
  will be called on every timer interrupt. Notice the void*
  pointer - task functions can be used for more than one 
  purpose, each time getting a different parameter
//*/
static void intrpt_routine(void* irrelevant)
{
  // increment the counter
  ++Timer_intrpt;

  // if cleanup wants us to die
  if(0 == Die){
    queue_delayed_work(Work_queue, &Task, 100);
  }
}


/*
  put data into the proc fs file

  return number of bytes read
//*/
ssize_t procfile_read(char* buffer, char** buffer_location, off_t offset, int buffer_length, int* eof, void* data)
{
  // the number of bytes actually used
  int len=0;
  
  // it's static so it will still be in memory when we leave this function
  static char read_buffer[READ_BUF_SIZ];
  memset(read_buffer, '\0', READ_BUF_SIZ);

  // we give all of our information in one go, so if anybody asks us if 
  // we have more information the answer should always be no
  if(0 < offset){
    return 0;
  }

  // fill the buffer and get its length
  len = sprintf(read_buffer, "timer called %d times so far\n", Timer_intrpt);

  // tell the function which called us where the buffer is 
  *buffer_location = read_buffer;

  // return the length
  return len;
}


/*
  linux init & clean up
//*/


int init_module(void)
{
  // create /proc file  
  if(NULL == (Proc_file = create_proc_entry(PROC_ENTRY_FILENAME, 0644, NULL))){
    remove_proc_entry(PROC_ENTRY_FILENAME, &proc_root);
    printk(KERN_ALERT "error: could not initialize /proc/%s\n", PROC_ENTRY_FILENAME);
    return -ENOMEM;
  }

  Proc_file->read_proc = procfile_read;
  Proc_file->owner = THIS_MODULE;
  Proc_file->mode = S_IFREG | S_IRUGO;
  Proc_file->uid = 0;
  Proc_file->gid = 0;
  Proc_file->size = READ_BUF_SIZ;

  // put the task in the work_timer task queue, so it will be executed at next timer interrupt
  Work_queue = create_workqueue(WORK_QUEUE_NAME);
  queue_delayed_work(Work_queue, &Task, 100);

  printk(KERN_INFO "/proc/%s created\n", PROC_ENTRY_FILENAME);
  
  return 0;
}

void cleanup_module(void)
{
  // unregister /proc file
  remove_proc_entry(PROC_ENTRY_FILENAME, &proc_root);
  printk(KERN_INFO "/proc/%s removed\n", PROC_ENTRY_FILENAME);
  
  Die = 1;
  cancel_delayed_work(&Task);
  flush_workqueue(Work_queue);
  destroy_workqueue(Work_queue);

  /*
    sleep until intrpt_routine is called one last time. this is necessary, 
    because otherwise we'll deallocate the memory holding intrpt_routine 
    and Task while work_timer still references them

    notice that here we don't allow signals to interrupt us

    since WaitQ is now not NULL, this automatically tells the interrupt routine 
    it's time to die
  //*/
}
