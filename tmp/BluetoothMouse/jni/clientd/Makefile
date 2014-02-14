main:
	$(CC) -c bluetooth.c hci.c sdp.c self_repr.c hidp_clientdlib.c
	$(CC) -static hidp_clientd.c bluetooth.o hci.o sdp.o self_repr.o hidp_clientdlib.o -o hidp_clientd
	$(CC) -static sender.c bluetooth.o hci.o sdp.o self_repr.o hidp_clientdlib.o -o sender
	
.PHONY: clean

clean:
	rm -v bluetooth.o hci.o sdp.o self_repr.o hidp_clientd



