/**
  ******************************************************************************
  * @file    fsme.c
  * @author  Arthur/liqiang(iamleary@live)
  * @version V4
  * @date    04-December-2017
  * @brief   2017-12-07 add comment 
  * @brief   2018-4-24  rewrite comment in english, to be compatiable with ANSI format
  * @brief   2018-5-22  fsme_update_state() will record 'ts_entry' automatically.
  * @brief   2020-4-11  rewrite to generate .lib file, do not include "rtos.h"
             note: for fsm_run(), what it calls under differnt situations.
             when no event          when a event occurs         after the event
             
             s1.action()            s1.exit();                  s2.action()
             osDelay(50)            s2.entry();                 osDelay(50)
                                    s2.action();
                                    osDelay(50);
  *                      
  ******************************************************************************
  */

#include "./stdtool_fsme.h"
#include <stdio.h> //to use print

/*******************************************************************************
******************************* Log   info setting *****************************
*******************************************************************************/

/* note: this is for task head file only.
    0= print none, 1= print err, 2= print err,usr, 3= print err, usr, dbg.*/
#define FSME_DEBUG_LEVEL        (2)

#if (FSME_DEBUG_LEVEL >= 1)
#define fsme_err_log(...)       __disable_irq(); \
                                printf("\r\n[FSM ERR %d]: ", xTaskGetTickCount()); \
                                printf(__VA_ARGS__);  \
                                __enable_irq();
#else
#define fsme_err_log(...)   
#endif            
                            
#if (FSME_DEBUG_LEVEL >= 2)
#define fsme_usr_log(...)       __disable_irq(); \
                                printf("\r\n[FSM USR %d]: ", xTaskGetTickCount()); \
                                printf(__VA_ARGS__);  \
                                __enable_irq();
#else
#define fsme_usr_log(...)   
#endif                         
                            
#if (FSME_DEBUG_LEVEL >= 3)                         
#define fsme_dbg_log(...)       __disable_irq(); \
                                printf(__VA_ARGS__);  \
                                __enable_irq();
#else
#define fsme_dbg_log(...)                         
#endif

/*******************************************************************************
******************************* Private functions ******************************
*******************************************************************************/

/*******************************************************************************
  * @brief  fsme_update_state, a state has several trans
  * @param  fsm
  * @retval none
  * @note   called by fsm_run.
  what is done:
  1.check trans table, if no event occurn, returns directly.
  2.if a event occurns, excute oldstate.exit_action(), then refresh state machine
    struct, then excute newstate.entry_action, finally reset timeout flag.
    
  *****************************************************************************/
static void fsme_update_state(fsme_t *fsm)
{
    uint32_t i = 0;
    uint32_t nbr;                                           //how many transitions for this state
    const fsme_trans_t *p_trans;                            //point to current transition table
    const fsme_state_t *p_newstate_stru;                    //the new state structure

    p_trans = fsm->trans;
    nbr     = fsm->trans_nbr;
                                                            
    for (i=0; i<nbr; i++){                                  //look up this state's trans table
                                                            //if a events have occured, we do the transition to new state.
        if(1 == p_trans[ i ].event(fsm)){
            if (fsm->states[fsm->current_state].exit_action){
                fsm->states[fsm->current_state].exit_action(fsm);//execute the exit() of old state
            }
                                                            //record last state and triggering event index num.
            fsm->last_state = fsm->current_state;
            fsm->last_event = i;
                                                            //update new state index
            fsm->current_state = p_trans[ i ].next_state;
            
                                                            //get pointer to the new state
            p_newstate_stru = & ( fsm->states[ fsm->current_state ] );
            fsme_usr_log("--> %s/%s", fsm->name, p_newstate_stru->name);
            
                                                            //load new state's trans table.
            fsm->trans     = p_newstate_stru->trans;
            fsm->trans_nbr = p_newstate_stru->trans_nbr;
                                                            //call new state entry(), and record the entry timestamp in ms.
            if (fsm->states[fsm->current_state].entry_action){
                fsm->states[fsm->current_state].entry_action(fsm); 
                fsm->ts_entry = fsme_get_tick_count(); 
            }
                                                            //clear timeout flag on new state entry.
            fsm->timeout_flag = 0;
            
            break;                                          //exit the lookup once found a event active
        }
    }
}

