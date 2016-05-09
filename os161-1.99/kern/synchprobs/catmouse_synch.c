#include <types.h>
#include <lib.h>
#include <synchprobs.h>
#include <synch.h>

#define debug 1

/*  SYNC VARIABLES */
static struct lock* globalCatMouseLock; // global starter lock
static struct cv* cv_cat; // control: cat allowed to eat
static struct cv* cv_mouse; // control: mouse allowed to eat

// Adding all logical variables right now... can trim back as needed
static int numBowls;  // Keep track of total bowls
static int closedBowls; // Keep track of bowls being unused
static int openBowls; // Keep track of bowls NOT being used
static int catsWaiting; // keep track of how many cats are hungry
static int miceWaiting; // keep track of how many mice are hungry
static int catsEating;  // keep track of how many cats are eating
static int miceEating;  // keep track of how many mice are eating

static struct lock** bowlLocks; // one lock for each bowl that is created
static volatile char* bowlStatus; // keeps status of the bowl:
                                  // M: mouse is eating
                                  // C: cat is eating
                                  // U: bowl is unused

// Method declarations
void catmouse_sync_init(int);
void catmouse_sync_cleanup(int);
void cat_before_eating(unsigned int);
void cat_after_eating(unsigned int);
void mouse_before_eating(unsigned int);
void mouse_after_eating(unsigned int);
bool isEating(char);

/*
 * The CatMouse simulation will call this function once before any cat or
 * mouse tries to each.
 *
 * You can use it to initialize synchronization and other variables.
 *
 * parameters: the number of bowls
 */
void
catmouse_sync_init(int bowls) {
  // Create global mutex
  lock_create("globalCatMouseLock");
  if (globalCatMouseLock == NULL) { panic("Panic creating GLOBALCATMOUSELOCK!"); }

  // Create cvcat
  cv_cat = cv_create("cv_cat");
  if (cv_cat == NULL) { panic("Panic creating CV_CAT!"); }

  // Create cvmouse
  cv_mouse = cv_create("cv_mouse");
  if (cv_mouse == NULL) { panic("Panic creating CV_MOUSE!"); }

  /* init bowls*/
  numBowls = bowls;
  bowlStatus = kmalloc(bowls * sizeof(char)); // Allocate mem
  bowlLocks = kmalloc(bowls * sizeof(struct lock)); // bollocks
  int i = 0;
  for (; i < numBowls; i++) {
    bowlStatus[i] = 'U';  // Init to unused
    lock_create("bowl" + i);  // make locks for each bowl
  }




  return;
}

/*
 * The CatMouse simulation will call this function once after all cat
 * and mouse simulations are finished.
 *
 * You can use it to clean up any synchronization and other variables.
 *
 * parameters: the number of bowls
 */
void
catmouse_sync_cleanup(int bowls)
{
  KASSERT(globalCatMouseLock != NULL);  lock_destroy(globalCatMouseLock);
  KASSERT(cv_cat != NULL); cv_destroy(cv_cat);
  KASSERT(cv_mouse != NULL); cv_destroy(cv_mouse);

  KASSERT(bowlLocks != NULL);
  int i = 0;
  for (; i < numBowls; i++) {
    lock_destroy(bowlLocks[i]);
  }

  if (bowlStatus != NULL) {
    kfree((void*) bowlStatus);
    bowlStatus = NULL;
  }

  (void) bowls;
}


/*
 * The CatMouse simulation will call this function each time a cat wants
 * to eat, before it eats.
 * This function should cause the calling thread (a cat simulation thread)
 * to block until it is OK for a cat to eat at the specified bowl.
 *
 * parameter: the number of the bowl at which the cat is trying to eat
 *             legal bowl numbers are 1..NumBowls
 *
 * return value: none
 */

void
cat_before_eating(unsigned int bowl) {
  bowl -= 1; // off by one prevention

  lock_acquire(globalCatMouseLock); // Grab the mutex
  while (isEating('M') || bowlStatus[bowl] == 'C') {  // While there is a mouse eating
                                                      // OR another cat is eating here

    if (debug) { // Debug info
      if (isEating('M')) kprintf("Cat waiting at bowl %d: mouse is eating right now", bowl);
      if (bowlStatus[bowl] == 'C') kprintf("Cat waiting at bowl %d: cat is eating at this bowl right now", bowl);
    }

    cv_wait(cv_cat, globalCatMouseLock);  // conditionally release mutex
                                          // until we are allowed to feed this cat
  }
  lock_acquire(bowlLocks[bowl]);  // reserve this table for the cat
  bowlStatus[bowl] = 'C';         // dinner is served
  lock_release(bowlLocks[bowl]);  // table secured, go ahead and release the lock while the cat chows down
  lock_release(globalCatMouseLock);
}

/*
 * The CatMouse simulation will call this function each time a cat finishes
 * eating.
 *
 * You can use this function to wake up other creatures that may have been
 * waiting to eat until this cat finished.
 *
 * parameter: the number of the bowl at which the cat is finishing eating.
 *             legal bowl numbers are 1..NumBowls
 *
 * return value: none
 */

void
cat_after_eating(unsigned int bowl) {
  bowl -= 1; // off by one prevention

}

/*
 * The CatMouse simulation will call this function each time a mouse wants
 * to eat, before it eats.
 * This function should cause the calling thread (a mouse simulation thread)
 * to block until it is OK for a mouse to eat at the specified bowl.
 *
 * parameter: the number of the bowl at which the mouse is trying to eat
 *             legal bowl numbers are 1..NumBowls
 *
 * return value: none
 */

void
mouse_before_eating(unsigned int bowl) {
  /* replace this default implementation with your own implementation of mouse_before_eating */
  (void)bowl;  /* keep the compiler from complaining about an unused parameter */
}

/*
 * The CatMouse simulation will call this function each time a mouse finishes
 * eating.
 *
 * You can use this function to wake up other creatures that may have been
 * waiting to eat until this mouse finished.
 *
 * parameter: the number of the bowl at which the mouse is finishing eating.
 *             legal bowl numbers are 1..NumBowls
 *
 * return value: none
 */

void
mouse_after_eating(unsigned int bowl) {
  /* replace this default implementation with your own implementation of mouse_after_eating */
  (void)bowl;  /* keep the compiler from complaining about an unused parameter */
}

/**
 * Check to see if a certain animal is eating right now
 *
 */
bool
isEating(char animal) {
  int i = 0;
  for (; i < numBowls; i++) {
    if (bowlStatus[i] == animal) {  // found this animal in the kitchen
      return true;
    }
  }

  return false; // time for me to eat
}
