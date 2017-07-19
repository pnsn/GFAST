/*!
 * @brief  
 */
int core_events_removeExpiredEvents(const double maxTime, 
                                    const double currenTime,
                                    const int verbose,
                                    struct GFAST_activeEvents_struct *events)
{
    struct SA
    // Nothing to remove
    if (events->nev < 1){return 0;}
    // Create a copy of the events

}
