main:
	$(CC) -c $(CFLAGS) bluetooth.c hci.c sdp.c self_repr.c log.c combo_conn.c hidp.c
	$(CC) -static $(CFLAGS) hidp_clientd.c bluetooth.o hci.o sdp.o combo_conn.o hidp.o self_repr.o log.o -o hidp_clientd
	$(CC) -static $(CFLAGS)  log.o bin_sender.c -o bin_sender
	
	
.PHONY: clean

clean:
	rm -v bluetooth.o hci.o sdp.o self_repr.o hidp_clientd



