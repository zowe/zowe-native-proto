# Troubleshooting

## CEEDUMP / 0C4 in list data sets (libzowed, load_storage_attr_from_catalog)

**Symptom:** CEEDUMP with CEE3204S (protection exception 0C4) in `libzowed.so` during list data sets with attributes, stack in `load_storage_attrs_from_catalog` / EBCDIC `basic_string` (CRTEQCXX).

**Cause:** Catalog response parsing in `zds_list_data_sets` did not validate `total_len` or `field_lens[]` from the CSI work area. Corrupt or unexpected values (e.g. from a different catalog level or malformed response) could cause:

- `load_storage_attr_from_catalog` to build a `std::string` from a huge `actual_len`, reading past the buffer → 0C4.
- `load_volsers_from_catalog` to use a huge `field_len` and read past the buffer or loop excessively.

**Fix (in `native/c/zds.cpp`):**

1. **Caller validation:** Before parsing catalog fields for each entry, validate `f->response.field.total_len` and each `field_lens[i]` (non-negative, ≤ total_len, sum ≤ total_len, volser field ≤ `MAX_VOLSER_FIELD_LEN`). If invalid, skip catalog/DSCB parsing for that entry and set `entry.volser = ZDS_VOLSER_UNKNOWN`, `entry.migrated = false`.
2. **load_storage_attr_from_catalog:** Cap `field_len` and ensure `actual_len <= (field_len - 2)` using a signed bound so huge lengths are rejected.
3. **load_volsers_from_catalog:** Reject `field_len < 0` or `field_len > MAX_VOLSER_FIELD_LEN`; on reject, set volser unknown and advance `csi_offset` by `field_len` so subsequent fields remain in sync.

If you see this CEEDUMP on an older build, upgrade to a version that includes these bounds checks.

## go: FSUM7351 not found

Ensure go is part of PATH

## Client connection error - Error: All configured authentication methods failed

Check that your username and password are correct.<br/>
For private keys, confirm that you can ssh into the LPAR/zVDT using it.

## FSUM9383 Configuration file `/etc/startup.mk' not found

You should be able to find the `startup.mk` file in `/samples`

- `cp /samples/startup.mk /etc/startup.mk` <br/>
  _source:_ <https://www.ibm.com/support/pages/fsum9383-configuration-file-etcstartupmk-not-found>

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
