/**
  ******************************************************************************
  * @file    fsme.h
  * @author  Arthur/liqiang(iamleary@live)
  * @version V1 build
  * @date    04-December-2017
  * @modify  [2]2018-1-3, rename 'timer' to 'ts_entry' in fsme_t.
  * @modify  [3]2018-4-24, rewrite comment.
    @modify  [4]2020-4-11, rewrite for make a .lib, note all the input param of 
                any fucntion the 'void *pfsm' is (fsme_t *pfsm) actually
  *
  ******************************************************************************
  */

/*******************************************************************************
********************* Define to prevent recursive inclusion ********************
*******************************************************************************/
#ifndef _FSME_H
#define _FSME_H

/*******************************************************************************
************************************ Includes **********************************
*******************************************************************************/

#include <stdint.h>

/*******************************************************************************
********************************* Exported macro *******************************
*******************************************************************************/
                                                            //get array item size
#define ARRAY_SIZE(arr)         (sizeof(arr) / sizeof((arr)[0]))
                                                            //a shortcut writing only
#define ARRAY_AND_SIZE(x)       (x), ARRAY_SIZE(x)

                                                            //to get systime for timout
extern uint32_t xTaskGetTickCount(void);
#define fsme_get_tick_count     xTaskGetTickCount

    
/*******************************************************************************
********************************* Exported types *******************************
*******************************************************************************/

typedef int32_t   (*fsme_event_t)(void *pfsm);              //fnct pointer fsme_event_t, format: int xxx(void *)
typedef void      (*fsme_action_t)(void *pfsm);             //fnct pointer fsme_action_t, format: void xxx(void *)

/*trans struct, a event and the deduced state */
typedef struct 
{
    fsme_event_t    event;                                  //event (input) function
    int32_t         next_state;
}fsme_trans_t;


/*state struct, has name, trans, and entry-action-exit fucntion */
typedef struct 
{
                                                            //entry-action-exit fnct of a state, all can be null
    fsme_action_t       entry_action;                       //void xxx(void *) like.
    fsme_action_t       action;
    fsme_action_t       exit_action;
    
    const fsme_trans_t  *trans;                             //a table shows what can this state go to.
    int32_t             trans_nbr;                          //item num of 'trans[]'
    char                *name;
    
}fsme_state_t;


/*main descriptor of a fsm, which contain a state table ( and every state contains a trans table)*/
typedef struct 
{
                                                            //***for the fsm
    char                *name;
    int32_t             enable;                             //swtich for this fsm, if =0, the fsm_run() will do nothing.
    const fsme_state_t  *states;                            //an array contains every states description, config only once. 
    int32_t             states_nbr;                         //how many states in 'states' 
    fsme_action_t       fsm_entry;                          //fnct run when fsm starts
    fsme_action_t       fsm_exit;                           //fnct run when fsm exit
    
                                                            //***for a state
    int32_t             current_state;                      //the index in 'states', range 0~(N-1), eg. '_S1' enumerate
    const fsme_trans_t  *trans;                             //an array, current state's trans's table, this and the below nbr are loaded from 'states' for use. 
    int32_t             trans_nbr;                          //how many trans in 'trans' 
    int32_t             last_state;                         //hold the last state item */
    int32_t             last_event;                         //hold the upate event which triggered this state item

    int32_t             timeout;                            //cfg timeout length of a state, in ms. note: it is for a state, but as timout is widely used, we save it here, see manual for details info.
    int32_t             timeout_flag;                       //indicate time is out, clear on fsm_update() in state entry() 
    int32_t             ts_entry;                           //record state item entry timestamp, in ms 
    int32_t             self_exit_flag;                     //used for self exit, user defined flag,
    
} fsme_t;


/*******************************************************************************
******************************* Exported functions *****************************
*******************************************************************************/

void    fsm_start      (fsme_t *pfsm);
void    fsm_run        (fsme_t *pfsm);
void    fsm_stop       (fsme_t *pfsm);

/* tool functions */
int32_t fsm_set_timeout_in_state_entry  (void *pfsm, uint32_t set_ms);
int32_t fsm_common_event_timeout        (void *pfsm);
int32_t fsm_common_event_true           (void *pfsm);
int32_t fsm_common_event_false          (void *pfsm);

#endif /* _FSME_H */

/********************************* end of file ********************************/



