savedcmd_/root/linux-5.6.18/test_myself/RCU/rcu.mod := printf '%s\n'   rcu.o | awk '!x[$$0]++ { print("/root/linux-5.6.18/test_myself/RCU/"$$0) }' > /root/linux-5.6.18/test_myself/RCU/rcu.mod
