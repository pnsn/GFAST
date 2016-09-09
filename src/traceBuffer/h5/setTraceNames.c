
int traceBuffer_h5_setTraceNames(const char *traceName,
                                 struct h5trace_struct *traces)
{
    traces->groupName = (char *)calloc(lens, sizeof(char));
    strcpy(traces->groupName, traceName);
    return 0;
} 
