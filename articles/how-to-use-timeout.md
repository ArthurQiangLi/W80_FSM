# How to use the 'timeout' function?

## 1. Timeout Usage

How `timeout` works? 
- Data: the state machine struct holds `timeout` (stores the timeout ts in ms) and `timeout_flag` for this function. <br> Note all state share one `timeout` data. 
- `fsm_set_timeout_in_state_entry()` and `fsm_common_event_timeout()` is offered for this function.
- Again, because `timeout` is shared, to use it, you need to set it eveytime when you enter a state.
- `timeout_flag` is cleared forcely in `update()` every cycle. 


```c

static void lv2_s1_entry(void *fsmp)
{
	fsme_t *fsm = (fsme_t *)fsmp;

    uint32_t  timeout;
    timeout = 1000 * AUTO_RUN_TIMEOUT_S;
    fsm_set_timeout_in_state_entry(pfsm, timeout);
    USR_LOG("id=%d, set timeout = %d ms", id,  timeout);
}

static const fsme_trans_t Lv1_Trans_Fault[] =
{
    {lv1_event_key1,				LV1_STATE_IDLE},
	{fsm_common_event_timeout,		LV1_STATE_FAULT},
};

//in fsm_set_timeout_in_state_entry(void *pfsm, uint32_t set_ms):
    f->timeout      = set_ms;                                  
    f->ts_entry     = fsme_get_tick_count();                
    f->timeout_flag = 0;                                    

//in fsm_common_event_timeout:

    ts = fsme_get_tick_count();

    /*if have timeout setting , and it is sure timeout*/
    if((fsm->timeout>0 && ts - fsm->ts_entry)> fsm->timeout) {
        fsme_dbg_log("[event timeout] in [%s] set=%d ms.", fsm->name, fsm->timeout);
        fsm->timeout_flag = 1;
        return 1;
```


# END