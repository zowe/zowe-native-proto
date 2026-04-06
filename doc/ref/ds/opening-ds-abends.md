# Opening Data Sets: Handling Abends

## Overview

Our error handling around some abends, such as [S913 (Insufficient Permissions)](https://www.ibm.com/docs/en/zos/2.5.0?topic=messages-iec150i), needs to be manually tested because it is non-trivial to implement an automated test to create an inaccessible data set. 

This document briefly describes how to:
- create a data set that other users cannot access
- implement a general error handling pattern around abend codes from LE C/C++ code

## Creating an inaccessible data set

In IBM's Resource Access Control Facility (RACF), making a data set "private" refers to ensuring only you (the owner) can access it while all other users are denied access.

### Create a Profile with UACC(NONE)

The most effective way to make a data set private is to create a RACF profile for it and set the **Universal Access Authority (UACC)** to **NONE**. This ensures that any user not specifically listed in the data set's access list is denied access.

- **For a single specific data set (Discrete Profile):**
  Use the `ADDSD` command to protect a specific data set.

  ```text
  ADDSD 'HLQ.DATASET.NAME' UACC(NONE)
  ```

- **For a group of data sets (Generic Profile):**
  If you want to protect multiple data sets that share a common naming pattern, use a generic profile.

  ```text
  ADDSD 'HLQ.PRIVATE.**' UACC(NONE) GENERIC
  ```

  *Note: The double asterisk (`**`) acts as a wildcard for all qualifiers following the prefix.*

### Test the S913 Abend

To properly test the S913 (Insufficient Permissions) abend, you will need two separate user IDs:
1. **User A (Creator)**: Create a partitioned data set and protect it using the commands above.
2. **User B (Tester)**: Attempt to list members for the protected data set created by User A. This triggers the S913 abend and manually validates the error handling.

When listing members for this data set, the following error appears:

`Insufficient permissions for opening data set 'HLQ.DATASET.NAME' (S913 abend)`

## Handling abends from `fopen`

If an abend is encountered when opening a data set using the `fopen` C function, the `errno` (C variable representing the current error number) is set to `EABEND` (92).

If the `errno` set by the `fopen` function is `EABEND`, you can use the `__amrc` structure defined in the `stdio.h` C header to access the abend code, for example:

```cpp
constexpr auto EABEND = 92;
if (errno == EABEND) // EABEND
{
    __amrc_type save_amrc = *__amrc;
    const auto abend_code = save_amrc.__code.__abend.__syscode;
    if (abend_code == 0x913)
    {
        // Insufficient permissions for this data set
        zds->diag.e_msg_len = sprintf(zds->diag.e_msg, "Insufficient permissions for opening data set '%s' (S913 abend)", dsn.c_str());
    }
}
```

Refer to the IBM article ["Using the __amrc structure"](https://www.ibm.com/docs/en/zos/3.1.0?topic=dip-using-amrc-structure) for more context on the diagnostic information contained within the `__amrc` structure.