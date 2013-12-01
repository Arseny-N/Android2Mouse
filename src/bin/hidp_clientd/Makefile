main:
	$(CC) -c $(CFLAGS) bluetooth.c hci.c sdp.c self_repr.c log.c 
	$(CC) -static $(CFLAGS) hidp_clientd.c bluetooth.o hci.o sdp.o self_repr.o log.o -o hidp_clientd
	
.PHONY: clean

clean:
	rm -v bluetooth.o hci.o sdp.o self_repr.o hidp_clientd



