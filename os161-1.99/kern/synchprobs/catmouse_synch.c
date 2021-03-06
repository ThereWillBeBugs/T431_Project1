#include <types.h>
#include <lib.h>
#include <synchprobs.h>
#include <synch.h>

#define debug 1

/*  SYNC VARIABLES */
static struct lock *globalCatMouseLock; // global starter lock
static struct cv *cv_can_eat; // control: cat allowed to eat
//static struct cv *cv_mouse; // control: mouse allowed to eat

// Adding all logical variables right now... can trim back as needed
static int numBowls;  // Keep track of total bowls
static int catsEating;  // keep track of how many cats are eating
static int miceEating;  // keep track of how many mice are eating

static struct lock **bowlLocks; // one lock for each bowl that is created
static volatile char *bowlStatus; // keeps status of the bowl:
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
  globalCatMouseLock = lock_create("globalCatMouseLock");
  if (globalCatMouseLock == NULL) { panic("Panic creating GLOBALCATMOUSELOCK\n!"); }

  // Create cvcat
  cv_can_eat = cv_create("cv_cat"); // only need one for this implementation
  if (cv_can_eat == NULL) { panic("Panic creating CV_CAT!\n"); }

  // Create cvmouse
  //cv_mouse = cv_create("cv_mouse");
  //if (cv_mouse == NULL) { panic("Panic creating CV_MOUSE!\n"); }

  /* init bowls*/
  numBowls = bowls;
  bowlStatus = kmalloc(bowls * sizeof(char)); // Allocate mem
  bowlLocks = kmalloc(bowls * sizeof(struct lock)); // bollocks
  int i = 0;
  for (; i < numBowls; i++) {
    bowlStatus[i] = 'U';  // Init to unused
    bowlLocks[i] = lock_create("bowl" + i);  // make locks for each bowl
  }

  // init starting values
  catsEating = 0;
  miceEating = 0;

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
  KASSERT(cv_can_eat != NULL); cv_destroy(cv_can_eat);
  //KASSERT(cv_mouse != NULL); cv_destroy(cv_mouse);

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
  while (bowlStatus[bowl] == 'C' || miceEating > 0) {   // While there is a mouse eating                                                    // OR there is another cat at this bowl
    if (debug && miceEating > 0) kprintf("Cat waiting at bowl %d: mouse is eating right now! \n", bowl);
    cv_wait(cv_can_eat, globalCatMouseLock);  // conditionally release mutex                                        // until we are allowed to feed this cat
  }
  lock_acquire(bowlLocks[bowl]);  // reserve this table for the cat
  bowlStatus[bowl] = 'C';         // dinner is served
  catsEating++;
  lock_release(bowlLocks[bowl]);  // table secured, go ahead and release the lock while the cat chows down
  lock_release(globalCatMouseLock);
  if (debug) kprintf("Cat started eating at bowl %d\n", bowl);
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

  lock_acquire(bowlLocks[bowl]);
  bowlStatus[bowl] = 'U'; // cat finished eating
  lock_release(bowlLocks[bowl]);
  lock_acquire(globalCatMouseLock);
  catsEating--;
  cv_broadcast(cv_can_eat, globalCatMouseLock); // Alert any waiting animals
  lock_release(globalCatMouseLock);
  if (debug) kprintf("Cat finished eating at bowl %d\n", bowl);

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
  bowl -= 1; // off by one prevention

  lock_acquire(globalCatMouseLock); // Grab the mutex
  while( bowlStatus[bowl] == 'M'|| catsEating > 0) {   // While there is a cat eating
                                                         // OR another mouse at this bowl
    if (debug && catsEating > 0) kprintf("Mouse waiting at bowl %d: cat is eating right now! \n", bowl);
    cv_wait(cv_can_eat, globalCatMouseLock);  // conditionally release mutex
                                            // until we are allowed to feed this mouse
  }
  lock_acquire(bowlLocks[bowl]);  // reserve this table for the mouse
  bowlStatus[bowl] = 'M';         // dinner is served
  miceEating++;
  lock_release(bowlLocks[bowl]);  // table secured, go ahead and release the lock while the mouse chows down
  lock_release(globalCatMouseLock);
  if (debug) kprintf("Mouse started eating at bowl %d\n", bowl);
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
  bowl -= 1; // off by one prevention

  lock_acquire(bowlLocks[bowl]);
  if (debug) kprintf("Mouse finished eating at bowl %d\n", bowl);
  bowlStatus[bowl] = 'U'; // mouse finished eating
  lock_release(bowlLocks[bowl]);
  lock_acquire(globalCatMouseLock);
  miceEating--;
  cv_broadcast(cv_can_eat, globalCatMouseLock); // Alert animals waiting
  lock_release(globalCatMouseLock);
}
