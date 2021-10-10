// helloblockthread.c
/*
  demonstrates setting a thread to sleep
TODO: there are still some things unclear here... will see later 
though it compiles and uses the proc, also via the client


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

  original code from "the Linux Kernel Module Programming Guide",
  (C) Peter Jay Salzman, 2007-05-18
//*/

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ori Pomerantz, Peter Jay Salzman, indention: Lothar Rubusch ;)");
MODULE_DESCRIPTION(
	"demonstrates thread blocking and uses a client application in userspace");

#define MESSAGE_SIZE 10
#define PROC_ENTRY_FILENAME "lothars_sleep_file"

/*
  prototypes
//*/

int init_module(void);
void cleanup_module(void);

static ssize_t module_output(struct file *, char *, size_t, loff_t *);
static ssize_t module_input(struct file *, const char *, size_t, loff_t *);
static int module_open(struct inode *, struct file *);
int module_close(struct inode *, struct file *);
static int module_permission(struct inode *, int, struct nameidata *);

/*
  globals
//*/

// file operations for our proc file
static struct file_operations Filps_proc = {
	.read = module_output,
	.write = module_input,
	.open = module_open,
	.release = module_close,
};

// inode operations for our proc file - it might also be possible to set functions
static struct inode_operations Inops_proc = {
	.permission = module_permission,
};

// last message received will be kept here to prove that we can process input
static char Message[MESSAGE_SIZE];
static struct proc_dir_entry *My_proc_file = NULL;

// 1 if the file is currently "already" open and used by someone
int Already_open = 0;

/* 
   env func calls & macros
//*/

// queue of processses who ant our file
DECLARE_WAIT_QUEUE_HEAD(WaitQ);

/*
  funcs
//*/

/*
  since we use the file operations struct, we can't use the special proc output 
  provisions - we have to use a standard read function, too, which is this function

  returns number of read bytes or 0 if eof
//*/
static ssize_t module_output(struct file *file, char *buf, size_t len,
			     loff_t *offset)
{
	// static "private" finished variable
	static int finished = 0;

	int idx = 0;
	char message[MESSAGE_SIZE + 30];
	memset(message, '\0', (MESSAGE_SIZE + 30));

	// return 0 to signify eof
	if (finished) {
		finished = 0;
		return 0;
	}

	// copy from the global "buffer" to the local variable
	sprintf(message, "last input:%s\n", Message);
	for (idx = 0; (idx < len) && (message[idx]); ++idx) {
		put_user(message[idx], buf + idx);
	}

	// set finished flag
	finished = 1;

	return idx;
}

/*
  receives input form the user when the user writes to the /proc file

  returns the number of bytes = input characters
//*/
static ssize_t module_input(struct file *file, const char *buf, size_t length,
			    loff_t *offset)
{
	int idx = 0;
	// put the input into "Message" to use it later with "module_output"
	for (idx = 0; (idx < MESSAGE_SIZE - 1) && (idx < length); ++idx) {
		get_user(Message[idx], buf + idx);
	}

	// set up a standard zero terminated string
	Message[idx] = '\0';

	// we need to return the number of input characters used
	return idx;
}

