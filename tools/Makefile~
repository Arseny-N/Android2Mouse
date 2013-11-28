MNT ?= mnt
HOME = /home/arseni
syncBuild:
	guestmount -a $(HOME)/Documents/vm/images/ubuntu.qcow -i $(MNT)
	rsync -av $(MNT)/blue/build/ $(HOME)/Documents/p/blue/build
	chown -R arseni:arseni $(HOME)/Documents/p/blue/build
	guestunmount $(MNT)
	
syncSource:
	guestmount -a $(HOME)/Documents/vm/images/ubuntu.qcow -i $(MNT)
	rsync -av $(HOME)/Documents/p/blue/src/bin/ $(MNT)/blue/src/bin/
	guestunmount $(MNT)
launchVM:
	qemu-kvm $(HOME)/Documents/vm/images/ubuntu.qcow -boot d -net nic -net user -m 1024 -localtime
mount:
	guestmount -a $(HOME)/Documents/vm/images/ubuntu.qcow -i $(MNT)
umount:
	guestunmount $(MNT)
