# Description
This library adds native access functionality for NodeJS, running on Apple Platform(s), for storing and retrieving Keychain item values.

This library is an NodeJS addOn and can be compiled, if so desired, on MacOS. Doing so requires at least command line development tools to be installed.

Prebuilt `.node` binaries are included for both x64 and arm64, so that compile at install time is not required. The library, at runtime, automatically identifies which platform native binary should be used. (This is built into the TS/JS portion of this library.)

Current supported Keychain features via the native code bridge are:
- Add generic password value
- Update generic password value
- Retrieve generic password value
- Remove generic password value

Notes:
- In this current version and prior, only arm64 has been tested. The cross compile to x86 and use on that architecture has not been tested yet. Once a testing environment has been setup and appropriate testing is completed, this document will be updated.

<br>

# User Guide
## Installation
From command line
- ``` npm i --save @h3-d/native-ext-macos-keychain ```

Inside of package.json
- ``` "dependencies": { ... "@h3-d/native-ext-macos-keychain": "x.x.x" ... } ```

<br>

## Importing (TypeScript)
``` import { NativeKeychain } from '@h3-d/native-ext-macos-keychain'; ```

<br>

## Add/Update a Keychain Item

### With service ID
```
let result:IUpdateKeychainResult = keychain.setGenericPassword({
    key     : 'a-key-name',
    service : 'a-service-name',
    value   : 'a-value'
});
```

### Without service ID
```
let result:IUpdateKeychainResult = keychain.setGenericPassword({
    key     : 'a-key-name',
    value   : 'a-value'
});
```

### Processing the update result
*IUpdateKeychainResult interface definition:*
```
/** Update keychain item result model definition. */
export interface IUpdateKeychainResult {
    /** The action taken or attempted. */
    actionPerformed:EUpdateKeychainAction;

    /** If the operation was successful. */
    success:boolean;

    /** Error type. */
    errorType?: EKeychainOperationErrorType;

    /** Thrown error if native operation failed due to a system error. */
    error?:Error;
}
```

*Success Checks:*
- If `result.success` is true, the operation was completed without error.
- If `result.actionPerformed` is === `EUpdateKeychainAction.UPDATE`, the value was updated or added.

*Error Checks:*
- If `result.success` is false, the operation did not complete.
- Error type check for update:
  - `EKeychainOperationErrorType.SYSTEM` : A native level communication, system error, or linking error has occurred.
- `result.error` reference to the error thrown by the native lib or node.
- If `result.actionPerformed` === `EUpdateKeychainAction.REMOVE`, a null or undefined value was passed in params, resulting in a remove not update/add.

*Thrown Errors:*
- If platform compatibility issues are encountered, these are notified by a thrown Error from `setGenericPassword(...)`.

<br>

## Remove a Keychain Item
To remove a keychain item, use `keychain.setGenericPassword({...})`, but: 
- do not set `params.value`
- (or) set `params.value` to `null` or `undefined`.

Differences between remove vr update
- Result action will be remove: `result.actionPerformed` === `EUpdateKeychainAction.REMOVE`
- Additional errorType of `NOT_FOUND` is used to indicate that the keychain item was not found to remove. In this case, the `result.success` value will be `false`, with `errorType` set to not found, and `error` set to undefined.

Examples:

### With service ID (Without value passed)
```
let result:IUpdateKeychainResult = keychain.setGenericPassword({
    key     : 'a-key-name',
    service : 'a-service-name'
});
```

### Without service ID (With value passed)
```
let result:IUpdateKeychainResult = keychain.setGenericPassword({
    key     : 'a-key-name',
    value   : undefined
});
```

<br>

## Get a keychain value

### With service ID
```
let result:IGetKeychainResult = keychain.getGenericPassword({
    key     : 'a-key-name',
    service : 'a-service-name'
});
```

### Without service ID
```
let result:IGetKeychainResult = keychain.getGenericPassword({
    key     : 'a-key-name'
});
```

### Processing Result
*IGetKeychainResult interface definition:*
```
/** Get keychain item result model definition. */
export interface IGetKeychainResult {
    /** The string value, if found. */
    value?:string;

    /** If the operation was successful. */
    success:boolean;

    /** Error type. */
    errorType?: EKeychainOperationErrorType;

    /** Thrown error if native operation failed due to a system error. */
    error?:Error;
}
```

Checks:
- If `result.success` is `true`, then `result.value` will contain the value of what the Keychain item is set to.
- If `result.success` is `false`, and `errorType` is EKeychainOperationErrorType.NOT_FOUND, then keychain did not contain the expected item.
- All other success false + errorType/error values represent an issue with connecting to keychain or the native lib.


<br>

## Building
For TS transpile to JS:
- Use ``` npm run build ```

For native code build:
- Use ``` npm run build-native ```

As a reminder: The development tools for MacOS must be installed in order to build the native set of files.

<br>

# Notices
License, warranty, and disclaimer information.

<br>

## License
---
MIT
@Copyright 2024 H3-D (h3.d@icloud.com)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.


## Disclaimer
---
Mac, Keychain, and macOS are trademarks of Apple Inc., registered in the U.S. and other countries and regions. All native code references to the Apple SDK, Libraries, and macOS Keychain are owned solely by Apple Inc.

This software requires macOS to operate. The only intended purpose of this software is to extend NodeJS so that service and client applications, running on Apple platforms, have secure access to store credentials and secrets. This software is not a replacement for Keychain, but only acts as a bridge from NodeJS to Apple provided functionality.

To find more information on Apple trademarks:
- https://www.apple.com/legal/intellectual-property/trademark/appletmlist.html

Use of Apple software is licensed for each operating system and not provided as part of this documentation. Users of this software should perform their own due diligence for understanding of rules/agreements for making use of Apple platform provided SDK(s) and/or libraries. 


## Warranty
---
THIS SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

This disclaimer clarifies that the software library is offered without any guarantees. It highlights two key points:

 - No Explicit Warranties: There are no express warranties, which are specific guarantees made by the developer about the library's functionality or performance.
 - No Implied Warranties: The disclaimer also removes any implied warranties that might arise by law, such as merchantability (fit to be sold for a particular purpose) or fitness for a specific purpose (meeting your specific needs).
