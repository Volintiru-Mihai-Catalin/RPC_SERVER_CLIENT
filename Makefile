build:
	rpcgen -NC interface.x
server:
	gcc -o server server.c interface_svc.c interface_xdr.c -lnsl -Wall
run_server:
	./server tests/test2/userIDs.db tests/test2/resources.db tests/test2/approvals.db 2
client:
	gcc -o client client.c interface_clnt.c interface_xdr.c -lnsl -Wall
run_client:
	./client localhost tests/test2/client.in
clean:
	rm server client
regen:
	rm interface.h interface_svc.c interface_clnt.c