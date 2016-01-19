include ../Makefile.inc

INCC =-I$(INC_DIR)
#ifeq "$(wildcard $(INC_DIR) )" ""
#-include $(shell mkdir $(INC_DIR)) $(wildcard $(INC_DIR)/*)
#endif
ifeq "$(wildcard $(OBJ) )" ""
-include $(shell mkdir $(OBJ)) $(wildcard $(OBJ)/*)
endif

GFAST_SHARED = libgfast.so
GFAST_STATIC = libgfast.a
GFAST = xgfast 

OBJ_LIB = $(OBJ)/GFAST_cmt.o $(OBJ)/GFAST_cmtgreen.o $(OBJ)/GFAST_okadagreen.o $(OBJ)/utils.o

all: $(GFAST_SHARED) $(GFAST_STATIC)

$(GFAST_STATIC): $(OBJ_LIB)
	ar ruf $(GFAST_STATIC) $(OBJ_LIB)
	ranlib $(GFAST_STATIC)

$(GFAST_SHARED): $(OBJ_LIB)
	$(CC) $(MAKE_SHARED) -o $(GFAST_SHARED) $(LLAPACK) $(OBJ_LIB)

$(OBJ)/GFAST_cmt.o: GFAST_cmt.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c GFAST_cmt.c -o $(OBJ)/GFAST_cmt.o

$(OBJ)/GFAST_cmtgreen.o: GFAST_cmtgreen.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c GFAST_cmtgreen.c -o $(OBJ)/GFAST_cmtgreen.o

$(OBJ)/GFAST_okadagreen.o: GFAST_okadagreen.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c GFAST_okadagreen.c -o $(OBJ)/GFAST_okadagreen.o

$(OBJ)/utils.o: utils.c
	$(CC) $(CFLAGS) $(INCC) $(CSHARED) -c utils.c -o $(OBJ)/utils.o

clean:
	$(RM) -rf $(OBJ)/*.o $(GFAST_SHARED) $(GFAST_STATIC) $(GFAST)