/*
  called when the /proc file is opened

  returns 0 if access was allowed, "Already_open" will be set to 1, and blocks further calls
//*/
static int module_open(struct inode *inode, struct file *file)
{
	/*
    if the file's flags include O_NONBLOCK, it means the process 
    doesn't want to wait for the file. 
    In this case, if the file is already open, we should fail with 
    -EAGAIN, meaning "you'll have to try again", instead of blocking 
    a process which would rather stay awake.
  //*/
	if ((file->f_flags & O_NONBLOCK) && (Already_open)) {
		return -EAGAIN;
	}

	/*
    ATTENTION:
    this is the correct place for try_module_get(THIS_MODULE) because if a 
    process is in the loop, which is within the kernel module, the kernel 
    module must not be removed
  //*/
	try_module_get(THIS_MODULE);

	/*
    if the file is already open, wait until it closes agian 
  //*/
	while (Already_open) {
		int idx = 0, is_sig = 0;
		/*
      puts the current process, including any systemcalls, such as us, to sleep!
 
      execution will be resumed right after the function call, 
      - either because somebody called wake_up(&WaitQ) (only module_close does 
      that!!! when the file is closed) or 
      - when a signal, such as CTRL + C, is sent to the process
    //*/
		wait_event_interruptible(WaitQ, !Already_open);

		/*
      if we woke up because we got a signal we're not blocking, return -EINTR 
      (fail the systemcall).
      this allows the processes to be killed or stopped.
    //*/

		/*
      DISCUSSION: about 64 bit signals! (original - Salzman, 2006)
      
      Emmanuel Papirakis:
      This is a little update to work with 2.2.*. Signals are contained in two 
      words (64 bits) and are stored in a structure that contains an array of 
      two unsigned longs. We now have to make 2 checks in our if.
    
      Ori Pomerantz:
      Nobody promised me they'll never use more than 64 bits, or that this book 
      won't be used for a version of Linux with a word size of 16 bits. This 
      code would work in any case.
    //*/

		for (idx = 0; idx < _NSIG_WORDS && !is_sig; ++idx) {
			is_sig = current->pending.signal.sig[idx] &
				 ~current->blocked.sig[idx];
		}

		if (is_sig) {
			/*
	it's important to put module_put(THIS_MODULE) here, because for processes 
	where the open is interrupted there will never be a corresponding close. 

	if we don't decrement the usage count here, we will be left with a positive 
	usage count which we'll have no way to bring down to zero, giving us an immortal 
	module, which can only be killed by reooting the machine
      //*/
			module_put(THIS_MODULE);
			return -EINTR;
		}
	}
	/* 
     if we got here, Already_open must be zero!
  //*/

	// open the file
	Already_open = 1;
	return 0;
}

/*
  called when the /proc file is closed

  CHECK: why is this function not static??
  
  returns 0 if everything went fine - "Already_open" will be set to 0 again, too
//*/
int module_close(struct inode *inode, struct file *file)
{
	/*
    set "Already_open" to 0, so one of the processes in the WaitQ will be able to 
    set "Already_open back to one and to open the file. All the other processes 
    will be called when "Already_open", is back to one, wo they'll go back to sleep
  //*/
	Already_open = 0;

	/*
    wake up all the processes in WaitQ, so if anybody is waiting for the file, they 
    can have it
  //*/
	wake_up(&WaitQ);

	module_put(THIS_MODULE);

	return 0;
}

/*
  this function decides whether to allow an operation (return zero) or not allow it 
  (return a non-zero which indicates why it is not allowed)
  
  the operation can have one of the following values:
  0 - execute (run the "file" - meaningless in our case!)
  2 - write (input to the kernel module)
  4 - read (output from the kernel module)

  this is the real function that checks file permissions. the permissions returned 
  by ls -l are for reference only, and can be overridden here.

  returns 0 if success, else -EACCES
//*/
static int module_permission(struct inode *inode, int op, struct nameidata *nd)
{
	// we allow everybody to read from our module, but only root (uid = 0) may write it
	if ((op == 4) || ((op == 2) && (current->euid == 0))) {
		return 0;
	}

	// if it's anything else, access is denied
	return -EACCES; // FIXME
}

/*
  linux init & clean up
//*/

int init_module(void)
{
	if (NULL == (My_proc_file = create_proc_entry(PROC_ENTRY_FILENAME, 0644,
						      NULL))) {
		remove_proc_entry(PROC_ENTRY_FILENAME, &proc_root);
		printk(KERN_ALERT "error: could not initialize %s\n",
		       PROC_ENTRY_FILENAME);
		return -ENOMEM;
	}

	My_proc_file->owner = THIS_MODULE;
	My_proc_file->proc_iops = &Inops_proc;
	My_proc_file->proc_fops = &Filps_proc;
	My_proc_file->mode = S_IFREG | S_IRUGO | S_IWUSR;
	My_proc_file->uid = 0;
	My_proc_file->gid = 0;
	My_proc_file->size = 80;

	printk(KERN_INFO "%s created\n", PROC_ENTRY_FILENAME);

	return 0;
}

void cleanup_module(void)
{
	remove_proc_entry(PROC_ENTRY_FILENAME, &proc_root);
	printk(KERN_INFO "%s removed\n", PROC_ENTRY_FILENAME);
}
