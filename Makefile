include MakeVars

COMMON = obj/common/framework.o \
	obj/common/sdfmessagequeue.o \
	obj/common/sdfthreadpool.o \
	obj/common/sdfhashtable.o \
	obj/common/sdfexpiretimer.o
		
DTMF = obj/dtmf/Dtmfmain.o \
    obj/dtmf/Dtmf.o \
    obj/dtmf/DatagramSocket.o \
    obj/dtmf/Dtmf_MessageStack.o \
    obj/dtmf/ProductInfo.o \
    obj/dtmf/rtpptoxy_reqInfo.o
   
LOGCLI = obj/logcli/clm_log.a
XMLLIB = obj/xml/libxml.a
#LIBEDIT =obj/logcli/libedit.a
#----------------------------------------------------------------------
# Rules to build binary
#----------------------------------------------------------------------
bin: app

app: ${COMMON}   ${DTMF} 
	${CC} ${COMMON}  ${DTMF}  ${XMLLIB} ${LOGCLI} -lpthread -Wall -o bin/DTMFServer


#----------------------------------------------------------------------
# Clean Rules
#----------------------------------------------------------------------
clean:
	@find obj/ -name "*.o" -exec rm -f {} \;
	rm -f bin/DTMFServer

#----------------------------------------------------------------------
# Rules 
#----------------------------------------------------------------------

obj/common/%.o: ${BUILDPATH}/code/common/src/%.cpp
	$(CC) $(INCLUDES) -c $< -o $@
	
	
obj/dtmf/%.o: ${BUILDPATH}/code/dtmf/src/%.cpp
	$(CC) $(INCLUDES) -c $< -o $@

