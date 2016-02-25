/** @file variable_queue.h
 *
 *  @brief Generalized queue module for data collection
 *
 *  @author Your name here
 **/



/** @def Q_NEW_HEAD(Q_HEAD_TYPE, Q_ELEM_TYPE) 
 *
 *  @brief Generates a new structure of type Q_HEAD_TYPE representing the head 
 *  of a queue of elements of type Q_ELEM_TYPE. 
 *  
 *  Usage: Q_NEW_HEAD(Q_HEAD_TYPE, Q_ELEM_TYPE); //create the type <br>
           Q_HEAD_TYPE headName; //instantiate a head of the given type
 *
 *  @param Q_HEAD_TYPE the type you wish the newly-generated structure to have.
 *         
 *  @param Q_ELEM_TYPE the type of elements stored in the queue.
 *         Q_ELEM_TYPE must be a structure.
 *  
 **/
 
#define Q_NEW_HEAD(Q_HEAD_TYPE, Q_ELEM_TYPE) ;

/** @def Q_NEW_LINK(Q_ELEM_TYPE)
 *
 *  @brief Instantiates a link within a structure, allowing that structure to be 
 *         collected into a queue created with Q_NEW_HEAD. 
 *
 *  Usage: <br>
 *  typedef struct Q_ELEM_TYPE {<br>
 *  Q_NEW_LINK(Q_ELEM_TYPE) LINK_NAME; //instantiate the link <br>
 *  } Q_ELEM_TYPE; <br>
 *
 *  A structure can have more than one link defined within it, as long as they
 *  have different names. This allows the structure to be placed in more than
 *  one queue simultanteously.
 *
 *  @param Q_ELEM_TYPE the type of the structure containing the link
 **/
 #define Q_NEW_LINK(Q_ELEM_TYPE) ;
 
 
/** @def Q_INIT_HEAD(Q_HEAD)
 *
 *  @brief Initializes the head of a queue so that the queue head can be used
 *         properly.
 *  @param Q_HEAD Pointer to queue head to initialize
 **/
#define Q_INIT_HEAD(Q_HEAD) ;

/** @def Q_INIT_ELEM(Q_ELEM, LINK_NAME)
 *
 *  @brief Initializes the link named LINK_NAME in an instance of the structure  
 *         Q_ELEM. 
 *  
 *  Once initialized, the link can be used to organized elements in a queue.
 *  
 *  @param Q_ELEM Pointer to the structure instance containing the link
 *  @param LINK_NAME The name of the link to initialize
 **/
#define Q_INIT_ELEM(Q_ELEM, LINK_NAME) ;
 
/** @def Q_INSERT_FRONT(Q_HEAD, Q_ELEM, LINK_NAME)
 *
 *  @brief Inserts the queue element pointed to by Q_ELEM at the front of the 
 *         queue headed by the structure Q_HEAD. 
 *  
 *  The link identified by LINK_NAME will be used to organize the element and
 *  record its location in the queue.
 *
 *  @param Q_HEAD Pointer to the head of the queue into which Q_ELEM will be 
 *         inserted
 *  @param Q_ELEM Pointer to the element to insert into the queue
 *  @param LINK_NAME Name of the link used to organize the queue
 *
 *  @return Void (you may change this if your implementation calls for a 
 *                return value)
 **/
#define Q_INSERT_FRONT(Q_HEAD, Q_ELEM, LINK_NAME) ;
 
/** @def Q_INSERT_TAIL(Q_HEAD, Q_ELEM, LINK_NAME) 
 *  @brief Inserts the queue element pointed to by Q_ELEM at the end of the 
 *         queue headed by the structure pointed to by Q_HEAD. 
 *  
 *  The link identified by LINK_NAME will be used to organize the element and
 *  record its location in the queue.
 *
 *  @param Q_HEAD Pointer to the head of the queue into which Q_ELEM will be 
 *         inserted
 *  @param Q_ELEM Pointer to the element to insert into the queue
 *  @param LINK_NAME Name of the link used to organize the queue
 *
 *  @return Void (you may change this if your implementation calls for a 
 *                return value)
 **/
#define Q_INSERT_TAIL(Q_HEAD, Q_ELEM, LINK_NAME) ;


/** @def Q_GET_FRONT(Q_HEAD)
 *  
 *  @brief Returns a pointer to the first element in the queue, or NULL 
 *  (memory address 0) if the queue is empty.
 *
 *  @param Q_HEAD Pointer to the head of the queue
 *  @return Pointer to the first element in the queue, or NULL if the queue
 *          is empty
 **/
#define Q_GET_FRONT(Q_HEAD) ;
 
/** @def Q_GET_TAIL(Q_HEAD)
 *
 *  @brief Returns a pointer to the last element in the queue, or NULL 
 *  (memory address 0) if the queue is empty.
 *
 *  @param Q_HEAD Pointer to the head of the queue
 *  @return Pointer to the last element in the queue, or NULL if the queue
 *          is empty
 **/
#define Q_GET_TAIL(Q_HEAD) ;


