main:
	$(CC) -c bluetooth.c hci.c sdp.c self_repr.c
	$(CC) -static hidp_clientd.c bluetooth.o hci.o sdp.o self_repr.o -o hidp_clientd
	
.PHONY: clean

clean:
	rm -v bluetooth.o hci.o sdp.o self_repr.o hidp_clientd



