// helloprocfs.c
/*
  using the /proc with seq_operations

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

  original version, Philippe Reynes, GPL

  original code from "the Linux Kernel Module Programming Guide",
  (C) Peter Jay Salzman, 2007-05-18
//*/


#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Lothar Rubusch <lothar.rubusch@nsn.com>");
MODULE_DESCRIPTION("demonstrates the usage of a procfs entry");

#define PROC_NAME "lothars_proc_entry"

// function declarations
int init_module(void);
void cleanup_module(void);
static void* proper_seq_start(struct seq_file*, loff_t*);
static void* proper_seq_next(struct seq_file*, void*, loff_t*);
static void proper_seq_stop(struct seq_file*, void*);
static int proper_seq_show(struct seq_file*, void*);
static int open_procfile(struct inode*, struct file*);

// file ops
static struct file_operations Fops_proc = {
  .owner    = THIS_MODULE,
  .open     = open_procfile,
  .read     = seq_read,
  .llseek   = seq_lseek,
  .release  = seq_release,
};

// seq ops
static struct seq_operations Sops_proc = {
  .start    = proper_seq_start,
  .next     = proper_seq_next,
  .stop     = proper_seq_stop,
  .show     = proper_seq_show,
};


/*
  function is called at the beginning of a sequence, i.e. when
  - the /proc file is read (first time)
  - after the function stops (end of sequence)

  returns NULL at the end of a sequence 
  or non-NULL at the beginning of a sequence
//*/
static void* proper_seq_start(struct seq_file* ptr_seqfile, loff_t* pos)
{
  static unsigned long cnt = 0;
  
  // beginning of a new sequence
  if(0 == *pos){
    // begin sequence, return a non-NULL
    return &cnt;
  }

  // end of the sequence, set pos to 0, return NULL
  *pos = 0;
  return NULL;
}  


/*
  function is called after the beginning of a sequence
  it's called until the return is NULL (ends the sequence)

  returns NULL
//*/
static void* proper_seq_next(struct seq_file* ptr_seqfile, void* ptr, loff_t* pos)
{
  unsigned long *tmp_ptr = (unsigned long*) ptr;
  ++(*tmp_ptr);
  ++(*pos);
  
  return NULL;
}


/*
  function is called at the end of a sequence
//*/
static void proper_seq_stop(struct seq_file* ptr_seqfile, void* ptr)
{
  // nothing to do - we use a static value in start()
}


/*
  this function is called for each "step of a sequence"

  returns 0
//*/
static int proper_seq_show(struct seq_file* ptr_seqfile, void* ptr)
{
  loff_t* seq_ptr_offset = (loff_t*) ptr;

  seq_printf(ptr_seqfile, "%Ld\n", *seq_ptr_offset);

  return 0;
}


/*
  function is called when /proc file is open
  
  returns the result of seq_open()
//*/
static int open_procfile(struct inode* inode, struct file* file)
{
  return seq_open(file, &Sops_proc);
}


/*
  linux init & clean up
//*/


int init_module(void)
{
  struct proc_dir_entry *entry = NULL;
  if(NULL != (entry = create_proc_entry(PROC_NAME, 0, NULL))){
    entry->proc_fops = &Fops_proc;
  }
  return 0;
}

void cleanup_module(void)
{
  remove_proc_entry(PROC_NAME, NULL);
  printk(KERN_INFO "/proc/%s removed\n", PROC_NAME);
}