/** @def Q_GET_NEXT(Q_ELEM, LINK_NAME)
 * 
 *  @brief Returns a pointer to the next element in the queue, as linked to by 
 *         the link specified with LINK_NAME. 
 *
 *  If Q_ELEM is not in a queue or is the last element in the queue, 
 *  Q_GET_NEXT should return NULL.
 *
 *  @param Q_ELEM Pointer to the queue element before the desired element
 *  @param LINK_NAME Name of the link organizing the queue
 *
 *  @return The element after Q_ELEM, or NULL if there is no next element
 **/
#define Q_GET_NEXT(Q_ELEM, LINK_NAME) ;
 
/** @def Q_GET_PREV(Q_ELEM, LINK_NAME)
 * 
 *  @brief Returns a pointer to the previous element in the queue, as linked to 
 *         by the link specified with LINK_NAME. 
 *
 *  If Q_ELEM is not in a queue or is the first element in the queue, 
 *  Q_GET_NEXT should return NULL.
 *
 *  @param Q_ELEM Pointer to the queue element after the desired element
 *  @param LINK_NAME Name of the link organizing the queue
 *
 *  @return The element before Q_ELEM, or NULL if there is no next element
 **/
#define Q_GET_PREV(Q_ELEM, LINK_NAME) ;

/** @def Q_INSERT_AFTER(Q_HEAD, Q_INQ, Q_TOINSERT, LINK_NAME)
 *
 *  @brief Inserts the queue element Q_TOINSERT after the element Q_INQ
 *         in the queue.
 *
 *  Inserts an element into a queue after a given element. If the given
 *  element is the last element, Q_HEAD should be updated appropriately
 *  (so that Q_TOINSERT becomes the tail element)
 *
 *  @param Q_HEAD head of the queue into which Q_TOINSERT will be inserted
 *  @param Q_INQ  Element already in the queue
 *  @param Q_TOINSERT Element to insert into queue
 *  @param LINK_NAME  Name of link field used to organize the queue
 **/

#define Q_INSERT_AFTER(Q_HEAD,Q_INQ,Q_TOINSERT,LINK_NAME);

/** @def Q_INSERT_BEFORE(Q_HEAD, Q_INQ, Q_TOINSERT, LINK_NAME)
 *
 *  @brief Inserts the queue element Q_TOINSERT before the element Q_INQ
 *         in the queue.
 *
 *  Inserts an element into a queue before a given element. If the given
 *  element is the first element, Q_HEAD should be updated appropriately
 *  (so that Q_TOINSERT becomes the front element)
 *
 *  @param Q_HEAD head of the queue into which Q_TOINSERT will be inserted
 *  @param Q_INQ  Element already in the queue
 *  @param Q_TOINSERT Element to insert into queue
 *  @param LINK_NAME  Name of link field used to organize the queue
 **/

#define Q_INSERT_BEFORE(Q_HEAD,Q_INQ,Q_TOINSERT,LINK_NAME);

/** @def Q_REMOVE(Q_HEAD,Q_ELEM,LINK_NAME)
 * 
 *  @brief Detaches the element Q_ELEM from the queue organized by LINK_NAME, 
 *         and returns a pointer to the element. 
 *
 *  If Q_HEAD does not use the link named LINK_NAME to organize its elements or 
 *  if Q_ELEM is not a member of Q_HEAD's queue, the behavior of this macro
 *  is undefined.
 *
 *  @param Q_HEAD Pointer to the head of the queue containing Q_ELEM. If 
 *         Q_REMOVE removes the first, last, or only element in the queue, 
 *         Q_HEAD should be updated appropriately.
 *  @param Q_ELEM Pointer to the element to remove from the queue headed by 
 *         Q_HEAD.
 *  @param LINK_NAME The name of the link used to organize Q_HEAD's queue
 * 
 *  @return Void (if you would like to return a value, you may change this
 *                specification)
 **/
#define Q_REMOVE(Q_HEAD,Q_ELEM,LINK_NAME) ;

/** @def Q_FOREACH(CURRENT_ELEM,Q_HEAD,LINK_NAME) 
 *
 *  @brief Constructs an iterator block (like a for block) that operates
 *         on each element in Q_HEAD, in order.
 *
 *  Q_FOREACH constructs the head of a block of code that will iterate through
 *  each element in the queue headed by Q_HEAD. Each time through the loop, 
 *  the variable named by CURRENT_ELEM will be set to point to a subsequent
 *  element in the queue.
 *
 *  Usage:<br>
 *  Q_FOREACH(CURRENT_ELEM,Q_HEAD,LINK_NAME)<br>
 *  {<br>
 *  ... operate on the variable CURRENT_ELEM ... <br>
 *  }
 *
 *  If LINK_NAME is not used to organize the queue headed by Q_HEAD, then
 *  the behavior of this macro is undefined.
 *
 *  @param CURRENT_ELEM name of the variable to use for iteration. On each
 *         loop through the Q_FOREACH block, CURRENT_ELEM will point to the
 *         current element in the queue. CURRENT_ELEM should be an already-
 *         defined variable name, and its type should be a pointer to 
 *         the type of data organized by Q_HEAD
 *  @param Q_HEAD Pointer to the head of the queue to iterate through
 *  @param LINK_NAME The name of the link used to organize the queue headed
 *         by Q_HEAD.
 **/

#define Q_FOREACH(CURRENT_ELEM,Q_HEAD,LINK_NAME) ;