/*******************************************************************************
  * @brief  fsme_action
  
  * @param  fsm
  
  * @retval none
  *****************************************************************************/
static void fsme_action(fsme_t *fsm)
{
    if (fsm->states[fsm->current_state].action){
        fsm->states[fsm->current_state].action(fsm);
    }
}


/*******************************************************************************
******************************* Public  functions ******************************
*******************************************************************************/


/*******************************************************************************
  * @brief  fsm_run
  
  * @param  fsm structure pointer
  
  * @retval none.
  *****************************************************************************/
void fsm_run(fsme_t *fsm)
{
    if (0 == fsm->enable){
        return;
    }
    fsme_update_state(fsm);
    fsme_action(fsm);
    fsme_dbg_log("FSM '%s' is running", fsm->name);
}

/*******************************************************************************
  * @brief  fsm_start
  
  * @param  fsm structure ptr
  
  * @retval none
  *****************************************************************************/
void fsm_start(fsme_t *fsm)
{
    fsm->enable = 1;
    
    if (fsm->fsm_entry){
        fsm->fsm_entry(fsm);
    }
    if (fsm->states[fsm->current_state].entry_action){
        fsm->states[fsm->current_state].entry_action(fsm); 
        fsm->timeout_flag = 0;
    }
    fsme_dbg_log("FSM '%s' is enabled", fsm->name);
}

/*******************************************************************************
  * @brief  fem_disable
  
  * @param  fsm structure ptr
  
  * @retval none
  *****************************************************************************/
void fsm_stop(fsme_t *fsm)
{
    fsm->enable = 0;
    if (fsm->fsm_exit)
        fsm->fsm_exit(fsm);
    fsme_dbg_log("FSM: '%s' is disabled", fsm->name);
}


/*******************************************************************************
  * @brief  set a timeout in ms
  * @param  void *pfsm
  * @param  set_ms = how long is the timeout setting, in ms
  * @retval ONE, fixed.
  * @note   CAUTION, CALL THIS FUNCITON IN ENTRY FUNCTION.
  * @example 
     SET TIMEOUT 
    uint32_t  timeout;
    timeout = 1000 * AUTO_RUN_TIMEOUT_S;
    fsm_set_timeout_in_state_entry(pfsm, timeout);
    USR_LOG("id=%d, set timeout = %d ms", id,  timeout);

  *****************************************************************************/
int32_t fsm_set_timeout_in_state_entry(void *pfsm, uint32_t set_ms)
{
    fsme_t *f = (fsme_t *)pfsm;

    /* set timeout */
    f->timeout      = set_ms;                                  

    f->ts_entry     = fsme_get_tick_count();                
    /* reduntantly clear flag */
    f->timeout_flag = 0;                                    
    return 1;
}


/*******************************************************************************
  * @brief  fsm_common_event_timeout
  
  * @param  void *pfsm
  
  * @retval 1=if condition is met
  *****************************************************************************/
int32_t fsm_common_event_timeout(void *pfsm)
{
    fsme_t *fsm = (fsme_t *)pfsm;
    uint32_t ts = 0;
    
    ts = fsme_get_tick_count();

    /*if have timeout setting , and it is sure timeout*/
    if((fsm->timeout>0 && ts - fsm->ts_entry)> fsm->timeout) {
        fsme_dbg_log("[event timeout] in [%s] set=%d ms.", fsm->name, fsm->timeout);
        fsm->timeout_flag = 1;
        return 1;
    }else{
        return 0;
    }
}


/*******************************************************************************
  * @brief  fsm_common_event_true
  
  * @param  void *pfsm
  
  * @retval always return 1.
  *****************************************************************************/
int32_t fsm_common_event_true(void *pfsm)
{
    fsme_t *fsm = (fsme_t *)pfsm;

    fsme_usr_log("\r\n[common event_true] in [%s].", fsm->name);
    
    return 1;
}

/*******************************************************************************
  * @brief  fsm_common_event_false
  
  * @param  void *pfsm
  
  * @retval always return 0.
  *****************************************************************************/
int32_t fsm_common_event_false(void *pfsm)
{
    fsme_t *fsm = (fsme_t *)pfsm;

    fsme_usr_log("\r\n[common event_false] in [%s].", fsm->name);
    
    return 0;
}

/********************************* end of file ********************************/





