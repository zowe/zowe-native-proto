# Troubleshooting

## go: FSUM7351 not found

Ensure go is part of PATH

## Client connection error - Error: All configured authentication methods failed

Check that your username and password are correct.<br/>
For private keys, confirm that you can ssh into the LPAR/zVDT using it.

## FSUM9383 Configuration file `/etc/startup.mk' not found

You should be able to find the `startup.mk` file in `/samples`

- `cp /samples/startup.mk /etc/startup.mk` <br/>
  _source:_ https://www.ibm.com/support/pages/fsum9383-configuration-file-etcstartupmk-not-found

## Building zut.o - FSUM3221 xlc++: Cannot spawn program /usr/lpp/cbclib/xlc/exe/ccndrvr

One workaround is to add `CBC.SCCNCMP` to your system LINKLIST concatenation. Below is an example of doing this via SYSVIEW commands.

:warning: These commands could ruin your system if the linklist is corrupted. Do not modify the linklist unless you know what you are doing. :warning:

```
linklist
linkdef zowex from current
linklist zowex
add CBC.SCCNCMP
linkact zowex
set asid 1
linkupd *
```

Note 1: You may need to run `linkact zowex` after an IPL if your linklist has been reset.<br/>
Note 2: Depending on your z/OS configuration, you may need to replace `*` with your mask character. For example, `linkact zowex =`

## Downloading Go dependencies: tls: failed to verify certificate: x509: certificate signed by unknown authority

Update your `config.yaml` to include this property:

- `goBuildEnv: 'GOINSECURE="*" GOPROXY=direct GIT_SSL_NO_VERIFY=true'  # Allow fetching Go dependencies`
