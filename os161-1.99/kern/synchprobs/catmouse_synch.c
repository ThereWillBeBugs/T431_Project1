#include <types.h>
#include <lib.h>
#include <synchprobs.h>
#include <synch.h>

#define debug 1

/*
 * This simple default synchronization mechanism allows only creature at a time to
 * eat.   The globalCatMouseSem is used as a a lock.   We use a semaphore
 * rather than a lock so that this code will work even before locks are implemented.
 */

/*
 * Replace this default synchronization mechanism with your own (better) mechanism
 * needed for your solution.   Your mechanism may use any of the available synchronzation
 * primitives, e.g., semaphores, locks, condition variables.   You are also free to
 * declare other global variables if your solution requires them.
 */

/*  SYNC VARIABLES */
static struct semaphore *globalCatMouseSem; // global starter lock
static struct cv *cv_cat; // cat synch
static struct cv *cv_mouse; // mouse synch

// Adding all logical variables right now... can trim back as needed
static int numBowls;  // Keep track of total bowls
static int closedBowls; // Keep track of bowls being unused
static int openBowls; // Keep track of bowls NOT being used
static int catsWaiting; // keep track of how many cats are hungry
static int miceWaiting; // keep track of how many mice are hungry
static int catsEating;  // keep track of how many cats are eating
static int miceEating;  // keep track of how many mice are eating

static volatile char* bowlStatus; // keeps status of the bowl:
                                  // M: mouse is eating
                                  // C: cat is eating
                                  // E: bowl is unused

// Method declarations
void catmouse_sync_init(int bowls);
void catmouse_sync_cleanup(int bowls);
void cat_before_eating(unsigned int bowl);
void cat_after_eating(unsigned int bowl);
void mouse_before_eating(unsigned int bowl);
void mouse_after_eating(unsigned int bowl);



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
  // Create sem
  globalCatMouseSem = sem_create("globalCatMouseSem",1);
  if (globalCatMouseSem == NULL) { panic("Panic creating GLOBALCATMOUSESEM!"); }

  // Create cvcat
  cv_cat = cv_create("cv_cat");
  if (cv_cat == NULL) { panic("Panic creating CV_CAT!"); }

  // Create cvmouse
  cv_mouse = cv_create("cv_mouse");
  if (cv_mouse == NULL) { panic("Panic creating CV_MOUSE!"); }

  /* init bowls*/
  numBowls = bowls;
  bowlStatus = kmalloc(bowls * sizeof(char)); // Allocate mem
  int i = 0;
  for (; i < numBowls; i++) {
    bowlStatus[i] = 'E';  // Init to empty
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
  KASSERT(globalCatMouseSem != NULL);  sem_destroy(globalCatMouseSem);
  KASSERT(cv_cat != NULL); cv_destroy(cv_cat);
  KASSERT(cv_mouse != NULL); cv_destroy(cv_mouse);

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
  /* replace this default implementation with your own implementation of cat_before_eating */
  (void)bowl;  /* keep the compiler from complaining about an unused parameter */
  KASSERT(globalCatMouseSem != NULL);
  P(globalCatMouseSem);
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
  /* replace this default implementation with your own implementation of cat_after_eating */
  (void)bowl;  /* keep the compiler from complaining about an unused parameter */
  KASSERT(globalCatMouseSem != NULL);
  V(globalCatMouseSem);
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
  KASSERT(globalCatMouseSem != NULL);
  P(globalCatMouseSem);
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
  KASSERT(globalCatMouseSem != NULL);
  V(globalCatMouseSem);
}
