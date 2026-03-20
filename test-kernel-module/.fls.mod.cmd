savedcmd_fls.mod := printf '%s\n'   fls.o | awk '!x[$$0]++ { print("./"$$0) }' > fls.mod
